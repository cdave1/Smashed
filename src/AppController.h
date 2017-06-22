/*
 * Copyright (c) 2017 Hackdirt Ltd.
 * Author: David Petrie (david@davidpetrie.com)
 *
 * License: https://www.gnu.org/licenses/gpl-3.0.en.html
 */

#ifndef APP_CONTROLLER_H_
#define APP_CONTROLLER_H_

#include "Box2D.h"
#include "LoadingController.h"
#include "IntroController.h"
#include "MenuController.h"
#include "GameController.h"
#include "LevelPickerController.h"
#include "TowerPlayerStateController.h"

class GameController;
class MenuController;
class LevelPickerController;
class IntroController;
class LoadingController;

class AppController : public hdInterfaceController
{
public:
    AppController();

    ~AppController();

    void StartApp();

    void IntroFinished();

    void ShowGameController();

    void GameControllerStopCurrentLevel();

    void GameControllerRestartCurrentLevel();

    void GameControllerQuitCurrentLevel();

    void LevelPickerControllerLevelClicked();

    void ShowLevelPickerController();

    void LevelPickerBackClicked();

    void SetLoading();

    void SaveGameState();

    void FreeIdleMemory();

    void LoadPlayerConfigFile();

    void TearDown();

    void Step(double interval);

    void Step(double sysInterval, double fixedInterval);

    void Draw();

    void AnimateShow() {}

    void HandleTapUp(float x, float y, int tapCount);

    void HandleTapDown(float x, float y, int tapCount);

    void HandleSingleTap(float x, float y);

    void HandleDoubleTap(float x, float y);

    void HandleTapMovedSingle(const float previousX, const float previousY,
                              const float currentX, const float currentY);

    void HandleTapMovedDouble(const float aPreviousX, const float aPreviousY,
                              const float aCurrentX, const float aCurrentY,
                              const float bPreviousX, const float bPreviousY,
                              const float bCurrentX, const float bCurrentY);

    void HandleResize(const float newWidth, const float newHeight);

    std::string GetCurrentTotemWorldName() const;

private:
    friend class hdAnimation;

    void FreeMemory();

    void InitLevelPickerWorld();

    void EnsureLevelPickerController();

    void EnsureMenuController();

    void EnsureGameController();

    void DispatchMessages();

    const TotemLevelStats* GetStatsForLevel(const World* currentWorld,
                                            const Level *currentLevel) const;

    /*
     * If the info has a completion state then we advance the level onwards.
     * otherwise, discard the game info and change nothing.
     */
    void UpdateGameStats(const World* currentWorld,
                         const Level *currentLevel,
                         const GameInfo& info);
    
    LoadingController *loadingController;
    
    IntroController *introController;
    
    LevelPickerController* levelPickerController;
    
    MenuController* menuController;
    
    GameController* gameController;
    
    hdInterfaceController* activeController;
    
    World *m_levelPickerWorld;
    
    int m_worldTag;
    
    int m_levelId;
    
    Level *m_selectedLevelPickerLevel;
    
    bool m_showNextWorldUnlockedMessage;

};

#endif
