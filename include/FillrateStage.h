#ifndef FILLRATESTAGE_H
#define FILLRATESTAGE_H

#include "OpenGLAPI.h"
#include "BaseStage.h"
#include "TimeSample.h"
#include "EllipticPathAnimation.h"

// Forward declarations

/**
 * Tests for graphics hardware fillrate.
 *
 * @author Matti Dahlbom
 * @since 0.1
 */
class FillrateStage : public BaseStage
{
public: // Construction and destruction
    FillrateStage(TextRenderer& textRenderer,
                 GLuint rectIndexBuffer, GLuint defaultFrameBuffer,
                 GLuint simpleColorProgram,
                 GLint simpleColorMvpLoc, GLint simpleColorColorLoc);
    ~FillrateStage();
    
protected: // From BaseStage
    bool SetupImpl();
    void RenderImpl(const TimeSample& now);
    void TeardownImpl();
    void UpdateScore(const TimeSample& now);
    
private:
    void RenderNormally();
    void RenderBenchmark(const TimeSample& now);
    bool SetupGeometry();
    
private: // Enums
    enum Phase {
        Unlighted,
        VertexLighted,
        PixelLighted,
        MappedLighted
    };
    
private: // Data
    // Geometry vertex/index buffers
    GLuint m_vertexBuffer;

    // Which test is going on
    Phase m_phase;
    
    // Whether a faulty driver has been detected
//    bool m_driverFaultDetected;

    // Rendering time measuring
    TimeSample* m_initialPhaseTime;
    TimeSample m_renderTime;
    
    // Light
    EllipticPathAnimation* m_lightAnim;
    float m_lightPos[3];

    // Texture
    GLuint m_texture;
    GLuint m_normalSpecTexture;
    
    // Shader programs
    GLuint m_vertexLightShader;
    GLuint m_pixelLightShader;
    GLuint m_mappedLightShader;
    
    // Uniform locations
    GLint m_vlMvpLoc;
    GLint m_vlTextureLoc;
    GLint m_vlLightPosLoc;
    GLint m_plMvpLoc;
    GLint m_plTextureLoc;
    GLint m_plLightPosLoc;
    GLint m_mlMvpLoc;
    GLint m_mlTextureLoc;
    GLint m_mlNormalSpecTextureLoc;
    GLint m_mlLightPosLoc;
};

#endif // FILLRATESTAGE_H
