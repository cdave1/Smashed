/*
 * Copyright (c) 2017 Hackdirt Ltd.
 * Author: David Petrie (david@davidpetrie.com)
 *
 * License: https://www.gnu.org/licenses/gpl-3.0.en.html
 */

#ifndef APP_CALLBACK_FUNCTIONS_H_
#define APP_CALLBACK_FUNCTIONS_H_

#undef BOOST_HAS_INT128

#include <boost/thread.hpp>

#include "AppController.h"

class AppCallbackFunctions
{
public:

    static void ShowLevelControllerCallback(void * object, hdAnimation * anim);

    static void IntroFinishedCallback(void * object, hdAnimation * anim);

    static void LevelMenuFinishedCallback(void * object, hdAnimation * anim);

    static void StoppedCurrentLevelCallback(void * object, hdAnimation * anim);

    static void RestartCurrentLevelCallback(void * object, hdAnimation * anim);

    static void ShowLevelPickerControllerCallback(void * object, hdAnimation * anim);

    static void LevelPickerLevelClickedCallback(void *object, hdAnimation *anim);

    static void LevelPickerBackCallback(void *object, hdAnimation *anim);

    static void QuitCurrentLevelCallback(void *object, hdAnimation *anim);

protected:

    friend class hdAnimation;

    static void IntroFinished_Thread(void * object, hdAnimation * anim);

    static void StoppedCurrentLevel_Thread(void * object, hdAnimation * anim);

    static void ShowLevelPickerController_Thread(void * object, hdAnimation * anim);

    static void LevelPickerLevelClicked_Thread(void *object, hdAnimation *anim);

    static void LevelPickerBack_Thread(void *object, hdAnimation *anim);

    static void QuitCurrentLevel_Thread(void *object, hdAnimation *anim);
    
    static void DoLoadingScreenTransition(void (*func)(void *, hdAnimation *), void * object);

};

#endif
