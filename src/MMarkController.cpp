#include <sstream>
#include <ctype.h>
#include <algorithm>

#include "json/json.h"

#include "MMarkController.h"
#include "ChessboardStage.h"
#include "FractalStage.h"
#include "PhysicsStage.h"
#include "FillrateStage.h"
#include "CommonFunctions.h"
#include "MMarkTextRenderer.h"
#include "ScoreTextRenderer.h"
#include "Torus.h"
#include "RotationAnimation.h"
#include "ChessboardDemoMode.h"
#include "md5.h"

// For debugging purposes only!
//#define USE_DEBUG_SCORES

// Fade in/out duration (in seconds)
static const float FadeInOutDuration = 0.4;

// Background fader duration
static const float BgFaderDuration = 0.4;

// Background fader darkness
static const float BgFaderAlpha = 0.5;

// Duration (in seconds) of the device info appear/disappear animation
const float DisplaceAnimDuration = 0.4;

// Duration (in seconds) of displaying benchmarking instructions on top of
// the first stage
const float BenchmarkInfoDuration = 10.0;

// Duration to display the "exit" button after tripletouching the screen
// during a running benchmark
const float ExitButtonVisibleDuration = 4.0;

// Salt used for md5-"signing" the JSON submission
const std::string JsonMD5Salt("40c654878b333e1ecb037483fbd35a36");

MMarkController::MMarkController()
	: GLController(), m_state(StateReady), m_prevState(StateReady), m_exitButtonTimer(
		NULL), m_runFullTest(true), m_scoreAvailable(false), m_submittingScore(
		false), m_scoreSubmitted(false), m_scoreSubmitFailed(false), m_overallScore(
		0), m_cpuScore(0), m_fillRateScore(0), m_loadTimeScore(0), m_simpleTextureProgram(
		0), m_simpleTextureProgramMvpLoc(-1), m_simpleTextureProgramTextureLoc(
		-1), m_torus(NULL), m_torusXRotAnim(NULL), m_torusYRotAnim(
		NULL), m_torusZRotAnim(NULL), m_torusXRot(0), m_torusYRot(0), m_torusZRot(
		0), m_activeTouch(false), m_prevTouchX(-1), m_prevTouchY(-1), m_touchTorusRotX(
		0), m_touchTorusRotZ(0), m_textRenderer(NULL), m_fadeInAnimation(
		1.0, 0.0, 0.0, FadeInOutDuration, &m_fadeValue), m_fadeOutAnimation(
		0.0, 1.0, 0.0, FadeInOutDuration, &m_fadeValue), m_fadeValue(
		0.0), m_bgFaderAnimation(0.0, BgFaderAlpha, 0.0,
					 BgFaderDuration, &m_bgFaderAlpha), m_bgFaderAlpha(
		-1), m_buttonsCreated(false), m_startButtonTexture(0), m_submitButtonTexture(
		0), m_infoButtonTexture(0), m_exitButtonTexture(0), m_demoButtonTexture(
		0), m_cputestButtonTexture(0), m_fulltestButtonTexture(0), m_analyzeButtonTexture(
		0), m_menuBgTexture(0), m_infoButton(NULL), m_startButton(NULL), m_demoButton(
		NULL), m_exitButton(NULL), m_cpuTestButton(NULL), m_fullTestButton(
		NULL), m_analyzeButton(NULL), m_dimmer(NULL), m_deviceInfo(), m_displaceAnimation(
		NULL), m_displace(-1), m_infoVisible(false), m_currentStage(
		NULL), m_demoMode(NULL)
{
}

MMarkController::~MMarkController()
{
    LOG_DEBUG("MMarkController::~MMarkController()");

    // Delete all stages
    for (unsigned int i = 0; i < m_stages.size(); i++)
    {
	delete m_stages[i];
    }
    m_stages.clear();
    m_currentStage = NULL;

    delete m_textRenderer;
    delete m_exitButtonTimer;

    DestroyWidgets();
    DeinitWidgets();

    // Deallocate OpenGL resources
    UnloadMenuScreen();
    DestroyWidgets();
    glDeleteBuffers(1, &m_fullscreenRectVertexBuffer);
    UnloadShader(m_simpleTextureProgram);
    UnloadShader(m_simpleColorProgram);
    glDeleteTextures(1, &m_startButtonTexture);
    glDeleteTextures(1, &m_submitButtonTexture);
    glDeleteTextures(1, &m_infoButtonTexture);
    glDeleteTextures(1, &m_demoButtonTexture);
    glDeleteTextures(1, &m_cputestButtonTexture);
    glDeleteTextures(1, &m_fulltestButtonTexture);
    glDeleteTextures(1, &m_analyzeButtonTexture);
    glDeleteTextures(1, &m_exitButtonTexture);

    // Deallocate global data
    DeinitCommonData();

    LOG_DEBUG("MMarkController::~MMarkController() done.");
}

void MMarkController::UpdateWidgetVisibility()
{
    // Hide all widgets by default
    std::list<CommonGL::BaseWidget*>::iterator iter;
    for (iter = m_widgets.begin(); iter != m_widgets.end(); iter++)
    {
	{
	    (*iter)->SetVisible(false);
	}
    }

    // Show proper widgets for the new state
    switch (m_state)
    {
    case StateReady:
	m_startButton->SetVisible(true);
	m_demoButton->SetVisible(true);
	m_infoButton->SetVisible(true);
	break;
    case StateFinished:
	m_startButton->SetVisible(true);
	m_demoButton->SetVisible(true);
	m_infoButton->SetVisible(true);
	if ( m_runFullTest && !m_scoreSubmitted )
	{
	    m_analyzeButton->SetVisible(true);
	}
	break;
    case StateStarting:
    case StateStartMenu:
	m_dimmer->SetVisible(true);
	m_cpuTestButton->SetVisible(true);
	m_fullTestButton->SetVisible(true);
	if ( m_prevState == StateReady )
	{
	    m_startButton->SetVisible(true);
	    m_demoButton->SetVisible(true);
	    m_infoButton->SetVisible(true);
	}
	else if ( m_prevState == StateFinished )
	{
	    m_startButton->SetVisible(true);
	    m_demoButton->SetVisible(true);
	    m_infoButton->SetVisible(true);
	    if ( m_runFullTest && !m_scoreSubmitted )
	    {
		m_analyzeButton->SetVisible(true);
	    }
	}
	break;
    case StateRunningDemo:
	m_exitButton->SetVisible(true);
	break;
    default:
	// No visible widgets
	break;
    }
}

void MMarkController::ChangeState(MMarkState newState)
{
    LOG_DEBUG("MMarkController::ChangeState(): newState = %d", newState);

    // Set the new state
    m_state = newState;

    if ( m_buttonsCreated )
    {
	// Resize widgets (will also call UpdateWidgetVisibility())
	ResizeWidgets();
    }
}

