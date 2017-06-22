/*
 * Copyright (c) 2017 Hackdirt Ltd.
 * Author: David Petrie (david@davidpetrie.com)
 *
 * License: https://www.gnu.org/licenses/gpl-3.0.en.html
 */

#include "GameController.h"

GameController::GameController(const hdInterfaceController* parentController) : AppLevelController(parentController)
{
    m_specialBlocks = new hdTypedefList<Block*, kGameControllerSpecialsCount>();
    ticksSinceLastUpdate = 0;
    ticksInfoCycle = 0;
    currGoalInfoCycle = 0;
    m_heavyCollisionCount = 0;
    m_levelStats = NULL;
    m_hasShownWinMessage = false;

    this->InitAnimations();
    this->InitSounds();
    this->InitInterface();
    this->RefreshLayout();
}


GameController::~GameController()
{
    delete m_btnShowMenu;
    delete m_btnFinishLevel;
    delete m_btnFailRestart;
    delete m_btnBG;
    delete m_btnMenuRestart;
    delete m_btnMenuQuit;
    delete m_btnMenuSkip;
    delete m_btnSettings;
    delete m_btnMenuClose;
    delete m_btnLobbyReadyPortrait;
    delete m_btnLobbyReadyLandscape;
    delete m_gameButtonLayer;
    delete m_menuButtonLayer;
    delete m_fontLevelName;
    delete m_fontLevelGoals;
    delete m_interfaceProjection;

    delete m_goalFailedMessageBox;
    delete m_goalCompleteMessageBox;
    delete m_specialGoalMessageBox;
    delete m_goalDescriptionMessageText;
    delete m_showGoalProgress;

    delete m_fontProgress;
    delete m_fontStats;
    delete m_fontLostMessage;
    delete multiplierFont;
    delete midAirChain;
    delete m_lostMessage;
    delete m_fragmentsText;

    delete uiImageContainer;
    delete imgEmptyMedal;
    delete imgBronzeMedal;
    delete imgSilverMedal;
    delete imgGoldMedal;
    delete midAirComboBox;
    delete m_completionMessageBox;
    delete m_fragmentsBox;
    delete m_lostMessageBox;
    delete m_winMessage1;
    delete m_winMessage2;
    delete m_winMessage3;
    delete m_failMessage;
    delete m_newHighScore;

    hdAnimationController::Instance()->StopAnimations(this);
    hdAnimationController::Instance()->PurgeAnimations(this);

    m_notificationPolygons->RemoveAll();

    delete m_notificationPolygons;
    delete m_specialBlocks;
    delete m_animPolygons;
    delete m_gameWorldAnimPolygons;
}


void GameController::InitInterface()
{
    hdAABB interfaceAABB;
    interfaceAABB.lower = hdVec3(0,0,100.0f);
    interfaceAABB.upper = hdVec3(m_PixelScreenWidth, m_PixelScreenHeight, -100.0f);
    m_interfaceProjection = new hdOrthographicProjection(NULL, interfaceAABB);

    m_gameButtonLayer = new hdUIContainer(NULL, interfaceAABB.lower, interfaceAABB.upper);
    m_btnShowMenu = new hdButton("/Interface/Textures/btnMenuNormal.png",
                                 "/Interface/Textures/btnMenuOver.png",
                                 "/Interface/Textures/btnMenuNormal.png",
                                 NULL);

    m_btnFinishLevel = new hdButton("/Interface/Textures/btnFinishLevelNormal.png",
                                    "/Interface/Textures/btnFinishLevelOver.png",
                                    "/Interface/Textures/btnFinishLevelOver.png",
                                    NULL);

    m_btnLobbyReadyLandscape = new hdButton("/Interface/Textures/btnStartPlayingNormal.png",
                                            "/Interface/Textures/btnStartPlayingOver.png",
                                            "/Interface/Textures/btnStartPlayingOver.png",
                                            NULL);

    m_btnLobbyReadyPortrait = new hdButton("/Interface/Textures/btnStartPlayingNormal.png",
                                           "/Interface/Textures/btnStartPlayingOver.png",
                                           "/Interface/Textures/btnStartPlayingOver.png",
                                           NULL);
    m_btnFinishLevel->Hide();

    m_btnFailRestart = new hdButton("/Interface/Textures/btnFailRestartNormal.png",
                                    "/Interface/Textures/btnFailRestartOver.png",
                                    "/Interface/Textures/btnFailRestartOver.png",
                                    NULL);
    m_btnFailRestart->Hide();

    m_gameButtonLayer->Add(m_btnShowMenu);

    m_gameButtonLayer->Add(m_btnLobbyReadyLandscape);
#ifdef IPHONE_BUILD
    m_gameButtonLayer->Add(m_btnLobbyReadyPortrait);
#endif

    m_btnLobbyReadyPortrait->SetAs2DBox(-220.0f, 50.0f, 220.0f, 100.0f);
    m_btnLobbyReadyLandscape->SetAs2DBox(-220.0f, 25.0f, 220.0f, 100.0f);
    m_btnLobbyReady = m_btnLobbyReadyLandscape;

    uiImageContainer = new hdUIContainer(NULL, interfaceAABB.lower, interfaceAABB.upper);

    imgEmptyMedal = new hdUIImage("/Materials/s_stargoalempty_15.tga", NULL);
    imgBronzeMedal = new hdUIImage("/Materials/s_stargoalbronze_15.tga", NULL);
    imgSilverMedal = new hdUIImage("/Materials/s_stargoalsilver_15.tga", NULL);
    imgGoldMedal  = new hdUIImage("/Materials/s_stargoalgold_15.tga", NULL);

    uiImageContainer->Add(imgEmptyMedal);

    m_menuButtonLayer = new hdUIContainer(NULL, interfaceAABB.lower, interfaceAABB.upper);
    m_btnBG = new hdButton("/Materials/s_glow_15.tga",
                           "/Materials/s_glow_15.tga",
                           "/Materials/s_glow_15.tga",
                           NULL);
    m_btnBG->Disable();
    m_btnMenuSkip = new hdButton("/Interface/Textures/btnSkipLevelNormal.png",
                                 "/Interface/Textures/btnSkipLevelOver.png",
                                 "/Interface/Textures/btnSkipLevelNormal.png",
                                 NULL);

    m_btnSettings = new hdButton("/Interface/Textures/btnSettingsNormal.png",
                                 "/Interface/Textures/btnSettingsOver.png",
                                 "/Interface/Textures/btnSettingsNormal.png",
                                 NULL);

    m_btnMenuRestart = new hdButton("/Interface/Textures/btnRestartLevelNormal.png",
                                    "/Interface/Textures/btnRestartLevelOver.png",
                                    "/Interface/Textures/btnRestartLevelNormal.png",
                                    NULL);

    m_btnMenuQuit = new hdButton("/Interface/Textures/btnQuitLevelNormal.png",
                                 "/Interface/Textures/btnQuitLevelOver.png",
                                 "/Interface/Textures/btnQuitLevelNormal.png",
                                 NULL);

    m_btnMenuClose = new hdButton("/Interface/Textures/btnCloseNormal.png",
                                  "/Interface/Textures/btnCloseOver.png",
                                  "/Interface/Textures/btnCloseNormal.png",
                                  NULL);

    m_menuButtonLayer->Add(m_btnBG);
    m_menuButtonLayer->Add(m_btnMenuRestart);
    m_menuButtonLayer->Add(m_btnMenuQuit);
    m_menuButtonLayer->Add(m_btnSettings);
    m_menuButtonLayer->Add(m_btnMenuSkip);
    m_menuButtonLayer->Add(m_btnMenuClose);

    m_menuButtonLayer->Hide();

    midAirComboBox = new hdUIImage("Interface/Textures/imgMidairComboNote.png", NULL);
    midAirComboBox->SetAlpha(0.0f);
    uiImageContainer->Add(midAirComboBox);

    multiplierFont = new hdFontPolygon(GAME_CONTROLLER_PERCENT_FONT, NULL);
    multiplierFont->SetAlpha(0.0f);
    multiplierFont->AlignLeft();

    midAirChain = new hdFontPolygon(GAME_CONTROLLER_NOTE_FONT, NULL);
    midAirChain->SetTint(0.0f, 0.0f, 0.0f, 0.75f);
    midAirChain->SetAlpha(0.0f);
    midAirChain->AlignLeft();

    uiImageContainer->Add(multiplierFont);
    uiImageContainer->Add(midAirChain);

    m_lostMessageBox = new hdUIImage("Interface/Textures/imgLostMessage.png", NULL);
    m_lostMessageBox->SetAlpha(1.0f);
    uiImageContainer->Add(m_lostMessageBox);

    m_lostMessage = new hdFontPolygon(GAME_CONTROLLER_NOTE_FONT, NULL);
    m_lostMessage->SetTint(0.0f, 0.0f, 0.0f, 1.0f);
    m_lostMessage->SetAlpha(0.0f);
    m_lostMessage->AlignLeft();
    uiImageContainer->Add(m_lostMessage);

    /*
     * Fonts
     */
    m_fontLevelName = new hdFontPolygon(GAME_CONTROLLER_LARGE_FONT, NULL);
    m_fontLevelName->SetTint(1.0f, 1.0f, 1.0f, 1.0f);
    m_fontLevelName->AlignCenter();
    m_animPolygons->Add(m_fontLevelName);

    m_fontLevelGoals = new hdFontPolygon(GAME_CONTROLLER_MED_FONT, NULL);
    m_fontLevelGoals->SetTint(1.0f, 1.0f, 1.0f, 1.0f);
    m_fontLevelGoals->AlignCenter();
    m_animPolygons->Add(m_fontLevelGoals);

    m_fontStats = new hdFontPolygon(GAME_CONTROLLER_STATS_FONT, NULL);
    m_fontStats->SetTint(1.0f, 1.0f, 1.0f, 1.0f);
    m_fontStats->SetLineHeightScale(0.9f);
    m_fontStats->AlignRight();
    m_animPolygons->Add(m_fontStats);

    m_fontProgress = new hdFontPolygon(GAME_CONTROLLER_PERCENT_FONT, NULL);
    m_fontProgress->SetTint(1.0f, 1.0f, 1.0f, 1.0f);
    m_fontProgress->AlignLeft();
    m_animPolygons->Add(m_fontProgress);

    m_fontLostMessage = new hdFontPolygon(GAME_CONTROLLER_NOTE_FONT, NULL);
    m_fontLostMessage->SetTint(0.0f, 0.0f, 0.0f, 0.75f);
    m_fontLostMessage->SetRenderToTexture(false);
    uiImageContainer->Add(m_fontLostMessage);

    m_panMomentumEnabled = true;

    m_starParticleEmitter = new StarEmitter(12, "/Materials/s_stargoalgold_15.tga", -200.0f, 100.0f, 24.0f, 1.5f);
    m_starParticleEmitter->m_ySpreadLower = -32.0f;
    m_starParticleEmitter->m_ySpreadUpper = 96.0f;

    m_stargoalbronze = hdTextureManager::Instance()->FindTexture("/Materials/s_stargoalbronze_15.tga", TT_Sprite);
    m_stargoalsilver = hdTextureManager::Instance()->FindTexture("/Materials/s_stargoalsilver_15.tga", TT_Sprite);
    m_stargoalgold = hdTextureManager::Instance()->FindTexture("/Materials/s_stargoalgold_15.tga", TT_Sprite);

    /*
     * Goal Messages
     */
#ifdef IPHONE_BUILD
    m_goalFailedMessageBox = new hdUIImage("Interface/Textures/imgGoalFailedMessage.png", NULL);
#else
    m_goalFailedMessageBox = new hdUIImage("Interface/Textures/imgGoalFailedMessage_512.png", NULL);
#endif
    m_goalFailedMessageBox->SetAlpha(0.0f);
    uiImageContainer->Add(m_goalFailedMessageBox);

    m_goalCompleteMessageBox = new hdUIImage("Interface/Textures/imgGoalCompleteMessage.png", NULL);
    m_goalCompleteMessageBox->SetAlpha(0.0f);
    uiImageContainer->Add(m_goalCompleteMessageBox);

    m_specialGoalMessageBox = new hdUIImage("Interface/Textures/imgSpecialGoalMessage.png", NULL);
    m_specialGoalMessageBox->SetAlpha(0.0f);
    uiImageContainer->Add(m_specialGoalMessageBox);

    m_newHighScore = new hdUIImage("Interface/Textures/imgNewHighScore.png", NULL);
    m_newHighScore->SetAlpha(0.0f);
    uiImageContainer->Add(m_newHighScore);

    m_goalDescriptionMessageText = new hdFontPolygon(GAME_CONTROLLER_MED_FONT, NULL);
    m_goalDescriptionMessageText->SetTint(1.0f, 1.0f, 1.0f, 1.0f);
    m_goalDescriptionMessageText->AlignCenter();
    uiImageContainer->Add(m_goalDescriptionMessageText);
    m_goalDescriptionMessageText->Hide();

    /*
     * Fragements messages
     */
    m_fragmentsBox = new hdUIImage("Interface/Textures/imgFragmentsBox.png", NULL);
    m_fragmentsBox->SetAlpha(1.0f);
    uiImageContainer->Add(m_fragmentsBox);

    m_fragmentsText = new hdFontPolygon(GAME_CONTROLLER_MED_FONT, NULL);
    m_fragmentsText->SetTint(0.0f, 0.0f, 0.0f, 0.75f);
    m_fragmentsText->SetAlpha(0.0f);
    m_fragmentsText->AlignRight();
    uiImageContainer->Add(m_fragmentsText);

    /*
     * Win messages
     */
    m_completionMessageBox = new hdUIImage("Interface/Textures/imgMessageBoxRight.png", NULL);
    m_completionMessageBox->SetAlpha(1.0f);
    uiImageContainer->Add(m_completionMessageBox);

    m_winMessage1 = new hdUIImage("/Interface/Textures/img75CompleteSmall.png", NULL);
    m_winMessage1->SetAlpha(0.0f);
    m_winMessage2 = new hdUIImage("/Interface/Textures/img90CompleteSmall.png", NULL);
    m_winMessage2->SetAlpha(0.0f);
    m_winMessage3 = new hdUIImage("/Interface/Textures/img100CompleteSmall.png", NULL);
    m_winMessage3->SetAlpha(0.0f);

    m_failMessage = new hdUIImage("/Interface/Textures/imgFail.png", NULL);

    uiImageContainer->Add(m_winMessage1);
    uiImageContainer->Add(m_winMessage2);
    uiImageContainer->Add(m_winMessage3);
    uiImageContainer->Add(m_failMessage);

    m_showGoalProgress = new hdButton(NULL, NULL, NULL, NULL);
    m_showGoalProgress->Enable();
    m_gameButtonLayer->Add(m_showGoalProgress);

    /*
     * Callbacks
     */
    m_btnFinishLevel->SetMouseDownListener((AppInterface *)this, AppInterface::GenericButtonDownCallback);
    m_btnFailRestart->SetMouseDownListener((AppInterface *)this, AppInterface::GenericButtonDownCallback);
    m_btnSettings->SetMouseDownListener((AppInterface *)this, AppInterface::GenericButtonDownCallback);
    m_showGoalProgress->SetMouseDownListener((AppInterface *)this, AppInterface::GenericButtonDownCallback);
    m_btnShowMenu->SetMouseDownListener((AppInterface *)this, AppInterface::GenericButtonDownCallback);
    m_btnLobbyReady->SetMouseDownListener((AppInterface *)this, AppInterface::GenericButtonDownCallback);

    m_btnShowMenu->SetMouseUpListener(this, GameController::btnShowMenuUpCallback);
    m_btnMenuClose->SetMouseUpListener(this, GameController::btnCloseMenuUpCallback);
    m_btnSettings->SetMouseUpListener(this, GameController::btnSettingsUpCallback);
    m_btnMenuRestart->SetMouseUpListener(this, GameController::btnMenuRestartCallback);
    m_btnMenuQuit->SetMouseUpListener(this, GameController::btnMenuQuitCallback);
    m_btnMenuSkip->SetMouseUpListener(this, GameController::btnMenuSkipCallback);

    DisableMenu();
}



