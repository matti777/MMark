#include <stdio.h>
#include <algorithm>

#include "OpenGLAPI.h"
#include "BaseStage.h"
#include "CommonFunctions.h"
#include "MatrixOperations.h"
#include "GLController.h"
#include "TextRenderer.h"
#include "InfoPopupAnimation.h"

// FOV value
static const float DefaultFov = 60;

// Default fade in/out duration (in seconds)
static const float DefaultFadeDuration = 3.0;

// Info popup constants
static const int MinInfoHeight = 150;
static const int MaxInfoHeight = 450;
static const float InfoPopupTransitionDuration = 0.5;
static const uint32_t InfoPopupMaxAlpha = 150;

BaseStage::BaseStage(TextRenderer& textRenderer, GLuint rectIndexBuffer,
                     GLuint defaultFrameBuffer, GLuint simpleColorProgram,
                     GLint simpleColorMvpLoc, GLint simpleColorColorLoc,
                     float stageDuration,
                     const char* infoPopupHeader, const char* infoPopupMessage,
                     float nearClip, float farClip,
                     float popupInitialDelay, float popupDisplayTime)
    : m_interactiveMode(false),
      m_numFrames(0),
      m_stageDuration(stageDuration),
      m_stageData(StageData()),
      m_fadeDuration(DefaultFadeDuration),
      m_fadeInAnimation(NULL),
      m_fadeOutAnimation(NULL),
      m_defaultFrameBuffer(defaultFrameBuffer),
      m_nearClip(nearClip),
      m_farClip(farClip),
      m_viewportWidth(0),
      m_viewportHeight(0),
      m_rectIndexBuffer(rectIndexBuffer),
      m_vertexBuffer(0),
      m_simpleColorProgram(simpleColorProgram),
      m_simpleColorMvpLoc(simpleColorMvpLoc),
      m_simpleColorColorLoc(simpleColorColorLoc),
      m_textRenderer(textRenderer),
      m_infoPopupInitialDelay(popupInitialDelay),
      m_infoPopupDisplayTime(popupDisplayTime),
      m_infoPopupHeader(infoPopupHeader),
      m_infoPopupMessage(infoPopupMessage),
      m_infoPopupTexture(0),
      m_infoPopupHeaderScale(0.0),
      m_infoPopupMessageScale(0.0),
      m_infoPopupHeaderYOffset(0),
      m_infoPopupHeaderXOffset(0),
      m_infoPopupMessageYOffset(0),
      m_infoPopupMessageXOffset(0),
      m_infoPopupTop(0.0),
      m_infoPopupHeight(0.0),
      m_infoPopupAnimation(NULL),
      m_fullscreenRectVertexBuffer(0),
      m_fade(0.0)
{
}

BaseStage::~BaseStage()
{
    // Not owned; just set to NULL
    m_infoPopupHeader = NULL;
    m_infoPopupMessage = NULL;

    // Not owned
    m_simpleColorProgram = 0;
}

void BaseStage::UpdateScore(const TimeSample& now)
{
    float execTime = now.ElapsedTimeSince(m_firstFrameTime);
    float fps = m_numFrames / execTime;

    // Calculate a resolution-based adjustment factor calibrated so that the
    // ipad retina resolution 2048x1536 adds 20% to the score.
    // 2048*1536 / 0.2 = 15728640
    double factor = 1.0 + ((m_viewportWidth * m_viewportHeight) / 15728640.0);

    m_stageData.m_fps = fps;
    m_stageData.m_score = (int)(fps * 50.0 * factor);
    LOG_DEBUG("BaseStage::UpdateScore(): FPS: %f, factor: %f, score: %d",
              m_stageData.m_fps, factor, m_stageData.m_score);
}

