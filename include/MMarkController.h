#ifndef MMARKCONTROLLER_H
#define MMARKCONTROLLER_H

#include <vector>
#include <list>

#include "GLController.h"
#include "DeviceInfo.h"
#include "ScalarAnimation.h"
#include "Button.h"
#include "Container.h"

// Forward declarations
class ChessboardDemoMode;

// Base URL
//#define BASE_URL "http://192.168.1.2"
//#define BASE_URL "http://127.0.0.1:8000"
//#define BASE_URL "http://localhost:8000"
#define BASE_URL "http://mmark.777-team.org"

// Backend URL for score submissions
const char* const ScoreSubmitURL = BASE_URL "/upload";

// URL to open in browser for viewing the score
const char* const ViewScoreURL = BASE_URL "/view_score?c=m&uuid=";

// HTTP header names
const char* const SignatureHeader = "X-MMark-Signature";

// JSON API authentication
const char* const JsonApiUsername = "mmarkclient";
const char* const JsonApiPassword = "a4b54eee8800697954b9ccdb685ffa9f";

// Forward declarations
class BaseStage;
class TextRenderer;
class Torus;
class RotationAnimation;

/**
 * This class controls the application flow.
 *
 * @author Matti Dahlbom
 * @since 0.1
 */
class MMarkController : public GLController, CommonGL::ButtonListener,
        CommonGL::WidgetTouchListener
{
public:
    // Controller states
    enum MMarkState {
        StateReady, // Menu screen with just Start button
        StateStartMenu, // Show benchmark selection
        StateStarting, // Menu screen with fader
        StateRunning, // Running the stages
        StateAborting, // Aborting the benchmark after a triple touch
        StateStartingDemo, // Starting interactive demo; do menu screen with fader
        StateRunningDemo, // Running the interactive demo
        StateFinished // Menu screen with start + submit buttons
    };

public: // Constructors and destructor
    MMarkController();
    virtual ~MMarkController();

public: // From GLController
    /** Does some initialization and calls superclass. */
    virtual void ViewportResized(int width, int height);

    /** Initializes the controller. */
    bool InitController();

    /** Draws everything. */
    virtual bool Draw();

    virtual bool TouchStarted(const void* touch, int x, int y);
    virtual bool TouchMoved(const void* touch, int x, int y);
    virtual bool TouchEnded(const void* touch, int x, int y);

    /** A touch with 3+ fingers received. */
    void TripleTouch();
    
public: // From ButtonListener
    void ButtonPressed(CommonGL::Button* button);

public: // From WidgetTouchListener
    void WidgetTouched(CommonGL::BaseWidget* widget, int x, int y);

private:
    void SetupDeviceInfo();
    bool SetupNextStage();
//    void SetupRects();
    void DrawImage(const CommonGL::Rect& rect, GLuint texture);
    void ToggleStartMenu();
    void StartBenchmark(bool cpuTestOnly);
    bool BenchmarkFinished();
    void StartDemo();
    void ToggleInfo();
    void UpdateWidgetVisibility();
    void CreateWidgets();
    bool ResizeWidgets();
    void DestroyWidgets();
    bool LoadMenuScreen();
    void UnloadMenuScreen();
    void UpdateScore();
    void DrawCenteredText(const char* text, int y, float scale, int xOffset = 0);
    
    bool RenderMenuBackground(const TimeSample& now);
    bool DrawFader(float faderValue);
    void DrawInfo(const TimeSample& now);
    bool HandleReady(const TimeSample& now);
    bool HandleStartMenu(const TimeSample& now);
    bool HandleRunning(const TimeSample& now);
    bool HandleAborting(const TimeSample& now);
    bool HandleRunningDemo(const TimeSample& now);
    bool HandleFinished(const TimeSample& now);

protected:
    /** Creates the JSON string used in score submission. */
    std::string CreateScoreSubmissionJson();

    /** Handles score submission completion. */
    void ScoreSubmitted(bool success, std::string response);

protected: // Platform dependant functionality
    /** Opens a URL in the default browser. */
    virtual void OpenInBrowser(std::string url) const = 0;

    /** Displays a popup (error) message. */
    virtual void ShowMessage(std::string msg) const = 0;

    /**
     * Asks user for his/her name
     * and returns the input. The string should be stored for the next time
     * and presented as a default value.
     */
//    virtual std::string QueryUserName() = 0;

    /** Returns device info. */
    virtual DeviceInfo GetDeviceInfo() const = 0;

    /** 
     * Returns the string to pass to server in platform_info field. Default
     * implementation returns a null/empty string.
     */
    virtual std::string GetPlatformInfo();

    /**
     * The implementation must:
     * - send the supplied JSON via HTTP POSTasynchronously
     * - Include the signature as a HTTP header
     * - handle errors and display a popup dialog if any errors
     * - call ScoreSubmitted(bool, response) when done
     */
    virtual void SubmitScore(const std::string& json,
                             const std::string& signature) = 0;

    /** Changes the app state. */
    virtual void ChangeState(MMarkState newState);
     
protected: // Data
    MMarkState m_state;
    MMarkState m_prevState;
    TimeSample* m_exitButtonTimer;

    TimeSample m_benchmarkStartTime;
    bool m_runFullTest;
    bool m_scoreAvailable;
    bool m_submittingScore;
    bool m_scoreSubmitted;
    bool m_scoreSubmitFailed;

    // Scores from latest run
    int m_overallScore;
    int m_cpuScore;
    int m_fillRateScore;
    int m_loadTimeScore;

    // 2D rendering program
    GLuint m_simpleTextureProgram;
    GLint m_simpleTextureProgramMvpLoc;
    GLint m_simpleTextureProgramTextureLoc;

    // Object(s) rendered in wireframe while menu is displayed
    Torus* m_torus;
    RotationAnimation* m_torusXRotAnim;
    RotationAnimation* m_torusYRotAnim;
    RotationAnimation* m_torusZRotAnim;
    float m_torusXRot;
    float m_torusYRot;
    float m_torusZRot;

    // Touch management
    bool m_activeTouch;
    int m_prevTouchX;
    int m_prevTouchY;
    float m_touchTorusRotX;
    float m_touchTorusRotZ;

    // Text renderers
    TextRenderer* m_textRenderer;

    // Fade out animation
    ScalarAnimation m_fadeInAnimation;
    ScalarAnimation m_fadeOutAnimation;
    float m_fadeValue;

    // Background fader
    ScalarAnimation m_bgFaderAnimation;
    float m_bgFaderAlpha;

    // Buttons
    bool m_buttonsCreated;
    GLuint m_startButtonTexture;
    GLuint m_submitButtonTexture;
    GLuint m_infoButtonTexture;
    GLuint m_exitButtonTexture;
    GLuint m_demoButtonTexture;
    GLuint m_cputestButtonTexture;
    GLuint m_fulltestButtonTexture;
    GLuint m_analyzeButtonTexture;
    GLuint m_menuBgTexture;
    CommonGL::Button* m_infoButton;
    CommonGL::Button* m_startButton;
    CommonGL::Button* m_demoButton;
    CommonGL::Button* m_exitButton;
    CommonGL::Button* m_cpuTestButton;
    CommonGL::Button* m_fullTestButton;
    CommonGL::Button* m_analyzeButton;
    CommonGL::Container* m_dimmer;

    // Device info
    DeviceInfo m_deviceInfo;
    std::list<std::string> m_deviceInfoStrings;
    ScalarAnimation* m_displaceAnimation;
    float m_displace;
    bool m_infoVisible;

    // Stages
    std::vector<BaseStage*> m_stages;
    std::vector<BaseStage*>::iterator m_stageIterator;
    BaseStage* m_currentStage;

    // Interactive mode
    ChessboardDemoMode* m_demoMode;

    // Camera+projection matrix for menu wireframe objects
    float m_vpMatrix[16];
};

#endif // MMARKCONTROLLER_H
