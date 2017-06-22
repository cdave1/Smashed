/*
 * Copyright (c) 2017 Hackdirt Ltd.
 * Author: David Petrie (david@davidpetrie.com)
 *
 * License: https://www.gnu.org/licenses/gpl-3.0.en.html
 */

#include "SmashedUtils.h"

static Block* visitedBlocks[kVisitedBlocksMax];

static int visitedBlockCount = 0;

/*
 * This function treats the block contact points as a graph. It counts
 * blocks that have a higher y value than the given block by traversing
 * the contact points of the block.
 *
 * It calls CountBlocksAbove - a function which does this recursively.
 */
int GetBlockRiskiness(const Block *block, const ContactPoint *contactPoints, const int contactPointCount)
{
    Block *b = (Block *)block;

    memset(visitedBlocks, 0, sizeof(visitedBlocks));
    visitedBlockCount = 0;

    if (b->GetPhysicsBody() == NULL) return 0;
    if (!b->IsVisible()) return 0;
    if (!b->IsDestroyable()) return 0;
    return CountBlocksAbove(b, contactPoints, contactPointCount);
}


bool AddBlockToVisited(Block *block)
{
    if (visitedBlockCount == kVisitedBlocksMax) return false;
    visitedBlocks[visitedBlockCount++] = block;
    return true;
}


bool BlockWasVisited(Block *block)
{
    if (visitedBlockCount == 0) return false;
    for (int i = 0; i < visitedBlockCount; ++i)
    {
        if (visitedBlocks[i] == block) return true;
    }
    return false;
}


/*
 * call CountBlocksAbove for each of the contact points the given block.
 */
int CountBlocksAbove(const Block *block, const ContactPoint *contactPoints, const int contactPointCount)
{
    Block *b = (Block *)block;

    if (BlockWasVisited(b)) return 0;
    if (!AddBlockToVisited(b)) return 0;

    if (b->GetPhysicsBody() == NULL) return 0;
    if (!b->IsVisible()) return 0;
    if (!b->IsDestroyable()) return 0;

    int count = 0;
    hdPolygon* obj1 = NULL;
    hdPolygon* obj2 = NULL;

    for (int i = 0; i < contactPointCount; ++i)
    {
        obj1 = (hdPolygon *)(contactPoints[i].body1->GetUserData());
        obj2 = (hdPolygon *)(contactPoints[i].body2->GetUserData());

        if (obj1 != NULL && obj2 != NULL)
        {
            if ((contactPoints[i].body1 == b->GetPhysicsBody() && obj2->GetUserType() == (int)e_hdkTypeBlock))
            {
                if (contactPoints[i].position.y > b->GetWorldCenter().y)
                {
                    ++count;
                    count += CountBlocksAbove((Block *)obj2, contactPoints, contactPointCount);
                }
            }
            else if ((contactPoints[i].body2 == b->GetPhysicsBody() && obj1->GetUserType() == (int)e_hdkTypeBlock))
            {
                if (contactPoints[i].position.y > b->GetWorldCenter().y)
                {
                    ++count;
                    count += CountBlocksAbove((Block *)obj1, contactPoints, contactPointCount);
                }
            }
        }
    }
    return count;
}


/*
 * Count contact points below the world center of the block
 *
 * If the block has NO contact points with other blocks below the world center
 * then the block is in mid air.
 */
bool IsBlockInMidair(const Block *block, const ContactPoint *contactPoints, const int contactPointCount)
{
    Block *b = (Block *)block;

    if (b->GetPhysicsBody() == NULL) return false;
    if (!b->IsVisible()) return false;
    if (!b->IsDestroyable()) return false;
    if (b->GetPhysicsBody()->IsSleeping()) return false;

    hdPolygon* obj1 = NULL;
    hdPolygon* obj2 = NULL;

    for (int i = 0; i < contactPointCount; ++i)
    {
        obj1 = (hdPolygon *)(contactPoints[i].body1->GetUserData());
        obj2 = (hdPolygon *)(contactPoints[i].body2->GetUserData());

        if (obj1 != NULL && obj2 != NULL)
        {
            if ((contactPoints[i].body1 == b->GetPhysicsBody() && obj2->GetUserType() == (int)e_hdkTypeBlock)
                || (contactPoints[i].body2 == b->GetPhysicsBody() && obj1->GetUserType() == (int)e_hdkTypeBlock))
            {
                if (contactPoints[i].position.y < b->GetWorldCenter().y)
                {
                    // no need to check anything else
                    return false;
                }
            }
        }
    }
    return true;
}


void JointMotorStart(Joint *joint, const bool isPositive)
{
    if (joint->GetJointType() == e_hdkJointTypeRevolute)
    {
        b2RevoluteJoint *physicsJoint = (b2RevoluteJoint *)joint->GetPhysicsJoint();

        physicsJoint->EnableMotor(true);
        if (isPositive)
        {
            physicsJoint->SetMotorSpeed(fabs(joint->GetJointMotorSpeed()));
        }
        else
        {
            physicsJoint->SetMotorSpeed(-fabs(joint->GetJointMotorSpeed()));
        }

        physicsJoint->EnableLimit(true);
        physicsJoint->SetLimits(joint->GetRevoluteJointLowerAngle(),
                                joint->GetRevoluteJointUpperAngle());
    }
}


