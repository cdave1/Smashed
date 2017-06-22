/*
 * Copyright (c) 2017 Hackdirt Ltd.
 * Author: David Petrie (david@davidpetrie.com)
 *
 * License: https://www.gnu.org/licenses/gpl-3.0.en.html
 */

#include <stdio.h>
#include <sys/time.h>

#include <hdk/input.h>

#include "AppController.h"

#define kAccelerometerFrequency		20.0 //Hz

typedef struct
{
    int frames;
    CFTimeInterval	LastTimeInterval;
    CFTimeInterval	LastFrameTime;
    CFTimeInterval	CurrentTime;
    CFTimeInterval	LastFPSUpdateTime;
    CFTimeInterval	TimeSinceLastFrame;
    bool isPrintable;
    int totalFrames;
    int totalSecs;
} timerValues_t;

bool hdApplicationActive;

bool synchronizedUpdates;
float frameDuration;

bool threadSleepingAllowed;
short sleepDuration;

AppController * controller;

#if TARGET_IPHONE_SIMULATOR  || TARGET_OS_IPHONE
TouchScreenValues *TouchScreen;
int CountTouchesBegan;
int CountTouchesMoved;
Accel *gAccel;
#else
mousevalues_t MouseValues;
hdUIImage *mousePoint;
#endif

static timerValues_t m_handleInputTimerValues;
static timerValues_t m_updateSceneTimerValues;
static timerValues_t m_renderSceneTimerValues;

void HandleUserInput();
void RenderSceneInternal();
void InitTimerValues(timerValues_t& timerValues);
void UpdateTimerValues(timerValues_t& timerValues);



bool hdApplication::InitApplication()
{
    char fsRootPath[512];

    SystemSettings_Init();
    
    hdglClearBuffers();

#if TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE
    gAccel = [Accel alloc];
    [gAccel SetupAccelerometer: kAccelerometerFrequency];
#endif

    // Root file path.
    snprintf(fsRootPath, 512, "%s/BaseDir/", [[[NSBundle mainBundle] resourcePath] cStringUsingEncoding:NSASCIIStringEncoding]);

    FileSystem_Init(fsRootPath);

    /*
     * script files loader
     */
    Scripts_Init();

    /*
     * Strings loader
     */
    Scripts_LoadStringsFile(STRINGS_FILE_PATH);

    printf("Running arch: %s\n", SystemSettings_SystemArchName());

#if TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE
    /*
     * Config file loader
     */
    if (0 == strcmp(SystemSettings_SystemName(), "iPhone1,1") ||
        0 == strcmp(SystemSettings_SystemName(), "iPod1,1"))
    {
        hdConfig::LoadConfigFile("Config/iPhone_1g.config");
    }
    else if  (0 == strcmp(SystemSettings_SystemName(), "iPhone1,2") ||
              0 == strcmp(SystemSettings_SystemName(), "iPod1,2") ||
              0 == strcmp(SystemSettings_SystemName(), "iPod2,1"))
    {
        hdConfig::LoadConfigFile("Config/iPhone_2g.config");
    }
    else if (0 == strcmp(SystemSettings_SystemName(), "iPhone2,1"))
    {
        hdConfig::LoadConfigFile("Config/iPhone_3gs.config");
    }
    else if (0 == strcmp(SystemSettings_SystemName(), "iPod3,1"))
    {
        hdConfig::LoadConfigFile("Config/iPod_31.config");
    }
    else if (0 == strcmp(SystemSettings_SystemName(), "i386")) // SIM
    {
        hdConfig::LoadConfigFile("Config/iPhone_Sim.config");
    }
    else
    {
        hdConfig::LoadConfigFile("Config/iPhone_3gs.config");
    }
#else
    hdConfig::LoadConfigFile("Config/osx.config");

    mousePoint = new hdUIImage("Interface/Textures/mousePoint.png", NULL);
#endif

    frameDuration = strtof(hdConfig::GetValueForKey(CONFIG_MINFRAMEDURATION_KEY).c_str(), NULL);
    frameDuration = hdClamp(frameDuration, 0.016f, 0.05f);

    synchronizedUpdates = true;
    sleepDuration = 0;
    threadSleepingAllowed = false;

    return true;
}


