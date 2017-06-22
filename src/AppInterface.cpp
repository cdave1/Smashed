/*
 * Copyright (c) 2017 Hackdirt Ltd.
 * Author: David Petrie (david@davidpetrie.com)
 *
 * License: https://www.gnu.org/licenses/gpl-3.0.en.html
 */

#include "AppInterface.h"


static hdUIImage *m_uiImageMessage = NULL;

static hdFontPolygon *m_uiTextMessage = NULL;

static hdUIImage *m_uiTextMessageBackground = NULL;

static hdButton *m_OkButton = NULL;

static hdButton *m_DoneButton = NULL;

static hdButton *m_SkipButton = NULL;

static hdButton *m_PrevButton = NULL;

static hdButton *m_NextButton = NULL;

static hdUIContainer *m_uiMessageContainer = NULL;

static hdUIContainer *m_uiMessageButtonContainer = NULL;


AppInterface::AppInterface(const hdInterfaceController* parentController) :
hdInterfaceController(parentController->GetOrientation(), parentController)
{
    hdInterfaceContext::Init(this);

    m_HACKHACKHACK_currentLevelId = 0;

    m_messageCount = 0;

    InitAppInterface();
}


AppInterface::~AppInterface()
{
    hdInterfaceContext::TearDown(this);

    delete m_uiMessageQueueButton;
    delete m_uiMessageQueueCount;
    delete m_uiMessageQueueContainer;

    delete m_uiLandscapeTextMessage;
    delete m_uiLandscapeTextMessageBackground;
    delete m_uiLandscapeMessageOKButton;
    delete m_uiLandscapeMessageSkipButton;
    delete m_uiLandscapeMessagePrevButton;
    delete m_uiLandscapeMessageNextButton;
    delete m_uiLandscapeMessageDoneButton;

    delete m_uiPortraitTextMessage;
    delete m_uiPortraitTextMessageBackground;
    delete m_uiPortraitMessageOKButton;
    delete m_uiPortraitMessageSkipButton;
    delete m_uiPortraitMessagePrevButton;
    delete m_uiPortraitMessageNextButton;
    delete m_uiPortraitMessageDoneButton;

    delete m_uiLandscapeMessageContainer;
    delete m_uiLandscapeMessageButtonContainer;
    delete m_uiPortraitMessageContainer;
    delete m_uiPortraitMessageButtonContainer;

    delete m_uiLandscapeAvatar;
    delete m_uiLandscapeAvatarChrome;
    delete m_uiLandscapeAvatarMessageText;
    delete m_uiLandscapeAvatarMessageBackground;
    delete m_uiLandscapeAvatarMessageChrome;

    delete m_uiPortraitAvatar;
    delete m_uiPortraitAvatarChrome;
    delete m_uiPortraitAvatarMessageText;
    delete m_uiPortraitAvatarMessageBackground;
    delete m_uiPortraitAvatarMessageChrome;

    delete m_uiLandscapeTitleMessageBackground;
    delete m_uiLandscapeTitleMessageFont;

    delete m_uiPortraitTitleMessageBackground;
    delete m_uiPortraitTitleMessageFont;
}



