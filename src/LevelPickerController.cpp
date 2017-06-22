/*
 * Copyright (c) 2017 Hackdirt Ltd.
 * Author: David Petrie (david@davidpetrie.com)
 *
 * License: https://www.gnu.org/licenses/gpl-3.0.en.html
 */

#include "LevelPickerController.h"

static hdVec3 leftPanelStartingPosition;
static hdVec3 rightPanelStartingPosition;

LevelPickerController::LevelPickerController(const hdInterfaceController* parentController) : AppLevelController(parentController)
{
    m_levelPickerBlocks = new hdTypedefList<LevelPickerBlock*, 128>();

    this->InitInterface();
    this->InitSounds();
    this->InitAnimations();
    this->RefreshLayout();
}


LevelPickerController::~LevelPickerController()
{
    hdSoundManager::StopSound(m_currAmbience);

    hdAnimationController::Instance()->StopAnimations(this);
    hdAnimationController::Instance()->PurgeAnimations(this);

    hdAssert(m_levelPickerBlocks != NULL);

    delete m_levelPickerBlocks;
    m_levelPickerBlocks = NULL;

    /* Misc Buttons and images */
    delete m_btnBack;
    delete m_interfaceProjection;
    delete m_fontStats;
    delete m_interfaceContainer;

    /* Level Info Panels - Landscape only */
    delete m_pnlLeftInfoPanelLandscape;
    delete m_imgLeftPanelChromeLandscape;
    delete m_fontLevelTitleLandscape;
    delete m_imgLevelScreenshotLandscape;
    delete m_pnlRightInfoPanelLandscape;
    delete m_pnlRightButtonPanelLandscape;
    delete m_imgRightPanelChromeLandscape;
    delete m_imgLevelStarLandscape;
    delete m_fontLevelPercentageLandscape;
    delete m_fontLevelStatsLandscape;
    delete m_imgLevelStatsBackgroundLandscape;
    delete m_btnPlayLevelLandscape;
    delete m_btnCancelInfoLandscape;

    /* Level Info Panels - Portrait only */
    delete m_pnlLeftInfoPanelPortrait;
    delete m_imgLeftPanelChromePortrait;
    delete m_fontLevelTitlePortrait;
    delete m_imgLevelScreenshotPortrait;
    delete m_pnlRightInfoPanelPortrait;
    delete m_pnlRightButtonPanelPortrait;
    delete m_imgRightPanelChromePortrait;
    delete m_imgLevelStarPortrait;
    delete m_fontLevelPercentagePortrait;
    delete m_fontLevelStatsPortrait;
    delete m_imgLevelStatsBackgroundPortrait;
    delete m_btnPlayLevelPortrait;
    delete m_btnCancelInfoPortrait;
}


