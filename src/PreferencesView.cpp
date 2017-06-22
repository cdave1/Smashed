/*
 * Copyright (c) 2017 Hackdirt Ltd.
 * Author: David Petrie (david@davidpetrie.com)
 *
 * License: https://www.gnu.org/licenses/gpl-3.0.en.html
 */

#include "PreferencesView.h"

PreferencesView::PreferencesView() : hdUIContainer(NULL)
{
    InitInterface();
    InitSounds();

    m_closePrefsCallbackObject = NULL;
    m_closePrefsCallback = NULL;
}


PreferencesView::~PreferencesView()
{
    delete m_btnMenuToggleMusic;
    delete m_btnMenuToggleSound;
    delete m_btnMenuClose;
}


void PreferencesView::SetCloseListener(void *obj, void (*func)(void *, void *))
{
    m_closePrefsCallbackObject = obj;
    m_closePrefsCallback = func;
}


void PreferencesView::DoClosePrefsCallback()
{
    if (m_closePrefsCallbackObject != NULL && m_closePrefsCallback != NULL)
    {
        (*m_closePrefsCallback)(m_closePrefsCallbackObject, this);
    }
}


void PreferencesView::InitInterface()
{
    m_outerChrome = new hdUIImage(NULL, NULL);
    m_outerChrome->SetTint(0.0f, 0.0f, 0.0f, 0.6f);

    m_innerChrome = new hdUIImage(NULL, NULL);
    m_innerChrome->SetTint(1.0f, 1.0f, 1.0f, 0.8f);

    // HACK HACK HACK
    //
    // For whatever reason, loading the OK button fails intermittently. This
    // is an attempt to fix the problem without spending time to understand the
    // cause of the problem :p
    hdTextureManager::Instance()->FindTexture("/Interface/Textures/imgWhiteBox.png", TT_16Pic);

    m_btnCancel = new hdButton("/Interface/Textures/btnCancelNormal.png",
                               "/Interface/Textures/btnCancelOver.png",
                               "/Interface/Textures/btnCancelNormal.png",
                               NULL);

    m_btnOK = new hdButton("/Interface/Textures/btnOkNormal.png",
                           "/Interface/Textures/btnOkOver.png",
                           "/Interface/Textures/btnOkNormal.png",
                           NULL);

    m_btnMenuToggleMusic = new hdCheckbox("/Interface/Textures/btnMusicOnNormal.png",
                                          "/Interface/Textures/btnMusicOnOver.png",
                                          "/Interface/Textures/btnMusicOffNormal.png",
                                          "/Interface/Textures/btnMusicOffOver.png",
                                          NULL);

    m_btnMenuToggleSound = new hdCheckbox("/Interface/Textures/btnSoundOnNormal.png",
                                          "/Interface/Textures/btnSoundOnOver.png",
                                          "/Interface/Textures/btnSoundOffNormal.png",
                                          "/Interface/Textures/btnSoundOffOver.png",
                                          NULL);

    m_btnMenuToggleVibration = new hdCheckbox("/Interface/Textures/btnVibrateOnNormal.png",
                                              "/Interface/Textures/btnVibrateOnOver.png",
                                              "/Interface/Textures/btnVibrateOffNormal.png",
                                              "/Interface/Textures/btnVibrateOffOver.png",
                                              NULL);

    m_btnMenuToggleFlipScreen = new hdCheckbox("/Interface/Textures/btnFlipScreenOnNormal.png",
                                               "/Interface/Textures/btnFlipScreenOnOver.png",
                                               "/Interface/Textures/btnFlipScreenOffNormal.png",
                                               "/Interface/Textures/btnFlipScreenOffOver.png",
                                               NULL);

    m_btnMenuClose = new hdButton("/Interface/Textures/btnCloseNormal.png",
                                  "/Interface/Textures/btnCloseOver.png",
                                  "/Interface/Textures/btnCloseNormal.png",
                                  NULL);

    m_settingsHeader = new hdFontPolygon("Fonts/bebas20.f", NULL);
#ifdef IPHONE_BUILD
    m_settingsHeader->SetScale(0.8f);
#endif
    m_settingsHeader->SetTint(0.0f, 0.0f, 0.0f, 1.0f);
    m_settingsHeader->AlignCenter();
    m_settingsHeader->SetText("Settings");

    m_btnOK->SetMouseUpListener(this, PreferencesView::btnOKCallback);
    m_btnCancel->SetMouseUpListener(this, PreferencesView::btnCancelCallback);

    m_btnMenuToggleMusic->AddValueChangedListener(this, PreferencesView::ToggleMusicCallback);
    m_btnMenuToggleSound->AddValueChangedListener(this, PreferencesView::ToggleSoundCallback);
    m_btnMenuToggleVibration->AddValueChangedListener(this, PreferencesView::ToggleVibrationCallback);
    m_btnMenuToggleFlipScreen->AddValueChangedListener(this, PreferencesView::ToggleFlipScreenCallback);

    m_btnMenuClose->SetMouseUpListener(this, PreferencesView::btnCloseMenuUpCallback);

    SetPreferenceValues();

    this->Add(m_outerChrome);
    this->Add(m_innerChrome);
    this->Add(m_btnMenuToggleMusic);
    this->Add(m_btnMenuToggleSound);
#ifdef IPHONE_BUILD
    this->Add(m_btnMenuToggleVibration);
#endif
    this->Add(m_settingsHeader);
    this->Add(m_btnOK);
    this->Add(m_btnCancel);
}


