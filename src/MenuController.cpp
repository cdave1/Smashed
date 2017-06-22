/*
 * Copyright (c) 2017 Hackdirt Ltd.
 * Author: David Petrie (david@davidpetrie.com)
 *
 * License: https://www.gnu.org/licenses/gpl-3.0.en.html
 */

#include "MenuController.h"

MenuController::MenuController(const hdInterfaceController* parentController) : AppLevelController(parentController)
{
    m_worldTag = -1;

    InitInterface();
    InitAnimations();
    InitSounds();

    m_interactionState = e_waitingForInput;

    m_focusBlock = NULL;
    m_introAnimFinished = false;
}


MenuController::~MenuController()
{
    hdSoundManager::StopSound(m_backgroundMusic);

    hdAnimationController::Instance()->StopAnimations(this);
    hdAnimationController::Instance()->PurgeAnimations(this);

    delete m_interfaceProjection;
}


void MenuController::InitInterface()
{
    hdAABB interfaceAABB;
    interfaceAABB.lower = hdVec3(0,0,100.0f);
    interfaceAABB.upper = hdVec3(m_PixelScreenWidth, m_PixelScreenHeight, -100.0f);
    m_interfaceProjection = new hdOrthographicProjection(NULL, interfaceAABB);

    m_panMomentumEnabled = true;
}


void MenuController::InitAnimations()
{
    m_fadeAction = new hdAlphaAction();
    m_fadeAction->SetDuration(0.5f);
    m_fadeAction->SetStartAlpha(1.0f);
    m_fadeAction->SetEndAlpha(0.0f);

    m_fadeAnim = hdAnimationController::CreateAnimation(this, false);
    m_fadeAnim->AddGameObject(m_fadeAction->GetAlphaObject());
    m_fadeAnim->AddAction(m_fadeAction);

    m_introPauseAction = new hdPauseAction();
    m_introPauseAction->SetDuration(0.0f);

    m_transitionPanSpeed = 0.1f;
    m_transitionZoomSpeed = 0.2f;

    m_introFunctionAction = new hdFunctionAction<MenuController>();
    m_introFunctionAction->SetDuration(5.0f);
    m_introFunctionAction->SetFunctionObject(this);
    m_introFunctionAction->SetFunction(&MenuController::UpdateProjection);

    m_introAnimation = hdAnimationController::CreateAnimation(this, false);
    m_introAnimation->AddGameObject(m_projection);
    m_introAnimation->AddAction(m_introFunctionAction);
    m_introAnimation->AddAction(m_introPauseAction);
    m_introAnimation->SetFinishedCallback(this, MenuController::TransitionCallback);

}


void MenuController::InitSounds()
{
#ifdef LIGHTVERSION
    m_backgroundMusic = hdSoundManager::FindSound("Music/decisionsLITE.mp3", e_soundTypeBackground);
#else
    m_backgroundMusic = hdSoundManager::FindSound("Music/decisions.mp3", e_soundTypeBackground);
#endif

    m_btnMenuClickDownSound = hdSoundManager::FindSound("Sounds/btnClickDown.caf", e_soundTypeNormal);
    m_btnMenuClickUpSound = hdSoundManager::FindSound("Sounds/btnClick01.caf", e_soundTypeNormal);
}


void MenuController::Draw()
{
    this->DrawInternal();

    DrawSelectedBlockHighlight();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    if (IsLandscapeOrientation())
    {
        glRotatef(m_landscapeRotationZValue, 0.0f, 0.0f, 1.0f);
    }

    glPushMatrix();
    hdglOrthof(m_interfaceProjection->GetAABB().lower.x,
               m_interfaceProjection->GetAABB().upper.x,
               m_interfaceProjection->GetAABB().lower.y,
               m_interfaceProjection->GetAABB().upper.y,
               m_interfaceProjection->GetAABB().lower.z,
               m_interfaceProjection->GetAABB().upper.z);

    hdglBindTexture(NULL);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);
    DrawInternalInterface();
    glDisable(GL_TEXTURE_2D);

    glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
    if (m_fadeAnim->GetStatus() == e_animationRunning)
    {
        hdglBegin(GL_QUADS);
        hdglColor4f(0.0f, 0.0f, 0.0f, m_fadeAction->GetAlphaObject()->GetAlpha());
        hdglVertex2f(m_interfaceProjection->GetAABB().lower.x, m_interfaceProjection->GetAABB().lower.y);
        hdglVertex2f(m_interfaceProjection->GetAABB().lower.x, m_interfaceProjection->GetAABB().upper.y);
        hdglVertex2f(m_interfaceProjection->GetAABB().upper.x, m_interfaceProjection->GetAABB().upper.y);
        hdglVertex2f(m_interfaceProjection->GetAABB().upper.x, m_interfaceProjection->GetAABB().lower.y);
        hdglEnd();
    }
    glDisable(GL_BLEND);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
}


