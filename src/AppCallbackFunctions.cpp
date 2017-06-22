/*
 * Copyright (c) 2017 Hackdirt Ltd.
 * Author: David Petrie (david@davidpetrie.com)
 *
 * License: https://www.gnu.org/licenses/gpl-3.0.en.html
 */

#include "AppCallbackFunctions.h"

/*
 * Static callback functions
 */

static AppController *currentController = NULL;

void AppCallbackFunctions::DoLoadingScreenTransition(void (*func)(void *, hdAnimation *), void * object)
{
    AppController *self = (AppController *)object;
    self->SetLoading();

    hdPauseAction *pause = new hdPauseAction();
    pause->SetDuration(0.05f);

    hdAnimation *pauseAnim = hdAnimationController::CreateAnimation(self);
    pauseAnim->AddAction(pause);
    pauseAnim->SetFinishedCallback(self, func);
    pauseAnim->StartAnimation();
}


void AppCallbackFunctions::IntroFinishedCallback(void * object, hdAnimation * anim)
{
#if 1
    anim->RemoveCallbackFunctions();
    AppController *self = (AppController *)object;
    DoLoadingScreenTransition(AppCallbackFunctions::IntroFinished_Thread, self);

#else
    currentController = (AppController *)object;
    currentController->SetLoading();
    boost::thread thrd(boost::ref(AppCallbackFunctions::IntroFinished_Thread), boost::ref(currentController));
    thrd.join();
#endif
}


void AppCallbackFunctions::IntroFinished_Thread(void * object, hdAnimation * anim)
{
    anim->RemoveCallbackFunctions();
    AppController *self = (AppController *)object;
    self->IntroFinished();
}


/*
 * Player finishes, goes back to level picker.
 */
void AppCallbackFunctions::StoppedCurrentLevelCallback(void * object, hdAnimation * anim)
{
    anim->RemoveCallbackFunctions();
    DoLoadingScreenTransition(AppCallbackFunctions::StoppedCurrentLevel_Thread, ((AppController *)object));
}


void AppCallbackFunctions::StoppedCurrentLevel_Thread(void * object, hdAnimation * anim)
{
    ((AppController *)object)->GameControllerStopCurrentLevel();
}


/*
 * NO LOADING SCREEN when player restarts the level, even though it goes through controller.
 */
void AppCallbackFunctions::RestartCurrentLevelCallback(void * object, hdAnimation * anim)
{
    anim->RemoveCallbackFunctions();
    ((AppController *)object)->GameControllerRestartCurrentLevel(); // No loading screen
}


/*
 * Transition from Game controller to level picker controller.
 */
void AppCallbackFunctions::ShowLevelPickerControllerCallback(void * object, hdAnimation * anim)
{
    anim->RemoveCallbackFunctions();
    AppController *self = (AppController *)object;
    DoLoadingScreenTransition(AppCallbackFunctions::ShowLevelPickerController_Thread, self);
}


void AppCallbackFunctions::ShowLevelPickerController_Thread(void * object, hdAnimation * anim)
{
    ((AppController *)object)->ShowLevelPickerController();
}


/*
 * Transition when player clicks on a level picker level
 */
void AppCallbackFunctions::LevelPickerLevelClickedCallback(void *object, hdAnimation *anim)
{
    anim->RemoveCallbackFunctions();
    AppController *self = (AppController *)object;
    DoLoadingScreenTransition(AppCallbackFunctions::LevelPickerLevelClicked_Thread, self);
}


void AppCallbackFunctions::LevelPickerLevelClicked_Thread(void * object, hdAnimation * anim)
{
    ((AppController *)object)->LevelPickerControllerLevelClicked();
}


/*
 * From lev picker back to menu
 */
void AppCallbackFunctions::LevelPickerBackCallback(void *object, hdAnimation *anim)
{
    anim->RemoveCallbackFunctions();
    DoLoadingScreenTransition(AppCallbackFunctions::LevelPickerBack_Thread, ((AppController *)object));
}


void AppCallbackFunctions::LevelPickerBack_Thread(void *object, hdAnimation *anim)
{
    ((AppController *)object)->LevelPickerBackClicked();
}


/*
 * Game controller quitting back to level picker
 */
void AppCallbackFunctions::QuitCurrentLevelCallback(void *object, hdAnimation *anim)
{
    anim->RemoveCallbackFunctions();
    DoLoadingScreenTransition(AppCallbackFunctions::QuitCurrentLevel_Thread, ((AppController *)object));
}


void AppCallbackFunctions::QuitCurrentLevel_Thread(void *object, hdAnimation *anim)
{
    ((AppController *)object)->GameControllerQuitCurrentLevel();
}