void MMarkController::CreateWidgets()
{
    // Create the button widgets
    m_infoButton = new CommonGL::Button(m_infoButtonTexture, this);
    m_startButton = new CommonGL::Button(m_startButtonTexture, this);
    m_analyzeButton = new CommonGL::Button(m_analyzeButtonTexture, this);
    m_demoButton = new CommonGL::Button(m_demoButtonTexture, this);
    m_exitButton = new CommonGL::Button(m_exitButtonTexture, this);
    m_cpuTestButton = new CommonGL::Button(m_cputestButtonTexture, this);
    m_fullTestButton = new CommonGL::Button(m_fulltestButtonTexture, this);
    m_dimmer = new CommonGL::Container(this);

    // Add test selection buttons as children of the dimmer
    m_dimmer->Add(m_cpuTestButton);
    m_dimmer->Add(m_fullTestButton);

    // Add them to the widget manager
    Add(m_infoButton);
    Add(m_startButton);
    //TODO uncomment! also uncomment the "press analyze.. " print
    Add(m_analyzeButton);
    Add(m_demoButton);
    Add(m_exitButton);
    Add(m_dimmer);
    Add(m_cpuTestButton);
    Add(m_fullTestButton);

    m_buttonsCreated = true;
}

// Dimensions of the physical button textures (all except info)
const float ButtonPixelWidth = 633;
const float ButtonPixelHeight = 250;
const float ButtonSizeAspectRatio = ButtonPixelWidth / ButtonPixelHeight;
const float TestSelButtonWidth = 470;
const float TestSelButtonHeight = 570;

bool MMarkController::ResizeWidgets()
{
    LOG_DEBUG("MMarkController::ResizeWidgets()");

    // Figure out the button dimensions and locations
    float buttonHeight = m_viewportHeight / 6;
    buttonHeight = std::min(buttonHeight, ButtonPixelHeight);
    float buttonWidth = buttonHeight * ButtonSizeAspectRatio;
    float maxWidth = m_viewportWidth * 0.22;
    if ( maxWidth < buttonWidth )
    {
	const float ratio = maxWidth / buttonWidth;
	buttonHeight *= ratio;
	buttonWidth *= ratio;
    }

    float buttonCenterY = m_viewportHeight * 0.88;
    int xmargin = m_viewportWidth / 50;

    float infoInset = m_viewportHeight / 30.0;
    float infoX = m_viewportWidth - infoInset - (buttonHeight / 2);

    // Create the rects for the buttons
    CommonGL::Rect infoRect = CommonGL::Rect::Centered(infoX, buttonCenterY,
						       buttonHeight);
    CommonGL::Rect exitRect = CommonGL::Rect::Centered((m_viewportWidth / 2),
						       buttonCenterY,
						       buttonWidth,
						       buttonHeight);
    CommonGL::Rect startRect, demoRect, analyzeRect;

    bool useReadyConfig = (m_state == StateReady)
			  || ((m_state == StateFinished) && !m_runFullTest)
			  || ((m_state == StateFinished) && m_scoreSubmitted);

    if ( (m_state == StateStartMenu) || (m_state == StateStarting) )
    {
	if ( (m_prevState == StateReady)
	     || ((m_prevState == StateFinished) && !m_runFullTest)
	     || ((m_prevState == StateFinished) && m_scoreSubmitted) )
	{
	    useReadyConfig = true;
	}
    }

    if ( useReadyConfig )
    {
	startRect.SetCentered(m_viewportWidth / 3, buttonCenterY, buttonWidth,
			      buttonHeight);
	demoRect.SetCentered(m_viewportWidth * 2 / 3.0, buttonCenterY,
			     buttonWidth, buttonHeight);
    }
    else
    {
	analyzeRect.SetCentered(m_viewportWidth / 2, buttonCenterY, buttonWidth,
				buttonHeight);
	startRect = analyzeRect;
	startRect.MoveBy(-(buttonWidth + xmargin), 0);
	demoRect = analyzeRect;
	demoRect.MoveBy(buttonWidth + xmargin, 0);
    }

    const float testSelY = m_viewportHeight * 0.65;
    const float testButtonWidth = buttonHeight * 3;
    const float testButtonHeight = testButtonWidth
				   * (TestSelButtonHeight / TestSelButtonWidth);
    CommonGL::Rect cputestRect = CommonGL::Rect::Centered(m_viewportWidth / 3.0,
							  testSelY,
							  testButtonWidth,
							  testButtonHeight);
    cputestRect.MoveBy(m_viewportWidth, 0);
    CommonGL::Rect fulltestRect = CommonGL::Rect::Centered(
	    m_viewportWidth * 2 / 3.0, testSelY, testButtonWidth,
	    testButtonHeight);
    fulltestRect.MoveBy(m_viewportWidth, 0);

    // The 'dimmer' container is twice the width of the screen, stretching
    // to the right (contains the test selection buttons)
    CommonGL::Rect dimmerRect(m_fullScreenRect);
    dimmerRect.m_right *= 2;
    m_dimmer->SetBounds(dimmerRect);

    // Update the widget's rects
    m_startButton->SetBounds(startRect);
    m_demoButton->SetBounds(demoRect);
    m_infoButton->SetBounds(infoRect);
    m_exitButton->SetBounds(exitRect);
    m_cpuTestButton->SetBounds(cputestRect);
    m_fullTestButton->SetBounds(fulltestRect);
    m_analyzeButton->SetBounds(analyzeRect);

    // Show/hide correct widgets
    UpdateWidgetVisibility();

    return true;
}

void MMarkController::DestroyWidgets()
{
    LOG_DEBUG("MMarkController::DestroyWidgets()");
    delete m_infoButton;
    m_infoButton = NULL;
    delete m_startButton;
    m_startButton = NULL;
    delete m_demoButton;
    m_demoButton = NULL;
    delete m_exitButton;
    m_exitButton = NULL;
    delete m_cpuTestButton;
    m_cpuTestButton = NULL;
    delete m_fullTestButton;
    m_fullTestButton = NULL;
    delete m_analyzeButton;
    m_analyzeButton = NULL;
    delete m_dimmer;
    m_dimmer = NULL;
}

bool MMarkController::LoadMenuScreen()
{
    LOG_DEBUG("MMarkController::LoadMenuScreen()");
    LOG_GL_ERROR();

    delete m_exitButtonTimer;
    m_exitButtonTimer = NULL;

    // Load menu background texture
    if ( !Load2DTextureFromBundle("menu_background.jpg", &m_menuBgTexture, true,
				  false) )
    {
	return false;
    }

    // Load wireframe objects
    m_torus = Torus::Create(16, 12, 3.0, 1.5);
    if ( m_torus == NULL )
    {
	return false;
    }

    // Make lines a bit wider for the torus
    glLineWidth(2.0);

    // Setup rotation animations with initial values
    m_torusXRot = 0.05 * 2 * M_PI;
    m_torusYRot = 0.3 * 2 * M_PI;
    m_torusXRotAnim = new RotationAnimation(0.35 * 2 * M_PI, 0.018 * 2 * M_PI,
					    &m_torusXRot);
    m_torusYRotAnim = new RotationAnimation(0.0, 0.08 * 2 * M_PI, &m_torusYRot);
    m_torusZRotAnim = new RotationAnimation(0.45 * 2 * M_PI, 0.035 * 2 * M_PI,
					    &m_torusZRot);

    if ( !m_scoreAvailable )
    {
	// Reset background fade animation for first time Ready screen is shown
	m_bgFaderAnimation.ResetTime();
    }

    return true;
}

