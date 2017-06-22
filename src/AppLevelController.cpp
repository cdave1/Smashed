/*
 * Copyright (c) 2017 Hackdirt Ltd.
 * Author: David Petrie (david@davidpetrie.com)
 *
 * License: https://www.gnu.org/licenses/gpl-3.0.en.html
 */

#include "AppLevelController.h"

static int m_textureFrameBuffer = -1;

AppLevelController::AppLevelController(const hdInterfaceController* parentController) : AppInterface(parentController)
{
    m_interactionState = e_interactionDisabled;

    m_currentLevel = NULL;
    m_testPoint.Set(0,0,0);
    m_lastRayScreenPoint.Set(0,0,0);
    m_rayQueryCount = 0;
    m_currBlock = NULL;
    m_mouseJoint = NULL;

    m_currBlockTapDown = false;
    m_currBlockTicksSinceTapDown = 1000;

    m_tapDown = false;

    m_pushedProjectionMatrix = false;

    if ("Low" == hdConfig::GetValueForKey(CONFIG_EFFECTSQUALITYLEVEL_KEY) ||
        "Medium" == hdConfig::GetValueForKey(CONFIG_EFFECTSQUALITYLEVEL_KEY))
    {
        m_reflectParticles = false;
    }
    else
    {
        m_reflectParticles = true;
    }

    m_messageTagQueue = new hdTypedefList<totemMessage, 16>();
    m_physicsParticleEmitter = NULL;

    m_minFrameDuration = strtof(hdConfig::GetValueForKey(CONFIG_MINFRAMEDURATION_KEY).c_str(), NULL);
    m_minFrameDuration = hdClamp(m_minFrameDuration, 0.016f, 0.05f);

    m_fixedPhysicsInterval = (true == strtol(hdConfig::GetValueForKey(CONFIG_FIXEDPHYSICSINTERVAL_KEY).c_str(), NULL, 0));

    m_physicsFixedFrameDuration = strtof(hdConfig::GetValueForKey(CONFIG_PHYSICSFIXEDFRAMEDURATION_KEY).c_str(), NULL);
    m_physicsFixedFrameDuration = hdClamp(m_physicsFixedFrameDuration, 0.016f, 0.05f);

    m_variablePhysicsRateMultiplier = strtof(hdConfig::GetValueForKey("VariablePhysicsRateMultiplier").c_str(), NULL);
    if (m_variablePhysicsRateMultiplier == 0.0f)
    {
        m_variablePhysicsRateMultiplier = 1.0f;
    }
    else
    {
        m_variablePhysicsRateMultiplier = hdClamp(m_variablePhysicsRateMultiplier, 0.2f, 10.0f);
    }

    m_physicsIterationsCount = strtol(hdConfig::GetValueForKey(CONFIG_PHYSICSITERATIONSCOUNT_KEY).c_str(), NULL, 0);
    m_physicsIterationsCount = hdClamp(m_physicsIterationsCount, 1, 20);

    m_HACK_lastJointedBlock = false;

    this->InitPhysics();
    this->InitAnimations();
    this->InitSounds();
    this->InitEffects();

    this->RefreshLayout();
}


AppLevelController::~AppLevelController()
{
    delete m_glow;
    delete m_blockLossSounds;
    delete m_lightSplashSounds;
    delete m_blockExplosionSoundsGlass;
    delete m_blockExplosionSoundsMetal;
    delete m_blockExplosionSoundsStone;
    delete m_blockExplosionSoundsWood;
    delete m_blockCollisionSoundsGlass;
    delete m_blockCollisionSoundsMetal;
    delete m_blockCollisionSoundsStone;
    delete m_blockCollisionSoundsWood;
    delete m_physicsParticleEmitter;
    delete m_smokeEmitter;
    delete m_gunSmokeEmitter;
    delete m_splashEmitter;
    delete m_gunSparkEmitter;
    delete m_waveParticleEmitter;
    delete m_physicsWorld;
}


void AppLevelController::InitPhysics()
{
    b2AABB m_physicsWorldAABB;
    m_physicsWorldAABB.lowerBound.Set(-256.0f, -256.0f);
    m_physicsWorldAABB.upperBound.Set(256.0f, 256.0f);

    b2Vec2 gravity(0.0f, kGravityMultiplier * -1.0f);
    bool doSleep = true;
    m_physicsWorld = new b2World(m_physicsWorldAABB, gravity, doSleep);
    m_physicsWorld->SetContactListener(this);
}


void AppLevelController::InitAnimations() {}


void AppLevelController::InitSounds()
{
    /*
     * Heavy splashes
     */
    m_blockLossSounds = new hdPointerList<hdSound, 8>();
    m_blockLossSounds->Add(hdSoundManager::FindSound("Sounds/heavy_splash_01.caf", e_soundTypeNormal));
    m_blockLossSounds->Add(hdSoundManager::FindSound("Sounds/heavy_splash_02.caf", e_soundTypeNormal));
    m_blockLossSounds->Add(hdSoundManager::FindSound("Sounds/heavy_splash_03.caf", e_soundTypeNormal));
    m_blockLossSounds->Add(hdSoundManager::FindSound("Sounds/heavy_splash_04.caf", e_soundTypeNormal));
    m_blockLossSounds->Add(hdSoundManager::FindSound("Sounds/heavy_splash_05.caf", e_soundTypeNormal));

    /*
     * Light splashes
     */
    m_lightSplashSounds = new hdPointerList<hdSound, 16>();
    m_lightSplashSounds->Add(hdSoundManager::FindSound("Sounds/light_splash_01.caf", e_soundTypeNormal));
    m_lightSplashSounds->Add(hdSoundManager::FindSound("Sounds/light_splash_02.caf", e_soundTypeNormal));
    m_lightSplashSounds->Add(hdSoundManager::FindSound("Sounds/light_splash_03.caf", e_soundTypeNormal));
    m_lightSplashSounds->Add(hdSoundManager::FindSound("Sounds/light_splash_04.caf", e_soundTypeNormal));
    m_lightSplashSounds->Add(hdSoundManager::FindSound("Sounds/light_splash_05.caf", e_soundTypeNormal));

    /*
     * Glass explosions
     */
    m_blockExplosionSoundsGlass = new hdPointerList<hdSound, 8>();

    /*
     * Metal 'splosions
     */
    m_blockExplosionSoundsMetal = new hdPointerList<hdSound, 8>();

    /*
     * Stone explosions
     */
    m_blockExplosionSoundsStone = new hdPointerList<hdSound, 8>();

    m_blockExplosionSoundsStone->Add(hdSoundManager::FindSound("Sounds/heavy_smashstone_01.caf", e_soundTypeFrequent));
    m_blockExplosionSoundsStone->Add(hdSoundManager::FindSound("Sounds/heavy_smashwood_01.caf", e_soundTypeFrequent));
    m_blockExplosionSoundsStone->Add(hdSoundManager::FindSound("Sounds/glassmetal_explosion.caf", e_soundTypeFrequent));

    /*
     * Wood explosions
     */
    m_blockExplosionSoundsWood = new hdPointerList<hdSound, 8>();

    /*
     * Glass Collision Sounds
     */
    m_blockCollisionSoundsGlass = new hdPointerList<hdSound, 8>();

    /*
     * Metal collisions
     */
    m_blockCollisionSoundsMetal = new hdPointerList<hdSound, 8>();
    m_blockCollisionSoundsMetal->Add(hdSoundManager::FindSound("Sounds/impactmetal_01.caf", e_soundTypeFrequent));
    m_blockCollisionSoundsMetal->Add(hdSoundManager::FindSound("Sounds/impactmetal_02.caf", e_soundTypeFrequent));
    m_blockCollisionSoundsMetal->Add(hdSoundManager::FindSound("Sounds/impactmetal_03.caf", e_soundTypeFrequent));

    /*
     * Stone collisions
     */
    m_blockCollisionSoundsStone = new hdPointerList<hdSound, 8>();
    m_blockCollisionSoundsStone->Add(hdSoundManager::FindSound("Sounds/impactstone_02.caf", e_soundTypeFrequent));
    m_blockCollisionSoundsStone->Add(hdSoundManager::FindSound("Sounds/impactstone_03.caf", e_soundTypeFrequent));

    /*
     * Wood collisions
     */
    m_blockCollisionSoundsWood = new hdPointerList<hdSound, 8>();
    m_blockCollisionSoundsWood->Add(hdSoundManager::FindSound("Sounds/light_impactwood_01.caf", e_soundTypeFrequent));
    m_blockCollisionSoundsWood->Add(hdSoundManager::FindSound("Sounds/light_impactwood_02.caf", e_soundTypeFrequent));
    m_blockCollisionSoundsWood->Add(hdSoundManager::FindSound("Sounds/heavy_impactwood_01.caf", e_soundTypeFrequent));
    m_shotty = hdSoundManager::FindSound("Sounds/shotty.caf", e_soundTypeNormal);

    m_dragSelectSound = hdSoundManager::FindSound("Sounds/blockSelect01.caf", e_soundTypeNormal);

    m_bombSelectSound = hdSoundManager::FindSound("Sounds/blockSelect03.caf", e_soundTypeNormal);
}


