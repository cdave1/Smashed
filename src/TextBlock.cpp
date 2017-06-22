/*
 * Copyright (c) 2017 Hackdirt Ltd.
 * Author: David Petrie (david@davidpetrie.com)
 *
 * License: https://www.gnu.org/licenses/gpl-3.0.en.html
 */

#include "TextBlock.h"

TextBlock::TextBlock(Block *parent, const int levelId) : Block()
{
    this->SetWorld(NULL);
    this->SetDepth(parent->GetDepth());
    this->SetTag(0);
    this->AddGameFlag((uint32)e_hdkGameFlagsBlockDrawRaw);

    m_parent = parent;
    m_levelId = levelId;

    this->SetAs2DBox(hdVec3(parent->GetAABB().lower.x, parent->GetAABB().upper.y, parent->GetAABB().lower.z),
                     hdVec3(parent->GetAABB().upper.x, parent->GetAABB().upper.y + 1.0f, parent->GetAABB().upper.z));

    this->SetMaterial(e_hdkMaterialRawPolygon);
    this->SetBlockType(e_hdkBlockTypeCosmetic);

    this->InitInterface();
}


TextBlock::~TextBlock()
{
    delete m_TextFont;
}


void TextBlock::InitInterface()
{
    char key[32];
    hdAABB parentAABB;

    parentAABB = m_parent->GetAABB();

    m_TextFont = new hdFontPolygon(CONFIG_TEXTBLOCK_FONT, "", NULL,
                                   parentAABB.lower.x + 0.2f, parentAABB.upper.y,
                                   0.6f * parentAABB.Width(), 1.0f * parentAABB.Height());
    m_TextFont->SetTint(0.0f, 0.0f, 0.0f, 1.0f);
    m_TextFont->SetScale(0.02f);
    m_TextFont->AlignCenter();

    snprintf(key, 32, "TextBlock_%d", m_parent->GetTag());

    m_TextFont->SetText(Scripts_GetStringForKey(key).c_str());

    Step();
}


void TextBlock::Step()
{
    if (m_parent->IsPhysicsActor() == false) return;

    // Static bodies do not translate or rotate

    if (m_parent->GetPhysicsBody() == NULL) return;

    b2XForm xform = m_parent->GetPhysicsBody()->GetXForm();

    m_TextFont->RotateAndTranslate(hdVec3(0.0f, 0.0f, m_parent->GetPhysicsBody()->GetAngle()),
                                   hdVec3(xform.position.x, xform.position.y, 0.0f));
    m_TextFont->ResetAABB();
}


void TextBlock::Draw() const
{
    GLboolean blendEnabled, depthEnabled, depthMaskEnabled;
    GLint blendFuncSrc, blendFuncDst, texture2D;
    float depth;
    float zOffset;

    if (m_parent == NULL) return;

    zOffset = m_parent->GetZOffset();
    depth = fabs(m_parent->GetDepth());

    glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMaskEnabled);
    glGetBooleanv(GL_BLEND, &blendEnabled);
    glGetBooleanv(GL_DEPTH_TEST, &depthEnabled);
    glGetIntegerv(GL_BLEND_SRC, &blendFuncSrc);
    glGetIntegerv(GL_BLEND_DST, &blendFuncDst);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glMatrixMode(GL_MODELVIEW);

    glDepthMask(GL_FALSE);

    glPushMatrix();
    glTranslatef(0.0f, 0.0f,
                 zOffset+abs(depth)+0.04f);
    m_TextFont->Draw();
    glPopMatrix();
    
    if (depthMaskEnabled == GL_TRUE)
    {
        glDepthMask(GL_TRUE);
    }
    
    if (depthEnabled == GL_FALSE)
    {
        glDisable(GL_DEPTH_TEST);
    }
    if (blendEnabled == GL_FALSE)
    {
        glDisable(GL_BLEND);
        
    }
    glDisable(GL_TEXTURE_2D);
    
    glBlendFunc(blendFuncSrc, blendFuncDst);
}