void PreferencesView::InitSounds()
{
    m_btnMenuClickDownSound = hdSoundManager::FindSound("Sounds/btnClickDown.caf", e_soundTypeNormal);
    m_btnMenuClickUpSound = hdSoundManager::FindSound("Sounds/btnClick01.caf", e_soundTypeNormal);
    m_btnMenuClickUpSoundDeep = hdSoundManager::FindSound("Sounds/btnClickBack01.caf", e_soundTypeNormal);
}


void PreferencesView::RefreshLayout(e_hdInterfaceOrientation orientation)
{
    float pixelScreenWidth, pixelScreenHeight;
    float miscButtonHeight;
    float buttonWidth, buttonHeight;
    float xOrigin, yOrigin;
    float distanceFromTop, distanceFromBottom, distanceFromLeft;
    float rowSpacing;

    ScreenSettings_GetScreenRect(&pixelScreenWidth, &pixelScreenHeight);
    buttonWidth = hdClamp(pixelScreenWidth * 0.25f, 160.0f, 320.0f);
    buttonHeight = hdClamp(pixelScreenHeight * 0.18f, 55.0f, 110.f);
    miscButtonHeight = hdClamp(pixelScreenHeight * 0.125f, 40.0f, 100.0f);

    distanceFromTop = 70.0f;
    distanceFromBottom = 40.0f;
    distanceFromLeft = 40.0f;
    rowSpacing = 20.0f;

    m_outerChrome->SetAs2DBox(10.0f, 10.0f, pixelScreenWidth - 20.0f, pixelScreenHeight - 20.0f);
    m_innerChrome->SetAs2DBox(20.0f, 20.0f, pixelScreenWidth - 40.0f, pixelScreenHeight - 40.0f);

    if (orientation == e_hdInterfaceOrientationLandscapeLeft ||
        orientation == e_hdInterfaceOrientationLandscapeRight)
    {
        m_btnMenuClose->SetAs2DBox((pixelScreenWidth - buttonWidth)/2.0f, 275.0f, 50.0f, 35.0f);

        // Column 1
        xOrigin = ((pixelScreenWidth/2.0f) - buttonWidth)/2.0f;
        yOrigin = (pixelScreenHeight - distanceFromTop) - (rowSpacing + (1 * buttonHeight));
        m_btnMenuToggleMusic->SetAs2DBox(xOrigin, yOrigin,
                                         buttonWidth, buttonHeight);

        yOrigin = (pixelScreenHeight - distanceFromTop) - (rowSpacing + (2 * buttonHeight));
        m_btnMenuToggleVibration->SetAs2DBox(xOrigin, yOrigin,
                                             buttonWidth, buttonHeight);

        // Column 2
        xOrigin = (pixelScreenWidth/2.0f) + ((pixelScreenWidth/2.0f) - buttonWidth)/2.0f;
        yOrigin = (pixelScreenHeight - distanceFromTop) - (rowSpacing + (1 * buttonHeight));
        m_btnMenuToggleSound->SetAs2DBox(xOrigin, yOrigin,
                                         buttonWidth, buttonHeight);

        yOrigin = (pixelScreenHeight - distanceFromTop) - (rowSpacing + (2 * buttonHeight));
        m_btnMenuToggleFlipScreen->SetAs2DBox(xOrigin, yOrigin,
                                              buttonWidth, buttonHeight);


        m_btnOK->SetAs2DBox((pixelScreenWidth - distanceFromBottom) - (0.175 * pixelScreenWidth), distanceFromBottom,
                            (0.175 * pixelScreenWidth), miscButtonHeight);

        m_btnCancel->SetAs2DBox(distanceFromLeft, distanceFromBottom,
                                (0.25 * pixelScreenWidth), miscButtonHeight);

        m_settingsHeader->SetAs2DBox(0, pixelScreenHeight - distanceFromTop, pixelScreenWidth, 40.0f);
    }
    else
    {
        m_btnMenuClose->SetAs2DBox((pixelScreenWidth - 160.0f)/2.0f, 345.0f, 50.0f, 35.0f);

        m_btnMenuToggleMusic->SetAs2DBox((pixelScreenWidth - 160.0f)/2.0f, 290.0f, buttonWidth, buttonHeight);
        m_btnMenuToggleSound->SetAs2DBox((pixelScreenWidth - 160.0f)/2.0f, 225.0f, buttonWidth, buttonHeight);
        m_btnMenuToggleVibration->SetAs2DBox((pixelScreenWidth - 160.0f)/2.0f, 160.0f, buttonWidth, buttonHeight);
        m_btnMenuToggleFlipScreen->SetAs2DBox((pixelScreenWidth - 160.0f)/2.0f, 95.0f, buttonWidth, buttonHeight);
    }
}