void GameController::InitSounds()
{
    m_btnMenuClickDownSound = hdSoundManager::FindSound("Sounds/btnClickDown.caf", e_soundTypeNormal);
    m_btnMenuClickUpSound = hdSoundManager::FindSound("Sounds/btnClick01.caf", e_soundTypeNormal);
    m_btnMenuClickUpSoundDeep = hdSoundManager::FindSound("Sounds/btnClickBack01.caf", e_soundTypeNormal);

    /*
     * Misc. Sounds
     */
    m_levelWin01Sound = hdSoundManager::FindSound("Sounds/levelWin_01.caf", e_soundTypeNormal);
    m_levelWin02Sound = hdSoundManager::FindSound("Sounds/levelWin_01.caf", e_soundTypeNormal);
    m_levelWin03Sound = hdSoundManager::FindSound("Sounds/levelWin_01.caf", e_soundTypeNormal);
    m_levelFailSound  = hdSoundManager::FindSound("Sounds/levelFail.caf", e_soundTypeNormal);
    m_levelFailSound->volume = 0.6f;

    m_midairSound = hdSoundManager::FindSound("Sounds/levelMidairBonus.caf", e_soundTypeNormal);
    m_midairSound->volume = 0.3f;

    m_midairComboSound = hdSoundManager::FindSound("Sounds/levelFinishButton01.caf", e_soundTypeNormal);
    m_midairComboSound->volume = 0.5f;

    m_soundIllegalTouch = hdSoundManager::FindSound("Sounds/illegalMove.caf", e_soundTypeNormal);
    m_soundIllegalTouch->volume = 0.7f;

    m_soundIllegalMove = hdSoundManager::FindSound("Sounds/illegalMove.caf", e_soundTypeNormal);

    m_levelFinishButton01Sound = hdSoundManager::FindSound("Sounds/levelFinishButton.caf", e_soundTypeNormal);
    m_btnLevelRestartSound = hdSoundManager::FindSound("Sounds/btnLevelRestart.caf", e_soundTypeNormal);

    m_blockZap01Sound = hdSoundManager::FindSound("Sounds/blockZap01.caf", e_soundTypeNormal);
    m_blockZap01Sound->volume = 0.5f;

    m_blockSelectSound = hdSoundManager::FindSound("Sounds/blockSelect01.caf", e_soundTypeNormal);
    m_blockZap01Sound->volume = 0.5f;

    m_jackCollectionSound = hdSoundManager::FindSound("Sounds/jackCollect01.caf", e_soundTypeNormal);
    m_jackCollectionSound->volume = 0.65f;

    m_Bonus50FragmentsSound = hdSoundManager::FindSound("Sounds/Bonus50Fragments.caf", e_soundTypeNormal);

    m_vibrationSound = hdSoundManager::FindSound("Vibration", e_soundTypeVibration);
}


void GameController::InitLevelInterface()
{
    m_gameButtonLayer->Show();
    m_btnFailRestart->Hide();
    m_btnFinishLevel->Hide();
    m_menuButtonLayer->Hide();

    imgEmptyMedal->Show();
    imgBronzeMedal->Hide();
    imgSilverMedal->Hide();
    imgGoldMedal->Hide();

    m_btnFinishLevel->MoveTo(hdVec3(m_PixelScreenWidth/2.0f, -80.0f, 0.0f));
    m_btnFinishLevel->Enable();

    m_btnFailRestart->MoveTo(hdVec3(m_PixelScreenWidth/2.0f, -80.0f, 0.0f));
    m_btnFailRestart->Enable();

    m_btnLobbyReadyPortrait->Enable();
    m_btnLobbyReadyLandscape->Enable();
}


