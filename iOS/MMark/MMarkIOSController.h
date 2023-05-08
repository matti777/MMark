//
//  MMarkIOSController.h
//  MMark
//
//  Created by Matti Dahlbom on 22.6.2012.
//  Copyright (c) 2012 All rights reserved.
//

#ifndef MMark_MMarkIOSController_h
#define MMark_MMarkIOSController_h

#include "MMarkController.h"

// Notifications
static NSString* const kPauseRequestNotification = @"PauseRequest";
static NSString* const kPause = @"Pause";

/**
 * iOS specific implementations of the controller features.
 *
 * @author Matti Dahlbom
 * @since 0.1
 */
class MMarkIOSController : public MMarkController
{
public: // Constructors and destructor
    MMarkIOSController(GLuint defaultFrameBuffer);
    virtual ~MMarkIOSController();
    
public: // Public API
    bool IsBenchmarkRunning();
    
public: // From GLController
    void SetPaused(bool paused);
    void Redraw();
    
protected: // From MMarkController
    void OpenInBrowser(std::string url) const;
    void ShowMessage(std::string msg) const;
//    std::string QueryUserName() const;
    DeviceInfo GetDeviceInfo() const;
    std::string GetPlatformInfo();
    void SubmitScore(const std::string& json, const std::string& signature);
    
private: // Data
    std::string m_platformInfo;
};

#endif
