#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "FractalStage.h"
#include "CommonFunctions.h"
#include "InfoPopupAnimation.h"
#include "TextRenderer.h"

// Initial number of iterations when drawing the fractal
const int MaxIterations = 50;// put this to 50

// Iteration count increase per frame
const int IterationIncr = 1;

// Initial values for fractal scaling
const float InitialFractalScale = 1.0;
#ifdef __BUILD_DESKTOP__
const float InitialFractalScaleFactor = 0.95;
#else
const float InitialFractalScaleFactor = 0.88;
#endif

// Initial re/im. Gotten by using a Mandelbrot examiner / trial&error
const double InitialRe = 0.25669830934;
const double InitialIm = 0.61722890149;

// Duration of the stage, in seconds
const float StageDuration = 20;

// Info popup texts
const char* const InfoPopupHeader = "cpu/fpu burn test";
const char* const InfoPopupMessage = "parallel mandelbrot";

// Initial width of the re(al) part
const float InitialReWidth = 0.5;

// log(2), where 2.0 is the "escape value"
const float LogEscape = 0.693147;

// Thread data
struct ThreadData 
{
    // Thread ID. This is a zero based running number, usable as index.
    int m_threadId;
    
    // First scanline to start calculating
    int m_firstScanline;
    
    // Stride; distance to next scanline to calculate as number of lines
    int m_stride;
    
    // Pointer to the stage object
    FractalStage* m_stage;
};

FractalStage::FractalStage(TextRenderer& textRenderer,
                           GLuint rectIndexBuffer, GLuint defaultFrameBuffer,
                           GLuint simpleColorProgram,
                           GLint simpleColorMvpLoc, GLint simpleColorColorLoc)
    : BaseStage(textRenderer, rectIndexBuffer, defaultFrameBuffer,
                simpleColorProgram, simpleColorMvpLoc, simpleColorColorLoc,
                StageDuration,
                InfoPopupHeader, InfoPopupMessage, 
                DefaultStageNearClip, DefaultStageFarClip),
      m_imageData(NULL),
      m_imageTexture(0),
      m_imageWidth(0),
      m_imageHeight(0),
      m_numIterations(MaxIterations),
      m_totalIterations(0),
      m_numFullCalculations(0),
      m_prevRenderTime(-1),
      m_zoomAnimation(NULL),
      m_threadsAlive(false),
      m_numFinishedThreads(0)
{
    memset(&m_processingThreadWait, 0, sizeof(m_processingThreadWait));
    memset(&m_mainThreadWait, 0, sizeof(m_mainThreadWait));
    memset(&m_mutex, 0, sizeof(m_mutex));
}

FractalStage::~FractalStage()
{
    LOG_DEBUG("FractalStage::~FractalStage()");
    TeardownImpl();
}

void FractalStage::UpdateScore(const TimeSample& now)
{
    BaseStage::UpdateScore(now);

    float fillRateFix = (m_viewportWidth * m_viewportHeight) / 100000;
    float score = (m_totalIterations / 10.0) * fillRateFix;
    m_stageData.m_cpuScore = (int)(score / 2);
    m_stageData.m_score = m_stageData.m_cpuScore;
    m_stageData.m_numImages = m_numFullCalculations;
    LOG_DEBUG("FractalStage::UpdateScore(): CPU score: %d",
              m_stageData.m_cpuScore);
}

void FractalStage::UploadImage()
{
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 m_imageWidth, m_imageHeight, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, m_imageData);
    
    // Must flush here so that the image data is uploaded before the 
    // processing threads mess up the memory area
    glFlush();
}

void FractalStage::RenderImpl(const TimeSample& time)
{
    // Clear the screen
    //glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    // Select the shader program & setup uniforms. This has to be done here
    // since the base class may have used different program / texture
    m_textRenderer.SetProgram();
    m_textRenderer.SetTexture(m_imageTexture);

    // Draw the image on screen
    CommonGL::Rect scaledRect(m_fullscreenRect);
    scaledRect.Scale(m_bitmapScale);
    DrawImage2D(scaledRect, m_viewportWidth, m_viewportHeight);

    // Advance the zoom animation
    bool zoomCompleted = m_zoomAnimation->Animate(time);

    // When the zoom animation completes, we need a new frame
    if ( m_threadsAlive && zoomCompleted )
    {
        // Wait till the threads have finished with the new frame
        WaitForProcessingThreads();
//        LOG_DEBUG("Frame render took %f seconds, fractal scale: %f, " \
//                  "bitmap scale: %f, frame ratio: %f, %d iterations",
//                  m_prevRenderTime, m_fractalScale, m_bitmapScale,
//                  m_renderTimeRatio, m_numIterations);

        m_totalIterations += m_numIterations;
        m_numIterations += IterationIncr;

        // Upload the new frame data
        UploadImage();

        // Reset zoom scale for new image
        m_bitmapScale = 1.0;
        
        // Reset the zoom animation
        m_zoomAnimation->Reset(1.0, 1.0 / m_fractalScaleFactor,
                               m_prevRenderTime);

        // Start processing next frame
        SignalProcessingThreads(true);
    }
    
    // If fadeout started, stop processing fractal frames but keep zooming
    if ( m_fadeOutAnimation->IsActive(time) )
    {
        m_threadsAlive = false;
        SignalProcessingThreads(false);
    }
}