void AppLevelController::InitEffects()
{
    m_physicsParticleEmitter = new hdPhysicsParticleEmitter(m_gameWorld, m_physicsWorld, m_projection, m_buoyancyController, 3.0f, 8.0f);

    m_smokeEmitter = new SmokeEmitter(48, "Materials/p_smoke3_15_32.tga", 1.44f, 1.5f, -6.0f, 0.005f);
    m_splinterEmitter = new SplinterEmitter(32, 1, 1.25f, -8.0f, 5.0f);

    // Perf tweaks for iPhone 1G
    if ("Low" == hdConfig::GetValueForKey(CONFIG_EFFECTSQUALITYLEVEL_KEY))
    {
        m_smokeEmitter->SetNextParticleCount(0);
        m_splinterEmitter->SetNextParticleCount(0);
    }
    else if ("Medium" == hdConfig::GetValueForKey(CONFIG_EFFECTSQUALITYLEVEL_KEY))
    {
        m_smokeEmitter->SetNextParticleCount(0);
        m_splinterEmitter->SetNextParticleCount(6);
    }

    m_explosionEmitter = new SmokeEmitter(48, "Materials/p_explode2_15_32.tga", 2.5f, 0.5f, -2.0f, 0.5f);
    m_explosionEmitter->SetParticleBlendStyle(e_particleBlendStyleMultiply);
    m_explosionEmitter->SetNextParticleCount(4);

    m_explosionSmokeEmitter = new SmokeEmitter(48, "Materials/p_smoke2_15_32.tga", 3.2f, 1.25f, -1.0f, 1.0f);
    m_explosionSmokeEmitter->SetParticleBlendStyle(e_particleBlendStyleDarken);
    m_explosionSmokeEmitter->SetNextParticleCount(4);

    m_gunSmokeEmitter = new SplashEmitter(16, "Materials/p_smoke3_15_32.tga", 32.0f, 0.32f, 15.0f, 6.0f);

    m_questionEmitter = new StarEmitter(8, "Materials/p_qmark_15_32.tga", 0.0f, 3.0f, 24.0f, 0.75f);
    m_questionEmitter->m_ySpreadLower = -2.0f;
    m_questionEmitter->m_ySpreadUpper = 2.0f;
    m_questionEmitter->m_onStopResume = true;

    m_splashEmitter =  new SplashEmitter(48, "Materials/p_splasha0001.tga", 1.8f, 2.0f, -10.0f, 0.005f);
    m_gunSparkEmitter = new SparkEmitter(1, "Materials/p_spark_14_128.tga", 3.0f, 0.075f, 0.00001f, 0.0001f);
    m_waveParticleEmitter = new QuadEmitter(32, "Materials/p_splashb_15_64.tga", 2.0f, 1.5f, 0.0f, 0.0f);
}


const bool AppLevelController::InitLevel(const Level* level, const int levelId)
{
    unsigned i;
    TextBlock *textBlock;
    Block *block;
    Event *event;
    Joint *joint;
    b2BuoyancyControllerDef bcd;
    totemMessage *scriptMessages[20];
    int msgCount = 0;

    if (level == NULL)
    {
        m_currentLevel = NULL;
        return false;
    }

    m_currentLevel = (Level *)level;
    m_currentLevelId = levelId;

    /*
     * HACK HACK HACK
     * HACK HACK HACK
     *
     * m_HACKHACKHACK_currentLevelId to support some code elsewhere that checks the current level.
     */
    m_HACKHACKHACK_currentLevelId = levelId;

    if (m_physicsParticleEmitter != NULL)
    {
        m_physicsParticleEmitter->RemoveAllParticles();
    }

    m_currentLevel->Restart();
    m_currentLevel->InitGameObjects(m_gameWorld, m_projection);
    m_currentLevel->InitPhysicsObjects(m_gameWorld, m_physicsWorld);

    // Buoyancy stuff...
    if (m_currentLevel->GetFloorInfo() != NULL)
    {
        bcd.offset = m_currentLevel->GetFloorInfo()->m_floorLevel;
        bcd.normal.Set(0,1);
        bcd.density = 15.0;
        bcd.linearDrag = 2;
        bcd.angularDrag = 1;

        m_buoyancyController = (b2BuoyancyController*)m_physicsWorld->CreateController(&bcd);
    }

    /* Question Particle Emitter */
    m_questionEmitter->m_onStopResume = false;
    m_questionEmitter->Stop();
    for (i = 0; i < m_currentLevel->GetEvents()->GetItemCount(); ++i)
    {
        event = (Event *)m_currentLevel->GetEvents()->GetItems()[i];

        if (event->GetEventType() == e_hdkEventTypeShowMessage)
        {
            hdGameObject *obj = (hdGameObject *)event->GetFirstObject();
            m_questionEmitter->Start(obj->GetWorldCenter().x, obj->GetWorldCenter().y);
            m_questionEmitter->m_onStopResume = true;
            break;
        }
    }

    /*
     * Only draggable blocks are buoyant
     */
    for (i= 0; i < m_currentLevel->GetBlockCount(); ++i)
    {
        block = (Block *)m_currentLevel->GetBlocks()[i];
        if (block->IsDraggable())
        {
            if (m_buoyancyController != NULL && block->GetPhysicsBody() != NULL)
            {
                m_buoyancyController->AddBody((b2Body*)block->GetPhysicsBody());
            }
        }

        if (block->GetMaterial() == e_hdkMaterialText)
        {
            textBlock = new TextBlock(block, levelId);
            ((Level *)level)->AddBlock(textBlock);
        }
    }

    /*
     * In game flags for jointed blocks.
     */
    for (i = 0; i < level->GetJointCount(); ++i)
    {
        joint = (Joint *)level->GetJoints()[i];
        if (joint->GetJointType() == e_hdkJointTypeRevolute)
        {
            block = (Block *)joint->GetFirstBody();
            if (block != NULL)
                block->AddGameFlag(e_hdkGameFlagsBlockHasRevJoint);

            block = (Block *)joint->GetSecondBody();
            if (block != NULL)
                block->AddGameFlag(e_hdkGameFlagsBlockHasRevJoint);
        }
    }

    m_projection->ResetAABB();
    m_worldAABB = m_currentLevel->GetAABB();

    // Barriers on top and either side of the world AABB.
    m_topBlock = new Block(m_gameWorld,
                                m_physicsWorld,
                                hdVec2(m_worldAABB.lower.x, m_worldAABB.upper.y),
                                hdVec2(m_worldAABB.upper.x, m_worldAABB.upper.y+4.0f),
                                e_hdkMaterialCustomTexture,
                                e_hdkShapeTypeRectangle,
                                e_hdkBlockTypeGround);
    m_topBlock->Hide();
    m_currentLevel->AddBlock(m_topBlock);


    Block* left = new Block(m_gameWorld,
                                      m_physicsWorld,
                                      hdVec2(m_worldAABB.lower.x - 0.1f, m_worldAABB.lower.y),
                                      hdVec2(m_worldAABB.lower.x, m_worldAABB.upper.y),
                                      e_hdkMaterialCustomTexture,
                                      e_hdkShapeTypeRectangle,
                                      e_hdkBlockTypeGround);
    left->Hide();
    m_currentLevel->AddBlock(left);

    Block* right = new Block(m_gameWorld,
                                       m_physicsWorld,
                                       hdVec2(m_worldAABB.upper.x, m_worldAABB.lower.y),
                                       hdVec2(m_worldAABB.upper.x+0.1f, m_worldAABB.upper.y),
                                       e_hdkMaterialCustomTexture,
                                       e_hdkShapeTypeRectangle,
                                       e_hdkBlockTypeGround);
    right->Hide();
    m_currentLevel->AddBlock(right);

    m_bottomBoundary = new Block(m_gameWorld,
                                      m_physicsWorld,
                                      hdVec2(m_worldAABB.lower.x, m_worldAABB.lower.y-2.0f),
                                      hdVec2(m_worldAABB.upper.x, m_worldAABB.lower.y),
                                      e_hdkMaterialCustomTexture,
                                      e_hdkShapeTypeRectangle,
                                      e_hdkBlockTypeHazard);
    m_bottomBoundary->Hide();
    m_currentLevel->AddBlock(m_bottomBoundary);


    // HACK HACK HACK
    // Floor info needs more options. The floor rules manifest themselves like this:
    // - no floor object means a hazard below the world AABB
    // - water floor means a water hanzard at floor level
    // - no textures mean an invisible floor hazard at the floor level
    // - otherwise the floor is a solid non-hazard.
    // WTF????
    m_floor = NULL;
    if (m_currentLevel->GetFloorInfo() != NULL)
    {
        if (m_currentLevel->GetFloorInfo()->m_isWater)
        {
            // Water is a hazard.
            m_floor = new Block(m_gameWorld,
                                     m_physicsWorld,
                                     hdVec2(m_worldAABB.lower.x, m_worldAABB.lower.y),
                                     hdVec2(m_worldAABB.upper.x, m_currentLevel->GetFloorInfo()->m_floorLevel),
                                     e_hdkMaterialWood,
                                     e_hdkShapeTypeRectangle,
                                     e_hdkBlockTypeHazard);

            hdTranslateVertices(m_floor->GetVertices(), m_floor->GetVertexCount(), hdVec3(0.0f, 0.0f, 10.0f));
            m_floor->Hide();
            m_currentLevel->AddBlock(m_floor);
        }
        else if (m_currentLevel->GetFloorInfo()->IsDrawable() == false)
        {
            m_floor = new Block(m_gameWorld,
                                     m_physicsWorld,
                                     hdVec2(m_worldAABB.lower.x, m_worldAABB.lower.y),
                                     hdVec2(m_worldAABB.upper.x, m_currentLevel->GetFloorInfo()->m_floorLevel),
                                     e_hdkMaterialWood,
                                     e_hdkShapeTypeRectangle,
                                     e_hdkBlockTypeHazard);
            m_floor->Hide();
            m_currentLevel->AddBlock(m_floor);
        }
        else
        {
            // Floor is NOT a hazard block
            m_floor = new Block(m_gameWorld,
                                     m_physicsWorld,
                                     hdVec2(m_worldAABB.lower.x, m_worldAABB.lower.y),
                                     hdVec2(m_worldAABB.upper.x, m_currentLevel->GetFloorInfo()->m_floorLevel),
                                     e_hdkMaterialWood,
                                     e_hdkShapeTypeRectangle,
                                     e_hdkBlockTypeBoundary);
            m_floor->Hide();
            m_currentLevel->AddBlock(m_floor);
        }
    }


    m_physicsParticleEmitter->RemoveAllParticles();

    if (m_currBlock == NULL)
    {
        m_currBlock = (Block *)FindBlockWithTag(1001);
    }

    // Preload script textures:
    msgCount = Scripts_GetLevelMessagesForTag(scriptMessages,
                                              20, levelId, HDK_SCRIPTS_WILDCARD_TAG);

    for (int i = 0; i < hdMin(msgCount, 20); i++)
    {
        if (scriptMessages[i]->messageType == e_hdkMessageTypeImage ||
            scriptMessages[i]->messageType == e_hdkMessageTypeCustomImageText ||
            scriptMessages[i]->messageType == e_hdkMessageTypeAvatar)
        {
            hdTextureManager::Instance()->FindTexture(scriptMessages[i]->texture, TT_Wall);
        }
    }

    Game_InitLevel(level, levelId);

    return true;
}


