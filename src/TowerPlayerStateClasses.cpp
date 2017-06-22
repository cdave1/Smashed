/*
 * Copyright (c) 2017 Hackdirt Ltd.
 * Author: David Petrie (david@davidpetrie.com)
 *
 * License: https://www.gnu.org/licenses/gpl-3.0.en.html
 */

#include "TowerPlayerStateClasses.h"

TotemLevelStats::TotemLevelStats()
{
    snprintf(levelName, kMaxLevelNameSize, "");
    this->Init("");
}


TotemLevelStats::TotemLevelStats(const char *levName)
{

    this->Init(levName);
}


void TotemLevelStats::Init(const char *levName)
{
    levelNumber = 0;
    memset(levelName, 0, sizeof(char) * kMaxLevelNameSize);
    snprintf(levelName, kMaxLevelNameSize, "%s", levName);

    highestScore = 0;
    totalPoints = 0;

    totalMidair = 0;
    totalRisky = 0;
    longestMidairChain = 0;
    longestRiskyChain = 0;
    totalLostBlocks = 0;
    totalSmashedBlocks = 0;
    m_levelFlags = 0;

    worldCenter.Set(0, 0, 0);

    attemptsCount = 0;
    completionCount = 0;
    highestCompletionPercentage = 0;
}


void TotemLevelStats::UpdateFromGameInfo(const GameInfo& info)
{
    highestScore = hdMax(highestScore, info.gamePoints);
    totalPoints += info.gamePoints;

    totalMidair += info.midAirSmashesTotal;
    totalRisky += info.riskySmashesTotal;
    longestMidairChain = hdMax(longestMidairChain, info.longestMidAirChain);
    longestRiskyChain = 0;
    totalLostBlocks += info.lostBlocksCurrent;
    totalSmashedBlocks += info.destroyedBlocksCurrent;

    attemptsCount += 1;

    if (info.allGoalsCompleted)
    {
        completionCount += 1;
    }

    highestCompletionPercentage = hdClamp((int)(100 * info.totalProportionComplete), highestCompletionPercentage, 100);
}


TotemWorldStats::TotemWorldStats()
{
    m_worldName = "";
    m_isNextLocked = true;
    m_difficultyLevel = e_hdkDifficultyEasy;
}


TotemWorldStats::~TotemWorldStats()
{
    if (m_levels != NULL)
    {
        delete m_levels;
    }
}


TotemWorldStats::TotemWorldStats(const World *world,
                                 const e_hdkDifficultyLevel difficultyLevel)
{
    m_worldName = "";
    m_isNextLocked = true;
    this->Init(world, difficultyLevel);
}


void TotemWorldStats::Init(const World *world,
                           const e_hdkDifficultyLevel difficultyLevel)
{
    m_levels = new hdSerializablePointerList<TotemLevelStats, 64>();
    m_worldName = world->GetName();
    m_difficultyLevel = difficultyLevel;

    hdAssert(world->GetName().length() > 0);
    hdAssert(world->GetLevelCount() > 0);

    TotemLevelStats *levelStats = NULL;

    // For each of the levels in the world, do the following:
    // - Create a TotemLevelStats for the world with the level id.
    for (int i = HDK_GAME_STARTING_LEVEL; i < world->GetLevelCount(); i++)
    {
        levelStats = new TotemLevelStats(world->GetLevels()[i]->GetLevelName());
        levelStats->levelNumber = i;
        if (m_levels->Add(levelStats) == -1)
        {
            hdError(-1, "Too many levels in the world to add to world set");
            break;
        }
    }

    // Make sure things were created proper.
    hdAssert(m_levels->GetItemCount() == (world->GetLevelCount() - HDK_GAME_STARTING_LEVEL) || m_levels->GetItemCount() == 64);

    // Activate the first level
    m_levels->GetItems()[0]->ActivateLevel();
}


void TotemWorldStats::MergeStatsWithWorld(const World *world)
{
    // Go through levels in world, looking for match - if no match for a level, new stats.
    Level	*lev = NULL;
    TotemLevelStats *levStats = NULL;
    int i, j;

    for (i = HDK_GAME_STARTING_LEVEL; i < world->GetLevelCount(); ++i)
    {
        lev = world->GetLevels()[i];
        levStats = NULL;
        for (j = 0; j < m_levels->GetItemCount(); ++j)
        {
            if (strncmp(lev->GetLevelName(), m_levels->GetItems()[j]->levelName, kMaxLevelNameSize) == 0)
            {
                levStats = m_levels->GetItems()[j];
                break;
            }
        }

        /* Means there is a new level, need some stats for it... */
        if (levStats == NULL)
        {
            levStats = new TotemLevelStats(world->GetLevels()[i]->GetLevelName());
            levStats->levelNumber = i;
            if (m_levels->Add(levStats) == -1)
            {
                hdError(-1, "Too many levels in the world to add to world set");
                break;
            }
        }
    }


    // Then go through levels in stats. If no level for stats in world, remove stats.
    for (j = 0; j < m_levels->GetItemCount(); ++j)
    {
        lev = NULL;
        levStats = m_levels->GetItems()[j];

        for (i = HDK_GAME_STARTING_LEVEL; i < world->GetLevelCount(); ++i)
        {
            if (strncmp(world->GetLevels()[i]->GetLevelName(), levStats->levelName, kMaxLevelNameSize) == 0)
            {
                lev = world->GetLevels()[i];
                break;
            }
        }

        /* Stats exists for a level that could not be found */
        if (lev == NULL)
        {
            m_levels->Remove(levStats);
            j -= 1;
        }
    }
}


TotemLevelStats* TotemWorldStats::ActivateNextLevel()
{
    TotemLevelStats* levelStats = NULL;
    
    for (int i = 0; i < m_levels->GetItemCount(); i++)
    {
        if (false == m_levels->GetItems()[i]->IsActive())
        {
            levelStats = m_levels->GetItems()[i];
            break;
        }
    }
    
    /*
     * Assume we've come to the last level in the list - there's nothing more to
     * activate.
     */
    if (levelStats != NULL)	
    {
        levelStats->ActivateLevel();
        return levelStats;
    }
    
    this->SetNextLocked(false);
    return NULL;
}


TotemGameStats::TotemGameStats()
{
    m_worldStats = NULL;
    m_currentLevel = NULL;
    m_currentWorldId = 0;
    m_isLevelInProgress = false;
}


void TotemGameStats::Init()
{
    m_worldStats = new hdSerializablePointerList<TotemWorldStats, 32>();
}


TotemGameStats::~TotemGameStats()
{
    if (m_worldStats != NULL)
    {
        delete m_worldStats;
    }
}
