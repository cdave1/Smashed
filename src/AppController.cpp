/*
 * Copyright (c) 2017 Hackdirt Ltd.
 * Author: David Petrie (david@davidpetrie.com)
 *
 * License: https://www.gnu.org/licenses/gpl-3.0.en.html
 */

#include "AppController.h"


AppController::AppController() : hdInterfaceController()
{
    levelPickerController = NULL;
    gameController = NULL;

    m_showNextWorldUnlockedMessage = false;

    m_levelPickerWorld = NULL;
    m_worldTag = 0;

    m_levelId = -1;
    m_selectedLevelPickerLevel = NULL;
}


AppController::~AppController()
{
    if (introController != NULL)
    {
        hdPrintf("\t*** Delete intro controller\n");
        delete introController;
        introController = NULL;
    }

    if (menuController != NULL)
    {
        hdPrintf("\t*** Delete menu controller\n");
        delete menuController;
        menuController = NULL;
    }

    if (gameController != NULL)
    {
        hdPrintf("\t*** Delete game controller\n");
        delete gameController;
        gameController = NULL;
    }

    if (levelPickerController != NULL)
    {
        hdPrintf("\t*** Delete level picker controller\n");
        delete levelPickerController;
        levelPickerController = NULL;
    }

    Scripts_TearDown();
    hdSoundManager::TearDown();
    hdFontManager::TearDown();
    hdTextureManager::TearDown();
    hdAnimationController::TearDown();
    WorldManager::TearDown();
    FileSystem_Teardown();
}


void AppController::StartApp()
{
    // Ensure we have a game dir
    hdAssert(strlen(FileSystem_BaseDir()) > 0);

    // Ensure config has been set up
    hdAssert(hdConfig::ConfigWasInitialized());

    // Load user config
    LoadPlayerConfigFile();

    loadingController = new LoadingController(this);

#if DEBUG == 1
    if (getenv("START_WORLD_OVERRIDE_PATH"))
    {
        m_levelPickerWorld = WorldManager::Instance()->FindTotemWorld(getenv("START_WORLD_OVERRIDE_PATH"));
        const char *levIdStr = getenv("START_WORLD_OVERRIDE_LEVELID");
        m_levelId = (levIdStr) ? hdClamp(atoi(levIdStr), 0, m_levelPickerWorld->GetLevelCount() - 1) : 0;
        Level *lev = m_levelPickerWorld->GetLevels()[m_levelId];

        EnsureGameController();
        gameController->InitLevel(lev, 0);
        gameController->SetLevelStats(NULL);
        activeController = gameController;
        gameController->AnimateShow();
        return;
    }
    else
    {
        introController = new IntroController(this);
        activeController = introController;
        introController->AnimateShow();
        menuController = NULL;
    }
#else
    introController = new IntroController(this);
    activeController = introController;
    introController->AnimateShow();
    menuController = NULL;
#endif
}


void AppController::IntroFinished()
{
    delete introController;
    introController = NULL;

    EnsureMenuController();

    activeController = menuController;
    activeController->AnimateShow();
}


void AppController::ShowLevelPickerController()
{
    hdAssert(menuController != NULL);

    FreeMemory();

    m_worldTag = menuController->GetWorldTag();
    delete menuController;
    menuController = NULL;

    hdAssert(m_worldTag >= 0 && m_worldTag <= HACK_MAX_WORLD_TAG);
    EnsureLevelPickerController();
    InitLevelPickerWorld();

    levelPickerController->ShowLobbyAnimation();
    levelPickerController->ShowWorldTitleMessage(m_worldTag);

    activeController = levelPickerController;
    activeController->AnimateShow();
}


void AppController::InitLevelPickerWorld()
{
    char filename[64];
    char fpath[256];

    if (m_levelPickerWorld != NULL)
    {
        hdAssert(0 == WorldManager::Instance()->DestroyTotemWorld(m_levelPickerWorld));
    }

    snprintf(filename, 64, Scripts_GetStringForKey("Chapter_World_File_Pattern").c_str(), m_worldTag);
    m_levelPickerWorld = WorldManager::Instance()->FindTotemWorld(filename);

    hdAssert(m_levelPickerWorld != NULL);

    snprintf(fpath, 256, Scripts_GetStringForKey("Chapter_Script_File_Pattern").c_str(), m_worldTag);
    Scripts_LoadScript(fpath, m_worldTag);

    if (levelPickerController == NULL) return;

    levelPickerController->SetWorld(m_levelPickerWorld, m_worldTag);
    levelPickerController->InitLevel(m_levelPickerWorld->GetLevels()[0], 0);
}


