#include <math.h>
#include <string.h>

#include "ChessboardStage.h"
#include "CommonFunctions.h"
#include "Chessboard.h"
#include "Chesspiece.h"
#include "ChesspieceInstance.h"
#include "ChesspieceAnimation.h"
#include "ScalarAnimation.h"
#include "MatrixOperations.h"
#include "Skybox.h"
#include "TextRenderer.h"
#include "InfoPopupAnimation.h"

// Width of one chessboard square
static const float SqrW = 1.0;

// Duration of the stage, in seconds
static const float StageDuration = 56.0;

// Info popup texts
static const char* InfoPopupHeader = "basic gpu test";
static const char* InfoPopupMessage = "multitexturing / postprocessing";

// Image size ratio for blurred : unblurred
const float BlurImageSizeRatio = 1.0 / 2;

// Near / far clip planes
static const float MyNearClip = 0.5;
static const float MyFarClip = 35.0;

// Square coordinates
static const PiecePosition A1 = { -3.5 * SqrW, 3.5 * SqrW };
static const PiecePosition A3 = { -3.5 * SqrW, 1.5 * SqrW };
static const PiecePosition D1 = { -0.5 * SqrW, 3.5 * SqrW };
static const PiecePosition D2 = { -0.5 * SqrW, 2.5 * SqrW };
static const PiecePosition E1 = { 0.5 * SqrW, 3.5 * SqrW };
static const PiecePosition F2 = { 1.5 * SqrW, 2.5 * SqrW };
static const PiecePosition G2 = { 2.5 * SqrW, 2.5 * SqrW };
static const PiecePosition E3 = { 0.5 * SqrW, 1.5 * SqrW };
static const PiecePosition D4 = { -0.5 * SqrW, 0.5 * SqrW };
static const PiecePosition H4 = { 3.5 * SqrW, 0.5 * SqrW };
static const PiecePosition A5 = { -3.5 * SqrW, -0.5 * SqrW };
static const PiecePosition E5 = { 0.5 * SqrW, -0.5 * SqrW };
static const PiecePosition B8 = { -2.5 * SqrW, -3.5 * SqrW };
static const PiecePosition H8 = { 3.5 * SqrW, -3.5 * SqrW };
static const PiecePosition A7 = { -3.5 * SqrW, -2.5 * SqrW };
static const PiecePosition C7 = { -1.5 * SqrW, -2.5 * SqrW };
static const PiecePosition F7 = { 1.5 * SqrW, -2.5 * SqrW };
static const PiecePosition G7 = { 2.5 * SqrW, -2.5 * SqrW };
static const PiecePosition A6 = { -3.5 * SqrW, -1.5 * SqrW };
static const PiecePosition B6 = { -2.5 * SqrW, -1.5 * SqrW };
static const PiecePosition E6 = { 0.5 * SqrW, -1.5 * SqrW };
static const PiecePosition H6 = { 3.5 * SqrW, -1.5 * SqrW };
static const PiecePosition D5 = { -0.5 * SqrW, -0.5 * SqrW };
static const PiecePosition D8 = { -0.5 * SqrW, -3.5 * SqrW };
static const PiecePosition E7 = { 0.5 * SqrW, -2.5 * SqrW };
static const PiecePosition F8 = { 1.5 * SqrW, -3.5 * SqrW };

// Scale matrix used to create the piece reflection by flipping the piece
// objects around the XZ plane
static const float ReflectionScaleMatrix[16] = {
    1.0, 0.0, 0.0, 0.0,
    0.0, -1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0
};

ChessboardStage::ChessboardStage(TextRenderer& textRenderer,
                                 GLuint rectIndexBuffer,
                                 GLuint defaultFrameBuffer,
                                 GLuint simpleColorProgram,
                                 GLint simpleColorMvpLoc,
                                 GLint simpleColorColorLoc)
    : BaseStage(textRenderer, rectIndexBuffer, defaultFrameBuffer,
                simpleColorProgram, simpleColorMvpLoc, simpleColorColorLoc,
                StageDuration,
                InfoPopupHeader, InfoPopupMessage, 
                MyNearClip, MyFarClip),
