/*
 * Copyright (c) 2017 Hackdirt Ltd.
 * Author: David Petrie (david@davidpetrie.com)
 *
 * License: https://www.gnu.org/licenses/gpl-3.0.en.html
 */
 
#import <Cocoa/Cocoa.h>
#import "AppMainView.h"
#import "constants.h"
#import "PlatformIncludes.h"

@interface AppController : NSResponder
{
	IBOutlet NSWindow *_glWindow;
	AppMainView*		_glView;
	NSObject*			_lock;
}

- (void) awakeFromNib;
- (void) keyDown:(NSEvent *)theEvent;
- (IBAction) setFullScreen:(id)sender;
- (IBAction) resize:(id)sender;
- (void) dealloc;



@end
