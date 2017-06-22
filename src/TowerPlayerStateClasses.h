/*
 * Copyright (c) 2017 Hackdirt Ltd.
 * Author: David Petrie (david@davidpetrie.com)
 *
 * License: https://www.gnu.org/licenses/gpl-3.0.en.html
 */

#ifndef _TOWER_PLAYER_STATE_CLASSES_H_
#define _TOWER_PLAYER_STATE_CLASSES_H_

#include <hdk/game.h>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

typedef enum
{
    e_levelFlagsPlayable = 0x0001,
    e_levelFlagsTouched = 0x0002,
    e_levelFlagsCompleted = 0x0004,
    e_levelFlagsBronzeAchievement = 0x0008,
    e_levelFlagsSilverAchievement = 0x0010,
    e_levelFlagsGoldAchievement = 0x0020
} e_hdkLevelStatsFlags;

struct GameInfo
{
    GameInfo() :
    gamePoints(0),
    destroyableBlocks(0),
    blocksTotal(0),
    destroyedBlocksCurrent(0),
    lostBlocksCurrent(0),

    fallSmashesTotal(0),
    fallSmashesCurrent(0),

    midAirSmashesTotal(0),
    midAirSmashesCurrent(0),
    riskySmashesTotal(0),
    riskySmashesCurrent(0),

    totalMidAirChains(0),
    longestMidAirChain(0),


    fragmentSmashCount(0),

    jacksCollected(0),
    jacksLost(0),
    jacksTotal(0),

    gameStarted(false),
    totalProportionComplete(0.0f),
    allGoalsCompleted(false),
    failEvent(false),
    newHighScore(false),

    goalsCompleted(0)

    {}

    int gamePoints;
    int destroyableBlocks;
    int blocksTotal;
    int destroyedBlocksCurrent;
    int lostBlocksCurrent;

    int fallSmashesTotal;
    int fallSmashesCurrent;

    int midAirSmashesTotal;
    int midAirSmashesCurrent;
    int riskySmashesTotal;
    int riskySmashesCurrent;

    int totalMidAirChains;
    int longestMidAirChain;

    int fragmentSmashCount;

    int jacksCollected;
    int jacksLost;
    int jacksTotal;

    bool gameStarted;
    float totalProportionComplete;
    bool allGoalsCompleted;
    bool failEvent;
    bool newHighScore;

    int goalsCompleted;

    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        if (version >= 0)
        {
            ar & gamePoints;
            ar & destroyableBlocks;
            ar & blocksTotal;
            ar & destroyedBlocksCurrent;
            ar & lostBlocksCurrent;

            ar & fallSmashesTotal;
            ar & fallSmashesCurrent;

            ar & midAirSmashesTotal;
            ar & midAirSmashesCurrent;
            ar & riskySmashesTotal;
            ar & riskySmashesCurrent;

            ar & totalMidAirChains;
            ar & longestMidAirChain;


            ar & fragmentSmashCount;

            ar & jacksCollected;
            ar & jacksLost;
            ar & jacksTotal;

            ar & gameStarted;
            ar & totalProportionComplete;
            ar & allGoalsCompleted;
            ar & failEvent;
            ar & newHighScore;
        }

        if (version >= 1)
        {
            ar & goalsCompleted;
        }
    }
};

BOOST_CLASS_VERSION(GameInfo, 1)


class TotemLevelStats
{
public:
    TotemLevelStats();

    TotemLevelStats(const char* levelName);

    ~TotemLevelStats() {}

    void UpdateFromGameInfo(const GameInfo& info);

    void ActivateLevel();

    void TouchLevel();

    void CompleteLevel();

    bool IsActive() const;

    bool IsTouched() const;

    bool IsCompleted() const;

    int levelNumber;
    char levelName[kMaxLevelNameSize];

    int highestScore;
    long totalPoints;

    int totalMidair;
    int totalRisky;
    int longestMidairChain;
    int longestRiskyChain;
    int totalLostBlocks;
    int totalSmashedBlocks;

    hdVec3 worldCenter;

    int attemptsCount;
    int completionCount;
    int highestCompletionPercentage;


    const uint32 GetFlags() const;

    const bool ContainsFlag(const uint32 flag) const;

    void AddFlag(const uint32 flag);

    const bool RemoveFlag(const uint32 flag);

    void SetFlag(const uint32 flag);


private:
    void Init(const char *levName);

    uint32 m_levelFlags;

    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & levelNumber;
        ar & levelName;
        ar & highestScore;
        ar & totalPoints;
        ar & totalMidair;
        ar & totalRisky;
        ar & longestMidairChain;
        ar & longestRiskyChain;
        ar & totalLostBlocks;
        ar & totalSmashedBlocks;
        ar & worldCenter.x;
        ar & worldCenter.y;
        ar & worldCenter.z;
        ar & m_levelFlags;