void AppInterface::InitAppInterface()
{
#ifdef IPHONE_BUILD
    float pixelScreenWidth, pixelScreenHeight;
    float titleBackgroundWidth, titleBackgroundHeight;
    float avatarSquareSize;
    float miscButtonHeight;
    float buttonWidth, buttonHeight;
    float gapDistance;

    ScreenSettings_GetScreenRect(&pixelScreenWidth, &pixelScreenHeight);
    buttonWidth = hdClamp(pixelScreenWidth * 0.25f, 160.0f, 320.0f);
    buttonHeight = hdClamp(pixelScreenHeight * 0.18f, 55.0f, 110.f);
    miscButtonHeight = hdClamp(pixelScreenHeight * 0.125f, 40.0f, 100.0f);
    gapDistance = 0.1875f * m_PixelScreenHeight;

    titleBackgroundWidth = 256.0f;
    titleBackgroundHeight = 128.0f;
    avatarSquareSize = hdClamp(0.267f * m_PixelScreenWidth, 128.0f, 256.0f);

    m_btnMessageClickDownSound = hdSoundManager::FindSound("Sounds/btnClickDown.caf", e_soundTypeNormal);
    m_btnMessageClickUpSound = hdSoundManager::FindSound("Sounds/btnClick01.caf", e_soundTypeNormal);

    /*
     * Prefs
     */
    m_prefsView = new PreferencesView();
    m_prefsView->Hide();
    m_prefsView->Disable();
    m_prefsView->SetCloseListener(this, AppInterface::ClosePreferencesCallback);

    /*
     * Landscape messages
     */
    float longSide;
    float shortSide;
    longSide = hdMax(m_PixelScreenWidth, m_PixelScreenHeight);
    shortSide = hdMin(m_PixelScreenWidth, m_PixelScreenHeight);
    m_uiLandscapeTitleMessageBackground = new hdUIImage("/Interface/Textures/imgTitleBackground.png", NULL);
    m_uiLandscapeTitleMessageBackground->SetAs2DBox((longSide-256.0f)/2.0f, (shortSide-100.0f)/2.0f, 256.0f, 100.0f);

    m_uiLandscapeTitleMessageFont = new hdFontPolygon(LEVEL_CONTROLLER_TITLE_FONT, NULL);
    m_uiLandscapeTitleMessageFont->SetTint(0.0f, 0.0f, 0.0f, 1.0f);
    m_uiLandscapeTitleMessageFont->SetAs2DBox((longSide-256.0f)/2.0f, (shortSide-150.0f)/2.0f, 256.0f, 100.0f);
    m_uiLandscapeTitleMessageFont->AlignCenter();

    /*
     * Landscape messagebox
     */
    m_uiLandscapeTextMessage = new hdFontPolygon(LEVEL_CONTROLLER_MESSAGE_FONT, NULL);
    m_uiLandscapeTextMessage->SetTint(0.0f, 0.0f, 0.0f, 1.0f);
    m_uiLandscapeTextMessage->SetScale(0.75f);
    m_uiLandscapeTextMessage->SetAs2DBox(200.0f-m_PixelScreenWidth, 75.0f, 175.0f, 173.0f);
    m_uiLandscapeTextMessage->AlignLeft();

    m_uiLandscapeTextMessageBackground = new hdUIImage(NULL, NULL);
    m_uiLandscapeTextMessageBackground->SetAs2DBox(((m_PixelScreenWidth-240.0f)/2.0f), 60.0f, 240.0f, 240.0f);

    // Avatar message support
    m_uiLandscapeAvatar = new hdUIImage("", NULL);
    m_uiLandscapeAvatar->SetAs2DBox(30.0f, m_PixelScreenHeight - (gapDistance + avatarSquareSize),
                                    avatarSquareSize, avatarSquareSize);
    m_uiLandscapeAvatar->Hide();

    m_uiLandscapeAvatarChrome = new hdUIImage(NULL, NULL);
    m_uiLandscapeAvatarChrome->SetTint(0.0f, 0.0f, 0.0f, 0.8f);
    m_uiLandscapeAvatarChrome->SetAs2DBox(20.0f, m_PixelScreenHeight - (gapDistance + 10.0f + avatarSquareSize),
                                          (avatarSquareSize + 20.0f), (avatarSquareSize + 20.0f));
    m_uiLandscapeAvatarChrome->Hide();

    // God this is ridiculous
    m_uiLandscapeAvatarMessageText = new hdFontPolygon(LEVEL_CONTROLLER_MESSAGE_FONT, NULL);
    m_uiLandscapeAvatarMessageText->SetTint(0.1f, 0.1f, 0.1f, 1.0f);
    m_uiLandscapeAvatarMessageText->SetAs2DBox(avatarSquareSize + (0.15f * m_PixelScreenWidth), 28.0f,
                                               m_PixelScreenWidth - (avatarSquareSize + (0.35 * m_PixelScreenWidth)), m_PixelScreenHeight - (gapDistance+40.0f));
    m_uiLandscapeAvatarMessageText->SetScale(0.8f);
    m_uiLandscapeAvatarMessageText->AlignLeft();
    m_uiLandscapeAvatarMessageText->Hide();

    m_uiLandscapeAvatarMessageBackground = new hdUIImage("Interface/Textures/imgLandscapeMessageBackground512.png", NULL);
    m_uiLandscapeAvatarMessageBackground->SetTint(1.0f, 1.0f, 1.0f, 1.0f);
    m_uiLandscapeAvatarMessageBackground->SetAs2DBox((avatarSquareSize + 40.0f), gapDistance,
                                                     m_PixelScreenWidth - (avatarSquareSize + 70.0f), m_PixelScreenHeight - 100.0f);
    m_uiLandscapeAvatarMessageBackground->Hide();

    m_uiLandscapeAvatarMessageChrome = new hdUIImage(NULL, NULL);
    m_uiLandscapeAvatarMessageChrome->SetTint(0.0f, 0.0f, 0.0f, 0.0f);
    m_uiLandscapeAvatarMessageChrome->SetAs2DBox((avatarSquareSize + 40.0f), gapDistance,
                                                 m_PixelScreenWidth - (10.0f + (avatarSquareSize + 60.0f)), m_PixelScreenHeight - 100.0f);
    m_uiLandscapeAvatarMessageChrome->Hide();

    /*
     * Landscape Buttons
     */
    m_uiLandscapeMessageOKButton  = new hdButton("/Interface/Textures/btnOkNormal.png",
                                                 "/Interface/Textures/btnOkOver.png",
                                                 "/Interface/Textures/btnOkOver.png", NULL);
    m_uiLandscapeMessageOKButton->SetAs2DBox(((m_PixelScreenWidth-64.0f)/2.0f)-m_PixelScreenWidth, 5.0f,
                                             (0.175 * pixelScreenWidth), miscButtonHeight);

    m_uiLandscapeMessageDoneButton  = new hdButton("/Interface/Textures/btnDoneLrgNormal.png",
                                                   "/Interface/Textures/btnDoneLrgOver.png",
                                                   "/Interface/Textures/btnDoneLrgOver.png", NULL);
    m_uiLandscapeMessageDoneButton->SetAs2DBox((m_PixelScreenWidth/2.0f)+10.0f-m_PixelScreenWidth, 5.0f, 64.0f, 40.0f);

    m_uiLandscapeMessagePrevButton  = new hdButton("/Interface/Textures/btnPrevLrgNormal.png",
                                                   "/Interface/Textures/btnPrevLrgOver.png",
                                                   "/Interface/Textures/btnPrevLrgOver.png", NULL);
    m_uiLandscapeMessagePrevButton->SetAs2DBox((m_PixelScreenWidth/2.0f)-74.0f-m_PixelScreenWidth, 5.0f, 64.0f, 40.0f);

    m_uiLandscapeMessageNextButton  = new hdButton("/Interface/Textures/btnNextLrgNormal.png",
                                                   "/Interface/Textures/btnNextLrgOver.png",
                                                   "/Interface/Textures/btnNextLrgOver.png", NULL);
    m_uiLandscapeMessageNextButton->SetAs2DBox((m_PixelScreenWidth/2.0f)+10.0f-m_PixelScreenWidth, 5.0f, 64.0f, 40.0f);

    m_uiLandscapeMessageSkipButton = new hdButton("/Interface/Textures/btnSkipSmallNormal.png",
                                                  "/Interface/Textures/btnSkipSmallOver.png",
                                                  "/Interface/Textures/btnSkipSmallOver.png", NULL);
    m_uiLandscapeMessageSkipButton->SetAs2DBox((m_PixelScreenWidth-74.0f)-m_PixelScreenWidth, 5.0f, 48.0f, 30.0f);

    m_uiLandscapeMessageContainer = new hdUIContainer(NULL, hdVec3(-m_PixelScreenWidth, 0.0f, 0.0f),
                                                      hdVec3(0.0f, m_PixelScreenHeight, 0.0f));

    m_uiLandscapeMessageContainer->Add(m_uiLandscapeTextMessage);
    m_uiLandscapeMessageContainer->Add(m_uiLandscapeTitleMessageBackground);
    m_uiLandscapeMessageContainer->Add(m_uiLandscapeTitleMessageFont);
    m_uiLandscapeMessageContainer->Add(m_uiLandscapeAvatarChrome);
    m_uiLandscapeMessageContainer->Add(m_uiLandscapeAvatarMessageChrome);
    m_uiLandscapeMessageContainer->Add(m_uiLandscapeAvatarMessageBackground);
    m_uiLandscapeMessageContainer->Add(m_uiLandscapeAvatar);
    m_uiLandscapeMessageContainer->Add(m_uiLandscapeAvatarMessageText);

    m_uiLandscapeMessageContainer->Translate(hdVec3(-m_PixelScreenWidth, 0.0f, 0.0f), m_uiLandscapeMessageContainer->GetWorldCenter());

    m_uiLandscapeMessageButtonContainer = new hdUIContainer(NULL, hdVec3(-m_PixelScreenWidth, 0.0f, 0.0f),
                                                            hdVec3(0.0f, m_PixelScreenHeight, 0.0f));
    m_uiLandscapeMessageButtonContainer->Add(m_uiLandscapeMessageOKButton);
    m_uiLandscapeMessageButtonContainer->Add(m_uiLandscapeMessageDoneButton);
    m_uiLandscapeMessageButtonContainer->Add(m_uiLandscapeMessageSkipButton);
    m_uiLandscapeMessageButtonContainer->Add(m_uiLandscapeMessagePrevButton);
    m_uiLandscapeMessageButtonContainer->Add(m_uiLandscapeMessageNextButton);


    /*
     * Portrait messagebox
     */
    m_uiPortraitTitleMessageBackground = new hdUIImage("/Interface/Textures/imgTitleBackground.png", NULL);
    m_uiPortraitTitleMessageBackground->SetAs2DBox((m_PixelScreenWidth - titleBackgroundWidth)/2.0f,
                                                   (m_PixelScreenHeight - titleBackgroundHeight)/2.0f,
                                                   titleBackgroundWidth, titleBackgroundHeight);

    m_uiPortraitTitleMessageFont = new hdFontPolygon(LEVEL_CONTROLLER_TITLE_FONT, NULL);
    m_uiPortraitTitleMessageFont->SetTint(0.0f, 0.0f, 0.0f, 1.0f);
    m_uiPortraitTitleMessageFont->SetAs2DBox((m_PixelScreenWidth - titleBackgroundWidth)/2.0f,
                                             (m_PixelScreenHeight - titleBackgroundHeight)/2.0f,
                                             titleBackgroundWidth, titleBackgroundHeight);
    m_uiPortraitTitleMessageFont->AlignCenter();


    m_uiPortraitTextMessage = new hdFontPolygon(LEVEL_CONTROLLER_MESSAGE_FONT, NULL);
    m_uiPortraitTextMessage->SetTint(0.0f, 0.0f, 0.0f, 1.0f);
    m_uiPortraitTextMessage->SetAs2DBox(30.0f, 140.0f, 200.0f, 130.0f);
    m_uiPortraitTextMessage->AlignLeft();

    m_uiPortraitTextMessageBackground = new hdUIImage(NULL, NULL);
    m_uiPortraitTextMessageBackground->SetAs2DBox(((m_PixelScreenHeight-256.0f)/2.0f), 80.0f, 256.0f, 256.0f);

    m_uiPortraitAvatar = new hdUIImage("", NULL);
    m_uiPortraitAvatar->SetAs2DBox(20.0f, m_PixelScreenWidth - (60.0f + 128.0f), 128.0f, 128.0f);
    m_uiPortraitAvatar->Hide();

    m_uiPortraitAvatarChrome = new hdUIImage(NULL, NULL);
    m_uiPortraitAvatarChrome->SetTint(0.0f, 0.0f, 0.0f, 0.7f);
    m_uiPortraitAvatarChrome->SetAs2DBox(10.0f, m_PixelScreenWidth - (50.0f + 148.0f), 148.0f, 148.0f);
    m_uiPortraitAvatarChrome->Hide();

    m_uiPortraitAvatarMessageText = new hdFontPolygon(LEVEL_CONTROLLER_MESSAGE_FONT, NULL);
    m_uiPortraitAvatarMessageText->SetTint(0.0f, 0.0f, 0.0f, 1.0f);
    m_uiPortraitAvatarMessageText->SetAs2DBox(30.0f, 80.0f, 200.0f, 180.0f);
    m_uiPortraitAvatarMessageText->AlignLeft();
    m_uiPortraitAvatarMessageText->SetScale(0.8f);
    m_uiPortraitAvatarMessageText->Hide();

    m_uiPortraitAvatarMessageChrome = new hdUIImage(NULL, NULL);
    m_uiPortraitAvatarMessageChrome->SetTint(0.0f, 0.0f, 0.0f, 0.0f);
    m_uiPortraitAvatarMessageChrome->SetAs2DBox(10.0f, 70.0f, m_PixelScreenHeight - 20.0f, m_PixelScreenWidth - (50.0f + 148.0f + 70.0f + 10.0f));
    m_uiPortraitAvatarMessageChrome->Hide();

    m_uiPortraitAvatarMessageBackground = new hdUIImage("Interface/Textures/imgPortraitMessageBackground.png", NULL);
    m_uiPortraitAvatarMessageBackground->SetTint(1.0f, 1.0f, 1.0f, 1.0f);
    m_uiPortraitAvatarMessageBackground->SetAs2DBox(10.0f, 70.0f, m_PixelScreenHeight - 20.0f, m_PixelScreenWidth - (50.0f + 128.0f + 70.0f + 10.0f));
    m_uiPortraitAvatarMessageBackground->Hide();

    /*
     * Portrain Message buttons
     */
    m_uiPortraitMessageOKButton  = new hdButton("/Interface/Textures/btnOkNormal.png",
                                                "/Interface/Textures/btnOkOver.png",
                                                "/Interface/Textures/btnOkOver.png", NULL);
    m_uiPortraitMessageOKButton->SetAs2DBox(((m_PixelScreenHeight-64.0f)/2.0f)-m_PixelScreenHeight, 20.0f, 64.0f, 40.0f);

    m_uiPortraitMessageDoneButton  = new hdButton("/Interface/Textures/btnDoneLrgNormal.png",
                                                  "/Interface/Textures/btnDoneLrgOver.png",
                                                  "/Interface/Textures/btnDoneLrgOver.png", NULL);
    m_uiPortraitMessageDoneButton->SetAs2DBox((m_PixelScreenHeight/2.0f)+10.0f-m_PixelScreenHeight, 20.0f, 64.0f, 40.0f);

    m_uiPortraitMessagePrevButton  = new hdButton("/Interface/Textures/btnPrevLrgNormal.png",
                                                  "/Interface/Textures/btnPrevLrgOver.png",
                                                  "/Interface/Textures/btnPrevLrgOver.png", NULL);
    m_uiPortraitMessagePrevButton->SetAs2DBox((m_PixelScreenHeight/2.0f)-74.0f-m_PixelScreenHeight, 20.0f, 64.0f, 40.0f);

    m_uiPortraitMessageNextButton  = new hdButton("/Interface/Textures/btnNextLrgNormal.png",
                                                  "/Interface/Textures/btnNextLrgOver.png",
                                                  "/Interface/Textures/btnNextLrgOver.png", NULL);
    m_uiPortraitMessageNextButton->SetAs2DBox((m_PixelScreenHeight/2.0f)+10.0f-m_PixelScreenHeight, 20.0f, 64.0f, 40.0f);


    m_uiPortraitMessageSkipButton = new hdButton("/Interface/Textures/btnSkipSmallNormal.png",
                                                 "/Interface/Textures/btnSkipSmallOver.png",
                                                 "/Interface/Textures/btnSkipSmallOver.png", NULL);
    m_uiPortraitMessageSkipButton->SetAs2DBox((m_PixelScreenHeight-74.0f)-m_PixelScreenHeight, 20.0f, 48.0f, 30.0f);


    m_uiPortraitMessageContainer = new hdUIContainer(NULL, hdVec3(-m_PixelScreenHeight, 0.0f, 0.0f),
                                                     hdVec3(0.0f, m_PixelScreenWidth, 0.0f));
    m_uiPortraitMessageContainer->Hide();


    m_uiPortraitMessageContainer->Translate(hdVec3(-m_PixelScreenHeight, 0.0f, 0.0f), m_uiPortraitMessageContainer->GetWorldCenter());

    m_uiPortraitMessageButtonContainer = new hdUIContainer(NULL, hdVec3(-m_PixelScreenHeight, 0.0f, 0.0f),
                                                           hdVec3(0.0f, m_PixelScreenWidth, 0.0f));

    /* Landscape */
    m_uiLandscapeMessageOKButton->SetMouseDownListener(this, AppInterface::GenericButtonDownCallback);
    m_uiLandscapeMessageOKButton->SetMouseUpListener(this, AppInterface::HideMessageClickCallback);

    m_uiLandscapeMessageDoneButton->SetMouseDownListener(this, AppInterface::GenericButtonDownCallback);
    m_uiLandscapeMessageDoneButton->SetMouseUpListener(this, AppInterface::HideMessageClickCallback);

    m_uiLandscapeMessageSkipButton->SetMouseDownListener(this, AppInterface::GenericButtonDownCallback);
    m_uiLandscapeMessageSkipButton->SetMouseUpListener(this, AppInterface::SkipUpCallback);

    m_uiLandscapeMessagePrevButton->SetMouseDownListener(this, AppInterface::GenericButtonDownCallback);
    m_uiLandscapeMessagePrevButton->SetMouseUpListener(this, AppInterface::PrevUpCallback);

    m_uiLandscapeMessageNextButton->SetMouseDownListener(this, AppInterface::GenericButtonDownCallback);
    m_uiLandscapeMessageNextButton->SetMouseUpListener(this, AppInterface::HideMessageClickCallback);

    /* Portrait */
    m_uiPortraitMessageOKButton->SetMouseDownListener(this, AppInterface::GenericButtonDownCallback);
    m_uiPortraitMessageOKButton->SetMouseUpListener(this, AppInterface::HideMessageClickCallback);

    m_uiPortraitMessageDoneButton->SetMouseDownListener(this, AppInterface::GenericButtonDownCallback);
    m_uiPortraitMessageDoneButton->SetMouseUpListener(this, AppInterface::HideMessageClickCallback);

    m_uiPortraitMessageSkipButton->SetMouseDownListener(this, AppInterface::GenericButtonDownCallback);
    m_uiPortraitMessageSkipButton->SetMouseUpListener(this, AppInterface::SkipUpCallback);

    m_uiPortraitMessagePrevButton->SetMouseDownListener(this, AppInterface::GenericButtonDownCallback);
    m_uiPortraitMessagePrevButton->SetMouseUpListener(this, AppInterface::PrevUpCallback);

    m_uiPortraitMessageNextButton->SetMouseDownListener(this, AppInterface::GenericButtonDownCallback);
    m_uiPortraitMessageNextButton->SetMouseUpListener(this, AppInterface::HideMessageClickCallback);

#else

    // IPAD VERSION
    float pixelScreenWidth, pixelScreenHeight;
    float titleBackgroundWidth, titleBackgroundHeight;
    float avatarSquareSize;
    float miscButtonHeight;
    float buttonWidth, buttonHeight;
    float gapDistance;

    ScreenSettings_GetScreenRect(&pixelScreenWidth, &pixelScreenHeight);
    buttonWidth = hdClamp(pixelScreenWidth * 0.25f, 160.0f, 320.0f);
    buttonHeight = hdClamp(pixelScreenHeight * 0.18f, 55.0f, 110.f);
    miscButtonHeight = hdClamp(pixelScreenHeight * 0.125f, 40.0f, 100.0f);
    gapDistance = 0.1875f * m_PixelScreenHeight;

    titleBackgroundWidth = hdClamp(0.52f * m_PixelScreenWidth, 160.0f, 300.0f);
    titleBackgroundHeight = hdClamp(0.34f * m_PixelScreenHeight, 80.0f, 150.0f);
    avatarSquareSize = hdClamp(0.267f * m_PixelScreenWidth, 128.0f, 256.0f);

    m_btnMessageClickDownSound = hdSoundManager::FindSound("Sounds/btnClickDown.caf", e_soundTypeNormal);
    m_btnMessageClickUpSound = hdSoundManager::FindSound("Sounds/btnClick01.caf", e_soundTypeNormal);

    /*
     * Prefs
     */
    m_prefsView = new PreferencesView();
    m_prefsView->Hide();
    m_prefsView->Disable();
    m_prefsView->SetCloseListener(this, AppInterface::ClosePreferencesCallback);

    /*
     * Landscape messages
     */
    m_uiLandscapeTitleMessageBackground = new hdUIImage("/Interface/Textures/imgTitleBackground.png", NULL);
    m_uiLandscapeTitleMessageBackground->SetAs2DBox((m_PixelScreenWidth - titleBackgroundWidth)/2.0f,
                                                    (m_PixelScreenHeight - titleBackgroundHeight)/2.0f,
                                                    titleBackgroundWidth, titleBackgroundHeight);

    m_uiLandscapeTitleMessageFont = new hdFontPolygon(LEVEL_CONTROLLER_TITLE_FONT, NULL);
    m_uiLandscapeTitleMessageFont->SetTint(0.0f, 0.0f, 0.0f, 1.0f);
    m_uiLandscapeTitleMessageFont->SetAs2DBox((m_PixelScreenWidth - titleBackgroundWidth)/2.0f,
                                              ((m_PixelScreenHeight - titleBackgroundHeight)/2.0f) - ((0.5f * titleBackgroundHeight) - 10.0f),
                                              titleBackgroundWidth, titleBackgroundHeight);
    m_uiLandscapeTitleMessageFont->AlignCenter();

    /*
     * Landscape messagebox
     */
    m_uiLandscapeTextMessage = new hdFontPolygon(LEVEL_CONTROLLER_MESSAGE_FONT, NULL);
    m_uiLandscapeTextMessage->SetTint(0.0f, 0.0f, 0.0f, 1.0f);
    m_uiLandscapeTextMessage->SetScale(0.75f);
    m_uiLandscapeTextMessage->SetAs2DBox(200.0f-m_PixelScreenWidth, 75.0f, 175.0f, 173.0f);
    m_uiLandscapeTextMessage->AlignLeft();

    m_uiLandscapeTextMessageBackground = new hdUIImage(NULL, NULL);
    m_uiLandscapeTextMessageBackground->SetAs2DBox(((m_PixelScreenWidth-240.0f)/2.0f), 60.0f, 240.0f, 240.0f);

    // Avatar message support
    m_uiLandscapeAvatar = new hdUIImage("", NULL);
    m_uiLandscapeAvatar->SetAs2DBox(30.0f, m_PixelScreenHeight - (gapDistance + avatarSquareSize),
                                    avatarSquareSize, avatarSquareSize);
    m_uiLandscapeAvatar->Hide();

    m_uiLandscapeAvatarChrome = new hdUIImage(NULL, NULL);
    m_uiLandscapeAvatarChrome->SetTint(0.0f, 0.0f, 0.0f, 0.8f);
    m_uiLandscapeAvatarChrome->SetAs2DBox(20.0f, m_PixelScreenHeight - (gapDistance + 10.0f + avatarSquareSize),
                                          (avatarSquareSize + 20.0f), (avatarSquareSize + 20.0f));
    m_uiLandscapeAvatarChrome->Hide();

    m_uiLandscapeAvatarMessageText = new hdFontPolygon(LEVEL_CONTROLLER_MESSAGE_FONT, NULL);
    m_uiLandscapeAvatarMessageText->SetTint(0.1f, 0.1f, 0.1f, 1.0f);
    m_uiLandscapeAvatarMessageText->SetAs2DBox(avatarSquareSize + (0.15f * m_PixelScreenWidth), 28.0f, //m_PixelScreenHeight - (gapDistance+40.0f),
                                               m_PixelScreenWidth - (avatarSquareSize + (0.35 * m_PixelScreenWidth)), m_PixelScreenHeight - (gapDistance+40.0f));
    m_uiLandscapeAvatarMessageText->SetScale(0.8f);
    m_uiLandscapeAvatarMessageText->AlignLeft();
    m_uiLandscapeAvatarMessageText->Hide();

    m_uiLandscapeAvatarMessageBackground = new hdUIImage("Interface/Textures/imgLandscapeMessageBackground512.png", NULL);
    m_uiLandscapeAvatarMessageBackground->SetTint(1.0f, 1.0f, 1.0f, 1.0f);
    m_uiLandscapeAvatarMessageBackground->SetAs2DBox((avatarSquareSize + 40.0f), gapDistance,
                                                     m_PixelScreenWidth - (avatarSquareSize + 70.0f), m_PixelScreenHeight - 100.0f);
    m_uiLandscapeAvatarMessageBackground->Hide();

    m_uiLandscapeAvatarMessageChrome = new hdUIImage(NULL, NULL);
    m_uiLandscapeAvatarMessageChrome->SetTint(0.0f, 0.0f, 0.0f, 0.0f);
    m_uiLandscapeAvatarMessageChrome->SetAs2DBox((avatarSquareSize + 40.0f), gapDistance,
                                                 m_PixelScreenWidth - (10.0f + (avatarSquareSize + 60.0f)), m_PixelScreenHeight - 100.0f);
    m_uiLandscapeAvatarMessageChrome->Hide();

    /*
     * Landscape Buttons
     */
    float msgButtonWidth = (0.12 * pixelScreenWidth);
    m_uiLandscapeMessageOKButton  = new hdButton("/Interface/Textures/btnOkNormal.png",
                                                 "/Interface/Textures/btnOkOver.png",
                                                 "/Interface/Textures/btnOkOver.png", NULL);
    m_uiLandscapeMessageOKButton->SetAs2DBox(((m_PixelScreenWidth-msgButtonWidth)/2.0f)-m_PixelScreenWidth, 5.0f,
                                             msgButtonWidth, miscButtonHeight);

    m_uiLandscapeMessageDoneButton  = new hdButton("/Interface/Textures/btnDoneLrgNormal.png",
                                                   "/Interface/Textures/btnDoneLrgOver.png",
                                                   "/Interface/Textures/btnDoneLrgOver.png", NULL);
    m_uiLandscapeMessageDoneButton->SetAs2DBox((m_PixelScreenWidth/2.0f)+10.0f-m_PixelScreenWidth, 5.0f,
                                               msgButtonWidth, miscButtonHeight);

    m_uiLandscapeMessagePrevButton  = new hdButton("/Interface/Textures/btnPrevLrgNormal.png",
                                                   "/Interface/Textures/btnPrevLrgOver.png",
                                                   "/Interface/Textures/btnPrevLrgOver.png", NULL);
    m_uiLandscapeMessagePrevButton->SetAs2DBox((m_PixelScreenWidth/2.0f)-(10.0f + msgButtonWidth)-m_PixelScreenWidth, 5.0f,
                                               msgButtonWidth, miscButtonHeight);

    m_uiLandscapeMessageNextButton  = new hdButton("/Interface/Textures/btnNextLrgNormal.png",
                                                   "/Interface/Textures/btnNextLrgOver.png",
                                                   "/Interface/Textures/btnNextLrgOver.png", NULL);
    m_uiLandscapeMessageNextButton->SetAs2DBox((m_PixelScreenWidth/2.0f)+10.0f-m_PixelScreenWidth, 5.0f,
                                               msgButtonWidth, miscButtonHeight);

    m_uiLandscapeMessageSkipButton = new hdButton("/Interface/Textures/btnSkipNormal.png",
                                                  "/Interface/Textures/btnSkipOver.png",
                                                  "/Interface/Textures/btnSkipOver.png", NULL);
    m_uiLandscapeMessageSkipButton->SetAs2DBox((m_PixelScreenWidth-(10.0f + msgButtonWidth))-m_PixelScreenWidth, 5.0f,
                                               0.8f * msgButtonWidth, miscButtonHeight);

    m_uiLandscapeMessageContainer = new hdUIContainer(NULL, hdVec3(-m_PixelScreenWidth, 0.0f, 0.0f),
                                                      hdVec3(0.0f, m_PixelScreenHeight, 0.0f));

    m_uiLandscapeMessageContainer->Add(m_uiLandscapeTextMessage);
    m_uiLandscapeMessageContainer->Add(m_uiLandscapeTitleMessageBackground);
    m_uiLandscapeMessageContainer->Add(m_uiLandscapeTitleMessageFont);
    m_uiLandscapeMessageContainer->Add(m_uiLandscapeAvatarChrome);
    m_uiLandscapeMessageContainer->Add(m_uiLandscapeAvatarMessageChrome);
    m_uiLandscapeMessageContainer->Add(m_uiLandscapeAvatarMessageBackground);
    m_uiLandscapeMessageContainer->Add(m_uiLandscapeAvatar);
    m_uiLandscapeMessageContainer->Add(m_uiLandscapeAvatarMessageText);

    m_uiLandscapeMessageContainer->Translate(hdVec3(-m_PixelScreenWidth, 0.0f, 0.0f), m_uiLandscapeMessageContainer->GetWorldCenter());

    m_uiLandscapeMessageButtonContainer = new hdUIContainer(NULL, hdVec3(-m_PixelScreenWidth, 0.0f, 0.0f),
                                                            hdVec3(0.0f, m_PixelScreenHeight, 0.0f));
    m_uiLandscapeMessageButtonContainer->Add(m_uiLandscapeMessageOKButton);
    m_uiLandscapeMessageButtonContainer->Add(m_uiLandscapeMessageDoneButton);
    m_uiLandscapeMessageButtonContainer->Add(m_uiLandscapeMessageSkipButton);
    m_uiLandscapeMessageButtonContainer->Add(m_uiLandscapeMessagePrevButton);
    m_uiLandscapeMessageButtonContainer->Add(m_uiLandscapeMessageNextButton);

    /*
     * Portrait messagebox
     */
    m_uiPortraitTitleMessageBackground = new hdUIImage("/Interface/Textures/imgTitleBackground.png", NULL);
    m_uiPortraitTitleMessageBackground->SetAs2DBox((m_PixelScreenWidth - titleBackgroundWidth)/2.0f,
                                                   (m_PixelScreenHeight - titleBackgroundHeight)/2.0f,
                                                   titleBackgroundWidth, titleBackgroundHeight);

    m_uiPortraitTitleMessageFont = new hdFontPolygon(LEVEL_CONTROLLER_TITLE_FONT, NULL);
    m_uiPortraitTitleMessageFont->SetTint(0.0f, 0.0f, 0.0f, 1.0f);
    m_uiPortraitTitleMessageFont->SetAs2DBox((m_PixelScreenWidth - titleBackgroundWidth)/2.0f,
                                             (m_PixelScreenHeight - titleBackgroundHeight)/2.0f,
                                             titleBackgroundWidth, titleBackgroundHeight);
    m_uiPortraitTitleMessageFont->AlignCenter();


    m_uiPortraitTextMessage = new hdFontPolygon(LEVEL_CONTROLLER_MESSAGE_FONT, NULL);
    m_uiPortraitTextMessage->SetTint(0.0f, 0.0f, 0.0f, 1.0f);
    m_uiPortraitTextMessage->SetAs2DBox(30.0f, 140.0f, 200.0f, 130.0f);
    m_uiPortraitTextMessage->AlignLeft();

    m_uiPortraitTextMessageBackground = new hdUIImage(NULL, NULL);
    m_uiPortraitTextMessageBackground->SetAs2DBox(((m_PixelScreenHeight-256.0f)/2.0f), 80.0f, 256.0f, 256.0f);

    m_uiPortraitAvatar = new hdUIImage("", NULL);
    m_uiPortraitAvatar->SetAs2DBox(20.0f, m_PixelScreenWidth - (60.0f + 128.0f), 128.0f, 128.0f);
    m_uiPortraitAvatar->Hide();

    m_uiPortraitAvatarChrome = new hdUIImage(NULL, NULL);
    m_uiPortraitAvatarChrome->SetTint(0.0f, 0.0f, 0.0f, 0.7f);
    m_uiPortraitAvatarChrome->SetAs2DBox(10.0f, m_PixelScreenWidth - (50.0f + 148.0f), 148.0f, 148.0f);
    m_uiPortraitAvatarChrome->Hide();

    m_uiPortraitAvatarMessageText = new hdFontPolygon(LEVEL_CONTROLLER_MESSAGE_FONT, NULL);
    m_uiPortraitAvatarMessageText->SetTint(0.0f, 0.0f, 0.0f, 1.0f);
    m_uiPortraitAvatarMessageText->SetAs2DBox(30.0f, 80.0f, 200.0f, 180.0f);
    m_uiPortraitAvatarMessageText->AlignLeft();
    m_uiPortraitAvatarMessageText->SetScale(0.8f);
    m_uiPortraitAvatarMessageText->Hide();

    m_uiPortraitAvatarMessageChrome = new hdUIImage(NULL, NULL);
    m_uiPortraitAvatarMessageChrome->SetTint(0.0f, 0.0f, 0.0f, 0.0f);
    m_uiPortraitAvatarMessageChrome->SetAs2DBox(10.0f, 70.0f, m_PixelScreenHeight - 20.0f, m_PixelScreenWidth - (50.0f + 148.0f + 70.0f + 10.0f));
    m_uiPortraitAvatarMessageChrome->Hide();

    m_uiPortraitAvatarMessageBackground = new hdUIImage("Interface/Textures/imgPortraitMessageBackground.png", NULL);
    m_uiPortraitAvatarMessageBackground->SetTint(1.0f, 1.0f, 1.0f, 1.0f);
    m_uiPortraitAvatarMessageBackground->SetAs2DBox(10.0f, 70.0f, m_PixelScreenHeight - 20.0f, m_PixelScreenWidth - (50.0f + 128.0f + 70.0f + 10.0f));
    m_uiPortraitAvatarMessageBackground->Hide();

    /*
     * Portrain Message buttons
     */
    m_uiPortraitMessageOKButton  = new hdButton("/Interface/Textures/btnOkNormal.png",
                                                "/Interface/Textures/btnOkOver.png",
                                                "/Interface/Textures/btnOkOver.png", NULL);
    m_uiPortraitMessageOKButton->SetAs2DBox(((m_PixelScreenHeight-64.0f)/2.0f)-m_PixelScreenHeight, 20.0f, 64.0f, 40.0f);


    m_uiPortraitMessageDoneButton  = new hdButton("/Interface/Textures/btnDoneNormal.png",
                                                  "/Interface/Textures/btnDoneOver.png",
                                                  "/Interface/Textures/btnDoneOver.png", NULL);
    m_uiPortraitMessageDoneButton->SetAs2DBox((m_PixelScreenHeight/2.0f)+10.0f-m_PixelScreenHeight, 20.0f, 64.0f, 40.0f);

    m_uiPortraitMessagePrevButton  = new hdButton("/Interface/Textures/btnPrevNormal.png",
                                                  "/Interface/Textures/btnPrevOver.png",
                                                  "/Interface/Textures/btnPrevOver.png", NULL);
    m_uiPortraitMessagePrevButton->SetAs2DBox((m_PixelScreenHeight/2.0f)-74.0f-m_PixelScreenHeight, 20.0f, 64.0f, 40.0f);

    m_uiPortraitMessageNextButton  = new hdButton("/Interface/Textures/btnNextNormal.png",
                                                  "/Interface/Textures/btnNextOver.png",
                                                  "/Interface/Textures/btnNextOver.png", NULL);
    m_uiPortraitMessageNextButton->SetAs2DBox((m_PixelScreenHeight/2.0f)+10.0f-m_PixelScreenHeight, 20.0f, 64.0f, 40.0f);


    m_uiPortraitMessageSkipButton = new hdButton("/Interface/Textures/btnSkipSmallNormal.png",
                                                 "/Interface/Textures/btnSkipSmallOver.png",
                                                 "/Interface/Textures/btnSkipSmallOver.png", NULL);
    m_uiPortraitMessageSkipButton->SetAs2DBox((m_PixelScreenHeight-74.0f)-m_PixelScreenHeight, 20.0f, 48.0f, 30.0f);


    m_uiPortraitMessageContainer = new hdUIContainer(NULL, hdVec3(-m_PixelScreenHeight, 0.0f, 0.0f),
                                                     hdVec3(0.0f, m_PixelScreenWidth, 0.0f));
    m_uiPortraitMessageContainer->Hide();

    m_uiPortraitMessageContainer->Add(m_uiPortraitTextMessage);

    m_uiPortraitMessageContainer->Add(m_uiPortraitTitleMessageBackground);
    m_uiPortraitMessageContainer->Add(m_uiPortraitTitleMessageFont);
    m_uiPortraitMessageContainer->Add(m_uiPortraitAvatarChrome);
    m_uiPortraitMessageContainer->Add(m_uiPortraitAvatar);
    m_uiPortraitMessageContainer->Add(m_uiPortraitAvatarMessageChrome);
    m_uiPortraitMessageContainer->Add(m_uiPortraitAvatarMessageBackground);
    m_uiPortraitMessageContainer->Add(m_uiPortraitAvatarMessageText);
    m_uiPortraitMessageContainer->Translate(hdVec3(-m_PixelScreenHeight, 0.0f, 0.0f), m_uiPortraitMessageContainer->GetWorldCenter());

    m_uiPortraitMessageButtonContainer = new hdUIContainer(NULL, hdVec3(-m_PixelScreenHeight, 0.0f, 0.0f),
                                                           hdVec3(0.0f, m_PixelScreenWidth, 0.0f));
    m_uiPortraitMessageButtonContainer->Add(m_uiPortraitMessageOKButton);
    m_uiPortraitMessageButtonContainer->Add(m_uiPortraitMessageSkipButton);
    m_uiPortraitMessageButtonContainer->Add(m_uiPortraitMessagePrevButton);
    m_uiPortraitMessageButtonContainer->Add(m_uiPortraitMessageNextButton);
    m_uiPortraitMessageButtonContainer->Add(m_uiPortraitMessageDoneButton);

    /* Landscape */
    m_uiLandscapeMessageOKButton->SetMouseDownListener(this, AppInterface::GenericButtonDownCallback);
    m_uiLandscapeMessageOKButton->SetMouseUpListener(this, AppInterface::HideMessageClickCallback);

    m_uiLandscapeMessageDoneButton->SetMouseDownListener(this, AppInterface::GenericButtonDownCallback);
    m_uiLandscapeMessageDoneButton->SetMouseUpListener(this, AppInterface::HideMessageClickCallback);

    m_uiLandscapeMessageSkipButton->SetMouseDownListener(this, AppInterface::GenericButtonDownCallback);
    m_uiLandscapeMessageSkipButton->SetMouseUpListener(this, AppInterface::SkipUpCallback);

    m_uiLandscapeMessagePrevButton->SetMouseDownListener(this, AppInterface::GenericButtonDownCallback);
    m_uiLandscapeMessagePrevButton->SetMouseUpListener(this, AppInterface::PrevUpCallback);

    m_uiLandscapeMessageNextButton->SetMouseDownListener(this, AppInterface::GenericButtonDownCallback);
    m_uiLandscapeMessageNextButton->SetMouseUpListener(this, AppInterface::HideMessageClickCallback);

#endif
    m_glow = new hdUIImage("Materials/s_glow_15.tga", NULL, 0.0f, 0.0f, 10.0f, 10.0f);

    /*
     * Message queue container button
     */
    m_uiMessageQueueContainer = new hdUIContainer(NULL, hdVec3(-m_PixelScreenWidth, 0.0f, 0.0f),
                                                  hdVec3(0.0f, m_PixelScreenHeight, 0.0f));

    m_uiMessageQueueButton = new hdButton("/Interface/Textures/btnMessageQueueNormal.png",
                                          "/Interface/Textures/btnMessageQueueOver.png",
                                          "/Interface/Textures/btnMessageQueueOver.png", NULL);
    m_uiMessageQueueButton->Hide();
    m_uiMessageQueueCount = new hdFontPolygon(LEVEL_CONTROLLER_TINY_FONT, NULL);
    m_uiMessageQueueCount->SetTint(1.0f, 0.75f, 0.0f, 1.0f);
    m_uiMessageQueueCount->AlignLeft();
    m_uiMessageQueueCount->Hide();

    m_uiMessageQueueContainer->Add(m_uiMessageQueueButton);
    m_uiMessageQueueContainer->Add(m_uiMessageQueueCount);

    /* Message Queue Handlers */
    m_uiMessageQueueButton->SetMouseDownListener(this, AppInterface::GenericButtonDownCallback);
    m_uiMessageQueueButton->SetMouseUpListener(this, AppInterface::MessageQueueUpClicked);
}