//http://en.wikipedia.org/wiki/Mandelbrot_set
//http://warp.povusers.org/Mandelbrot/
//http://www.physics.emory.edu/~weeks/software/mandel.c
//http://plus.maths.org/content/computing-mandelbrot-set
//https://developer.mozilla.org/User:ksymeon/Mandelbrot_Explorer_in_JavaScript
// FlipCode: http://www.flipcode.com/archives/The_Art_of_Demomaking-Issue_08_Fractal_Zooming.shtml

inline void DrawScanline(uint32_t* data, int width,
                         float min_re, float re_step, float c_im, 
                         uint32_t colorMap[], int numIterations)
{
    float c_re = min_re;

    for ( int i = 0; i < width; i++ )
    {
        float z_re = c_re;
        float z_im = c_im;
        bool inside = true;
        
        float zMod;
        int iterNo;
        for ( iterNo = 0; iterNo < numIterations; iterNo++ )
        {
            float z_re2 = z_re * z_re;
            float z_im2 = z_im * z_im;

            if ( (z_re2 + z_im2) > 4.0 )
            {
                zMod = sqrtf(z_re2 + z_im2);
                inside = false;
                break;
            }

            float temp = z_re2 - z_im2 + c_re;
            z_im = 2 * z_re * z_im + c_im;
            z_re = temp;
        }

        if ( inside )
        {
            // Render the actual set with black (alpha = 1)
            *data++ = (0xFF << 24);
        }
        else
        {
            // Use the iteration count to determine color when outside set
            float smooth = iterNo + 1 - log(log(zMod)) / LogEscape;
            int colorIndex = (int)(smooth / MaxIterations * NumColors);
            *data++ = colorMap[colorIndex];
        }

        // Next real value
        c_re += re_step;
    }
}

void FractalStage::SignalProcessingThreads(bool updateData)
{
//    LOG_DEBUG("FractalStage::SignalProcessingThreads()");
    
    if ( updateData )
    {
        // The processing threads are about to draw a new frame; update the data
        float reWidth = InitialReWidth * m_fractalScale;
        float imWidth = reWidth * ((float)m_imageHeight / (float)m_imageWidth);
        m_reStep = reWidth / m_imageWidth;
        m_imStep = imWidth / m_imageHeight;
        m_minRe = m_re - (m_reStep * (m_imageWidth / 2));
        m_minIm = m_im - (m_imStep * (m_imageHeight / 2));
        m_fractalScale *= m_fractalScaleFactor;
    }
    
    // Reset the render timer
    m_renderTimer.Reset();
    
    pthread_mutex_lock(&m_mutex);
    m_numFinishedThreads = 0;
    for ( int i = 0; i < NumProcessingThreads; i++ )
    {
        m_processingThreadSignaled[i] = true;
    }
    pthread_cond_broadcast(&m_processingThreadWait);
    pthread_mutex_unlock(&m_mutex);
//    LOG_DEBUG("FractalStage::SignalProcessingThreads() done.");
}

bool FractalStage::AreProcessingThreadsDone()
{
    bool done = false;
    pthread_mutex_lock(&m_mutex);
    done = (m_numFinishedThreads == NumProcessingThreads);
    pthread_mutex_unlock(&m_mutex);

    return done;
}

void FractalStage::WaitForProcessingThreads()
{
//    LOG_DEBUG("FractalStage::WaitForProcessingThreads()");
    bool done = false;
    
    while ( !done ) 
    {
        pthread_mutex_lock(&m_mutex);
        if ( m_numFinishedThreads < NumProcessingThreads ) 
        {
            pthread_cond_wait(&m_mainThreadWait, &m_mutex);
        }
        done = (m_numFinishedThreads == NumProcessingThreads);
        pthread_mutex_unlock(&m_mutex);
    }
//    LOG_DEBUG("FractalStage::WaitForProcessingThreads() done.");
}

