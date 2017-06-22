/*
 * Copyright (c) 2017 Hackdirt Ltd.
 * Author: David Petrie (david@davidpetrie.com)
 *
 * License: https://www.gnu.org/licenses/gpl-3.0.en.html
 */

#ifndef INTRO_CONTROLLER_H
#define INTRO_CONTROLLER_H

#include "AppCallbackFunctions.h"

class IntroController : public hdInterfaceController
{
public:
    IntroController(const hdInterfaceController* parentController);

    ~IntroController();

    void Step(double interval);

    void Draw();

    void AnimateHide();

    void HandleTapUp(float x, float y, int tapCount);

    static void AnimationFinishedHandler(void* object, hdAnimation *animation);

private:
    hdTexture* m_logo;

    hdUIImage *m_logoImage;

    hdUIImage *m_fade;

    hdAlphaAction *m_fadeAction;

    hdAnimation *m_fadeAnim;

    hdSound *m_introSound;

};

#endif