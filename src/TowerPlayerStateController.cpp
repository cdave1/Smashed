/*
 * Copyright (c) 2017 Hackdirt Ltd.
 * Author: David Petrie (david@davidpetrie.com)
 *
 * License: https://www.gnu.org/licenses/gpl-3.0.en.html
 */

#include "TowerPlayerStateController.h"


static TowerPlayerStateController* m_playerStateControllerInstance = NULL;


TowerPlayerStateController* TowerPlayerStateController::Instance()
{
    if (m_playerStateControllerInstance == NULL)
    {
        m_playerStateControllerInstance = new TowerPlayerStateController();
    }
    return m_playerStateControllerInstance;
}


/*
 * TODO: File decryption.
 */
TowerPlayerStateController::TowerPlayerStateController()
{
    void *buffer;
    int len, res;

    if (false == FileSystem_FileExists("usersave.001"))
    {
        // No user save file exists.
        m_gameStats = new TotemGameStats();
        m_gameStats->Init();
        return;
    }
    else
    {
        res = FileSystem_ReadFromFile(&buffer, &len, "usersave.001");
        if (res != 0)
        {
            m_gameStats = new TotemGameStats();
            m_gameStats->Init();
            return;
        }
        else
        {
            m_gameStats = new TotemGameStats();
            std::istringstream iss(std::ios::in | std::ios::out);
            iss.rdbuf()->sputn((const char*)buffer, len);
            boost::archive::binary_iarchive ia(iss);
            ia >> (* m_gameStats);
            free(buffer);
        }
    }
}


TotemWorldStats* TowerPlayerStateController::GetStatsForWorld(const World *world,
                                                              const e_hdkDifficultyLevel difficultyLevel)
{
    TotemWorldStats* worldStats;

    if (world->GetName().length() == 0) return NULL;
    if (world->GetLevelCount() == 0) return NULL;

    worldStats = NULL;
    for (int i = 0; i < m_gameStats->GetWorldStats()->GetItemCount(); i++)
    {
        if (m_gameStats->GetWorldStats()->GetItems()[i]->GetWorldName() == world->GetName() &&
            m_gameStats->GetWorldStats()->GetItems()[i]->GetDifficulty() == difficultyLevel)
        {
            worldStats =  m_gameStats->GetWorldStats()->GetItems()[i];
            break;
        }
    }

    if (worldStats == NULL)
    {
        worldStats = new TotemWorldStats(world, difficultyLevel);

        if (m_gameStats->GetWorldStats()->Add(worldStats) == -1)
        {
            hdError(-1, "World stats list is full - game has too many worlds.");
            return NULL;
        }
    }

    return worldStats;
}


void TowerPlayerStateController::SetCurrentLevelInfo(const GameInfo& gameInfo, const Level *level)
{
    if (m_gameStats == NULL) return;

    m_gameStats->m_currentGameInfo = gameInfo;
    m_gameStats->m_currentLevel = (Level*)level;
    m_gameStats->m_isLevelInProgress = true;
}


/*
 * Nulls the current level info.
 */
void TowerPlayerStateController::ClearCurrentLevelInfo()
{
    m_gameStats->m_currentLevel = NULL;
    m_gameStats->m_isLevelInProgress = NULL;
}


/*
 * TODO: File encryption.
 */
int TowerPlayerStateController::SaveChanges() const
{
    int createRes, writeRes;
    char *ossBuffer;
    long ossLen;

    if (!FileSystem_FileExists("usersave.001"))
    {
        createRes = FileSystem_CreateFile("usersave.001");
        if (0 != createRes)
            return createRes;
    }

    std::ostringstream oss(std::ios::in | std::ios::out);
    boost::archive::binary_oarchive boa(oss);
    boa << (* m_gameStats);

    ossLen = oss.str().size() + 1;
    if ((ossBuffer = new char[ossLen]) == NULL)
    {
        printf("TowerPlayerStateController::SaveChanges: Out of memory.\n");
        return -1;
    }

    oss.rdbuf()->sgetn(ossBuffer, ossLen);
    writeRes = FileSystem_WriteToFile("usersave.001", ossBuffer, ossLen);
    delete [] ossBuffer;

    return writeRes;
}


const TotemWorldStats * TowerPlayerStateController::GetStatsForWorld(const int& worldNumber)
{
    TotemWorldStats* worldStats;
    char tmp[256];

    hdAssert(m_gameStats != NULL);

    // If 1, no
    if (worldNumber < 1) return false;

    snprintf(tmp, 256, Scripts_GetStringForKey("Chapter_World_Name_Pattern").c_str(), worldNumber);
    worldStats = NULL;
    for (int i = 0; i < m_gameStats->GetWorldStats()->GetItemCount(); i++)
    {
        if (strcmp(m_gameStats->GetWorldStats()->GetItems()[i]->GetWorldName().c_str(), tmp) == 0)
        {
            worldStats =  m_gameStats->GetWorldStats()->GetItems()[i];
            break;
        }
    }
    
    return worldStats;
}


const bool TowerPlayerStateController::IsWorldLocked(const int& worldNumber)
{
    TotemWorldStats* worldStats;
    hdAssert(m_gameStats != NULL);
    
    // If 1, no
    if (worldNumber <= 1) return false;
    
    worldStats = (TotemWorldStats *)TowerPlayerStateController::Instance()->GetStatsForWorld(worldNumber-1);
    
    // null means the previous world has not actually been played
    if (worldStats == NULL) return true;
    
    return worldStats->isNextLocked();
}
