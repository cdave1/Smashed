/*
 * Copyright (c) 2017 Hackdirt Ltd.
 * Author: David Petrie (david@davidpetrie.com)
 *
 * License: https://www.gnu.org/licenses/gpl-3.0.en.html
 */

#ifndef _HDK_LEVEL_PICKER_BLOCK_
#define _HDK_LEVEL_PICKER_BLOCK_

#include "Box2D.h"
#include <hdk/game.h>
#include <hdk/ui.h>
#include <hdk/animation.h>
#include <hdk/scripting.h>
#include "TowerPlayerStateClasses.h"

class LevelPickerBlock : public Block
{
public:
    LevelPickerBlock(hdGameWorld *gameWorld, const b2World *physicsWorld,
                          TotemLevelStats* levelStats, Level *level,
                          const hdVec3& startingPos, float halfWidth,
                          hdOrthographicProjection *projection);

    ~LevelPickerBlock();

    void Step();

    void Draw() const;

    void DrawInfo() const;

    void DrawSpecial() const;

    void DrawFonts() const;

    void AnimateNewLevelMessage();

    void UpdateInterface();

    Level *m_level;

    TotemLevelStats *m_levelStats;

    int m_levelId;

    float m_halfWidth;

    hdUIContainer *m_levelInfoContainer;

    hdFontPolygon *m_fontPolygon;


private:

    void InitInterface();

    void InitAnimations();

    hdOrthographicProjection *m_projection;

    hdAnimation *m_messageFadeAnim;
    hdAlphaAction *m_messageFadeIn;

    hdAnimation *m_messageBounceAnim;
    hdVectorAction *m_messageBounceAction;
    
    hdUIImage *m_glow;
    hdUIImage *m_messagePolygon;
    
    hdUIImage *m_imgBronzeStar;
    hdUIImage *m_imgSilverStar;	
    hdUIImage *m_imgGoldStar;
    
    hdUIImage *m_imgBarWhite;
    hdFontPolygon *m_lvlStatsPolygon;

};

#endif