void JointMotorStop(Joint *joint)
{
    if (joint->GetJointType() == e_hdkJointTypeRevolute)
    {
        b2RevoluteJoint *physicsJoint = (b2RevoluteJoint *)joint->GetPhysicsJoint();
        physicsJoint->EnableLimit(true);
        float32 angle = physicsJoint->GetJointAngle();
        physicsJoint->SetLimits(angle, angle);
    }
}


/**
 * Mag is initially the absolute distance between the object and the camera world center. It is the
 * _magnitude_ of the distance the camera must move to center on the object.
 *
 * We then update magnitude so that it is distance the camera
 * needs to move to ensure that the object is merely INSIDE the
 * camera's box.
 *
 * The problem with this is that the camera will move this distance,
 * thus making the object inside the box. On the next frame, the object will
 * be inside the box, and the camera will not have to move. But after this,
 * the object would have moved a fraction -- if it moved outside the box again
 * then the camera will jerk into place again.
 */
const hdVec3 PanToGameObject(const hdVec3& gameObjectCenter, const hdVec3& cameraWorldCenter, const hdAABB& cameraAABB)
{
    hdVec3 physPos, box, diff, mag;

    // Pan the projection a fraction of the direction between the
    // projection center and the current block.
    box = (0.2f * (cameraAABB.upper - cameraAABB.lower));

    physPos.Set(gameObjectCenter.x, gameObjectCenter.y, 0.0f);

    diff = (physPos - cameraWorldCenter);
    mag.Set(fabs(diff.x), fabs(diff.y), 0.0f);

    mag.x = (mag.x < box.x) ? 0.0f : mag.x - box.x;
    mag.y = (mag.y < box.y) ? 0.0f : mag.y - box.y;

    diff.x = (diff.x > 0.0f) ? -mag.x : mag.x;
    diff.y = (diff.y < 0.0f) ? -mag.y : mag.y;

    return diff;
}


extern const hdVec3 PanToGameObject(const Block *block, const hdVec3& cameraCenter, const hdAABB& cameraAABB, const double interval)
{
    hdVec3 physPos, box, diff, mag, vel;

    vel.Set(((Block *)block)->GetPhysicsBody()->GetLinearVelocity().x,
            ((Block *)block)->GetPhysicsBody()->GetLinearVelocity().y,
            0.0f);
    vel = interval * vel;
    // Pan the projection a fraction of the direction between the
    // projection center and the current block.
    box = (0.2f * (cameraAABB.upper - cameraAABB.lower));

    physPos.Set(((Block *)block)->GetWorldCenter().x,
                ((Block *)block)->GetWorldCenter().y, 0.0f);

    diff = (physPos - cameraCenter);
    mag.Set(fabs(diff.x + vel.x), fabs(diff.y + vel.y), 0.0f);

    mag.x = (mag.x < box.x) ? 0.0f : mag.x - box.x;
    mag.y = (mag.y < box.y) ? 0.0f : mag.y - box.y;

    diff.x = (diff.x > 0.0f) ? -mag.x : mag.x;
    diff.y = (diff.y < 0.0f) ? -mag.y : mag.y;
    return diff;
}




/*
 * Need to move in a direction such that the center of the camera is some percentage away from the
 * block, in the direction of the block's current movement vector.
 *
 * If movement vector is nil, then pan to middle.
 */
static float lastSpeed = 0.0f;
const hdVec3 PanToGameObject(const Block *block,
                             const hdVec3& cameraWorldCenter,
                             const hdAABB& cameraAABB,
                             const bool tapDown,
                             const double interval)
{
    hdAABB boxAABB, innerAABB;
    hdVec3 p, v, vn, diff;
    float speed;

    hdVec3 box, mag;

    boxAABB = ((Block *)block)->GetAABB();
    
    p.Set(((Block *)block)->GetWorldCenter().x, 
          ((Block *)block)->GetWorldCenter().y,
          0.0f);
    
    v.Set(((Block *)block)->GetPhysicsBody()->GetLinearVelocity().x,
          ((Block *)block)->GetPhysicsBody()->GetLinearVelocity().y,
          0.0f);
    
    diff.Set(0, 0, 0);
    
    speed = hdFastHypotenuse(v.x, v.y);
    
    if (tapDown)
    {
        vn = hdVec2toVec3(hdNormalize(hdVec3toVec2(v)));
        
#ifdef IPHONE_BUILD
        if (speed > 8.0f)
        {
            diff = (interval * (0.48f * speed * vn)) + ((speed * 0.005f) * (p - cameraWorldCenter));
        }
        else if (speed > 4.0f)
        {
            diff = (interval * (4.0f * vn)) + (0.01f * (p - cameraWorldCenter));
        }
        else if (speed > 0.0f)
        {
            diff = (interval * v);
        }
#else
        if (speed > 10.0f)
        {
            diff = (interval * (0.5 * speed * vn)) + (interval * (p - cameraWorldCenter));
        }
        else if (speed > 5.0f)
        {
            diff = (interval * (0.3f * speed * vn)) + (interval * (p - cameraWorldCenter));
        }
        else if (speed > 0.0f)
        {
            diff = PanToGameObject(((Block *)block)->GetWorldCenter(), cameraWorldCenter, cameraAABB);
            diff.Set(-diff.x, diff.y, 0.0f);
            diff = 5.0f * interval * diff;
        }
#endif
        
    }
    else
    {
        if (speed > 0)
        {
            diff = (interval * v) + (interval * (p - cameraWorldCenter));
        }
    }
    
    lastSpeed = speed;
    
    return diff;
}