bool BaseStage::SetupInfoPopup()
{
    // Set up the info popup background texture; calculate the height
    // from the view port height
    int height = m_viewportHeight / 4;
    
    // Impose min/max limits to the height
    height = std::max(height, MinInfoHeight);
    height = std::min(height, MaxInfoHeight);
    
    // For width we'll use a small number; this will be u-wrapped across
    // the width of the whole viewport.
    const int width = 2;
    
    uint32_t* data = (uint32_t*)malloc(width * height * sizeof(uint32_t));
    if ( data == NULL ) 
    {
        LOG_DEBUG("BaseStage::SetupInfoPopup(): memory allocation failed");
        return false;
    }
    
    // We'll make the top edge of the popup partly transparent in a gradient
    // fashion
    float alphaDelta = (float)InfoPopupMaxAlpha / (height / 8.0);

    uint32_t color = 0;
    uint32_t* p = data;

    for ( int j = (height - 1); j >= 0; j-- ) 
    {
        uint32_t alpha = (uint32_t)(j * alphaDelta);
        if ( alpha > InfoPopupMaxAlpha )
        {
            alpha = InfoPopupMaxAlpha;
        }

        for ( int i = 0; i < width; i++ )
        {
            // Fill the texture with horizontal lines of varying alpha
            *p++ = color + (alpha << 24);
        }
    }
    
    // Delete the existing texture
    glDeleteTextures(1, &m_infoPopupTexture);

    // Upload the texture 
    Create2DTexture(width, height, data, &m_infoPopupTexture, true, false);
    free(data);
    
    // Calculate the font sizes for info popup header + text
    m_infoPopupHeaderScale = ((float)height * 0.30) /
                             m_textRenderer.GetFontHeight();
    m_infoPopupMessageScale = m_infoPopupHeaderScale * 0.75;
    
    // Check that the scales aren't too big for the texts not to fit
    int maxTextWidth = (int)(0.95 * m_viewportWidth);
    int messageWidth = m_textRenderer.GetTextWidth(m_infoPopupMessage,
                                                   m_infoPopupMessageScale);
    if ( messageWidth > maxTextWidth ) 
    {
        m_infoPopupMessageScale *= (float)maxTextWidth / (float)messageWidth;
        messageWidth = maxTextWidth;
    }
    int headerWidth = m_textRenderer.GetTextWidth(m_infoPopupHeader,
                                                  m_infoPopupHeaderScale);
    if ( headerWidth > maxTextWidth ) 
    {
        m_infoPopupHeaderScale *= (float)maxTextWidth / (float)headerWidth;
        headerWidth = maxTextWidth;
    }

    // Calculate text positions compared to the upper/left edges of the
    // popup background
    float headerHeight =
            m_infoPopupHeaderScale * m_textRenderer.GetFontHeight();
    float messageHeight =
            m_infoPopupMessageScale * m_textRenderer.GetFontHeight();

    m_infoPopupTop = (float)(m_viewportHeight - height);
    m_infoPopupHeight = height;
    m_infoPopupHeaderYOffset = (m_infoPopupHeight / 5) + headerHeight;
    m_infoPopupHeaderXOffset = (m_viewportWidth - headerWidth) / 2;
    m_infoPopupMessageYOffset = (m_infoPopupHeight / 8) +
                                m_infoPopupHeaderYOffset + messageHeight;
    m_infoPopupMessageXOffset = (m_viewportWidth - messageWidth) / 2;

    if ( m_infoPopupAnimation != NULL )
    {
        m_infoPopupAnimation->UpdateHeights((float)m_viewportHeight,
                                            m_infoPopupTop);
    }
    else
    {
        m_infoPopupAnimation = new InfoPopupAnimation(m_infoPopupInitialDelay,
                                                      InfoPopupTransitionDuration,
                                                      m_infoPopupDisplayTime,
                                                      (float)m_viewportHeight,
                                                      m_infoPopupTop,
                                                      &m_infoPopupTop);
    }

    return true;
}

void BaseStage::RecalculateProjection()
{
    MatrixPerspectiveProjection(m_perspectiveProjectionMatrix,
                                DefaultFov,
                                (float)(m_viewportWidth)/m_viewportHeight,
                                m_nearClip, m_farClip);
    
    MatrixOrthographicProjection(m_orthoProjectionMatrix,
                                 -m_viewportWidth/2, m_viewportWidth/2,
                                 -m_viewportHeight/2, m_viewportHeight/2,
                                 -m_viewportWidth/2, m_viewportWidth/2);
}