void AppInterface::RefreshLayout()
{
    m_uiMessageQueueButton->SetAs2DBox(5.0f, 5.0f, 45.0f, 45.0f);
    m_uiMessageQueueCount->SetAs2DBox(38.0f, 17.0f, 14.0f, 14.0f);

    if (IsLandscapeOrientation())
    {
        m_uiTextMessage = m_uiLandscapeTextMessage;
        m_uiTextMessageBackground = m_uiLandscapeTextMessageBackground;

        m_OkButton = m_uiLandscapeMessageOKButton;
        m_DoneButton = m_uiLandscapeMessageDoneButton;
        m_SkipButton = m_uiLandscapeMessageSkipButton;
        m_PrevButton = m_uiLandscapeMessagePrevButton;
        m_NextButton = m_uiLandscapeMessageNextButton;

        m_uiMessageContainer = m_uiLandscapeMessageContainer;
        m_uiMessageButtonContainer = m_uiLandscapeMessageButtonContainer;

        m_prefsView->RefreshLayout(GetOrientation());
    }
    else
    {
        m_uiTextMessage = m_uiPortraitTextMessage;
        m_uiTextMessageBackground = m_uiPortraitTextMessageBackground;

        m_OkButton = m_uiPortraitMessageOKButton;
        m_DoneButton = m_uiPortraitMessageDoneButton;
        m_SkipButton = m_uiPortraitMessageSkipButton;
        m_PrevButton = m_uiPortraitMessagePrevButton;
        m_NextButton = m_uiPortraitMessageNextButton;

        m_uiMessageContainer = m_uiPortraitMessageContainer;
        m_uiMessageButtonContainer = m_uiPortraitMessageButtonContainer;

        m_prefsView->RefreshLayout(e_hdInterfaceOrientationPortrait);
    }

    m_OkButton->Show();
    m_SkipButton->Hide();
    m_PrevButton->Hide();
    m_NextButton->Hide();
    m_DoneButton->Hide();
}