void MMarkController::UnloadMenuScreen()
{
    LOG_DEBUG("MMarkController::UnloadMenuScreen()");

    delete m_torus;
    m_torus = NULL;

    delete m_torusXRotAnim;
    m_torusXRotAnim = NULL;

    delete m_torusYRotAnim;
    m_torusYRotAnim = NULL;

    delete m_torusZRotAnim;
    m_torusZRotAnim = NULL;

    glDeleteTextures(1, &m_menuBgTexture);
    m_menuBgTexture = 0;
}

void MMarkController::StartBenchmark(bool runFullTest)
{
    LOG_DEBUG("StartBenchmark(): runFullTest = %d", runFullTest);
    m_runFullTest = runFullTest;
    m_currentStage = NULL;
    m_stageIterator = m_stages.begin();
    m_fadeOutAnimation.Reset(0.0, 1.0, FadeInOutDuration);
    m_benchmarkStartTime.Reset();
    ChangeState(StateStarting);
}

bool MMarkController::BenchmarkFinished()
{
    m_scoreAvailable = true;
    m_scoreSubmitted = false;

    ChangeState(StateFinished);

    if ( !LoadMenuScreen() )
    {
	LOG_DEBUG("BenchmarkFinished(): LoadMenuScreen() failed");
	return false;
    }
    m_fadeInAnimation.Reset(1.0, 0.0, FadeInOutDuration);
    UpdateScore();
    return true;
}

void MMarkController::StartDemo()
{
    DEBUG_ASSERT(m_demoMode == NULL);

    m_fadeOutAnimation.Reset(0.0, 1.0, FadeInOutDuration);
    m_prevState = m_state;
    ChangeState(StateStartingDemo);
}

std::string lc(std::string input)
{
    // Return an lowercased copy of the argument string
    std::string data = input;
    std::transform(data.begin(), data.end(), data.begin(), ::tolower);
    return data;
}

void MMarkController::SetupDeviceInfo()
{
    m_deviceInfo = GetDeviceInfo();
    std::stringstream ss;
    ss << "cpu: ";
    if ( m_deviceInfo.m_cpuType != "" )
    {
	ss << lc(m_deviceInfo.m_cpuType);
    }
    else
    {
	ss << "n/a";
    }

    m_deviceInfoStrings.push_back(ss.str());
    ss.str(std::string());
    ss << "num cores: " << m_deviceInfo.m_numCpuCores;
    m_deviceInfoStrings.push_back(ss.str());
    ss.str(std::string());
    ss << "frequency: ";
    if ( m_deviceInfo.m_cpuFrequency == 0 )
    {
	ss << "n/a";
    }
    else
    {
	ss << m_deviceInfo.m_cpuFrequency << " mhz";
    }
    m_deviceInfoStrings.push_back(ss.str());
    ss.str(std::string());
    ss << "ram: " << m_deviceInfo.m_totalRam << " kb";
    m_deviceInfoStrings.push_back(ss.str());
    ss.str(std::string());
    ss << "gl vendor: " << lc((const char*) glGetString(GL_VENDOR));
    m_deviceInfoStrings.push_back(ss.str());
    ss.str(std::string());
    ss << "gl renderer: " << lc((const char*) glGetString(GL_RENDERER));
    m_deviceInfoStrings.push_back(ss.str());
    ss.str(std::string());
    ss << "gl version: " << lc((const char*) glGetString(GL_VERSION));
    m_deviceInfoStrings.push_back(ss.str());
    ss.str(std::string());
    ss << "glsl version: "
       << lc((const char*) glGetString(GL_SHADING_LANGUAGE_VERSION));
    m_deviceInfoStrings.push_back(ss.str());
}

bool MMarkController::InitController()
{
    LOG_DEBUG("MMarkController::InitController()");

    // Get device info and generate the strings to be displayed
    SetupDeviceInfo();

//    const char* extensions = (const char*)glGetString(GL_EXTENSIONS);
//    LOG_DEBUG("GL extensions: %s", extensions);

    // Clear any GL errors
    LOG_DEBUG("Initial GL error: %d", glGetError());

    // Start with the superclass defaults
    if ( !GLController::InitController() )
    {
	LOG_DEBUG("GLController::InitController() failed!");
	return false;
    }

    // Initialize the Common data
    if ( !InitCommonData() )
    {
	LOG_DEBUG("MMarkController::InitController(): common data init failed");
	return false;
    }

    // Enable vertex attrib arrays
    glEnableVertexAttribArray(COORD_INDEX);
    glEnableVertexAttribArray(TEXCOORD_INDEX);
    glEnableVertexAttribArray(NORMAL_INDEX);

    // Load the 2D rendering program
    if ( !LoadShaderFromBundle("SimpleTexture", &m_simpleTextureProgram) )
    {
	return false;
    }

    // Get GLSL uniform locations
    m_simpleTextureProgramMvpLoc = glGetUniformLocation(m_simpleTextureProgram,
							"mvp_matrix");
    m_simpleTextureProgramTextureLoc = glGetUniformLocation(
	    m_simpleTextureProgram, "texture");

    // Load button textures
    if ( !Load2DTextureFromBundle("start_button.png", &m_startButtonTexture,
				  true, false)
	 || !Load2DTextureFromBundle("analyze_button.png",
				     &m_submitButtonTexture, true, false)
	 || !Load2DTextureFromBundle("info_button.png", &m_infoButtonTexture,
				     true, false)
	 || !Load2DTextureFromBundle("exit_button.png", &m_exitButtonTexture,
				     true, false)
	 || !Load2DTextureFromBundle("cputest_button.png",
				     &m_cputestButtonTexture, true, false)
	 || !Load2DTextureFromBundle("fulltest_button.png",
				     &m_fulltestButtonTexture, true, false)
	 || !Load2DTextureFromBundle("demo_button.png", &m_demoButtonTexture,
				     true, false)
	 || !Load2DTextureFromBundle("analyze_button.png",
				     &m_analyzeButtonTexture, true, false) )
    {
	return false;
    }

    LOG_GL_ERROR();

    // Initialize widget system
    if ( !InitWidgets() )
    {
	LOG_DEBUG("InitWidgets() failed!");
	return false;
    }
    CreateWidgets();

    // Create & init the score text renderer
    m_textRenderer = new ScoreTextRenderer(m_simpleTextureProgram,
					   m_simpleTextureProgramMvpLoc,
					   m_simpleTextureProgramTextureLoc,
					   g_rectangleIndexBuffer);
    if ( !m_textRenderer->Setup() )
    {
	LOG_DEBUG("score text renderer setup failed!");
	return false;
    }

#ifndef USE_DEBUG_SCORES
    // Create stages
    m_stages.push_back(
	    new FractalStage(*m_textRenderer, g_rectangleIndexBuffer,
		    m_defaultFrameBuffer, m_simpleColorProgram,
		    m_simpleColorMvpLoc, m_simpleColorColorLoc));
    m_stages.push_back(
	    new FillrateStage(*m_textRenderer, g_rectangleIndexBuffer,
		    m_defaultFrameBuffer, m_simpleColorProgram,
		    m_simpleColorMvpLoc, m_simpleColorColorLoc));
    m_stages.push_back(
	    new ChessboardStage(*m_textRenderer, g_rectangleIndexBuffer,
		    m_defaultFrameBuffer, m_simpleColorProgram,
		    m_simpleColorMvpLoc, m_simpleColorColorLoc));
    m_stages.push_back(
	    new PhysicsStage(*m_textRenderer, g_rectangleIndexBuffer,
		    m_defaultFrameBuffer, m_simpleColorProgram,
		    m_simpleColorMvpLoc, m_simpleColorColorLoc));
#endif
    m_stageIterator = m_stages.begin();

    // Set initial state; note that we cant do ChangeState() here since widgets
    // are still NULL
    m_state = StateReady;
    if ( !LoadMenuScreen() )
    {
	LOG_DEBUG("LoadMenuScreen() failed");
	return false;
    }
    m_fadeInAnimation.Reset(1.0, 0.0, FadeInOutDuration);

    LOG_GL_ERROR();

    return true;
}