//      m_dofEnabled(true),
      m_chessboard(NULL),
      m_chesspawn(NULL),
      m_chessknight(NULL),
      m_chessbishop(NULL),
      m_chessrook(NULL),
      m_chessqueen(NULL),
      m_chessking(NULL),
      m_skybox(NULL),
      m_chessboardTopTexture(0),
      m_whiteMarbleTexture(0),
      m_darkMarbleTexture(0),
      m_skyboxTexture(0),
      m_blurImageWidth(-1),
      m_blurImageHeight(-1),
      m_frameBufferFull(0),
      m_depthStencilBufferFull(0),
      m_frameBufferSmall(0),
      m_unblurredImage(0),
      m_xBlurredImage(0),
      m_blurredImage(0),
      m_chessboardProgram(0),
      m_chesspieceProgram(0),
      m_chesspieceReflProgram(0),
      m_skyboxProgram(0),
      m_xBlurProgram(0),
      m_yBlurProgram(0),
      m_combineProgram(0),
      m_chessboardMvpLoc(-1),
      m_chessboardMvLoc(-1),
      m_chessboardTextureLoc(-1),
      m_chessboardEyeposLoc(-1),
      m_chessboardEnvmapLoc(-1),
      m_chessboardDofParamsLoc(-1),
      m_chesspieceMvpLoc(-1),
      m_chesspieceTextureLoc(-1),
      m_chesspieceEyeposLoc(-1),
      m_chesspieceEnvmapLoc(-1),
      m_chesspieceMvLoc(-1),
      m_chesspieceDofParamsLoc(-1),
      m_chesspieceReflMvpLoc(-1),
      m_chesspieceReflTextureLoc(-1),
      m_chesspieceReflEyeposLoc(-1),
      m_skyboxMvpLoc(-1),
      m_skyboxTextureLoc(-1),
      m_xBlurTextureLoc(-1),
      m_xBlurSampleWidthLoc(-1),
      m_yBlurTextureLoc(-1),
      m_yBlurSampleHeightLoc(-1),
      m_combineUnblurredTextureLoc(-1),
      m_combineBlurredTextureLoc(-1)
{
    memset(m_cameraTarget, 0, sizeof(m_cameraTarget));
    memset(m_cameraLocation, 0, sizeof(m_cameraLocation));
}

ChessboardStage::~ChessboardStage()
{
    LOG_DEBUG("ChessboardStage::~ChessboardStage()");
    TeardownImpl();
}

void ChessboardStage::UpdateScore(const TimeSample& now)
{
    BaseStage::UpdateScore(now);

    if ( !PackedDepthStencilExtensionPresent() )
    {
        LOG_DEBUG("Packed depth-stencil buffer support missing, reducing score");
        m_stageData.m_score /= 2;
    }
}

void ChessboardStage::RenderBoardStencil(float* vpMatrix)
{
    // Use simplest possible program
    glUseProgram(m_simpleColorProgram);
    glUniformMatrix4fv(m_simpleColorMvpLoc, 1, GL_FALSE, vpMatrix);
    const float color[] =  {0.0, 0.0, 0.0, 1.0};
    glUniform4fv(m_simpleColorColorLoc, 1, color);

    // Renders the board into the stencil buffer only for clipping the
    // piece reflections
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    glStencilFunc(GL_ALWAYS, 1, 1);

    // Draw the board with a simple & fast shader program; we're only
    // interested in filling the stencil buffer
    m_chessboard->Render();
}

void ChessboardStage::RenderBoard(float* vMatrix, float* vpMatrix)
{
    glUseProgram(m_chessboardProgram);
    glUniformMatrix4fv(m_chessboardMvpLoc, 1, GL_FALSE, vpMatrix);
    glUniformMatrix4fv(m_chessboardMvLoc, 1, GL_FALSE, vMatrix);
    glUniform3fv(m_chessboardEyeposLoc, 1, m_cameraLocation);

    // Set DoF parameters
    glUniform2fv(m_chessboardDofParamsLoc, 1, m_dofParams);

    glActiveTexture(GL_TEXTURE1);
    glUniform1i(m_chessboardEnvmapLoc, 1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_skyboxTexture);

    glActiveTexture(GL_TEXTURE0);
    glUniform1i(m_chessboardTextureLoc, 0);
    glBindTexture(GL_TEXTURE_2D, m_chessboardTopTexture);

    // Make the board slightly transparent to make it less bright
    glBlendColor(0.0, 0.0, 0.0, 0.7);

    m_chessboard->Render();
}

