/*
 * Copyright (c) 2017 Hackdirt Ltd.
 * Author: David Petrie (david@davidpetrie.com)
 *
 * License: https://www.gnu.org/licenses/gpl-3.0.en.html
 */

/*
 * Controller for:
 * - Levels - which one player is on.
 * - Currently selected world.
 * - Level picker menu.
 */

#ifndef APP_LEVEL_CONTROLLER_H
#define APP_LEVEL_CONTROLLER_H

#include "Box2D.h"

#include <hdk/game.h>
#include "hdk/effects.h"

#include "TextBlock.h"
#include "SmashedUtils.h"
#include "TowerPlayerStateClasses.h"
#include "b2BuoyancyController.h"

#include "AppInterface.h"

#define kGravityMultiplier 10.0f

class AppLevelController : public ContactManager, public AppInterface
{
public:

    AppLevelController(const hdInterfaceController* parentController);

    ~AppLevelController();

    void Step(double interval);


    void HandleTapUp(float x, float y, int tapCount);

    void HandleTapDown(float x, float y, int tapCount);

    void HandleSingleTap(const float x, const float y);

    void HandleDoubleTap(float x, float y);

    void HandleTapMovedSingle(const float previousX, const float previousY,
                              const float currentX, const float currentY);

    void HandleTapMovedDouble(const float aPreviousX, const float aPreviousY,
                              const float aCurrentX, const float aCurrentY,
                              const float bPreviousX, const float bPreviousY,
                              const float bCurrentX, const float bCurrentY);

    const bool InitLevel(const Level* level, const int levelId);

    const Level* GetCurrentLevel() const;

    void ResetCurrentLevel();

    void RestartCurrentLevel();


protected:

    void DrawInternal();

    void PushProjectionMatrix();

    void PopProjectionMatrix();

    const Block* GetSelectedTotemBlock() const;

    void SetSelectedTotemBlock(const Block *block);

    void DrawSelectedBlockHighlight();

    const b2World* GetPhysicsWorld() const;

    bool AddBlockToCurrentLevel(Block *block);

    Block* FindBlockWithTag(const int tag) const;

    const hdSound* GetCollisionSoundForMaterial(const e_hdkMaterial material) const;

    /* 
     * Virtual Functions - all optional 
     */
    virtual const bool Game_InitLevel(const Level *level, const int levelId);

    virtual void Game_Step(double interval);

    virtual void Game_ResetCurrentLevel();

    virtual const bool Game_CanSelectBlock(const Block *block);

    virtual void Game_hdkBlockWasSelected(const Block *block) const;

    virtual const bool Game_HandleTapUp(float x, float y, int tapCount);

    virtual const bool Game_HandleTapDown(float x, float y, int tapCount);

    virtual const bool Game_HandleSingleTap(float x, float y);

    virtual const bool Game_HandleDoubleTap(float x, float y);

    virtual const bool Game_HandleTapMovedSingle(const float previousX, const float previousY,
                                                 const float currentX, const float currentY);

    virtual const bool Game_HandleTapMovedDouble(const float aPreviousX, const float aPreviousY,
                                                 const float aCurrentX, const float aCurrentY,
                                                 const float bPreviousX, const float bPreviousY,
                                                 const float bCurrentX, const float bCurrentY);

    virtual const bool Game_HandleBlockTap(Block *block, const hdVec3& impactPoint, bool hazardCollision);

    virtual const bool Game_HandleParticleTap(hdPhysicsParticle* particle, const hdVec3& impactPoint, bool hazardCollision);

    virtual const bool Game_HandleBlockLoss(Block* block, const hdVec3& contactPoint);

    virtual void Game_HandleCollision(const ContactPoint* point);

    /*
     * Returns true if the game controller changed either of block1 or block2.
     */
    virtual bool Game_HandleBlockBlockCollision(Block *block1, Block *block2, const ContactPoint *point);

    virtual void Game_PlayLoudCollisionSound(const Block* block, const b2Body* body);

    virtual void Game_PlaySoftCollisionSound(const Block* block, const b2Body* body);

    virtual void Game_AddBlockToLevel(const Block* block);

    virtual void Game_HandleGoalCollision(Block *goalBlock, Block *block, const hdVec3& impactPoint);

    char m_currentWorldName[128];

    hdPhysicsParticleEmitter* m_physicsParticleEmitter;

private:

    void InitPhysics();

    void InitLevel(Level *level);

    void InitEffects();

    void InitAnimations();

    void InitSounds();

    void PhysicsStep(double interval);

    void PanToSelectedObject(double interval);

    void HandleCollision(const ContactPoint* point);

    void HandleParticleBlockCollision(hdPhysicsParticle *particle, Block *block, const ContactPoint *point);