void PreferencesView::btnOKCallback(void *handler, void *sender)
{
    PreferencesView *self = (PreferencesView *)handler;
    if (!self) return;

    if (self->m_btnMenuToggleMusic->IsOn())
    {
        hdSoundManager::SetMusicMaxVolume(1.0f);
        hdPlayerConfig::SetValue("PlayMusic", "On");
    }
    else
    {
        hdSoundManager::SetMusicMaxVolume(0.0f);
        hdPlayerConfig::SetValue("PlayMusic", "Off");
    }

    if (self->m_btnMenuToggleSound->IsOn())
    {
        hdSoundManager::SetSoundMaxVolume(1.0f);
        hdPlayerConfig::SetValue("PlaySounds", "On");
    }
    else
    {
        hdSoundManager::SetSoundMaxVolume(0.0f);
        hdPlayerConfig::SetValue("PlaySounds", "Off");
    }

    if (self->m_btnMenuToggleVibration->IsOn())
    {
        hdSoundManager::SetVibrationsOn();
        hdPlayerConfig::SetValue("PlayVibrations", "On");
    }
    else
    {
        hdSoundManager::SetVibrationsOff();
        hdPlayerConfig::SetValue("PlayVibrations", "Off");
    }

    if (self->m_btnMenuToggleFlipScreen->IsOn())
    {
        hdPlayerConfig::SetValue("FlipScreen", "On");
    }
    else
    {
        hdPlayerConfig::SetValue("FlipScreen", "Off");
    }

    hdSoundManager::PlaySound(self->m_btnMenuClickUpSoundDeep);
    self->DoClosePrefsCallback();
}


void PreferencesView::btnCancelCallback(void *handler, void *sender)
{
    PreferencesView *self = (PreferencesView *)handler;
    if (!self) return;

    hdSoundManager::PlaySound(self->m_btnMenuClickUpSoundDeep);
    self->DoClosePrefsCallback();
}


void PreferencesView::ToggleMusicCallback(void *handler, void *sender)
{
    PreferencesView *self = (PreferencesView *)handler;
    if (!self) return;

    hdSoundManager::PlaySound(self->m_btnMenuClickUpSound);
}


void PreferencesView::ToggleSoundCallback(void *handler, void *sender)
{
    PreferencesView *self = (PreferencesView *)handler;
    if (!self) return;

    hdSoundManager::PlaySound(self->m_btnMenuClickUpSound);
}


void PreferencesView::ToggleVibrationCallback(void *handler, void *sender)
{
    PreferencesView *self = (PreferencesView *)handler;
    if (!self) return;

    hdSoundManager::PlaySound(self->m_btnMenuClickUpSound);
}


void PreferencesView::ToggleFlipScreenCallback(void *handler, void *sender)
{
    PreferencesView *self = (PreferencesView *)handler;
    if (!self) return;
    
    hdSoundManager::PlaySound(self->m_btnMenuClickUpSound);
}


void PreferencesView::btnCloseMenuUpCallback(void *handler, void *sender)
{
    PreferencesView *self = (PreferencesView *)handler;
    if (!self) return;
    
    hdSoundManager::PlaySound(self->m_btnMenuClickUpSoundDeep);
    self->DoClosePrefsCallback();
}


void PreferencesView::SetPreferenceValues()
{
    if (hdPlayerConfig::GetValue("PlayMusic") == "On")
        m_btnMenuToggleMusic->SetOn();
    else
        m_btnMenuToggleMusic->SetOff();
    
    if (hdPlayerConfig::GetValue("PlaySounds") == "On")
        m_btnMenuToggleSound->SetOn();
    else
        m_btnMenuToggleSound->SetOff();
    
    if (hdPlayerConfig::GetValue("PlayVibrations") == "On")
        m_btnMenuToggleVibration->SetOn();
    else
        m_btnMenuToggleVibration->SetOff();
    
    if (hdPlayerConfig::GetValue("FlipScreen") == "On")
        m_btnMenuToggleFlipScreen->SetOn();
    else
        m_btnMenuToggleFlipScreen->SetOff();
}


void PreferencesView::Enable()
{
    m_btnMenuClose->Enable();
    m_btnMenuToggleMusic->Enable();
    m_btnMenuToggleSound->Enable();
    m_btnMenuToggleVibration->Enable();
    m_btnMenuToggleFlipScreen->Enable();
    m_btnMenuClose->Enable();
    
    SetPreferenceValues();
}


void PreferencesView::Disable()
{
    m_btnMenuClose->Disable();
    m_btnMenuToggleMusic->Disable();
    m_btnMenuToggleSound->Disable();
    m_btnMenuToggleVibration->Disable();
    m_btnMenuToggleFlipScreen->Disable();
    m_btnMenuClose->Disable();
}
