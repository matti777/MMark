#ifndef BASESTAGE_H
#define BASESTAGE_H

#include <string>

#include "OpenGLAPI.h"
#include "TimeSample.h"
#include "Rect.h"

// forward declarations
class TextRenderer;
class InfoPopupAnimation;
class ScalarAnimation;

// Default near / far clip planes
static const float DefaultStageNearClip = 0.5;
static const float DefaultStageFarClip = 250.0;

/** Stage performance data. */
struct StageData
{
    float m_fps;
    int m_score;
    int m_cpuScore;
    int m_fillRateScore;
    int m_numImages;

    // Stage loading time in seconds
    float m_loadTime;

    // Fill rates are in Mpix (million pixels) / second
    float m_unlightedFillRate;
    float m_vertexLightedFillRate;
    float m_pixelLightedFillRate;
    float m_mappedLightedFillRate;

    std::string m_missingFeatures;
};

// Default values for infopopup timings (in seconds)
static const float InfoPopupInitialDelayDefault = 3.0;
static const float InfoPopupDisplayTimeDefault = 3.5;

/**
 * Base class for MMark stages (ie. tests). A stage can also represent a
 * transitional non-test.
 *
 * @author Matti Dahlbom
 * @since 0.1
 */
class BaseStage
{
public: // Constructors and destructor
    virtual ~BaseStage();

public: // Public methods
    /**
     * Sets up this stage; loads textures, model data etc. into OpenGL
     * memory. This will call SetupImpl() to allow for stage setup.
     *
     * @return true if initialized ok, false if failed and cannot continue.
     */
    bool Setup(int viewportWidth, int viewportHeight);

    /** The viewport was resized. */
    virtual bool ViewportResized(int viewportWidth, int viewportHeight);

    /**
     * Tears down this stage; releases all allocated memory and OpenGL resources
     * etc. This will call TeardownImpl();
     */
    void Teardown();
    
    /** 
     * Aborts any running calculations; can be called to dismiss the stage
     * withouth it completing. Base implementation does nothing.
     */
    virtual void Abort();

    /**
     * Render a single OpenGL frame.
     *
     * @param time current time
     * @return true if this Stage still has processing left to do (and hence
     * Render() should be called again), false when all done. This will call
     * RenderImpl().
     */
    bool Render(const TimeSample& time);

    /**
     * Returns the stage data for this stage.
     */
    StageData GetStageData() const { return m_stageData; };

    /** Sets a new duration for the stage. Fade-out time will be adjusted. */
    void UpdateStageDurationFromNow(float remainingDuration);

protected: // Constructors
    /**
     * Constructs the base class.
     *
     * @param textRenderer
     * @param rectIndexBuffer
     * @param stageDuration Duration of the stage in seconds
     * @param infoPopupHeader Header text for the info popup dialog. The caller
     * retains the ownership of the string.
     * @param infoPopupMessage text for the info popup dialog The caller
     * retains the ownership of the string.
     */
    BaseStage(TextRenderer& textRenderer, GLuint rectIndexBuffer,
              GLuint defaultFrameBuffer, GLuint simpleColorProgram,
              GLint simpleColorMvpLoc, GLint simpleColorColorLoc,
              float stageDuration,
              const char* infoPopupHeader, const char* infoPopupMessage,
              float nearClip, float farClip,
              float popupInitialDelay = InfoPopupInitialDelayDefault,
              float popupDisplayTime = InfoPopupDisplayTimeDefault);

protected:
    /** Stage implementation specific setup. */
    virtual bool SetupImpl() = 0;

    /** Stage implementation must provide rendering implementation. */
    virtual void RenderImpl(const TimeSample& now) = 0;

    /** Stage implementation for its teardown. */
    virtual void TeardownImpl() = 0;

    /**
     * Stage implementation to calculate its score. Called right after the
     * stage completes. The base class method will provide fps/score calculation
     * and should be called in the beginning of the implemented method.
     */
    virtual void UpdateScore(const TimeSample& now);

    /** Sets a new duration for the stage. Fade-out time will be adjusted. */
    void UpdateStageDuration(float stageDuration);

    /** Recalculate the projection matrices. */
    void RecalculateProjection();
    
private: 
    bool SetupInfoPopup();
    void DrawInfoPopup();

    /**
     * Draws a "fader" polygon on the screen. The color/alpha/program/ortho
     * projection must be set beforehand.
     */
    void DrawFader();

protected: // Data
    // Whether running in interactive mode; default is false for normal
    // benchmarking mode
    bool m_interactiveMode;

    // Stage data
    int m_numFrames;
    TimeSample m_firstFrameTime;
    float m_stageDuration;
    StageData m_stageData;

    // Fade in/out animations
    float m_fadeDuration;
    ScalarAnimation* m_fadeInAnimation;
    ScalarAnimation* m_fadeOutAnimation;
    
    // id of the default frame buffer
    GLuint m_defaultFrameBuffer;

    // Near / far clip plane distances (as Z depth)
    float m_nearClip;
    float m_farClip;
    
    // Screen dimensions
    int m_viewportWidth;
    int m_viewportHeight;

    // Vertex/Index buffer for drawing rectangles
    GLuint m_vertexBuffer;
    GLuint m_rectIndexBuffer;

    // Simple color shader
    GLuint m_simpleColorProgram;
    GLuint m_simpleColorMvpLoc;
    GLuint m_simpleColorColorLoc;

    // Text renderer
    TextRenderer& m_textRenderer;

    // Info popup
    float m_infoPopupInitialDelay;
    float m_infoPopupDisplayTime;
    const char* m_infoPopupHeader;
    const char* m_infoPopupMessage;
    GLuint m_infoPopupTexture;
    float m_infoPopupHeaderScale;
    float m_infoPopupMessageScale;
    int m_infoPopupHeaderYOffset;
    int m_infoPopupHeaderXOffset;
    int m_infoPopupMessageYOffset;
    int m_infoPopupMessageXOffset;
    float m_infoPopupTop;
    float m_infoPopupHeight;
    InfoPopupAnimation* m_infoPopupAnimation;

    // Full-screen rect for fadeins/outs
    CommonGL::Rect m_fullscreenRect;
    
    // Vertex buffer for rendering the fader
    GLuint m_fullscreenRectVertexBuffer;

    // Alpha value for fadeins/outs (0 = no fade, 1 = full fade)
    float m_fade;

    // Perspective projection matrix
    float m_perspectiveProjectionMatrix[16];

    // Orthographic projection matrix
    float m_orthoProjectionMatrix[16];
};

#endif // BASESTAGE_H