void MenuController::AnimateShow()
{
    m_fadeAction->SetStartAlpha(1.0f);
    m_fadeAction->SetEndAlpha(0.0f);
    m_fadeAnim->StartAnimation();

    m_introAnimation->StartAnimation();

    hdSoundManager::PlaySound(m_backgroundMusic);
}


#define BLOCK_TUTORIAL_TAG 222
#define BLOCK_LIGHT_VERSION_APP_STORE_TAG 211
const bool MenuController::Game_InitLevel(const Level *level, const int levelId)
{
    unsigned i;
    Block* block;
    TotemWorldStats* stats;
    bool completed;
    char title[128];
    char key[64];
    ChapterBlock* chapterBlock;

    /*
     * This is a gigantic hack - using block tags to get the world???????
     */
    for (i = 0; i < level->GetBlockCount(); ++i)
    {
        block = (Block *)level->GetBlocks()[i];

        if (block->GetTag() > 0 && block->GetTag() <= HACK_MAX_WORLD_TAG)
        {
            stats = (TotemWorldStats*)TowerPlayerStateController::Instance()->GetStatsForWorld(block->GetTag());
            if (stats) {
                completed = (!stats->isNextLocked());
            }
            else
            {
                completed = false;
            }

            block->SetBlockType(e_hdkBlockTypeNormal);

            snprintf(key, 64, "Chapter%d_Title", block->GetTag());
            snprintf(title, 128, "%s %d:\n\"%s\"",
                     Scripts_GetStringForKey("Chapter").c_str(),
                     block->GetTag() + 1,
                     Scripts_GetStringForKey(key).c_str());

            chapterBlock = new ChapterBlock(block, title, false, (completed) ? 1.0f : 0.0f);

            ((Level *)level)->AddBlock(chapterBlock);
        }
        else if (block->GetTag() == BLOCK_TUTORIAL_TAG)
        {
            // Tutorial world episode number is 0, so we have to load the stats
            // by loading the actual world.
            char filename[64];
            snprintf(filename, 64, Scripts_GetStringForKey("Chapter_World_File_Pattern").c_str(), 0);
            World *tw = WorldManager::Instance()->FindTotemWorld(filename);

            stats = (TotemWorldStats*)TowerPlayerStateController::Instance()->GetStatsForWorld(tw, e_hdkDifficultyEasy);
            if (stats)
            {
                completed = (!stats->isNextLocked());
            }
            else
            {
                completed = false;
            }

            block->SetBlockType(e_hdkBlockTypeNormal);
            chapterBlock = new ChapterBlock(block, "Episode 1:\nTraining", false, (completed) ? 1.0f : 0.0f);
            ((Level *)level)->AddBlock(chapterBlock);
        }
    }
    ((Level *)level)->ResetTextureCache();

    // Find block with tag 2000.
    m_focusBlock = FindBlockWithTag(2000);

    if (m_focusBlock == NULL) m_focusBlock = FindBlockWithTag(1001);
    if (m_focusBlock == NULL) return false;

    Game_HandleTapUp(0,0,0);
    return true;
}


const bool MenuController::Game_CanSelectBlock(const Block *block)
{
    return (!block->IsStatic()) ||
    block->IsEventSender() ||
    block->GetTag() == 110;
}


const bool MenuController::Game_HandleTapUp(float x, float y, int tapCount)
{
    if (!m_introAnimFinished)
    {
        m_focusBlock = FindBlockWithTag(1001);
        m_introAnimation->StopAnimationDead();
        if (m_focusBlock != NULL)
        {
            m_transitionPanSpeed = 4.5f;
            m_transitionZoomSpeed = 1.3f;

            m_introPauseAction->SetDuration(0.0f);
            m_introFunctionAction->SetDuration(1.25f);
            m_introFunctionAction->SetFunction(&MenuController::UpdateProjection);
            m_introAnimation->StartAnimation();
        }
        m_interactionState = e_waitingForInput;
        m_introAnimFinished = true;
    }
    return false;
}


