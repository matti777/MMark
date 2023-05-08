#ifndef CHESSBOARDSTAGE_H
#define CHESSBOARDSTAGE_H

#include <vector>
#include <list>
#include "BaseStage.h"
#include "ChesspieceInstance.h"

// Forward declarations
class Chessboard;
class Chesspiece;
class Skybox;
class BaseAnimation;

/**
 * Multi-texturing stage, displaying a chessboard.
 *
 * @author Matti Dahlbom
 * @since 0.1
 */
class ChessboardStage : public BaseStage
{
public:
    ChessboardStage(TextRenderer& textRenderer,
                    GLuint rectIndexBuffer, GLuint defaultFrameBuffer,
                    GLuint simpleColorProgram,
                    GLint simpleColorMvpLoc, GLint simpleColorColorLoc);
    virtual ~ChessboardStage();

protected: // From BaseStage
    virtual bool SetupImpl();
    virtual void RenderImpl(const TimeSample& time);
    virtual void TeardownImpl();
    void UpdateScore(const TimeSample& now);

private:
    void Animate(const TimeSample& time);
    ChesspieceInstance* AddWhitePiece(Chesspiece* piece,
                                      const PiecePosition& initialPosition);
    ChesspieceInstance* AddBlackPiece(Chesspiece* piece,
                                      const PiecePosition& initialPosition);
    void AddTransAnim(const float* initialPtr,
                      float destinationX, float destinationY,
                      float destinationZ, float initialDelay,
                      float duration, float* location);
    bool CreateTexture(int width, int height, GLuint* texture);
    bool CreateFullscreenBuffer();
    bool SetupOffscreenRendering();
    void SetupPieces();
    void RenderBoardStencil(float* vpMatrix);
    void RenderBoard(float* vMatrix, float* vpMatrix);
    void RenderPieces(float* vMatrix, float* vpMatrix);
    void RenderReflectedPieces(float* vpMatrix);
    void RenderSkybox(float* viewMatrix);
    bool SwitchRenderTexture(GLuint texture);
    void RenderXBlur();
    void RenderYBlur();
    void RenderCombinedBlur();

protected: // Data
    // Whether depth-of-field is on or not; true by default
//    bool m_dofEnabled;

    // Objects
    Chessboard* m_chessboard;
    Chesspiece* m_chesspawn;
    Chesspiece* m_chessknight;
    Chesspiece* m_chessbishop;
    Chesspiece* m_chessrook;
    Chesspiece* m_chessqueen;
    Chesspiece* m_chessking;
    Skybox* m_skybox;

    // Object instances
    std::vector<ChesspieceInstance*> m_pieces;

    // List of active animations
    std::list<BaseAnimation*> m_animations;

    // Camera location (in world coordinates)
    float m_cameraLocation[3];
    float m_cameraTarget[3];

    // Textures
    GLuint m_chessboardTopTexture;
    GLuint m_whiteMarbleTexture;
    GLuint m_darkMarbleTexture;
    GLuint m_skyboxTexture;

    // Depth-of-field
    float m_dofParams[2]; // x = focal distance, y = focal range
    int m_blurImageWidth;
    int m_blurImageHeight;
    GLuint m_frameBufferFull;
    GLuint m_depthStencilBufferFull;
    GLuint m_frameBufferSmall;
    GLuint m_unblurredImage;
    GLuint m_xBlurredImage;
    GLuint m_blurredImage;

    // Shader programs
    GLuint m_chessboardProgram;
    GLuint m_chesspieceProgram;
    GLuint m_chesspieceReflProgram;
    GLuint m_skyboxProgram;
    GLuint m_xBlurProgram;
    GLuint m_yBlurProgram;
    GLuint m_combineProgram;

    // GLSL unifroms
    GLint m_chessboardMvpLoc;
    GLint m_chessboardMvLoc;
    GLint m_chessboardTextureLoc;
    GLint m_chessboardEyeposLoc;
    GLint m_chessboardEnvmapLoc;
    GLint m_chessboardDofParamsLoc;
    GLint m_chesspieceMvpLoc;
    GLint m_chesspieceTextureLoc;
    GLint m_chesspieceEyeposLoc;
    GLint m_chesspieceEnvmapLoc;
    GLint m_chesspieceMvLoc;
    GLint m_chesspieceDofParamsLoc;
    GLint m_chesspieceReflMvpLoc;
    GLint m_chesspieceReflTextureLoc;
    GLint m_chesspieceReflEyeposLoc;
    GLint m_chesspieceReflFadeLoc;
    GLint m_skyboxMvpLoc;
    GLint m_skyboxTextureLoc;
    GLint m_xBlurTextureLoc;
    GLint m_xBlurSampleWidthLoc;
    GLint m_yBlurTextureLoc;
    GLint m_yBlurSampleHeightLoc;
    GLint m_combineUnblurredTextureLoc;
    GLint m_combineBlurredTextureLoc;
};

#endif // CHESSBOARDSTAGE_H