void AppLevelController::ResetCurrentLevel()
{
    if (m_currentLevel != NULL)
    {
        m_currentLevel->DestroyGameObjects();
        if (m_buoyancyController != NULL)
        {
            m_physicsWorld->DestroyController(m_buoyancyController);
            m_buoyancyController = NULL;
        }
    }
}


void AppLevelController::RestartCurrentLevel()
{
    Game_ResetCurrentLevel();
    this->ResetCurrentLevel();
    this->ResetGameWorld();
    this->InitLevel(m_currentLevel, m_currentLevelId);
}


Block* AppLevelController::FindBlockWithTag(const int tag) const
{
    Block *b = NULL;

    for (int i = 0; i < m_currentLevel->GetBlockCount(); ++i)
    {
        if (m_currentLevel->GetBlocks()[i]->GetTag() == tag)
        {
            b = (Block*)m_currentLevel->GetBlocks()[i];
            break;
        }
    }
    return b;
}


void AppLevelController::PhysicsStep(double interval)
{
    float proportion;

    if (m_currentLevel == NULL) return;
    if (m_physicsWorld == NULL) return;

    this->ResetContactPoints();

    // Paused
    if (m_interactionState != e_interactionDisabled)
    {
        proportion = hdClamp(interval, 0.02, 2.0 * m_physicsFixedFrameDuration);

        // Stepping twice, for half the interval, produces the best results...
        m_physicsWorld->Step(0.5f * proportion * m_variablePhysicsRateMultiplier,
                             m_physicsIterationsCount,
                             m_physicsIterationsCount);

        m_physicsWorld->Step(0.5f * proportion * m_variablePhysicsRateMultiplier,
                             m_physicsIterationsCount,
                             m_physicsIterationsCount);
    }
}


void AppLevelController::PanToSelectedObject(double interval)
{
    hdVec3 pan;
    if (m_currBlock != NULL &&
        m_interactionState != e_interactionDisabled &&
        m_currBlock->GetPhysicsBody() != NULL && m_currBlock->IsDraggable())
    {
        pan = PanToGameObject(m_currBlock,
                              m_projection->GetWorldCenter(),
                              m_projection->GetAABB(),
                              m_tapDown,
                              interval);

#ifdef IPHONE_BUILD
        if (pan.x == 0 && pan.y == 0 && pan.z == 0)
        {
            if (m_currBlock->ContainsGameFlag((uint32)e_hdkGameFlagsBlockDrawRaw))
            {

                pan = PanToGameObject(m_currBlock->GetWorldCenter(),
                                      m_projection->GetWorldCenter(),
                                      m_projection->GetAABB());

                PanProjection(0.0f, 0.0f, pan.x * 5.0f, pan.y * 5.0f);
            }
        }
        else
        {
            TranslateProjection(pan);
        }
#else
        if (fabs(((Block *)m_currBlock)->GetPhysicsBody()->GetLinearVelocity().x) < FLT_EPSILON &&
            fabs(((Block *)m_currBlock)->GetPhysicsBody()->GetLinearVelocity().y) < FLT_EPSILON)
        {
            if (m_currBlock->ContainsGameFlag((uint32)e_hdkGameFlagsBlockDrawRaw))
            {

                pan = PanToGameObject(m_currBlock->GetWorldCenter(),
                                      m_projection->GetWorldCenter(),
                                      m_projection->GetAABB());
                pan *= (interval/0.016);
                PanProjection(0.0f, 0.0f, pan.x * 5.0f, pan.y * 5.0f);
            }
        }
        else
        {
            TranslateProjection(pan);
        }
#endif
    }
}


static bool playedLoudCollisionSound = false;
static bool playedSoftCollisionSound = false;
static bool playedHeavySplashSound = false;
static bool playedLightSplashSound = false;
static bool playedBlockSmashSound = false;
static bool playedParticleSmashSound = false;

void AppLevelController::Step(double interval)
{
    if (m_currentLevel == NULL) return;

    PhysicsStep(interval);

    m_worldAABB = m_currentLevel->GetAABB();

    ++m_currBlockTicksSinceTapDown;

    playedLoudCollisionSound = false;
    playedSoftCollisionSound = false;
    playedHeavySplashSound = false;
    playedLightSplashSound = false;
    playedBlockSmashSound = false;
    playedParticleSmashSound = false;

    for (int i = 0; i < m_contactCount; ++i)
    {
        ContactPoint* point = m_contacts + i;
        this->HandleCollision(point);
    }

    if (m_interactionState != e_interactionDisabled)
    {
        if (m_fixedPhysicsInterval)
        {
            m_physicsParticleEmitter->Step(m_minFrameDuration);
        }
        else
        {
            m_physicsParticleEmitter->Step(interval);
        }
    }

    Game_Step(interval);

    m_currentLevel->Step();

    PanToSelectedObject(interval);
    m_panMomentumEnabled = ((m_interactionState != e_panAndZoom) || !m_tapDown); //(m_interactionState == e_waitingForInput);
    if (hdFastHypotenuse(m_lastPanDirectionVector.x, m_lastPanDirectionVector.y) > 0.0f)
    {
        m_lastPanDirectionVector = hdClamp(m_lastPanDirectionVector - (0.11f * m_lastPanDirectionVector),
                                           hdVec2(-70.0f, -70.0f), hdVec2(70.0f, 70.0f));

        if (m_panMomentumEnabled)
        {
            this->PanProjection(m_lastPanDirectionVector.x,
                                m_lastPanDirectionVector.y,
                                0.0f, 0.0f);
        }
    }
}


// Check the collisions and divert to collision
// game event handler functions.
void AppLevelController::HandleCollision(const ContactPoint* point)
{
    b2Body* b1 = point->body1;
    b2Body* b2 = point->body2;

    if (point->state == e_contactAdded && b1->GetUserData() != NULL && b2->GetUserData() != NULL)
    {
        hdPolygon* obj1 = (hdPolygon *)(b1->GetUserData());
        hdPolygon* obj2 = (hdPolygon *)(b2->GetUserData());

        if (obj1->GetUserType() == (int)e_hdkTypeBlock && obj2->GetUserType() == (int)kPhysicsParticleUserType)
        {
            HandleParticleBlockCollision((hdPhysicsParticle *)obj2, (Block *)obj1, point);
        }
        else if (obj2->GetUserType() == (int)e_hdkTypeBlock && obj1->GetUserType() == (int)kPhysicsParticleUserType)
        {
            HandleParticleBlockCollision((hdPhysicsParticle *)obj1, (Block *)obj2, point);
        }
        else if (obj1->GetUserType() == (int)e_hdkTypeBlock && obj2->GetUserType() == (int)e_hdkTypeBlock)
        {
            HandleBlockBlockCollision((Block *)obj1, (Block *)obj2, point);

        }
        else if (point->state == e_contactAdded)
        {
            PlaySoftCollisionSound((Block *)obj1, b1);
        }
    }

    this->Game_HandleCollision(point);
}


void AppLevelController::HandleParticleBlockCollision(hdPhysicsParticle *particle, Block *block, const ContactPoint *point)
{
    if (block == m_floor)
    {
        HandleFragmentSplash(particle, point);
        return;
    }

    if (block->IsHazard() ||
        block->GetMaterial() == e_hdkMaterialGrinder ||
        block->GetMaterial() == e_hdkMaterialBomb)
    {
        HandleParticleTap(particle, hdVec3(point->position.x, point->position.y, 0.0f), true);
        return;
    }

    // - Wood block on plastic particle means a particle tap if velocity > 1 m/s; no hazard collision.
    if (particle->m_parentTag == (unsigned int)e_hdkMaterialPlastic &&
        block->GetMaterial() == e_hdkMaterialWood)
    {
        HandleParticleTap(particle, hdVec3(point->position.x, point->position.y, 0.0f), true);
    }
}


void AppLevelController::HandleFragmentSplash(hdPhysicsParticle *fragment, const ContactPoint *point)
{
    if (false == fragment->m_interacts) return;

    if (m_currentLevel->GetFloorInfo() != NULL && m_currentLevel->GetFloorInfo()->m_isWater)
    {
        // Splash initial size is the depth of the falling item
        m_waveParticleEmitter->SetParticlePointSize(fabs(fragment->m_depth * 3.0f));

        // Update level
        m_waveParticleEmitter->SetLevel(m_currentLevel);

        // Ensure the splash begins the the same z-distance as the fragment.
        m_waveParticleEmitter->Start(hdVec3(point->position.x, point->position.y, fragment->GetWorldCenter().z));

        m_splashEmitter->SetNextSplashIsSmall(true);

        m_splashEmitter->Start(hdVec3(point->position.x, point->position.y, fragment->GetWorldCenter().z));

        if (playedLightSplashSound == false)
        {
            playedLightSplashSound = true;
            hdSound *sound = m_lightSplashSounds->GetItems()[rand() % m_lightSplashSounds->GetItemCount()];
            sound->volume = hdClamp(100.0f * float(fabs(point->body2->GetLinearVelocity().y)), 0.4f, 1.0f);
            sound->pitchOffset = hdRandom(0.9f, 1.05f);
            hdSoundManager::PlaySound(sound);
        }
        fragment->m_interacts = false; // so we don't get multiple splashes
    }
}


