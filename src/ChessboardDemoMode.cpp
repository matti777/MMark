#include "ChessboardDemoMode.h"
#include "CommonFunctions.h"
#include "MatrixOperations.h"
#include "TextRenderer.h"



ChessboardDemoMode::ChessboardDemoMode(TextRenderer& textRenderer,
                                       GLuint rectIndexBuffer,
                                       GLuint defaultFrameBuffer,
                                       GLuint simpleColorProgram,
                                       GLint simpleColorMvpLoc,
                                       GLint simpleColorColorLoc)
    : ChessboardStage(textRenderer, rectIndexBuffer, defaultFrameBuffer,
                      simpleColorProgram, simpleColorMvpLoc,
                      simpleColorColorLoc),
      m_cameraRotAngle(DEG_TO_RAD(70)),
      m_cameraDistance(9.0)
{
    m_interactiveMode = true;

    // Signal the base class that we're deciding stage 'duration' later
    m_stageDuration = -1;
    
    // Override the fade in/out duration by a faster value
    m_fadeDuration = 0.4;
}

ChessboardDemoMode::~ChessboardDemoMode()
{
}

bool ChessboardDemoMode::SetupImpl()
{
    bool ok = ChessboardStage::SetupImpl();
    if ( ok )
    {
        MatrixCreateRotation(m_cameraRotationMatrix, m_cameraRotAngle, 0, 1, 0);

        // Clear animations; not used in interactive mode
        m_animations.clear();
    }
    return ok;
}

void ChessboardDemoMode::RenderImpl(const TimeSample& now)
{
    // FPS measuring
    m_fpsMeter.EndFrame();
    m_fpsMeter.StartFrame();

    // Update the camera position
    const float camPos[] = { 0, 2.0, m_cameraDistance };
    Transformv3(m_cameraRotationMatrix, camPos, m_cameraLocation);

    // Make parent class render the scene
    ChessboardStage::RenderImpl(now);

    LOG_GL_ERROR();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_textRenderer.SetProgram();
    m_textRenderer.SetTexture();

    // Draw short usage instruction
    float scale = (m_viewportHeight * 0.05) /
            m_textRenderer.GetFontHeight();
    int y = (m_textRenderer.GetFontHeight() * scale * 1.5);
    const char* text = "swipe screen to move around";
    int w = m_textRenderer.GetTextWidth(text, scale);
    int x = (m_viewportWidth - w) / 2;
    m_textRenderer.DrawText(x, y, text, scale);

    // Draw FPS
    char fpsText[32];
    sprintf(fpsText, "fps: %.1f", m_fpsMeter.GetFps());
    m_textRenderer.DrawText(20, m_viewportHeight - 20, fpsText, scale);

    LOG_GL_ERROR();
}

void ChessboardDemoMode::TouchMoved(int xdiff, int ydiff)
{
    // Check that we're not already exiting the demo mode
    if ( m_fadeOutAnimation == NULL )
    {
        m_cameraRotAngle += xdiff / 340.0;
        m_cameraDistance -= ydiff / 100.0;

        m_cameraDistance = std::max(2.0f, m_cameraDistance);
        m_cameraDistance = std::min(12.0f, m_cameraDistance);

        // Update the rotation matrix
        MatrixCreateRotation(m_cameraRotationMatrix, m_cameraRotAngle, 0, 1, 0);
    }
}