void MMarkController::ScoreSubmitted(bool success, std::string response)
{
    m_submittingScore = false;
    m_scoreSubmitFailed = !success;

    if ( !success )
    {
	LOG_DEBUG("MMarkController::ScoreSubmitted(): submission failed");
	return;
    }

    LOG_DEBUG("Server response = %s", response.c_str());

    // Parse the newly created score UUID from the response
    Json::Reader reader;
    Json::Value root;
    if ( !reader.parse(response, root, false) )
    {
	// This is an error obviously - one which should not happen
	ShowMessage("Failed to parse server response!");
	return;
    }

    LOG_DEBUG("MMarkController::ScoreSubmitted(): successful!");

    // Change the UI configuration (remove Analyze button)
    m_scoreSubmitted = true;
    ResizeWidgets();

    std::string uuid = root["score_uuid"].asString();
    std::string nonce = root["nonce"].asString();

    // Construct the URL
    std::stringstream ss;
    ss << ViewScoreURL << uuid << "&nonce=" << nonce;
    std::string url = ss.str();
    LOG_DEBUG("Opening browser to URL: %s", url.c_str());

    // Open in a browser
    OpenInBrowser(url);
}

void MMarkController::UpdateScore()
{
    LOG_DEBUG("MMarkController::UpdateScore()");

#ifdef USE_DEBUG_SCORES
    StageData data1, data2, data3, data4;
    data1.m_fps = 25;
    data1.m_score = 454;
    data1.m_cpuScore = 0;
    data1.m_fillRateScore = 0;
    data1.m_loadTime = 1.0;
    data1.m_numImages = 14;
    data2.m_fps = 25;
    data2.m_score = 454;
    data2.m_cpuScore = 567;
    data2.m_fillRateScore = 123;
    data2.m_loadTime = 2.0;
    data3.m_fps = 25;
    data3.m_score = 454;
    data3.m_cpuScore = 0;
    data3.m_fillRateScore = 0;
    data3.m_loadTime = 3.0;
    data4.m_fps = 28;
    data4.m_score = 644;
    data4.m_cpuScore = 0;
    data4.m_fillRateScore = 0;
    data4.m_loadTime = 4.0;
#else
    StageData data1 = m_stages[0]->GetStageData();
    StageData data2 = m_stages[1]->GetStageData();
    StageData data3 = m_stages[2]->GetStageData();
    StageData data4 = m_stages[3]->GetStageData();
#endif

    float totalLoadTime = data1.m_loadTime + data2.m_loadTime + data3.m_loadTime
			  + data4.m_loadTime;
    m_loadTimeScore = (int) (3000.0 / totalLoadTime);

    m_overallScore = data1.m_score + data2.m_score + data3.m_score
		     + data4.m_score + m_loadTimeScore;
    m_cpuScore = data1.m_cpuScore + data2.m_cpuScore + data3.m_cpuScore
		 + data4.m_cpuScore;
    m_fillRateScore = data1.m_fillRateScore + data2.m_fillRateScore
		      + data3.m_fillRateScore + data4.m_fillRateScore;
    LOG_DEBUG("MMarkController::UpdateScore(): final scores: overall: %d, "
              "CPU: %d, fillrate: %d, loadTime: %d",
              m_overallScore, m_cpuScore, m_fillRateScore, m_loadTimeScore);
}

std::string MMarkController::GetPlatformInfo()
{
    return std::string("");
}