void AppController::EnsureLevelPickerController()
{
    if (levelPickerController == NULL)
    {
        levelPickerController = new LevelPickerController(this);
        levelPickerController->SetOrientation(this->GetOrientation());
    }
    hdAssert(levelPickerController);
}


void AppController::EnsureGameController()
{
    if (gameController == NULL)
    {
        gameController = new GameController(this);
        gameController->SetOrientation(this->GetOrientation());
    }
    hdAssert(gameController);
}


void AppController::EnsureMenuController()
{
    char fpath[256];
    World *menuworld = NULL;
    if (!menuController)
    {
        menuController = new MenuController(this);
        WorldManager::TearDown();

        menuworld = WorldManager::Instance()->FindTotemWorld(Scripts_GetStringForKey("Menu_World_File").c_str());

        hdAssert(menuworld != NULL);
        hdAssert(menuworld->GetLevelCount() > 0);

        Scripts_LoadScript(Scripts_GetStringForKey("Menu_Script_File").c_str(), -1);

        menuController->SetOrientation(this->GetOrientation());
        menuController->InitLevel(menuworld->GetLevels()[0], 0);
    }
    hdAssert(menuController);
}


void AppController::DispatchMessages()
{
    if (levelPickerController != NULL)
    {
        if (m_showNextWorldUnlockedMessage)
        {
            // set the message
            levelPickerController->ShowNextWorldUnlockedMessage(m_worldTag + 1);
            m_showNextWorldUnlockedMessage = false;
        }
    }
}


void AppController::LevelPickerControllerLevelClicked()
{
    hdAssert(levelPickerController != NULL);
    hdAssert(levelPickerController->GetSelectedLevel() != NULL);

    FreeMemory();

    m_levelId = levelPickerController->GetSelectedLevelId();
    delete levelPickerController;
    levelPickerController = NULL;
    InitLevelPickerWorld();
    m_selectedLevelPickerLevel = m_levelPickerWorld->GetLevels()[m_levelId];

    EnsureGameController();
    gameController->InitLevel(m_selectedLevelPickerLevel, m_levelId);
    gameController->SetLevelStats(GetStatsForLevel(m_levelPickerWorld, m_selectedLevelPickerLevel));
    activeController = gameController;
    gameController->AnimateShow();

    DispatchMessages();
}


void AppController::LevelPickerBackClicked()
{
    hdAssert(levelPickerController != NULL);

    // Saves everything...
    if (-1 == TowerPlayerStateController::Instance()->SaveChanges())
    {
        hdError(-1, "There was a fault with the file system via TowerPlayerStateController::Instance()->SaveChanges(). Bailing out\n");
        hdAssert(false);
    }
    FreeMemory();

    EnsureMenuController();
    activeController = menuController;
    activeController->AnimateShow();

    // Delete the level picker controller to free up memory
    delete levelPickerController;
    levelPickerController = NULL;

    if (gameController == NULL) return;
    delete gameController;
    gameController = NULL;

    DispatchMessages();
}


void AppController::GameControllerStopCurrentLevel()
{
    hdAssert(gameController != NULL);
    hdAssert(gameController->GetCurrentLevel() != NULL);
    hdAssert(m_levelPickerWorld != NULL);

    FreeMemory();

    // Update the game stats from the recently played level
    UpdateGameStats(m_levelPickerWorld, gameController->GetCurrentLevel(), gameController->gameInfo);
    SaveGameState();

    // Show the level picker controller.
    EnsureLevelPickerController();
    InitLevelPickerWorld();
    levelPickerController->PanToNewLevelBlock();
    levelPickerController->ShowNewLevelMessage(gameController->GetLevelStats());
    activeController = levelPickerController;
    activeController->AnimateShow();

    // Free up the game controller resources
    delete gameController;
    gameController = NULL;

    DispatchMessages();
}


void AppController::GameControllerRestartCurrentLevel()
{
#if DEBUG == 1
    if (getenv("START_WORLD_OVERRIDE_PATH"))
    {
        delete gameController;
        gameController = NULL;
        activeController = NULL;

        Level *lev = m_levelPickerWorld->GetLevels()[m_levelId];

        EnsureGameController();
        gameController->InitLevel(lev, m_levelId);
        gameController->SetLevelStats(NULL);
        activeController = gameController;
        gameController->AnimateShow();
        DispatchMessages();
        return;
    }
#endif

    // Complete rebuild the game controller
    // No need to free asset memory though - every texture/sound/font
    // should still be in memory. Recreating will just get handles to these.
    hdAssert(gameController != NULL);
    hdAssert(gameController->GetCurrentLevel() != NULL);
    hdAssert(m_levelPickerWorld != NULL);

    UpdateGameStats(m_levelPickerWorld, gameController->GetCurrentLevel(), gameController->gameInfo);

    delete gameController;
    gameController = NULL;
    activeController = NULL;

    InitLevelPickerWorld();
    m_selectedLevelPickerLevel = m_levelPickerWorld->GetLevels()[m_levelId];

    EnsureGameController();
    gameController->InitLevel(m_selectedLevelPickerLevel, m_levelId);
    gameController->SetLevelStats(GetStatsForLevel(m_levelPickerWorld, m_selectedLevelPickerLevel));
    activeController = gameController;
    gameController->AnimateShow();

    DispatchMessages();
}