void GameController::RefreshLayout()
{
    hdAABB interfaceAABB;

    if (m_interfaceProjection != NULL)
    {
        delete m_interfaceProjection;
        m_interfaceProjection = NULL;
    }
    interfaceAABB.lower = hdVec3(0,0,100.0f);
    interfaceAABB.upper = hdVec3(m_PixelScreenWidth, m_PixelScreenHeight, -100.0f);
    m_interfaceProjection = new hdOrthographicProjection(NULL, interfaceAABB);

#ifdef IPHONE_BUILD
    /*
     * Positioning.
     */
    m_btnShowMenu->SetAs2DBox(2.0f, m_PixelScreenHeight - 40.0f, 50.0f, 40.0f);
    m_btnFinishLevel->SetAs2DBox((m_PixelScreenWidth - 200.0f)/2.0f, 0.0f, 200.0f, 80.0f);
    m_btnFailRestart->SetAs2DBox((m_PixelScreenWidth - 200.0f)/2.0f, 0.0f, 200.0f, 80.0f);

    imgEmptyMedal->SetAs2DBox(60.0f, m_PixelScreenHeight - 40.0f, 40.0f, 40.0f);
    imgBronzeMedal->SetAs2DBox(60.0f, m_PixelScreenHeight - 40.0f, 40.0f, 40.0f);
    imgSilverMedal->SetAs2DBox(60.0f, m_PixelScreenHeight - 40.0f, 40.0f, 40.0f);
    imgGoldMedal->SetAs2DBox(60.0f, m_PixelScreenHeight - 40.0f, 40.0f, 40.0f);

    /*
     * Left slider messages
     */
    midAirComboBox->SetAs2DBox(-140.0f, m_PixelScreenHeight - 125.0f, 140.0f, 50.0f);
    multiplierFont->SetAs2DBox(10.0f, m_PixelScreenHeight - 115.0f, 310.0f, 40.0f);
    midAirChain->SetAs2DBox(42.0f, m_PixelScreenHeight - 142.0f, 300.0f, 40.0f);

    m_fragmentsBox->SetAs2DBox(-140.0f, m_PixelScreenHeight - 85.0f, 140.0f, 50.0f);
    m_fragmentsText->SetAs2DBox(92.0f, m_PixelScreenHeight - 87.0f, 35.0f, 40.0f);

    /*
     * Right slider messages.
     */
    m_lostMessageBox->SetAs2DBox(m_PixelScreenWidth, m_PixelScreenHeight -125.0f, 140.0f, 50.0f);
    m_lostMessage->SetAs2DBox(m_PixelScreenWidth - 124.0f, m_PixelScreenHeight - 143.0f, 310.0f, 40.0f);

    m_completionMessageBox->SetAs2DBox(m_PixelScreenWidth, m_PixelScreenHeight - 85.0f, 140.0f, 50.0f);
    m_winMessage1->SetAs2DBox(m_PixelScreenWidth - 136.0f, m_PixelScreenHeight-79.0f,  120.0f, 40.0f);
    m_winMessage2->SetAs2DBox(m_PixelScreenWidth - 136.0f, m_PixelScreenHeight-79.0f,  120.0f, 40.0f);
    m_winMessage3->SetAs2DBox(m_PixelScreenWidth - 136.0f, m_PixelScreenHeight-79.0f,  120.0f, 40.0f);

    m_fontStats->SetAs2DBox(m_PixelScreenWidth - 160.0f, m_PixelScreenHeight - 34.0f, 150, 30);
    m_fontProgress->SetAs2DBox(m_PixelScreenWidth - 208.0f, m_PixelScreenHeight - 29.0f, 100, 35);

    m_goalFailedMessageBox->SetAs2DBox(m_PixelScreenWidth, m_PixelScreenHeight, 300.0f, 100.0f);
    m_goalCompleteMessageBox->SetAs2DBox(m_PixelScreenWidth, m_PixelScreenHeight, 200.0f, 70.0f);
    m_specialGoalMessageBox->SetAs2DBox(m_PixelScreenWidth, m_PixelScreenHeight, 200.0f, 70.0f);
    m_goalDescriptionMessageText->SetAs2DBox(m_PixelScreenWidth-30.0f, m_PixelScreenHeight-80.0f, 240.0f, 25.0f);
    m_newHighScore->SetAs2DBox(m_PixelScreenWidth, m_PixelScreenHeight, 200.0f, 70.0f);

    m_showGoalProgress->SetAs2DBox(hdVec2(m_PixelScreenWidth - 213.0f, m_PixelScreenHeight - 42.0f),
                                   hdVec2(m_PixelScreenWidth - 5.0f,		m_PixelScreenHeight - 5.0f));

    if (IsLandscapeOrientation())
    {
        m_btnBG->SetAs2DBox((m_PixelScreenWidth - m_PixelScreenHeight)/2.0f, 0.0f, m_PixelScreenHeight, m_PixelScreenHeight);

        m_btnMenuClose->SetAs2DBox((m_PixelScreenWidth - 160.0f)/2.0f, 275.0f, 50.0f, 35.0f);

        m_btnSettings->SetAs2DBox((m_PixelScreenWidth - 160.0f)/2.0f, 145.0f, 160.0f, 55.0f);

        m_btnMenuRestart->SetAs2DBox((m_PixelScreenWidth - 160.0f)/2.0f, 80.0f, 160.0f, 55.0f);
        m_btnMenuQuit->SetAs2DBox((m_PixelScreenWidth - 160.0f)/2.0f, 15.0f, 160.0f, 55.0f);

#ifdef DEBUG
        m_btnMenuSkip->SetAs2DBox((m_PixelScreenWidth - 200.0f)/2.0f + 50.0f, 260.0f, 100.0f, 35.0f);
#else
        m_btnMenuSkip->Hide();
#endif

        m_fontLevelName->SetAs2DBox((m_PixelScreenWidth - 320.0f)/2.0f, 260.0f, 320, 20);
        m_fontLevelGoals->SetAs2DBox((m_PixelScreenWidth - 320.0f)/2.0f, 130.0f, 320, 80);

        m_btnLobbyReady = m_btnLobbyReadyLandscape;
        m_btnLobbyReadyLandscape->Show();
        m_btnLobbyReadyPortrait->Hide();
    }
    else
    {
        m_btnBG->SetAs2DBox(0.0f, (m_PixelScreenHeight - m_PixelScreenWidth)/2.0f, m_PixelScreenHeight, m_PixelScreenHeight);

        m_btnMenuClose->SetAs2DBox((m_PixelScreenWidth - 160.0f)/2.0f, 345.0f, 50.0f, 35.0f);

        m_btnSettings->SetAs2DBox((m_PixelScreenWidth - 160.0f)/2.0f , 225.0f, 160.0f, 55.0f);

        m_btnMenuRestart->SetAs2DBox((m_PixelScreenWidth - 160.0f)/2.0f, 165.0f, 160.0f, 55.0f);
        m_btnMenuQuit->SetAs2DBox((m_PixelScreenWidth - 160.0f)/2.0f, 100.0f, 160.0f, 55.0f);


#ifdef DEBUG
        m_btnMenuSkip->SetAs2DBox((m_PixelScreenWidth - 200.0f)/2.0f + 50.0f, 340.0f, 100.0f, 35.0f);
#else
        m_btnMenuSkip->Hide();
#endif

        m_fontLevelName->SetAs2DBox((m_PixelScreenWidth - 220.0f)/2.0f, m_PixelScreenHeight - 120.0f, 240, 20);
        m_fontLevelGoals->SetAs2DBox((m_PixelScreenWidth - 260.0f)/2.0f, m_PixelScreenHeight - 240.0f, 260, 20);

        m_btnLobbyReady = m_btnLobbyReadyPortrait;
        m_btnLobbyReadyLandscape->Hide();
        m_btnLobbyReadyPortrait->Show();
    }

#else
    float messageBoxWidth = hdClamp(0.3 * m_PixelScreenWidth, 140.0, 256.0);
    float messageBoxHeight = hdClamp(0.16 * m_PixelScreenHeight, 50.0, 91.0);
    float winMessageWidth = messageBoxWidth * 0.98;
    float winMessageHeight = messageBoxHeight * 0.8;

    float framentsTextX = 0.68 * messageBoxWidth;
    float fragmentsMessageBoxY = m_PixelScreenHeight - 40.0f - messageBoxHeight;
    float midairComboBoxY = m_PixelScreenHeight - 40.0f - (2.0f * messageBoxHeight);
    float completionBoxY = fragmentsMessageBoxY;
    float winMessageY = completionBoxY + 6.0f;

    float lostBoxY = completionBoxY;

    /*
     * Positioning.
     */
    m_btnShowMenu->SetAs2DBox(2.0f, m_PixelScreenHeight - 40.0f, 50.0f, 40.0f);
    m_btnFinishLevel->SetAs2DBox((m_PixelScreenWidth - 256.0f)/2.0f, 0.0f, 256.0f, 120.0f);
    m_btnFailRestart->SetAs2DBox((m_PixelScreenWidth - 256.0f)/2.0f, 0.0f, 256.0f, 120.0f);

    imgEmptyMedal->SetAs2DBox(60.0f, m_PixelScreenHeight - 40.0f, 40.0f, 40.0f);
    imgBronzeMedal->SetAs2DBox(60.0f, m_PixelScreenHeight - 40.0f, 40.0f, 40.0f);
    imgSilverMedal->SetAs2DBox(60.0f, m_PixelScreenHeight - 40.0f, 40.0f, 40.0f);
    imgGoldMedal->SetAs2DBox(60.0f, m_PixelScreenHeight - 40.0f, 40.0f, 40.0f);

    /*
     * Left slider messages
     */
    midAirComboBox->SetAs2DBox(-messageBoxWidth, midairComboBoxY, messageBoxWidth, messageBoxHeight);
    multiplierFont->SetAs2DBox(20.0f, midairComboBoxY + 18.0f, 310.0f, 48.0f);
    midAirChain->SetAs2DBox((0.08 * m_PixelScreenWidth), midairComboBoxY - 5.0f, 300.0f, 40.0f);

    m_fragmentsBox->SetAs2DBox(-messageBoxWidth, fragmentsMessageBoxY, messageBoxWidth, messageBoxHeight);
    m_fragmentsText->SetAs2DBox(framentsTextX, fragmentsMessageBoxY + 8.0f, 35.0f, 48.0f);

    /*
     * Right slider messages.
     */
    m_lostMessageBox->SetAs2DBox(m_PixelScreenWidth, lostBoxY, messageBoxWidth, messageBoxHeight);
    m_lostMessage->SetAs2DBox(m_PixelScreenWidth - (messageBoxWidth - 40.0f), lostBoxY + 15.0f, 310.0f, 20.0f);

    m_completionMessageBox->SetAs2DBox(m_PixelScreenWidth, completionBoxY, messageBoxWidth, messageBoxHeight);
    m_winMessage1->SetAs2DBox(m_PixelScreenWidth - winMessageWidth, winMessageY, winMessageWidth, winMessageHeight);
    m_winMessage2->SetAs2DBox(m_PixelScreenWidth - winMessageWidth, winMessageY, winMessageWidth, winMessageHeight);
    m_winMessage3->SetAs2DBox(m_PixelScreenWidth - winMessageWidth, winMessageY, winMessageWidth, winMessageHeight);

    m_fontStats->SetAs2DBox(m_PixelScreenWidth - 160.0f, m_PixelScreenHeight - 34.0f, 150, 30);
    m_fontProgress->SetAs2DBox(m_PixelScreenWidth - 208.0f, m_PixelScreenHeight - 29.0f, 100, 35);

    m_goalFailedMessageBox->SetAs2DBox(m_PixelScreenWidth, m_PixelScreenHeight, 420.0f, 140.0f);
    m_goalCompleteMessageBox->SetAs2DBox(m_PixelScreenWidth, m_PixelScreenHeight, 200.0f, 70.0f);
    m_specialGoalMessageBox->SetAs2DBox(m_PixelScreenWidth, m_PixelScreenHeight, 200.0f, 70.0f);
    m_goalDescriptionMessageText->SetAs2DBox(m_PixelScreenWidth-30.0f, m_PixelScreenHeight-80.0f, 240.0f, 25.0f);
    m_newHighScore->SetAs2DBox(m_PixelScreenWidth, m_PixelScreenHeight, 200.0f, 70.0f);

    m_showGoalProgress->SetAs2DBox(hdVec2(m_PixelScreenWidth - 213.0f, m_PixelScreenHeight - 42.0f),
                                   hdVec2(m_PixelScreenWidth - 5.0f,		m_PixelScreenHeight - 5.0f));
    m_btnLobbyReadyLandscape->SetAs2DBox(-256.0f, 25.0f, 256.0f, 128.0f);
    if (IsLandscapeOrientation())
    {
        m_btnBG->SetAs2DBox((m_PixelScreenWidth - m_PixelScreenHeight)/2.0f, 0.0f, m_PixelScreenHeight, m_PixelScreenHeight);

        m_btnMenuClose->SetAs2DBox((m_PixelScreenWidth - 160.0f)/2.0f, 275.0f, 50.0f, 35.0f);

        m_btnSettings->SetAs2DBox((m_PixelScreenWidth - 160.0f)/2.0f, 145.0f, 160.0f, 55.0f);

        m_btnMenuRestart->SetAs2DBox((m_PixelScreenWidth - 160.0f)/2.0f, 80.0f, 160.0f, 55.0f);
        m_btnMenuQuit->SetAs2DBox((m_PixelScreenWidth - 160.0f)/2.0f, 15.0f, 160.0f, 55.0f);

#ifdef DEBUG
        m_btnMenuSkip->SetAs2DBox((m_PixelScreenWidth - 200.0f)/2.0f + 50.0f, 260.0f, 100.0f, 35.0f);
#else
        m_btnMenuSkip->Hide();
#endif
        m_fontLevelName->SetAs2DBox(0.0f, m_PixelScreenHeight * 0.5f + 120.0f, m_PixelScreenWidth, 20);
        m_fontLevelGoals->SetAs2DBox(0.0f, (m_PixelScreenHeight * 0.5f), m_PixelScreenWidth, 20);

        m_btnLobbyReady = m_btnLobbyReadyLandscape;
        m_btnLobbyReadyLandscape->Show();
        m_btnLobbyReadyPortrait->Hide();
    }
    else
    {
        m_btnBG->SetAs2DBox(0.0f, (m_PixelScreenHeight - m_PixelScreenWidth)/2.0f, m_PixelScreenHeight, m_PixelScreenHeight);

        m_btnMenuClose->SetAs2DBox((m_PixelScreenWidth - 160.0f)/2.0f, 345.0f, 50.0f, 35.0f);

        m_btnSettings->SetAs2DBox((m_PixelScreenWidth - 160.0f)/2.0f , 225.0f, 160.0f, 55.0f);

        m_btnMenuRestart->SetAs2DBox((m_PixelScreenWidth - 160.0f)/2.0f, 165.0f, 160.0f, 55.0f);
        m_btnMenuQuit->SetAs2DBox((m_PixelScreenWidth - 160.0f)/2.0f, 100.0f, 160.0f, 55.0f);

#ifdef DEBUG
        m_btnMenuSkip->SetAs2DBox((m_PixelScreenWidth - 200.0f)/2.0f + 50.0f, 340.0f, 100.0f, 35.0f);
#else
        m_btnMenuSkip->Hide();
#endif

        m_fontLevelName->SetAs2DBox(0.0f, m_PixelScreenHeight * 0.5f, m_PixelScreenWidth, 20);
        m_fontLevelGoals->SetAs2DBox(0.0f, (m_PixelScreenHeight * 0.5f) + 120.0f, m_PixelScreenWidth, 20);

        m_btnLobbyReady = m_btnLobbyReadyPortrait;
        m_btnLobbyReadyLandscape->Hide();
        m_btnLobbyReadyPortrait->Show();
    }
#endif
}