void ChessboardStage::RenderPieces(float* vMatrix, float* vpMatrix)
{
    glUseProgram(m_chesspieceProgram);

    // Set DoF parameters
    glUniform2fv(m_chesspieceDofParamsLoc, 1, m_dofParams);

    glActiveTexture(GL_TEXTURE1);
    glUniform1i(m_chesspieceEnvmapLoc, 1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_skyboxTexture);

    GLuint texture = 0;
    float fade = -1;

    // Use constant alpha for blending, leaving actual alpha channel
    // for the depth blur information
    glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);

    for ( unsigned int i = 0; i < m_pieces.size(); i++ )
    {
        ChesspieceInstance* piece = m_pieces[i];

        // Transform the camera position to object space
        float inverseObjectTransform[16];
        CalculateInverseTransform(piece->Transform(), inverseObjectTransform);
        float eyeObjectSpacePos[3];
        Transformv3(inverseObjectTransform, m_cameraLocation,
                    eyeObjectSpacePos);
        glUniform3fv(m_chesspieceEyeposLoc, 1, eyeObjectSpacePos);

        // Calculate & upload mvp matrix
        float mvpMatrix[16];
        MatrixMultiply(piece->Transform(), vpMatrix, mvpMatrix);
        glUniformMatrix4fv(m_chesspieceMvpLoc, 1, GL_FALSE, mvpMatrix);

        // Calculate & upload mv matrix for blur
        float mvMatrix[16];
        MatrixMultiply(piece->Transform(), vMatrix, mvMatrix);
        glUniformMatrix4fv(m_chesspieceMvLoc, 1, GL_FALSE, mvMatrix);

        // Set fade value
        if ( piece->Fade() != fade )
        {
            fade = piece->Fade();
            glBlendColor(0.0, 0.0, 0.0, fade);
        }

        // Only set main texture when needed
        if ( texture != piece->Texture() )
        {
            glActiveTexture(GL_TEXTURE0);
            glUniform1i(m_chesspieceTextureLoc, 0);
            glBindTexture(GL_TEXTURE_2D, piece->Texture());
            texture = piece->Texture();
        }

        piece->Object().Render();
    }
}

void ChessboardStage::RenderReflectedPieces(float* vpMatrix)
{
    glUseProgram(m_chesspieceReflProgram);
    glCullFace(GL_FRONT);

    // Set stencil params so that we only draw where stencil buffer value = 1
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilFunc(GL_EQUAL, 1, 1);

    GLuint texture = 0;
    float fade = -1;

    for ( unsigned int i = 0; i < m_pieces.size(); i++ )
    {
        ChesspieceInstance* piece = m_pieces[i];

        // Transform the camera position to object space
        float eyePosObjectSpace[3];
        Transformv3(piece->Transform(), m_cameraLocation, eyePosObjectSpace);
        glUniform3fv(m_chesspieceReflEyeposLoc, 1, eyePosObjectSpace);

        // Calculate mvp matrix
        float mvpMatrix[16];
        float objectMatrix[16];
        MatrixMultiply(piece->Transform(), ReflectionScaleMatrix, objectMatrix);
        MatrixMultiply(objectMatrix, vpMatrix, mvpMatrix);
        glUniformMatrix4fv(m_chesspieceReflMvpLoc, 1, GL_FALSE, mvpMatrix);

        // Set fade value
        if ( piece->Fade() != fade )
        {
            fade = piece->Fade();
            glBlendColor(0.0, 0.0, 0.0, fade);
        }

        // Only set main texture when needed
        if ( texture != piece->Texture() )
        {
            glActiveTexture(GL_TEXTURE0);
            glUniform1i(m_chesspieceReflTextureLoc, 0);
            glBindTexture(GL_TEXTURE_2D, piece->Texture());
            texture = piece->Texture();
        }

        piece->Object().Render();
    }

    glCullFace(GL_BACK);

    glStencilFunc(GL_ALWAYS, 0, 0);
}

void ChessboardStage::RenderSkybox(float* viewMatrix)
{
    // Extract rotation from the view matrix and calculate (m)vp matrix
    float rotationMatrix[16];
    MatrixExtractRotation(viewMatrix, rotationMatrix);
    float skyboxMvpMatrix[16];
    MatrixMultiply(rotationMatrix, m_perspectiveProjectionMatrix,
                   skyboxMvpMatrix);

    glUseProgram(m_skyboxProgram);
    glUniformMatrix4fv(m_skyboxMvpLoc, 1, GL_FALSE, skyboxMvpMatrix);
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(m_skyboxTextureLoc, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_skyboxTexture);

    m_skybox->Render();
}

void ChessboardStage::Animate(const TimeSample& time)
{
    for (std::list<BaseAnimation*>::iterator iter = m_animations.begin();
        iter != m_animations.end(); iter++)
    {
        if ( (*iter)->Animate(time) )
        {
            // Animation completed; remove it from the list
            delete (*iter);
            iter = m_animations.erase(iter);
        }
    }

    // Remove any pieces that have been faded out
    for (std::vector<ChesspieceInstance*>::iterator iter = m_pieces.begin();
        iter != m_pieces.end(); iter++)
    {
        ChesspieceInstance* piece = *iter;
        if ( piece->Fade() < 0.01 )
        {
            iter = m_pieces.erase(iter);
            delete piece;
        }
    }
}