void AppController::GameControllerQuitCurrentLevel()
{
    hdAssert(gameController != NULL);
    hdAssert(gameController->GetCurrentLevel() != NULL);
    hdAssert(m_levelPickerWorld != NULL);

    FreeMemory();

    // Show the level picker controller.
    EnsureLevelPickerController();
    InitLevelPickerWorld();
    levelPickerController->PanToNewLevelBlock();
    activeController = levelPickerController;
    activeController->AnimateShow();

    // Free up the game controller resources
    delete gameController;
    gameController = NULL;

    DispatchMessages();
}


void AppController::SetLoading()
{
    loadingController->SetOrientation(this->GetOrientation());
    activeController = loadingController;
    loadingController->Reset();
    loadingController->AnimateShow();
}


const TotemLevelStats* AppController::GetStatsForLevel(const World* currentWorld,
                                                       const Level *currentLevel) const
{
    TotemWorldStats* stats = TowerPlayerStateController::Instance()->GetStatsForWorld(currentWorld, e_hdkDifficultyEasy);
    TotemLevelStats* levelStats = NULL;

    /* Find the level stats for the current level */
    for (int i = 0; i < stats->GetLevels()->GetItemCount(); i++)
    {
        if (strncmp(currentLevel->GetLevelName(), stats->GetLevels()->GetItems()[i]->levelName, kMaxLevelNameSize) == 0)
        {
            levelStats = stats->GetLevels()->GetItems()[i];
            break;
        }
    }

    return levelStats;
}


void AppController::UpdateGameStats(const World* currentWorld,
                                    const Level *currentLevel,
                                    const GameInfo& info)
{
    char worldFileName[256];
    World *next;
    TotemWorldStats *stats, *nextStats;
    TotemLevelStats *levelStats;

    /* TODO: difficulty levels  */
    stats = TowerPlayerStateController::Instance()->GetStatsForWorld(currentWorld, e_hdkDifficultyEasy);
    levelStats = NULL;
    next = NULL;
    nextStats = NULL;

    /* Find the level stats for the current level */
    for (int i = 0; i < stats->GetLevels()->GetItemCount(); i++)
    {
        if (strncmp(currentLevel->GetLevelName(), stats->GetLevels()->GetItems()[i]->levelName, kMaxLevelNameSize) == 0)
        {
            levelStats = stats->GetLevels()->GetItems()[i];
            break;
        }
    }

    // TODO: Serious error - no stats found for the level just played.
    if (levelStats == NULL) return;

    levelStats->TouchLevel();
    levelStats->UpdateFromGameInfo(info);

    // Once the level is completed, it's _completed_
    // Activate the next level if all the goals were completed.
    if (levelStats->IsCompleted() == false)
    {
        if (info.allGoalsCompleted)
        {
            levelStats->CompleteLevel();
            //levelStats->TouchLevel();

            // Activating stats - if the next world was unlocked
            // by this action, then we show an event.
            bool nextWorldWasPreviouslyLocked = stats->isNextLocked();
            stats->ActivateNextLevel();
            bool nextWorldIsNowUnlocked = !stats->isNextLocked();

            if (nextWorldWasPreviouslyLocked && nextWorldIsNowUnlocked)
            {
                // Check if there is actually a world to unlock.
                snprintf(worldFileName, 256, Scripts_GetStringForKey("Chapter_World_File_Pattern").c_str(), m_worldTag + 1);

                next = WorldManager::Instance()->FindTotemWorld(worldFileName);

                // If there's no world to unlock
                if (NULL == next)
                {
                    m_showNextWorldUnlockedMessage = false;
                }
                else
                {
                    nextStats = TowerPlayerStateController::Instance()->GetStatsForWorld(next, e_hdkDifficultyEasy);

                    if (NULL == nextStats)
                    {
                        // Player has finished the final level.
                        m_showNextWorldUnlockedMessage = false;
                    }
                    else
                    {
                        m_showNextWorldUnlockedMessage = true;
                    }
                }
            }
        }
    }
}


