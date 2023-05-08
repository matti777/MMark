//
//  MMarkIOSController.mm
//  MMark
//
//  Created by Matti Dahlbom on 22.6.2012.
//  Copyright (c) 2012 All rights reserved.
//

#include <iostream>
#import <sys/utsname.h>

#import "InputDialog.h"
#import "NSData+Base64.h"
#import "UIDevice-Hardware.h"

#include "MMarkIOSController.h"
#include "CommonFunctions.h"

MMarkIOSController::MMarkIOSController(GLuint defaultFrameBuffer)
    : MMarkController()
{
    m_defaultFrameBuffer = defaultFrameBuffer;
    m_platformInfo = std::string([[[UIDevice currentDevice] platform] UTF8String]);
}

MMarkIOSController::~MMarkIOSController()
{
}

bool MMarkIOSController::IsBenchmarkRunning()
{
    return (m_state == StateRunning);
}

void MMarkIOSController::SetPaused(bool paused)
{
    NSDictionary* userInfo = [NSDictionary dictionaryWithObject:[NSNumber numberWithBool:paused] forKey:kPause];
    [[NSNotificationCenter defaultCenter] postNotificationName:kPauseRequestNotification 
                                                        object:nil 
                                                      userInfo:userInfo];
}

void MMarkIOSController::Redraw()
{
    // No action
}

void MMarkIOSController::OpenInBrowser(std::string url) const
{
    NSString* urlStr = [NSString stringWithUTF8String:url.c_str()];
    [[UIApplication sharedApplication] openURL:[NSURL URLWithString:urlStr]];
}

void MMarkIOSController::ShowMessage(std::string msg) const
{
//    NSString* str = [NSString stringWithUTF8String:msg.c_str()];
//    UIAlertView* dlg = [[UIAlertView alloc] initWithTitle:@"MMark"
//                                                  message:str
//                                                 delegate:nil
//                                        cancelButtonTitle:@"OK"
//                                        otherButtonTitles:nil];
//    [dlg show];
//    [dlg release];
}

//std::string MMarkIOSController::QueryUserName() const
//{
//    InputDialog* dlg = [InputDialog dialog];
//    [dlg showModal];
//    return std::string([[dlg getInput] UTF8String]);
//}

std::string parseCpuType(const char* versionString) {
    LOG_DEBUG("parsing version string: '%s'", versionString);
    std::string ret = "n/a";

    NSError* error = nil;
    NSString* pattern = @"RELEASE_(.+)?";
    NSRegularExpression* regex = [[NSRegularExpression alloc] initWithPattern:pattern
                                                                      options:NSRegularExpressionCaseInsensitive
                                                                        error:&error];
    if ( error != nil ) {
        NSLog(@"Failed to create regex: %@", error);
        return ret;
    }
    
//    NSString* input = @"Darwin Kernel Version 13.0.0: Wed Oct 10 23:32:19 PDT 2012; root:xnu-2107.2.34~2/RELEASE_ARM_S5L8950X";
    NSString* input = [NSString stringWithUTF8String:versionString];
    NSArray* matches = [regex matchesInString:input options:0 range:NSMakeRange(0, input.length)];
    for ( NSTextCheckingResult* result in matches ) {
        for ( int captureIndex = 1; captureIndex < result.numberOfRanges; captureIndex++) {
            NSString* capture = [input substringWithRange:[result rangeAtIndex:captureIndex]];
//            NSLog(@"Found '%@'", capture);
            ret = [capture UTF8String];
            break;
        }
    }
    
    [regex release];
    
    return ret;
}