bool ChessboardStage::SwitchRenderTexture(GLuint texture)
{
    // NOTE - the proper frame buffer object must be bound when calling this

    // Set the selected texture as the color attachment
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, texture, 0);

    // Check FBO status
    GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if ( fboStatus != GL_FRAMEBUFFER_COMPLETE )
    {
        LOG_DEBUG("ChessboardStage::SwitchRenderTexture(): " \
                  "FBO not complete: 0x%x", fboStatus);
        return false;
    }

    return true;
}

void ChessboardStage::RenderXBlur()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(m_xBlurProgram);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_unblurredImage);
    glUniform1i(m_xBlurTextureLoc, 0);
    const float sampleWidth = 1.0 / m_blurImageWidth;
    glUniform1f(m_xBlurSampleWidthLoc, sampleWidth);
    glBindBuffer(GL_ARRAY_BUFFER, g_rectangleCoordsVertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_rectangleIndexBuffer);
    glVertexAttribPointer(COORD_INDEX, 3, GL_FLOAT, GL_FALSE,
                          sizeof(VertexAttribsCoordsOnly),
                          (const GLvoid*)offsetof(VertexAttribsCoordsOnly, x));
    glDrawElements(GL_TRIANGLES, 2*3, GL_UNSIGNED_SHORT, NULL);
}

void ChessboardStage::RenderYBlur()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(m_yBlurProgram);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_xBlurredImage);
    glUniform1i(m_yBlurTextureLoc, 0);
    const float sampleHeight = 1.0 / m_blurImageHeight;
    glUniform1f(m_yBlurSampleHeightLoc, sampleHeight);
    glBindBuffer(GL_ARRAY_BUFFER, g_rectangleCoordsVertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_rectangleIndexBuffer);
    glVertexAttribPointer(COORD_INDEX, 3, GL_FLOAT, GL_FALSE,
                          sizeof(VertexAttribsCoordsOnly),
                          (const GLvoid*)offsetof(VertexAttribsCoordsOnly, x));
    glDrawElements(GL_TRIANGLES, 2*3, GL_UNSIGNED_SHORT, NULL);
}

void ChessboardStage::RenderCombinedBlur()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(m_combineProgram);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_unblurredImage);
    glUniform1i(m_combineUnblurredTextureLoc, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_blurredImage);
    glUniform1i(m_combineBlurredTextureLoc, 1);
    glBindBuffer(GL_ARRAY_BUFFER, g_rectangleCoordsVertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_rectangleIndexBuffer);
    glVertexAttribPointer(COORD_INDEX, 3, GL_FLOAT, GL_FALSE,
                          sizeof(VertexAttribsCoordsOnly),
                          (const GLvoid*)offsetof(VertexAttribsCoordsOnly, x));
    glDrawElements(GL_TRIANGLES, 2*3, GL_UNSIGNED_SHORT, NULL);
}

void ChessboardStage::RenderImpl(const TimeSample& time)
{
    // Animate everything; pieces & camera
    Animate(time);

    // Create a look-at matrix
    float lookat[16];
    MatrixSetLookat(lookat, m_cameraLocation, m_cameraTarget);

    // Setup projection
    float vpMatrix[16];
    MatrixMultiply(lookat, m_perspectiveProjectionMatrix, vpMatrix);

    // Start rendering to the fullscreen offscreen frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferFull);

    // Clear all the buffers in the unblurred image
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

//    LOG_DEBUG("glGetError() = 0x%x", glGetError());

    // Render the board stencil
    // Disable writing into the color buffer; we're only generating stencil
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glEnable(GL_STENCIL_TEST);
    glDisable(GL_DEPTH_TEST);
    RenderBoardStencil(vpMatrix);
    glEnable(GL_DEPTH_TEST);

    // Use constant alpha for blending, leaving actual alpha channel
    // for the depth blur information
    glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
    glEnable(GL_BLEND);

    // Re-enable writing colors; still block writing alpha
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);

    // Draw the pieces inverted (reflection)
    RenderReflectedPieces(vpMatrix);
    glDisable(GL_STENCIL_TEST);

    // Re-enable writing all components
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    // Draw the chessboard (will be blended with the reflection)
    RenderBoard(lookat, vpMatrix);

    // Draw the pieces
    RenderPieces(lookat, vpMatrix);

    glDisable(GL_BLEND);

    // Draw the skybox
    RenderSkybox(lookat);

    // Start rendering to the small-size offscreen frame buffer
    glViewport(0, 0, m_blurImageWidth, m_blurImageHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferSmall);

    // Apply horizontal Gaussian blur
    SwitchRenderTexture(m_xBlurredImage);
    glDisable(GL_DEPTH_TEST);
    RenderXBlur();

    // Apply vertical Gaussian blur
    SwitchRenderTexture(m_blurredImage);
    RenderYBlur();

    // Now render onscreen, combining the blurred + unblurred images
    glViewport(0, 0, m_viewportWidth, m_viewportHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFrameBuffer);
    RenderCombinedBlur();
}