void AppInterface::ShowPreferences()
{
    m_prefsView->Show();
    m_prefsView->Enable();
    m_interactionState = e_interactionDisabled;
}


void AppInterface::ClosePreferencesCallback(void *handler, void *sender)
{
    AppInterface *self = (AppInterface *)handler;
    self->HidePreferences();
}


void AppInterface::HidePreferences()
{
    m_interactionState = e_waitingForInput;
    m_prefsView->Disable();
    m_prefsView->Hide();
    this->ResetOrientation();
}


void AppInterface::DrawInternalInterface()
{
    m_uiMessageQueueContainer->Draw();

    m_prefsView->Draw();

    if (IsLandscapeOrientation())
    {
        m_uiLandscapeMessageContainer->Draw();
        m_uiLandscapeMessageButtonContainer->Draw();
    }
    else
    {
        m_uiPortraitMessageContainer->Draw();
        m_uiPortraitMessageButtonContainer->Draw();
    }
}


void AppInterface::HideMessageClickCallback(void *handler, void *sender)
{
    AppInterface *self = (AppInterface *)handler;
    if (!self) return;

    hdSoundManager::PlaySound(self->m_btnMessageClickUpSound);
    self->HideMessage();
}


void AppInterface::SkipUpCallback(void *handler, void *sender)
{
    AppInterface *self = (AppInterface *)handler;
    if (!self) return;

    self->m_messageCount = 0;
    hdSoundManager::PlaySound(self->m_btnMessageClickUpSound);
    self->HideMessage();
}