const bool GameController::Game_InitLevel(const Level* level, const int levelId)
{
    unsigned i;
    Block *block;
    Joint *joint;

    hdAssert(level != NULL);

    gameInfo = GameInfo();

    for (i = 0; i < level->GetBlockCount(); ++i)
    {
        block = (Block *)level->GetBlocks()[i];

        if (block->IsDestroyable())
        {
            if (block->GetMaterial() == e_hdkMaterialJack)
            {
                ++gameInfo.jacksTotal;
            }
            else if (block->GetMaterial() == e_hdkMaterialGrinder)
            {
                // TODO: grinder counts
            }
            else
            {
                ++gameInfo.destroyableBlocks;
            }
        }

        if (block->ContainsGameFlag(e_hdkGameFlagsBlockLost))
        {
            block->RemoveGameFlag(e_hdkGameFlagsBlockLost);
        }

        if (block->IsGoal())
        {
            GoalBlock* goal = new GoalBlock(block);
            if (-1 != m_specialBlocks->Add(goal))
            {
                ((Level *)level)->AddBlock(goal);
            }
        }
    }

    gameInfo.gamePoints = 0;
    gameInfo.destroyedBlocksCurrent = 0;
    gameInfo.lostBlocksCurrent = 0;

    gameInfo.midAirSmashesTotal = 0;
    gameInfo.midAirSmashesCurrent = 0;
    gameInfo.riskySmashesTotal = 0;
    gameInfo.riskySmashesCurrent = 0;

    gameInfo.totalMidAirChains = 0;
    gameInfo.longestMidAirChain = 0;

    gameInfo.gameStarted = false;
    gameInfo.failEvent = false;

    // Current progress message - so it doesn't repeat
    m_currentProgressMessage = NULL;

    /*
     * Goal Setup
     */
    memset(m_goals, 0, sizeof(m_goals));
    m_goalCount = Scripts_GetGoalsForLevel(m_goals, 5, levelId);
    if (m_goalCount == 0)
    {
        ++m_goalCount;

        m_goals[0].levelId = levelId;
        m_goals[0].goalType = e_hdkGoalTypeNormal;
        m_goals[0].value = 0;

        if (gameInfo.jacksTotal > 0)
        {
            ++m_goalCount;

            m_goals[1].levelId = levelId;
            m_goals[1].goalType = e_hdkGoalTypeJackCollection;
            m_goals[1].value = gameInfo.jacksTotal;
        }
    }

    m_interactionState = e_interactionDisabled;

    char info[256];
    snprintf(info, 256, "0%%");
    m_fontProgress->SetText(info);

    m_fontLevelName->SetTextFormatted("%s %d:\n%s",
                                      Scripts_GetStringForKey("Level").c_str(),
                                      levelId,
                                      Scripts_GetStringForKey(level->GetLevelName()).c_str());

    char levelGoals[256];
    snprintf(levelGoals, 256, "\n");
    for (i = 0; i < m_goalCount; ++i)
    {
        if (m_goals[i].goalType == e_hdkGoalTypeNormal)
        {
            snprintf(m_goals[i].description, 256, "Smash at least %d blocks",
                     (int)ceil(HDK_GAME_GOAL_MIN_PROPORTION * gameInfo.destroyableBlocks));
        }
        else if (m_goals[i].goalType == e_hdkGoalTypeMinBlocksPercentage)
        {
            if (m_goals[i].value == 100)
            {
                snprintf(m_goals[i].description, 256, "Smash ALL blocks");
            }
            else
            {

                snprintf(m_goals[i].description, 256, "Smash at least %d blocks",
                         (int)ceil(((float)m_goals[i].value/100.0f) * gameInfo.destroyableBlocks));
            }
        }

        if (strlen(m_goals[i].description) > 0)
        {
            // Caution: recursive
            snprintf(levelGoals, 256, "%s%s\n", levelGoals, m_goals[i].description);
        }
    }
    m_fontLevelGoals->SetText(levelGoals);


    this->InitLevelInterface();
    this->UpdateGameState();
    this->InitMusic(levelId);

    /*
     * Mist emitter was cool, but slowed the framerate too much, so is not used
     */
#ifdef MIST_EMITTER
    m_mistEmitter = new WeatherParticleEmitter(4, "Materials/p_mist01_15_128.tga", 36.0f, 12.0f, 1.0f, 0.0f, ((Level*)level),
                                                    hdVec3(5.5f, -0.8f, 0),
                                                    hdVec3(8.5f, 0.8f, 0));
    hdAABB aabb, levelAABB;
    levelAABB = ((Level*)level)->GetAABB();
    aabb.lower.Set(levelAABB.lower.x - 18.0f, levelAABB.lower.y, 10.0f);
    aabb.upper.Set(levelAABB.lower.x, levelAABB.upper.y, 0.0f);
    m_mistEmitter->Start(aabb);
#endif

    ((Level *)level)->ResetTextureCache();
    return true;
}


