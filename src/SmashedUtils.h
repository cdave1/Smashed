/*
 * Copyright (c) 2017 Hackdirt Ltd.
 * Author: David Petrie (david@davidpetrie.com)
 *
 * License: https://www.gnu.org/licenses/gpl-3.0.en.html
 */

#ifndef SMASHED_UTILS_H
#define SMASHED_UTILS_H

#include "Box2D.h"
#include <hdk/game.h>
#include <hdk/physics.h>

#define kVisitedBlocksMax 128

extern int GetBlockRiskiness(const Block *block, const ContactPoint *contactPoints, const int contactPointCount);

extern bool IsBlockInMidair(const Block *block, const ContactPoint *contactPoints, const int contactPointCount);

int CountBlocksAbove(const Block *block, const ContactPoint *contactPoints, const int contactPointCount);

bool AddBlockToVisited(Block *block);

bool BlockWasVisited(Block *block);

extern void JointMotorStart(Joint *joint, const bool isPositive);

extern void JointMotorStop(Joint *joint);

extern const hdVec3 PanToGameObject(const hdVec3& gameObjectCenter, const hdVec3& cameraCenter, const hdAABB& cameraAABB);

extern const hdVec3 PanToGameObject(const Block *block, const hdVec3& cameraCenter, const hdAABB& cameraAABB, const double interval);

extern const hdVec3 PanToGameObject(const Block *block,
                                    const hdVec3& cameraWorldCenter,
                                    const hdAABB& cameraAABB,
                                    const bool tapDown,
                                    const double interval);

#endif
