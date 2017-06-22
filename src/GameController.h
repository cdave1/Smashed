/*
 * Copyright (c) 2017 Hackdirt Ltd.
 * Author: David Petrie (david@davidpetrie.com)
 *
 * License: https://www.gnu.org/licenses/gpl-3.0.en.html
 */

/*
 * CONTROLLER FOR IN-GAME STUFF.
 */

#ifndef GAME_CONTROLLER_H_
#define GAME_CONTROLLER_H_

#include "Box2D.h"

#include <hdk/game.h>
#include "hdk/effects.h"
#include "AppCallbackFunctions.h"
#include "AppLevelController.h"

#include "GoalBlock.h"
#include "LevelPickerBlock.h"

#include "TowerPlayerStateClasses.h"
#include "TowerPlayerStateController.h"

#define GAME_CONTROLLER_STATS_FONT		CONFIG_SMALL_FONT
#define GAME_CONTROLLER_NOTE_FONT		CONFIG_NOTE_FONT
#define GAME_CONTROLLER_PERCENT_FONT	CONFIG_PERCENT_FONT
#define GAME_CONTROLLER_POINTS_FONT	CONFIG_POINTS_FONT
#define GAME_CONTROLLER_SMALL_FONT		CONFIG_SMALL_FONT
#define GAME_CONTROLLER_MED_FONT		CONFIG_MED_FONT
#define GAME_CONTROLLER_LARGE_FONT		CONFIG_LARGE_FONT

#define HDK_GAME_BlockMax 512

#define ICON_POSITION_LEFT 0
#define ICON_POSITION_CENTER 1
#define ICON_POSITION_RIGHT 2

class GameController : public AppLevelController
{
public:
    GameController(const hdInterfaceController* parentController);

    ~GameController();

    //void Step(double interval);

    void Draw();

    void AnimateShow();

    void FinishedClicked();

    void SkipClicked();

    void QuitClicked();

    void RestartClicked();

    void IntroPanProjection(hdTimeInterval interval);

    void IntroZoomProjection(hdTimeInterval interval);

    void PlayerReadyMoveProjection(hdTimeInterval interval);

    TotemLevelStats *GetLevelStats() const;

    void SetLevelStats(const TotemLevelStats *levelStats);

    GameInfo gameInfo;


protected:
    void UpdateGameState();

    const bool Game_InitLevel(const Level *level, const int levelId);

    void Game_Step(double interval);

    void Game_ResetCurrentLevel();

    const bool Game_CanSelectBlock(const Block *block);

    const bool Game_HandleTapUp(float x, float y, int tapCount);

    const bool Game_HandleTapDown(float x, float y, int tapCount);

    const bool Game_HandleSingleTap(float x, float y);

    const bool Game_HandleDoubleTap(float x, float y);

    const bool Game_HandleTapMovedSingle(const float previousX, const float previousY,
                                         const float currentX, const float currentY);

    const bool Game_HandleTapMovedDouble(const float aPreviousX, const float aPreviousY,
                                         const float aCurrentX, const float aCurrentY,
                                         const float bPreviousX, const float bPreviousY,
                                         const float bCurrentX, const float bCurrentY);

    const bool Game_HandleBlockTap(Block *block, const hdVec3& impactPoint, bool hazardCollision);

    const bool Game_HandleParticleTap(hdPhysicsParticle* particle, const hdVec3& impactPoint, bool hazardCollision);

    const bool Game_HandleBlockLoss(Block* block, const hdVec3& contactPoint);

    void Game_HandleCollision(const ContactPoint* point);

    void Game_ShowMessage();

    void Game_HideMessage();

    void Game_PlayLoudCollisionSound(const Block* block, const b2Body* body);

    void Game_PlaySoftCollisionSound(const Block* block, const b2Body* body);

    void Game_AddBlockToLevel(const Block* block) {}

    void Game_HandleGoalCollision(Block *goalBlock, Block *block, const hdVec3& impactPoint);

