#ifndef _MMARK13_H_
#define _MMARK13_H_

#include "tizenx.h"
#include <FGrpGlPlayer.h>

/**
 * [MMark13App] UiApp must inherit from UiApp class
 * which provides basic features necessary to define an UiApp.
 */
class MMark13App
	: public Tizen::App::UiApp
	, public Tizen::System::IScreenEventListener,
	public Tizen::Ui::IPropagatedKeyEventListener
{
public:
	/**
	 * [Test] UiApp must have a factory method that creates an instance of itself.
	 */
	static Tizen::App::UiApp* CreateInstance(void);

public:
	MMark13App(void);
	virtual~MMark13App(void);

public: // From IPropagatedKeyEventListener
	virtual bool OnKeyPressed(Tizen::Ui::Control& source,
	                           const Tizen::Ui::KeyEventInfo& keyEventInfo);
	virtual bool OnKeyReleased(Tizen::Ui::Control& source,
	                           const Tizen::Ui::KeyEventInfo& keyEventInfo);
	virtual bool OnPreviewKeyPressed(Tizen::Ui::Control& source,
	                           const Tizen::Ui::KeyEventInfo& keyEventInfo);
	virtual bool OnPreviewKeyReleased(Tizen::Ui::Control& source,
	                           const Tizen::Ui::KeyEventInfo& keyEventInfo);

public:
	// Called when the UiApp is initializing.
	virtual bool OnAppInitializing(Tizen::App::AppRegistry& appRegistry);

	// Called when the UiApp initializing is finished. 
	virtual bool OnAppInitialized(void); 

	// Called when the UiApp is requested to terminate. 
	virtual bool OnAppWillTerminate(void); 

	// Called when the UiApp is terminating.
	virtual bool OnAppTerminating(Tizen::App::AppRegistry& appRegistry, bool forcedTermination = false);

	// Called when the UiApp's frame moves to the top of the screen.
	virtual void OnForeground(void);

	// Called when this UiApp's frame is moved from top of the screen to the background.
	virtual void OnBackground(void);

	// Called when the system memory is not sufficient to run the UiApp any further.
	virtual void OnLowMemory(void);

	// Called when the battery level changes.
	virtual void OnBatteryLevelChanged(Tizen::System::BatteryLevel batteryLevel);

	// Called when the screen turns on.
	virtual void OnScreenOn(void);

	// Called when the screen turns off.
	virtual void OnScreenOff(void);

private:
	Tizen::Graphics::Opengl::GlPlayer* __player;
	Tizen::Graphics::Opengl::IGlRenderer* __renderer;
};

#endif // _MMARK13_H_
