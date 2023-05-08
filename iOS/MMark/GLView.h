//
//  GLView.h
//  MMark
//
//  Created by Matti Dahlbom on 22.6.2012.
//  Copyright (c) 2012 Qvik Oy. All rights reserved.
//

#import <UIKit/UIKit.h>

/**
 * Main view controller. Bootstraps the MMark framework and provides
 * the redraws via CADisplayLink.
 *
 * @author Matti Dahlbom
 * @since 0.1
 */
@interface GLView : UIView <UIGestureRecognizerDelegate>

-(BOOL) isBenchmarking;

@end
