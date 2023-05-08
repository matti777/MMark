/**
 * Name        : MMark13
 * Version     :
 * Vendor      :
 * Description :
 */


#include "MMark13.h"
#include "MMark13Frame.h"
#include "GLRenderer.h"

using namespace Tizen::App;
using namespace Tizen::Base;
using namespace Tizen::System;
using namespace Tizen::Ui;
using namespace Tizen::Ui::Controls;
using namespace Tizen::System;

MMark13App::MMark13App(void)
{
}

MMark13App::~MMark13App(void)
{
}

UiApp*
MMark13App::CreateInstance(void)
{
	// Create the instance through the constructor.
	return new MMark13App();
}

bool
MMark13App::OnAppInitializing(AppRegistry& appRegistry)
{
	// TODO:
	// Initialize Frame and App specific data.
	// The App's permanent data and context can be obtained from the appRegistry.
	//
	// If this method is successful, return true; otherwise, return false.
	// If this method returns false, the App will be terminated.

	// Uncomment the following statement to listen to the screen on/off events.
	//PowerManager::SetScreenEventListener(*this);

	// TODO:
	// Add your initialization code here
	return true;
}

bool
MMark13App::OnAppInitialized(void)
{
    // TODO:
    // Add code to do after initialization here.

    // Create a Frame
    MMark13Frame* pMMark13Frame = new MMark13Frame();
    pMMark13Frame->Construct();
    pMMark13Frame->SetName(L"MMark13");
    pMMark13Frame->SetPropagatedKeyEventListener(this);
    AddFrame(*pMMark13Frame);
    pMMark13Frame->SetOrientation(ORIENTATION_LANDSCAPE);

    {
	__player = new Tizen::Graphics::Opengl::GlPlayer;
	__player->Construct(Tizen::Graphics::Opengl::EGL_CONTEXT_CLIENT_VERSION_2_X,
	                    pMMark13Frame);

	__player->SetFps(-1); // As fast as possible
	//__player->SetEglAttributePreset(Tizen::Graphics::Opengl::EGL_ATTRIBUTES_PRESET_RGB565);
	__player->SetEglAttributePreset(Tizen::Graphics::Opengl::EGL_ATTRIBUTES_PRESET_ARGB8888);

	__player->Start();
    }

    GLRenderer* renderer = new GLRenderer();
    __renderer = renderer;
    __player->SetIGlRenderer(__renderer);

    // Set touch event listener to pass Tizen touch events to MMark core
    pMMark13Frame->AddTouchEventListener(renderer->GetTouchEventListener());

    // Disable screen turning off / dimming
    PowerManager::KeepScreenOnState(true, false);

    // Dont let CPU go to sleep mode (not that it would..)
    PowerManager::KeepCpuAwake(true);

    return true;
}

bool
MMark13App::OnAppWillTerminate(void)
{
    // TODO:
    // Add code to do somethiing before application termination.
    return true;
}

bool
MMark13App::OnAppTerminating(AppRegistry& /*appRegistry*/,
                             bool /*forcedTermination*/)
{
    // TODO:
    // Deallocate resources allocated by this App for termination.
    // The App's permanent data and context can be saved via appRegistry.
    AppLogDebug("MMark13App::OnAppTerminating(): deallocating resources..");

    __player->Stop();

    if(__renderer != null)
    {
	delete __renderer;
    }
    delete __player;

    return true;
}

void
MMark13App::OnForeground(void)
{
	// TODO:
	// Start or resume drawing when the application is moved to the foreground.
}

void
MMark13App::OnBackground(void)
{
	// TODO:
	// Stop drawing when the application is moved to the background.
}

void
MMark13App::OnLowMemory(void)
{
	// TODO:
	// Free unused resources or close the application.
}

void
MMark13App::OnBatteryLevelChanged(BatteryLevel batteryLevel)
{
	// TODO:
	// Handle any changes in battery level here.
	// Stop using multimedia features(camera, mp3 etc.) if the battery level is CRITICAL.
}

void
MMark13App::OnScreenOn(void)
{
	// TODO:
	// Get the released resources or resume the operations that were paused or stopped in OnScreenOff().
}

void
MMark13App::OnScreenOff(void)
{
	// TODO:
	// Unless there is a strong reason to do otherwise, release resources (such as 3D, media, and sensors) to allow the device
	// to enter the sleep mode to save the battery.
	// Invoking a lengthy asynchronous method within this listener method can be risky, because it is not guaranteed to invoke a
	// callback before the device enters the sleep mode.
	// Similarly, do not perform lengthy operations in this listener method. Any operation must be a quick one.
}

// IPropagatedKeyEventListener

bool MMark13App::OnKeyPressed(Tizen::Ui::Control& /*source*/,
	                           const Tizen::Ui::KeyEventInfo& /*keyEventInfo*/)
{
    return false;
}

bool MMark13App::OnKeyReleased(Tizen::Ui::Control& /*source*/,
	                           const Tizen::Ui::KeyEventInfo& keyEventInfo)
{
    KeyCode key = keyEventInfo.GetKeyCode();
    if ( (key == KEY_BACK) || (key == KEY_ESC) )
    {
	AppLogDebug("Back/ESC pressed, terminating application..");
	Terminate();
	return true;
    }

    return false;
}

bool MMark13App::OnPreviewKeyPressed(Tizen::Ui::Control& /*source*/,
	                           const Tizen::Ui::KeyEventInfo& /*keyEventInfo*/)
{
    return false;
}

bool MMark13App::OnPreviewKeyReleased(Tizen::Ui::Control& /*source*/,
	                           const Tizen::Ui::KeyEventInfo& /*keyEventInfo*/)
{
    return false;
}