void LevelPickerController::InitInterface()
{
    float longSide;
    float shortSide;
    hdAABB interfaceAABB;

    interfaceAABB.lower = hdVec3(0,0,100.0f);
    interfaceAABB.upper = hdVec3(m_PixelScreenWidth, m_PixelScreenHeight, -100.0f);
    m_interfaceProjection = new hdOrthographicProjection(NULL, interfaceAABB);

    m_interfaceContainer = new hdUIContainer(NULL, interfaceAABB.lower, interfaceAABB.upper);

    m_btnBack = new hdButton("/Interface/Textures/btnBackNormal.png",
                             "/Interface/Textures/btnBackOver.png",
                             "/Interface/Textures/btnBackOver.png", NULL, 5.0f, 440.0f, 50.0f, 35.0f);

    m_fontStats = new hdFontPolygon("Fonts/bebas12.f", NULL);
    m_fontStats->SetTint(1.0f, 1.0f, 1.0f, 1.0f);
    m_fontStats->SetScale(0.8f);
    m_fontStats->AlignRight();

    longSide = hdMax(m_PixelScreenWidth, m_PixelScreenHeight);
    shortSide = hdMin(m_PixelScreenWidth, m_PixelScreenHeight);

    m_interfaceContainer->Add(m_btnBack);
    m_interfaceContainer->Add(m_fontStats);

    m_panMomentumEnabled = true;

    SetupPointSprites();

    // Preload these textures...
    hdTextureManager::Instance()->FindTexture("/Materials/s_stargoalempty_15.tga", TT_Sprite);
    hdTextureManager::Instance()->FindTexture("/Materials/s_stargoalbronze_15.tga", TT_Sprite);
    hdTextureManager::Instance()->FindTexture("/Materials/s_stargoalsilver_15.tga", TT_Sprite);
    hdTextureManager::Instance()->FindTexture("/Materials/s_stargoalgold_15.tga", TT_Sprite);

    /*
     * Landscape Info Panel
     */
#ifdef IPHONE_BUILD
    m_pnlLeftInfoPanelLandscape = new hdUIContainer(NULL, hdVec3(10,10,0),
                                                    hdVec3((longSide * 0.5f)-7.5f, shortSide - 40.0f, 0.0f));
    m_pnlLeftInfoPanelLandscape->SetAs2DBox(10.0f, 10.0f,
                                            (longSide * 0.5f)-15.0f,
                                            shortSide - 50.0f);

    m_imgLeftPanelChromeLandscape = new hdUIImage(NULL, NULL);
    m_imgLeftPanelChromeLandscape->SetAs2DBox(10.0f, 10.0f,
                                              (longSide * 0.5f)-15.0f,
                                              shortSide - 60.0f);
    m_imgLeftPanelChromeLandscape->SetTint(1.0f, 1.0f, 1.0f, 0.85f);

    m_fontLevelTitleLandscape = new hdFontPolygon("Fonts/bebas16.f", NULL);
    m_fontLevelTitleLandscape->SetAs2DBox(30.0f,
                                          (shortSide - 90.0f),
                                          (longSide * 0.5f)-60.0f,
                                          38.0f);
    m_fontLevelTitleLandscape->SetTint(0.0f, 0.0f, 0.0f, 1.0f);
    m_fontLevelTitleLandscape->AlignCenter();

    m_imgLevelScreenshotLandscape = new hdUIImage("", NULL);
    m_imgLevelScreenshotLandscape->SetAs2DBox(20.0f, 20.0f,
                                              (longSide * 0.5f)-35.0f,
                                              (shortSide - 130.0f));

    m_pnlLeftInfoPanelLandscape->Add(m_imgLeftPanelChromeLandscape);
    m_pnlLeftInfoPanelLandscape->Add(m_fontLevelTitleLandscape);
    m_pnlLeftInfoPanelLandscape->Add(m_imgLevelScreenshotLandscape);
    m_pnlLeftInfoPanelLandscape->Hide();

    m_pnlRightInfoPanelLandscape = new hdUIContainer(NULL, hdVec3((longSide * 0.5f)+7.5f,10,0),
                                                     hdVec3((longSide * 0.5f)-7.5f, shortSide - 40.0f, 0.0f));
    m_pnlRightInfoPanelLandscape->SetAs2DBox((longSide * 0.5f)+5.0f, 10.0f,
                                             (longSide * 0.5f)-15.0f, shortSide - 50.0f);

    m_pnlRightButtonPanelLandscape = new hdUIContainer(NULL, hdVec3((longSide * 0.5f)+7.5f,10,0),
                                                       hdVec3((longSide * 0.5f)-7.5f, shortSide - 40.0f, 0.0f));
    m_pnlRightButtonPanelLandscape->SetAs2DBox((longSide * 0.5f)+10.0f, 10.0f,
                                               (longSide * 0.5f)-20.0f, shortSide - 50.0f);

    m_imgRightPanelChromeLandscape = new hdUIImage(NULL, NULL);
    m_imgRightPanelChromeLandscape->SetTint(1.0f, 1.0f, 1.0f, 0.85f);
    m_imgRightPanelChromeLandscape->SetAs2DBox((longSide * 0.5f)+10.0f, 115.0f,
                                               (longSide * 0.5f)-20.0f,
                                               shortSide - 165.0f);

    m_imgLevelStarLandscape = new hdUIImage("/Materials/s_stargoalempty_15.tga", NULL);
    m_imgLevelStarLandscape->SetAs2DBox(longSide - 60.0f, shortSide - 90.0f, 40.0f, 40.0f);

    m_fontLevelPercentageLandscape = new hdFontPolygon("Fonts/bebas16.f", NULL);
    m_fontLevelPercentageLandscape->SetTint(0.0f, 0.0f, 0.0f, 1.0f);
    m_fontLevelPercentageLandscape->AlignLeft();
    m_fontLevelPercentageLandscape->SetAs2DBox((longSide * 0.5f)+24.0f,
                                               (shortSide - 94.0f),
                                               (longSide * 0.5f)-100.0f,
                                               40.0f);

    m_fontLevelStatsLandscape = new hdFontPolygon("Fonts/collegiate16.f", NULL);
    m_fontLevelStatsLandscape->SetScale(0.65f);
    m_fontLevelStatsLandscape->SetTint(0.0f, 0.0f, 0.0f, 1.0f);
    m_fontLevelStatsLandscape->AlignLeft();
    m_fontLevelStatsLandscape->SetAs2DBox((longSide * 0.5f)+30.0f, 125.0f,
                                          (longSide * 0.5f)-60.0f, 104.0f);

    m_imgLevelStatsBackgroundLandscape = new hdUIImage(NULL, NULL);
    m_imgLevelStatsBackgroundLandscape->SetTint(1.0f, 1.0f, 1.0f, 0.85f);
    m_imgLevelStatsBackgroundLandscape->SetAs2DBox((longSide * 0.5f)+20.0f, 125.0f,
                                                   (longSide * 0.5f)-40.0f,
                                                   shortSide - 175.0f - 40.0f);

    m_btnPlayLevelLandscape = new hdButton("Interface/Textures/btnPlayLevelNormal.png",
                                           "Interface/Textures/btnPlayLevelOver.png",
                                           "Interface/Textures/btnPlayLevelOver.png",
                                           NULL);
    m_btnPlayLevelLandscape->SetAs2DBox((longSide * 0.5f)+35.0f, 50.0f,
                                        (longSide * 0.5f)-70.0f, 60.0f);

    m_btnCancelInfoLandscape = new hdButton("Interface/Textures/btnCancelNormal.png",
                                            "Interface/Textures/btnCancelOver.png",
                                            "Interface/Textures/btnCancelOver.png",
                                            NULL);
    m_btnCancelInfoLandscape->SetAs2DBox((longSide * 0.5f)+70.0f, 10.0f,
                                         (longSide * 0.5f)-140.0f, 35.0f);

    m_pnlRightInfoPanelLandscape->Add(m_imgRightPanelChromeLandscape);
    m_pnlRightInfoPanelLandscape->Add(m_imgLevelStatsBackgroundLandscape);
    m_pnlRightInfoPanelLandscape->Add(m_imgLevelStarLandscape);
    m_pnlRightInfoPanelLandscape->Add(m_fontLevelStatsLandscape);
    m_pnlRightInfoPanelLandscape->Add(m_fontLevelPercentageLandscape);
    m_pnlRightInfoPanelLandscape->Add(m_pnlRightButtonPanelLandscape);
    m_pnlRightInfoPanelLandscape->Hide();

    m_pnlRightButtonPanelLandscape->Add(m_btnPlayLevelLandscape);
    m_pnlRightButtonPanelLandscape->Add(m_btnCancelInfoLandscape);
    m_pnlRightButtonPanelLandscape->Hide();

#else
    float distanceFromLeft = 150.0f;
    float distanceFromRight = 150.0f;
    float distanceToMiddle = (m_PixelScreenWidth * 0.5f);

    float leftPanelWidth = distanceToMiddle-distanceFromLeft;
    float leftPanelHeight = 0.55f * m_PixelScreenHeight;
    float leftPanelDistanceFromBottom = 170.0f;

    float rightPanelWidth = distanceToMiddle-distanceFromRight;
    float rightPanelHeight = leftPanelHeight - 140.0f;
    float rightPanelDistanceFromBottom = 310.0f;
    float buttonsDistanceFromBottom = leftPanelDistanceFromBottom;


    m_pnlLeftInfoPanelLandscape = new hdUIContainer(NULL, hdVec3(distanceFromLeft,leftPanelDistanceFromBottom, 0.0f),
                                                    hdVec3(distanceToMiddle-7.5f, m_PixelScreenHeight - leftPanelDistanceFromBottom, 0.0f));
    m_pnlLeftInfoPanelLandscape->SetAs2DBox(distanceFromLeft, leftPanelDistanceFromBottom,
                                            leftPanelWidth, leftPanelHeight);

    m_imgLeftPanelChromeLandscape = new hdUIImage(NULL, NULL);
    m_imgLeftPanelChromeLandscape->SetTint(1.0f, 1.0f, 1.0f, 0.85f);
    m_imgLeftPanelChromeLandscape->SetAs2DBox(distanceFromLeft, leftPanelDistanceFromBottom,
                                              leftPanelWidth, leftPanelHeight);

    m_fontLevelTitleLandscape = new hdFontPolygon(CONFIG_LARGE_FONT, NULL);
    m_fontLevelTitleLandscape->SetTint(0.0f, 0.0f, 0.0f, 1.0f);
    m_fontLevelTitleLandscape->AlignCenter();
    m_fontLevelTitleLandscape->SetAs2DBox(distanceFromLeft + 20.0f, leftPanelDistanceFromBottom + leftPanelHeight - 35.0f,
                                          leftPanelWidth - 20.0f, 30.0f);

    m_imgLevelScreenshotLandscape = new hdUIImage("", NULL);
    m_imgLevelScreenshotLandscape->SetAs2DBox(distanceFromLeft + 20.0f, leftPanelDistanceFromBottom + 20.0f,
                                              leftPanelWidth - 40.0f, leftPanelHeight - 120.0f);

    m_pnlLeftInfoPanelLandscape->Add(m_imgLeftPanelChromeLandscape);
    m_pnlLeftInfoPanelLandscape->Add(m_fontLevelTitleLandscape);
    m_pnlLeftInfoPanelLandscape->Add(m_imgLevelScreenshotLandscape);
    m_pnlLeftInfoPanelLandscape->Hide();

    m_pnlRightInfoPanelLandscape = new hdUIContainer(NULL, hdVec3(distanceToMiddle + 7.5f, leftPanelDistanceFromBottom, 0),
                                                     hdVec3(m_PixelScreenWidth - distanceFromRight, rightPanelDistanceFromBottom + rightPanelHeight, 0.0f));
    m_pnlRightInfoPanelLandscape->SetAs2DBox((m_PixelScreenWidth * 0.5f)+5.0f, leftPanelDistanceFromBottom,
                                             rightPanelWidth, leftPanelHeight);

    m_pnlRightButtonPanelLandscape = new hdUIContainer(NULL, hdVec3(distanceToMiddle+7.5f,10,0),
                                                       hdVec3((m_PixelScreenWidth - distanceFromRight), m_PixelScreenHeight - 40.0f, 0.0f));
    m_pnlRightButtonPanelLandscape->SetAs2DBox(distanceToMiddle + 10.0f, leftPanelDistanceFromBottom,
                                               rightPanelWidth, 120.0f);

    m_imgRightPanelChromeLandscape = new hdUIImage(NULL, NULL);
    m_imgRightPanelChromeLandscape->SetTint(1.0f, 1.0f, 1.0f, 0.85f);
    m_imgRightPanelChromeLandscape->SetAs2DBox(distanceToMiddle+10.0f, rightPanelDistanceFromBottom,
                                               rightPanelWidth, rightPanelHeight);

    m_imgLevelStatsBackgroundLandscape = new hdUIImage(NULL, NULL);
    m_imgLevelStatsBackgroundLandscape->SetTint(1.0f, 1.0f, 1.0f, 0.85f);
    m_imgLevelStatsBackgroundLandscape->SetAs2DBox(distanceToMiddle+30.0f, rightPanelDistanceFromBottom + 20.0f,
                                                   rightPanelWidth - 40.0f, rightPanelHeight - 80.0f);

    m_imgLevelStarLandscape = new hdUIImage("/Materials/s_stargoalempty_15.tga", NULL);
    m_imgLevelStarLandscape->SetAs2DBox(distanceToMiddle + rightPanelWidth - 60.0f, rightPanelDistanceFromBottom + rightPanelHeight - 55.0f,
                                        50.0f, 50.0f);

    m_fontLevelPercentageLandscape = new hdFontPolygon(CONFIG_LARGE_FONT, NULL);
    m_fontLevelPercentageLandscape->SetTint(0.0f, 0.0f, 0.0f, 1.0f);
    m_fontLevelPercentageLandscape->AlignLeft();
    m_fontLevelPercentageLandscape->SetAs2DBox(distanceToMiddle+30.0f, rightPanelDistanceFromBottom + rightPanelHeight - 35.0f,
                                               rightPanelWidth-100.0f, 30.0f);

    m_fontLevelStatsLandscape = new hdFontPolygon(CONFIG_STATS_FONT, NULL);
    m_fontLevelStatsLandscape->SetTint(0.0f, 0.0f, 0.0f, 0.75f);
    m_fontLevelStatsLandscape->AlignLeft();
    m_fontLevelStatsLandscape->SetAs2DBox(distanceToMiddle+50.0f, rightPanelDistanceFromBottom + 38.0f,
                                          rightPanelWidth-60.0f, rightPanelHeight - 100.0f);

    float playLevelButtonWidth = (rightPanelWidth * 0.55f);
    float playLevelButtonHeight = 0.33f * playLevelButtonWidth;
    float playLevelButtonX = 10.0f + distanceToMiddle + (0.5f * (rightPanelWidth - playLevelButtonWidth));

    float cancelButtonWidth = 0.7f * playLevelButtonWidth;
    float cancelButtonHeight = 0.7f * playLevelButtonHeight;
    float cancelButtonX = 10.0f + distanceToMiddle + (0.5f * (rightPanelWidth - cancelButtonWidth));

    m_btnPlayLevelLandscape = new hdButton("Interface/Textures/btnPlayLevelNormal.png",
                                           "Interface/Textures/btnPlayLevelOver.png",
                                           "Interface/Textures/btnPlayLevelOver.png",
                                           NULL);
    m_btnPlayLevelLandscape->SetAs2DBox(playLevelButtonX, buttonsDistanceFromBottom + cancelButtonHeight + 10.0f,
                                        playLevelButtonWidth, playLevelButtonHeight);

    m_btnCancelInfoLandscape = new hdButton("Interface/Textures/btnCancelNormal.png",
                                            "Interface/Textures/btnCancelOver.png",
                                            "Interface/Textures/btnCancelOver.png",
                                            NULL);
    m_btnCancelInfoLandscape->SetAs2DBox(cancelButtonX, buttonsDistanceFromBottom,
                                         cancelButtonWidth, cancelButtonHeight);

    m_pnlRightInfoPanelLandscape->Add(m_imgRightPanelChromeLandscape);
    m_pnlRightInfoPanelLandscape->Add(m_imgLevelStatsBackgroundLandscape);
    m_pnlRightInfoPanelLandscape->Add(m_imgLevelStarLandscape);
    m_pnlRightInfoPanelLandscape->Add(m_fontLevelStatsLandscape);
    m_pnlRightInfoPanelLandscape->Add(m_fontLevelPercentageLandscape);
    m_pnlRightInfoPanelLandscape->Add(m_pnlRightButtonPanelLandscape);
    m_pnlRightInfoPanelLandscape->Hide();

    m_pnlRightButtonPanelLandscape->Add(m_btnPlayLevelLandscape);
    m_pnlRightButtonPanelLandscape->Add(m_btnCancelInfoLandscape);
    m_pnlRightButtonPanelLandscape->Hide();

    leftPanelStartingPosition = m_pnlLeftInfoPanelLandscape->GetWorldCenter();
    rightPanelStartingPosition = m_pnlRightInfoPanelLandscape->GetWorldCenter();
#endif

    /*
     * Portrait Info Panel
     */
    m_pnlLeftInfoPanelPortrait = new hdUIContainer(NULL);
    m_pnlLeftInfoPanelPortrait->SetAs2DBox(10.0f, longSide - (180.0f + 50.0f),
                                           (shortSide - 20.0f), 180.0f);

    m_imgLeftPanelChromePortrait = new hdUIImage(NULL, NULL);
    m_imgLeftPanelChromePortrait->SetTint(1.0f, 1.0f, 1.0f, 0.85f);
    m_imgLeftPanelChromePortrait->SetAs2DBox(10.0f, longSide - (180.0f + 50.0f),
                                             (shortSide - 20.0f), 180.0f);

    m_fontLevelTitlePortrait = new hdFontPolygon("Fonts/bebas16.f", NULL);
    m_fontLevelTitlePortrait->SetTint(0.0f, 0.0f, 0.0f, 0.75f);
    m_fontLevelTitlePortrait->AlignCenter();
    m_fontLevelTitlePortrait->SetAs2DBox(30.0f, (longSide - 90.0f),
                                         shortSide-60.0f, 38.0f);

    m_imgLevelScreenshotPortrait = new hdUIImage("", NULL);
    m_imgLevelScreenshotPortrait->SetAs2DBox(20.0f, longSide - (180.0f + 40.0f),
                                             shortSide-40.0f, 130.0f);

    m_pnlLeftInfoPanelPortrait->Add(m_imgLeftPanelChromePortrait);
    m_pnlLeftInfoPanelPortrait->Add(m_fontLevelTitlePortrait);
    m_pnlLeftInfoPanelPortrait->Add(m_imgLevelScreenshotPortrait);
    m_pnlLeftInfoPanelPortrait->Hide();

    m_pnlRightInfoPanelPortrait = new hdUIContainer(NULL);
    m_pnlRightInfoPanelPortrait->SetAs2DBox(10.0f, 10.0f,
                                            shortSide - 20.0f,
                                            longSide - (180.0f + 50.0f + 20.0f) );

    m_pnlRightButtonPanelPortrait = new hdUIContainer(NULL);
    m_pnlRightButtonPanelPortrait->SetAs2DBox(10.0f, 10.0f,
                                              shortSide - 20.0f, 70.0f);

    m_imgRightPanelChromePortrait = new hdUIImage(NULL, NULL);
    m_imgRightPanelChromePortrait->SetTint(1.0f, 1.0f, 1.0f, 0.85f);
    m_imgRightPanelChromePortrait->SetAs2DBox(10.0f, 100.0f,
                                              shortSide - 20.0f,
                                              longSide - (180.0f + 50.0f + 110.0f));

    m_imgLevelStarPortrait = new hdUIImage("/Materials/s_stargoalempty_15.tga", NULL);
    m_imgLevelStarPortrait->SetAs2DBox(shortSide - 60.0f,
                                       longSide - (180.0f + 50.0f + 60.0f),
                                       40.0f, 40.0f);

    // TODO: Fixme m_fontLevelPercentagePortrait
    m_fontLevelPercentagePortrait = new hdFontPolygon("Fonts/bebas16.f", NULL);
    m_fontLevelPercentagePortrait->SetAs2DBox((longSide * 0.5f)+30.0f,
                                              (shortSide - 90.0f),
                                              (longSide * 0.5f)-100.0f,
                                              40.0f);

    m_fontLevelStatsPortrait = new hdFontPolygon("Fonts/collegiate8.f", NULL);
    m_fontLevelStatsPortrait->SetTint(0.0f, 0.0f, 0.0f, 1.0f);
    m_fontLevelStatsPortrait->AlignLeft();
    m_fontLevelStatsPortrait->SetAs2DBox(30.0f, 120.0f,
                                         shortSide - 80.0f,
                                         longSide - (180.0f + 50.0f + 140.0f));

    m_imgLevelStatsBackgroundPortrait = new hdUIImage(NULL, NULL);
    m_imgLevelStatsBackgroundPortrait->SetTint(1.0f, 1.0f, 1.0f, 1.0f);
    m_imgLevelStatsBackgroundPortrait->SetAs2DBox(20.0f, 110.0f,
                                                  shortSide - 40.0f,
                                                  longSide - (180.0f + 50.0f + 130.0f));

    m_btnPlayLevelPortrait = new hdButton("Interface/Textures/btnPlayLevelNormal.tga",
                                          "Interface/Textures/btnPlayLevelOver.tga",
                                          "Interface/Textures/btnPlayLevelOver.tga",
                                          NULL);
    m_btnPlayLevelPortrait->SetAs2DBox((shortSide * 0.5f)-80.0f, 40.0f,
                                       160.0f, 60.0f);

    m_btnCancelInfoPortrait = new hdButton("Interface/Textures/btnCancelNormal.tga",
                                           "Interface/Textures/btnCancelOver.tga",
                                           "Interface/Textures/btnCancelOver.tga",
                                           NULL);
    m_btnCancelInfoPortrait->SetAs2DBox((shortSide * 0.5f)-40.0f, 10.0f,
                                        80.0f, 30.0f);

    m_pnlRightInfoPanelPortrait->Add(m_imgRightPanelChromePortrait);
    m_pnlRightInfoPanelPortrait->Add(m_imgLevelStatsBackgroundPortrait);
    m_pnlRightInfoPanelPortrait->Add(m_imgLevelStarPortrait);
    m_pnlRightInfoPanelPortrait->Add(m_fontLevelStatsPortrait);
    m_pnlRightInfoPanelPortrait->Add(m_pnlRightButtonPanelPortrait);
    m_pnlRightInfoPanelPortrait->Add(m_fontLevelPercentagePortrait);
    m_pnlRightInfoPanelPortrait->Hide();

    m_pnlRightButtonPanelPortrait->Add(m_btnPlayLevelPortrait);
    m_pnlRightButtonPanelPortrait->Add(m_btnCancelInfoPortrait);
    m_pnlRightButtonPanelPortrait->Hide();

    /*
     * Callbacks
     */
    m_btnPlayLevelPortrait->SetMouseDownListener((AppInterface *)this, AppInterface::GenericButtonDownCallback);
    m_btnCancelInfoPortrait->SetMouseDownListener((AppInterface *)this, AppInterface::GenericButtonDownCallback);
    m_btnPlayLevelLandscape->SetMouseDownListener((AppInterface *)this, AppInterface::GenericButtonDownCallback);
    m_btnCancelInfoLandscape->SetMouseDownListener((AppInterface *)this, AppInterface::GenericButtonDownCallback);
    m_btnBack->SetMouseDownListener((AppInterface *)this, AppInterface::GenericButtonDownCallback);

    m_btnPlayLevelPortrait->SetMouseUpListener(this, LevelPickerController::btnPlayUpCallback);
    m_btnCancelInfoPortrait->SetMouseUpListener(this, LevelPickerController::btnCancelUpCallback);

    m_btnPlayLevelLandscape->SetMouseUpListener(this, LevelPickerController::btnPlayUpCallback);
    m_btnCancelInfoLandscape->SetMouseUpListener(this, LevelPickerController::btnCancelUpCallback);

    m_btnBack->SetMouseUpListener(this, LevelPickerController::btnBackUpCallback);

    m_btnPlayLevelPortrait->Disable();
    m_btnCancelInfoPortrait->Disable();
    m_btnPlayLevelLandscape->Disable();
    m_btnCancelInfoLandscape->Disable();
}


