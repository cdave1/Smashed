/*
 * Copyright (c) 2017 Hackdirt Ltd.
 * Author: David Petrie (david@davidpetrie.com)
 *
 * License: https://www.gnu.org/licenses/gpl-3.0.en.html
 */

#ifndef MENU_CONTROLLER_H
#define MENU_CONTROLLER_H

#include <hdk/game.h>
#include "AppCallbackFunctions.h"
#include "AppLevelController.h"

#include "ChapterBlock.h"

class MenuController : public AppLevelController
{
public:

    MenuController(const hdInterfaceController* parentController);

    ~MenuController();

    void Draw();

    void AnimateShow();

    void SetWorldTag(const int tag);

    const int GetWorldTag() const;


protected:

    const bool Game_CanSelectBlock(const Block *block);

    const bool Game_InitLevel(const Level *level, const int levelId);

    const bool Game_HandleTapUp(float x, float y, int tapCount);

    const bool Game_HandleBlockTap(Block *block, const hdVec3& impactPoint, bool hazardCollision);

    virtual const bool Game_HandleTapMovedSingle(const float previousX, const float previousY,
                                                 const float currentX, const float currentY);

    void Game_HandleCollision(const ContactPoint* point) {}

    void Game_ShowMessage(const hdTexture *texture) {}

    void Game_HideMessage();

    void Game_hdkBlockWasSelected(const Block *block) const;

    void PanAndZoomToObject(const hdVec3& point, const float panRate, const float zoomRate);

    /* HACK HACK HACK
     *
     * Doing these animations all within a chain of callbacks
     *
     * In future, this should be done with a script
     */
    void AnimateTransition(hdTimeInterval interval);

    static void TransitionCallback(void *handler, hdAnimation *anim);

    bool QueueNextAnimation();

    void IntroAnimationDone();

    void UpdateProjection(hdTimeInterval interval);

    float m_transitionPanSpeed;

    float m_transitionZoomSpeed;

private:

    friend class hdAnimation;

    void InitInterface();

    void InitAnimations();

    void InitSounds();


    int m_worldTag;

    Block *m_focusBlock;

    bool m_introAnimFinished;

    hdOrthographicProjection* m_interfaceProjection;

    hdSound *m_btnMenuClickDownSound;

    hdSound *m_btnMenuClickUpSound;

    hdSound *m_backgroundMusic;

    hdAnimation* m_introAnimation;
    
    hdPauseAction *m_introPauseAction;
    
    hdAnimation* m_fadeAnim;
    
    hdAlphaAction* m_fadeAction;
    
    hdFunctionAction<MenuController>* m_introFunctionAction;
    
};


inline void MenuController::SetWorldTag(const int tag)
{
    m_worldTag = tag;
}


inline const int MenuController::GetWorldTag() const
{
    return m_worldTag;
}

#endif