ChesspieceInstance* ChessboardStage::AddWhitePiece(Chesspiece* piece,
                                                   const PiecePosition& pos)
{
    ChesspieceInstance* instance =
            new ChesspieceInstance(*piece, m_whiteMarbleTexture, pos);
    m_pieces.push_back(instance);
    return instance;
}

ChesspieceInstance* ChessboardStage::AddBlackPiece(Chesspiece* piece,
                                                   const PiecePosition& pos)
{
    ChesspieceInstance* instance =
            new ChesspieceInstance(*piece, m_darkMarbleTexture, pos);
    m_pieces.push_back(instance);
    return instance;
}

void ChessboardStage::AddTransAnim(const float* initialPtr,
                                   float destinationX, float destinationY,
                                   float destinationZ, float initialDelay,
                                   float duration, float* location)
{
    m_animations.push_back(new TranslationAnimation(initialPtr, destinationX,
                                                    destinationY, destinationZ,
                                                    initialDelay, duration,
                                                    location));
}

void ChessboardStage::SetupPieces()
{
    // White pieces
    AddWhitePiece(m_chessrook, A1);
    AddWhitePiece(m_chessbishop, D1);
    AddWhitePiece(m_chessking, E1);
    AddWhitePiece(m_chessknight, D2);
    AddWhitePiece(m_chesspawn, F2);
    AddWhitePiece(m_chesspawn, G2);
    ChesspieceInstance* wQueen = AddWhitePiece(m_chessqueen, E3);
    AddWhitePiece(m_chesspawn, D4);
    ChesspieceInstance* wBishop1 = AddWhitePiece(m_chessbishop, H4);
    ChesspieceInstance* wPawn1 = AddWhitePiece(m_chesspawn, A5);
    AddWhitePiece(m_chesspawn, E5);

    // Black pieces
    AddBlackPiece(m_chessking, B8);
    ChesspieceInstance* bRook1 = AddBlackPiece(m_chessrook, D8);
    ChesspieceInstance* bRook2 = AddBlackPiece(m_chessrook, H8);
    AddBlackPiece(m_chesspawn, A7);
    ChesspieceInstance* bQueen = AddBlackPiece(m_chessqueen, C7);
    AddBlackPiece(m_chesspawn, F7);
    ChesspieceInstance* bBishop1 = AddBlackPiece(m_chessbishop, G7);
    ChesspieceInstance* bBishop2 = AddBlackPiece(m_chessbishop, A6);
    ChesspieceInstance* bPawn1 = AddBlackPiece(m_chesspawn, B6);
    AddBlackPiece(m_chesspawn, E6);
    AddBlackPiece(m_chesspawn, H6);

    // NOTE the game played is Kasparov - Timman, 1991, Amsterdam
    // Moves 24.-> http://www.365chess.com/view_game.php?g=2051441
    // Game ended in agreed draw at move 29.

    // Defines a starting offset for piece animations
    const float SOFF = 24.0;

    // Attach animations to the pieces
    m_animations.push_front(new ChesspieceAnimation(D8, D5, SOFF+0.0, *bRook1));
    m_animations.push_front(new ChesspieceAnimation(A5, B6, SOFF+3.0, *wPawn1));
    m_animations.push_front(new ScalarAnimation(1.0, 0.0, SOFF+3.0, 1.0,
                                                bPawn1->FadePtr()));
    m_animations.push_front(new ChesspieceAnimation(C7, B6, SOFF+5.0, *bQueen));
    m_animations.push_front(new ScalarAnimation(1.0, 0.0, SOFF+5.0, 0.8,
                                                wPawn1->FadePtr()));
    m_animations.push_front(new ChesspieceAnimation(H4, E7, SOFF+7.0, *wBishop1));
    m_animations.push_front(new ChesspieceAnimation(G7, F8, SOFF+11.0, *bBishop1));
    m_animations.push_front(new ChesspieceAnimation(E7, F8, SOFF+13.0, *wBishop1));
    m_animations.push_front(new ScalarAnimation(1.0, 0.0, SOFF+13.0, 1.0,
                                                bBishop1->FadePtr()));
    m_animations.push_front(new ChesspieceAnimation(H8, F8, SOFF+15.0, *bRook2));
    m_animations.push_front(new ScalarAnimation(1.0, 0.0, SOFF+15.0, 1.0,
                                                wBishop1->FadePtr()));
    m_animations.push_front(new ChesspieceAnimation(E3, A3, SOFF+17.0, *wQueen));
    m_animations.push_front(new ChesspieceAnimation(F8, D8, SOFF+20.0, *bRook2));
    m_animations.push_front(new ChesspieceAnimation(A3, A6, SOFF+22.0, *wQueen));
    m_animations.push_front(new ScalarAnimation(1.0, 0.0, SOFF+22.0, 1.0,
                                                bBishop2->FadePtr()));

    // Add camera position pathing animations
    m_animations.push_front(new TranslationAnimation(-10, 5, 3.0,
                                                     -5, 2, 3, 0, 7,
                                                     m_cameraLocation));
    AddTransAnim(m_cameraLocation, -2.5, 1, 3, 7, 5, m_cameraLocation);
    AddTransAnim(m_cameraLocation, -1.5, 0.6, -1.0, 12, 6, m_cameraLocation);
    AddTransAnim(m_cameraLocation, 3.0, 2.0, -7.5, 18, 6, m_cameraLocation);
    AddTransAnim(m_cameraLocation, 3.0, 4.0, -6.0, 24, 4, m_cameraLocation);
    AddTransAnim(m_cameraLocation, 6.0, 3.3, -1.0, 28, 9, m_cameraLocation);
    AddTransAnim(m_cameraLocation, 2.0, 2.5, 3.5, 37.0, 6, m_cameraLocation);
    AddTransAnim(m_cameraLocation, -11.0, 2, -1, 43, 14, m_cameraLocation);

    // Add camera target pathing animations
    m_animations.push_front(new TranslationAnimation(0.0, 0.0, 0.0,
                                                     -1.0, 0.0, -1.0,
                                                     7.0, 2.0,
                                                     m_cameraTarget));
    AddTransAnim(m_cameraTarget, 0.0, 0.5, 0.0, 15, 3, m_cameraTarget);
    AddTransAnim(m_cameraTarget, -3, 0.5, -1.5, 40, 3, m_cameraTarget);
    AddTransAnim(m_cameraTarget, -3, 0.5, -3, 43, 2, m_cameraTarget);
    AddTransAnim(m_cameraTarget, 0, 0.0, 0, 49, 4, m_cameraTarget);
}