DeviceInfo MMarkIOSController::GetDeviceInfo() const
{
    UIDevice* device = [UIDevice currentDevice];
    NSString* deviceModel = device.model;
    NSString* deviceModelType = @"";

    // Split the platformString ('iPhone 4S') into model and model type
    NSString* platform = device.platformString;
    NSRange r = [platform rangeOfString:@" "];
    if ( r.location != NSNotFound ) {
        deviceModel = [platform substringToIndex:r.location];
        deviceModelType = [platform substringFromIndex:(r.location + 1)];
    }
    
    DeviceInfo info = DeviceInfo();
    info.m_manufacturer = "Apple";
    info.m_model = [deviceModel UTF8String];
    info.m_productName = [deviceModelType UTF8String];
    info.m_osVersion = [device.systemVersion UTF8String];
    info.m_totalRam = (int)(device.totalMemory / 1024); // to kB
    struct utsname systemInfo;
    uname(&systemInfo);
    info.m_cpuType = parseCpuType(systemInfo.version);
    info.m_numCpuCores = device.cpuCount;
    info.m_cpuFrequency = device.cpuFrequency / 1000000ul; // to MHz

    // Sanity check the cpu frequency and possibly use a hardcoded value if
    // a proper value is not available
    if ( (info.m_cpuFrequency <= 0) || (info.m_cpuFrequency > 10000) )
    {
        info.m_cpuFrequency = 0;
        
        // A hack to provide cpu frequencies from specs
        if ( strcmp(systemInfo.machine, "iPhone5,2") == 0 )
        {
            info.m_cpuFrequency = 1200; // 1.2 GHz for iphone 5
        } else if ( (strncmp(systemInfo.machine, "iPad4", 5) == 0) ||
                   (strcmp(systemInfo.machine, "iPad3,5") == 0) ||
                   (strcmp(systemInfo.machine, "iPad3,6") == 0) )
        {
            info.m_cpuFrequency = 1400; // 1.4 GHz for ipad4
        } else if ( (strcmp(systemInfo.machine, "iPad2,5") == 0) ||
                   (strcmp(systemInfo.machine, "iPad2,6") == 0) ||
                   (strcmp(systemInfo.machine, "iPad2,7") == 0) ) {
            info.m_cpuFrequency = 1000; // 1 GHz for ipad mini
        }
    }
    
    if ( UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad )
    {
        info.m_deviceType = "tablet";
    }
    else
    {
        info.m_deviceType = "mobilephone";
    }
    
    return info;
}

std::string MMarkIOSController::GetPlatformInfo()
{
    return m_platformInfo;
}

void MMarkIOSController::SubmitScore(const std::string& json,
                                     const std::string& signature)
{
    LOG_DEBUG("MMarkIOSController::SubmitScore(): json = \n%s", json.c_str());
    
    NSURL* url = [NSURL URLWithString:[NSString stringWithUTF8String:ScoreSubmitURL]];
    NSMutableURLRequest* request = [NSMutableURLRequest requestWithURL:url
                                                           cachePolicy:NSURLRequestReloadIgnoringLocalCacheData
                                                       timeoutInterval:15.0];
    NSData* requestData = [NSData dataWithBytes:json.c_str() length:json.length()];

    // Create Basic auth to avoid unnecessary auth check later
    NSString* authStr = [NSString stringWithFormat:@"%s:%s", JsonApiUsername, JsonApiPassword];
    NSData* data = [authStr dataUsingEncoding:NSUTF8StringEncoding];
    NSString* authHeader = [NSString stringWithFormat:@"Basic %@", [data base64EncodedString]];
    
    [request setHTTPMethod:@"POST"];
    [request setValue:authHeader forHTTPHeaderField:@"Authorization"];
    [request setValue:@"application/json" forHTTPHeaderField:@"Accept"];
    [request setValue:@"application/json; charset=utf-8" forHTTPHeaderField:@"Content-Type"];
    [request setValue:[NSString stringWithUTF8String:signature.c_str()] forHTTPHeaderField:[NSString stringWithUTF8String:SignatureHeader]];
    [request setValue:[NSString stringWithFormat:@"%d", [requestData length]] forHTTPHeaderField:@"Content-Length"];
    [request setHTTPBody:requestData];
    
    NSOperationQueue* queue = [[NSOperationQueue alloc] init];
    
    [NSURLConnection sendAsynchronousRequest:request queue:queue completionHandler:^(NSURLResponse* response, NSData* data, NSError* error)
    {
        if ( error != nil ) {
            NSLog(@"Error with HTTP request = %@", error);
            if ( error.code == NSURLErrorNotConnectedToInternet ) {
                ShowMessage("Submit failed - check your internet connection.");
            } else {
                ShowMessage("Submit failed - you might be using an old version " \
                            "or the server could be down.");
            }
            
            ScoreSubmitted(false, std::string());
        } else {
            NSLog(@"Submit successful! data size: %d", [data length]);
            std::string responseJson;
            responseJson.assign((const char*)[data bytes], (size_t)[data length]);
            ScoreSubmitted(true, responseJson);
        }
        [queue release];
    }];
}

