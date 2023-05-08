#include "FillrateStage.h"
#include "CommonFunctions.h"
#include "TextRenderer.h"
#include "ScalarAnimation.h"
#include "InfoPopupAnimation.h"

// How many polygons drawn on top of each other/frame
const int NumPolys = 200;

const float EllipseAnimDuration = 5;
const float MappedPhaseDuration = 8;

// Duration of the stage, in seconds
const float StageDuration = 60.0; // NOTE this will get adjusted on the fly

// Info popup texts
const char* InfoPopupHeader = "fill rate tests";
const char* InfoPopupMessage = "unlighted / vertex / pixel lighted polys";

// Near / far clip planes for this stage
const float StageNearClip = 0.1;
const float StageFarClip = 50.0;

// Info popup timings for this stage
const float InfoPopupInitialDelay = 1.5;
const float InfoPopupDisplayTime = 2.0;

FillrateStage::FillrateStage(TextRenderer& textRenderer,
                             GLuint rectIndexBuffer, GLuint defaultFrameBuffer,
                             GLuint simpleColorProgram,
                             GLint simpleColorMvpLoc, GLint simpleColorColorLoc)
    : BaseStage(textRenderer, rectIndexBuffer, defaultFrameBuffer,
                simpleColorProgram, simpleColorMvpLoc, simpleColorColorLoc,
                StageDuration,
                InfoPopupHeader, InfoPopupMessage, 0.1, 10.0,
                InfoPopupInitialDelay, InfoPopupDisplayTime),
      m_phase(Unlighted),
//      m_driverFaultDetected(false),
      m_initialPhaseTime(NULL),
      m_lightAnim(NULL),
      m_texture(0),
      m_normalSpecTexture(0),
      m_vertexLightShader(0),
      m_pixelLightShader(0),
      m_mappedLightShader(0),
      m_vlMvpLoc(-1),
      m_vlTextureLoc(-1),
      m_vlLightPosLoc(-1),
      m_plMvpLoc(-1),
      m_plTextureLoc(-1),
      m_plLightPosLoc(-1),
      m_mlMvpLoc(-1),
      m_mlTextureLoc(-1),
      m_mlNormalSpecTextureLoc(-1),
      m_mlLightPosLoc(-1)
{
}

FillrateStage::~FillrateStage()
{
    LOG_DEBUG("FillrateStage::~FillrateStage()");
    
    delete m_lightAnim;
    delete m_initialPhaseTime;
}

void FillrateStage::RenderNormally()
{
    // Draw the screen
    glDrawElements(GL_TRIANGLES, 2*3, GL_UNSIGNED_SHORT, NULL);

    if ( m_initialPhaseTime == NULL )
    {
        // Actual test not started yet, skip phase transition
        return;
    }
    
    float elapsed = m_initialPhaseTime->ElapsedTime();
    
    // Handle phase transitions
    switch ( m_phase )
    {
        case Unlighted:
            m_phase = VertexLighted;
            LOG_DEBUG("transition to VertexLighted");
            break;
        case VertexLighted:
            if ( elapsed >= EllipseAnimDuration )
            {
                LOG_DEBUG("transition to PixelLighted");
                m_phase = PixelLighted;
            }
            break;
        case PixelLighted:
            if ( elapsed >= EllipseAnimDuration )
            {
                LOG_DEBUG("transition to MappedLighted");
                m_phase = MappedLighted;
            }
            break;
        case MappedLighted:
            // No more state transitions
            break;
    }
}