    void Game_hdkBlockWasSelected(const Block *block) const;


    /*
     * Callback handlers
     */
    static void btnShowMenuUpCallback(void *handler, void *sender);
    static void btnCloseMenuUpCallback(void *handler, void *sender);
    static void btnMenuRestartCallback(void *handler, void *sender);
    static void btnMenuQuitCallback(void *handler, void *sender);
    static void btnMenuSkipCallback(void *handler, void *sender);


    // User interface and menus
    hdButton *m_btnShowMenu;
    hdButton *m_btnFinishLevel;
    hdButton *m_btnFailRestart;
    hdButton *m_btnBG;

    hdButton *m_btnMenuRestart;
    hdButton *m_btnSettings;
    hdButton *m_btnMenuSkip;
    hdButton *m_btnMenuQuit;
    hdButton *m_btnMenuClose;

    static void btnSettingsUpCallback(void *handler, void *sender);

    hdButton *m_btnLobbyReady;
    hdButton *m_btnLobbyReadyPortrait;
    hdButton *m_btnLobbyReadyLandscape;

    hdUIContainer *m_gameButtonLayer;
    hdUIContainer *m_menuButtonLayer;

    hdFontPolygon *m_fontLevelName;
    hdFontPolygon *m_fontLevelGoals;


    hdOrthographicProjection* m_interfaceProjection;


    // Level start projection animation
    hdAnimation* m_levelStartProjectionAnimation;
    hdFunctionAction<GameController> *m_levelStartZoomProjectionAction;
    hdFunctionAction<GameController> *m_levelStartPanProjectionAction;
    hdVectorAction* m_lobbyButtonShowAction;

    hdAnimation* m_playerReadyProjectionAnimation;
    hdFunctionAction<GameController>* m_playerReadyProjectionAction;

    // Lobby button - hide/show
    void HideLobbyButton();
    void ShowLobbyButton();

    // Finish Level button - show only
    void ShowFinishLevelButton();
    void ShowFailRestartButton();

    hdPointerList<hdReceiver, kAnimationPoolCapacity> *m_animPolygons;
    hdPointerList<hdReceiver, kAnimationPoolCapacity> *m_gameWorldAnimPolygons;
    hdPointerList<Polygon, kAnimationPoolCapacity> *m_notificationPolygons;


    /*
     * Pulse the object (scale up then down).
     */
    void AnimatePulse(hdGameObject *object, float duration);

    void AnimateIcon(const char* iconTexture, const hdVec3& gamePosition,
                     int position, bool drifts, float xWidth, float duration);
    static void AnimateIconCallback(void *handler, hdAnimation *anim);


    // Points on smash
    // Need to have multiple animations.
    // - Font image
    // - animations that translates up, fades out, and scale up then down.
    void AnimatePointsMessage(const hdVec3& pos, int points, float scale);
    static void PointsMessageCallback(void *handler, hdAnimation *anim);


    void AnimateMidairChainMessage(Block *block, int points);
    static void MidairChainMessageCallback(void *handler, hdAnimation *anim);

    bool isAnimatingMidairChain;


    void AnimateRiskyChainMessage(Block *block, int points);
    static void RiskyChainMessageCallback(void *handler, hdAnimation *anim);


    /*
     * Multiplier
     *
     */
    void AnimateMultiplierMessage(int multiplier);
    static void MultiplierMessageCallback(void *handler, hdAnimation *anim);


    void AnimateLevelClearNotification();
    static void LevelClearNotificationCallback(void *handler, hdAnimation *anim);


    //void AnimateAchievementReceivedMessage(totemAchievement* achievement);
    //static void AchievementReceivedMessageCallback(void *handler, hdAnimation *anim);
    hdAnimation *m_goalAnim;
    hdAnimation *m_goalFadeAnim;
    void AnimateGoalMessage(const char *description, hdGameObject *messageBox, const float duration);
    void AnimateGoalMessage(const totemGoal& goal, hdGameObject *messageBox);
    static void GoalMessageCallback(void *handler, hdAnimation *anim);


