#ifndef CHESSBOARDDEMOMODE_H
#define CHESSBOARDDEMOMODE_H

#include "ChessboardStage.h"
#include "FpsMeter.h"
#include "Rect.h"

/**
 * Interactive controls added on top of the chessboard stage.
 *
 * @author Matti Dahlbom
 * @since 1.0
 */
class ChessboardDemoMode : public ChessboardStage
{
public:
    ChessboardDemoMode(TextRenderer& textRenderer,
                       GLuint rectIndexBuffer, GLuint defaultFrameBuffer,
                       GLuint simpleColorProgram,
                       GLint simpleColorMvpLoc, GLint simpleColorColorLoc);
    virtual ~ChessboardDemoMode();

public: // Public API
    void TouchMoved(int xdiff, int ydiff);

protected: // From BaseStage
    virtual bool SetupImpl();
    virtual void RenderImpl(const TimeSample& now);

private: // Data
    // FPS measuring
    FpsMeter m_fpsMeter;

    // Rotation of camera location around the world Y axis (in radians)
    float m_cameraRotAngle;

    // Distance of camera from the world origin
    float m_cameraDistance;

    // Rotation matrix for the camera position
    float m_cameraRotationMatrix[16];
};

#endif // CHESSBOARDDEMOMODE_H
