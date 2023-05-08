//
//  ViewController.m
//  MMark
//
//  Created by Matti Dahlbom on 22.6.2012.
//  Copyright (c) 2012 Qvik Oy. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>

#import "ViewController.h"
#import "GLView.h"

// Private stuff
@interface ViewController ()
@end

@implementation ViewController

#pragma mark - lifecycle etc

- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view, typically from a nib.
}

- (void)viewDidUnload
{
    [super viewDidUnload];
    // Release any retained subviews of the main view.
}

- (NSUInteger)supportedInterfaceOrientations {
    return (UIInterfaceOrientationMaskLandscapeRight | UIInterfaceOrientationMaskLandscapeLeft);
}

- (BOOL) shouldAutorotate {
    GLView* glView = (GLView*)self.view;
    return ![glView isBenchmarking];
}

- (UIInterfaceOrientation)preferredInterfaceOrientationForPresentation {
    return UIInterfaceOrientationMaskLandscapeRight;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    GLView* glView = (GLView*)self.view;
    if ( [glView isBenchmarking] ) {
        return NO;
    } else {
        return ( (interfaceOrientation == UIInterfaceOrientationLandscapeRight) ||
                (interfaceOrientation == UIInterfaceOrientationLandscapeLeft) );
    }
}

-(void) dealloc {
    
    [super dealloc];
}

@end