    // When player loses a block
    // - fade in and pulse
    // - pause for a second
    // - quick fade out.
    void AnimateLostBlockNotification(Block* block);
    static void HandleNotificationUpdateCallback(void *handler, void *notification);
    static void LostBlockNotificationCallback(void *handler, hdAnimation *anim);

    void AnimateLostMessage(int count);
    static void LostMessageCallback(void *handler, hdAnimation *anim);

    bool isAnimatingLostMessage;

    void AnimateFragmentsMessage(int count);
    static void FragmentsMessageCallback(void *handler, hdAnimation *anim);

    bool isAnimatingFragmentsMessage;

    // When player does a chain combo
    // - show polygon with combo count.

    // When player gets to 75% completion
    // When player gets to 85% completion
    // When player gets to 99% completion.
    void AnimateCompletionNotification(const float duration, hdGameObject* notification);
    static void CompletionNotificationCallback(void *handler, hdAnimation *anim);



    // When more than 25% of blocks fall in the water.
    void AnimateFailNotification();
    static void FailNotificationCallback(void *handler, hdAnimation *anim);

    // Checkpoint
    void AnimateCheckpointMessage();

    // When player clicks checkpoint block.
    hdAnimation* AnimateFadeInOut(hdGameObject *obj, const float fadeInDuration, const float pauseDuration, const float fadeOutDuration);

    hdAnimation* AnimateSlideInOut(hdGameObject *messageBox, e_screenEdge side, const float duration);

    // Camera Vibration animation
    void StartCameraShake();

    void CameraShakeFunctionCallback(hdTimeInterval interval);

    float m_cameraShakeDelta;

    // Goals



#define kGameControllerSpecialsCount 64
    hdTypedefList<Block*, kGameControllerSpecialsCount> *m_specialBlocks;

    WeatherParticleEmitter *m_mistEmitter;

    /**********************************************************************/
    /* Animations */
    /**********************************************************************/
    hdAnimation* m_fadeAnim;

    hdAnimation *m_cameraShakeAnimation;

    hdAnimation *m_completionSlideAnim;

    hdAnimation *m_completionFadeAnim;

    hdAnimation *m_goalTextAnim;

    hdAnimation *m_showFinishButtonAnim;

    hdAnimation *m_midAirNoteAnim;

    hdAnimation *m_fragmentsBoxAnim;

    hdAnimation *m_lostMessageBoxAnim;

    hdAnimation *m_failRestartAnim;

    hdAlphaAction* m_fadeAction;





private:
    friend class hdAnimation;

    void InitInterface();

    void InitAnimations();

    void InitSounds();

    void InitMusic(int levelId);

    void InitLevelInterface();

    void RefreshLayout();

    void DisableMenu();

    void ShowGoalProgressMessage();

    void ShowWinMessage();

    void ShowFailMessage(const totemGoal& goal);

    void UpdateGameInfo();

    StarEmitter *m_starParticleEmitter;



    /**********************************************************************/
    /* Goals and goal handlers */
    /**********************************************************************/
#define GAME_GOAL_UNACHIEVED 0
#define GAME_GOAL_NORMAL_FIRST 5001
#define GAME_GOAL_NORMAL_SECOND 5002
#define GAME_GOAL_NORMAL_THIRD 5003
#define GAME_GOAL_MINBLOCKS_FIRST 5004
#define GAME_GOAL_MINBLOCKS_ALL 5005
#define GAME_GOAL_MINPOINTS_ALL 5006
#define GAME_GOAL_FRAGMENTS_ALL 5007
#define GAME_GOAL_JACKS_FIRST 5008
#define GAME_GOAL_JACKS_ALL 5009
#define GAME_GOAL_BLOCKSWITHTAG_FIRST 5010
#define SMASHED_GOALS_ACHIEVED_MESSAGE_TAG 6000
#define LEVELPICKER_NEW_LEVEL_MESSAGE_TAG 6001
    int ticksSinceLastUpdate;