void LevelPickerController::InitSounds()
{
    m_btnMenuClickDownSound = hdSoundManager::FindSound("Sounds/btnClickDown.caf", e_soundTypeNormal);
    m_btnMenuClickUpSound = hdSoundManager::FindSound("Sounds/btnClick01.caf", e_soundTypeNormal);
    m_sound_btnClickBack01 = hdSoundManager::FindSound("Sounds/btnClickBack01.caf", e_soundTypeNormal);
    m_soundNextWorldUnlocked = hdSoundManager::FindSound("Sounds/levelWin_01.caf", e_soundTypeNormal);
}


void LevelPickerController::InitAnimations()
{
    m_fadeAction = new hdAlphaAction();
    m_fadeAction->SetDuration(0.5f);
    m_fadeAction->SetStartAlpha(1.0f);
    m_fadeAction->SetEndAlpha(0.0f);

    m_fadeAnim = hdAnimationController::CreateAnimation(this, false);
    m_fadeAnim->AddGameObject(m_fadeAction->GetAlphaObject());
    m_fadeAnim->AddAction(m_fadeAction);

    m_lobbyZoomProjectionAction = new hdFunctionAction<LevelPickerController>();
    m_lobbyZoomProjectionAction->SetDuration(1.5f);
    m_lobbyZoomProjectionAction->SetFunctionObject(this);
    m_lobbyZoomProjectionAction->SetFunction(&LevelPickerController::IntroZoomProjection);

    m_lobbyPanProjectionAction = new hdFunctionAction<LevelPickerController>();
    m_lobbyPanProjectionAction->SetDuration(100.0f);
    m_lobbyPanProjectionAction->SetFunctionObject(this);
    m_lobbyPanProjectionAction->SetFunction(&LevelPickerController::IntroPanProjection);

    m_lobbyProjectionAnimation = hdAnimationController::CreateAnimation(this, false);
    m_lobbyProjectionAnimation->AddGameObject(m_projection);
    m_lobbyProjectionAnimation->AddAction(m_lobbyPanProjectionAction);
    m_lobbyProjectionAnimation->AddAction(m_lobbyZoomProjectionAction);
}