void AppInterface::PrevUpCallback(void *handler, void *sender)
{
    AppInterface *self = (AppInterface *)handler;
    if (!self) return;

    if (self->m_currentMessageIndex >= 2)
    {
        hdSoundManager::PlaySound(self->m_btnMessageClickUpSound);
        self->m_currentMessageIndex = hdMax(0, self->m_currentMessageIndex-2); // because the index is incremented
        self->HideMessage();
    }
}


void AppInterface::MessageQueueUpClicked(void *handler, void *sender)
{
    AppInterface *self = (AppInterface *)handler;
    if (!self) return;

    self->FindAndShowQueueMessage();

}


void AppInterface::GenericButtonDownCallback(void *handler, void *sender)
{
    AppInterface *self = (AppInterface *)handler;
    if (!self) return;

    hdSoundManager::PlaySound(self->m_btnMessageClickDownSound);
}


void AppInterface::FindAndShowQueueMessage()
{
    totemMessage msg;

    hdSoundManager::PlaySound(m_btnMessageClickUpSound);

    hdAssert (m_messageTagQueue->GetItemCount() > 0);

    msg = m_messageTagQueue->GetItems()[0];
    if (msg.levelId == -1)
    {
        LoadScriptMessageForTag(msg.targetId, false);
    }
    else
    {
        LoadScriptMessageForTagAndLevel(msg.targetId, msg.levelId, false);
    }
    m_messageTagQueue->RemoveAtIndex(0);

    if (m_messageTagQueue->IsEmpty())
    {
        m_uiMessageQueueButton->Hide();
        m_uiMessageQueueCount->Hide();
    }
    else
    {
        m_uiMessageQueueCount->SetTextFormatted("%d", m_messageTagQueue->GetItemCount());
    }
}