void AppLevelController::HandleBlockBlockCollision(Block *block1, Block *block2, const ContactPoint *point)
{
    Joint *joint;
    hdAABB combinedAABB;
    unsigned i;

    /*
     * Reject collisions between two blocks sharing the same revolute joint
     */
    if (block1->ContainsGameFlag(e_hdkGameFlagsBlockHasRevJoint) &&
        block2->ContainsGameFlag(e_hdkGameFlagsBlockHasRevJoint))
    {
        // If there is a joint connect these two, then reject collision.
        for (i = 0; i < m_currentLevel->GetJointCount(); ++i)
        {
            joint = (Joint *)m_currentLevel->GetJoints()[i];
            if (joint->GetJointType() == e_hdkJointTypeRevolute &&
                joint->GetPhysicsJoint() != NULL &&
                joint->GetFirstBody() == block1 &&
                joint->GetSecondBody() == block2)
            {
                return;
            }
        }
    }

    if (Game_HandleBlockBlockCollision(block1, block2, point)) return;

    if (block1 == m_floor)
    {
        this->HandleBlockLoss(hdVec3(point->position.x, point->position.y, 0.0f), block2);
        return;
    }
    else if (block1 == m_bottomBoundary)
    {
        this->HandleBlockWorldExit(hdVec3(point->position.x, point->position.y, 0.0f), block2);
        return;
    }

    if (block2 == m_floor)
    {
        this->HandleBlockLoss(hdVec3(point->position.x, point->position.y, 0.0f), block1);
        return;
    }
    else if (block2 == m_bottomBoundary)
    {
        this->HandleBlockWorldExit(hdVec3(point->position.x, point->position.y, 0.0f), block1);
        return;
    }

    PlayLoudCollisionSound(block1, point->body1);
    PlayLoudCollisionSound(block2, point->body2);


    /* Collision with the boundary means the block disappears */
    if (block1->IsGoal())
    {
        if (block2->GetMaterial() == e_hdkMaterialJack)
        {
            // Handle block to goal.
            Game_HandleGoalCollision(block1, block2, hdVec3(point->position.x, point->position.y, 0.0f));
            return;
        }
    }

    if (block2->IsGoal())
    {
        if (block1->GetMaterial() == e_hdkMaterialJack)
        {
            // Handle block to goal
            Game_HandleGoalCollision(block2, block1, hdVec3(point->position.x, point->position.y, 0.0f));
            return;
        }
    }

    if (block1->IsHazard() || block1->GetMaterial() == e_hdkMaterialGrinder)
    {
        if (block2->GetMaterial() != e_hdkMaterialBomb &&
            (block2->IsDestroyable() || block2->IsSpecial() || block2->IsDraggable()))
        {
            this->HandleBlockTap(block2, hdVec3(point->position.x, point->position.y, 0.0f), true);
            return;
        }
    }

    if (block2->IsHazard() || block2->GetMaterial() == e_hdkMaterialGrinder)
    {
        if (block1->GetMaterial() != e_hdkMaterialBomb &&
            (block1->IsDestroyable() || block1->IsSpecial() || block1->IsDraggable()))
        {
            this->HandleBlockTap(block1, hdVec3(point->position.x, point->position.y, 0.0f), true);
            return;
        }
    }

    // Destroy both blocks as long as the target is destroyable, not draggable.
    // No mutual bomb destruction allowed.

    if (block1->GetMaterial() == e_hdkMaterialBomb)
    {
        if (block2->GetMaterial() != e_hdkMaterialBomb &&
            (block2->IsDestroyable() || block2->IsSpecial()))
        {
            combinedAABB.lower = hdMin(block1->GetAABB().lower, block2->GetAABB().lower);
            combinedAABB.upper = hdMax(block1->GetAABB().upper, block2->GetAABB().upper);

            m_explosionEmitter->Start(combinedAABB);
            m_explosionSmokeEmitter->Start(combinedAABB);

            m_shotty->pitchOffset = 0.75f;
            hdSoundManager::PlaySound(m_shotty);

            this->HandleBlockTap(block1, hdVec3(point->position.x, point->position.y, 0.0f), true);
            this->HandleBlockTap(block2, hdVec3(point->position.x, point->position.y, 0.0f), true);
            return;
        }
    }

    if (block2->GetMaterial() == e_hdkMaterialBomb)
    {
        if (block1->GetMaterial() != e_hdkMaterialBomb &&
            (block1->IsDestroyable() || block1->IsSpecial()))
        {
            combinedAABB.lower = hdMin(block1->GetAABB().lower, block2->GetAABB().lower);
            combinedAABB.upper = hdMax(block1->GetAABB().upper, block2->GetAABB().upper);

            m_explosionEmitter->Start(combinedAABB);
            m_explosionSmokeEmitter->Start(combinedAABB);

            m_shotty->pitchOffset = 0.75f;
            hdSoundManager::PlaySound(m_shotty);

            this->HandleBlockTap(block1, hdVec3(point->position.x, point->position.y, 0.0f), true);
            this->HandleBlockTap(block2, hdVec3(point->position.x, point->position.y, 0.0f), true);
            return;
        }
    }


    if (block1->GetMaterial() == e_hdkMaterialPlastic)
    {
        if (block2->GetMaterial() != e_hdkMaterialPlastic &&
            (block2->IsDestroyable() || block2->IsSpecial()))
        {
            // if impact force was toward ground and with good enough speed, destroy block 1
            // ADDED: The contact point must also be ABOVE the plastic blocks center
            if (block2->GetPhysicsBody() != NULL &&
                block2->GetPhysicsBody()->GetLinearVelocity().y < 0.0f &&
                point->position.y > block1->GetWorldCenter().y &&
                hdFastHypotenuse(block2->GetPhysicsBody()->GetLinearVelocity().x, block2->GetPhysicsBody()->GetLinearVelocity().y) > 2.0f)
            {
                this->HandleBlockTap(block1, hdVec3(point->position.x, point->position.y, 0.0f), true);
            }
            return;
        }
    }

    if (block2->GetMaterial() == e_hdkMaterialPlastic)
    {
        if (block1->GetMaterial() != e_hdkMaterialPlastic &&
            (block1->IsDestroyable() || block1->IsSpecial()))
        {
            // if impact force was toward ground and with good enough speed, destroy block 1
            if (block1->GetPhysicsBody() != NULL &&
                block1->GetPhysicsBody()->GetLinearVelocity().y < 0.0f &&
                point->position.y > block2->GetWorldCenter().y &&
                hdFastHypotenuse(block1->GetPhysicsBody()->GetLinearVelocity().x, block1->GetPhysicsBody()->GetLinearVelocity().y) > 2.0f)
            {
                this->HandleBlockTap(block2, hdVec3(point->position.x, point->position.y, 0.0f), true);
            }
            return;
        }
    }
}



void AppLevelController::PlayLoudCollisionSound(const Block* block, const b2Body* body)
{
    float vol;
    hdSound *sound = NULL;

    if (playedLoudCollisionSound == false)
    {
        vol = 0.2f + (0.5f * fabs(body->GetLinearVelocity().y) + fabs(body->GetLinearVelocity().x));

        if (vol > 0.25f)
        {
            switch(((Block *)block)->GetMaterial())
            {
                case e_hdkMaterialWood:
                    sound = m_blockCollisionSoundsWood->GetItems()[rand() % m_blockCollisionSoundsWood->GetItemCount()];
                    break;
                case e_hdkMaterialMetal:
                    sound = m_blockCollisionSoundsMetal->GetItems()[rand() % m_blockCollisionSoundsMetal->GetItemCount()];
                    break;
                case e_hdkMaterialRock:
                case e_hdkMaterialCustomTexture:
                case e_hdkMaterialEarth:
                default:
                    sound = m_blockCollisionSoundsStone->GetItems()[rand() % m_blockCollisionSoundsStone->GetItemCount()];
                    break;
            }
            if (sound == NULL) return;

            sound->volume = vol;
            sound->pitchOffset = hdRandom(0.8f, 1.2f);
            hdSoundManager::PlaySound(sound);
            playedLoudCollisionSound = true;
        }
    }

    Game_PlayLoudCollisionSound(block, body);
}


void AppLevelController::PlaySoftCollisionSound(const Block* block, const b2Body* body)
{
    if (playedSoftCollisionSound == false)
    {
        float vol = 0.05f + (0.2f * fabs(body->GetLinearVelocity().y) + fabs(body->GetLinearVelocity().x));
        vol = hdClamp(vol, 0.05f, 0.3f);

        if (vol > 0.1f)
        {
            hdSound *sound = m_blockCollisionSoundsWood->GetItems()[rand() % m_blockCollisionSoundsWood->GetItemCount()];
            if (sound == NULL) return;

            sound->volume = hdClamp(100.0f * float(fabs(body->GetLinearVelocity().y)), 0.10f, 0.3f);
            sound->pitchOffset = hdRandom(0.85f, 1.1f);
            hdSoundManager::PlaySound(sound);
            playedSoftCollisionSound = true;
        }
    }
}


const hdSound* AppLevelController::GetCollisionSoundForMaterial(const e_hdkMaterial material) const
{
    hdSound *sound = NULL;

    switch(material)
    {
        case e_hdkMaterialBomb:
            sound = m_bombSelectSound;
            break;
        case e_hdkMaterialWood:
            sound = m_blockCollisionSoundsWood->GetItems()[rand() % m_blockCollisionSoundsWood->GetItemCount()];
            break;
        case e_hdkMaterialMetal:
            sound = m_blockCollisionSoundsMetal->GetItems()[rand() % m_blockCollisionSoundsMetal->GetItemCount()];
            break;
        case e_hdkMaterialRock:
        case e_hdkMaterialGrinder:
        case e_hdkMaterialPlastic:
        case e_hdkMaterialRawPolygon:
        case e_hdkMaterialText:
        case e_hdkMaterialJack:
        case e_hdkMaterialEarth:
            sound = m_blockCollisionSoundsStone->GetItems()[rand() % m_blockCollisionSoundsStone->GetItemCount()];
            break;
        case e_hdkMaterialCustomTexture:
        default:
            sound = m_dragSelectSound;

            break;
    }
    return sound;
}


bool AppLevelController::AddBlockToCurrentLevel(Block *block)
{
    if (m_currentLevel == NULL) return false;

    if (-1 == m_currentLevel->AddBlock(block))
    {
        return false;
    }

    if (block->IsDraggable())
    {
        if (m_buoyancyController != NULL && block->GetPhysicsBody() != NULL)
        {
            m_buoyancyController->AddBody((b2Body*)block->GetPhysicsBody());
        }
    }

    Game_AddBlockToLevel(block);
    return true;

}