const bool MenuController::Game_HandleBlockTap(Block *block, const hdVec3& impactPoint, bool hazardCollision)
{
    char tmp[256];
    totemMessage msg;

    if (IsShowingMessage()) return false;

    if (block->GetTag() > 0 &&
        block->GetTag() <= HACK_MAX_WORLD_TAG &&
        m_fadeAnim->GetStatus() != e_animationRunning)
    {
        this->SetWorldTag(block->GetTag());
        hdSoundManager::PlaySound(m_btnMenuClickUpSound);

        m_fadeAction->SetStartAlpha(0.0f);
        m_fadeAction->SetEndAlpha(1.0f);
        m_fadeAnim->SetFinishedCallback(m_parentController, AppCallbackFunctions::ShowLevelPickerControllerCallback);
        m_fadeAnim->StartAnimation();
    }
    else if (block->GetTag() > HACK_MAX_WORLD_TAG &&
             block->GetTag() == 110)
    {
        hdSoundManager::PlaySound(m_btnMenuClickDownSound);
        ((MenuController *)this)->ShowPreferences();
        return false;
    }
    else if (block->GetTag() > HACK_MAX_WORLD_TAG &&
             block->GetTag() == BLOCK_LIGHT_VERSION_APP_STORE_TAG)
    {
        hdSoundManager::PlaySound(m_btnMenuClickUpSound);
        OSFunctions_LoadExternalURL(Scripts_GetStringForKey("URL_FullVersion").c_str());
        return false;
    }
    else if (block->GetTag() == BLOCK_TUTORIAL_TAG)
    {
        this->SetWorldTag(0);
        hdSoundManager::PlaySound(m_btnMenuClickUpSound);

        m_fadeAction->SetStartAlpha(0.0f);
        m_fadeAction->SetEndAlpha(1.0f);
        m_fadeAnim->SetFinishedCallback(m_parentController, AppCallbackFunctions::ShowLevelPickerControllerCallback);
        m_fadeAnim->StartAnimation();
    }

    if (!hazardCollision) return false;
    if (!block->IsDestroyable()) return false;

    return true;
}


const bool MenuController::Game_HandleTapMovedSingle(const float previousX, const float previousY,
                                                     const float currentX, const float currentY)
{
    if (m_interactionState == e_interactionDisabled) return false;

    m_interactionState = e_panAndZoom;
    this->PanProjection(previousX, previousY, currentX, currentY);
    return true;
}


void MenuController::Game_hdkBlockWasSelected(const Block *block) const
{
    if (block &&
        block->GetTag() > 0 &&
        block->GetTag() <= HACK_MAX_WORLD_TAG)
    {
        hdSoundManager::PlaySound(m_btnMenuClickDownSound);
    }
}


void MenuController::Game_HideMessage()
{
    m_interactionState = e_waitingForInput;
}


void MenuController::AnimateTransition(hdTimeInterval interval)
{
}


void MenuController::TransitionCallback(void *handler, hdAnimation *anim)
{
    assert (handler && anim);

    MenuController *self = (MenuController *)handler;
    if (!self->QueueNextAnimation())
    {
        self->IntroAnimationDone();
    }
}


bool MenuController::QueueNextAnimation()
{
    Block *block501;

    assert(m_focusBlock != NULL);

    // Check m_focusBlock tag to get next one, then set props of animation and start again.
    if (m_focusBlock->GetTag() == 2000)
    {
        m_focusBlock = FindBlockWithTag(1001);
        if (m_focusBlock != NULL)
        {
            m_transitionPanSpeed = 5.0f;
            m_transitionZoomSpeed = 1.0f;
            
            m_introAnimation->StopAnimationDead();
            m_introPauseAction->SetDuration(0.0f);
            m_introFunctionAction->SetDuration(1.0f);
            m_introFunctionAction->SetFunction(&MenuController::UpdateProjection);
            m_introAnimation->StartAnimation();
            
            m_introAnimFinished = true;
            return true;
        }
    }
    return false;
}


void MenuController::IntroAnimationDone()
{
}


void MenuController::UpdateProjection(hdTimeInterval interval)
{
    if (m_focusBlock)
    {
        assert(m_focusBlock != NULL);
        PanAndZoomToObject(m_focusBlock->GetWorldCenter(), m_transitionPanSpeed, m_transitionZoomSpeed);
    }
}


void MenuController::PanAndZoomToObject(const hdVec3& point, const float panRate, const float zoomRate)
{
    // While the projection is not at the starting position (0.0, upper.x = level.upper.x)
    // move a fraction of the difference between the aabb and the destination.
    float dx, dy;
    dx = dy = 0.0f;
    
    hdVec3 box(0.05f * (m_projection->GetAABB().upper - m_projection->GetAABB().lower));
    
    hdVec3 diff = (m_focusBlock->GetWorldCenter() - m_projection->GetWorldCenter());
    
    hdVec3 mag(fabs(diff.x), fabs(diff.y), 0.0f);
    
    mag.x = (mag.x < box.x) ? 0.0f : mag.x - box.x;
    mag.y = (mag.y < box.y) ? 0.0f : mag.y - box.y;
    
    dx = (diff.x > 0.0f) ? -mag.x : mag.x;
    dy = (diff.y < 0.0f) ? -mag.y : mag.y;
    
    dx *= panRate;
    dy *= panRate;
    
    if (zoomRate < 0.0f)
    {
        ZoomProjection(0.0f, 0.0f,
                       zoomRate, zoomRate,
                       0.0f, 0.0f,
                       -zoomRate, -zoomRate);
    }
    else
    {
        ZoomProjection(zoomRate, zoomRate,
                       0.0f, 0.0f,
                       -zoomRate, -zoomRate,	
                       0.0f, 0.0f);
    }
    
    PanProjection(0.0f, 0.0f, dx, dy);
}
