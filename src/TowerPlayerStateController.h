/*
 * Copyright (c) 2017 Hackdirt Ltd.
 * Author: David Petrie (david@davidpetrie.com)
 *
 * License: https://www.gnu.org/licenses/gpl-3.0.en.html
 */

#ifndef _TOWER_PLAYER_STATE_CONTROLLER_H_
#define _TOWER_PLAYER_STATE_CONTROLLER_H_

#include <fstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <hdk/game.h>
#include <hdk/scripting.h>
#include "TowerPlayerStateClasses.h"

/*
 * HACK HACK HACK
 *
 * This whole player state stuff is a total dog's breakfast. The
 * way I've designed it makes it very difficult to reason about
 * what's actually going on. We can't easily find out things like
 * - Has the player completed a level
 * - Has the player completed the final level for a world.
 * - Has the player finished all the worlds?
 * etc etc
 *
 * Think of a better way to do this.
 */
class TowerPlayerStateController
{
public:
    static TowerPlayerStateController* Instance();

    static void TearDown();

    /*
     * checks if it has an instance of TotemWorldStats for the given
     * world and given difficulty level. If not, then the player state controller
     * will create the stats collection to return.
     *
     * Returns NULL if:
     * - there are no levels in the given world
     * - the world name is empty
     */
#warning "Difficulty level has been deprecated and is a source of errors.  Remove ASAP!"
    TotemWorldStats* GetStatsForWorld(const World *world,
                                      const e_hdkDifficultyLevel difficultyLevel);

    /*
     * Saves the contents of *gameStats out to disk.
     *
     * Return 0 on success; -1 if there is a fault with the file system.
     */
    int SaveChanges() const;

    /*
     * Sets the level info in the game stats class. Called if the player
     * is in the middle of their game.
     */
    void SetCurrentLevelInfo(const GameInfo& gameInfo, const Level *level);

    /*
     * Nulls the current level info.
     */
    void ClearCurrentLevelInfo();

    const bool IsWorldLocked(const int& worldNumber);

    const TotemWorldStats * GetStatsForWorld(const int& worldNumber);

protected:
    /*
     * When the game loads, this function will attempt to load
     * the user saves file. If the file is not present we create
     * a new gameStats class.
     */
    TowerPlayerStateController();
    
    ~TowerPlayerStateController();
    
private:
    
    /*
     * Holds all the game info - serialise this object on save.
     */
    TotemGameStats* m_gameStats;

};

#endif