static hdVec3 direction(1.0f, 0.1f, 0.0f);
void LevelPickerController::IntroPanProjection(hdTimeInterval interval)
{
    // If the projection aabb is at a boundary, reverse direction.
    if (m_projection->GetAABB().lower.y == m_worldAABB.lower.y || m_projection->GetAABB().upper.y == m_worldAABB.upper.y)
    {
        direction.Set(direction.x, -direction.y, direction.z);
    }
    else if (m_projection->GetAABB().lower.x == m_worldAABB.lower.x || m_projection->GetAABB().upper.x == m_worldAABB.upper.x)
    {
        direction.Set(-direction.x, direction.y, direction.z);
    }

    PanProjection(0.0f, 0.0f, direction.x, direction.y);
}


void LevelPickerController::RefreshLayout()
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

    m_btnBack->SetAs2DBox(5.0f, m_PixelScreenHeight - 40.0f, 50.0f, 35.0f);
    m_fontStats->SetAs2DBox(m_PixelScreenWidth - 162, m_PixelScreenHeight - 40.0f, 150, 35);
}


/*
 * just zoom out
 */
void LevelPickerController::IntroZoomProjection(hdTimeInterval interval)
{
    ZoomProjection(1.0f, 1.0f,
                   0.0f, 0.0f,
                   -1.0f, -1.0f,
                   0.0f, 0.0f);
}


/*
 * HACK HACK HACK FIXME
 *
 * A lot of disparate, disjoint function calls here.
 */
void LevelPickerController::SetWorld(World *tw, const int worldTag)
{
    char tmp[256];
    LevelPickerBlock *levelPickerBlock;
    TotemWorldStats* stats;
    TotemLevelStats *levelStats;
    Level* level;

    // Load all the level picker blocks from the current world.
    stats = TowerPlayerStateController::Instance()->GetStatsForWorld(tw, e_hdkDifficultyEasy);
    hdAssert(stats != NULL);

    m_completedLevels = 0;
    m_percentComplete = 0.0f;
    m_lastCompleteLevelStats = NULL;

    for (int i = 0; i < stats->GetLevels()->GetItemCount(); i++)
    {
        levelStats = stats->GetLevels()->GetItems()[i];
        if (levelStats->IsActive())
        {
            level = tw->GetLevels()[levelStats->levelNumber];

            if (level == NULL)
            {
                break;
            }

            if (levelStats->worldCenter == hdVec3(0,0,0))
            {
                levelStats->worldCenter.Set(hdRandom(-4.0f, 4.0f), 0, 0);
            }

            levelPickerBlock = new LevelPickerBlock(m_gameWorld, GetPhysicsWorld(),
                                                         levelStats, level,
                                                         levelStats->worldCenter,
                                                         hdClamp(0.8f + (level->GetBlockCount() / 120.0f), 0.8f, 1.5f),
                                                         m_projection);
            UtilGenerateColor((float*)levelPickerBlock->GetTint(), levelStats->levelNumber, stats->GetLevels()->GetItemCount());

            tw->GetLevels()[0]->AddBlock(levelPickerBlock);

            m_levelPickerBlocks->Add(levelPickerBlock);

            // Lazy way to make sure selected block is last one.
            SetSelectedTotemBlock(levelPickerBlock);

            if (levelStats->IsCompleted())
            {
                m_completedLevels++;
                m_lastCompleteLevelStats = levelStats;
            }

            m_percentComplete += levelStats->highestCompletionPercentage;
        }
    }

    snprintf(tmp, 256, "%2.0f%% Complete\n%d/%d Levels Finished",
             m_percentComplete / (float)(stats->GetLevels()->GetItemCount()),
             m_completedLevels,
             stats->GetLevels()->GetItemCount());
    m_fontStats->SetText(tmp);

    if (false == stats->isNextLocked())
    {
        // Add a star
        //float halfWidth;
        //hdTexture *goalTexture;
        //float overallPercentage = percentComplete / (float)(stats->GetLevels()->GetItemCount());

        /*
         if (overallPercentage >= 75.0f)
         {
         halfWidth = 0.75f;
         Block *bronzeBlock = new Block(m_gameWorld, (b2World *)GetPhysicsWorld(), hdVec3(-3.0f, 3.0f, 0.0f),
         halfWidth, halfWidth, e_hdkMaterialCustomTexture,
         e_hdkShapeTypeCylinder, e_hdkBlockTypeDraggable);
         bronzeBlock->DestroyPhysicsBody();
         bronzeBlock->SetDepth(-0.3f);
         bronzeBlock->SetTextureName(hdTextureManager::Instance()->FindTexture("/Textures/c_bronze_14_128.tga", TT_16Wall)->name);
         tw->GetLevels()[0]->AddBlock(bronzeBlock);
         SetSelectedTotemBlock(bronzeBlock);
         }
         if (overallPercentage >= 90.0f)
         {
         halfWidth = 1.0f;
         Block *silverBlock = new Block(m_gameWorld, (b2World *)GetPhysicsWorld(), hdVec3(4.0f, 2.5f, 0.0f),
         halfWidth, halfWidth, e_hdkMaterialCustomTexture,
         e_hdkShapeTypeCylinder, e_hdkBlockTypeDraggable);
         silverBlock->DestroyPhysicsBody();
         silverBlock->SetDepth(-0.3f);
         silverBlock->SetTextureName(hdTextureManager::Instance()->FindTexture("/Textures/c_silver_14_128.tga", TT_16Wall)->name);
         tw->GetLevels()[0]->AddBlock(silverBlock);
         SetSelectedTotemBlock(silverBlock);
         }
         if (overallPercentage >= 100.0f)
         {
         halfWidth = 1.25f;
         Block *goldBlock = new Block(m_gameWorld, (b2World *)GetPhysicsWorld(), hdVec3(1.0f, 2.0f, 0.0f),
         halfWidth, halfWidth, e_hdkMaterialCustomTexture,
         e_hdkShapeTypeCylinder, e_hdkBlockTypeDraggable);
         goldBlock->DestroyPhysicsBody();
         goldBlock->SetDepth(-0.3f);
         goldBlock->SetTextureName(hdTextureManager::Instance()->FindTexture("/Textures/c_gold_14_128.tga", TT_16Wall)->name);
         tw->GetLevels()[0]->AddBlock(goldBlock);
         SetSelectedTotemBlock(goldBlock);
         }
         */
    }

    InitAmbience(hdMax(0, worldTag-1));
}



void LevelPickerController::InitAmbience(const int worldTag)
{
#ifdef LIGHTVERSION
    switch ((worldTag % 3))
    {
        case 0:
            m_currAmbience = hdSoundManager::FindSound("Music/ddd.mp3", e_soundTypeBackgroundLooping);
            break;
        case 1:
            m_currAmbience = hdSoundManager::FindSound("Music/night.mp3", e_soundTypeBackgroundLooping);
            break;
        default:
            m_currAmbience = hdSoundManager::FindSound("Music/ddd.mp3", e_soundTypeBackgroundLooping);
            break;
    }
#else
    switch ((worldTag % 8))
    {
        case 0:
            m_currAmbience = hdSoundManager::FindSound("Music/ddd.mp3", e_soundTypeBackgroundLooping);
            break;
        case 1:
            m_currAmbience = hdSoundManager::FindSound("Music/home.mp3", e_soundTypeBackgroundLooping);
            break;
        case 2:
            m_currAmbience = hdSoundManager::FindSound("Music/forest.mp3", e_soundTypeBackgroundLooping);
            break;
        case 3:
            m_currAmbience = hdSoundManager::FindSound("Music/wind.mp3", e_soundTypeBackgroundLooping);
            break;
        case 4:
            m_currAmbience = hdSoundManager::FindSound("Music/park.mp3", e_soundTypeBackgroundLooping);
            break;
        case 5:
            m_currAmbience = hdSoundManager::FindSound("Music/night.mp3", e_soundTypeBackgroundLooping);
            break;
        case 6:
            m_currAmbience = hdSoundManager::FindSound("Music/factory.mp3", e_soundTypeBackgroundLooping);
            break;
        default:
            m_currAmbience = hdSoundManager::FindSound("Music/factory.mp3", e_soundTypeBackgroundLooping);
            break;
    }
#endif

    if (m_currAmbience->isBackgroundPlaying == false)
    {
        m_currAmbience->volume = 0.75f;
        hdSoundManager::PlaySound(m_currAmbience);
    }
}


void LevelPickerController::ShowWorldTitleMessage(const int worldTag)
{
    char key[64];
    totemMessage titleMessage;

    titleMessage.messageType = e_hdkMessageTypeTitle;

    snprintf(key, 64, "Chapter%d_Title", worldTag);

    snprintf(titleMessage.message, 256, "%s %d:\n\"%s\"",
             Scripts_GetStringForKey("Chapter").c_str(),
             worldTag + 1,
             Scripts_GetStringForKey(key).c_str());

    SetMessage(&titleMessage);
    ShowMessage();

    // Show starting message ONCE.
    if (m_percentComplete == 0 && m_completedLevels == 0)
    {
        LoadScriptMessageForTagAndLevel(0, 0, false, false);
    }
    else
    {
        LoadScriptMessageForTag(0, true);
    }
}


void LevelPickerController::ShowNewLevelMessage(const TotemLevelStats* stats)
{
    if (stats == NULL)
    {
        LoadScriptMessageForTagAndLevel(LEVELPICKER_NEW_LEVEL_MESSAGE_TAG, -1, true);
    }
    else
    {
        // Load the introduction message.
        // Put it on the queue if:
        // - the next level has been completed
        LoadScriptMessageForTagAndLevel(LEVELPICKER_NEW_LEVEL_MESSAGE_TAG,
                                        stats->levelNumber,
                                        stats->IsCompleted() &&
                                        stats->completionCount > 1 &&
                                        stats->attemptsCount > 1);
    }
}