std::string MMarkController::CreateScoreSubmissionJson()
{
    Json::Value deviceInfo;
#if defined(__BUILD_DEVICE__)
#if defined(__ANDROID__)
    deviceInfo["platform"] = "android";
#elif defined(__BLACKBERRY__)
    deviceInfo["platform"] = "blackberry";
#elif defined(__IOS__)
    deviceInfo["platform"] = "ios";
#elif defined(__TIZEN__)
    deviceInfo["platform"] = "tizen";
#else
#error Must define a platform!
#endif
#else // __BUILD_DESKTOP__
    deviceInfo["platform"] = "Desktop";
#endif

    deviceInfo["platform_info"] = GetPlatformInfo();
    deviceInfo["manufacturer"] = m_deviceInfo.m_manufacturer;
    deviceInfo["model"] = m_deviceInfo.m_model;
    deviceInfo["product_name"] = m_deviceInfo.m_productName;
    deviceInfo["device_type"] = m_deviceInfo.m_deviceType;
    deviceInfo["os_version"] = m_deviceInfo.m_osVersion;
    deviceInfo["total_ram"] = m_deviceInfo.m_totalRam;
    deviceInfo["cpu_type"] = m_deviceInfo.m_cpuType;
    deviceInfo["num_cpu_cores"] = m_deviceInfo.m_numCpuCores;
    deviceInfo["cpu_max_frequency"] = m_deviceInfo.m_cpuFrequency;
    deviceInfo["total_ram"] = m_deviceInfo.m_totalRam;
    deviceInfo["screen_width"] = m_viewportWidth;
    deviceInfo["screen_height"] = m_viewportHeight;
    deviceInfo["gl_vendor"] = (const char*) glGetString(GL_VENDOR);
    deviceInfo["gl_renderer"] = (const char*) glGetString(GL_RENDERER);
    deviceInfo["gl_version"] = (const char*) glGetString(GL_VERSION);
    deviceInfo["glsl_version"] = (const char*) glGetString(
	    GL_SHADING_LANGUAGE_VERSION);

#ifdef USE_DEBUG_SCORES
    StageData data1, data2, data3, data4;
    data1.m_fps = 25;
    data1.m_score = 454;
    data1.m_cpuScore = 0;
    data1.m_fillRateScore = 0;
    data1.m_loadTime = 1.0;
    data1.m_numImages = 14;
    data2.m_fps = 25;
    data2.m_score = 454;
    data2.m_cpuScore = 567;
    data2.m_fillRateScore = 123;
    data2.m_loadTime = 2.0;
    data2.m_unlightedFillRate = 78.4;
    data2.m_vertexLightedFillRate = 69.4;
    data2.m_pixelLightedFillRate = 45.8;
    data2.m_mappedLightedFillRate = 21.1;
    data3.m_fps = 25;
    data3.m_score = 454;
    data3.m_cpuScore = 0;
    data3.m_fillRateScore = 0;
    data3.m_loadTime = 3.0;
    data4.m_fps = 28;
    data4.m_score = 644;
    data4.m_cpuScore = 0;
    data4.m_fillRateScore = 0;
    data4.m_loadTime = 4.0;
#else
    StageData data1 = m_stages[0]->GetStageData();
    StageData data2 = m_stages[1]->GetStageData();
    StageData data3 = m_stages[2]->GetStageData();
    StageData data4 = m_stages[3]->GetStageData();
#endif

    Json::Value score;
#ifdef DEBUG
    score["version"] = "debug";
#else
    score["version"] = "1.1";
#endif
    score["fractal_score"] = data1.m_score;
    score["fractal_loadtime"] = data1.m_loadTime;
    score["fractal_num_images"] = data1.m_numImages;
    score["fillrate_score"] = data2.m_score;
    score["fillrate_loadtime"] = data2.m_loadTime;
    score["chess_score"] = data3.m_score;
    score["chess_loadtime"] = data3.m_loadTime;
    score["chess_fps"] = data3.m_fps;
    score["mountains_score"] = data4.m_score;
    score["mountains_loadtime"] = data4.m_loadTime;
    score["mountains_fps"] = data4.m_fps;

    score["total_score"] = m_overallScore;
    score["loadtime_score"] = m_loadTimeScore;
    score["unlighted_fillrate"] = data2.m_unlightedFillRate;
    score["vertex_lighted_fillrate"] = data2.m_vertexLightedFillRate;
    score["pixel_lighted_fillrate"] = data2.m_pixelLightedFillRate;
    score["mapped_lighted_fillrate"] = data2.m_mappedLightedFillRate;

    // Check the existence of required extensions
    std::string missingFeatures = data2.m_missingFeatures;
    if ( !PackedDepthStencilExtensionPresent() )
    {
        missingFeatures += " ";
        missingFeatures += PackedDepthStencilExtension;
    }
    if ( !DepthBufferExtensionPresent() )
    {
        missingFeatures += " ";
        missingFeatures += DepthTextureExtension;
    }
    LOG_DEBUG("missingFeatures: '%s'", missingFeatures.c_str());
    deviceInfo["missing_features"] = missingFeatures;

    Json::Value user;
//    user["name"] = QueryUserName();
    user["name"] = "";

    Json::Value root;
    root["submit_id"] = RandomUuid();
    root["device_info"] = deviceInfo;
    root["score"] = score;
    root["user"] = user;

    Json::StyledWriter writer;

    return writer.write(root);
}

bool MMarkController::SetupNextStage()
{
    LOG_DEBUG("MMarkController::SetupNextStage()");

    if ( m_stageIterator != m_stages.end() )
    {
	m_currentStage = *m_stageIterator++;
	if ( !m_currentStage->Setup(m_viewportWidth, m_viewportHeight) )
	{
	    LOG_DEBUG("MMarkController::SetupNextStage(): Stage setup failed!");
	    m_currentStage = NULL;
	    return false;
	}
	else
	{
	    // Stage set up and ready to go
//            m_state = StateRunning;
	    ChangeState(StateRunning);
	    return true;
	}
    }
    else
    {
	LOG_DEBUG("MMarkController::SetupNextStage() Last stage finished.");
	return BenchmarkFinished();
    }
}

void MMarkController::DrawImage(const CommonGL::Rect& rect, GLuint texture)
{
    // Set up the shader program + uniforms
    //TODO no need to set the program every time!
    m_textRenderer->SetProgram();
    m_textRenderer->SetTexture(texture);
    DrawImage2D(rect, m_viewportWidth, m_viewportHeight);
}

bool MMarkController::RenderMenuBackground(const TimeSample& now)
{
    // Clear the screen & depth buffer with white
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw the background image without writing depth
    glDepthMask(false);
    DrawImage(m_fullScreenRect, m_menuBgTexture);
    glDepthMask(true);

    // Draw a fader on top of the background
    m_bgFaderAnimation.Animate(now);
    DrawFader(m_bgFaderAlpha);

    // Update wireframe object transforms
    m_torusXRotAnim->Animate(now);
    m_torusYRotAnim->Animate(now);
    m_torusZRotAnim->Animate(now);

    float modelTransform[16];
    float mvpMatrix[16];
    float xRotation[16];
    float torusRotX = m_torusXRot + m_touchTorusRotX;
    MatrixCreateRotation(xRotation, torusRotX, 1, 0, 0);
    float yRotation[16];
    MatrixCreateRotation(yRotation, m_torusYRot, 0, 1, 0);
    float zRotation[16];
    float torusRotZ = m_torusZRot + m_touchTorusRotZ;
    MatrixCreateRotation(zRotation, torusRotZ, 0, 0, 1);
    MatrixMultiply(yRotation, zRotation, modelTransform);
    MatrixMultiply(modelTransform, xRotation, modelTransform);
    MatrixMultiply(modelTransform, m_vpMatrix, mvpMatrix);

    // Render the torus with 2-pass; first filled with alpha = 0 just for the
    // depth and then with white lines
    glDepthFunc(GL_LESS);
    glUseProgram(m_simpleColorProgram);
    glUniformMatrix4fv(m_simpleColorMvpLoc, 1, GL_FALSE, mvpMatrix);
    const float clearColor[] =
    {
    0.0, 0.0, 0.0, 0.0
    };
    glUniform4fv(m_simpleColorColorLoc, 1, clearColor);
    m_torus->Render(true);
    glDepthFunc(GL_LEQUAL);
    const float whiteColor[] =
    {
    1.0, 1.0, 1.0, 0.3
    };
    glUniform4fv(m_simpleColorColorLoc, 1, whiteColor);
    m_torus->Render(false);

    return true;
}