void AppInterface::ShowMessage()
{
    if (m_uiLandscapeMessageContainer->IsVisible() &&
        m_uiPortraitMessageContainer->IsVisible()) return;

    m_uiMessageQueueContainer->Hide();

    ShowMessageInterface(m_uiLandscapeMessageContainer, m_uiLandscapeMessageButtonContainer);
    ShowMessageInterface(m_uiPortraitMessageContainer, m_uiPortraitMessageButtonContainer);

    Game_ShowMessage();
}


void AppInterface::HideMessage()
{
    if (m_uiLandscapeMessageContainer->IsHidden() &&
        m_uiPortraitMessageContainer->IsHidden()) return;

    hdAnimation* anim = hdAnimationController::CreateAnimation(this);

    hdPauseAction *action = new hdPauseAction();
    action->SetDuration(0.3f);

    anim->AddGameObject(m_uiLandscapeMessageContainer);
    anim->AddAction(action);
    anim->SetFinishedCallback(this, AppInterface::HideMessageCallback);
    anim->StartAnimation();

    HideMessageInterface(m_uiLandscapeMessageContainer, m_uiLandscapeMessageButtonContainer);
}


void AppInterface::ShowMessageInterface(hdUIContainer *container, hdUIContainer *buttonContainer)
{
    hdAnimation* anim1 = hdAnimationController::CreateAnimation(this);
    hdAnimation* anim2 = hdAnimationController::CreateAnimation(this);

    hdVectorAction *action1 = new hdVectorAction();
    action1->SetDuration(0.15f);
    action1->SetDestination(hdVec3(container->GetWorldCenter().x + m_PixelScreenWidth,
                                   container->GetWorldCenter().y,
                                   container->GetWorldCenter().z));

    anim1->AddGameObject(container);
    anim1->AddAction(action1);
    anim1->StartAnimation();

    hdVectorAction *action2 = new hdVectorAction();
    action2->SetDuration(0.25f);
    action2->SetDestination(hdVec3(buttonContainer->GetWorldCenter().x + m_PixelScreenWidth,
                                   buttonContainer->GetWorldCenter().y,
                                   buttonContainer->GetWorldCenter().z));

    anim2->AddGameObject(buttonContainer);
    anim2->AddAction(action2);
    anim2->StartAnimation();
}