bool hdApplication::PrepareGameLoop()
{
    InitTimerValues(m_handleInputTimerValues);
    InitTimerValues(m_updateSceneTimerValues);
    InitTimerValues(m_renderSceneTimerValues);

    m_renderSceneTimerValues.isPrintable = true;

    controller = new AppController();
    return true;
}


bool hdApplication::StartGameLoop()
{
    // Get the initial accelerometer position so we know what orientation to
    // start with. It seems that at app startup, neither the device nor the
    // statusBarOrientation can be used for this.
    double AccelerometerVector[3];
    [gAccel GetAccelerometerVector:(double *) AccelerometerVector];

    if (AccelerometerVector[0] < 0.0f)
    {
        controller->SetOrientation(e_hdInterfaceOrientationLandscapeRight);
        hdPlayerConfig::SetValue("Orientation", "e_hdInterfaceOrientationLandscapeRight");
        [[UIApplication sharedApplication] setStatusBarOrientation:UIInterfaceOrientationLandscapeRight animated:NO];
    }
    else
    {
        controller->SetOrientation(e_hdInterfaceOrientationLandscapeLeft);
        hdPlayerConfig::SetValue("Orientation", "e_hdInterfaceOrientationLandscapeLeft");
        [[UIApplication sharedApplication] setStatusBarOrientation:UIInterfaceOrientationLandscapeLeft animated:NO];
    }
    hdInitImmediateModeGL();
    controller->StartApp();
    return true;
}


bool hdApplication::QuitApplication()
{
    if (controller != NULL)
    {
        delete controller;
    }
    controller = NULL;

    return true;
}


bool hdApplication::PauseApplication()
{
    // save changes
    if (controller == NULL) return false;
    controller->SaveGameState();
    hdApplicationActive = false;
    return true;
}


bool hdApplication::ResumeApplication()
{
    hdApplicationActive = true;
    return true;
}


bool hdApplication::HandleLowMemoryWarning()
{
    if (controller == NULL) return false;
    controller->SaveGameState();

    controller->FreeIdleMemory();

    return true;
}


bool hdApplication::ApplicationWillTerminate()
{
    // save changes
    if (controller == NULL) return false;
    controller->SaveGameState();

    if (controller != NULL)
    {
        printf("Deleting main controller\n");
        delete controller;
        controller = NULL;
    }


    return true;
}


void InitTimerValues(timerValues_t& timerValues)
{
    timerValues.frames = 0;
    timerValues.LastTimeInterval = 0;
    timerValues.LastFrameTime = 0;
    timerValues.CurrentTime = 0;
    timerValues.LastFPSUpdateTime = 0;
    timerValues.TimeSinceLastFrame = 0;
    timerValues.isPrintable = false;
    timerValues.totalFrames = 0;
    timerValues.totalSecs = 0;
}


void UpdateTimerValues(timerValues_t& timerValues)
{
#if TARGET_IPHONE_SIMULATOR == 1 || TARGET_OS_IPHONE == 1
    /*
     * CACurrentMediaTime was recommended by an Apple engineer
     * for thread and game loop timing.
     */
    timerValues.CurrentTime = CACurrentMediaTime();
#else
    timerValues.CurrentTime = CFAbsoluteTimeGetCurrent();
#endif

    if (timerValues.LastFPSUpdateTime == 0)
        timerValues.LastFPSUpdateTime = timerValues.CurrentTime;

    if (timerValues.LastFrameTime == 0)
        timerValues.LastFrameTime = timerValues.CurrentTime;

    if ((timerValues.CurrentTime - timerValues.LastFPSUpdateTime) > 1.0f)
    {
#ifdef DEBUG
        if (timerValues.isPrintable)
        {
            printf("fps: %d\n", timerValues.frames);
            timerValues.totalFrames += timerValues.frames;
            ++timerValues.totalSecs;
            if (timerValues.totalSecs == 10)
            {
                printf("Frame rate: %d\n", timerValues.totalFrames/timerValues.totalSecs);
                timerValues.totalSecs =0;
                timerValues.totalFrames = 0;
            }
        }
#endif

        timerValues.frames = 0;
        timerValues.LastFPSUpdateTime = timerValues.CurrentTime;
    }

    timerValues.TimeSinceLastFrame = timerValues.CurrentTime - timerValues.LastFrameTime;
}