bool ChessboardStage::CreateTexture(int width, int height, GLuint* texture)
{
    // Create the texture
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Clamping is needed for these textures are not power-of-2
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Allocate texture size
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    return true;
}

bool ChessboardStage::CreateFullscreenBuffer()
{
    // Create a framebuffer object
    glGenFramebuffers(1, &m_frameBufferFull);
    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferFull);

    GLenum internalFormat = GL_DEPTH24_STENCIL8;
    if ( !PackedDepthStencilExtensionPresent() )
    {
        LOG_DEBUG("Missing GL_DEPTH24_STENCIL8, falling back " \
                  "to GL_DEPTH_COMPONENT16!");
        internalFormat = GL_DEPTH_COMPONENT16;
    }

    // Create a depth+stencil buffer
    glGenRenderbuffers(1, &m_depthStencilBufferFull);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthStencilBufferFull);
    glRenderbufferStorage(GL_RENDERBUFFER, internalFormat,
                          m_viewportWidth, m_viewportHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER, m_depthStencilBufferFull);
    if ( internalFormat == GL_DEPTH24_STENCIL8 )
    {
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                                  GL_RENDERBUFFER, m_depthStencilBufferFull);
    }

    int error = glGetError();
    if ( error != GL_NO_ERROR )
    {
        LOG_DEBUG("ChessboardStage::CreateBuffers(): error: 0x%x", error);
    }

    return (error == GL_NO_ERROR);
}

