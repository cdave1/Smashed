/*
 * Copyright (c) 2017 Hackdirt Ltd.
 * Author: David Petrie (david@davidpetrie.com)
 *
 * License: https://www.gnu.org/licenses/gpl-3.0.en.html
 */

#include "LoadingController.h"

LoadingController::LoadingController(const hdInterfaceController* parentController) :
hdInterfaceController(parentController->GetOrientation(), parentController)
{
    hdAABB interfaceAABB;
    interfaceAABB.lower = hdVec3(0,0,100.0f);
    interfaceAABB.upper = hdVec3(m_PixelScreenWidth, m_PixelScreenHeight, -100.0f);
    m_projection->SetAABB(interfaceAABB.lower, interfaceAABB.upper);

    // Pause action
    m_fadeAction = new hdAlphaAction();
    m_fadeAction->SetDuration(0.25f);
    m_fadeAction->SetStartAlpha(1.0f);
    m_fadeAction->SetEndAlpha(0.0f);

    m_fadeAnim = hdAnimationController::CreateAnimation(this, false);
    m_fadeAnim->AddGameObject(m_fadeAction->GetAlphaObject());
    m_fadeAnim->AddAction(m_fadeAction);

    m_loadingImage = new hdUIImage("Interface/Textures/imgLoading.png", NULL);
    m_loadingImage->SetAs2DBox(0.5f * (m_PixelScreenWidth - 128.0f), 0.5f * (m_PixelScreenHeight - 32.0f), 128.0f, 32.0f);
}


LoadingController::~LoadingController()
{
    hdAnimationController::Instance()->StopAnimations(this);
    hdAnimationController::Instance()->PurgeAnimations(this);
    delete m_loadingImage;
}


void LoadingController::Reset()
{
    hdAABB interfaceAABB;

    // Just ensure the loading texture is still around.
    //hdTextureManager::Instance()->FindTexture("Interface/Textures/imgLoading.tga", TT_Background);
    if (m_loadingImage != NULL)
    {
        delete m_loadingImage;
    }
    m_loadingImage = new hdUIImage("Interface/Textures/imgLoading.png", NULL);
    m_loadingImage->SetAs2DBox(0.5f * (m_PixelScreenWidth - 128.0f), 0.5f * (m_PixelScreenHeight - 32.0f), 128.0f, 32.0f);

    interfaceAABB.lower = hdVec3(0,0,100.0f);
    interfaceAABB.upper = hdVec3(m_PixelScreenWidth, m_PixelScreenHeight, -100.0f);
    m_projection->SetAABB(interfaceAABB.lower, interfaceAABB.upper);
}


void LoadingController::Step(double interval) {}


void LoadingController::AnimateShow()
{
}


void LoadingController::Draw()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glPushMatrix();
    hdglOrthof(m_projection->GetAABB().lower.x,
               m_projection->GetAABB().upper.x,
               m_projection->GetAABB().lower.y,
               m_projection->GetAABB().upper.y,
               m_projection->GetAABB().lower.z,
               m_projection->GetAABB().upper.z);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_TEXTURE_2D);
    hdglBindTexture(NULL);

    hdglBegin(GL_QUADS);
    hdglColor4f(0.0f, 0.0f, 0.0f, 1.0f);
    hdglVertex2f(m_projection->GetAABB().lower.x, m_projection->GetAABB().lower.y);
    hdglVertex2f(m_projection->GetAABB().lower.x, m_projection->GetAABB().upper.y);
    hdglVertex2f(m_projection->GetAABB().upper.x, m_projection->GetAABB().upper.y);
    hdglVertex2f(m_projection->GetAABB().upper.x, m_projection->GetAABB().lower.y);
    hdglEnd();

    m_loadingImage->Draw();

    glDisable(GL_TEXTURE_2D);
    /*
     glEnable(GL_BLEND);
     glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
     if (m_fadeAction->GetAlphaObject()->GetAlpha() != 0.0f)
     {
     hdglBegin(GL_QUADS);
     hdglColor4f(0.0f, 0.0f, 0.0f, m_fadeAction->GetAlphaObject()->GetAlpha());
     hdglVertex2f(m_projection->GetAABB().lower.x, m_projection->GetAABB().lower.y);
     hdglVertex2f(m_projection->GetAABB().lower.x, m_projection->GetAABB().upper.y);
     hdglVertex2f(m_projection->GetAABB().upper.x, m_projection->GetAABB().upper.y);
     hdglVertex2f(m_projection->GetAABB().upper.x, m_projection->GetAABB().lower.y);
     hdglEnd();
     }
     glDisable(GL_BLEND);
     */
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
}