void FillrateStage::RenderBenchmark(const TimeSample& now)
{
    // Finish any pending rendering
    glFinish();
   // LOG_DEBUG("Start ---> 0x%x", glGetError());
    // Start the clock
    m_renderTime.Reset();
    
    // Render the polygons
    for ( int i = 0; i < NumPolys; i++ )
    {
        glDrawElements(GL_TRIANGLES, 2*3, GL_UNSIGNED_SHORT, NULL);
    }
    
    // Flush any pending rendering
//    glFlush();
    glFinish();
    LOG_DEBUG("** DONE **! : 0x%x", glGetError());
    
    // Measure the rendering time
    float sample = m_renderTime.ElapsedTime();

    // Normalize the sample by number of screen pixels
    double numPixels = (double)m_viewportWidth * (double)m_viewportHeight * (double)NumPolys;
    double mpixPerSec = numPixels / (sample * 1000000.0);
    
    LOG_DEBUG("Time sample: %f sec, %f Mpix / s", sample, mpixPerSec);
    
    // Sanity check the result; on some broken OpenGL implementation glFinish() might
    // return before actually completing the draw; discard such flawed results.
    if ( mpixPerSec > 3000 )
    {
        LOG_INFO("mpixPerSec sanity check failed! Discarding the result.");
        mpixPerSec = 0.0;
        m_stageData.m_missingFeatures = "glFinish";
//        m_driverFaultDetected = true;
    } else if ( sample < 0.11 ) {
        LOG_INFO("Time sample sanity check failed! Discarding the result.");
        mpixPerSec = 0.0;
        m_stageData.m_missingFeatures = "glFinish";
//        m_driverFaultDetected = true;
    }
    
    // Process the results
    switch ( m_phase )
    {
        case Unlighted:
            m_stageData.m_unlightedFillRate = (float)mpixPerSec;
            LOG_DEBUG("Unlighted done");

            // actual test started here
            m_initialPhaseTime = new TimeSample();
            break;
        case VertexLighted:
            m_stageData.m_vertexLightedFillRate = (float)mpixPerSec;
            LOG_DEBUG("Vertex lighted done");
            break;
        case PixelLighted:
            {
                m_stageData.m_pixelLightedFillRate = (float)mpixPerSec;
                LOG_DEBUG("Pixel lighted done");
            }
            break;
        case MappedLighted:
            {
                m_stageData.m_mappedLightedFillRate = (float)mpixPerSec;
                LOG_DEBUG("Mapped lighted done");

                // Adjust stage end time
                float elapsed = now.ElapsedTimeSince(m_firstFrameTime);
                float duration = elapsed + MappedPhaseDuration + m_fadeDuration;
                UpdateStageDuration(duration);
            }
            break;
    }
    
    // Reset the phase time
    m_initialPhaseTime->Reset();
}

void FillrateStage::RenderImpl(const TimeSample& now)
{
    // We have no use for depth information
    glClear(GL_COLOR_BUFFER_BIT);
    if ( glIsEnabled(GL_DEPTH_TEST) )
    {
        glDisable(GL_DEPTH_TEST);
    }

    // Update the light animation
    m_lightAnim->Animate(now);

    bool benchmark = false;

    // Phase-specific setup
    switch ( m_phase )
    {
        case Unlighted:
            m_textRenderer.SetProgram();
            m_textRenderer.SetTexture(m_texture);
            benchmark = ( m_stageData.m_unlightedFillRate < 0.0f ) ;
            break;
        case VertexLighted:
            glUseProgram(m_vertexLightShader);
            glActiveTexture(GL_TEXTURE0);
            glUniform1i(m_vlTextureLoc, 0);
            glBindTexture(GL_TEXTURE_2D, m_texture);
            glUniformMatrix4fv(m_vlMvpLoc, 1, GL_FALSE, m_orthoProjectionMatrix);
            glUniform3fv(m_vlLightPosLoc, 1, m_lightPos);
            benchmark = ( m_stageData.m_vertexLightedFillRate < 0.0f ) ;
            break;
        case PixelLighted:
            glUseProgram(m_pixelLightShader);
            glActiveTexture(GL_TEXTURE0);
            glUniform1i(m_plTextureLoc, 0);
            glBindTexture(GL_TEXTURE_2D, m_texture);
            glUniformMatrix4fv(m_plMvpLoc, 1, GL_FALSE, m_orthoProjectionMatrix);
            glUniform3fv(m_plLightPosLoc, 1, m_lightPos);
            benchmark = ( m_stageData.m_pixelLightedFillRate < 0.0f );
            break;
        case MappedLighted:
            glUseProgram(m_mappedLightShader);
            glActiveTexture(GL_TEXTURE0);
            glUniform1i(m_mlTextureLoc, 0);
            glBindTexture(GL_TEXTURE_2D, m_texture);
            glActiveTexture(GL_TEXTURE1);
            glUniform1i(m_mlNormalSpecTextureLoc, 1);
            glBindTexture(GL_TEXTURE_2D, m_normalSpecTexture);
            glUniformMatrix4fv(m_mlMvpLoc, 1, GL_FALSE, m_orthoProjectionMatrix);
            glUniform3fv(m_mlLightPosLoc, 1, m_lightPos);
            benchmark = ( m_stageData.m_mappedLightedFillRate < 0.0f );
            break;
    }

    // Bind VBOs
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_rectangleIndexBuffer);

    // Set the vertex attrib pointers
    SetVertexAttribsPointers();

    if ( m_fadeOutAnimation->IsActive(now) ||
        !m_infoPopupAnimation->HasCompleted(now) )
    {
        // Do not run benchmark while fade in/out or popup is happening
        benchmark = false;
    }

    if ( benchmark )
    {
        RenderBenchmark(now);
    }
    else
    {
        RenderNormally();
    }
}