void LevelPickerController::ShowNextWorldUnlockedMessage(const int worldId)
{
    totemMessage msg;

    msg.messageType = e_hdkMessageTypeAvatar;

    snprintf(msg.message, 256, "%s%s %d%s%s",
             Scripts_GetStringForKey("Menu_ChapterUnlocked1").c_str(),
             Scripts_GetStringForKey("Chapter").c_str(),
             worldId,
             Scripts_GetStringForKey("Menu_ChapterUnlocked2").c_str(),
             Scripts_GetStringForKey("Menu_ChapterUnlocked3").c_str());

    snprintf(msg.texture, 256, "%s", Scripts_GetStringForKey("Menu_ChapterUnlockedImage").c_str());

    SetMessage(&msg);
    ShowMessage();

    hdSoundManager::PlaySound(m_soundNextWorldUnlocked);
}


static bool playedSoftCollisionSound = false;
void LevelPickerController::Game_Step(double interval)
{
    playedSoftCollisionSound = false;
}


void LevelPickerController::Game_HandleCollision(const ContactPoint* point)
{
    b2Body* b1 = point->body1;
    b2Body* b2 = point->body2;

    if (point->state == e_contactAdded && b1->GetUserData() != NULL && b2->GetUserData() != NULL)
    {
        hdPolygon* obj1 = (hdPolygon *)(b1->GetUserData());
        hdPolygon* obj2 = (hdPolygon *)(b2->GetUserData());

        if (obj1->GetUserType() == (int)e_hdkTypeLevelPickerBlock && obj2->GetUserType() == (int)e_hdkTypeBlock)
        {
            PlaySoftCollisionSound((Block *)obj2, b1);
        }
        else if (obj1->GetUserType() == (int)e_hdkTypeBlock && obj2->GetUserType() == (int)e_hdkTypeLevelPickerBlock)
        {
            PlaySoftCollisionSound((Block *)obj1, b2);
        }
    }
}


void LevelPickerController::PlaySoftCollisionSound(const Block* block, const b2Body* body)
{
    hdSound *sound;
    if (playedSoftCollisionSound == false)
    {
        if (NULL == (sound = (hdSound *)GetCollisionSoundForMaterial(e_hdkMaterialCustomTexture)))
        {
            return;
        }

        float vol = 0.1f + (0.15f * (fabs(body->GetLinearVelocity().y) + fabs(body->GetLinearVelocity().x)));

        vol = hdClamp(vol, 0.2f, 1.0f);

        if (vol > 0.15f)
        {
            sound->volume = vol;
            sound->pitchOffset = hdRandom(0.85f, 1.1f);
            hdSoundManager::PlaySound(sound);
            playedSoftCollisionSound = true;
        }
    }
}


void LevelPickerController::ShowLobbyAnimation()
{
    m_interactionState = e_interactionDisabled;
    m_lobbyProjectionAnimation->StartAnimation();
    ShowMessage();
}


void LevelPickerController::PanToNewLevelBlock()
{
    m_interactionState = e_waitingForInput;

    if (m_lobbyProjectionAnimation->GetStatus() == e_animationRunning)
    {
        m_lobbyProjectionAnimation->StopAnimation(false, true);
    }

    hdVec3 lo = m_projection->GetWorldCenter() - hdVec3(m_GameMaxScreenWidth/2.0f, m_GameMaxScreenHeight/2.0f, 0.0f);
    hdVec3 hi = m_projection->GetWorldCenter() + hdVec3(m_GameMaxScreenWidth/2.0f, m_GameMaxScreenHeight/2.0f, 0.0f);

    m_projection->SetAABB(lo, hi);
}