void AppController::SaveGameState()
{
    if (activeController == gameController && levelPickerController != NULL)
    {
        TowerPlayerStateController::Instance()->SetCurrentLevelInfo(gameController->gameInfo, levelPickerController->GetCurrentLevel());
    }
    else
    {
        TowerPlayerStateController::Instance()->ClearCurrentLevelInfo();
    }

    // Saves everything...
    if (-1 == TowerPlayerStateController::Instance()->SaveChanges())
    {
        hdError(-1, "There was a fault with the file system >> TowerPlayerStateController::Instance()->SaveChanges(). Bailing out\n");
        hdAssert(false);
    }

    // Save user config settings
    if (!hdPlayerConfig::SaveConfigFile())
    {
        hdError(-1, "Could not save player config settings.\n");
    }
}


void AppController::LoadPlayerConfigFile()
{
    if (!hdPlayerConfig::LoadConfigFile())
    {
        // Set some defaults
        hdError(-1, "Could not load config file: setting default config values.\n");

        hdPlayerConfig::SetValue("PlayMusic", "On");
        hdPlayerConfig::SetValue("PlaySounds", "On");
        hdPlayerConfig::SetValue("PlayVibrations", "On");
        hdPlayerConfig::SetValue("FlipScreen", "On");
    }

    if (hdPlayerConfig::GetValue("PlayMusic") == "On")
    {
        hdSoundManager::SetMusicMaxVolume(1.0f);
    }
    else
    {
        hdSoundManager::SetMusicMaxVolume(0.0f);
    }

    if (hdPlayerConfig::GetValue("PlaySounds") == "On")
    {
        hdSoundManager::SetSoundMaxVolume(1.0f);
    }
    else
    {
        hdSoundManager::SetSoundMaxVolume(0.0f);
    }
    
    if (hdPlayerConfig::GetValue("PlayVibrations") == "On")
    {
        hdSoundManager::SetVibrationsOn();
    } 
    else
    {
        hdSoundManager::SetVibrationsOff();
    }
    
}


void AppController::FreeIdleMemory()
{
    SaveGameState();
}


void AppController::TearDown()
{
}


void AppController::FreeMemory()
{
    hdFontManager::TearDown();
    hdTextureManager::TearDown();
    hdSoundManager::DestroySounds();
}


void AppController::Step(double interval)
{
    hdAnimationController::Instance()->StepWithInterval(interval);
    activeController->Step(interval);
}


void AppController::Step(double sysInterval, double fixedInterval)
{
#if (TARGET_IPHONE_SIMULATOR == 1) || (TARGET_OS_IPHONE == 1)
    hdAnimationController::Instance()->StepWithInterval(hdClamp(sysInterval, 0.016, 0.04));
#else
    hdAnimationController::Instance()->StepWithInterval(hdMin(fixedInterval, 0.04));
#endif
    activeController->Step(sysInterval);
}


void AppController::Draw()
{
    hdAssert(activeController != NULL);
    activeController->Draw();
}


void AppController::HandleSingleTap(float x, float y)
{
    hdInterfaceContext::HandleSingleTap(x, y);
}


void AppController::HandleDoubleTap(float x, float y)
{
    hdInterfaceContext::HandleDoubleTap(x, y);
}


void AppController::HandleTapUp(float x, float y, int tapCount)
{
    hdInterfaceContext::HandleTapUp(x, y, tapCount);
}


void AppController::HandleTapDown(float x, float y, int tapCount)
{
    hdInterfaceContext::HandleTapDown(x, y, tapCount);
}


void AppController::HandleTapMovedSingle(const float previousX, const float previousY, 
                                         const float currentX, const float currentY)
{
    hdInterfaceContext::HandleTapMovedSingle(previousX, previousY, currentX, currentY);
}


void AppController::HandleTapMovedDouble(const float aPreviousX, const float aPreviousY,
                                         const float aCurrentX, const float aCurrentY,
                                         const float bPreviousX, const float bPreviousY,	
                                         const float bCurrentX, const float bCurrentY)
{
    hdInterfaceContext::HandleTapMovedDouble(aPreviousX, aPreviousY, aCurrentX, aCurrentY, 
                                             bPreviousX, bPreviousY, bCurrentX, bCurrentY);
}


void AppController::HandleResize(const float newWidth, const float newHeight)
{
    if (activeController == NULL) return;
    
    activeController->HandleResize(newWidth, newHeight);
}


std::string AppController::GetCurrentTotemWorldName() const
{
    char filename[64];
    snprintf(filename, 64, "smashed%d", m_worldTag);
    std::string s(filename);
    return s;
}