bool MMarkController::HandleReady(const TimeSample& now)
{
    RenderMenuBackground(now);

    // Draw the copyright text
    float scale = (m_viewportHeight * 0.047) / m_textRenderer->GetFontHeight();
    int y = (m_textRenderer->GetFontHeight() * scale);
    m_textRenderer->SetProgram();
    m_textRenderer->SetTexture();
    DrawCenteredText("copyright 2012-2013 matti dahlbom", y, scale);
    //    DrawCenteredText("abc", y, scale);

    // Draw the button widgets
    DrawWidgets();

    // Draw fader if fadein/out active
    DrawFader(m_fadeValue);

    if ( m_infoVisible )
    {
	DrawInfo(now);
    }

    return true;
}

bool MMarkController::HandleFinished(const TimeSample& now)
{
    RenderMenuBackground(now);

    // Draw the score
    float scale = (m_viewportHeight * 0.15) / m_textRenderer->GetFontHeight();
    int y = (m_viewportHeight * 0.4)
	    - (m_textRenderer->GetFontHeight() * scale);
    m_textRenderer->SetProgram();
    m_textRenderer->SetTexture();
    DrawCenteredText("score", y, scale);

    char msg[64];
    y += (m_textRenderer->GetFontHeight() * scale);
    scale *= 0.80;
    if ( m_runFullTest )
    {
	sprintf(msg, "overall: %d", m_overallScore);
	DrawCenteredText(msg, y, scale);
    }
    sprintf(msg, "cpu: %d", m_cpuScore);
    y += (m_textRenderer->GetFontHeight() * scale);
    DrawCenteredText(msg, y, scale);
    if ( m_runFullTest )
    {
	scale *= 0.80;
	sprintf(msg, "fill rate: %d", m_fillRateScore);
	y += (m_textRenderer->GetFontHeight() * scale);
	DrawCenteredText(msg, y, scale);
    }

    // Draw the copyright text
    scale = (m_viewportHeight * 0.05) / m_textRenderer->GetFontHeight();
    y = (m_textRenderer->GetFontHeight() * scale);
    DrawCenteredText("copyright 2012-2013 matti dahlbom", y, scale);

    scale = (m_viewportHeight * 0.05) / m_textRenderer->GetFontHeight();
    y = m_viewportHeight * 0.75;

    if ( m_scoreSubmitFailed )
    {
	DrawCenteredText("analyze failed, check network", y, scale);
    }
    else if ( !m_scoreSubmitted && m_runFullTest )
    {
	//TODO uncomment! also uncomment the Add()
	DrawCenteredText("press analyze for in-depth report", y, scale);
    }

    LOG_GL_ERROR();
    DrawWidgets();

    // Draw fader if fadein/out active
    DrawFader(m_fadeValue);

    if ( m_submittingScore )
    {
	DrawFader(0.8);
    }
    else if ( m_infoVisible )
    {
	DrawInfo(now);
    }
    LOG_GL_ERROR();

    return true;
}

bool MMarkController::HandleStartMenu(const TimeSample& now)
{
    // Update dimmer alpha
    m_dimmer->ColorPtr()[3] = ((1.0 - m_displace) * 0.9);

    // Update dimmer position
    int dimmerx = -((1.0 - m_displace) * m_viewportWidth);
    m_dimmer->BoundsRef().MoveTo(dimmerx, 0);
//    LOG_DEBUG("state = %d, dimmerx = %d, m_displace = %f",
//              m_state, dimmerx, m_displace);

    // Advance animation
    if ( (m_displaceAnimation != NULL) && m_displaceAnimation->Animate(now) )
    {
	// Animation completed
	LOG_DEBUG("deleting m_displaceAnimation");
	delete m_displaceAnimation;
	m_displaceAnimation = NULL;
	if ( m_displace >= 1.0 )
	{
	    ChangeState(m_prevState);
	    return true;
	}
    }

    bool ret;
    if ( m_prevState == StateReady )
    {
	ret = HandleReady(now);
    }
    else if ( m_prevState == StateFinished )
    {
	ret = HandleFinished(now);
    }
    else
    {
	LOG_DEBUG("HandleStartMenu(): Illegal state: %d", m_prevState);
	return false;
    }

    if ( (m_state == StateStartMenu) && (m_displace <= 0.0) )
    {
	// Only draw this text if menu animation not pending
	float scale = (m_viewportHeight * 0.1)
		      / m_textRenderer->GetFontHeight();
	int y = (m_textRenderer->GetFontHeight() * scale) * 2.5;
	m_textRenderer->SetProgram();
	m_textRenderer->SetTexture();
	DrawCenteredText("select benchmark", y, scale);
    }

    return ret;
}

bool MMarkController::HandleRunning(const TimeSample& now)
{
    if ( m_currentStage == NULL )
    {
	return SetupNextStage();
    }

    if ( !m_currentStage->Render(now) )
    {
	// A stage has completed
	LOG_DEBUG("Current stage finished.");
	m_currentStage->Teardown();
	m_currentStage = NULL;

	// If not running the full test, we're done already
	if ( !m_runFullTest )
	{
	    return BenchmarkFinished();
	}
    }

    if ( now.ElapsedTimeSince(m_benchmarkStartTime) <= BenchmarkInfoDuration )
    {
	// Display simple instructions/info in the beginning of the benchmarking
	float scale = (m_viewportHeight * 0.055)
		      / m_textRenderer->GetFontHeight();
	int y = (m_textRenderer->GetFontHeight() * scale);
	m_textRenderer->SetProgram();
	m_textRenderer->SetTexture();
	if ( m_runFullTest )
	{
	    DrawCenteredText("full test takes about 3 min 17 s", y, scale);
	}
	else
	{
	    DrawCenteredText("cpu test takes about 20 s", y, scale);
	}

	DrawCenteredText("touch with 3 or more fingers to exit", 2 * y, scale);
    }

    if ( m_exitButtonTimer != NULL )
    {
	if ( m_exitButtonTimer->ElapsedTime() <= ExitButtonVisibleDuration )
	{
	    m_exitButton->SetVisible(true);
	    DrawWidgets();
	}
	else
	{
	    delete m_exitButtonTimer;
	    m_exitButtonTimer = NULL;
	}
    }

    return true;
}

bool MMarkController::HandleRunningDemo(const TimeSample& now)
{
    // Check if we need to init
    if ( m_demoMode == NULL )
    {
	m_demoMode = new ChessboardDemoMode(*m_textRenderer,
					    g_rectangleIndexBuffer,
					    m_defaultFrameBuffer,
					    m_simpleColorProgram,
					    m_simpleColorMvpLoc,
					    m_simpleColorColorLoc);
	if ( !m_demoMode->Setup(m_viewportWidth, m_viewportHeight) )
	{
	    LOG_DEBUG("MMarkController::HandleRunningDemo(): setup failed!");
	    m_demoMode->Teardown();
	    delete m_demoMode;
	    m_demoMode = NULL;
	    return false;
	}
	else
	{
	    // Demo setup ok, running it!
	    ChangeState(StateRunningDemo);
	    return true;
	}
    }

    if ( !m_demoMode->Render(now) )
    {
	// Demo mode exited into the menu
	LOG_DEBUG("Demo mode done, going back to mode %d", m_prevState);
	m_demoMode->Teardown();
	delete m_demoMode;
	m_demoMode = NULL;
	ChangeState(m_prevState);
	m_fadeInAnimation.Reset(1.0, 0.0, FadeInOutDuration);
    }

    DrawWidgets();

    return true;
}