void GameController::InitMusic(int levelId)
{
#ifdef LIGHTVERSION
    switch ((levelId % 3))
    {
        case 0:
            m_currBackgroundTrack = hdSoundManager::FindSound("Music/shahLITE.mp3", e_soundTypeBackground);
            break;
        case 1:
            m_currBackgroundTrack = hdSoundManager::FindSound("Music/up_the_antiLITE.mp3", e_soundTypeBackground);
            break;
        case 2:
            m_currBackgroundTrack = hdSoundManager::FindSound("Music/decisionsLITE.mp3", e_soundTypeBackground);
            break;
        default:
            m_currBackgroundTrack = hdSoundManager::FindSound("Music/shahLITE.mp3", e_soundTypeBackground);
            break;
    }
#else
    switch ((levelId % 6))
    {
        case 0:
            m_currBackgroundTrack = hdSoundManager::FindSound("Music/shah.mp3", e_soundTypeBackground);
            break;
        case 1:
            m_currBackgroundTrack = hdSoundManager::FindSound("Music/ddragon.mp3", e_soundTypeBackground);
            break;
        case 2:
            m_currBackgroundTrack = hdSoundManager::FindSound("Music/scoop_beats.mp3", e_soundTypeBackground);
            break;
        case 3:
            m_currBackgroundTrack = hdSoundManager::FindSound("Music/solid_set.mp3", e_soundTypeBackground);
            break;
        case 4:
            m_currBackgroundTrack = hdSoundManager::FindSound("Music/decisions.mp3", e_soundTypeBackground);
            break;
        case 5:
            m_currBackgroundTrack = hdSoundManager::FindSound("Music/up_the_anti.mp3", e_soundTypeBackground);
            break;
        default:
            m_currBackgroundTrack = hdSoundManager::FindSound("Music/scoop_beats.mp3", e_soundTypeBackground);
            break;
    }
#endif
    if (m_currBackgroundTrack->isBackgroundPlaying == false)
    {
        m_currBackgroundTrack->volume = 1.0f;
        hdSoundManager::PlaySound(m_currBackgroundTrack);
    }
}


void GameController::Game_ResetCurrentLevel()
{
    m_notificationPolygons->RemoveAll();
}


const bool GameController::Game_CanSelectBlock(const Block *block)
{
    return ((!block->IsStatic() && (((Block *)block)->GetBlockType() != e_hdkBlockTypeNormalInv)) ||
            (block->IsStatic() && block->IsDestroyable())) && !(block->ContainsGameFlag((int)e_hdkGameFlagsBlockLost));
}


void GameController::Game_HandleCollision(const ContactPoint* point) {}


void GameController::Game_PlayLoudCollisionSound(const Block* block, const b2Body* body)
{
    float vol = 0.2f + (0.5f * fabs(body->GetLinearVelocity().y) + fabs(body->GetLinearVelocity().x));
    if (vol > 0.5f)
    {
        ++m_heavyCollisionCount;
    }

    if (m_heavyCollisionCount > 5)
    {
        StartCameraShake();
    }
}


void GameController::Game_PlaySoftCollisionSound(const Block* block, const b2Body* body) {}


void GameController::Game_Step(double interval)
{
    m_heavyCollisionCount = 0;

    // Auto evaluate the game state every 30 ticks.
    if (ticksSinceLastUpdate == 20)
    {
        ticksSinceLastUpdate = 0;
        UpdateGameState();
        ShowWinMessage();
    }
    else
    {
        ++ticksSinceLastUpdate;
    }

    UpdateGameInfo();
}


void GameController::Draw()
{
    DrawInternal();

    DrawSelectedBlockHighlight();

    PushProjectionMatrix();
    for(int i = 0; i < m_specialBlocks->GetItemCount(); ++i)
    {
        m_specialBlocks->GetItems()[i]->DrawSpecial();
    }

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    m_gameWorldAnimPolygons->Map(&hdReceiver::Draw);
    glDisable(GL_BLEND);

#ifdef MIST_EMITTER
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    m_mistEmitter->Draw();
    glDisable(GL_BLEND);
#endif

    PopProjectionMatrix();

    hdglBindTexture(NULL);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (IsLandscapeOrientation())
    {
        glRotatef(m_landscapeRotationZValue, 0.0f, 0.0f, 1.0f);
    }
    glPushMatrix();
    hdglOrthof(m_interfaceProjection->GetAABB().lower.x,
               m_interfaceProjection->GetAABB().upper.x,
               m_interfaceProjection->GetAABB().lower.y,
               m_interfaceProjection->GetAABB().upper.y,
               m_interfaceProjection->GetAABB().lower.z,
               m_interfaceProjection->GetAABB().upper.z);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);

    if (((hdGameObject *)m_prefsView)->IsVisible())
    {
        hdglBindTexture(NULL);
        DrawInternalInterface();
    }
    else
    {
        if (m_menuButtonLayer->IsHidden())
        {
            m_gameButtonLayer->Draw();

            DrawInternalInterface();

            hdglBindTexture(NULL);

            hdglBegin(GL_QUADS);
            hdglColor4ub(0, 0, 0, 127);
            hdglVertex2f(m_PixelScreenWidth - 213.0f, m_PixelScreenHeight - 42.0f);
            hdglColor4ub(0, 0, 0, 63);
            hdglVertex2f(m_PixelScreenWidth - 213.0f,	m_PixelScreenHeight - 5.0f);
            hdglColor4ub(0, 0, 0, 63);
            hdglVertex2f(m_PixelScreenWidth - 5.0f,		m_PixelScreenHeight - 5.0f);
            hdglColor4ub(0, 0, 0, 127);
            hdglVertex2f(m_PixelScreenWidth - 5.0f,   m_PixelScreenHeight - 42.0f);
            hdglEnd();
            uiImageContainer->Draw();
        }
        else
        {
            m_menuButtonLayer->Draw();
        }
    }

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    if (m_menuButtonLayer->IsHidden())
    {
        m_animPolygons->Map(&hdReceiver::Draw);
        m_notificationPolygons->Map(&Polygon::Draw);
    }
    glDisable(GL_BLEND);

    EnablePointSprite();
    m_starParticleEmitter->Draw();
    DisablePointSprite();

    hdglBindTexture(NULL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
    if (m_fadeAnim->GetStatus() != e_animationFinished)
    {
        hdglBegin(GL_QUADS);
        hdglColor4f(0.0f, 0.0f, 0.0f, m_fadeAction->GetAlphaObject()->GetAlpha());
        hdglVertex2f(m_interfaceProjection->GetAABB().lower.x, m_interfaceProjection->GetAABB().lower.y);
        hdglVertex2f(m_interfaceProjection->GetAABB().lower.x, m_interfaceProjection->GetAABB().upper.y);
        hdglVertex2f(m_interfaceProjection->GetAABB().upper.x, m_interfaceProjection->GetAABB().upper.y);
        hdglVertex2f(m_interfaceProjection->GetAABB().upper.x, m_interfaceProjection->GetAABB().lower.y);
        hdglEnd();
    }
    glDisable(GL_BLEND);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
}


void GameController::AnimateShow()
{
    m_fadeAction->SetStartAlpha(1.0f);
    m_fadeAction->SetEndAlpha(0.0f);
    m_fadeAnim->StartAnimation();

    this->ShowLobbyButton();

    m_levelStartProjectionAnimation->StartAnimation();
}


const bool GameController::Game_HandleTapUp(float x, float y, int tapCount)
{
    return false;
}


const bool GameController::Game_HandleTapDown(float x, float y, int tapCount)
{
    hdVec2 screen;
    ConvertRawToScreen(screen, x, y);

    // Stop the projection animation if user taps on the screen
    // after enabling interaction.
    if (m_interactionState != e_interactionDisabled)
    {
        if (m_playerReadyProjectionAnimation->GetStatus() == e_animationRunning)
        {
            m_playerReadyProjectionAnimation->StopAnimation(false);
        }
    }
    return false;
}


const bool GameController::Game_HandleSingleTap(float x, float y)
{

    return true;
}


const bool GameController::Game_HandleDoubleTap(float x, float y)
{
    hdVec2 screen;
    ConvertRawToScreen(screen, x, y);

    if (m_gameButtonLayer->IsVisible() && m_fadeAnim->GetStatus() != e_animationRunning)
    {
        if (m_showGoalProgress->MouseDoubleClick(screen.x, screen.y))
        {
            ShowGoalProgressMessage();
            hdSoundManager::PlaySound(m_btnMenuClickUpSoundDeep);
            return true;
        }
        else if (m_btnFinishLevel->MouseDoubleClick(screen.x, screen.y))
        {
            m_btnFinishLevel->Disable();
            m_interactionState = e_interactionDisabled;
            this->FinishedClicked();
            return true;
        }
        else if (m_btnFailRestart->MouseDoubleClick(screen.x, screen.y))
        {
            hdSoundManager::PlaySound(m_btnMenuClickUpSound);
            m_interactionState = e_interactionDisabled;
            this->RestartClicked();
            return true;
        }
        else if (m_btnLobbyReady->MouseDoubleClick(screen.x, screen.y))
        {
            m_levelStartProjectionAnimation->StopAnimation(false);
            m_playerReadyProjectionAnimation->StartAnimation();
            this->HideLobbyButton();
            m_btnLobbyReady->Disable();
            gameInfo.gameStarted = true;
            m_interactionState = e_waitingForInput;

            // Show a message if one there.
            // 0 is the level start tag.
            if (m_levelStats == NULL)
            {
                LoadScriptMessageForTag(0, false);
            }
            else if (m_levelStats->IsActive() && m_levelStats->attemptsCount > 0)
            {
                // If they've played the level, then queue the script messages so people don't
                // have to read it all again.
                LoadScriptMessageForTag(0, true);
            }
            else
            {
                LoadScriptMessageForTag(0, false);
            }

            return true;
        }
    }

    return false;
}


const bool GameController::Game_HandleTapMovedSingle(const float previousX, const float previousY,
                                                     const float currentX, const float currentY)
{
    return false;
}


const bool GameController::Game_HandleTapMovedDouble(const float aPreviousX, const float aPreviousY,
                                                     const float aCurrentX, const float aCurrentY,
                                                     const float bPreviousX, const float bPreviousY,
                                                     const float bCurrentX, const float bCurrentY) { return false; }


void GameController::btnShowMenuUpCallback(void *handler, void *sender)
{
    GameController *self = (GameController *)handler;
    if (!self) return;

    hdSoundManager::PlaySound(self->m_btnMenuClickUpSoundDeep);

    self->m_interactionState = e_interactionDisabled;
    self->m_gameButtonLayer->Hide();
    self->m_menuButtonLayer->Show();

    self->m_btnMenuClose->Enable();
    self->m_btnMenuRestart->Enable();
    self->m_btnMenuQuit->Enable();
    self->m_btnMenuSkip->Enable();
}


