/*
 * Copyright (c) 2017 Hackdirt Ltd.
 * Author: David Petrie (david@davidpetrie.com)
 *
 * License: https://www.gnu.org/licenses/gpl-3.0.en.html
 */
#import "Delegate.h"
#include <hdk/hdk.h>
#include "PlatformConfig.h"
#include "Smashed.h"

static hdApplication *shell = NULL;

@implementation AppController

- (void) applicationDidFinishLaunching:(UIApplication*)application
{
    NSLog(@"Version: %d.%d.%d", MAJOR_VERSION, MINOR_VERSION, DISTRIBUTION_VERSION);
    
    application.statusBarHidden = YES;
    application.idleTimerDisabled = YES;

    _lock = [[NSObject alloc] init];
    CGRect rect = [[UIScreen mainScreen] bounds];
    float screenWidth = hdMax(rect.size.width, rect.size.height);
    float screenHeight = hdMin(rect.size.width, rect.size.height);

    // create a full-screen window
    self.window = [[[UIWindow alloc] initWithFrame:CGRectMake(rect.origin.x, rect.origin.y,
                                                              screenWidth, screenHeight)] autorelease];

    _glView = [[hdMultiTouchView alloc] initWithFrame:CGRectMake(rect.origin.x, rect.origin.y,
                                                                 screenWidth, screenHeight)];
    _glView.renderLock = _lock;
    _glView.logOutput = true;
    _glViewController = [[hdMultiTouchViewController alloc] initWithNibName:nil bundle:nil];
    _glViewController.delegate = self;
    _glViewController.touchView = _glView;
    [_glViewController.view addSubview:_glView];

    self.window.rootViewController = _glViewController;
    [self.window makeKeyAndVisible];

    // App screen settings
    float aspectRatio = screenWidth / screenHeight;

    ScreenSettings_SetLandscape();
    ScreenSettings_SetPixelsPerGameUnit(kPixelsPerGameUnit);
    ScreenSettings_SetScreenRect(aspectRatio,
                                 screenWidth,
                                 screenHeight);
    ScreenSettings_SetDefaultCameraRect(screenWidth / kPixelsPerGameUnit, screenHeight / kPixelsPerGameUnit);
    ScreenSettings_SetMinCameraRect(kLandscapeGameMinScreenHeight * aspectRatio,
                                    kLandscapeGameMinScreenHeight);
    ScreenSettings_SetMaxCameraRect(kLandscapeGameMaxScreenHeight * aspectRatio,
                                    kLandscapeGameMaxScreenHeight);
    /*
    ScreenSettings_SetScreenRect(kLandscapePixelAspectRatio,
                                 kLandscapePixelScreenWidth,
                                 kLandscapePixelScreenHeight);
    ScreenSettings_SetDefaultCameraRect(kLandscapeGameScreenWidth,
                                        kLandscapeGameScreenHeight);
    ScreenSettings_SetMinCameraRect(kLandscapeGameMinScreenWidth,
                                    kLandscapeGameMinScreenHeight);
    ScreenSettings_SetMaxCameraRect(kLandscapeGameMaxScreenWidth,
                                    kLandscapeGameMaxScreenHeight);*/

    if([[UIScreen mainScreen] respondsToSelector:@selector(scale)])
    {
        ScreenSettings_SetScaleFactor([[UIScreen mainScreen] scale]);
    }
    else
    {
        ScreenSettings_SetScaleFactor(1.0f);
    }

    if(!shell->InitApplication())
    {
        printf("InitApplication error\n");
    }

    hdSoundManager::InitSoundManager();

    shell->PrepareGameLoop();
    shell->StartGameLoop();
}


- (void) Update {
}


- (void) ReportFPS:(NSNumber *)frames {
}


- (void) Render {
    @synchronized(_lock)
    {
        if (!shell->HandlePlayerInput())
            printf("HandlePlayerInput error\n");

        if(!shell->UpdateScene())
            printf("UpdateScene error\n");

        if(!shell->RenderScene())
            printf("RenderScene error\n");
    }
}


/*
 * Save changes and pause the timer
 */
- (void)applicationWillResignActive:(UIApplication *)application
{
    @synchronized(_lock)
    {
        if(!shell->PauseApplication())
            printf("PauseApplication error\n");
    }
}


/*
 * Un pause timer.
 */
- (void)applicationDidBecomeActive:(UIApplication *)application
{
    @synchronized(_lock)
    {
        if(!shell->ResumeApplication())
            printf("PauseApplication error\n");
    }
}


-(void)applicationDidReceiveMemoryWarning:(UIApplication *)application
{
    printf("Application applicationDidReceiveMemoryWarning \n");

    @synchronized(_lock)
    {
        if(!shell->HandleLowMemoryWarning())
            printf("Handle low mem error\n");
    }
}


/*
 * Save changes only - dealloc will take care of the rest.
 */
-(void)applicationWillTerminate:(UIApplication *)application 
{
    @synchronized(_lock)
    {
        if(!shell->ApplicationWillTerminate())
            printf("ApplicationWillTerminate error\n");
    }
}


- (void) dealloc
{
    @synchronized(_lock)
    {
        if(!shell->QuitApplication())
            printf("QuitApplication error\n");
    }
    
    [_glView release];
    [_window release];
    [_lock release];
    
    [super dealloc];
}

@end