bool MMarkController::HandleAborting(const TimeSample& now)
{
    m_fadeOutAnimation.Animate(now);
    m_currentStage->Render(now);
    DrawFader(m_fadeValue);
    if ( m_fadeOutAnimation.HasCompleted(now) )
    {
	m_currentStage->Teardown();
	m_currentStage = NULL;

	LOG_DEBUG("Abort done, going to StateReady!");
	ChangeState(StateReady);
	if ( !LoadMenuScreen() )
	{
	    LOG_DEBUG("LoadMenuScreen() failed");
	    return false;
	}
	m_fadeInAnimation.Reset(1.0, 0.0, FadeInOutDuration);
    }

    return true;
}

void MMarkController::DrawCenteredText(const char* text, int y, float scale,
				       int xOffset)
{
    int w = m_textRenderer->GetTextWidth(text, scale);
    int x = (m_viewportWidth - w) / 2;
    m_textRenderer->DrawText(x + xOffset, y, text, scale);
}

void MMarkController::DrawInfo(const TimeSample& now)
{
    if ( (m_displaceAnimation != NULL) && m_displaceAnimation->Animate(now) )
    {
	// Animation completed; delete the object
	delete m_displaceAnimation;
	m_displaceAnimation = NULL;

	if ( m_displace >= 1.0 )
	{
	    // The info has completed animating off the screen; mark info
	    // as being not visible
	    m_infoVisible = false;
	    return;
	}
    }

    DrawFader((1.0 - m_displace) * 0.9);
    int xOffset = m_displace * m_viewportWidth;
    float scale = (m_viewportHeight * 0.05) / m_textRenderer->GetFontHeight();
    int y = m_viewportHeight * 0.2;
    m_textRenderer->SetProgram();
    m_textRenderer->SetTexture();
    const float headerScale = scale * 1.30;
    DrawCenteredText("device info", y, headerScale, xOffset);
    y += (m_textRenderer->GetFontHeight() * headerScale * 1.3);

    std::list<std::string>::const_iterator iter = m_deviceInfoStrings.begin();
    for (; iter != m_deviceInfoStrings.end(); iter++)
    {
	DrawCenteredText(iter->c_str(), y, scale, xOffset);
	y += (m_textRenderer->GetFontHeight() * scale);
    }
}

bool MMarkController::DrawFader(float fadeValue)
{
    if ( fadeValue > 0.0 )
    {
	// Fade in/out going on, draw an occlusing fader
	glUseProgram(m_simpleColorProgram);
	glUniformMatrix4fv(m_simpleColorMvpLoc, 1, GL_FALSE,
			   m_orthoProjectionMatrix);
	float color[] =
	{
	0.0, 0.0, 0.0, fadeValue
	};
	glUniform4fv(m_simpleColorColorLoc, 1, color);
	DrawQuad2D(m_fullscreenRectVertexBuffer, g_rectangleIndexBuffer);
    }

    return true;
}

bool MMarkController::Draw()
{
    bool ret = true;
    TimeSample now;

    //    LOG_DEBUG("Draw(): state = %d", m_state);

    switch (m_state)
    {
    case StateRunning:
	ret = HandleRunning(now);
	break;
    case StateStartMenu:
	ret = HandleStartMenu(now);
	break;
    case StateAborting:
	ret = HandleAborting(now);
	break;
    case StateReady:
	m_fadeInAnimation.Animate(now);
	ret = HandleReady(now);
	break;
    case StateStartingDemo:
    case StateStarting:
	ret = HandleStartMenu(now);

	if ( m_fadeOutAnimation.Animate(now) )
	{
	    // Fadeout done, start!
	    if ( m_state == StateStartingDemo )
	    {
		ChangeState(StateRunningDemo);
	    }
	    else
	    {
		UnloadMenuScreen();
		ChangeState(StateRunning);
	    }
	}
	break;
    case StateRunningDemo:
	ret = HandleRunningDemo(now);
	break;
    case StateFinished:
	m_fadeInAnimation.Animate(now);
	ret = HandleFinished(now);
	break;
    }

    return ret;
}

void MMarkController::ViewportResized(int width, int height)
{
    LOG_DEBUG("MMarkController::ViewportResized(): %d x %d", width, height);

    if ( (width == m_viewportWidth) && (height == m_viewportHeight) )
    {
	LOG_DEBUG("Unnecessary resize call; already have those dimensions!");
	return;
    }

    // Call superclass
    GLController::ViewportResized(width, height);

    // Update the text renderer's projection matrix
    m_textRenderer->ViewportResized(m_viewportWidth, m_viewportHeight);

    // Resize button widgets
    if ( m_buttonsCreated )
    {
	ResizeWidgets();
    }

    // Create view*projection matrix for the menu wireframe objects
    float perspectiveProj[16];
    MatrixPerspectiveProjection(perspectiveProj, 25,
				(float) (m_viewportWidth) / m_viewportHeight,
				0.5, 100.0);
    float lookat[16];
    float cameraPos[] =
    {
    0.0, 0.0, 35.0
    };
    float cameraTarget[] =
    {
    0.0, -1.0, 0.0
    };
    MatrixSetLookat(lookat, cameraPos, cameraTarget);
    MatrixMultiply(lookat, perspectiveProj, m_vpMatrix);

    if ( m_currentStage != NULL )
    {
	// Notify the current stage as well
	m_currentStage->ViewportResized(m_viewportWidth, m_viewportHeight);
    }
}

void MMarkController::ToggleStartMenu()
{
    LOG_DEBUG("ToggleStartMenu()");

    if ( m_state == StateStartMenu )
    {
	DEBUG_ASSERT(m_displaceAnimation == NULL);

	// Get rid of start menu
	m_displaceAnimation = new ScalarAnimation(0, 1, 0, DisplaceAnimDuration,
						  &m_displace);
    }
    else
    {
	// Bring in the start menu
	m_displace = 1.0;
	m_prevState = m_state;
	ChangeState(StateStartMenu);
	m_displaceAnimation = new ScalarAnimation(1, 0, 0, DisplaceAnimDuration,
						  &m_displace);
    }
}

void MMarkController::ToggleInfo()
{
    if ( m_infoVisible )
    {
	// The screen was tapped while info is visible
	//TODO this doesnt need to be handled, touches are blocked while animation is active
	if ( m_displaceAnimation != NULL )
	{
	    // Info still appearing; ignore this tap
	    return;
	}
	else
	{
	    // Start animating the info off the screen; note that
	    // m_infoVisible will be set to false in DrawInfo() once the
	    // animation completes
	    m_displaceAnimation = new ScalarAnimation(0, 1, 0,
						      DisplaceAnimDuration,
						      &m_displace);
	}
    }
    else
    {
	// Show the info; start animating the info onto the screen
	m_infoVisible = true;
	m_displaceAnimation = new ScalarAnimation(1, 0, 0, DisplaceAnimDuration,
						  &m_displace);
    }
}