void AppInterface::HideMessageInterface(hdUIContainer *container, hdUIContainer *buttonContainer)
{
    hdAnimation* anim1 = hdAnimationController::CreateAnimation(this);
    hdAnimation* anim2 = hdAnimationController::CreateAnimation(this);

    hdVectorAction *action1 = new hdVectorAction();
    action1->SetDuration(0.25f);
    action1->SetDestination(hdVec3(container->GetWorldCenter().x - m_PixelScreenWidth,
                                   container->GetWorldCenter().y,
                                   container->GetWorldCenter().z));

    anim1->AddGameObject(container);
    anim1->AddAction(action1);
    anim1->StartAnimation();

    hdVectorAction *action2 = new hdVectorAction();
    action2->SetDuration(0.15f);
    action2->SetDestination(hdVec3(buttonContainer->GetWorldCenter().x - m_PixelScreenWidth,
                                   buttonContainer->GetWorldCenter().y,
                                   buttonContainer->GetWorldCenter().z));

    anim2->AddGameObject(buttonContainer);
    anim2->AddAction(action2);
    anim2->StartAnimation();
}


void AppInterface::HideMessageCallback(void *handler, hdAnimation *anim)
{
    if (handler == NULL || anim == NULL)
    {
        hdError(-1, "Fucking animation and handler have gone away in between anim starting and anim stopping.");
        return;
    }

    AppInterface *self = (AppInterface *)handler;

    self->HideMessageHandleCallback();
}


void AppInterface::HideMessageHandleCallback()
{
    m_uiMessageQueueContainer->Show();
    Game_HideMessage();
    ShowNextScriptMessage();
}


void AppInterface::SetMessageTexture(const char *texturePrefix)
{
    m_uiLandscapeTextMessageBackground->SetTexture(hdTextureManager::Instance()->FindTexture(texturePrefix, TT_Wall));
    m_uiLandscapeTextMessageBackground->Show();

    m_uiPortraitTextMessageBackground->SetTexture(hdTextureManager::Instance()->FindTexture(texturePrefix, TT_Wall));
    m_uiPortraitTextMessageBackground->Show();
}