bool BaseStage::ViewportResized(int viewportWidth, int viewportHeight)
{
    LOG_DEBUG("BaseStage::ViewportResized(): viewport %d x %d",
              viewportWidth, viewportHeight);
    m_viewportWidth = viewportWidth;
    m_viewportHeight = viewportHeight;
    m_fullscreenRect.Set(0, 0, m_viewportWidth, m_viewportHeight);

    RecalculateProjection();
    
    // Delete existing fader buffer
    glDeleteBuffers(1, &m_fullscreenRectVertexBuffer);

    // Create fader vertex buffer
    int x = m_fullscreenRect.m_left;
    int y = m_fullscreenRect.m_top;

    // adjust x/y according to viewport size so that 0,0 is upper left
    x -= m_viewportWidth / 2;
    y = -y + (m_viewportHeight / 2) - m_fullscreenRect.GetHeight();

    VertexAttribsCoordsOnly faderVertices[] = {
        { x, y + m_fullscreenRect.GetHeight(), 0                              },
        { x, y, 0                                                             },
        { x + m_fullscreenRect.GetWidth(), y, 0                               },
        { x + m_fullscreenRect.GetWidth(), y + m_fullscreenRect.GetHeight(), 0}
    };

    glGenBuffers(1, &m_fullscreenRectVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_fullscreenRectVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(faderVertices),
                 faderVertices, GL_STATIC_DRAW);

    if ( !m_interactiveMode )
    {
        return SetupInfoPopup();
    }
    else
    {
        return true;
    }
}

void BaseStage::UpdateStageDurationFromNow(float remainingDuration)
{
    UpdateStageDuration(m_firstFrameTime.ElapsedTime() + remainingDuration);
}

void BaseStage::UpdateStageDuration(float stageDuration)
{
    m_stageDuration = stageDuration;
    if ( m_stageDuration > 0.0 )
    {
        if ( m_fadeOutAnimation != NULL )
        {
            m_fadeOutAnimation->UpdateTimings(m_stageDuration - m_fadeDuration,
                                              m_fadeDuration);
        }
        else
        {
            float elapsed = m_firstFrameTime.ElapsedTime();
            float initialDelay = stageDuration - elapsed - m_fadeDuration;
            m_fadeOutAnimation = new ScalarAnimation(0.0, 1.0, initialDelay,
                                                     m_fadeDuration, &m_fade);
        }
    }
    else
    {
        delete m_fadeOutAnimation;
        m_fadeOutAnimation = NULL;
    }
}

bool BaseStage::Setup(int viewportWidth, int viewportHeight)
{
    TimeSample setupStartTime;
    m_numFrames = 0;
    m_stageData = StageData();

    // Start by clearing any OpenGL errors set previously
    LOG_DEBUG("glGetError() at stage Setup(): 0x%x", glGetError());

    // Setup for viewport size
    if ( !ViewportResized(viewportWidth, viewportHeight) )
    {
        return false;
    }

    // Setup fadein/out animations
//    m_fade = 1.0;
    m_fadeInAnimation = new ScalarAnimation(1.0, 0.0, 0.0,
                                            m_fadeDuration, &m_fade);
    if ( m_stageDuration > 0.0 )
    {
        m_fadeOutAnimation = new ScalarAnimation(0.0, 1.0,
                                                 m_stageDuration - m_fadeDuration,
                                                 m_fadeDuration, &m_fade);
    }

    glGenBuffers(1, &m_vertexBuffer);

    // Stage specific setup
    if ( !SetupImpl() )
    {
        return false;
    }

    // Reset stage animations
    m_fadeInAnimation->ResetTime();
    
    if ( m_fadeOutAnimation != NULL )
    {
        m_fadeOutAnimation->ResetTime();
    }
    if ( m_infoPopupAnimation != NULL )
    {
        m_infoPopupAnimation->ResetTime();
    }

    // Record stage loading time
    m_stageData.m_loadTime = setupStartTime.ElapsedTime();
    LOG_DEBUG("BaseStage::Setup() done, Setup() took: %f seconds.",
              setupStartTime.ElapsedTime());

    return true;
}

