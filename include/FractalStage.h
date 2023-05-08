#ifndef FRACTALSTAGE_H
#define FRACTALSTAGE_H

#include <pthread.h>
#include <vector>

#include "BaseStage.h"

// Forward declarations
class ScalarAnimation;

// Number of processing threads used to calculate the fractal
static const int NumProcessingThreads = 4;

// Number of colors in the palette
static const int NumColors = 768;

/**
 * Fractal stage; multicore CPU burn testing.
 *
 * @author Matti Dahlbom
 * @since 0.1
 */
class FractalStage : public BaseStage
{
public:
    FractalStage(TextRenderer& textRenderer,
                 GLuint rectIndexBuffer, GLuint defaultFrameBuffer,
                 GLuint simpleColorProgram,
                 GLint simpleColorMvpLoc, GLint simpleColorColorLoc);
    virtual ~FractalStage();

public: // From BaseStage
    bool ViewportResized(int viewportWidth, int viewportHeight);

protected: // From BaseStage
    bool SetupImpl();
    void RenderImpl(const TimeSample& time);
    void TeardownImpl();
    void UpdateScore(const TimeSample& now);
    void Abort();
    
private:
    void TerminateThreads();
    void UploadImage();
    void SignalProcessingThreads(bool updateData);
    void WaitForProcessingThreads();
    bool AreProcessingThreadsDone();
    void ProcessingThreadLoop(int threadId, int firstScanline, int stride);
    static void* ThreadMethod(void* data);
    
private: // Data
    uint32_t* m_imageData;
    GLuint m_imageTexture;
    int m_imageWidth;
    int m_imageHeight;
    uint32_t m_colorMap[NumColors];
    int m_numIterations;
    int m_totalIterations;

    // Mandelbrot zooming
    double m_re;
    double m_im;
    double m_minRe;
    double m_minIm;
    double m_reStep;
    double m_imStep;
    float m_fractalScale;
    float m_fractalScaleFactor;
    float m_bitmapScale; 

    // Render time measuring
    int m_numFullCalculations; // number of calculated fractal frames
    TimeSample m_renderTimer;
    float m_prevRenderTime;
    
    // Zooming animation
    ScalarAnimation* m_zoomAnimation;
    
    // Threading resources
    bool m_threadsAlive;
    int m_numFinishedThreads;    
    std::vector<pthread_t> m_threads;
    pthread_cond_t m_processingThreadWait;
    pthread_cond_t m_mainThreadWait;
    pthread_mutex_t m_mutex;
    bool m_processingThreadSignaled[NumProcessingThreads];
    
    // Cumulative frame processing time; used to calculate score
//    float m_processingTime;
};

#endif // FRACTALSTAGE_H