#define ROTATION_EXPERIMENT 0
void AppLevelController::PushProjectionMatrix()
{
    hdAssert(!m_pushedProjectionMatrix);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glPushMatrix();

    hdMatrix mProjection;

#if (TARGET_OS_IPHONE == 1) || (TARGET_IPHONE_SIMULATOR == 1)
    MatrixPerspectiveFovRH(mProjection, 90.0f*(hd_pi/180.0f), m_ScreenAspectRatio, 1.0f, 10000.0f, false);
#else
    MatrixPerspectiveFovRH(mProjection, 67.38f*(hd_pi/180.0f), 1.0f/m_ScreenAspectRatio, 1.0f, 10000.0f, false);
#endif
    glMultMatrixf(mProjection.f);

#if ROTATION_EXPERIMENT == 1
    //glMatrixMode(GL_MODELVIEW);
    //glLoadIdentity();
    //glPushMatrix();
    glPushMatrix();
    glRotatef(-10.0f, 0.0f, 1.0f, 0.0f);
    //	MatrixRotationXYZAxis(mRot, 0.25f, hdVec3(0,1,0));
    //	glMultMatrixf(mRot.f);
    glPushMatrix();
#endif

#if (TARGET_OS_IPHONE == 1) || (TARGET_IPHONE_SIMULATOR == 1)
    if (IsLandscapeOrientation())
    {
        glRotatef(m_landscapeRotationZValue, 0.0f, 0.0f, 1.0f);
        glTranslatef(-m_projection->GetWorldCenter().x, -m_projection->GetWorldCenter().y,
                     -((m_projection->GetAABB().upper.y - m_projection->GetAABB().lower.y)/2.0f));
    }
    else
    {
        glRotatef(m_landscapeRotationZValue, 0.0f, 0.0f, 1.0f);
        glTranslatef(-m_projection->GetWorldCenter().x, -m_projection->GetWorldCenter().y,
                     -((m_projection->GetAABB().upper.y - m_projection->GetAABB().lower.y)/2.0f));
    }
#else
    glTranslatef(-m_projection->GetWorldCenter().x, -m_projection->GetWorldCenter().y,
                 -((m_projection->GetAABB().upper.x - m_projection->GetAABB().lower.x)/2.0f));
#endif

    hdglError("frustum");
    m_pushedProjectionMatrix = true;
}


void AppLevelController::PopProjectionMatrix()
{
    hdAssert(m_pushedProjectionMatrix);
    glMatrixMode(GL_PROJECTION);
#if ROTATION_EXPERIMENT == 1
    glPopMatrix();
    glPopMatrix();
#endif

    glPopMatrix();
    m_pushedProjectionMatrix = false;
}


void AppLevelController::DrawInternal()
{
    hdAABB reflectionAABB;
    Block *block;
    hdVec3 lo, hi;
    float width;

    PushProjectionMatrix();
    hdglBindTexture(NULL);

    if (m_currentLevel != NULL)
        m_currentLevel->Draw();

    reflectionAABB = m_currentLevel->GetScissorAABB(false);

    /*
     * Reflect our physics particles.
     */
    if (m_currentLevel->GetFloorInfo() != NULL &&
        m_currentLevel->GetFloorInfo()->m_isReflective &&
        m_reflectParticles &&
        m_physicsParticleEmitter->GetParticles()->GetItemCount() < 16 &&
        (fabs(reflectionAABB.upper.y - reflectionAABB.lower.y) > 0.0f))
    {
        GLfloat equation1[] = {0, -1.0f, 0, m_currentLevel->GetFloorDrawingLevel()};
        hdglClipPlanef(GL_CLIP_PLANE0, equation1);
        glEnable( GL_CLIP_PLANE0 );
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();

        glTranslatef(0.0f, -2.0f*(0.0f-m_currentLevel->GetFloorDrawingLevel()), 0.0f);
        glRotatef(180.0f, 1.0f, 0.0f, 0.0f);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        m_physicsParticleEmitter->SetViewportAABB(m_currentLevel->GetScissorAABB(false));
        m_physicsParticleEmitter->Draw();
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        glDisable( GL_CLIP_PLANE0 );
    }

    m_currentLevel->DrawFloor();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    m_physicsParticleEmitter->SetViewportAABB(m_currentLevel->GetScissorAABB(true));
    m_physicsParticleEmitter->Draw();
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);

    /*
     * Lights
     */
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    for (int i = 0; i < GetCurrentLevel()->GetBlockCount(); i++)
    {
        block = GetCurrentLevel()->GetBlocks()[i];
        if (block->GetBlockType() == e_hdkBlockTypeLight)
        {
            // Draw a quad on top of the block repping the light.
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glTranslatef(0.0f, 0.0f, fabs(block->GetDepth())+0.05f);
            width = 5.0f * (block->GetStartingAABB().upper.x - block->GetStartingAABB().lower.x);
            lo = block->GetWorldCenter() - hdVec3(width, width, width);
            hi = block->GetWorldCenter() + hdVec3(width, width, width);
            m_glow->SetAs2DBox(lo, hi);
            m_glow->Draw();
            glMatrixMode(GL_MODELVIEW);
            glPopMatrix();
        }
    }
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);


    glDepthMask(GL_FALSE);
    glEnable(GL_DEPTH_TEST);

    m_splashEmitter->Draw();
    m_splinterEmitter->Draw();
    m_smokeEmitter->Draw();
    m_explosionSmokeEmitter->Draw();
    m_explosionEmitter->Draw();

    m_gunSmokeEmitter->Draw();
    m_gunSparkEmitter->Draw();
    m_waveParticleEmitter->Draw();
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

#ifdef DEBUG2
    // Draw contact points.
    hdglBindTexture(NULL);
    glPointSize(5.0f);
    hdglBegin(GL_POINTS);
    for (int32 i = 0; i < m_contactCount; ++i)
    {
        hdglColor4f(1.0f, 1.0f, 1.0f, 0.75f);
        ContactPoint* point = m_contacts + i;
        hdglVertex3f(point->position.x, point->position.y, 0.0f);
    }
    hdglEnd();
#endif

    PopProjectionMatrix();
}


void AppLevelController::DrawSelectedBlockHighlight()
{
    float len;
    hdAABB draggableAABB;

    if (m_rayQueryCount > 0)
    {
        hdglBindTexture(NULL);
        PushProjectionMatrix();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#ifdef DEBUG2
        // DEBUG ray test lines and aabbs.
        hdglBegin(GL_LINES);
        hdVec3 center = m_projection->GetWorldCenter();
        hdglVertex3f(center.x, center.y, center.z + ((m_projection->GetAABB().upper.y - m_projection->GetAABB().lower.y)/2.0f));
        hdglVertex3f(m_lastRayScreenPoint.x, m_lastRayScreenPoint.y, m_lastRayScreenPoint.z);
        hdglEnd();

        hdglBegin(GL_POINTS);
        glPointSize(10.0f);
        hdglColor4f(1.0f, 1.0f, 0.0f, 1.0f);
        hdglVertex3f(m_testPoint.x, m_testPoint.y, m_testPoint.z + 0.01f);
        hdglEnd();

        hdAABB rayAABB;
        rayAABB.lower.Set(m_lastRayScreenPoint.x - 1.0f, m_lastRayScreenPoint.y - 1.0f, -100.0f);
        rayAABB.upper.Set(m_lastRayScreenPoint.x + 1.0f, m_lastRayScreenPoint.y + 1.0f, 100.0f);

        UtilDrawAABB(rayAABB);
#endif

        if (m_currBlock != NULL)
        {
            if (m_currBlock->IsDraggable())
            {
                draggableAABB = (hdAABB)(m_currBlock->GetStartingAABB());
                len = fabs(hdFastHypotenuse(draggableAABB.Width(),
                                            draggableAABB.Height()));

                m_glow->SetAs2DBox(m_currBlock->GetWorldCenter().x - (0.8f * len),
                                   m_currBlock->GetWorldCenter().y - (0.8f * len),
                                   1.6f * len,
                                   1.6f * len);

                glEnable(GL_TEXTURE_2D);
                glMatrixMode(GL_MODELVIEW);
                glPushMatrix();
                glTranslatef(0.0f, 0.0f, -fabs(m_currBlock->GetDepth()));
                m_glow->Draw();
                glPopMatrix();
                glDisable(GL_TEXTURE_2D);
            }

            glEnable(GL_DEPTH_TEST);

            if (m_currBlock->IsDestroyable() &&
                (GetBlockRiskiness(m_currBlock, m_contacts, m_contactCount)/2) > 3)
            {
                m_currBlock->DrawHighlight(m_projection->GetWorldCenter(), hdColor4(127, 127, 255, 127));
            }
            else
            {
                m_currBlock->DrawHighlight(m_projection->GetWorldCenter(), hdColor4(255, 255, 0, 127));
            }
            glDisable(GL_DEPTH_TEST);
        }

        glDisable(GL_BLEND);

        PopProjectionMatrix();
    }
}



/*
 * Crazy block interaction function.
 */
void AppLevelController::HandleTapUp(float x, float y, int tapCount)
{
    hdReceiver *hideClicked;
    hdReceiver *queueClicked;
    hdVec2 screen;
    totemMessage msg;

    m_tapDown = false;

    /*
     * Game_HandleTapUp might have something more important to do; if so, returns true.
     */
    if (Game_HandleTapUp(x, y, tapCount)) return;

    hideClicked = NULL;
    queueClicked = NULL;
    m_rayQueryCount = 0;

    ConvertRawToScreen(screen, x, y);

    if (m_interactionState != e_interactionDisabled)
    {
        if (m_mouseJoint != NULL)
        {
            DestroyMouseJoint();
            return;
        }
        else if (m_interactionState == e_interactionStateSelecting)
        {
            Block *curr = m_currBlock;

            /*
             * If nothing was selected on this up event, we still want to check if something was
             * selected on the down event. If there was something selected, we execute the tap up
             * event on this older object only if a short amount of time elapsed between the down
             * and up events.
             *
             * The motivation for this is to detect taps on blocks that are moving quickly and have
             * moved away from the player's finger before the player raises their finger off the
             * device.
             *
             * m_currBlockTicksSinceTapDown stores the number of ticks since the player pressed
             * on the block.
             *
             * While testing, players would feel cheated if they felt they accurately tapped the
             * block but didn't get any result. This little change massively improves the
             * playability of the game by dealing with quick, rapid, taps properly.
             */
            if (m_currBlockTicksSinceTapDown < 10)
            {
                hdVec3 tapPoint;
                m_currBlock = curr;

                if (m_currBlock)
                {
                    if (hdPolygonContainsPoint(m_currBlock->GetVertices(), m_currBlock->GetVertexCount(), m_mouseDownTestPoint))
                    {
                        tapPoint = m_mouseDownTestPoint;
                    }
                    else
                    {
                        hdVec3 tapDownUpDiff = m_currBlock->GetWorldCenter() - m_tapDownBlockWorldCenter;
                        tapPoint = m_mouseDownTestPoint + tapDownUpDiff;
                        if (!hdPolygonContainsPoint(m_currBlock->GetVertices(), m_currBlock->GetVertexCount(), tapPoint))
                        {
                            tapPoint = m_currBlock->GetWorldCenter();
                        }
                    }
                }
                hdPrintf("AAAAAAAAAAAAA");
                this->HandleBlockTap(m_currBlock, tapPoint, false);
            }
            else
            {
                hdPrintf("BBBBBBBBBBB");
                this->SelectBlockAtScreenPoint(screen.x, screen.y);
                if (curr!= NULL && curr == m_currBlock)
                {
                    this->HandleBlockTap(m_currBlock, m_testPoint, false);
                }
                else
                {
                    m_currBlock = NULL;
                }
            }
        }

        /*
         * HACK HACK HACK
         *
         * HandleBlockTap may change the interaction state to disabled. If that's the case, we
         * ignore this check.
         *
         * Fixing this flaw: a structural change is needed. The interaction state is a side effect
         * that can drastically alter the interaction model of the app and can make interface
         * quirks very difficult to debug. A state machine or some other solution will be needed
         * to fix it.
         */
        if (m_interactionState != e_interactionDisabled)
        {
            if ((m_currBlock == NULL ||
                 Game_CanSelectBlock(m_currBlock)) &&
                !IsShowingMessage()) {
                m_interactionState = e_waitingForInput;
            }
        }
        m_currBlock = NULL;
    }
}


