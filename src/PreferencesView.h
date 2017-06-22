/*
 * Copyright (c) 2017 Hackdirt Ltd.
 * Author: David Petrie (david@davidpetrie.com)
 *
 * License: https://www.gnu.org/licenses/gpl-3.0.en.html
 */

#ifndef HD_PREFERENCES_VIEW_H
#define HD_PREFERENCES_VIEW_H

#include <hdk/hdk.h>

class PreferencesView : hdUIContainer
{
public:
    PreferencesView();

    ~PreferencesView();

    void SetCloseListener(void *obj, void (*func)(void *, void *));


protected:

    static void btnOKCallback(void *handler, void *sender);

    static void btnCancelCallback(void *handler, void *sender);

    static void ToggleMusicCallback(void *handler, void *sender);

    static void ToggleSoundCallback(void *handler, void *sender);

    static void ToggleVibrationCallback(void *handler, void *sender);

    static void ToggleFlipScreenCallback(void *handler, void *sender);

    static void btnCloseMenuUpCallback(void *handler, void *sender);

    void Enable();

    void Disable();


private:

    friend class AppInterface;

    void InitInterface();

    void InitSounds();

    void RefreshLayout(e_hdInterfaceOrientation orientation);

    void SetPreferenceValues();

    hdFontPolygon *m_settingsHeader;

    hdButton *m_btnOK;

    hdButton *m_btnCancel;

    hdUIImage *m_outerChrome;

    hdUIImage *m_innerChrome;

    hdCheckbox *m_btnMenuToggleMusic;

    hdCheckbox *m_btnMenuToggleSound;

    hdCheckbox *m_btnMenuToggleVibration;

    hdCheckbox *m_btnMenuToggleFlipScreen;

    hdButton *m_btnMenuClose;


    /* Sound */
    hdSound *m_btnMenuClickDownSound;
    
    hdSound *m_btnMenuClickUpSound;
    
    hdSound *m_btnMenuClickUpSoundDeep;
    
    
    /* Callbacks */
    void (*m_closePrefsCallback)(void *, void *);
    
    void *m_closePrefsCallbackObject;
    
    void DoClosePrefsCallback();

};

#endif