    void HandleBlockBlockCollision(Block *block1, Block *block2, const ContactPoint *point);

    void HandleFragmentSplash(hdPhysicsParticle *fragment, const ContactPoint *point);

    bool DoParticleTapDown(float screenX, float screenY);

    /*
     * Block under mouse
     */
    bool SelectBlockAtScreenPoint(float x, float y);

    bool SelectBlockAtScreenPoint(e_hdInterfaceClickState clickState, float x, float y);

    hdVec3 m_distanceFromScreenCenter;

    /*
     * Mouse Joints
     */
    void CreateMouseJoint();

    void UpdateMouseJoint(float x, float y);

    void DestroyMouseJoint();

    /*
     * Game related stuff
     */
    void HandleBlockLoss(const hdVec3& contactPoint, Block* block);

    void HandleBlockWorldExit(const hdVec3& contactPoint, Block* block);

    void HandleSenderBlockTapUpEvent(const Block* sender, Event* ev);

    void HandleSenderBlockTapDownEvent(const Block* sender, Event* ev);

    void HandleGameAreaTapUp(float x, float y);

    void HandleBlockTap(Block* block, const hdVec3& impactPoint, bool hazardCollision);

    void HandleParticleTap(hdPhysicsParticle* particle, const hdVec3& impactPoint, bool hazardCollision);

    void PlayLoudCollisionSound(const Block* block, const b2Body* body);

    void PlaySoftCollisionSound(const Block* block, const b2Body* body);


    bool m_pushedProjectionMatrix;

    /* 
     * Particle Emitters 
     */
    SmokeEmitter* m_smokeEmitter;

    SplashEmitter* m_splashEmitter;

    SplashEmitter* m_gunSmokeEmitter;

    SparkEmitter* m_gunSparkEmitter;

    QuadEmitter* m_waveParticleEmitter;

    StarEmitter *m_starParticleEmitter;

    StarEmitter *m_questionEmitter;

    SplinterEmitter *m_splinterEmitter;

    SmokeEmitter *m_explosionEmitter;

    SmokeEmitter *m_explosionSmokeEmitter;

    int m_currentLevelId;


    /* 
     * Sound
     */
    hdPointerList<hdSound, 8>* m_blockExplosionSoundsGlass;
    hdPointerList<hdSound, 8>* m_blockExplosionSoundsMetal;
    hdPointerList<hdSound, 8>* m_blockExplosionSoundsStone;
    hdPointerList<hdSound, 8>* m_blockExplosionSoundsWood;

    hdPointerList<hdSound, 8>* m_blockCollisionSoundsGlass;
    hdPointerList<hdSound, 8>* m_blockCollisionSoundsMetal;
    hdPointerList<hdSound, 8>* m_blockCollisionSoundsStone;
    hdPointerList<hdSound, 8>* m_blockCollisionSoundsWood;

    hdPointerList<hdSound, 16>* m_lightSplashSounds;
    hdPointerList<hdSound, 8>* m_blockLossSounds;

    hdSound *m_shotty;


    hdSound *m_bombSelectSound;
    hdSound *m_dragSelectSound;


    /*
     * Misc 
     */
    b2World* m_physicsWorld;

    float m_minFrameDuration;

    float m_physicsFixedFrameDuration;

    bool m_fixedPhysicsInterval;

    float m_variablePhysicsRateMultiplier;

    int m_physicsIterationsCount;

    b2MouseJoint* m_mouseJoint;

    Block *m_HACK_lastJointedBlock;

    Block* m_topBlock;

    Block* m_floor;

    Block* m_bottomBoundary;
    
    bool m_reflectParticles;

    Level* m_currentLevel;
    
    hdAABB m_blockDeselectThreshold;

    b2BuoyancyController* m_buoyancyController;
    
    hdVec3 m_mouseDownTestPoint;

    hdVec3 m_testPoint;
    
    hdVec3 m_lastRayScreenPoint;
    
    hdVec3 m_tapDownBlockWorldCenter;
    
    int m_rayQueryCount;
    
    Block* m_currBlock;
    
    bool m_currBlockTapDown;
    
    int m_currBlockTicksSinceTapDown;
    
    bool m_tapDown;
};


inline const b2World* AppLevelController::GetPhysicsWorld() const
{
    return m_physicsWorld;
}


inline const Level* AppLevelController::GetCurrentLevel() const
{
    return m_currentLevel;
}


inline void AppLevelController::SetSelectedTotemBlock(const Block *block)
{
    m_currBlock = (Block *)block;
}


inline const Block* AppLevelController::GetSelectedTotemBlock() const
{
    return m_currBlock;
}

#endif
