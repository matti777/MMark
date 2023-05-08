//
//  GLView.m
//  MMark
//
//  Created by Matti Dahlbom on 22.6.2012.
//  Copyright (c) 2012 Qvik Oy. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/EAGLDrawable.h>

#import "GLView.h"

#include "MMarkIOSController.h"
#include "CommonFunctions.h"

class MMarkIOSController;

// Private stuff
@interface GLView () {
    //////////////////////////////////////////////
    // C++ resources
    //////////////////////////////////////////////
    
    // MMark controller
    MMarkIOSController* m_mmarkController;
    
    //////////////////////////////////////////////
    // Objective-C resources
    //////////////////////////////////////////////
    
    // OpenGL context
    EAGLContext* context;
    
    // UI scale (1.0 for non-retina devices, 2.0 for retina)
    float uiScale; 
    
    // Display link for redraws
    CADisplayLink* displayLink;
    
    // Gesture recognizers
//    UITapGestureRecognizer* tapRecognizer;
//    UIPanGestureRecognizer* panRecognizer;
    
    // OpenGL objects
    GLuint defaultFrameBuffer;
    GLuint colorRenderBuffer;
    GLuint depthStencilRenderBuffer;
    
    // Render buffer dimensions
    GLint bufferWidth; 
    GLint bufferHeight;
}
@end

@implementation GLView

-(BOOL) isBenchmarking {
    if ( m_mmarkController == NULL ) {
        return NO;
    } else {
        return (BOOL)m_mmarkController->IsBenchmarkRunning();
    }
}

#pragma mark - from UIGestureRecognizerDelegate

-(BOOL) gestureRecognizer:(UIGestureRecognizer*)gestureRecognizer shouldReceiveTouch:(UITouch*)touch {
    // Only allow for touches to trigger the gesture if the touched
    // view is this GLView
    return (touch.view == self);
}

#pragma mark - private methods

//-(void) tapEvent:(UITapGestureRecognizer*)recognizer {
//    // We only care about completed tab events..
//    if ( recognizer.state != UIGestureRecognizerStateEnded ) {
//        return;
//    }
//    
//    CGPoint p = [recognizer locationInView:self];
//    m_mmarkController->TapEvent(round(p.x * uiScale), round(p.y * uiScale));
//}

/*
-(void) panEvent:(UITapGestureRecognizer*)recognizer {
    CGPoint p = [recognizer locationInView:self];
    int x = round(p.x * uiScale);
    int y = round(p.y * uiScale);
    
    switch ( recognizer.state ) {
        case UIGestureRecognizerStateBegan:
            m_mmarkController->PointerDragStarted(x, y);
            break;
        case UIGestureRecognizerStateChanged:
            m_mmarkController->PointerDragged(x, y);
            break;
        default:
            m_mmarkController->PointerDragEnded();
            break;
    }
}
*/

-(void) pauseRequest:(NSNotification*)notification {
    BOOL pause = [[notification.userInfo objectForKey:kPause] boolValue];
    if ( pause ) {
        LOG_DEBUG("Pausing rendering..");
        displayLink.paused = YES;
    } else {
        LOG_DEBUG("Resuming rendering..");
        displayLink.paused = NO;
    }
}

-(void) render:(CADisplayLink*)displayLink {
    DEBUG_ASSERT(m_mmarkController->Draw());
    
    // Apple specific "depth buffer discard" optimization
    const GLenum discards[] = {GL_DEPTH_ATTACHMENT};
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFrameBuffer);
    glDiscardFramebufferEXT(GL_FRAMEBUFFER, 1, discards);
    
    // Displays the renderbuffer
    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderBuffer);
    [context presentRenderbuffer:GL_RENDERBUFFER];
}

-(BOOL) initializeFrameBuffer {
    // Framebuffer & color buffer
    glGenFramebuffers(1, &defaultFrameBuffer);
    glGenRenderbuffers(1, &colorRenderBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFrameBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderBuffer);
    
    // Bind the rendering context to the render buffer and get the dimensions
    [context renderbufferStorage:GL_RENDERBUFFER fromDrawable:(CAEAGLLayer*)self.layer];
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &bufferWidth);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &bufferHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRenderBuffer);
    
    LOG_DEBUG("GLView: Render buffer dimension: %d x %d", bufferWidth, bufferHeight);
    
    // Depth & stencil buffer
	glGenRenderbuffers(1, &depthStencilRenderBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthStencilRenderBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES, bufferWidth, bufferHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 
                              GL_RENDERBUFFER, depthStencilRenderBuffer);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, 
                              GL_RENDERBUFFER, depthStencilRenderBuffer);
    
    // Check that the render buffer was set up properly
    if ( glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE )
	{
        LOG_DEBUG("GLView: Failed to make complete framebuffer object 0x%x", 
                  glCheckFramebufferStatus(GL_FRAMEBUFFER));
        return NO;
    }

    return YES;
}