/*
 void MMarkController::TapEvent(int x, int y)
 {
 LOG_DEBUG("MMarkController::TapGesture(): %d, %d", x, y);

 // TODO move to TouchStarted()
 if ( m_infoVisible )
 {
 ToggleInfo();
 return;
 }

 if ( m_state == StateReady )
 {
 //        if ( m_startButtonReadyRect.IsInside(x, y) )
 //        {
 //            // Start the test!
 //            StartStage();
 //        }
 //        else if ( m_infoButtonRect.IsInside(x, y) )
 //        {
 //            ToggleInfo();
 //        }
 //        else if ( m_demoButtonReadyRect.IsInside(x, y) )
 //        {
 //            StartDemo();
 //        }
 }
 //    else if ( m_state == StateRunningDemo )
 //    {
 //        m_demoMode->TapEvent(x, y);
 //    }
 else if ( m_state == StateFinished )
 {
 //        if ( m_infoButtonRect.IsInside(x, y) ) {
 //            ToggleInfo();
 //            return;
 //        }

 //        if ( m_scoreSubmitted )
 //        {
 //            if ( m_startButtonReadyRect.IsInside(x, y) )
 //            {
 //                // Restart the test, rewinding back to the first stage.
 //                StartBenchmark();
 //            }
 //            else if ( m_demoButtonReadyRect.IsInside(x, y) )
 //            {
 //                StartDemo();
 //            }
 //        }
 //        else
 //        {
 //            if ( m_startButtonFinishedRect.IsInside(x, y) )
 //            {
 //                // Restart the test, rewinding back to the first stage.
 //                StartBenchmark();
 //            }
 //            else if ( m_demoButtonFinishedRect.IsInside(x, y) )
 //            {
 //                StartDemo();
 //            }
 //            else if ( !m_submittingScore && m_submitButtonRect.IsInside(x, y) )
 //            {
 //                m_submittingScore = true;
 //                m_scoreSubmitFailed = false;
 //                std::string json = CreateScoreSubmissionJson();
 //                std::string saltedJson = json + JsonMD5Salt;
 //                std::string signature = md5(saltedJson);
 //                LOG_DEBUG("saltedJson = '%s', signature = '%s'",
 //                          saltedJson.c_str(), signature.c_str());
 //                LOG_DEBUG("md5(json) = %s", md5(json).c_str());
 //                SubmitScore(json, signature);
 //            }
 //        }
 }
 }
 */

bool MMarkController::TouchStarted(const void* touch, int x, int y)
{
    LOG_DEBUG("TouchStarted at (%d,%d)", x, y);

    // Block all touches while displacement (sliding) animation is active
    if ( m_displaceAnimation != NULL )
    {
	LOG_DEBUG("TouchStarted(): m_displaceAnimation != NULL, discard touch");
	return false;
    }

    if ( m_infoVisible )
    {
	ToggleInfo();
	return true;
    }

    if ( GLController::TouchStarted(touch, x, y) )
    {
	return true;
    }

    m_activeTouch = true;
    m_prevTouchX = x;
    m_prevTouchY = y;

    return true;
}

bool MMarkController::TouchMoved(const void* touch, int x, int y)
{
    LOG_DEBUG("TouchMoved at (%d,%d)", x, y);

    if ( GLController::TouchMoved(touch, x, y) )
    {
	return true;
    }

    if ( m_activeTouch
	 && ((m_state == StateReady) || (m_state == StateFinished)
	     || (m_state == StateRunningDemo)) )
    {
	const int diffX = (x - m_prevTouchX);
	const int diffY = (y - m_prevTouchY);

	const float SwipeToRadiansConstant = 1.0 / 300.0;
	m_touchTorusRotX += diffY * SwipeToRadiansConstant;
	m_touchTorusRotZ += -diffX * SwipeToRadiansConstant;
	m_prevTouchX = x;
	m_prevTouchY = y;

	if ( m_demoMode != NULL )
	{
	    m_demoMode->TouchMoved(diffX, diffY);
	}

	return true;
    }

    return false;
}

bool MMarkController::TouchEnded(const void* touch, int x, int y)
{
    LOG_DEBUG("TouchEnded at (%d,%d)", x, y);

    GLController::TouchEnded(touch, x, y);
    m_activeTouch = false;
    return true;
}

void MMarkController::TripleTouch()
{
    LOG_DEBUG("MMarkController::TripleTouch()");

    if ( m_state == StateRunning )
    {
	if ( m_exitButtonTimer == NULL )
	{
	    LOG_DEBUG("new m_exitButtonTimer()");
	    m_exitButtonTimer = new TimeSample();
	}
	else
	{
	    LOG_DEBUG("m_exitButtonTimer->Reset()");
	    m_exitButtonTimer->Reset();
	}
    }
}

void MMarkController::ButtonPressed(CommonGL::Button* button)
{
    LOG_DEBUG("MMarkController::ButtonPressed() = 0x%x", (size_t)button);

    if ( button == m_infoButton )
    {
	ToggleInfo();
    }
    else if ( button == m_startButton )
    {
	ToggleStartMenu();
    }
    else if ( button == m_demoButton )
    {
	StartDemo();
    }
    else if ( button == m_exitButton )
    {
	if ( m_state == StateRunningDemo )
	{
	    m_demoMode->UpdateStageDurationFromNow(FadeInOutDuration);
	}
	else
	{
	    m_currentStage->Abort();
	    ChangeState(StateAborting);
	    m_fadeOutAnimation.Reset(0.0, 1.0, FadeInOutDuration);
	}
    }
    else if ( button == m_cpuTestButton )
    {
	StartBenchmark(false);
    }
    else if ( button == m_fullTestButton )
    {
	StartBenchmark(true);
    }
    else if ( button == m_analyzeButton )
    {
	if ( !m_submittingScore )
	{
	    m_submittingScore = true;
	    m_scoreSubmitFailed = false;
	    std::string json = CreateScoreSubmissionJson();
	    std::string saltedJson = json + JsonMD5Salt;
	    std::string signature = md5(saltedJson);
	    LOG_DEBUG(
		    "saltedJson = '%s', signature = '%s'", saltedJson.c_str(), signature.c_str());
	    LOG_DEBUG("md5(json) = %s", md5(json).c_str());
	    SubmitScore(json, signature);
	}
    }
}

void MMarkController::WidgetTouched(CommonGL::BaseWidget*, int, int)
{
    LOG_DEBUG("MMarkController::WidgetTouched()");

    if ( m_state == StateStartMenu )
    {
	ToggleStartMenu();
    }
}