        if (version >= 2)
        {
            ar & attemptsCount;
            ar & highestCompletionPercentage;
        }

        if (version >= 3)
        {
            ar & completionCount;
        }
    }
};
BOOST_CLASS_VERSION(TotemLevelStats, 2)


inline void TotemLevelStats::ActivateLevel()
{
    this->AddFlag(e_levelFlagsPlayable);
}


inline void TotemLevelStats::TouchLevel()
{
    this->AddFlag(e_levelFlagsTouched);
}


inline void TotemLevelStats::CompleteLevel()
{
    this->AddFlag(e_levelFlagsCompleted);
}


inline bool TotemLevelStats::IsActive() const
{
    return this->ContainsFlag(e_levelFlagsPlayable);
}


inline bool TotemLevelStats::IsTouched() const
{
    return this->ContainsFlag(e_levelFlagsTouched);
}


inline bool TotemLevelStats::IsCompleted() const
{
    return this->ContainsFlag(e_levelFlagsCompleted);
}


inline const uint32 TotemLevelStats::GetFlags() const
{
    return m_levelFlags;
}


inline const bool TotemLevelStats::ContainsFlag(const uint32 flag) const
{
    return ((m_levelFlags & flag) == flag);
}


inline void TotemLevelStats::AddFlag(const uint32 flag)
{
    m_levelFlags |= flag;
}


inline const bool TotemLevelStats::RemoveFlag(const uint32 flag)
{
    if (this->ContainsFlag(flag))
    {
        m_levelFlags ^= flag;
        return true;
    }
    return false;
}


inline void TotemLevelStats::SetFlag(const uint32 flag)
{
    m_levelFlags = flag;
}



class TotemWorldStats
{
public:
    TotemWorldStats();

    TotemWorldStats(const World *world,
                    const e_hdkDifficultyLevel difficultyLevel);

    ~TotemWorldStats();

    std::string GetWorldName() const;

    const e_hdkDifficultyLevel GetDifficulty() const;

    hdSerializablePointerList<TotemLevelStats, 64>* GetLevels() const;

    const bool isNextLocked() const;

    void SetNextLocked(const bool isNextLocked);

    /*
     * Deletes any stats that do not match levels in the given world.
     * Creates stats for new levels.
     */
    void MergeStatsWithWorld(const World *world);

    /*
     * Activate the next inactive level in the stats, and then
     * returns a pointer to the corresponding Level.
     */
    TotemLevelStats* ActivateNextLevel();

private:
    void Init(const World *world,
              const e_hdkDifficultyLevel difficultyLevel);

    std::string m_worldName;

    e_hdkDifficultyLevel m_difficultyLevel;

    hdSerializablePointerList<TotemLevelStats, 64>* m_levels;

    bool m_isNextLocked;

    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & m_worldName;
        ar & m_difficultyLevel;
        ar & m_levels;
        ar & m_isNextLocked;
    }
};
BOOST_CLASS_VERSION(TotemWorldStats, 0)


inline std::string TotemWorldStats::GetWorldName() const
{
    return m_worldName;
}


inline const e_hdkDifficultyLevel TotemWorldStats::GetDifficulty() const
{
    return m_difficultyLevel;
}


inline 	const bool TotemWorldStats::isNextLocked() const
{
    return m_isNextLocked;
}


inline void TotemWorldStats::SetNextLocked(const bool isNextLocked)
{
    m_isNextLocked = isNextLocked;
}


inline hdSerializablePointerList<TotemLevelStats, 64>* TotemWorldStats::GetLevels() const
{
    return m_levels;
}


/*
 * A container for world stats
 *
 * Also stores info about what level the player is playing and
 * the specific level in progress, if applicable.
 */
class TotemGameStats
{
public:
    TotemGameStats();
    
    ~TotemGameStats();
    
    void Init();
    
    hdSerializablePointerList<TotemWorldStats, 32>* GetWorldStats();
    
private:
    friend class TowerPlayerStateController;
    
    int m_currentWorldId;
    
    Level *m_currentLevel;
    
    GameInfo m_currentGameInfo;
    
    bool m_isLevelInProgress;
    
    hdSerializablePointerList<TotemWorldStats, 32>* m_worldStats;
    
    friend class boost::serialization::access;
    
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        if (version >= 0)
        {
            ar & m_currentWorldId;
            ar & m_isLevelInProgress;
            ar & m_currentLevel;
            ar & m_currentGameInfo;
            ar & m_worldStats;
        }
    }
};
BOOST_CLASS_VERSION(TotemGameStats, 0)


inline hdSerializablePointerList<TotemWorldStats, 32>* TotemGameStats::GetWorldStats()
{
    return m_worldStats;
}

#endif