void FractalStage::ProcessingThreadLoop(int threadId, int firstScanline, 
                                        int stride)
{
    while ( m_threadsAlive )
    {
        // Wait for the main thread to signal
        pthread_mutex_lock(&m_mutex);
        while ( !m_processingThreadSignaled[threadId] ) 
        {
//            LOG_DEBUG("THREAD %d: Waiting for main thread", threadId);
            pthread_cond_wait(&m_processingThreadWait, &m_mutex);
//            LOG_DEBUG("THREAD %d: signaled by main thread", threadId);
        }
        m_processingThreadSignaled[threadId] = false;
        pthread_mutex_unlock(&m_mutex);
        
        // Check for exit condition again
        if ( !m_threadsAlive )
        {
            return;
        }

        // Calculate all the scanlines assigned to this thread
        uint32_t* data = m_imageData + (firstScanline * m_imageWidth);
        uint32_t dataStep = stride * m_imageWidth;
        float im = m_minIm + (firstScanline * m_imStep);
        float imStep = m_imStep * stride;
        
        for ( int i = firstScanline; i < m_imageHeight; i += stride )
        {
            DrawScanline(data, m_imageWidth, m_minRe, m_reStep, im,
                         m_colorMap, m_numIterations);
            data += dataStep;
            im += imStep;
            
            if ( !m_threadsAlive )
            {
                break;
            }
        }
        
        // Signal the main thread
        pthread_mutex_lock(&m_mutex);
        m_numFinishedThreads++;
        if ( m_numFinishedThreads == NumProcessingThreads )
        {
            // All threads finished computing; new frame is ready
            m_numFullCalculations++;
            LOG_DEBUG("All threads done; m_numFullCalculations = %d",
                      m_numFullCalculations);

            // Update the render time
            m_prevRenderTime = m_renderTimer.ElapsedTime();
        }
//        LOG_DEBUG("THREAD %d: Signaling main thread; m_numFinishedThreads: %d", 
//                  threadId, m_numFinishedThreads);
        pthread_cond_broadcast(&m_mainThreadWait);
        pthread_mutex_unlock(&m_mutex);
    }
}

void* FractalStage::ThreadMethod(void* data)
{
    ThreadData* threadData = static_cast<ThreadData*>(data);

    LOG_DEBUG("Running thread %d with scanline: %d, stride: %d", 
              threadData->m_threadId, 
              threadData->m_firstScanline, 
              threadData->m_stride);
    
    // Call the instance method to do the processing
    threadData->m_stage->ProcessingThreadLoop(threadData->m_threadId, 
                                              threadData->m_firstScanline, 
                                              threadData->m_stride);
    
    // Finally, delete the data object
    delete threadData;
    
    return NULL;
}

bool FractalStage::ViewportResized(int viewportWidth, int viewportHeight)
{
    LOG_DEBUG("FractalStage::ViewportResized()");

    // The processing threads must not be running at this point!
    if ( m_threadsAlive )
    {
        LOG_DEBUG("Processing threads are running, failed to resize!");
        return false;
    }
    
    // Call superclass method
    if ( !BaseStage::ViewportResized(viewportWidth, viewportHeight) )
    {
        LOG_DEBUG("BaseStage::ViewportResized() failed!");
        return false;
    }

    // Delete any previously allocated memory
    if ( m_imageData != NULL )
    {
        free(m_imageData);
        m_imageData = NULL;
    }

    // Figure out a size for the fractal image
    m_imageWidth = viewportWidth;
    m_imageHeight = viewportHeight;    
    LOG_DEBUG("FractalStage::ViewportResized(): image size %d x %d", 
              m_imageWidth, m_imageHeight);
    
    // Allocate memory for the fractal image
    m_imageData = (uint32_t*)malloc(m_imageWidth * m_imageHeight *
                                    sizeof(uint32_t));
    if ( m_imageData == NULL )
    {
        LOG_DEBUG("FractalStage::ViewportResized(): memory allocation failed");
        return false;
    }

    // Create the texture
    glDeleteTextures(1, &m_imageTexture);
    if ( !Create2DTexture(m_imageWidth, m_imageHeight, NULL, &m_imageTexture,
                          true, false) )
    {
        LOG_DEBUG("Failed to create OpenGL texture image for the fractal!");
        return false;
    }

    return true;
}