bool hdApplication::HandlePlayerInput()
{
    UpdateTimerValues(m_handleInputTimerValues);

    if (!hdApplicationActive) return true;

    if (synchronizedUpdates)
    {
        m_handleInputTimerValues.frames++;
        m_handleInputTimerValues.LastFrameTime = m_handleInputTimerValues.CurrentTime;
        HandleUserInput();
    }
    else
    {
        if (m_handleInputTimerValues.TimeSinceLastFrame >= (frameDuration)) // - 0.005f))
        {
            m_handleInputTimerValues.frames++;
            m_handleInputTimerValues.LastFrameTime = m_handleInputTimerValues.CurrentTime;

            HandleUserInput();
        }
        else
        {
            if (threadSleepingAllowed)
                usleep(((frameDuration) - m_handleInputTimerValues.TimeSinceLastFrame) * sleepDuration);
        }
    }
    return true;
}


bool hdApplication::UpdateScene()
{
    UpdateTimerValues(m_updateSceneTimerValues);

    if (!hdApplicationActive) return true;

    if  (synchronizedUpdates)
    {
        m_updateSceneTimerValues.frames++;
        m_updateSceneTimerValues.LastFrameTime = m_updateSceneTimerValues.CurrentTime;
        controller->Step(m_updateSceneTimerValues.TimeSinceLastFrame, frameDuration);
    }
    else
    {
        if (m_updateSceneTimerValues.TimeSinceLastFrame >= frameDuration)
        {
            m_updateSceneTimerValues.frames++;
            m_updateSceneTimerValues.LastFrameTime = m_updateSceneTimerValues.CurrentTime;
            controller->Step(m_updateSceneTimerValues.TimeSinceLastFrame, frameDuration);
        }
        else
        {
            if (threadSleepingAllowed)
                usleep((frameDuration - m_updateSceneTimerValues.TimeSinceLastFrame) * sleepDuration);
        }
    }

    return true;
}


void HandleUserInput()
{
#if TARGET_IPHONE_SIMULATOR == 1 || TARGET_OS_IPHONE == 1
    TouchScreen = GetValuesMultiTouchScreen();

    int TouchCount = GetTouchCount() + 0;

    if (TouchScreen == nil) return;

#ifdef DEBUG_INPUT
    if (TouchCount > 0)
        hdPrintf("Handling touches\n");
#endif

    if (TouchCount == 1)
    {
        if (TouchScreen->TouchDown)
        {
#ifdef DEBUG_INPUT
            hdPrintf("HandleTapDown: %d, %d\n", TouchScreen->LocationXTouchBegan, TouchScreen->LocationYTouchBegan);
#endif
            controller->HandleTapDown(TouchScreen->LocationXTouchBegan, TouchScreen->LocationYTouchBegan, TouchScreen->TapCount);
        }

        if (TouchScreen->TouchMoved)
        {
#ifdef DEBUG_INPUT
            hdPrintf("HandleTapMoved: %d, %d\n", TouchScreen->LocationXTouchMoved, TouchScreen->LocationYTouchMoved);
#endif
            controller->HandleTapMovedSingle(TouchScreen[0].LocationXTouchMovedPrevious,
                                             TouchScreen[0].LocationYTouchMovedPrevious,
                                             TouchScreen[0].LocationXTouchMoved,
                                             TouchScreen[0].LocationYTouchMoved);
        }

        if (TouchScreen->TouchUp)
        {
#ifdef DEBUG_INPUT
            hdPrintf("HandleTapUp: %d, %d\n", TouchScreen->LocationXTouchEnded, TouchScreen->LocationYTouchEnded);
#endif
            controller->HandleTapUp(TouchScreen->LocationXTouchEnded, TouchScreen->LocationYTouchEnded, TouchScreen->TapCount);

            if (TouchScreen->TapCount == 1)
            {
                controller->HandleSingleTap(TouchScreen->LocationXTouchEnded, TouchScreen->LocationYTouchEnded);
            }
            else if (TouchScreen->TapCount == 2)
            {
                controller->HandleDoubleTap(TouchScreen->LocationXTouchEnded, TouchScreen->LocationYTouchEnded);
            }
        }
    }
    else if (TouchCount == 2)
    {
        controller->HandleTapMovedDouble(TouchScreen[0].LocationXTouchMovedPrevious,
                                         TouchScreen[0].LocationYTouchMovedPrevious,
                                         TouchScreen[0].LocationXTouchMoved,
                                         TouchScreen[0].LocationYTouchMoved,
                                         TouchScreen[1].LocationXTouchMovedPrevious,
                                         TouchScreen[1].LocationYTouchMovedPrevious,
                                         TouchScreen[1].LocationXTouchMoved,
                                         TouchScreen[1].LocationYTouchMoved);
    }
#else
    MouseValues = GetMouseValues();

    if (MouseValues.leftButton.isDown)
    {
        controller->HandleTapDown(MouseValues.leftButton.xClickDown,
                                  MouseValues.leftButton.yClickDown,
                                  1);
    }
    if (MouseValues.leftButton.isMoving)
    {
        controller->HandleTapMovedSingle(MouseValues.xPrevious,
                                         MouseValues.yPrevious,
                                         MouseValues.xCurrent,
                                         MouseValues.yCurrent);
    }
    if (MouseValues.leftButton.isUp)
    {
        controller->HandleTapUp(MouseValues.leftButton.xClickUp,
                                MouseValues.leftButton.yClickUp,
                                1);

        if (MouseValues.leftButton.clickCount == 1)
        {
            controller->HandleSingleTap(MouseValues.leftButton.xClickDown,
                                        MouseValues.leftButton.yClickDown);
        }
        else if (MouseValues.leftButton.clickCount == 2)
        {
            controller->HandleDoubleTap(MouseValues.leftButton.xClickDown,
                                        MouseValues.leftButton.yClickDown);
        }
    }

    if (MouseValues.scrollWheel.isMoving)
    {
        if (MouseValues.scrollWheel.yClickDown > 0)
        {
            controller->HandleTapMovedDouble(0, 0, 0, 0,
                                             0, 0,
                                             MouseValues.scrollWheel.xClickDown,
                                             3.0f * MouseValues.scrollWheel.yClickDown);
        }
        else
        {
            controller->HandleTapMovedDouble(0, 0, 0, 0,
                                             MouseValues.scrollWheel.xClickDown,
                                             3.0f * MouseValues.scrollWheel.yClickDown,
                                             0, 0);
        }
    }

#endif
}