void GameController::btnCloseMenuUpCallback(void *handler, void *sender)
{
    GameController *self = (GameController *)handler;
    if (!self) return;

    hdSoundManager::PlaySound(self->m_btnMenuClickUpSoundDeep);

    // if the player has not yet started, then this cannot happen.
    if (self->gameInfo.gameStarted)
    {
        self->m_interactionState = e_waitingForInput;
    }
    self->m_gameButtonLayer->Show();
    self->m_menuButtonLayer->Hide();

    self->DisableMenu();
}


void GameController::btnSettingsUpCallback(void *handler, void *sender)
{
    GameController *self = (GameController *)handler;
    if (!self) return;
    self->ShowPreferences();
}


void GameController::btnMenuRestartCallback(void *handler, void *sender)
{
    GameController *self = (GameController *)handler;
    if (!self) return;
    self->RestartClicked();
}


void GameController::btnMenuQuitCallback(void *handler, void *sender)
{
    GameController *self = (GameController *)handler;
    if (!self) return;
    self->QuitClicked();
}


void GameController::btnMenuSkipCallback(void *handler, void *sender)
{
    GameController *self = (GameController *)handler;
    if (!self) return;
    self->SkipClicked();
}


/*
 * TODO: A collision action depends entirely on what kind of game is loaded
 */
const bool GameController::Game_HandleBlockLoss(Block* block, const hdVec3& contactPoint)
{
    if (!block->IsDestroyable() && !block->IsSpecial()) return false;

    /*
     * If you lose something, reset bonus accumulators.
     */
    gameInfo.midAirSmashesCurrent = 0;
    gameInfo.riskySmashesCurrent = 0;

    if (block->IsDestroyable())
    {
        if (block->GetMaterial() == e_hdkMaterialJack)
        {
            ++gameInfo.jacksLost;
        }
        else if (block->GetMaterial() == e_hdkMaterialGrinder)
        {
            // TODO: grinder counts
        }
        else
        {
            ++gameInfo.lostBlocksCurrent;
            AnimateLostMessage(gameInfo.lostBlocksCurrent);
        }
        hdSoundManager::PlaySound(m_vibrationSound);
    }

    block->AddGameFlag(e_hdkGameFlagsBlockLost);

    AnimateIcon("Interface/Textures/imgBlockLost.png", contactPoint, ICON_POSITION_CENTER, false, 70.0f, 1.75f);

    this->UpdateGameState();
    block->Step();

    hdSoundManager::PlaySound(m_soundIllegalMove);

    return true;
}


void GameController::Game_ShowMessage()
{
    m_playerReadyProjectionAnimation->PauseAnimation();
    m_levelStartProjectionAnimation->PauseAnimation();

    if (m_goalAnim != NULL)	m_goalAnim->PauseAnimation();
    if (m_goalFadeAnim != NULL) m_goalFadeAnim->PauseAnimation();
    if (m_goalTextAnim != NULL) m_goalTextAnim->PauseAnimation();
    if (m_showFinishButtonAnim != NULL) m_showFinishButtonAnim->PauseAnimation();

    m_gameButtonLayer->Hide();
    uiImageContainer->Hide();

    m_interactionState = e_interactionDisabled;
}


void GameController::Game_HideMessage()
{
    m_playerReadyProjectionAnimation->ResumeAnimation();
    m_levelStartProjectionAnimation->ResumeAnimation();

    if (m_goalAnim != NULL)	m_goalAnim->ResumeAnimation();
    if (m_goalFadeAnim != NULL) m_goalFadeAnim->ResumeAnimation();
    if (m_goalTextAnim != NULL) m_goalTextAnim->ResumeAnimation();
    if (m_showFinishButtonAnim != NULL) m_showFinishButtonAnim->ResumeAnimation();

    m_gameButtonLayer->Show();
    uiImageContainer->Show();

    m_interactionState = e_waitingForInput;
}


/*
 * User destroyed a block!
 * - Decrement the block counter
 * - play physics emitter
 * - play volumetric material particle emitter.
 * - remove physics and game object from world.
 * - play a sound
 * - Check the characteristics of the block to see what kind it is.
 *		- Award risky bonus
 *		- Award material bonus.
 */
const bool GameController::Game_HandleBlockTap(Block* block, const hdVec3& impactPoint, bool hazardCollision)
{
    float goalMultiplier = 0.0f;
    int blockPoints = 0;
    int midAirPoints = 0;
    int riskyPoints = 0;
    int totalPoints = 0;
    int materialPoints = 0;

    if (block->GetMaterial() == e_hdkMaterialBomb)
    {
        StartCameraShake();
        hdSoundManager::PlaySound(m_vibrationSound);
        if (!hazardCollision)
            return false;
    }
    else if (block->GetMaterial() == e_hdkMaterialJack)
    {
        if (hazardCollision)
        {
            StartCameraShake();

            AnimateIcon("Interface/Textures/imgBlockLost.png", block->GetWorldCenter(), ICON_POSITION_CENTER, false, 70.0f, 1.75f);
            this->UpdateGameState();
            block->Step();
            hdSoundManager::PlaySound(m_soundIllegalMove);
        }
    }
    else if (block->IsDestroyable() == false)
    {
        if (!hazardCollision)
            hdSoundManager::PlaySound(m_soundIllegalTouch);

        return false;
    }

    /*
     * Grinders, Bombs, Jacks can not be destroyed on tap, but are all destroyable when coming
     * into contact with something.
     *
     * Metal blocks and plastic blocks can not be destroyed if the user directly tapped on the
     * block.
     *
     * In fact, the only material that can be destroyed on a direct tap is the wood block!
     */
    if (hazardCollision == false &&
        (block->GetMaterial() != e_hdkMaterialWood))
    {
        hdSoundManager::PlaySound(m_soundIllegalTouch);
        return false;
    }

    /*
     * Lost blocks can't be smashed.
     */
    if (block->ContainsGameFlag(e_hdkGameFlagsBlockLost))
    {
        hdSoundManager::PlaySound(m_soundIllegalTouch);
        GameController::AnimateLostBlockNotification(block);
        return false;
    }

    /*
     * Check for completed goals and update the multiplier (to be used in the score calculation
     * for the block).
     */
    goalMultiplier = 1.0f;
    for (int i = 0; i < m_goalCount; ++i)
    {
        if (m_goals[i].goalType == e_hdkGoalTypeMultiplier)
        {
            goalMultiplier = ((float)m_goals[i].value) / 100.0f;
        }
    }

    /*
     * Get more points for larger blocks.  Shouldn't it be around the other way...?
     */
    blockPoints = (int)(ceil(block->GetArea() * 10.0f * goalMultiplier) * 10);

    /*
     * Midair bonus check.
     */
    if (IsBlockInMidair(block, m_contacts, m_contactCount))
    {
        hdSoundManager::PlaySound(m_midairSound);
        ++gameInfo.midAirSmashesTotal;
        ++gameInfo.midAirSmashesCurrent;
        midAirPoints = blockPoints * gameInfo.midAirSmashesCurrent;

        /*
         * Midair message
         */
        AnimateIcon("/Interface/Textures/iconBonusMidair.png", impactPoint, ICON_POSITION_RIGHT, true, 70.0f, hdRandom(1.2f, 1.8f));

        /*
         * Multiplier message
         */
        if (gameInfo.midAirSmashesCurrent >= 2)
        {
            AnimateMultiplierMessage(gameInfo.midAirSmashesCurrent);

            ++gameInfo.totalMidAirChains;
            gameInfo.longestMidAirChain = hdMax(gameInfo.longestMidAirChain, gameInfo.midAirSmashesCurrent);

            /*
             * Midair chain message - each chain gets player more points.
             */
            if (gameInfo.midAirSmashesCurrent > 2)
            {
                midAirPoints += (gameInfo.midAirSmashesCurrent * 250);
                AnimateMidairChainMessage(block, gameInfo.midAirSmashesCurrent);
                hdSoundManager::PlaySound(m_midairComboSound);
            }
        }
    }
    else
    {
        gameInfo.midAirSmashesCurrent = 0;
    }

    if (hazardCollision)
    {
        // points for a hazard collision?
    }
    else
    {
        int risk = (GetBlockRiskiness(block, m_contacts, m_contactCount))/2;
        if (risk > 3)
        {
            ++gameInfo.riskySmashesTotal;
            ++gameInfo.riskySmashesCurrent;
            riskyPoints = gameInfo.riskySmashesCurrent * blockPoints;

            AnimateIcon("/Interface/Textures/iconBonusRisky.png", impactPoint, ICON_POSITION_LEFT, false, 70.0f, 1.0f);
            hdSoundManager::PlaySound(m_blockZap01Sound);
        }
        else
        {
            gameInfo.riskySmashesCurrent = 0;
        }
    }

    /*
     * Player smashed a plastic block - block points.
     */
    if (block->GetMaterial() == e_hdkMaterialPlastic)
    {
        materialPoints = blockPoints;
        AnimateIcon("/Interface/Textures/iconBonusPlastic.png", block->GetWorldCenter(), ICON_POSITION_LEFT, true, 70.0f, 1.5f);
    }

    totalPoints = materialPoints + blockPoints + riskyPoints + midAirPoints;
    gameInfo.gamePoints += totalPoints;

    if (block->IsDestroyable())
    {
        if (block->GetMaterial() == e_hdkMaterialJack)
        {
            ++gameInfo.jacksLost;
        }
        else if (block->GetMaterial() == e_hdkMaterialGrinder)
        {
            // TODO: grinder counts
        }
        else
        {
            ++gameInfo.destroyedBlocksCurrent;
        }
    }

    AnimatePointsMessage(block->GetWorldCenter(), totalPoints, hdClamp((float)totalPoints / 750.0f, 0.75f, 1.25f));

    this->AnimatePulse(m_fontProgress, 0.5f);
    this->UpdateGameState();

    return true;
}



