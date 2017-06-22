/*
 * Copyright (c) 2017 Hackdirt Ltd.
 * Author: David Petrie (david@davidpetrie.com)
 *
 * License: https://www.gnu.org/licenses/gpl-3.0.en.html
 */

#ifndef _HDK_GOAL_BLOCK_H
#define _HDK_GOAL_BLOCK_H

#include <hdk/game.h>
#include <hdk/effects.h>

class GoalBlock : public Block
{
public:
    GoalBlock(Block* parent);

    ~GoalBlock();

    void Step();

    void Draw() const;

    void DrawSpecial() const;

private:
    void InitInterface();

    void InitAnimations();

    Block *m_parent;
    
    WeatherParticleEmitter *m_emberEmitter;

};

#endif