bool ChessboardStage::SetupOffscreenRendering()
{
    // Image size for rendering the blurred images
    m_blurImageWidth = m_viewportWidth * BlurImageSizeRatio;
    m_blurImageHeight = m_viewportHeight * BlurImageSizeRatio;

    // Create render-to textures for each processing stage
    if ( !CreateTexture(m_viewportWidth, m_viewportHeight, &m_unblurredImage) ||
         !CreateTexture(m_blurImageWidth, m_blurImageHeight,
                        &m_xBlurredImage) ||
         !CreateTexture(m_blurImageWidth, m_blurImageHeight,
                        &m_blurredImage) )
    {
        LOG_INFO("ChessboardStage::SetupOffscreenRendering(): " \
                  "CreateTexture() failed");
        return false;
    }

    // Create frame/rendering buffers for fullscreen rendering
    if ( !CreateFullscreenBuffer() )
    {
        LOG_INFO("Setting up frame/rendering buffers failed!");
        return false;
    }

    // Attach unblurred image texture as the color for the fullscreen buffer
    if ( !SwitchRenderTexture(m_unblurredImage) )
    {
        LOG_INFO("ChessboardStage::SetupOffscreenRendering(): " \
                  "fullscreen FBO setup failed!");
        return false;
    }

    // Create frame buffer for small-size blur rendering. We dont need
    // depth nor stencil buffers.
    glGenFramebuffers(1, &m_frameBufferSmall);

    return true;
}

bool ChessboardStage::SetupImpl()
{
    LOG_DEBUG("ChessboardStage::Setup()");

    // Clear any existing GL errors
    glGetError();
    
    if ( !SetupOffscreenRendering() )
    {
        LOG_INFO("ChessboardStage::SetupImpl(): " \
                  "SetupOffscreenRendering() failed!");
        return false;
    }

    if ( !Load2DTextureFromBundle("chessboard.jpg", &m_chessboardTopTexture,
                                  true, false) )
    {
        return false;
    }

    if ( !Load2DTextureFromBundle("white_marble.jpg", &m_whiteMarbleTexture,
                                false, true) )
    {
        return false;
    }

    if ( !Load2DTextureFromBundle("dark_marble.jpg", &m_darkMarbleTexture,
                                false, true) )
    {
        return false;
    }

    if ( !LoadCubeTextureFromBundle("space_skybox_left.jpg",
                                    "space_skybox_right.jpg",
                                    "space_skybox_top.jpg",
                                    "space_skybox_bottom.jpg",
                                    "space_skybox_back.jpg",
                                    "space_skybox_front.jpg",
                                    &m_skyboxTexture) )
    {
        return false;
    }

    if ( !LoadShaderFromBundle("Chessboard", &m_chessboardProgram) )
    {
        return false;
    }

    if ( !LoadShaderFromBundle("Chesspiece", &m_chesspieceProgram) )
    {
        return false;
    }

    if ( !LoadShaderFromBundle("ChesspieceReflection",
                               &m_chesspieceReflProgram) )
    {
        return false;
    }

    if ( !LoadShaderFromBundle("Skybox", &m_skyboxProgram) )
    {
        return false;
    }

    if ( !LoadShaderFromBundle("YBlur", &m_yBlurProgram) )
    {
        return false;
    }

    if ( !LoadShaderFromBundle("XBlur", &m_xBlurProgram) )
    {
        return false;
    }

    if ( !LoadShaderFromBundle("CombineBlur", &m_combineProgram) )
    {
        return false;
    }

    // Get GLSL uniform locations
    m_chessboardTextureLoc = glGetUniformLocation(m_chessboardProgram,
                                                     "texture");
    m_chessboardMvpLoc = glGetUniformLocation(m_chessboardProgram,
                                                 "mvp_matrix");
    m_chessboardMvLoc = glGetUniformLocation(m_chessboardProgram,
                                                 "mv_matrix");
    m_chessboardEyeposLoc = glGetUniformLocation(m_chessboardProgram,
                                                    "eye_pos");
    m_chessboardEnvmapLoc = glGetUniformLocation(m_chessboardProgram,
                                                    "env_cube_map");
    m_chessboardDofParamsLoc = glGetUniformLocation(m_chessboardProgram,
                                                    "dof_params");

    m_chesspieceMvpLoc = glGetUniformLocation(m_chesspieceProgram,
                                                 "mvp_matrix");
    m_chesspieceMvLoc = glGetUniformLocation(m_chesspieceProgram, "mv_matrix");
    m_chesspieceTextureLoc = glGetUniformLocation(m_chesspieceProgram,
                                                  "texture");
    m_chesspieceEyeposLoc = glGetUniformLocation(m_chesspieceProgram,
                                                 "eye_pos");
    m_chesspieceEnvmapLoc = glGetUniformLocation(m_chesspieceProgram,
                                                 "env_cube_map");
    m_chesspieceDofParamsLoc = glGetUniformLocation(m_chesspieceProgram,
                                                    "dof_params");

    m_chesspieceReflTextureLoc = glGetUniformLocation(m_chesspieceReflProgram,
                                                     "texture");
    m_chesspieceReflMvpLoc = glGetUniformLocation(m_chesspieceReflProgram,
                                                 "mvp_matrix");
    m_chesspieceReflEyeposLoc = glGetUniformLocation(m_chesspieceReflProgram,
                                                 "eye_pos");
    m_chesspieceReflFadeLoc = glGetUniformLocation(m_chesspieceReflProgram,
                                                 "fade");

    m_skyboxTextureLoc = glGetUniformLocation(m_skyboxProgram, "skybox");
    m_skyboxMvpLoc = glGetUniformLocation(m_skyboxProgram, "mvp_matrix");

    m_xBlurTextureLoc = glGetUniformLocation(m_xBlurProgram, "texture");
    m_xBlurSampleWidthLoc =
            glGetUniformLocation(m_xBlurProgram, "sample_width");

    m_yBlurTextureLoc = glGetUniformLocation(m_yBlurProgram, "texture");
    m_yBlurSampleHeightLoc =
            glGetUniformLocation(m_yBlurProgram, "sample_height");

    m_combineUnblurredTextureLoc = glGetUniformLocation(m_combineProgram,
                                                        "unblurred_texture");
    m_combineBlurredTextureLoc = glGetUniformLocation(m_combineProgram,
                                                      "blurred_texture");

    m_chessboard = Chessboard::Create();
    m_chesspawn = Chesspiece::Create(Chesspiece::Pawn);
    m_chessrook = Chesspiece::Create(Chesspiece::Rook);
    m_chessknight = Chesspiece::Create(Chesspiece::Knight);
    m_chessbishop = Chesspiece::Create(Chesspiece::Bishop);
    m_chessqueen = Chesspiece::Create(Chesspiece::Queen);
    m_chessking = Chesspiece::Create(Chesspiece::King);
    m_skybox = Skybox::Create(20);

    if ( (m_chessboard == NULL) || (m_chesspawn == NULL) ||
         (m_chessrook == NULL) || (m_chessknight == NULL) ||
         (m_chessbishop == NULL) || (m_chessqueen == NULL) ||
         (m_chessking == NULL) || (m_skybox == NULL) )
    {
        LOG_INFO("Object initialization failed!");
        return false;
    }

    // Setup the chess pieces
    SetupPieces();

    // Setup DoF params
    m_dofParams[0] = 4.0;
    m_dofParams[1] = 4.0;

    int glError = glGetError();
    if ( glError != GL_NO_ERROR )
    {
        LOG_INFO("glError in setup: 0x%x", glError);
        return false;
    }
    
    return true;
}