void AppInterface::SetMessage(const hdTexture *texture)
{
    m_uiLandscapeTextMessageBackground->SetTexture(texture);
    m_uiLandscapeTextMessageBackground->Show();

    m_uiPortraitTextMessageBackground->SetTexture(texture);
    m_uiPortraitTextMessageBackground->Show();

    m_uiLandscapeTextMessage->Hide();
    m_uiPortraitTextMessage->Hide();
}


void AppInterface::SetMessage(const totemMessage *msg)
{
    m_uiLandscapeAvatar->Hide();
    m_uiLandscapeAvatarChrome->Hide();
    m_uiPortraitAvatar->Hide();
    m_uiPortraitAvatarChrome->Hide();

    m_uiLandscapeTextMessage->Hide();
    m_uiLandscapeTextMessageBackground->Hide();

    m_uiPortraitTextMessage->Hide();
    m_uiPortraitTextMessageBackground->Hide();

    m_uiLandscapeAvatarMessageText->Hide();
    m_uiLandscapeAvatarMessageChrome->Hide();
    m_uiLandscapeAvatarMessageBackground->Hide();
    m_uiPortraitAvatarMessageText->Hide();
    m_uiPortraitAvatarMessageChrome->Hide();
    m_uiPortraitAvatarMessageBackground->Hide();
    
    m_uiLandscapeTitleMessageBackground->Hide();
    m_uiLandscapeTitleMessageFont->Hide();
    m_uiPortraitTitleMessageBackground->Hide();
    m_uiPortraitTitleMessageFont->Hide();
    
    if (msg->messageType == e_hdkMessageTypeImage)
    { 
        // Assume text is a texture o_O
        SetMessageTexture(msg->texture);
        
        m_uiLandscapeTextMessage->Hide();
        
        m_uiPortraitTextMessage->Hide();
    }
    else if (msg->messageType == e_hdkMessageTypeTitle)
    {
        m_uiLandscapeTitleMessageFont->SetText(msg->message);
        m_uiLandscapeTitleMessageFont->Show();
        m_uiLandscapeTitleMessageBackground->Show();
        
        m_uiPortraitTitleMessageFont->SetText(msg->message);
        m_uiPortraitTitleMessageFont->Show();
        m_uiPortraitTitleMessageBackground->Show();
    }
    else if (msg->messageType == e_hdkMessageTypeText)
    {
        SetMessageTexture("Interface/Textures/imgScriptMessage");
        
        m_uiLandscapeTextMessage->SetText(msg->message);
        m_uiLandscapeTextMessage->Show();
        
        m_uiPortraitTextMessage->SetText(msg->message);
        m_uiPortraitTextMessage->Show();
    }
    else if (msg->messageType == e_hdkMessageTypeCustomImageText)
    {
        SetMessageTexture(msg->texture);
        
        m_uiLandscapeTextMessage->SetText(msg->message);
        m_uiLandscapeTextMessage->Show();
        
        m_uiPortraitTextMessage->SetText(msg->message);
        m_uiPortraitTextMessage->Show();
    }
    else if (msg->messageType == e_hdkMessageTypeAvatar)
    {
        m_uiLandscapeAvatar->SetTexture(hdTextureManager::Instance()->FindTexture(msg->texture, TT_Wall));
        m_uiLandscapeAvatar->Show();
        m_uiLandscapeAvatarChrome->Show();
        
        m_uiPortraitAvatar->SetTexture(hdTextureManager::Instance()->FindTexture(msg->texture, TT_Wall));
        m_uiPortraitAvatar->Show();
        m_uiPortraitAvatarChrome->Show();
        
        m_uiLandscapeAvatarMessageText->SetText(msg->message);
        m_uiLandscapeAvatarMessageText->Show();
        m_uiLandscapeAvatarMessageChrome->Show();
        m_uiLandscapeAvatarMessageBackground->Show();
        
        m_uiPortraitAvatarMessageText->SetText(msg->message);
        m_uiPortraitAvatarMessageText->Show();
        m_uiPortraitAvatarMessageChrome->Show();
        m_uiPortraitAvatarMessageBackground->Show();
    }
}


const bool AppInterface::IsShowingMessage() const
{
    if (m_uiMessageContainer == NULL) return false;
    return m_uiLandscapeMessageContainer->IsVisible() && m_uiPortraitMessageContainer->IsVisible();
}


void AppInterface::LoadScriptMessageForTag(const int tag)
{
    LoadScriptMessageForTag(tag, false);
}


void AppInterface::LoadScriptMessageForTag(const int tag, const bool addToQueue)
{
    LoadScriptMessageForTagAndLevel(tag, m_HACKHACKHACK_currentLevelId, addToQueue);
}


void AppInterface::LoadScriptMessageForTagAndLevel(const int tag, const int levelId,  const bool addToQueue)
{
    int msgCount;
    totemMessage t;
    
    if (addToQueue || IsShowingMessage())
    {
        t.levelId = levelId;
        t.targetId = tag;
        
        // Actually any messages???
        msgCount = Scripts_CountLevelMessagesForTag(levelId, tag);
        
        if (msgCount > 0)
        {
            if (-1 == m_messageTagQueue->Add(t))
            {
                hdPrintf("Message Queue is full!\n");
            }
            
            m_uiMessageQueueCount->SetTextFormatted("%d", m_messageTagQueue->GetItemCount());
            m_uiMessageQueueButton->Show();
            m_uiMessageQueueCount->Show();
        }
    }
    else
    {
        m_currentMessageIndex = 0;
        m_messageCount = Scripts_GetLevelMessagesForTag(m_scriptMessages, 10, levelId, tag);
        ShowNextScriptMessage();
    }
}


void AppInterface::LoadScriptMessageForTagAndLevel(const int tag, const int levelId, const bool addToQueue, const bool noShow)
{
    m_currentMessageIndex = 0;
    m_messageCount = Scripts_GetLevelMessagesForTag(m_scriptMessages, 10, levelId, tag);
}


const bool AppInterface::ShowNextScriptMessage()
{
    m_OkButton->Hide();
    m_PrevButton->Enable();
    if (m_messageCount <= 1)
    {
        m_SkipButton->Hide();
        m_PrevButton->Hide();
        m_NextButton->Hide();
        m_DoneButton->Hide();
        m_OkButton->Show();
    }
    else
    {
        m_SkipButton->Show();
        if (m_currentMessageIndex == 0)
        {
            m_PrevButton->Disable();
            m_PrevButton->Show();
            m_NextButton->Show();
            m_DoneButton->Hide();
        }
        else if (m_currentMessageIndex == (m_messageCount - 1))
        {
            m_PrevButton->Show();
            m_NextButton->Hide();
            m_DoneButton->Show();
        }
        else
        {
            m_PrevButton->Show();
            m_NextButton->Show();
            m_DoneButton->Hide();
        }
    }
    
    if (m_messageTagQueue->GetItemCount() == 0)
    {
        m_uiMessageQueueButton->Hide();
        m_uiMessageQueueCount->Hide();
    }	
    
    if (m_messageCount > 0 &&
        m_currentMessageIndex < m_messageCount)
    {
        SetMessage(m_scriptMessages[m_currentMessageIndex]);
        m_interactionState = e_interactionDisabled;
        ShowMessage();
        ++m_currentMessageIndex;
        return true;
    }
    
    m_OkButton->Show();
    m_SkipButton->Hide();
    m_PrevButton->Hide();
    m_NextButton->Hide();
    m_DoneButton->Hide();
    
    return false;
}


void AppInterface::Game_ShowMessage() {}


void AppInterface::Game_HideMessage() {}