void AppLevelController::HandleTapDown(float x, float y, int tapCount)
{
    hdVec2 screen;
    bool selected;

    m_tapDown = true;

    /*
     * Game_HandleTapDown might have something more important to do; if so, return.
     */
    if (Game_HandleTapDown(x, y, tapCount)) return;

    ConvertRawToScreen(screen, x, y);

    if (m_interactionState == e_interactionDisabled) return;

    if (m_interactionState == e_waitingForInput)
    {
        selected = this->SelectBlockAtScreenPoint(e_hdInterfaceClickStateDown, screen.x, screen.y);

        if (selected)
        {
            m_mouseDownTestPoint.Set(m_testPoint.x, m_testPoint.y, m_testPoint.z);
            m_interactionState = e_interactionStateSelecting;
        }
        else
        {
            return;
        }

        if (m_currBlock != NULL &&
            Game_CanSelectBlock(m_currBlock) &&
            m_currBlock->IsVisible())
        {
            // TODO: This code ostensibly has no purpose. Find out what it is here for.
            m_distanceFromScreenCenter = m_currBlock->GetWorldCenter() - m_projection->GetWorldCenter();

            // Position of block on tap down.
            m_tapDownBlockWorldCenter = m_currBlock->GetWorldCenter();

            Game_hdkBlockWasSelected(m_currBlock);

            // This tick counter time stamp will be reset on tap up.
            m_currBlockTicksSinceTapDown = 0;

            m_blockDeselectThreshold.lower.Set(screen.x-40.0f, screen.y-40.0f, 0.0f);
            m_blockDeselectThreshold.upper.Set(screen.x+40.0f, screen.y+40.0f, 0.0f);

            m_interactionState = e_interactionStateSelecting;

            // Fire the event if we tapped an event sender
            if (m_currBlock->IsEventSender())
            {
                Event *ev = NULL;
                for (int i = 0; i < m_currentLevel->GetEvents()->GetItemCount(); ++i)
                {
                    ev = m_currentLevel->GetEvents()->GetItems()[i];
                    if (ev->GetFirstObject() == (hdGameObject *)m_currBlock)
                    {
                        HandleSenderBlockTapDownEvent(m_currBlock, ev);
                    }
                }
                return;
            }
        }
    }
}


void AppLevelController::HandleSingleTap(float x, float y)
{
    Game_HandleSingleTap(x, y);
}


void AppLevelController::HandleDoubleTap(float x, float y)
{
    Game_HandleDoubleTap(x, y);
}


void AppLevelController::HandleTapMovedSingle(const float previousX, const float previousY,
                                              const float currentX, const float currentY)
{
    if (m_interactionState == e_interactionDisabled) return;

    if (Game_HandleTapMovedSingle(previousX, previousY, currentX, currentY)) return;

    hdVec2 screen;
    ConvertRawToScreen(screen, currentX, currentY);

    if (m_currBlock != NULL)
    {
        hdVec2 pp;
        this->ConvertScreenToInterface(pp, screen.x, screen.y);

        if (m_mouseJoint != NULL)
        {

            UpdateMouseJoint(pp.x, pp.y);
            return;
        }
        else
        {
            CreateMouseJoint();
        }

        /*
         If the player moves their finger outside the block deselect threshold then
         we deselect the current block - this allows players to pan around more
         easily without having to explicitly avoid putting their fingers on blocks
         and getting accidental explosions.
         */
        if (!m_currBlock->IsDraggable() &&
            m_mouseJoint == NULL &&
            m_interactionState == e_interactionStateSelecting)
        {
            if (!hdPointInsideAABB(hdVec2toVec3(screen), m_blockDeselectThreshold))
            {
                m_currBlock = NULL;
                m_interactionState = e_panAndZoom;
            }
        }
    }
    else
    {
        m_interactionState = e_panAndZoom;
        this->PanProjection(previousX, previousY, currentX, currentY);
        m_uiMessageQueueContainer->MouseOver(screen.x, screen.y);
    }

}


void AppLevelController::HandleTapMovedDouble(const float aPreviousX, const float aPreviousY,
                                              const float aCurrentX, const float aCurrentY,
                                              const float bPreviousX, const float bPreviousY,
                                              const float bCurrentX, const float bCurrentY)
{
    if (m_interactionState == e_interactionDisabled) return;

    m_currBlock = NULL;
    if (m_mouseJoint)
    {
        DestroyMouseJoint();
    }
    const float kZoomFactor = 0.1f;
    this->ZoomProjection(kZoomFactor * aPreviousX, kZoomFactor * aPreviousY,
                         kZoomFactor * aCurrentX,  kZoomFactor * aCurrentY,
                         kZoomFactor * bPreviousX, kZoomFactor * bPreviousY,
                         kZoomFactor * bCurrentX,  kZoomFactor * bCurrentY);
}


bool AppLevelController::SelectBlockAtScreenPoint(float x, float y)
{
    return SelectBlockAtScreenPoint(e_hdInterfaceClickStateUp, x, y);
}


bool AppLevelController::SelectBlockAtScreenPoint(e_hdInterfaceClickState clickState, float x, float y)
{
    hdVec2 pp;
    bool particleFound;
    hdVec3 ray[2];
    hdVec3 center;
    const int k_maxCount = 10;
    hdGameObject* objs[k_maxCount];
    hdGameObject* top;

    this->ConvertScreenToInterface(pp, x, y);
    center = m_projection->GetWorldCenter();
    ray[0].Set(center.x, center.y, center.z + ((m_projection->GetAABB().upper.y - m_projection->GetAABB().lower.y)/2.0f));
    ray[1].Set(pp.x, pp.y, 0.0f);
    m_lastRayScreenPoint.Set(pp.x, pp.y, 0.0f);

    if (clickState == e_hdInterfaceClickStateDown)
    {
        m_currBlock = NULL;
        top = NULL;
        particleFound = false;

        m_rayQueryCount = m_gameWorld->RayQuery((unsigned int)e_hdkTypeBlock |
                                                (unsigned int)kPhysicsParticleUserType,
                                                ray, objs, k_maxCount, m_testPoint);

        /*
         * If first is a selectable block, then set curr_block = top and break.
         * If a particle, call HandleParticleTap until we get a block.
         */
        if (m_rayQueryCount == 0) return false;

        for (int i = 0; i < hdMin(m_rayQueryCount, k_maxCount); ++i)
        {
            if (objs[i]->GetUserType() == (int)kPhysicsParticleUserType)
            {
                break;
            }
            else if (objs[i]->GetUserType() == (int)e_hdkTypeBlock &&
                     Game_CanSelectBlock((Block*)objs[i]))
            {
                top = objs[i];
                break;
            }
        }

        if (top != NULL)
        {
            m_currBlock = (Block*)top;
            return true;
        }

        for (int i = 0; i < hdMin(m_rayQueryCount, k_maxCount); ++i)
        {
            if (objs[i]->GetUserType() == (int)kPhysicsParticleUserType)
            {
                HandleParticleTap((hdPhysicsParticle *)objs[i], m_testPoint, false);
                particleFound = true;
            }
            else if (objs[i]->GetUserType() == (int)e_hdkTypeBlock &&
                     Game_CanSelectBlock((Block*)objs[i]))
            {
                break;
            }
        }
        return particleFound;
    }
    else
    {
        m_rayQueryCount = m_gameWorld->RayQuery((unsigned int)e_hdkTypeBlock, ray, objs, k_maxCount, m_testPoint);

        for (int i = 0; i < hdMin(m_rayQueryCount,k_maxCount); ++i)
        {
            if (objs[i]->GetUserType() == (int)e_hdkTypeBlock)
            {
                if (Game_CanSelectBlock((Block*)objs[i]))
                {
                    m_currBlock = (Block*)objs[i];
                    return true;
                }
            }
        }
        m_currBlock = NULL;
        return false;
    }
}


// TODO: A collision action depends entirely on what kind of game is loaded
void AppLevelController::HandleBlockLoss(const hdVec3& contactPoint, Block* block)
{
#ifdef DEBUG2
    hdPrintf("[I] AppLevelController::HandleBlockLoss contact point at (%3.5f, %3.5f, %3.5f)", contactPoint.x, contactPoint.y, contactPoint.z);
#endif

    // A block that is already "lost" cannot be lost again.
    if (block->ContainsGameFlag(e_hdkGameFlagsBlockLost)) return;
    if (block->GetPhysicsBody() == NULL) return;

    if (m_currentLevel->GetFloorInfo()->m_isWater)
    {
        m_waveParticleEmitter->SetParticlePointSize(hdMax(1.2f, float(fabs(block->GetDepth()*3.0f))));
        m_waveParticleEmitter->SetLevel(m_currentLevel);
        m_waveParticleEmitter->Start(contactPoint + hdVec3(0, 0.02f, 0));

        if (block->GetPhysicsBody()->GetLinearVelocity().y < -4.0f)
        {
            m_splashEmitter->SetNextSplashIsSmall(false);
            m_splashEmitter->Start(contactPoint.x, contactPoint.y);

            if (!playedHeavySplashSound)
            {
                hdSoundManager::PlaySound(m_blockLossSounds->GetItems()[rand() % m_blockLossSounds->GetItemCount()]);
                playedHeavySplashSound = true;
            }
        }
    }

    // Convert the block into a particle, and then destroy it.
    // The particle ensures that the block fades out and disappears on loss.
    if (Game_HandleBlockLoss(block, contactPoint))
    {
        block->Step(); // In case the block is off screen and has not been updated in a while.
    }
}


