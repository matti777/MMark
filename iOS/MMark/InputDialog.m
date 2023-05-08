//
//  InputDialog.m
//  ModalDialogTest
//
//  Created by Matti Dahlbom on 2.10.201240.
//  Copyright (c) 2012 Qvik. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>

#import "InputDialog.h"

static NSString* const kUserNameKey = @"UserName";

// Privates
@interface InputDialog ()
@property (nonatomic, assign) IBOutlet UITextField* textField;
@property (nonatomic, assign) CFRunLoopRef runloopRef;
@end

@implementation InputDialog

#pragma mark - public methods

-(void) showModal {
    // Get the topmost UIView and add the dialog as the child of that view
    UIWindow* keyWindow = [[UIApplication sharedApplication] keyWindow];
    UIView* topmostView = [keyWindow.subviews objectAtIndex:0];
//    NSLog(@"topmostView = %@", topmostView);
    [topmostView addSubview:self];
    
    // Center the dialog but allow room for keyboard not to block OK button
    CGSize s = topmostView.bounds.size;
    self.center = CGPointMake(s.width / 2, (s.height / 2) - 80);

    self.textField.text = [[NSUserDefaults standardUserDefaults] stringForKey:kUserNameKey];
    
    // Start a nested runloop to make the call block until dialog is closed
//    NSLog(@"showModal(): calling CFRunLoopRun()");
    CFRunLoopRun();
}

-(NSString*) getInput {
    [[NSUserDefaults standardUserDefaults] setValue:self.textField.text forKey:kUserNameKey];
    [[NSUserDefaults standardUserDefaults] synchronize];
    
    return self.textField.text;
}

+(InputDialog*) dialog {
    NSArray* nibViews = [[NSBundle mainBundle] loadNibNamed:@"InputDialog"
                                                      owner:nil
                                                    options:nil];
    InputDialog* dlg = [[nibViews objectAtIndex:0] retain];
    
    // Customize the dialog a bit
    dlg.layer.borderColor = [UIColor blackColor].CGColor;
    dlg.layer.borderWidth = 2;
    dlg.layer.cornerRadius = 8;
    dlg.layer.shadowColor = [UIColor blackColor].CGColor;
    dlg.layer.shadowOpacity = 1.0;
    dlg.layer.shadowRadius = 4.0;
    dlg.layer.shadowOffset = CGSizeMake(0, 3);
    dlg.clipsToBounds = NO;
    
    [dlg autorelease];
    
    return dlg;
}

#pragma mark - IBActions

-(IBAction) okPressed {
//    NSLog(@"okPressed");
    
    // Close the dialog by removing it from its superview
    [self removeFromSuperview];
    
    // Close the the nested runloop
//    NSLog(@"calling CFRunLoopStop()");
    CFRunLoopStop(CFRunLoopGetCurrent());
}

#pragma mark - lifecycle etc

-(NSString*) description {
    return @"InputDialog";
}


@end
