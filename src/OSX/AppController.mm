/*
 * Copyright (c) 2017 Hackdirt Ltd.
 * Author: David Petrie (david@davidpetrie.com)
 *
 * License: https://www.gnu.org/licenses/gpl-3.0.en.html
 */
 
#import "AppController.h"
#import "hdApplication.h"
#include "hdSoundManager.h"

static hdApplication *shell = NULL;
static bool keepRenderThreadAlive = true;

@interface AppController (Private)
- (void)StartApplication;
- (void)RenderThread;
- (void) createFailed;
@end

@implementation AppController

- (void) update
{
	if(!shell->UpdateScene())
		printf("UpdateScene error\n");
	
    if(!shell->RenderScene())
		printf("RenderScene error\n");
}


- (void)RenderThread
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	hdSoundManager::InitSoundManager();
	[[_glView openGLContext] makeCurrentContext];
	shell->PrepareGameLoop();
	
	while (keepRenderThreadAlive)
	{
		@synchronized(_lock)
		{
			shell->HandlePlayerInput();
		}
		
		@synchronized(_lock)
		{		
			if(!shell->UpdateScene())
				printf("UpdateScene error\n");
		}
		
		//if(!shell->HandleResize([_glWindow frame].size.width, [_glWindow frame].size.height))
		//	printf("UpdateScene error\n");
		
		[[_glView openGLContext] makeCurrentContext];
		//[_glView bindFrameBuffer];
		@synchronized(_lock)
		{
			if(!shell->RenderScene())
				printf("RenderScene error\n");
		}
		[ [ _glView openGLContext ] flushBuffer ];
	}
	[pool drain];
}


- (void)StartApplication
{
	shell = new hdApplication();
	
	if(!shell->InitApplication())
		printf("InitApplication error\n");
	
	// create our rendering timer
	//[NSTimer scheduledTimerWithTimeInterval:kFrameDuration target:self selector:@selector(update) userInfo:nil repeats:YES];
	
	[NSThread detachNewThreadSelector:@selector(RenderThread) toTarget:self withObject:nil];
}


- (void) awakeFromNib
{  
	_lock = [[NSObject alloc] init];
	
	[ NSApp setDelegate:self ];   // We want delegate notifications
	
	_glView = [ [ AppMainView alloc ] initWithFrame:[ _glWindow frame ]
										  colorBits:32 depthBits:16 fullscreen:FALSE ];
	if( _glView != nil )
	{
		[ _glWindow setContentView:_glView ];
		[ _glWindow makeKeyAndOrderFront:self ];
		[ self StartApplication ];
	}
	else
		[ self createFailed ];
} 


/*
 * Handle key presses
 */
- (void) keyDown:(NSEvent *)theEvent
{
	unichar unicodeKey;
	
	unicodeKey = [ [ theEvent characters ] characterAtIndex:0 ];
	
	hdPrintf("%c", unicodeKey);
	
	switch( unicodeKey )
	{
	}
} 


- (IBAction) resize:(id)sender
{
	@synchronized(_lock)
	{
		if(!shell->HandleResize([_glWindow frame].size.width, [_glWindow frame].size.height))
			printf("UpdateScene error\n");
	}
}

- (IBAction)setFullScreen:(id)sender
{
}

- (void) createFailed
{
	NSWindow *infoWindow;
	
	infoWindow = NSGetCriticalAlertPanel( @"Initialization failed",
                                         @"Failed to initialize OpenGL",
                                         @"OK", nil, nil );
	[ NSApp runModalForWindow:infoWindow ];
	[ infoWindow close ];
	[ NSApp terminate:self ];
}

- (void) dealloc
{
	keepRenderThreadAlive = false;
	
	if(!shell->QuitApplication())
		printf("QuitApplication error\n");
	
	[_glView release];
	[_glWindow release];
	[_lock release];

	[super dealloc];
}

@end