void AppLevelController::HandleBlockWorldExit(const hdVec3& contactPoint, Block* block)
{
    if (block->ContainsGameFlag(e_hdkGameFlagsBlockLost))
    {
        block->DestroyPhysicsBody();
        m_gameWorld->RemoveGameObject(block);
    }
}


void AppLevelController::CreateMouseJoint()
{
    if (m_mouseJoint != NULL) return;

    if (m_currBlock->IsDraggable() && (NULL != m_currBlock->GetPhysicsBody()))
    {
        if (m_HACK_lastJointedBlock != NULL &&
            m_HACK_lastJointedBlock->GetPhysicsBody() != NULL)
        {
            // Fixed rot not always reset; program crashes due to assertion
            // at line 75 on b2RevoluteJoint.cpp
            ((b2Body*)m_HACK_lastJointedBlock->GetPhysicsBody())->SetFixedRotation(false);
            m_HACK_lastJointedBlock = NULL;
        }

        b2MouseJointDef md;
        md.body1 = (b2Body*)m_topBlock->GetPhysicsBody();
        md.body2 = (b2Body*)m_currBlock->GetPhysicsBody();
        md.collideConnected = true;
        if (m_currBlock->GetTag() == 0 || m_currBlock->GetTag() == 1001)
        {
            md.body2->SetFixedRotation(true);
        }
#ifdef IPHONE_BUILD
        if (m_currBlock->GetMaterial() == e_hdkMaterialBomb)
        {
            md.target = b2Vec2(m_currBlock->GetWorldCenter().x, m_currBlock->GetWorldCenter().y-1.25f);
        }
        else
        {
            md.target = b2Vec2(m_currBlock->GetWorldCenter().x, m_currBlock->GetWorldCenter().y-0.25f);
        }
#else
        if (m_currBlock->GetMaterial() == e_hdkMaterialBomb)
        {
            md.target = b2Vec2(m_currBlock->GetWorldCenter().x, m_currBlock->GetWorldCenter().y);
        }
        else
        {
            md.target = b2Vec2(m_currBlock->GetWorldCenter().x, m_currBlock->GetWorldCenter().y);
        }
#endif

        md.maxForce = 80.0f * (m_currBlock->GetPhysicsBody())->GetMass();
        m_mouseJoint = (b2MouseJoint*)m_physicsWorld->CreateJoint(&md);
        md.body2->WakeUp();

        m_HACK_lastJointedBlock = m_currBlock;
    }
}


void AppLevelController::UpdateMouseJoint(float x, float y)
{
    if (m_mouseJoint == NULL) return;
    if (m_currBlock == NULL) return;
    m_mouseJoint->SetTarget(b2Vec2(x, y));
}


void AppLevelController::DestroyMouseJoint()
{
    if (m_mouseJoint == NULL) return;

    m_physicsWorld->DestroyJoint(m_mouseJoint);
    m_mouseJoint = NULL;
    m_interactionState = e_waitingForInput;

    if (m_currBlock &&
        m_currBlock->GetPhysicsBody() &&
        ((b2Body*)m_currBlock->GetPhysicsBody())->IsFixedRotation())
    {
        if (m_currBlock->GetTag() == 0 || m_currBlock->GetTag() == 1001)
        {
            ((b2Body*)m_currBlock->GetPhysicsBody())->SetFixedRotation(false);
        }
    }
}


void AppLevelController::HandleSenderBlockTapDownEvent(const Block* sender, Event* ev)
{
    Joint *receiver;
    switch(ev->GetEventType())
    {
        case e_hdkEventTypeFireGun:
            break;
        case e_hdkEventTypeShowMessage:
            break;
        case e_hdkEventTypeJointMotorStartPos:
            if (ev->GetSecondObject()->GetUserType() == (int)e_hdkTypeJoint)
            {
                receiver = (Joint *)ev->GetSecondObject();
                JointMotorStart(receiver, true);
            }
            break;
        case e_hdkEventTypeJointMotorStartNeg:
            if (ev->GetSecondObject()->GetUserType() == (int)e_hdkTypeJoint)
            {
                receiver = (Joint *)ev->GetSecondObject();
                JointMotorStart(receiver, false);
            }
            break;
        case e_hdkEventTypeJointMotorStop:
            
            break;
        default:
            break;
    }
}


void AppLevelController::HandleSenderBlockTapUpEvent(const Block* sender, Event* ev)
{
    switch(ev->GetEventType())
    {
        case e_hdkEventTypeFireGun:
        {
            // The second body emits a bullet from the world center 
            // in the direction of the axis of rotation.
            Block *receiver = (Block *)ev->GetSecondObject();
            
            hdVec3 center = receiver->GetWorldCenter();
            hdVec3 line(0.0f, ((receiver->GetAABB().upper.y - receiver->GetAABB().lower.y)/2.0f), 0.0f);
            hdVec3 out;
            hdMatrix rotMatrix;
            
            MatrixRotationZ(rotMatrix, receiver->GetOBB().transform.rotation.z);
            MatrixVec3Multiply(out, line, rotMatrix);
            
            hdVec2 aa, bb;
            aa.Set(center.x + out.x - 0.05f, center.y + out.y - 0.05f);
            bb.Set(center.x + out.x + 0.05f, center.y + out.y + 0.05f);
            
            Block *block = new Block(m_gameWorld, m_physicsWorld, aa, bb, e_hdkMaterialMetal, 
                                               e_hdkShapeTypeCylinder, e_hdkBlockTypeHazardDest);
            m_currentLevel->AddBlock(block);
            block->SetDepth(-0.05f);
            b2Body *body = (b2Body *)block->GetPhysicsBody();
            
            body->ApplyForce(b2Vec2(out.x * 250.0f, out.y * 250.0f), body->GetWorldCenter());
            body->SetBullet(true);
            
            m_gunSmokeEmitter->Start(block->GetWorldCenter().x, block->GetWorldCenter().y);
            m_gunSparkEmitter->SetDirection(out.x, out.y, out.z);
            m_gunSparkEmitter->Start(block->GetWorldCenter().x, block->GetWorldCenter().y);
            
            hdSoundManager::PlaySound(m_shotty);
        }
            break;
        case e_hdkEventTypeShowMessage:
            // Get the receiver, and its texture
            // Set the UIMessageboard texture to this, and then show
            // Game must be paused!
        {
            Block *receiver = (Block *)ev->GetSecondObject();
            
            if (receiver == NULL) return;
            
            m_interactionState = e_interactionDisabled;
            SetMessage(receiver->GetTexture());
            this->ShowMessage();			
            m_questionEmitter->m_onStopResume = false;
        }
            break;
        case e_hdkEventTypeJointMotorStartPos:
        case e_hdkEventTypeJointMotorStartNeg:
        case e_hdkEventTypeJointMotorStop:
        {
            Joint *receiver = (Joint *)ev->GetSecondObject();
            JointMotorStop(receiver);	
        }	
            break;
        default:
            break;
    }
}


/*
 * User tapped a block.
 *
 * The result of a tap on a block will be specific to the type of the block being tapped. In lieu
 * of specific subclasses of Block, we have to deal  with taps on a case-by-case basis 
 * according to the values of various flags on the block.
 *
 * TODO: HandleBlockTap is not the way to do this, however.
 *
 * A material class would perhaps be a better way. We can then handle specific interactions for the
 * specific material.
 *
 * However, something still needs to determine what the actual interaction consists of. If someone 
 * fires a gun at a block, do we need to pass on the type of gun? What about retaining previous 
 * knowledge of interactions and combining them with future ones?
 *
 * This feels like the sort of thing that could be solved with a software pattern -- the material 
 * idea is just the strategy pattern. It's not adequate however, because the specific collision or 
 * tap action depends on two things (for material collisions, the collision action for item A 
 * depends on the material type of item B, and vice versa).
 */