void ChessboardStage::TeardownImpl()
{
    LOG_DEBUG("ChessboardStage::Teardown()");

    glDeleteTextures(1, &m_chessboardTopTexture);
    glDeleteTextures(1, &m_whiteMarbleTexture);
    glDeleteTextures(1, &m_darkMarbleTexture);
    glDeleteTextures(1, &m_skyboxTexture);
    glDeleteTextures(1, &m_unblurredImage);
    glDeleteTextures(1, &m_xBlurredImage);
    glDeleteTextures(1, &m_blurredImage);

    glDeleteRenderbuffers(1, &m_depthStencilBufferFull);
    glDeleteFramebuffers(1, &m_frameBufferFull);
    glDeleteFramebuffers(1, &m_frameBufferSmall);

    UnloadShader(m_chessboardProgram);
    UnloadShader(m_chesspieceProgram);
    UnloadShader(m_chesspieceReflProgram);
    UnloadShader(m_skyboxProgram);
    UnloadShader(m_xBlurProgram);
    UnloadShader(m_yBlurProgram);
    UnloadShader(m_combineProgram);

    for ( size_t i = 0; i < m_pieces.size(); i++ )
    {
        delete m_pieces[i];
    }
    m_pieces.clear();

    while ( !m_animations.empty() )
    {
        delete m_animations.front();
        m_animations.pop_front();
    }

    delete m_chessboard;
    m_chessboard = NULL;

    delete m_chesspawn;
    m_chesspawn = NULL;

    delete m_chessknight;
    m_chessknight = NULL;

    delete m_chessbishop;
    m_chessbishop = NULL;

    delete m_chessrook;
    m_chessrook = NULL;

    delete m_chessqueen;
    m_chessqueen = NULL;

    delete m_chessking;
    m_chessking = NULL;

    delete m_skybox;
    m_skybox = NULL;
}