-(NSString*) description {
    return @"GLView";
}

-(BOOL) initialize {
    // TODO verify that this is the right thing to do    
    if ( context != nil ) {
        LOG_DEBUG("GLView initialize: already initialized!");
        return YES;
    }
    
    // Get the layer
    CAEAGLLayer* eaglLayer = (CAEAGLLayer*)self.layer;
    
    eaglLayer.opaque = YES;
    eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                    [NSNumber numberWithBool:NO], kEAGLDrawablePropertyRetainedBacking,
                                    kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
    
    // Setup OpenGL ES 2.0 context
    context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2]; 
    if ( (context == nil) || ![EAGLContext setCurrentContext:context] ) {
        LOG_DEBUG("Loading OpenGL ES 2.0 context failed");
        return NO;
    }
    
    // Enable native resolution on high-resolution devices
    uiScale = [[UIScreen mainScreen] scale];
    self.contentScaleFactor = uiScale;
    LOG_DEBUG("UI scale = %f", uiScale);
    
    // Setup framebuffer
    if ( ![self initializeFrameBuffer] ) {
        return NO;
    }

    // Create & initialize the MMark controller framework
    m_mmarkController = new MMarkIOSController(defaultFrameBuffer);
    if ( !m_mmarkController->InitController() ) {
        LOG_DEBUG("InitController() failed, exiting..");
        return NO;
    }
    m_mmarkController->ViewportResized(bufferWidth, bufferHeight);

    // Add notification observers
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(pauseRequest:)
                                                 name:kPauseRequestNotification object:nil];
    
    // Add gesture recognizers
//    tapRecognizer = [[UITapGestureRecognizer alloc]
//                     initWithTarget:self action:@selector(tapEvent:)];
//    tapRecognizer.delegate = self;
//    [self addGestureRecognizer:tapRecognizer];

//    panRecognizer = [[UIPanGestureRecognizer alloc]
//                     initWithTarget:self action:@selector(panEvent:)];
//    panRecognizer.delegate = self;
//    [self addGestureRecognizer:panRecognizer];

    // Setup display link for redraws
    displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(render:)];
    [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];

    LOG_DEBUG("GLView setup done.");
    
    return YES;
}

#pragma mark - from UIView

-(void) processTouches:(NSSet*)touches {
    for ( UITouch* touch in touches ) {
        CGPoint p = [touch locationInView:self];
        int x = round(p.x * uiScale);
        int y = round(p.y * uiScale);

        switch ( touch.phase ) {
            case UITouchPhaseBegan:
                m_mmarkController->TouchStarted((void*)touch, x, y);
                break;
            case UITouchPhaseMoved:
                m_mmarkController->TouchMoved((void*)touch, x, y);
                break;
            case UITouchPhaseEnded:
            case UITouchPhaseCancelled:
                m_mmarkController->TouchEnded((void*)touch, x, y);
                break;
            case UITouchPhaseStationary:
                // No action
                break;
        }
    }
}

-(void) touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
    if ( [event touchesForView:self].count >= 3 ) {
        // 3+ touches active, send event
        m_mmarkController->TripleTouch();
    }
    
    [self processTouches:[event touchesForView:self]];
}

-(void) touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
    [self processTouches:[event touchesForView:self]];
}

-(void) touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
    [self processTouches:[event touchesForView:self]];
}

-(void) touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event {
    [self processTouches:[event touchesForView:self]];
}

+(Class) layerClass {
    return [CAEAGLLayer class];
}

-(void) layoutSubviews {
    LOG_DEBUG("GLView.layoutSubviews");
    
    [super layoutSubviews];
    
    // Initialize everything
    assert([self initialize]);
}

#pragma mark - lifecycle etc

-(id) initWithCoder:(NSCoder *)aDecoder {
    self = [super initWithCoder:aDecoder];
    if ( self != nil ) {
        self.multipleTouchEnabled = YES;
    }
    return self;
}

-(void) dealloc {
//    [self removeGestureRecognizer:tapRecognizer];
//    [tapRecognizer release];

    [[NSNotificationCenter defaultCenter] removeObserver:self];
    
    delete m_mmarkController;
    
    [displayLink release];
    displayLink = nil;
    
    // Release OpenGL resources
    glDeleteFramebuffers(1, &defaultFrameBuffer);
    glDeleteRenderbuffers(1, &colorRenderBuffer);
    glDeleteRenderbuffers(1, &depthStencilRenderBuffer);
    
    // Tear down context
    if ( [EAGLContext currentContext] == context ) {
        [EAGLContext setCurrentContext:nil];
    }
    
    [context release];
    context = nil;
    
    [super dealloc];
}

@end