void AppLevelController::HandleBlockTap(Block* block, const hdVec3& impactPoint, bool hazardCollision)
{
    polygon_info_t polygonInfo;
    
    Event *ev;
    uint32 i;
    
    hdSound *sound;
    
    hdAABB startAABB;
    hdVec3 center, orig, trans;
    
    if (block == NULL) return;
    
    // Event senders send special events; other blocks can send messages
    // if there's something in the scripts file for it.
    if (block->IsEventSender() && false == hazardCollision)
    {
        ev = NULL;
        for (i = 0; i < m_currentLevel->GetEvents()->GetItemCount(); ++i)
        {
            if (m_currentLevel->GetEvents()->GetItems()[i]->GetFirstObject() == block)
            {
                ev = m_currentLevel->GetEvents()->GetItems()[i];
                HandleSenderBlockTapUpEvent(block, ev);
            }
        }
        if (ev != NULL) return;
    }
    
    if (block->GetTag() > 0 && false == hazardCollision)
    {
        LoadScriptMessageForTag(block->GetTag(), false);
    }
    
    if (block->GetPhysicsBody() == NULL)
    {
        return;
    }
    
    /* 
     * Will stop here if the currently loaded game allows a tap on the selected block.
     */
    if (!Game_HandleBlockTap(block, impactPoint, hazardCollision)) return;
    
    /*
     * If there's a mouse joint attached to the block, destroy it.
     */
    if (m_mouseJoint) {
        DestroyMouseJoint();
    }
    
    /* 
     * In case the block is off screen and has not been updated in a while 
     */
    if (hazardCollision) {
        block->Step();
    }
    
    /*
     * At this point, we want to deal with the block tap.
     */
    
    /*
     * Destroy block physics info so it no longer affects the world.
     */
    block->DestroyPhysicsBody(); 
    m_gameWorld->RemoveGameObject(block);
    
    if (playedBlockSmashSound == false)
    {
        sound = m_blockExplosionSoundsStone->GetItems()[rand() % m_blockExplosionSoundsStone->GetItemCount()];
        sound->volume = 0.9f;
        sound->pitchOffset = hdRandom(0.85f, 1.15f);
        hdSoundManager::PlaySound(sound);
        playedBlockSmashSound = true;
    }
    
    /*
     * Bombs do their own thing
     */
    if (block->GetMaterial() == e_hdkMaterialBomb) {
        return;
    }
    
    /*
     * Smash the block into tiny pieces and then emit those pieces with physics info.
     */
    {
        /*
         * Find the impact point as if it were on the original block.
         */
        startAABB = block->GetStartingAABB();
        center = startAABB.lower + (0.5f * (startAABB.upper - startAABB.lower));
        
        orig.Set(impactPoint.x, impactPoint.y, impactPoint.z);
        trans = block->GetWorldCenter() - center;
        
        hdTranslateVertices(&orig, 1, -(block->GetWorldCenter()));
        hdRotateVertices(&orig, 1, -block->GetOBB().transform.rotation);
        hdTranslateVertices(&orig, 1, block->GetWorldCenter());
        
        startAABB.lower = startAABB.lower + trans;
        startAABB.upper = startAABB.upper + trans;
        polygonInfo.impactTexCoord = block->GetTextureCoordForPoint(startAABB, orig);
        
        polygonInfo.texture = (hdTexture *)block->GetTexture();
        
        polygonInfo.tint[0] = block->GetTint()[0]; 
        polygonInfo.tint[1] = block->GetTint()[1];
        polygonInfo.tint[2] = block->GetTint()[2];
        polygonInfo.tint[3] = block->GetTint()[3];
        
        polygonInfo.vertices = block->GetVertices();
        polygonInfo.startingVertices = block->GetStartingVertices();
        polygonInfo.texCoords = block->GetTextureCoordinates();
        polygonInfo.vertexCount = block->GetVertexCount();
        
        polygonInfo.worldCenter = block->GetWorldCenter();
        polygonInfo.impactPoint = impactPoint;
        polygonInfo.depth = block->GetDepth();
        polygonInfo.zOffset = block->GetZOffset();
        polygonInfo.parentType = block->GetUserType();
        polygonInfo.parentTag = block->GetMaterial();
        
        m_physicsParticleEmitter->StartWithTexture(polygonInfo);
    }
    
    hdAABB blockAABB;
    blockAABB.lower = (block->GetDepthAABB().lower + hdVec3(0.1f, 0.1f, 0.1f));
    blockAABB.upper = (block->GetDepthAABB().upper - hdVec3(0.1f, 0.1f, 0.1f));
    
    m_smokeEmitter->Start(blockAABB);
    
    m_splinterEmitter->SetParticlePointSize(fabs(block->GetDepth()));
    m_splinterEmitter->SetParticleColor(hdColor4(block->GetTint()[0], block->GetTint()[1], block->GetTint()[2], 1.0f));
    m_splinterEmitter->Start(block->GetTexture(), blockAABB);
}


/*
 * Block particles are slightly different to normal blocks.
 */
void AppLevelController::HandleParticleTap(hdPhysicsParticle* particle, const hdVec3& impactPoint, bool hazardCollision)
{
    polygon_info_t polygonInfo;
    
    /*
     * Game unit and texture unit calculations.
     */
    hdAABB texAABB;
    hdVec3 vDiff, diff;
    hdVec2 tDiff;
    
    float tpgx, tpgy;
    
    /*
     * Interaction can be turned off - don't allow taps.
     */
    if (false == particle->m_interacts) return;
    
    /*
     * Need a physics body
     */
    if (particle->m_physicsBody == NULL) return;
    
    /*
     * TODO: Fade out threshold so we don't get the somewhat distrubing effect
     * of resurrecting fading particles.
     */
    if (particle->life <= 0.0f) return;
    
    /*
     * Fading particles don't get destroyed by hazard collisions.
     */
    if (hazardCollision && (particle->m_duration - particle->life) < 0.25f) return;
    
    if (Game_HandleParticleTap(particle, impactPoint, hazardCollision))
    {
        if (!hdPolygonContainsPoint(particle->GetVertices(),
                                    particle->GetVertexCount(),
                                    impactPoint))
        {
            polygonInfo.impactPoint.Set(particle->GetWorldCenter().x, 
                                        particle->GetWorldCenter().y,
                                        particle->GetWorldCenter().z);
        }
        else
        {
            polygonInfo.impactPoint.Set(impactPoint.x, 
                                        impactPoint.y, 
                                        impactPoint.z);
        }
        
        /*
         * - Find texture units per game unit
         * - Diff between impact point and lowest vertex
         * - new tex coord = lowest tex coord + (Diff * tex units)
         */
        texAABB.lower.Set(particle->GetTextureCoordinates()[0].x,
                          particle->GetTextureCoordinates()[0].y,
                          0.0f);
        texAABB.upper.Set(particle->GetTextureCoordinates()[0].x,
                          particle->GetTextureCoordinates()[0].y,
                          0.0f);
        for (int i = 0; i < particle->GetVertexCount(); ++i)
        {
            texAABB.lower = hdMin(texAABB.lower, hdVec2toVec3(particle->GetTextureCoordinates()[i]));
            texAABB.upper = hdMax(texAABB.upper, hdVec2toVec3(particle->GetTextureCoordinates()[i]));
        }
        
        hdVec2 tDiff = hdVec3toVec2(texAABB.upper - texAABB.lower);
        
        hdVec3 vDiff = (particle->GetAABB().upper - particle->GetAABB().lower);
        
        tpgx = tDiff.x / vDiff.x; 
        tpgy = tDiff.y / vDiff.y;
        
        diff = polygonInfo.impactPoint - particle->GetAABB().lower;
        
        polygonInfo.impactTexCoord = hdVec2(tpgx * diff.x, 1.0f - (tpgy * diff.y));
        
        polygonInfo.texture = particle->m_texture;
        polygonInfo.tint[0] = particle->m_tint[0];
        polygonInfo.tint[1] = particle->m_tint[1];
        polygonInfo.tint[2] = particle->m_tint[2];
        polygonInfo.tint[3] = particle->m_tint[3];
        polygonInfo.vertices = particle->GetVertices();
        polygonInfo.startingVertices = particle->GetStartingVertices();
        polygonInfo.texCoords = particle->GetTextureCoordinates();
        
        polygonInfo.vertexCount = particle->GetVertexCount();
        polygonInfo.worldCenter = particle->GetWorldCenter();
        
        polygonInfo.depth = particle->m_depth;
        polygonInfo.zOffset = 0.0f;
        polygonInfo.parentType = particle->GetUserType();
        polygonInfo.parentTag = particle->m_parentTag;
        
        m_physicsParticleEmitter->StartWithTexture(polygonInfo);
    }
    
    hdAABB blockAABB;
    blockAABB.lower = (particle->GetAABB().lower + hdVec3(0.01f, 0.01f, 0.01f));
    blockAABB.upper = (particle->GetAABB().upper - hdVec3(0.01f, 0.01f, 0.01f));
    
    m_splinterEmitter->SetParticlePointSize(fabs(particle->m_depth));
    m_splinterEmitter->SetParticleColor(hdColor4(particle->m_tint[0], 
                                                 particle->m_tint[1],
                                                 particle->m_tint[2], 
                                                 1.0f));
    m_splinterEmitter->Start(particle->m_texture, blockAABB);
    
    if (!playedParticleSmashSound)
    {
        hdSound *sound = m_blockExplosionSoundsStone->GetItems()[rand() % m_blockExplosionSoundsStone->GetItemCount()];
        sound->volume = 0.4f;
        sound->pitchOffset = hdRandom(1.25f, 1.5f);
        hdSoundManager::PlaySound(sound);
        playedParticleSmashSound = true;
    }
    
    particle->DestroyPhysicsBody(m_physicsWorld);
    particle->life = 0.0f;
}


const bool AppLevelController::Game_InitLevel(const Level *level, const int levelId)
{
    return true;
}


void AppLevelController::Game_Step(double interval) {}


void AppLevelController::Game_ResetCurrentLevel() {}


const bool AppLevelController::Game_CanSelectBlock(const Block *block)
{
    return (!block->IsStatic());
}


void AppLevelController::Game_hdkBlockWasSelected(const Block *block) const {}


const bool AppLevelController::Game_HandleTapUp(float x, float y, int tapCount)
{
    return false;
}


const bool AppLevelController::Game_HandleTapDown(float x, float y, int tapCount)
{
    return false;
}


const bool AppLevelController::Game_HandleSingleTap(float x, float y)
{
    return false;
}


const bool AppLevelController::Game_HandleDoubleTap(float x, float y)
{
    return false;
}


const bool AppLevelController::Game_HandleTapMovedSingle(const float previousX, const float previousY, 
                                                         const float currentX, const float currentY)
{
    return false;
}


const bool AppLevelController::Game_HandleTapMovedDouble(const float aPreviousX, const float aPreviousY,
                                                         const float aCurrentX, const float aCurrentY,
                                                         const float bPreviousX, const float bPreviousY,	
                                                         const float bCurrentX, const float bCurrentY)
{
    return false;
}


const bool AppLevelController::Game_HandleBlockTap(Block *block, const hdVec3& impactPoint, bool hazardCollision)
{
    return true;
}


const bool AppLevelController::Game_HandleParticleTap(hdPhysicsParticle* particle, const hdVec3& impactPoint, bool hazardCollision)
{
    return true;
}


const bool AppLevelController::Game_HandleBlockLoss(Block* block, const hdVec3& contactPoint) 
{
    return false;
}


void AppLevelController::Game_HandleCollision(const ContactPoint* point) {}


bool AppLevelController::Game_HandleBlockBlockCollision(Block *block1, Block *block2, const ContactPoint *point)
{
    return false;
}


void AppLevelController::Game_PlayLoudCollisionSound(const Block* block, const b2Body* body) {}


void AppLevelController::Game_PlaySoftCollisionSound(const Block* block, const b2Body* body) {}


void AppLevelController::Game_AddBlockToLevel(const Block* block) {}


void AppLevelController::Game_HandleGoalCollision(Block *goalBlock, Block *block, const hdVec3& impactPoint) {}
