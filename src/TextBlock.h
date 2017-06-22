/*
 * Copyright (c) 2017 Hackdirt Ltd.
 * Author: David Petrie (david@davidpetrie.com)
 *
 * License: https://www.gnu.org/licenses/gpl-3.0.en.html
 */

#ifndef _HDK_TEXT_BLOCK_H_
#define _HDK_TEXT_BLOCK_H_

#include <hdk/game/GameBlock.h>
#include <hdk/ui.h>
#include <hdk/scripting.h>

class TextBlock : public Block
{
public:
    TextBlock(Block *parent, const int levelId);

    ~TextBlock();

    void Step();

    void Draw() const;

private:
    void InitInterface();

    Block *m_parent;

    hdFontPolygon *m_TextFont;
    
    int m_levelId;

};

#endif