bool FractalStage::SetupImpl()
{
    LOG_DEBUG("FractalStage::Setup()");

    // No need for depth checking here
    glDisable(GL_DEPTH_TEST);

    // Set up the initial data
    m_fractalScale = InitialFractalScale;
    m_fractalScaleFactor = InitialFractalScaleFactor;
    m_re = InitialRe; 
    m_im = InitialIm; 
    
    // Reset state data
    m_totalIterations = 0;
    m_numIterations = MaxIterations;
    m_prevRenderTime = -1;

    // Create the color map (palette)
    for ( int i = 0; i < NumColors; i++ ) 
    {
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        
        if ( i >= 512 )
        {
            red = i - 512;
            green = 255 - red;
        }
        else if ( i >= 256 )
        {
            green = i - 256;
            blue = 255 - green;
        }
        else
        {
            blue = i;
        }
        m_colorMap[i] = (255 << 24) + (red << 16) + (green << 8) + blue;
    }

    // NOTE m_imageData will be allocated in ViewportResized() which
    // gets called by BaseStage::Setup()

    m_zoomAnimation = new ScalarAnimation(0.0, 0.0, 0.0, 0.0, &m_bitmapScale);
    
    // Setup threading; mutexes and conditionals
    if ( pthread_cond_init(&m_processingThreadWait, NULL) != 0 )
    {
        LOG_DEBUG("Failed to init m_processingThreadWait!");
        return false;
    }
    if ( pthread_cond_init(&m_mainThreadWait, NULL) != 0 )
    {
        LOG_DEBUG("Failed to init m_mainThreadWait!");
        return false;
    }
    if ( pthread_mutex_init(&m_mutex, NULL) != 0 ) 
    {
        LOG_DEBUG("Failed to init m_mutex!");
        return false;
    }
    
    // Initially no threads signaled
    for ( int i = 0; i < NumProcessingThreads; i++ )
    {
        m_processingThreadSignaled[i] = false;
    }
    
    // Create the processing threads
    m_threadsAlive = true;
    for ( int i = 0; i < NumProcessingThreads; i++ ) 
    {
        LOG_DEBUG("FractalStage::Setup(): creating thread %d..", i);
        pthread_t thread;
        ThreadData* data = new ThreadData();
        data->m_threadId = i;
        data->m_firstScanline = i;
        data->m_stride = NumProcessingThreads;
        data->m_stage = this;
        
        if ( pthread_create(&thread, NULL, 
                            &FractalStage::ThreadMethod, data) != 0 ) 
        {
            LOG_DEBUG("Thread creation failed!");
            return false;
        }
        m_threads.push_back(thread);
    }

    // Draw the first frame
    SignalProcessingThreads(true);
    WaitForProcessingThreads();
//    LOG_DEBUG("First frame render took %f seconds, %d iterations",
//              m_prevRenderTime, m_numIterations);

//    m_processingTime += m_prevRenderTime;
    m_totalIterations += m_numIterations;
    m_numIterations += IterationIncr;

    // Upload the first frame
    UploadImage();
    
    // Set up the initial zoom animation
    m_zoomAnimation->Reset(1.0, 1.0 / m_fractalScaleFactor, m_prevRenderTime);
    
    // Start drawing the next frame
    SignalProcessingThreads(true);

    return true;
}

void FractalStage::TeardownImpl()
{
    LOG_DEBUG("FractalStage::Teardown()");

    // Terminate the processing threads
    TerminateThreads();

    m_threads.clear();
    pthread_mutex_destroy(&m_mutex);
    pthread_cond_destroy(&m_processingThreadWait);
    pthread_cond_destroy(&m_mainThreadWait);
    memset(&m_processingThreadWait, 0, sizeof(m_processingThreadWait));
    memset(&m_mainThreadWait, 0, sizeof(m_mainThreadWait));
    memset(&m_mutex, 0, sizeof(m_mutex));

    if ( m_imageData != NULL )
    {
        free(m_imageData);
        m_imageData = NULL;
    }

    glDeleteTextures(1, &m_imageTexture);

    // Re-enable depth testing
    glEnable(GL_DEPTH_TEST);

    delete m_zoomAnimation;
    m_zoomAnimation = NULL;
    LOG_DEBUG("FractalStage::Teardown() done.");
}

void FractalStage::TerminateThreads()
{
    if ( m_threadsAlive )
    {
        m_threadsAlive = false;
        SignalProcessingThreads(false);
        
        for ( unsigned int i = 0; i < m_threads.size(); i++ )
        {
            pthread_join(m_threads[i], NULL);
        }
    }
}

void FractalStage::Abort()
{
    // Terminate the processing threads
    TerminateThreads();
}