const bool GameController::Game_HandleParticleTap(hdPhysicsParticle* particle, const hdVec3& impactPoint, bool hazardCollision)
{
    float area;
    int totalPoints, bonusPoints;

    totalPoints = 25;
    ++gameInfo.fragmentSmashCount;

    if (gameInfo.fragmentSmashCount > 0 &&
        (gameInfo.fragmentSmashCount % 50) == 0)
    {
        bonusPoints = (floor(gameInfo.fragmentSmashCount / 50)) * 1000;
        AnimateIcon("/Interface/Textures/iconBonusFiftyFragments.png", impactPoint, ICON_POSITION_RIGHT, true, 128.0f, 1.5f);
        AnimatePointsMessage(impactPoint, bonusPoints, hdClamp((float)totalPoints / 750.0f, 0.75f, 1.25f));
        hdSoundManager::PlaySound(m_Bonus50FragmentsSound);
        totalPoints += bonusPoints;
    }

    gameInfo.gamePoints += totalPoints;

    AnimateFragmentsMessage(gameInfo.fragmentSmashCount);

    this->UpdateGameState();

    hdPrintf("\tParticle Smashed: area: %1.5f\n", area);

    area = hdPolygonArea(particle->GetVertices(), particle->GetVertexCount());
    return area > 0.4f;
}


/*
 * Block lands in a goal area...
 */
void GameController::Game_HandleGoalCollision(Block *goalBlock, Block *block, const hdVec3& impactPoint)
{
    int points = 0;

    if (block->ContainsGameFlag(e_hdkGameFlagsBlockLost))
    {
        return;
    }

    block->Step();
    block->DestroyPhysicsBody();

    /*
     * "Collected!" icon pops up
     */
    AnimateIcon("Interface/Textures/iconBonusCollected.png", impactPoint, ICON_POSITION_RIGHT, true, 128.0f, 2.0f);

    /*
     * Award points
     */
    points = 1000;
    gameInfo.gamePoints += points;
    AnimatePointsMessage(impactPoint, points, 1.1f);

    /*
     * Play a sound
     */
    hdSoundManager::PlaySound(m_jackCollectionSound);
    hdSoundManager::PlaySound(m_vibrationSound);

    /*
     * Increment star block count
     */
    ++gameInfo.jacksCollected;

    /*
     * Add the lost flag...
     */
    block->AddGameFlag(e_hdkGameFlagsBlockLost);

    UpdateGameState();
}


void GameController::Game_hdkBlockWasSelected(const Block *block) const
{
    e_hdkMaterial material = ((Block *)block)->GetMaterial();
    hdSound *sound = (hdSound *)GetCollisionSoundForMaterial(material);
    sound->volume = 1.0f;
    hdSoundManager::PlaySound(sound);
}


void GameController::UpdateGameState()
{
    char stats[256];
    unsigned i;
    unsigned completed;
    float goalWeight;
    float goalProportionComplete;
    float totalProportionComplete;

    if (gameInfo.goalsCompleted == m_goalCount)
    {
        m_fontStats->SetTextFormatted("POINTS: %d\nALL GOALS COMPLETED",
                                      gameInfo.gamePoints);
    }
    else
    {

        m_fontStats->SetTextFormatted("POINTS: %d\n%s",
                                      gameInfo.gamePoints,
                                      m_goals[currGoalInfoCycle].description);
    }

    completed = 0;
    totalProportionComplete = 0.0f;
    goalWeight = 1.0f / (float)m_goalCount;

    for (i = 0; i < m_goalCount; ++i)
    {
        switch(m_goals[i].goalType)
        {
            case e_hdkGoalTypeNormal:
                goalProportionComplete = UpdateNormalGoal(m_goals[i]);
                break;
            case e_hdkGoalTypeMinBlocksPercentage:
                goalProportionComplete = UpdateMinBlocksPercentageGoal(m_goals[i]);
                break;
            case e_hdkGoalTypeMinPoints:
                goalProportionComplete = UpdateMinPointsGoal(m_goals[i]);
                break;
            case e_hdkGoalTypeFragments:
                goalProportionComplete = UpdateFragmentsGoal(m_goals[i]);
                break;
            case e_hdkGoalTypeJackCollection:
                goalProportionComplete = UpdateJackCollectionGoal(m_goals[i]);
                break;
            case e_hdkGoalTypeAllBlocksWithTag:
                goalProportionComplete = UpdateAllBlocksWithTagGoal(m_goals[i]);
                break;
            case e_hdkGoalTypeMultiplier:
                m_goals[i].state = 1;
                break;
            default:
                break;
        }
        totalProportionComplete += (goalWeight * goalProportionComplete);

        if (m_goals[i].state > GAME_GOAL_UNACHIEVED)
        {
            ++completed;
        }
    }

    if (totalProportionComplete >= HDK_GAME_GOAL_MIN_PROPORTION &&
        m_currentProgressMessage == NULL)
    {
        m_currentProgressMessage = m_winMessage1;
        this->AnimateCompletionNotification(1.0f, m_winMessage1);
        m_starParticleEmitter->Start(m_stargoalbronze,
                                     imgBronzeMedal->GetWorldCenter().x,
                                     imgBronzeMedal->GetWorldCenter().y);
        imgEmptyMedal->Hide();
        uiImageContainer->Add(imgBronzeMedal);
        imgBronzeMedal->Show();
    }

    if (totalProportionComplete >= HDK_GAME_GOAL_MIDDLE_PROPORTION &&
        m_currentProgressMessage == m_winMessage1)
    {
        m_winMessage1->Hide();
        m_currentProgressMessage = m_winMessage2;
        this->AnimateCompletionNotification(1.0f, m_winMessage2);
        m_starParticleEmitter->Start(m_stargoalsilver,
                                     imgSilverMedal->GetWorldCenter().x,
                                     imgSilverMedal->GetWorldCenter().y);
        imgBronzeMedal->Hide();
        uiImageContainer->Add(imgSilverMedal);
        imgSilverMedal->Show();
    }

    if (totalProportionComplete >= 1.0f && m_currentProgressMessage == m_winMessage2)
    {
        m_winMessage2->Hide();
        m_currentProgressMessage = m_winMessage3;
        this->AnimateCompletionNotification(1.0f, m_winMessage3);
        m_starParticleEmitter->Start(m_stargoalgold,
                                     imgGoldMedal->GetWorldCenter().x,
                                     imgGoldMedal->GetWorldCenter().y);
        imgSilverMedal->Hide();
        uiImageContainer->Add(imgGoldMedal);
        imgGoldMedal->Show();
    }

    gameInfo.totalProportionComplete = totalProportionComplete;

    m_fontProgress->SetTextFormatted("%1.0f%%", 100.0f * totalProportionComplete);

    if (m_levelStats != NULL)
    {
        if (gameInfo.gamePoints > m_levelStats->highestScore &&
            m_levelStats->attemptsCount > 0 &&
            gameInfo.newHighScore == false)
        {
            gameInfo.newHighScore = true;
            hdSoundManager::PlaySound(m_levelWin02Sound);
            AnimateGoalMessage("", m_newHighScore, 2.0f);
        }
    }

    gameInfo.goalsCompleted = completed;
    ticksSinceLastUpdate = 0;
}


void GameController::UpdateGameInfo()
{
    if (ticksInfoCycle == 0)
    {
        currGoalInfoCycle = (currGoalInfoCycle + 1) % m_goalCount;
    }

    ticksInfoCycle = (ticksInfoCycle + 1) & 127;
}


void GameController::ShowWinMessage()
{
    /*
     * HACK
     *
     * Completeness determined by goals completed, so the above progress messages are not the right
     * way to award the player.  If player earns bronze but has not completed all the goals, do they
     * keep the bronze medal? Does not seem right if they do.
     *
     * On the other hand, there should be SOME reward for work done. Completeness should not be 
     * determined by proportion completed.
     */
    if (gameInfo.goalsCompleted == m_goalCount &&
        !m_hasShownWinMessage)
    {
        m_hasShownWinMessage = true;
        gameInfo.allGoalsCompleted = true;
        m_gameButtonLayer->Add(m_btnFinishLevel);
        m_btnFinishLevel->Show();
        this->ShowFinishLevelButton();
        
        if (m_levelStats == NULL)
        {
            LoadScriptMessageForTag(SMASHED_GOALS_ACHIEVED_MESSAGE_TAG);
        }
        else
        {
            LoadScriptMessageForTag(SMASHED_GOALS_ACHIEVED_MESSAGE_TAG, 
                                    (m_levelStats->IsCompleted() && (m_levelStats->attemptsCount >= 1)));
        }
    }
}


void GameController::ShowFailMessage(const totemGoal& goal)
{
    if (!gameInfo.failEvent)
    {
        m_gameButtonLayer->Add(m_btnFailRestart);
        m_btnFailRestart->Show();
        this->ShowFailRestartButton();
        gameInfo.failEvent = true;
        hdSoundManager::PlaySound(m_levelFailSound);
        AnimateGoalMessage(goal.description, m_goalFailedMessageBox, 6.0f);
    }
}


void GameController::ShowGoalProgressMessage()
{
    totemMessage msg;
    
    if (!IsShowingMessage())
    {
        msg.messageType = e_hdkMessageTypeAvatar;
        
        snprintf(msg.texture, 256, "Interface/Textures/imgGoalProgress.png");
        snprintf(msg.message, 256, "Overall: %1.0f%%\n\n", 
                 100.0f * gameInfo.totalProportionComplete);
        for (unsigned i = 0; i < m_goalCount; ++i)
        {
            if (strlen(m_goals[i].description) > 0)
            {
                snprintf(msg.message, 256, "%sGoal: \"%s\"\n  %s\n\n", 
                         msg.message, 
                         m_goals[i].description,
                         m_goals[i].progressMessage);
            }
        }
    }
    m_interactionState = e_interactionDisabled;
    SetMessage(&msg);
    ShowMessage();
}