void LevelPickerController::Draw()
{
    DrawInternal();
    PushProjectionMatrix();

    for(int i = 0; i < m_levelPickerBlocks->GetItemCount(); ++i)
    {
        m_levelPickerBlocks->GetItems()[i]->DrawSpecial();
    }

    PopProjectionMatrix();
    DrawSelectedBlockHighlight();

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
    DrawInternalInterface();
    m_interfaceContainer->Draw();

    if (IsLandscapeOrientation())
    {
        m_pnlLeftInfoPanelLandscape->Draw();
        m_pnlRightInfoPanelLandscape->Draw();
    }
    else
    {
        m_pnlLeftInfoPanelPortrait->Draw();
        m_pnlRightInfoPanelPortrait->Draw();
    }

    glDisable(GL_TEXTURE_2D);

    glDisable(GL_BLEND);

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


void LevelPickerController::AnimateShow()
{
    m_fadeAction->SetStartAlpha(1.0f);
    m_fadeAction->SetEndAlpha(0.0f);
    m_fadeAnim->StartAnimation();

}


const bool LevelPickerController::Game_CanSelectBlock(const Block *block)
{
    return (!block->IsStatic()) &&
    m_pnlLeftInfoPanelLandscape->IsHidden() &&
    m_pnlRightInfoPanelLandscape->IsHidden() &&
    m_pnlRightButtonPanelLandscape->IsHidden() &&
    m_pnlLeftInfoPanelPortrait->IsHidden() &&
    m_pnlRightInfoPanelPortrait->IsHidden() &&
    m_pnlRightButtonPanelPortrait->IsHidden();
}


void LevelPickerController::btnCancelUpCallback(void *handler, void *sender)
{
    LevelPickerController *self = (LevelPickerController *)handler;
    if (!self) return;

    hdSoundManager::PlaySound(self->m_btnMenuClickUpSound);
    self->HideLevelInfoInterface();
}


void LevelPickerController::btnPlayUpCallback(void *handler, void *sender)
{
    LevelPickerController *self = (LevelPickerController *)handler;
    if (!self) return;

    hdSoundManager::PlaySound(self->m_btnMenuClickUpSound);

    self->m_fadeAction->SetStartAlpha(0.0f);
    self->m_fadeAction->SetEndAlpha(1.0f);

    self->m_fadeAnim->SetFinishedCallback(self->m_parentController, AppCallbackFunctions::LevelPickerLevelClickedCallback);
    self->m_fadeAnim->StartAnimation();
}


void LevelPickerController::btnBackUpCallback(void *handler, void *sender)
{
    LevelPickerController *self = (LevelPickerController *)handler;
    if (!self) return;
    self->BackToMenuClicked();
}


const bool LevelPickerController::Game_HandleTapUp(float x, float y, int tapCount)
{
    return false;
}


const bool LevelPickerController::Game_HandleTapDown(float x, float y, int tapCount)
{
    return false;
}


const bool LevelPickerController::Game_HandleTapMovedSingle(const float previousX, const float previousY,
                                                            const float currentX, const float currentY)
{
    return false;
}


void LevelPickerController::Game_HideMessage()
{
    if (m_lobbyProjectionAnimation->GetStatus() == e_animationRunning)
    {
        m_lobbyProjectionAnimation->StopAnimation(false, true);
    }

    m_interactionState = e_waitingForInput;
}


const bool LevelPickerController::Game_HandleBlockTap(Block* block, const hdVec3& impactPoint, bool hazardCollision)
{
    char tmp[256];
    char levelImageName[64];
    TotemLevelStats *levelStats;

    if (m_interactionState == e_interactionDisabled) return false;

    // HACK HACK HACK - check anim state to ensure tap can't happen twice
    if (block->GetTag() == (int)e_hdkTypeLevelPickerBlock &&
        m_fadeAnim->GetStatus() != e_animationRunning)
    {
        hdSoundManager::PlaySound(m_btnMenuClickUpSound);

        m_selectedLevel = ((LevelPickerBlock *)block)->m_level;
        m_selectedLevelId = ((LevelPickerBlock *)block)->m_levelId;
        levelStats = ((LevelPickerBlock *)block)->m_levelStats;

        m_fontLevelTitleLandscape->SetTextFormatted("%s %d:\n %s",
                                                    Scripts_GetStringForKey("Level").c_str(),
                                                    m_selectedLevelId,
                                                    Scripts_GetStringForKey(m_selectedLevel->GetLevelName()).c_str());


        m_fontLevelTitlePortrait->SetTextFormatted("%s %d:\n %s",
                                                   Scripts_GetStringForKey("Level").c_str(),
                                                   m_selectedLevelId,
                                                   Scripts_GetStringForKey(m_selectedLevel->GetLevelName()).c_str());

        snprintf(levelImageName, 64, "Levels/%s.png", m_selectedLevel->GetLevelName());

        // Get level stats
        if (levelStats == NULL || (0 == levelStats->attemptsCount))
        {
            m_imgLevelStarLandscape->SetTexture(hdTextureManager::Instance()->FindTexture("/Materials/s_stargoalempty_15.tga", TT_Sprite));
            m_imgLevelStarPortrait->SetTexture(hdTextureManager::Instance()->FindTexture("/Materials/s_stargoalempty_15.tga", TT_Sprite));
        }
        else
        {
            if (levelStats->highestCompletionPercentage >= 100 * HDK_GAME_GOAL_MIN_PROPORTION)
            {
                m_imgLevelStarLandscape->SetTexture(hdTextureManager::Instance()->FindTexture("/Materials/s_stargoalbronze_15.tga", TT_Sprite));
                m_imgLevelStarPortrait->SetTexture(hdTextureManager::Instance()->FindTexture("/Materials/s_stargoalbronze_15.tga", TT_Sprite));
            }
            if (levelStats->highestCompletionPercentage >= 100 * HDK_GAME_GOAL_MIDDLE_PROPORTION)
            {
                m_imgLevelStarLandscape->SetTexture(hdTextureManager::Instance()->FindTexture("/Materials/s_stargoalsilver_15.tga", TT_Sprite));
                m_imgLevelStarPortrait->SetTexture(hdTextureManager::Instance()->FindTexture("/Materials/s_stargoalsilver_15.tga", TT_Sprite));
            }
            if (levelStats->highestCompletionPercentage >= 100 * HDK_GAME_GOAL_MAX_PROPORTION)
            {
                m_imgLevelStarLandscape->SetTexture(hdTextureManager::Instance()->FindTexture("/Materials/s_stargoalgold_15.tga", TT_Sprite));
                m_imgLevelStarPortrait->SetTexture(hdTextureManager::Instance()->FindTexture("/Materials/s_stargoalgold_15.tga", TT_Sprite));
            }
        }


        snprintf(tmp, 256, "Plays: %d\nHighest Score: %d\nAverage Score: %d\nTotal Blocks Smashed: %d\nBest Midair Combo: %d\nBest Risky Combo: %d\n",
                 levelStats->attemptsCount,
                 levelStats->highestScore,
                 levelStats->totalPoints / hdMax(1, levelStats->attemptsCount),
                 levelStats->totalSmashedBlocks,
                 levelStats->longestMidairChain,
                 levelStats->longestRiskyChain);

        m_fontLevelPercentageLandscape->SetTextFormatted("%d%% COMPLETE", levelStats->highestCompletionPercentage);
        m_fontLevelPercentagePortrait->SetTextFormatted("%d%% COMPLETE", levelStats->highestCompletionPercentage);

        m_fontLevelStatsLandscape->SetText(tmp);
        m_fontLevelStatsPortrait->SetText(tmp);

        m_imgLevelScreenshotLandscape->SetTexture(hdTextureManager::Instance()->FindTexture(levelImageName, TT_Sprite));
        m_imgLevelScreenshotPortrait->SetTexture(hdTextureManager::Instance()->FindTexture(levelImageName, TT_Sprite));

        ShowLevelInfoInterface();

        m_interactionState = e_interactionDisabled;
    }
    return false;
}



void LevelPickerController::ShowLevelInfoInterface()
{
#ifdef IPHONE_BUILD
    hdVec3 leftCenterLandscape, rightCenterLandscape;
    hdVec3 leftCenterPortrait, rightCenterPortrait;

    hdAABB leftLandscapeAABB, rightLandscapeAABB;
    hdAABB leftPortraitAABB, rightPortraitAABB;


    hdAnimation *leftSlideAnimLandscape, *rightSlideAnimLandscape;
    hdVectorAction *leftVectorActionLandscape, *rightVectorActionLandscape;

    hdAnimation *leftSlideAnimPortrait, *rightSlideAnimPortrait;
    hdVectorAction *leftVectorActionPortrait, *rightVectorActionPortrait;

    hdPauseAction  *pauseActionLandscape, *pauseActionPortrait;
    hdAnimation *pauseCallbackAnim;
    hdPauseAction  *pauseCallbackAction;

    m_pnlLeftInfoPanelLandscape->Show();
    m_pnlRightButtonPanelLandscape->Show();
    m_pnlRightInfoPanelLandscape->Show();

    m_pnlLeftInfoPanelPortrait->Show();
    m_pnlRightButtonPanelPortrait->Show();
    m_pnlRightInfoPanelPortrait->Show();

    /*
     * Landscape animation
     */
    leftLandscapeAABB = ((hdUIContainer *)m_pnlLeftInfoPanelLandscape)->GetAABB();
    rightLandscapeAABB = ((hdUIContainer *)m_pnlRightInfoPanelLandscape)->GetAABB();

    leftCenterLandscape = m_pnlLeftInfoPanelLandscape->GetWorldCenter();
    rightCenterLandscape = m_pnlRightInfoPanelLandscape->GetWorldCenter();

    // Start just off the left of the screen
    m_pnlLeftInfoPanelLandscape->Translate(hdVec3(-leftLandscapeAABB.Width(), 0.0f, 0.0f), leftCenterLandscape);

    // Start just off the right of the screen
    m_pnlRightInfoPanelLandscape->Translate(hdVec3(rightLandscapeAABB.Width(), 0.0f, 0.0f), rightCenterLandscape);

    leftSlideAnimLandscape = hdAnimationController::CreateAnimation(this, true);
    leftVectorActionLandscape = new hdVectorAction();
    leftVectorActionLandscape->SetDuration(0.20f);
    leftVectorActionLandscape->SetDestination(leftCenterLandscape);
    leftSlideAnimLandscape->AddAction(leftVectorActionLandscape);
    leftSlideAnimLandscape->AddGameObject(m_pnlLeftInfoPanelLandscape);
    leftSlideAnimLandscape->StartAnimation();

    rightSlideAnimLandscape = hdAnimationController::CreateAnimation(this, true);
    rightVectorActionLandscape = new hdVectorAction();
    pauseActionLandscape = new hdPauseAction();
    pauseActionLandscape->SetDuration(0.05f);
    rightVectorActionLandscape->SetDuration(0.20f);
    rightVectorActionLandscape->SetDestination(rightCenterLandscape);

    rightSlideAnimLandscape->AddAction(pauseActionLandscape);
    rightSlideAnimLandscape->AddAction(rightVectorActionLandscape);
    rightSlideAnimLandscape->AddGameObject(m_pnlRightInfoPanelLandscape);
    rightSlideAnimLandscape->StartAnimation();

    /*
     * Portrait animation
     */
    leftPortraitAABB = ((hdUIContainer *)m_pnlLeftInfoPanelPortrait)->GetAABB();
    rightPortraitAABB = ((hdUIContainer *)m_pnlRightInfoPanelPortrait)->GetAABB();

    leftCenterPortrait = m_pnlLeftInfoPanelPortrait->GetWorldCenter();
    rightCenterPortrait = m_pnlRightInfoPanelPortrait->GetWorldCenter();

    // Start just off the left of the screen
    m_pnlLeftInfoPanelPortrait->Translate(hdVec3(-(leftCenterPortrait.x -  (leftCenterPortrait.x-leftPortraitAABB.Width())), 0.0f, 0.0f), leftCenterPortrait);

    // Start just off the right of the screen
    m_pnlRightInfoPanelPortrait->Translate(hdVec3((rightCenterPortrait.x + rightPortraitAABB.Width()) - rightCenterPortrait.x, 0.0f, 0.0f), rightCenterPortrait);

    leftSlideAnimPortrait = hdAnimationController::CreateAnimation(this, true);
    leftVectorActionPortrait = new hdVectorAction();
    leftVectorActionPortrait->SetDuration(0.20f);
    leftVectorActionPortrait->SetDestination(leftCenterPortrait);
    leftSlideAnimPortrait->AddAction(leftVectorActionPortrait);
    leftSlideAnimPortrait->AddGameObject(m_pnlLeftInfoPanelPortrait);
    leftSlideAnimPortrait->StartAnimation();

    rightSlideAnimPortrait = hdAnimationController::CreateAnimation(this, true);
    rightVectorActionPortrait = new hdVectorAction();
    pauseActionPortrait = new hdPauseAction();
    pauseActionPortrait->SetDuration(0.05f);
    rightVectorActionPortrait->SetDuration(0.20f);
    rightVectorActionPortrait->SetDestination(rightCenterPortrait);

    rightSlideAnimPortrait->AddAction(pauseActionPortrait);
    rightSlideAnimPortrait->AddAction(rightVectorActionPortrait);
    rightSlideAnimPortrait->AddGameObject(m_pnlRightInfoPanelPortrait);
    rightSlideAnimPortrait->StartAnimation();

    /*
     * Pause - so callback is called just after the two above anims stop
     */
    pauseCallbackAnim  = hdAnimationController::CreateAnimation(this, true);
    pauseCallbackAction  = new hdPauseAction();
    pauseCallbackAction->SetDuration(0.3f);
    pauseCallbackAnim->AddAction(pauseCallbackAction);
    pauseCallbackAnim->SetFinishedCallback(this, LevelPickerController::ShowLevelInfoAnimCallback);
    pauseCallbackAnim->StartAnimation();

    m_btnPlayLevelLandscape->Disable();
    m_btnCancelInfoLandscape->Disable();

    m_btnPlayLevelPortrait->Disable();
    m_btnCancelInfoPortrait->Disable();
#else
    hdVec3 leftCenterLandscape, rightCenterLandscape;
    hdVec3 leftCenterPortrait, rightCenterPortrait;

    hdAABB leftLandscapeAABB, rightLandscapeAABB;
    hdAABB leftPortraitAABB, rightPortraitAABB;

    hdAnimation *leftSlideAnimLandscape, *rightSlideAnimLandscape;
    hdVectorAction *leftVectorActionLandscape, *rightVectorActionLandscape;

    hdAnimation *leftSlideAnimPortrait, *rightSlideAnimPortrait;
    hdVectorAction *leftVectorActionPortrait, *rightVectorActionPortrait;

    hdPauseAction  *pauseActionLandscape, *pauseActionPortrait;
    hdAnimation *pauseCallbackAnim;
    hdPauseAction  *pauseCallbackAction;

    m_pnlLeftInfoPanelLandscape->Show();
    m_pnlRightButtonPanelLandscape->Show();
    m_pnlRightInfoPanelLandscape->Show();

    m_pnlLeftInfoPanelPortrait->Show();
    m_pnlRightButtonPanelPortrait->Show();
    m_pnlRightInfoPanelPortrait->Show();

    /*
     * Landscape animation
     */
    leftLandscapeAABB = ((hdUIContainer *)m_pnlLeftInfoPanelLandscape)->GetAABB();
    rightLandscapeAABB = ((hdUIContainer *)m_pnlRightInfoPanelLandscape)->GetAABB();

    leftCenterLandscape = leftPanelStartingPosition;
    rightCenterLandscape = rightPanelStartingPosition;

    // Start just off the left of the screen
    m_pnlLeftInfoPanelLandscape->Translate(hdVec3(-leftLandscapeAABB.Width(), 0.0f, 0.0f), leftCenterLandscape);

    // Start just off the right of the screen
    m_pnlRightInfoPanelLandscape->Translate(hdVec3(rightLandscapeAABB.Width(), 0.0f, 0.0f), rightCenterLandscape);

    leftSlideAnimLandscape = hdAnimationController::CreateAnimation(this, true);
    leftVectorActionLandscape = new hdVectorAction();
    leftVectorActionLandscape->SetDuration(0.20f);
    leftVectorActionLandscape->SetDestination(leftCenterLandscape);
    leftSlideAnimLandscape->AddAction(leftVectorActionLandscape);
    leftSlideAnimLandscape->AddGameObject(m_pnlLeftInfoPanelLandscape);
    leftSlideAnimLandscape->StartAnimation();

    rightSlideAnimLandscape = hdAnimationController::CreateAnimation(this, true);
    rightVectorActionLandscape = new hdVectorAction();
    pauseActionLandscape = new hdPauseAction();
    pauseActionLandscape->SetDuration(0.05f);
    rightVectorActionLandscape->SetDuration(0.20f);
    rightVectorActionLandscape->SetDestination(rightCenterLandscape);

    rightSlideAnimLandscape->AddAction(pauseActionLandscape);
    rightSlideAnimLandscape->AddAction(rightVectorActionLandscape);
    rightSlideAnimLandscape->AddGameObject(m_pnlRightInfoPanelLandscape);
    rightSlideAnimLandscape->StartAnimation();

    /*
     * Portrait animation
     */
    leftPortraitAABB = ((hdUIContainer *)m_pnlLeftInfoPanelPortrait)->GetAABB();
    rightPortraitAABB = ((hdUIContainer *)m_pnlRightInfoPanelPortrait)->GetAABB();

    leftCenterPortrait = m_pnlLeftInfoPanelPortrait->GetWorldCenter();
    rightCenterPortrait = m_pnlRightInfoPanelPortrait->GetWorldCenter();

    // Start just off the left of the screen
    m_pnlLeftInfoPanelPortrait->Translate(hdVec3(-(leftCenterPortrait.x -  (leftCenterPortrait.x-leftPortraitAABB.Width())), 0.0f, 0.0f), leftCenterPortrait);

    // Start just off the right of the screen
    m_pnlRightInfoPanelPortrait->Translate(hdVec3((rightCenterPortrait.x + rightPortraitAABB.Width()) - rightCenterPortrait.x, 0.0f, 0.0f), rightCenterPortrait);

    leftSlideAnimPortrait = hdAnimationController::CreateAnimation(this, true);
    leftVectorActionPortrait = new hdVectorAction();
    leftVectorActionPortrait->SetDuration(0.20f);
    leftVectorActionPortrait->SetDestination(leftCenterPortrait);
    leftSlideAnimPortrait->AddAction(leftVectorActionPortrait);
    leftSlideAnimPortrait->AddGameObject(m_pnlLeftInfoPanelPortrait);
    leftSlideAnimPortrait->StartAnimation();

    rightSlideAnimPortrait = hdAnimationController::CreateAnimation(this, true);
    rightVectorActionPortrait = new hdVectorAction();
    pauseActionPortrait = new hdPauseAction();
    pauseActionPortrait->SetDuration(0.05f);
    rightVectorActionPortrait->SetDuration(0.20f);
    rightVectorActionPortrait->SetDestination(rightCenterPortrait);

    rightSlideAnimPortrait->AddAction(pauseActionPortrait);
    rightSlideAnimPortrait->AddAction(rightVectorActionPortrait);
    rightSlideAnimPortrait->AddGameObject(m_pnlRightInfoPanelPortrait);
    rightSlideAnimPortrait->StartAnimation();

    /*
     * Pause - so callback is called just after the two above anims stop
     */
    pauseCallbackAnim  = hdAnimationController::CreateAnimation(this, true);
    pauseCallbackAction  = new hdPauseAction();
    pauseCallbackAction->SetDuration(0.3f);
    pauseCallbackAnim->AddAction(pauseCallbackAction);
    pauseCallbackAnim->SetFinishedCallback(this, LevelPickerController::ShowLevelInfoAnimCallback);
    pauseCallbackAnim->StartAnimation();

    m_btnPlayLevelLandscape->Disable();
    m_btnCancelInfoLandscape->Disable();

    m_btnPlayLevelPortrait->Disable();
    m_btnCancelInfoPortrait->Disable();
#endif
}


void LevelPickerController::ShowLevelInfoAnimCallback(void *handler, hdAnimation *anim)
{
    LevelPickerController *self = (LevelPickerController *)handler;

    if (self != NULL)
    {
        self->m_btnPlayLevelLandscape->Enable();
        self->m_btnCancelInfoLandscape->Enable();

        self->m_btnPlayLevelPortrait->Enable();
        self->m_btnCancelInfoPortrait->Enable();
    }
}


void LevelPickerController::HideLevelInfoInterface()
{
#ifdef IPHONE_BUILD
    hdVec3 leftCenterLandscape, rightCenterLandscape;
    hdVec3 leftCenterPortrait, rightCenterPortrait;
    hdVec3 rightVectorDestination;

    hdAnimation *leftSlideAnimLandscape, *rightSlideAnimLandscape;
    hdVectorAction *leftVectorActionLandscape, *rightVectorActionLandscape;

    hdAnimation *leftSlideAnimPortrait, *rightSlideAnimPortrait;
    hdVectorAction *leftVectorActionPortrait, *rightVectorActionPortrait;

    hdPauseAction  *pauseActionLandscape, *pauseActionPortrait;
    hdAnimation *pauseCallbackAnim;
    hdPauseAction  *pauseCallbackAction;

    /*
     * Landscape slide back
     */
    leftCenterLandscape = m_pnlLeftInfoPanelLandscape->GetWorldCenter();
    rightCenterLandscape = m_pnlRightInfoPanelLandscape->GetWorldCenter();

    leftSlideAnimLandscape = hdAnimationController::CreateAnimation(this, true);
    leftVectorActionLandscape = new hdVectorAction();
    leftVectorActionLandscape->SetDuration(0.15f);
    leftVectorActionLandscape->SetDestination(hdVec3(-leftCenterLandscape.x,
                                                     leftCenterLandscape.y,
                                                     leftCenterLandscape.z));
    leftSlideAnimLandscape->AddAction(leftVectorActionLandscape);
    leftSlideAnimLandscape->AddGameObject(m_pnlLeftInfoPanelLandscape);
    leftSlideAnimLandscape->StartAnimation();

    rightSlideAnimLandscape = hdAnimationController::CreateAnimation(this, true);
    rightVectorActionLandscape = new hdVectorAction();
    pauseActionLandscape = new hdPauseAction();
    pauseActionLandscape->SetDuration(0.05f);
    rightVectorActionLandscape->SetDuration(0.15f);
    rightVectorDestination.Set(m_PixelScreenWidth + (m_PixelScreenWidth - rightCenterLandscape.x),
                               rightCenterLandscape.y,
                               rightCenterLandscape.z);
    rightVectorActionLandscape->SetDestination(rightVectorDestination);

    rightSlideAnimLandscape->AddAction(pauseActionLandscape);
    rightSlideAnimLandscape->AddAction(rightVectorActionLandscape);
    rightSlideAnimLandscape->AddGameObject(m_pnlRightInfoPanelLandscape);
    rightSlideAnimLandscape->StartAnimation();

    /*
     * Portrait slide back
     */
    leftCenterPortrait = m_pnlLeftInfoPanelPortrait->GetWorldCenter();
    rightCenterPortrait = m_pnlRightInfoPanelPortrait->GetWorldCenter();

    leftSlideAnimPortrait = hdAnimationController::CreateAnimation(this, true);
    leftVectorActionPortrait = new hdVectorAction();
    leftVectorActionPortrait->SetDuration(0.15f);
    leftVectorActionPortrait->SetDestination(hdVec3(-leftCenterPortrait.x,
                                                    leftCenterPortrait.y,
                                                    leftCenterPortrait.z));
    leftSlideAnimPortrait->AddAction(leftVectorActionPortrait);
    leftSlideAnimPortrait->AddGameObject(m_pnlLeftInfoPanelPortrait);
    leftSlideAnimPortrait->StartAnimation();

    rightSlideAnimPortrait = hdAnimationController::CreateAnimation(this, true);
    rightVectorActionPortrait = new hdVectorAction();
    pauseActionPortrait = new hdPauseAction();
    pauseActionPortrait->SetDuration(0.05f);
    rightVectorActionPortrait->SetDuration(0.15f);
    rightVectorDestination.Set(m_PixelScreenWidth + (m_PixelScreenWidth - rightCenterPortrait.x),
                               rightCenterPortrait.y,
                               rightCenterPortrait.z);
    rightVectorActionPortrait->SetDestination(rightVectorDestination);

    rightSlideAnimPortrait->AddAction(pauseActionPortrait);
    rightSlideAnimPortrait->AddAction(rightVectorActionPortrait);
    rightSlideAnimPortrait->AddGameObject(m_pnlRightInfoPanelPortrait);
    rightSlideAnimPortrait->StartAnimation();

    /*
     * Pause - so callback is called just after the two above anims stop
     */
    pauseCallbackAnim  = hdAnimationController::CreateAnimation(this, true);
    pauseCallbackAction  = new hdPauseAction();
    pauseCallbackAction->SetDuration(0.25f);
    pauseCallbackAnim->AddAction(pauseCallbackAction);
    pauseCallbackAnim->SetFinishedCallback(this, LevelPickerController::HideLevelInfoAnimCallback);
    pauseCallbackAnim->StartAnimation();

    m_btnPlayLevelLandscape->Disable();
    m_btnCancelInfoLandscape->Disable();

    m_btnPlayLevelPortrait->Disable();
    m_btnCancelInfoPortrait->Disable();

#else
    hdVec3 leftCenterLandscape, rightCenterLandscape;
    hdVec3 leftCenterPortrait, rightCenterPortrait;
    hdVec3 rightVectorDestination;
    
    hdAnimation *leftSlideAnimLandscape, *rightSlideAnimLandscape;
    hdVectorAction *leftVectorActionLandscape, *rightVectorActionLandscape;
    
    hdAnimation *leftSlideAnimPortrait, *rightSlideAnimPortrait;
    hdVectorAction *leftVectorActionPortrait, *rightVectorActionPortrait;
    
    hdPauseAction  *pauseActionLandscape, *pauseActionPortrait;
    hdAnimation *pauseCallbackAnim;
    hdPauseAction  *pauseCallbackAction;
    
    /*
     * Landscape slide back
     */
    leftCenterLandscape = m_pnlLeftInfoPanelLandscape->GetWorldCenter();
    rightCenterLandscape = m_pnlRightInfoPanelLandscape->GetWorldCenter();	
    
    leftSlideAnimLandscape = hdAnimationController::CreateAnimation(this, true);
    leftVectorActionLandscape = new hdVectorAction();
    leftVectorActionLandscape->SetDuration(0.15f);
    leftVectorActionLandscape->SetDestination(hdVec3(-leftCenterLandscape.x, 
                                                     leftCenterLandscape.y, 
                                                     leftCenterLandscape.z));
    leftSlideAnimLandscape->AddAction(leftVectorActionLandscape);
    leftSlideAnimLandscape->AddGameObject(m_pnlLeftInfoPanelLandscape);
    leftSlideAnimLandscape->StartAnimation();
    
    rightSlideAnimLandscape = hdAnimationController::CreateAnimation(this, true);
    rightVectorActionLandscape = new hdVectorAction();
    pauseActionLandscape = new hdPauseAction();
    pauseActionLandscape->SetDuration(0.05f);
    rightVectorActionLandscape->SetDuration(0.15f);
    rightVectorDestination.Set(m_PixelScreenWidth + (m_PixelScreenWidth - rightCenterLandscape.x), 
                               rightCenterLandscape.y, 
                               rightCenterLandscape.z);
    rightVectorActionLandscape->SetDestination(rightVectorDestination);
    
    rightSlideAnimLandscape->AddAction(pauseActionLandscape);
    rightSlideAnimLandscape->AddAction(rightVectorActionLandscape);
    rightSlideAnimLandscape->AddGameObject(m_pnlRightInfoPanelLandscape);
    rightSlideAnimLandscape->StartAnimation();
    
    /*
     * Portrait slide back
     */
    leftCenterPortrait = m_pnlLeftInfoPanelPortrait->GetWorldCenter();
    rightCenterPortrait = m_pnlRightInfoPanelPortrait->GetWorldCenter();	
    
    leftSlideAnimPortrait = hdAnimationController::CreateAnimation(this, true);
    leftVectorActionPortrait = new hdVectorAction();
    leftVectorActionPortrait->SetDuration(0.15f);
    leftVectorActionPortrait->SetDestination(hdVec3(-leftCenterPortrait.x, 
                                                    leftCenterPortrait.y, 
                                                    leftCenterPortrait.z));
    leftSlideAnimPortrait->AddAction(leftVectorActionPortrait);
    leftSlideAnimPortrait->AddGameObject(m_pnlLeftInfoPanelPortrait);
    leftSlideAnimPortrait->StartAnimation();
    
    rightSlideAnimPortrait = hdAnimationController::CreateAnimation(this, true);
    rightVectorActionPortrait = new hdVectorAction();
    pauseActionPortrait = new hdPauseAction();
    pauseActionPortrait->SetDuration(0.05f);
    rightVectorActionPortrait->SetDuration(0.15f);
    rightVectorDestination.Set(m_PixelScreenWidth + (m_PixelScreenWidth - rightCenterPortrait.x), 
                               rightCenterPortrait.y, 
                               rightCenterPortrait.z);
    rightVectorActionPortrait->SetDestination(rightVectorDestination);
    
    rightSlideAnimPortrait->AddAction(pauseActionPortrait);
    rightSlideAnimPortrait->AddAction(rightVectorActionPortrait);
    rightSlideAnimPortrait->AddGameObject(m_pnlRightInfoPanelPortrait);
    rightSlideAnimPortrait->StartAnimation();
    
    /*
     * Pause - so callback is called just after the two above anims stop
     */
    pauseCallbackAnim  = hdAnimationController::CreateAnimation(this, true);
    pauseCallbackAction  = new hdPauseAction();
    pauseCallbackAction->SetDuration(0.25f);
    pauseCallbackAnim->AddAction(pauseCallbackAction);
    pauseCallbackAnim->SetFinishedCallback(this, LevelPickerController::HideLevelInfoAnimCallback);
    pauseCallbackAnim->StartAnimation();
    
    
    m_btnPlayLevelLandscape->Disable();
    m_btnCancelInfoLandscape->Disable();
    
    m_btnPlayLevelPortrait->Disable();
    m_btnCancelInfoPortrait->Disable();
#endif
}


void LevelPickerController::HideLevelInfoAnimCallback(void *handler, hdAnimation *anim)
{
#ifdef IPHONE_BUILD
    hdAABB leftLandscapeAABB, rightLandscapeAABB;
    hdAABB leftPortraitAABB, rightPortraitAABB;
    
    LevelPickerController *self = (LevelPickerController *)handler;
    self->RefreshLayout();
    
    self->m_interactionState = e_waitingForInput;
    
    self->m_pnlLeftInfoPanelLandscape->Hide();
    self->m_pnlRightInfoPanelLandscape->Hide();
    self->m_pnlRightButtonPanelLandscape->Hide();
    
    leftLandscapeAABB = ((hdUIContainer *)(self->m_pnlLeftInfoPanelLandscape))->GetAABB();
    rightLandscapeAABB = ((hdUIContainer *)(self->m_pnlRightInfoPanelLandscape))->GetAABB();
    
    self->m_pnlLeftInfoPanelLandscape->Translate(hdVec3(10.0f - leftLandscapeAABB.lower.x, 0.0f, 0.0f), 
                                                 self->m_pnlLeftInfoPanelLandscape->GetWorldCenter());
    self->m_pnlRightInfoPanelLandscape->Translate(hdVec3(-(rightLandscapeAABB.Width() + 20.0f), 0.0f, 0.0f), 
                                                  self->m_pnlRightInfoPanelLandscape->GetWorldCenter());
    
    /*
     * Landscape animation
     */
    self->m_pnlLeftInfoPanelPortrait->Hide();
    self->m_pnlRightInfoPanelPortrait->Hide();
    self->m_pnlRightButtonPanelPortrait->Hide();
    
    leftPortraitAABB = ((hdUIContainer *)(self->m_pnlLeftInfoPanelPortrait))->GetAABB();
    rightPortraitAABB = ((hdUIContainer *)(self->m_pnlRightInfoPanelPortrait))->GetAABB();
    
    self->m_pnlLeftInfoPanelPortrait->Translate(hdVec3(10.0f - leftPortraitAABB.lower.x, 0.0f, 0.0f), 
                                                self->m_pnlLeftInfoPanelPortrait->GetWorldCenter());
    self->m_pnlRightInfoPanelPortrait->Translate(hdVec3(-(rightPortraitAABB.lower.x - 10.0f), 0.0f, 0.0f), 
                                                 self->m_pnlRightInfoPanelPortrait->GetWorldCenter());
#else
    hdAABB leftLandscapeAABB, rightLandscapeAABB;
    hdAABB leftPortraitAABB, rightPortraitAABB;
    
    LevelPickerController *self = (LevelPickerController *)handler;
    self->RefreshLayout();
    
    self->m_interactionState = e_waitingForInput;
    
    self->m_pnlLeftInfoPanelLandscape->Hide();
    self->m_pnlRightInfoPanelLandscape->Hide();
    self->m_pnlRightButtonPanelLandscape->Hide();
    
    leftLandscapeAABB = ((hdUIContainer *)(self->m_pnlLeftInfoPanelLandscape))->GetAABB();
    rightLandscapeAABB = ((hdUIContainer *)(self->m_pnlRightInfoPanelLandscape))->GetAABB();
    
    self->m_pnlLeftInfoPanelLandscape->Translate(hdVec3(10.0f - leftLandscapeAABB.lower.x, 0.0f, 0.0f), 
                                                 self->m_pnlLeftInfoPanelLandscape->GetWorldCenter());
    self->m_pnlRightInfoPanelLandscape->Translate(hdVec3(-(rightLandscapeAABB.Width() + 20.0f), 0.0f, 0.0f), 
                                                  self->m_pnlRightInfoPanelLandscape->GetWorldCenter());
    
    /*
     * Landscape animation
     */
    self->m_pnlLeftInfoPanelPortrait->Hide();
    self->m_pnlRightInfoPanelPortrait->Hide();
    self->m_pnlRightButtonPanelPortrait->Hide();
    
    leftPortraitAABB = ((hdUIContainer *)(self->m_pnlLeftInfoPanelPortrait))->GetAABB();
    rightPortraitAABB = ((hdUIContainer *)(self->m_pnlRightInfoPanelPortrait))->GetAABB();
    
    self->m_pnlLeftInfoPanelPortrait->Translate(hdVec3(10.0f - leftPortraitAABB.lower.x, 0.0f, 0.0f), 
                                                self->m_pnlLeftInfoPanelPortrait->GetWorldCenter());
    self->m_pnlRightInfoPanelPortrait->Translate(hdVec3(-(rightPortraitAABB.lower.x - 10.0f), 0.0f, 0.0f), 
                                                 self->m_pnlRightInfoPanelPortrait->GetWorldCenter());
#endif
}



void LevelPickerController::BackToMenuClicked()
{
    if (m_fadeAnim->GetStatus() == e_animationRunning) return;
    
    HideMessage();
    
    hdSoundManager::PlaySound(m_sound_btnClickBack01);
    
    m_fadeAction->SetStartAlpha(0.0f);
    m_fadeAction->SetEndAlpha(1.0f);
    
    m_fadeAnim->SetFinishedCallback(m_parentController, AppCallbackFunctions::LevelPickerBackCallback);
    m_fadeAnim->StartAnimation();
}