#if 1
hdUIImage *touchPointImage = NULL;
#define kTouchHitsRevolvingCount 256
typedef struct touchHit
{
    float x;
    float y;
    float age;
    bool on;
}
touchHit_t;

static touchHit_t touchHits[kTouchHitsRevolvingCount];
static uint32 touchHitCount = 0;
#endif


bool hdApplication::RenderScene()
{
    UpdateTimerValues(m_renderSceneTimerValues);

    if (!hdApplicationActive) return true;

    if  (synchronizedUpdates)
    {
        m_renderSceneTimerValues.frames++;
        m_renderSceneTimerValues.LastFrameTime = m_renderSceneTimerValues.CurrentTime;
        RenderSceneInternal();
    }
    else
    {
        if (m_renderSceneTimerValues.TimeSinceLastFrame >= frameDuration)
        {
            m_renderSceneTimerValues.frames++;
            m_renderSceneTimerValues.LastFrameTime = m_renderSceneTimerValues.CurrentTime;
            RenderSceneInternal();
        }
        else
        {
            if (threadSleepingAllowed)
                usleep((frameDuration - m_renderSceneTimerValues.TimeSinceLastFrame) * sleepDuration);
        }
    }
    return true;
}


void RenderSceneInternal()
{
    hdglClearBuffers();

#if TARGET_GL_OPENGL == 1
    glEnable(GL_LINE_SMOOTH);
#endif

    controller->Draw();

#if 0
    /*
     * Draw lines indicating touch points.
     */
    if (TouchScreen != NULL)
    {
        for (int i = 0; i < kMultiTouchScreenValues; i++)
        {
            if (TouchScreen[i].TouchMoved)
            {

                touchHits[touchHitCount].x = TouchScreen[i].LocationXTouchMoved;
                touchHits[touchHitCount].y = TouchScreen[i].LocationYTouchMoved;
                touchHits[touchHitCount].on = true;
                touchHits[touchHitCount].age = 1.0f;
                touchHitCount = (touchHitCount + 1) % kTouchHitsRevolvingCount;
            }

            if (TouchScreen[i].TouchDown)
            {
                touchHits[touchHitCount].x = TouchScreen[i].LocationXTouchBegan;
                touchHits[touchHitCount].y = TouchScreen[i].LocationYTouchBegan;
                touchHits[touchHitCount].on = true;
                touchHits[touchHitCount].age = 1.0f;
                touchHitCount = (touchHitCount + 1) % kTouchHitsRevolvingCount;
            }

            if (TouchScreen[i].TouchUp)
            {
                touchHits[touchHitCount].x = TouchScreen[i].LocationXTouchEnded;
                touchHits[touchHitCount].y = TouchScreen[i].LocationYTouchEnded;
                touchHits[touchHitCount].on = true;
                touchHits[touchHitCount].age = 1.0f;
                touchHitCount = (touchHitCount + 1) % kTouchHitsRevolvingCount;
            }
        }
    }
    
    if (!touchPointImage)
    {
        touchPointImage = new hdUIImage("Interface/Textures/mousePoint.png", NULL);
    }
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glPushMatrix();
    hdglOrthof(0, 320, 0, 480, 0, -100);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    for (int i = 0; i < kTouchHitsRevolvingCount; ++i)
    {
        if (touchHits[i].on)
        {
            touchPointImage->SetAs2DBox(touchHits[i].x - 4.0f,
                                        480.0f - (touchHits[i].y - 4.0f),
                                        8.0f, 8.0f);
            touchPointImage->SetAlpha(touchHits[i].age);
            touchPointImage->Draw();
            
            touchHits[i].age -= 0.02f;
        }
        
        if (touchHits[i].age <= 0.0f)
        {
            touchHits[i].on = false;
            touchHits[i].age = 0.0f;
        }
    }
    
    glDisable(GL_BLEND);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
#endif
    
    
#ifdef DEBUG
    
#if TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE
    if (TouchScreen != NULL)
    {
        for (int i = 0; i < kMultiTouchScreenValues; i++)
        {
#ifdef DEBUG_INPUT
            if (TouchScreen[i].TouchDown)
            {
                
                hdPrintf("%d: touch: X: %3.2f Y: %3.2f Tap Count: %d\n", 
                         i + 1,
                         TouchScreen[i].LocationXTouchBegan, 
                         TouchScreen[i].LocationYTouchBegan,
                         TouchScreen[i].TapCount);
            } 
            
            if (TouchScreen[i].TouchMoved)
            {
                hdPrintf("\t\tmoved: X: %3.2f Y: %3.2f Tap Count: %d\n", 
                         TouchScreen[i].LocationXTouchMoved, 
                         TouchScreen[i].LocationYTouchMoved,
                         TouchScreen[i].TapCount);
            }
#endif
            
            touchHits[touchHitCount].x = TouchScreen[i].LocationXTouchMoved;
            touchHits[touchHitCount].y = TouchScreen[i].LocationYTouchMoved;
            touchHits[touchHitCount].on = true;
            touchHits[touchHitCount].age = 1.0f;
            touchHitCount = (touchHitCount+ 1) & (touchHitCount - 1);
        }
    }
    
#else
#ifdef DEBUG
    mousePoint->SetTexture(hdTextureManager::Instance()->FindTexture("Interface/mousePoint.png", TT_16Pic));
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glPushMatrix();
    hdglOrthof(0, 480, 0, 320, 0, -100);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    if (MouseValues.leftButton.isDown)
    {
        mousePoint->SetAs2DBox(MouseValues.leftButton.xClickDown - 32.0f,
                               MouseValues.leftButton.yClickDown - 32.0f,
                               64.0f, 64.0f);
        mousePoint->Draw();
    }
    if (MouseValues.leftButton.isMoving)
    {
        mousePoint->SetAs2DBox(MouseValues.xCurrent - 32.0f,
                               MouseValues.yCurrent - 32.0f,
                               64.0f, 64.0f);
        mousePoint->Draw();
    }
    if (MouseValues.leftButton.isUp)
    {
        mousePoint->SetAs2DBox(MouseValues.leftButton.xClickUp - 32.0f,
                               MouseValues.leftButton.yClickUp - 32.0f,
                               64.0f, 64.0f);
        mousePoint->Draw();
    }
    glDisable(GL_BLEND);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
#endif
#endif
#endif
}


bool hdApplication::HandleResize(const float newWidth, const float newHeight)
{
    if (controller == NULL) return false;
    return true;
}
