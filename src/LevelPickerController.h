/*
 * Copyright (c) 2017 Hackdirt Ltd.
 * Author: David Petrie (david@davidpetrie.com)
 *
 * License: https://www.gnu.org/licenses/gpl-3.0.en.html
 */

#ifndef _HDK_LEVEL_PICKER_CONTROLLER_
#define _HDK_LEVEL_PICKER_CONTROLLER_

#include "Box2D.h"

#include <hdk/game.h>
#include "AppCallbackFunctions.h"

#include "AppLevelController.h"
#include "LevelPickerBlock.h"

#include "TowerPlayerStateClasses.h"
#include "TowerPlayerStateController.h"


class LevelPickerController : public AppLevelController
{
public:
    LevelPickerController(const hdInterfaceController* parentController);

    ~LevelPickerController();

    void Draw();

    void AnimateShow();

    void SetWorld(World *tw, const int worldTag);

    void BackToMenuClicked();

    const int GetSelectedLevelId() const;

    Level* GetSelectedLevel();

    void ShowLobbyAnimation();

    void PanToNewLevelBlock();


protected:

    friend class AppController;

    void HandleLevelClick();

    void ShowLevelController();

    void CreateLevelPickerBlocks();

    void HandleQuitToLevelMenu();

    void IntroZoomProjection(hdTimeInterval interval);

    void IntroPanProjection(hdTimeInterval interval);

    void ShowWorldTitleMessage(const int worldTag);

    void ShowNewLevelMessage(const TotemLevelStats* stats);

    void ShowNextWorldUnlockedMessage(const int worldTag);

    void Game_Step(double interval);

    const bool Game_HandleTapUp(float x, float y, int tapCount);

    const bool Game_HandleTapDown(float x, float y, int tapCount);

    const bool Game_HandleTapMovedSingle(const float previousX, const float previousY,
                                         const float currentX, const float currentY);

    const bool Game_HandleBlockTap(Block *block, const hdVec3& impactPoint, bool hazardCollision);

    void Game_HandleCollision(const ContactPoint* point);

    void Game_HideMessage();

    const bool Game_CanSelectBlock(const Block *block);


    void ShowLevelInfoInterface();

    static void ShowLevelInfoAnimCallback(void *handler, hdAnimation *anim);

    void HideLevelInfoInterface();

    static void HideLevelInfoAnimCallback(void *handler, hdAnimation *anim);

    /*
     * Interface callback handlers
     */
    static void btnCancelUpCallback(void *handler, void *sender);

    static void btnPlayUpCallback(void *handler, void *sender);

    static void btnBackUpCallback(void *handler, void *sender);


private:
    friend class hdAnimation;

    void InitInterface();

    void InitAnimations();

    void InitSounds();

    void InitPickerLevel();

    void InitAmbience(const int worldTag);

    void RefreshLayout();

    void SetCurrentLevel(const Level* level);

    void PlaySoftCollisionSound(const Block* block, const b2Body* body);


    /*
     * Book keeping
     */
    Level* m_selectedLevel;

    int m_selectedLevelId;

    hdGameObject* m_currentObject;

    int m_completedLevels;

    float m_percentComplete;

    TotemLevelStats *m_lastCompleteLevelStats;

    Block *startBlock;


    /*
     * Sounds
     */
    hdSound *m_currAmbience;

    hdSound *m_btnMenuClickDownSound;

    hdSound *m_btnMenuClickUpSound;

    hdSound *m_btnNextLevel01Sound;

    hdSound *m_sound_btnClickBack01;

    hdSound *m_sound_btnClick01;

    hdSound *m_soundNextWorldUnlocked;


    /*
     * Misc Buttons and images
     */
    hdButton* m_btnBack;

    hdOrthographicProjection *m_interfaceProjection;

    hdUIContainer *m_interfaceContainer;

    hdFontPolygon *m_fontStats;


    /*
     * Special Blocks
     */
    hdTypedefList<LevelPickerBlock*, 128>* m_levelPickerBlocks;


    /*
     * Level Info Panels - Landscape only
     */

    hdUIContainer *m_pnlLeftInfoPanelLandscape;

    hdUIImage *m_imgLeftPanelChromeLandscape;

    hdFontPolygon *m_fontLevelTitleLandscape;

    hdUIImage *m_imgLevelScreenshotLandscape;

    hdUIContainer *m_pnlRightInfoPanelLandscape;

    hdUIContainer *m_pnlRightButtonPanelLandscape;

    hdUIImage *m_imgRightPanelChromeLandscape;

    hdUIImage *m_imgLevelStarLandscape;

    hdFontPolygon *m_fontLevelPercentageLandscape;

    hdFontPolygon *m_fontLevelStatsLandscape;

    hdUIImage *m_imgLevelStatsBackgroundLandscape;

    hdButton *m_btnPlayLevelLandscape;

    hdButton *m_btnCancelInfoLandscape;


    /*
     * Level Info Panels - Portrait only
     */
    hdUIContainer *m_pnlLeftInfoPanelPortrait;

    hdUIImage *m_imgLeftPanelChromePortrait;

    hdFontPolygon *m_fontLevelTitlePortrait;

    hdUIImage *m_imgLevelScreenshotPortrait;

    hdUIContainer *m_pnlRightInfoPanelPortrait;

    hdUIContainer *m_pnlRightButtonPanelPortrait;
    
    hdUIImage *m_imgRightPanelChromePortrait;
    
    hdUIImage *m_imgLevelStarPortrait;
    
    hdFontPolygon *m_fontLevelPercentagePortrait;
    
    hdFontPolygon *m_fontLevelStatsPortrait;
    
    hdUIImage *m_imgLevelStatsBackgroundPortrait;
    
    hdButton *m_btnPlayLevelPortrait;
    
    hdButton *m_btnCancelInfoPortrait;
    
    
    /* 
     * Animations 
     */
    
    hdAnimation* m_fadeAnim;
    
    hdAlphaAction* m_fadeAction;
    
    hdAnimation *m_lobbyProjectionAnimation;
    
    hdFunctionAction<LevelPickerController> *m_lobbyZoomProjectionAction;
    
    hdFunctionAction<LevelPickerController> *m_lobbyPanProjectionAction;
    
    e_hdkDifficultyLevel m_difficultyLevel;
    
};


inline Level* LevelPickerController::GetSelectedLevel()
{
    return m_selectedLevel;
}


inline const int LevelPickerController::GetSelectedLevelId() const
{
    return m_selectedLevelId;
}

#endif
