/*
 * Copyright (c) 2017 Hackdirt Ltd.
 * Author: David Petrie (david@davidpetrie.com)
 *
 * License: https://www.gnu.org/licenses/gpl-3.0.en.html
 */

#include "IntroController.h"

IntroController::IntroController(const hdInterfaceController* parentController) : hdInterfaceController(parentController->GetOrientation(), parentController)
{
    hdVec3 origin(0,0,0);
    hdVec3 diff = origin - m_projection->GetAABB().lower;
    m_projection->SetAABB(m_projection->GetAABB().lower + diff, m_projection->GetAABB().upper);

    m_fadeAction = new hdAlphaAction();
    m_fadeAction->SetDuration(0.5f);
    m_fadeAction->SetStartAlpha(1.0f);
    m_fadeAction->SetEndAlpha(0.0f);

    m_fadeAnim = hdAnimationController::CreateAnimation(this, false);
    m_fadeAnim->AddGameObject(m_fadeAction->GetAlphaObject());

    m_fadeAnim->AddAction(m_fadeAction);

    m_fadeAnim->SetFinishedCallback(this, AnimationFinishedHandler);
    m_fadeAnim->StartAnimation();

    m_introSound = hdSoundManager::FindSound("Sounds/introHackdirt.caf", e_soundTypeFrequent);
    m_introSound->volume = 0.6f;
    hdSoundManager::PlaySound(m_introSound);

    m_logoImage = new hdUIImage(NULL, NULL);

#ifdef IPHONE_BUILD
    float scaleFactor = 1.0f;
    ScreenSettings_GetScaleFactor(&scaleFactor);
    if (scaleFactor == 1.0f)
    {
        m_logo = hdTextureManager::Instance()->FindTexture("Interface/Textures/hd_logo1_512.png", TT_Wall);
    }
    else
    {
        m_logo = hdTextureManager::Instance()->FindTexture("Interface/Textures/hd_logo1_1024.png", TT_Wall);
    }
#else
    m_logo = hdTextureManager::Instance()->FindTexture("Interface/Textures/hd_logo1_1024.png", TT_Wall);
#endif

    m_logoImage->SetTexture(m_logo);
    m_logoImage->SetTint(1.0f, 1.0f, 1.0f, 1.0f);
    m_logoImage->SetAs2DBox(0.0f, 0.0f, m_PixelScreenWidth, m_PixelScreenWidth);
    m_logoImage->ResetAABB();
    m_logoImage->ResetTextureCoords();

    //if (GetOrientation() == e_hdInterfaceOrientationLandscapeRight)
    {

        hdTranslateVertices(m_logoImage->GetVertices(), m_logoImage->GetVertexCount(), -m_logoImage->GetWorldCenter());
        hdRotateVertices(m_logoImage->GetVertices(), m_logoImage->GetVertexCount(), hdVec3(0,0,-hd_half_pi));
        hdTranslateVertices(m_logoImage->GetVertices(), m_logoImage->GetVertexCount(), m_logoImage->GetWorldCenter());
#ifndef IPHONE_BUILD
        //hdTranslateVertices(m_logoImage->GetVertices(), m_logoImage->GetVertexCount(), hdVec3(0,-256,0));
#endif

    }

    m_fade = new hdUIImage(NULL, NULL);
    m_fade->SetTint(0.0f, 0.0f, 0.0f, 1.0f);
    m_fade->SetAs2DBox(0.0f, 0.0f, m_PixelScreenWidth, m_PixelScreenHeight);
}


IntroController::~IntroController()
{
    hdAnimationController::Instance()->StopAnimations(this);
    hdAnimationController::Instance()->PurgeAnimations(this);

    hdSoundManager::UnloadSound(m_introSound);

    delete m_logoImage;
    delete m_fade;

    hdTextureManager::Instance()->DestroyTexture(m_logo);
}


void IntroController::Step(double interval) {}


void IntroController::AnimationFinishedHandler(void* object, hdAnimation *animation)
{
    IntroController* self = (IntroController *)object;
    self->AnimateHide();
}


void IntroController::AnimateHide()
{
    hdPauseAction *m_pauseAction = new hdPauseAction();
    m_pauseAction->SetDuration(2.5f);
    m_fadeAnim->AddAction(m_pauseAction);

    m_fadeAction->SetStartAlpha(0.0f);
    m_fadeAction->SetEndAlpha(1.0f);
    m_fadeAnim->SetFinishedCallback(m_parentController, AppCallbackFunctions::IntroFinishedCallback);
    m_fadeAnim->StartAnimation();
}


void IntroController::Draw()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glPushMatrix();
    hdglOrthof(0,
               m_PixelScreenWidth,
               0,
               m_PixelScreenHeight,
               -50,
               50);
    glEnable(GL_TEXTURE_2D);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    m_logoImage->Draw();

    glDisable(GL_TEXTURE_2D);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);

    if (m_fadeAction->GetAlphaObject()->GetAlpha() != 0.0f && m_fadeAnim->GetStatus() == e_animationRunning)
    {
        m_fade->SetTint(0.0f, 0.0f, 0.0f, m_fadeAction->GetAlphaObject()->GetAlpha());
        m_fade->Draw();
    }
    glDisable(GL_BLEND);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    if (m_fadeAnim->GetStatus() != e_animationRunning)
    {
        m_fadeAnim->StartAnimation(); 
    }
}


void IntroController::HandleTapUp(float x, float y, int tapCount)
{
    if (m_fadeAction->GetStartAlpha() == 1.0f)
    {
        m_fadeAnim->StopAnimation(true, false);
        AnimateHide();
    }
}