float GameController::UpdateNormalGoal(totemGoal& goal)
{
    /*
     * Goal Setup
     *
     * Goal proportions are: 75/90/100
     */
    const int destroyedBlocksGoal_First = ceil(HDK_GAME_GOAL_MIN_PROPORTION * gameInfo.destroyableBlocks);
    const int destroyedBlocksGoal_Second = ceil(HDK_GAME_GOAL_MIDDLE_PROPORTION * gameInfo.destroyableBlocks);
    
    if (gameInfo.destroyedBlocksCurrent >= destroyedBlocksGoal_First &&
        goal.state == GAME_GOAL_UNACHIEVED)
    {
        goal.state = GAME_GOAL_NORMAL_FIRST;
        hdSoundManager::PlaySound(m_levelWin01Sound);
        AnimateGoalMessage(goal, m_goalCompleteMessageBox);
    }
    
    if (gameInfo.destroyedBlocksCurrent >= destroyedBlocksGoal_Second &&
        goal.state == GAME_GOAL_NORMAL_FIRST)
    {
        goal.state = GAME_GOAL_NORMAL_SECOND;
        hdSoundManager::PlaySound(m_levelWin02Sound);
        AnimateGoalMessage("90% of blocks smashed!", m_specialGoalMessageBox, 2.5f);
    }
    
    if (gameInfo.destroyedBlocksCurrent >= gameInfo.destroyableBlocks &&
        goal.state == GAME_GOAL_NORMAL_SECOND)
    {
        goal.state = GAME_GOAL_NORMAL_THIRD;
        hdSoundManager::PlaySound(m_levelWin03Sound);
        AnimateGoalMessage("All blocks smashed!", m_specialGoalMessageBox, 3.0f);
    }
    
    snprintf(goal.progressMessage, 256, "You have %d/%d blocks", 
             gameInfo.destroyedBlocksCurrent, destroyedBlocksGoal_First);
    if (goal.state != GAME_GOAL_UNACHIEVED)
    {
        snprintf(goal.progressMessage, 256, "%s (Complete)", goal.progressMessage);
    }
    
    if (gameInfo.destroyableBlocks - gameInfo.lostBlocksCurrent < destroyedBlocksGoal_First)
    {
        ShowFailMessage(goal);
    }
    
    return hdClamp((float)gameInfo.destroyedBlocksCurrent / 
                   (float)gameInfo.destroyableBlocks, 0.0f, 1.0f);
}


float GameController::UpdateMinBlocksPercentageGoal(totemGoal& goal)
{
    const int destroyedBlocksMin = ceil(((float)goal.value / (float)100) * gameInfo.destroyableBlocks);
    
    if (gameInfo.destroyedBlocksCurrent >= destroyedBlocksMin &&
        goal.state == GAME_GOAL_UNACHIEVED)
    {
        goal.state = GAME_GOAL_MINBLOCKS_FIRST;
        hdSoundManager::PlaySound(m_levelWin02Sound);
        AnimateGoalMessage(goal, m_goalCompleteMessageBox);
    }
    
    if (destroyedBlocksMin < gameInfo.destroyableBlocks)
    {
        if (gameInfo.destroyedBlocksCurrent >= gameInfo.destroyableBlocks &&
            goal.state == GAME_GOAL_MINBLOCKS_FIRST)
        {
            goal.state = GAME_GOAL_MINBLOCKS_ALL;
            hdSoundManager::PlaySound(m_levelWin03Sound);
            AnimateGoalMessage("All blocks smashed!", m_specialGoalMessageBox, 2.0f);
        }
    }
    
    snprintf(goal.progressMessage, 256, "You have %d/%d blocks", 
             gameInfo.destroyedBlocksCurrent, destroyedBlocksMin);
    if (goal.state != GAME_GOAL_UNACHIEVED)
    {
        snprintf(goal.progressMessage, 256, "%s (Complete)", goal.progressMessage);
    }
    
    // Fail
    if (gameInfo.destroyableBlocks - gameInfo.lostBlocksCurrent < destroyedBlocksMin)
    {
        ShowFailMessage(goal);
    }
    
    return hdClamp((float)gameInfo.destroyedBlocksCurrent / (float)destroyedBlocksMin, 0.0f, 1.0f);
}


float GameController::UpdateMinPointsGoal(totemGoal& goal)
{
    if (gameInfo.gamePoints >= goal.value &&
        goal.state == GAME_GOAL_UNACHIEVED)
    {
        goal.state = GAME_GOAL_MINPOINTS_ALL;
        hdSoundManager::PlaySound(m_levelWin03Sound);
        AnimateGoalMessage(goal, m_goalCompleteMessageBox);
    }
    
    snprintf(goal.progressMessage, 256, "You have %d points", 
             gameInfo.gamePoints);
    if (goal.state != GAME_GOAL_UNACHIEVED)
    {
        snprintf(goal.progressMessage, 256, "%s (Complete)", goal.progressMessage);
    }
    
    // If there are no blocks left, no shards, and points less than value, then fail.
    if (gameInfo.gamePoints < goal.value && 
        (gameInfo.destroyableBlocks - (gameInfo.lostBlocksCurrent + gameInfo.destroyedBlocksCurrent)) == 0 &&
        m_physicsParticleEmitter->GetParticles()->GetItemCount() == 0)
    {
        ShowFailMessage(goal);
    }
    
    return hdClamp((float)gameInfo.gamePoints / (float)goal.value, 0.0f, 1.0f);
}


float GameController::UpdateFragmentsGoal(totemGoal& goal)
{
    if (gameInfo.fragmentSmashCount >= goal.value && 
        goal.state == GAME_GOAL_UNACHIEVED)
    {
        goal.state = GAME_GOAL_FRAGMENTS_ALL;
        hdSoundManager::PlaySound(m_levelWin03Sound);
        AnimateGoalMessage(goal, m_goalCompleteMessageBox);
    }
    
    snprintf(goal.progressMessage, 256, "You have %d/%d fragments", 
             gameInfo.fragmentSmashCount, goal.value);
    if (goal.state != GAME_GOAL_UNACHIEVED)
    {
        snprintf(goal.progressMessage, 256, "%s (Complete)", goal.progressMessage);
    }
    
    // No blocks and no particles means you fail.
    if (gameInfo.fragmentSmashCount < goal.value && 
        (gameInfo.destroyableBlocks - (gameInfo.lostBlocksCurrent + gameInfo.destroyedBlocksCurrent)) == 0 &&
        m_physicsParticleEmitter->GetParticles()->GetItemCount() == 0)
    {
        ShowFailMessage(goal);
    }
    
    return hdClamp((float)gameInfo.fragmentSmashCount / (float)goal.value, 0.0f, 1.0f);
}


float GameController::UpdateJackCollectionGoal(totemGoal& goal)
{
    if (gameInfo.jacksCollected >= goal.value && 
        goal.state == GAME_GOAL_UNACHIEVED)
    {
        goal.state = GAME_GOAL_JACKS_FIRST;
        hdSoundManager::PlaySound(m_levelWin03Sound);
        AnimateGoalMessage(goal, m_goalCompleteMessageBox);
    }
    
    if (gameInfo.jacksCollected >= gameInfo.jacksTotal && 
        goal.state == GAME_GOAL_JACKS_FIRST)
    {
        goal.state = GAME_GOAL_JACKS_ALL;
        hdSoundManager::PlaySound(m_levelWin03Sound);
        AnimateGoalMessage(goal, m_goalCompleteMessageBox);
    }
    
    snprintf(goal.progressMessage, 256, "You've collected %d/%d\n", 
             gameInfo.jacksCollected, goal.value);
    if (goal.state != GAME_GOAL_UNACHIEVED)
    {
        snprintf(goal.progressMessage, 256, "%s (Complete)", goal.progressMessage);
    }
    
    if ((gameInfo.jacksTotal - gameInfo.jacksLost) < goal.value)
    {
        ShowFailMessage(goal);
    }
    
    if (goal.state == GAME_GOAL_UNACHIEVED)
    {
        return hdClamp(HDK_GAME_GOAL_MIDDLE_PROPORTION * ((float)gameInfo.jacksCollected / 
                                                            (float)goal.value), 0.0f, HDK_GAME_GOAL_MIDDLE_PROPORTION);
    }
    else if (goal.state == GAME_GOAL_JACKS_FIRST)
    {
        return hdClamp(((float)gameInfo.jacksCollected / 
                        (float)gameInfo.jacksTotal), HDK_GAME_GOAL_MIDDLE_PROPORTION, 1.0f);
    }
    else
    {
        return 1.0f;
    }
}


/*
 * TODO
 */
float GameController::UpdateAllBlocksWithTagGoal(totemGoal& goal) { return 0.0f; }


void GameController::FinishedClicked()
{
    SkipClicked();
}


void GameController::SkipClicked()
{
    DisableMenu();
    
    hdSoundManager::PlaySound(m_btnMenuClickUpSound);
    if (m_btnFinishLevel->IsHidden())
    {
        gameInfo.failEvent = false;
        gameInfo.allGoalsCompleted = true;
    }
    hdSoundManager::PlaySound(m_levelWin01Sound);
    m_interactionState = e_interactionDisabled;
    
    m_levelStartProjectionAnimation->StopAnimation(false);
    
    hdSoundManager::PlaySound(m_levelFinishButton01Sound);
    hdSoundManager::StopSound(m_currBackgroundTrack);
    
    // anim fade
    m_fadeAction->SetStartAlpha(0.0f);
    m_fadeAction->SetEndAlpha(1.0f);
    m_fadeAnim->SetFinishedCallback(m_parentController, AppCallbackFunctions::StoppedCurrentLevelCallback);
    m_fadeAnim->StartAnimation();
    
    m_interactionState = e_interactionDisabled;
}


void GameController::QuitClicked()
{
    DisableMenu();
    
    hdSoundManager::PlaySound(m_btnMenuClickUpSound);
    m_interactionState = e_interactionDisabled;
    
    m_levelStartProjectionAnimation->StopAnimation(false);
    
    hdSoundManager::StopSound(m_currBackgroundTrack);
    m_fadeAction->SetStartAlpha(0.0f);
    m_fadeAction->SetEndAlpha(1.0f);
    m_fadeAnim->SetFinishedCallback(m_parentController, AppCallbackFunctions::QuitCurrentLevelCallback);
    m_fadeAnim->StartAnimation();
    
    m_interactionState = e_interactionDisabled;
}


void GameController::RestartClicked()
{
    DisableMenu();
    
    hdSoundManager::PlaySound(m_btnMenuClickUpSound);
    m_interactionState = e_interactionDisabled;
    
    m_levelStartProjectionAnimation->StopAnimation(false);
    
    hdSoundManager::PlaySound(m_btnLevelRestartSound);
    m_fadeAction->SetStartAlpha(0.0f);
    m_fadeAction->SetEndAlpha(1.0f);
    m_fadeAnim->SetFinishedCallback(m_parentController, AppCallbackFunctions::RestartCurrentLevelCallback);
    m_fadeAnim->StartAnimation();
    
    m_interactionState = e_interactionDisabled;
}


void GameController::DisableMenu()
{
    m_btnMenuClose->Disable();
    m_btnMenuSkip->Disable();
    m_btnMenuRestart->Disable();
    m_btnMenuQuit->Disable();
}