    uint32 ticksInfoCycle;

    uint32 currGoalInfoCycle;

    totemGoal m_goals[5];

    int m_goalCount;

    hdUIImage *m_currentProgressMessage;

    hdUIImage *m_goalFailedMessageBox;

    hdUIImage *m_goalCompleteMessageBox;

    hdUIImage *m_specialGoalMessageBox;

    hdFontPolygon *m_goalDescriptionMessageText;

    float UpdateNormalGoal(totemGoal& goal);

    float UpdateMinBlocksPercentageGoal(totemGoal& goal);

    float UpdateMinPointsGoal(totemGoal& goal);

    float UpdateFragmentsGoal(totemGoal& goal);

    float UpdateJackCollectionGoal(totemGoal& goal);

    float UpdateAllBlocksWithTagGoal(totemGoal& goal);

    hdButton *m_showGoalProgress;



    /**********************************************************************/
    /* Music */
    /**********************************************************************/
    hdSound *m_currBackgroundTrack;


    /**********************************************************************/
    /* Sound */
    /**********************************************************************/
    hdSound *m_btnMenuClickDownSound;
    hdSound *m_btnMenuClickUpSound;
    hdSound *m_btnMenuClickUpSoundDeep;

    hdSound *m_midairSound;
    hdSound *m_midairComboSound;
    hdSound *m_soundIllegalTouch;
    hdSound *m_soundIllegalMove;
    hdSound *m_btnLevelRestartSound;
    hdSound *m_levelFinishButton01Sound;
    hdSound *m_blockZap01Sound;
    hdSound *m_levelWin01Sound;
    hdSound *m_levelWin02Sound;
    hdSound *m_levelWin03Sound;
    hdSound *m_levelFailSound;
    hdSound *m_blockSelectSound;
    hdSound *m_jackCollectionSound;

    hdSound *m_vibrationSound;

    hdSound *m_Bonus50FragmentsSound;


    /**********************************************************************/
    /* Fonts */
    /**********************************************************************/
    hdFontPolygon *m_fontProgress;

    hdFontPolygon *m_fontStats;
    
    hdFontPolygon* m_fontLostMessage;
    
    hdFontPolygon *multiplierFont;
    
    hdFontPolygon *midAirChain;
    
    hdFontPolygon *m_lostMessage;
    
    hdFontPolygon *m_fragmentsText;
    
    
    /**********************************************************************/
    /* Images */
    /**********************************************************************/
    hdUIContainer *uiImageContainer;
    
    hdUIImage *imgEmptyMedal;
    
    hdUIImage *imgBronzeMedal;
    
    hdUIImage *imgSilverMedal;
    
    hdUIImage *imgGoldMedal;
    
    hdUIImage *midAirComboBox;
    
    hdUIImage *m_completionMessageBox;
    
    hdUIImage *m_fragmentsBox;
    
    hdUIImage *m_lostMessageBox;
    
    hdUIImage *m_winMessage1;
    
    hdUIImage *m_winMessage2;
    
    hdUIImage *m_winMessage3;
    
    hdUIImage *m_failMessage;
    
    hdUIImage *m_newHighScore;
    
    
    /**********************************************************************/
    /* Misc */
    /**********************************************************************/
    int m_heavyCollisionCount;
    
    bool m_hasShownWinMessage;
    
    
    /**********************************************************************/
    /* Textures */
    /**********************************************************************/
    hdTexture *m_background;
    
    hdTexture *m_seaTexture1, *m_seaTexture2;
    
    hdTexture *m_stargoalbronze, *m_stargoalsilver, *m_stargoalgold;
    
    TotemLevelStats *m_levelStats;
    
};


inline void GameController::SetLevelStats(const TotemLevelStats *levelStats)
{
    m_levelStats = (TotemLevelStats *)levelStats;
}


inline TotemLevelStats *GameController::GetLevelStats() const
{
    return m_levelStats;
}


#endif
