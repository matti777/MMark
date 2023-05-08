//
//  InputDialog.h
//  ModalDialogTest
//
//  Created by Matti Dahlbom on 2.10.201240.
//  Copyright (c) 2012 Qvik. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface InputDialog : UIView

/** Creates a new dialog. */
+(InputDialog*) dialog;

/** Displays the dialog in a modal fashion. */
-(void) showModal;

/** Returns the input value. */
-(NSString*) getInput;

@end