void BaseStage::DrawInfoPopup()
{
    // Set up the shader program + uniforms
    m_textRenderer.SetProgram();
    m_textRenderer.SetTexture(m_infoPopupTexture);

    // Adjust x/y according to viewport size so that 0,0 is upper left
    float x = -m_viewportWidth / 2;
    float y = -m_infoPopupTop + (m_viewportHeight / 2) - m_infoPopupHeight;

    VertexAttribsTexCoords vertices[] = {
        { x, y + m_infoPopupHeight, 0,                   0.0, 1.0 },
        { x, y, 0,                                       0.0, 0.0 },
        { x + m_viewportWidth, y, 0,                     1.0, 0.0 },
        { x + m_viewportWidth, y + m_infoPopupHeight, 0, 1.0, 1.0 }
    };

    glDisable(GL_DEPTH_TEST);
    glDisableVertexAttribArray(NORMAL_INDEX);

    // Upload geometry
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rectIndexBuffer);

    glVertexAttribPointer(COORD_INDEX, 3, GL_FLOAT, GL_FALSE,
                          sizeof(VertexAttribsTexCoords),
                          (const GLvoid*)offsetof(VertexAttribsTexCoords, x));
    glVertexAttribPointer(TEXCOORD_INDEX, 2, GL_FLOAT, GL_FALSE,
                          sizeof(VertexAttribsTexCoords),
                          (const GLvoid*)offsetof(VertexAttribsTexCoords, u));

    // Draw the popup bg rect as two triangles
    glDrawElements(GL_TRIANGLES, 2*3, GL_UNSIGNED_SHORT, NULL);

    // Render the popup texts
    m_textRenderer.SetTexture();
    m_textRenderer.DrawText(m_infoPopupHeaderXOffset,
                            (int)(m_infoPopupTop + m_infoPopupHeaderYOffset),
                            m_infoPopupHeader, m_infoPopupHeaderScale);
    m_textRenderer.DrawText(m_infoPopupMessageXOffset,
                            (int)(m_infoPopupTop + m_infoPopupMessageYOffset),
                            m_infoPopupMessage, m_infoPopupMessageScale);

    glEnableVertexAttribArray(NORMAL_INDEX);
    glEnable(GL_DEPTH_TEST);
}

void BaseStage::DrawFader()
{
    DrawQuad2D(m_fullscreenRectVertexBuffer, m_rectIndexBuffer);
}

void BaseStage::Teardown()
{
    LOG_DEBUG("BaseStage::Teardown()");
    
    glDeleteBuffers(1, &m_fullscreenRectVertexBuffer);
    glDeleteBuffers(1, &m_vertexBuffer);
    glDeleteTextures(1, &m_infoPopupTexture);

    delete m_fadeInAnimation;
    m_fadeInAnimation = NULL;

    delete m_fadeOutAnimation;
    m_fadeOutAnimation = NULL;

    delete m_infoPopupAnimation;
    m_infoPopupAnimation = NULL;

    // Call stage implementation teardown
    TeardownImpl();

    // Clear the glGetError()
    glGetError();

    LOG_DEBUG("BaseStage::Teardown() done.");
}

void BaseStage::Abort()
{
    // Empty implementation
}

bool BaseStage::Render(const TimeSample& now)
{
    // Count the new frame
    if ( m_numFrames == 0 )
    {
        m_firstFrameTime.Reset();
    }
    m_numFrames++;

    // Advance the fade animations
    if ( (m_fadeInAnimation != NULL) && m_fadeInAnimation->Animate(now) )
    {
        delete m_fadeInAnimation;
        m_fadeInAnimation = NULL;
    }
    
    if ( m_fadeOutAnimation != NULL )
    {
        m_fadeOutAnimation->Animate(now);
    }

    // Render the frame
    RenderImpl(now);

    bool done = false;
    if ( m_stageDuration > 0.0 )
    {
        if ( now.ElapsedTimeSince(m_firstFrameTime) >= m_stageDuration )
        {
            done = true;
            if ( !m_interactiveMode )
            {
                UpdateScore(now);
            }
        }
    }

    // Make sure blending is on
    if ( !glIsEnabled(GL_BLEND) )
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

//    LOG_DEBUG("m_fade = %f", m_fade);
    if ( m_fade > 0.0 )
    {
        // Fade in/out going on, draw an occlusing fader
        glUseProgram(m_simpleColorProgram);
        glUniformMatrix4fv(m_simpleColorMvpLoc, 1, GL_FALSE,
                           m_orthoProjectionMatrix);
        float color[] = { 0.0, 0.0, 0.0, m_fade };
        glUniform4fv(m_simpleColorColorLoc, 1, color);
        DrawFader();
    }

    if ( !m_interactiveMode )
    {    // Draw the info popup if it is active
        m_infoPopupAnimation->Animate(now);
        if ( m_infoPopupAnimation->IsActive(now) )
        {
            DrawInfoPopup();
        }
    }

    return !done;
}