bool FillrateStage::SetupGeometry()
{
    int x = 0;
    int y = 0;
    int width = m_viewportWidth;
    int height = m_viewportHeight;
    
    // Adjust x/y according to viewport size so that 0,0 is upper left
    x -= m_viewportWidth / 2;
    y = -y + (m_viewportHeight / 2) - height;
    
    // NOTE : this will not produce unit normal; normalize() in shader
    const float nx = 0.2;
    const float ny = nx;
    const float nz = 1.0;
    
    VertexAttribs vertices[] = {
        { x, y + height, 0,         0, 1, -nx, ny, nz }, // top left
        { x, y, 0,                  0, 0, -nx, -ny, nz }, // bottom left
        { x + width, y, 0,          1, 0,  nx, -ny, nz}, // bottom right
        { x + width, y + height, 0, 1, 1, nx, ny, nz } // top right
    };
    
    glGenBuffers(1, &m_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices),
                 vertices, GL_STATIC_DRAW);
    
    return (glGetError() == GL_NO_ERROR);
}

bool FillrateStage::SetupImpl()
{
    m_stageData.m_unlightedFillRate = -1;
    m_stageData.m_vertexLightedFillRate = -1;
    m_stageData.m_pixelLightedFillRate = -1;
    m_stageData.m_mappedLightedFillRate = -1;

    // Load the texture to use as background
    if ( !Load2DTextureFromBundle("menu_background.jpg", &m_texture,
                                  true, false) )
    {
        return false;
    }
    if ( !Load2DTextureFromBundle("menu_background_maps.jpg",
                                  &m_normalSpecTexture, true, false) )
    {
        return false;
    }

    // Load shaders
    if ( !LoadShaderFromBundle("VertexLight", &m_vertexLightShader) )
    {
        return false;
    }
    m_vlMvpLoc = glGetUniformLocation(m_vertexLightShader, "mvp_matrix");
    m_vlTextureLoc = glGetUniformLocation(m_vertexLightShader, "texture");
    m_vlLightPosLoc = glGetUniformLocation(m_vertexLightShader, "light_pos");

    if ( !LoadShaderFromBundle("PixelLight", &m_pixelLightShader) )
    {
        return false;
    }
    m_plMvpLoc = glGetUniformLocation(m_pixelLightShader, "mvp_matrix");
    m_plTextureLoc = glGetUniformLocation(m_pixelLightShader, "texture");
    m_plLightPosLoc = glGetUniformLocation(m_pixelLightShader, "light_pos");

    if ( !LoadShaderFromBundle("MappedLight", &m_mappedLightShader) )
    {
        return false;
    }
    m_mlMvpLoc = glGetUniformLocation(m_mappedLightShader, "mvp_matrix");
    m_mlTextureLoc = glGetUniformLocation(m_mappedLightShader, "texture");
    m_mlNormalSpecTextureLoc =
            glGetUniformLocation(m_mappedLightShader, "normalspec_texture");
    m_mlLightPosLoc = glGetUniformLocation(m_mappedLightShader, "light_pos");

    // Create geometry for the full screen quad
    if ( !SetupGeometry() )
    {
        LOG_DEBUG("FillrateStage::SetupImpl(): SetupGeometry() failed");
        return false;
    }

    m_lightAnim = new EllipticPathAnimation((m_viewportWidth / 2) * 0.8,
                                            (m_viewportHeight / 2) * 0.8,
                                            (m_viewportWidth / 2),
                                            EllipseAnimDuration, m_lightPos);
    // Reset test phase
    m_phase = Unlighted;

    // Disable writing depth
    glDepthMask(false);

    return true;
}

void FillrateStage::TeardownImpl()
{
    glDeleteTextures(1, &m_texture);
    glDeleteTextures(1, &m_normalSpecTexture);
    UnloadShader(m_vertexLightShader);
    UnloadShader(m_pixelLightShader);
    UnloadShader(m_mappedLightShader);

    // Make sure depth mask is on for next stage
    glDepthMask(true);

    delete m_lightAnim;
    m_lightAnim = NULL;
    
    delete m_initialPhaseTime;
    m_initialPhaseTime = NULL;
}

void FillrateStage::UpdateScore(const TimeSample& now)
{
    BaseStage::UpdateScore(now);

    // Calculate the score for this stage based on the fillrates, weighing the
    // rates according to their performance impact.
    float unlightedScore = (m_stageData.m_unlightedFillRate * 0.75);
    float vertexLightedScore = (m_stageData.m_vertexLightedFillRate * 0.8);
    float pixelLightedScore = m_stageData.m_pixelLightedFillRate;
    float mappedLightedScore = (m_stageData.m_mappedLightedFillRate * 1.2);
    LOG_DEBUG("FillrateStage::UpdateScore(): raw scores: %f, %f, %f, %f",
              unlightedScore, vertexLightedScore, pixelLightedScore,
              mappedLightedScore);
    
    float total = unlightedScore + vertexLightedScore + pixelLightedScore +
                  mappedLightedScore;
    m_stageData.m_fillRateScore = (int)(total);
    m_stageData.m_score = m_stageData.m_fillRateScore;
    LOG_DEBUG("FillrateStage::UpdateScore(): fill rate scores: %d",
              m_stageData.m_fillRateScore);
}

