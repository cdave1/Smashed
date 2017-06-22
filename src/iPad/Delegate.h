/*
 * Copyright (c) 2017 Hackdirt Ltd.
 * Author: David Petrie (david@davidpetrie.com)
 *
 * License: https://www.gnu.org/licenses/gpl-3.0.en.html
 */

#import <hdk/hdInput/hdMultiTouchView.h>
#import <hdk/hdInput/hdMultiTouchViewController.h>

@interface AppController : NSObject <GLKViewControllerDelegate>
{
	hdMultiTouchView *_glView;
    hdMultiTouchViewController *_glViewController;
	NSObject *_lock;
}

@property (strong, nonatomic) UIWindow *window;

- (void) Update;
- (void) ReportFPS:(NSNumber *)frames;
- (void) Render;

@end 