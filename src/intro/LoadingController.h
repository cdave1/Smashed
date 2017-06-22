/*
 * Copyright (c) 2017 Hackdirt Ltd.
 * Author: David Petrie (david@davidpetrie.com)
 *
 * License: https://www.gnu.org/licenses/gpl-3.0.en.html
 */

#ifndef LOADING_CONTROLLER_H
#define LOADING_CONTROLLER_H

#include <hdk/hdk.h>
#include "AppCallbackFunctions.h"

class LoadingController : public hdInterfaceController
{
public:

    LoadingController(const hdInterfaceController* parentController);

    ~LoadingController();

    void Reset();

    void Step(double interval);

    void Draw();

    void AnimateShow();


private:

    hdUIImage *m_loadingImage;

    hdAlphaAction *m_fadeAction;
    
    hdAnimation *m_fadeAnim;
    
    float longSide;
    
    float shortSide;

};

#endif