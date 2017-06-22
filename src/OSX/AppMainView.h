/*
 * Copyright (c) 2017 Hackdirt Ltd.
 * Author: David Petrie (david@davidpetrie.com)
 *
 * License: https://www.gnu.org/licenses/gpl-3.0.en.html
 */

#import <Cocoa/Cocoa.h>
#include "hdMouseInput.h"

@interface AppMainView : NSOpenGLView
{
	int colorBits, depthBits;
	BOOL runningFullScreen;
	NSDictionary *originalDisplayMode;
}

- (id) initWithFrame:(NSRect)frame colorBits:(int)numColorBits
		   depthBits:(int)numDepthBits fullscreen:(BOOL)runFullScreen;
- (void) reshape;
- (void) drawRect:(NSRect)rect;
- (BOOL) isFullScreen;
- (BOOL) setFullScreen:(BOOL)enableFS inFrame:(NSRect)frame;
- (void) dealloc;
- (mousevalues_t)GetMouseValues;
@end
