/*
 * Copyright (c) 2017 Hackdirt Ltd.
 * Author: David Petrie (david@davidpetrie.com)
 *
 * License: https://www.gnu.org/licenses/gpl-3.0.en.html
 */

#include "GameController.h"

void GameController::InitAnimations()
{
    m_fadeAction = new hdAlphaAction();
    m_fadeAction->SetDuration(0.8f);
    m_fadeAction->SetStartAlpha(0.0f);
    m_fadeAction->SetEndAlpha(1.0f);

    m_fadeAnim = hdAnimationController::CreateAnimation(this, false);
    m_fadeAnim->AddGameObject(m_fadeAction->GetAlphaObject());
    m_fadeAnim->AddAction(m_fadeAction);

    // quickly scale the projection up.
    m_levelStartZoomProjectionAction = new hdFunctionAction<GameController>();
    m_levelStartZoomProjectionAction->SetDuration(1.0f);
    m_levelStartZoomProjectionAction->SetFunctionObject(this);
    m_levelStartZoomProjectionAction->SetFunction(&GameController::IntroZoomProjection);

    m_levelStartPanProjectionAction = new hdFunctionAction<GameController>();
    m_levelStartPanProjectionAction->SetDuration(1500.0f);
    m_levelStartPanProjectionAction->SetFunctionObject(this);
    m_levelStartPanProjectionAction->SetFunction(&GameController::IntroPanProjection);

    m_levelStartProjectionAnimation = hdAnimationController::CreateAnimation(this, false);
    m_levelStartProjectionAnimation->AddGameObject(m_projection);
    m_levelStartProjectionAnimation->AddAction(m_levelStartPanProjectionAction);
    m_levelStartProjectionAnimation->AddAction(m_levelStartZoomProjectionAction);

    m_playerReadyProjectionAction = new hdFunctionAction<GameController>();
    m_playerReadyProjectionAction->SetDuration(2.0f);
    m_playerReadyProjectionAction->SetFunctionObject(this);
    m_playerReadyProjectionAction->SetFunction(&GameController::PlayerReadyMoveProjection);

    m_playerReadyProjectionAnimation = hdAnimationController::CreateAnimation(this, false);
    m_playerReadyProjectionAnimation->AddGameObject(m_projection);
    m_playerReadyProjectionAnimation->AddAction(m_playerReadyProjectionAction);

    m_animPolygons = new hdPointerList<hdReceiver, kAnimationPoolCapacity>();
    m_gameWorldAnimPolygons = new hdPointerList<hdReceiver, kAnimationPoolCapacity>();
    m_notificationPolygons = new hdPointerList<Polygon, kAnimationPoolCapacity>();

    isAnimatingMidairChain = false;
    isAnimatingLostMessage = false;

    m_goalAnim = NULL;
    m_goalFadeAnim = NULL;
    m_goalTextAnim = NULL;
    m_showFinishButtonAnim = NULL;
    m_cameraShakeAnimation = NULL;

    m_completionSlideAnim = NULL;
    m_completionFadeAnim = NULL;

    m_midAirNoteAnim = NULL;
    m_fragmentsBoxAnim = NULL;
    m_lostMessageBoxAnim = NULL;
    m_failRestartAnim = NULL;
}



hdAnimation* GameController::AnimateFadeInOut(hdGameObject *obj,
                                              const float fadeInDuration,
                                              const float pauseDuration,
                                              const float fadeOutDuration)
{
    hdAnimation* fadeAnim = hdAnimationController::CreateAnimation(this);

    hdAlphaAction* fadein = new hdAlphaAction();
    fadein->SetDuration(fadeInDuration);
    fadein->SetStartAlpha(0.0f);
    fadein->SetEndAlpha(1.0f);

    hdAlphaAction* fadeout = new hdAlphaAction();
    fadeout->SetDuration(fadeOutDuration);
    fadeout->SetStartAlpha(1.0f);
    fadeout->SetEndAlpha(0.0f);

    hdPauseAction* pauseAction = new hdPauseAction();
    pauseAction->SetDuration(pauseDuration);
    fadeAnim->AddGameObject(obj);
    fadeAnim->AddAction(fadeout);
    fadeAnim->AddAction(pauseAction);
    fadeAnim->AddAction(fadein);
    fadeAnim->StartAnimation();

    return fadeAnim;
}


static hdVec3 direction(1.0f, 2.0f, 0.0f);
void GameController::IntroPanProjection(hdTimeInterval interval)
{
    // If the projection aabb is at a boundary, reverse direction.
    if (m_projection->GetAABB().lower.y == m_worldAABB.lower.y
        || m_projection->GetAABB().upper.y == m_worldAABB.upper.y)
    {
        direction.Set(direction.x, -direction.y, direction.z);
    }
    else if (m_projection->GetAABB().lower.x == m_worldAABB.lower.x
             || m_projection->GetAABB().upper.x == m_worldAABB.upper.x)
    {
        direction.Set(-direction.x, direction.y, direction.z);
    }
    PanProjection(0.0f, 0.0f, direction.x, direction.y);
}


// just zoom out
void GameController::IntroZoomProjection(hdTimeInterval interval)
{
    ZoomProjection(1.0f, 1.0f,
                   0.0f, 0.0f,
                   -1.0f, -1.0f,
                   0.0f, 0.0f);
}


void GameController::PlayerReadyMoveProjection(hdTimeInterval interval)
{
    // While the projection is not at the starting position (0.0, upper.x = level.upper.x)
    // move a fraction of the difference between the aabb and the destination.
    float dx, dy;
    dx = dy = 0.0f;

    Block* keyBlock = NULL;

    // find block with tag of 1001 - camera focuses on this block when player is ready...
    for (int i = 0; i < GetCurrentLevel()->GetBlockCount(); i++)
    {
        if (GetCurrentLevel()->GetBlocks()[i]->GetTag() == 1001)
        {
            keyBlock = (Block*)GetCurrentLevel()->GetBlocks()[i];
            SetSelectedTotemBlock(keyBlock);
            break;
        }
    }

    if (keyBlock == NULL)
    {
        /*
         * HACK HACK HACK
         *
         * Focus on the center if there is no key block.
         */
        float worldCenterX = (m_worldAABB.upper.x + m_worldAABB.lower.x) / 2.0f;
        float projCenterX = m_projection->GetAABB().lower.x + ((m_projection->GetAABB().upper.x - m_projection->GetAABB().lower.x) / 2.0f);

        if (m_projection->GetAABB().upper.y <= m_worldAABB.upper.y)
        {
            // move clamped to diff between y
            dy = 0.2f;
        }

        if (projCenterX <= worldCenterX)
        {
            dx = hdMin(0.2f, (projCenterX - worldCenterX));
        }
        else
        {
            dx = -0.2f;
            if ((projCenterX - worldCenterX) > dx)
            {
                dx = (projCenterX - worldCenterX);
            }
        }

        dx *= 10.0f;
        dy *= 100.0f;
    }
    else
    {
        hdVec3 box(0.05f * (m_projection->GetAABB().upper - m_projection->GetAABB().lower));
        hdVec3 diff = (keyBlock->GetWorldCenter() - m_projection->GetWorldCenter());
        hdVec3 mag(fabs(diff.x), fabs(diff.y), 0.0f);

        mag.x = (mag.x < box.x) ? 0.0f : mag.x - box.x;
        mag.y = (mag.y < box.y) ? 0.0f : mag.y - box.y;

        dx = (diff.x > 0.0f) ? -mag.x : mag.x;
        dy = (diff.y < 0.0f) ? -mag.y : mag.y;

        dx *= 5.0f;
        dy *= 5.0f;
    }
    PanProjection(0.0f, 0.0f, dx, dy);
    ZoomProjection(1.0f, 1.0f,
                   0.0f, 0.0f,
                   -1.0f, -1.0f,
                   0.0f, 0.0f);
}


void GameController::AnimatePulse(hdGameObject *object, float duration)
{
    hdAnimation* anim = hdAnimationController::CreateAnimation(this);

    hdAlphaAction* pulseIn = new hdAlphaAction();
    pulseIn->SetStartAlpha(1.0f);
    pulseIn->SetEndAlpha(0.2f);
    pulseIn->SetDuration(duration/2.0f);

    hdAlphaAction* pulseOut = new hdAlphaAction();
    pulseOut->SetStartAlpha(0.2f);
    pulseOut->SetEndAlpha(1.0f);
    pulseOut->SetDuration(duration/2.0f);
    anim->AddGameObject(object);
    anim->AddAction(pulseOut);
    anim->AddAction(pulseIn);

    anim->StartAnimation();
}


void GameController::StartCameraShake()
{
    if (m_cameraShakeAnimation == NULL)
    {
        m_cameraShakeAnimation = hdAnimationController::CreateAnimation(this, false);
        hdFunctionAction<GameController>* funcAction = new hdFunctionAction<GameController>();
        funcAction->SetDuration(0.5f);
        funcAction->SetFunctionObject(this);
        funcAction->SetFunction(&GameController::CameraShakeFunctionCallback);

        m_cameraShakeAnimation->AddGameObject(m_projection); // dummy
        m_cameraShakeAnimation->AddAction(funcAction);
    }

    if (m_cameraShakeAnimation->GetStatus() == e_animationRunning)
    {
        if (m_cameraShakeAnimation->GetActionList() != NULL &&
            m_cameraShakeAnimation->GetActionList()->GetItems()[0] != NULL &&
            m_cameraShakeAnimation->GetActionList()->GetItems()[0]->GetProgress() < 0.25f)
        {
            return;
        }

        // rewind
        hdAssert(m_cameraShakeAnimation->GetActionList()->GetItemCount() == 1);
        m_cameraShakeAnimation->GetActionList()->GetItems()[0]->SetProgress(0.0);

        if (m_cameraShakeDelta > 0.0f)
        {
            m_cameraShakeDelta = -0.175f;
        }
        else
        {
            m_cameraShakeDelta = 0.175f;
        }
    }
    else
    {
        m_cameraShakeDelta = 0.175f;
        m_cameraShakeAnimation->StartAnimation();
    }
}


void GameController::CameraShakeFunctionCallback(hdTimeInterval interval)
{
    hdAABB projAABB;

    if (m_cameraShakeDelta > 0.0f)
    {
        m_cameraShakeDelta = -(m_cameraShakeDelta);
    }
    else
    {
        // reduce slightly over time...
        m_cameraShakeDelta = -(0.8f * m_cameraShakeDelta);
    }

    TranslateProjection(hdVec3(m_cameraShakeDelta, 0.5f * m_cameraShakeDelta, 0));
}


void GameController::HideLobbyButton()
{
    float shorter, longer;

    shorter = hdMin(m_PixelScreenWidth, m_PixelScreenHeight);
    longer  = hdMax(m_PixelScreenWidth, m_PixelScreenHeight);

    m_fontLevelName->Hide();
    m_fontLevelGoals->Hide();

    hdAnimation* landscapeAnim = hdAnimationController::CreateAnimation(this);
    hdVectorAction *landscapeHideAction = new hdVectorAction();
    landscapeHideAction->SetDuration(0.5f);
    landscapeHideAction->SetDestination(hdVec3(-longer/2.0f,
                                               m_btnLobbyReadyLandscape->GetWorldCenter().y, 0.0f));

    landscapeAnim->AddGameObject(m_btnLobbyReadyLandscape);
    landscapeAnim->AddAction(landscapeHideAction);
    landscapeAnim->StartAnimation();


    hdAnimation* portraitAnim = hdAnimationController::CreateAnimation(this);
    hdVectorAction *portraitHideAction = new hdVectorAction();
    portraitHideAction->SetDuration(0.5f);
    portraitHideAction->SetDestination(hdVec3(-shorter/2.0f,
                                              m_btnLobbyReadyLandscape->GetWorldCenter().y, 0.0f));

    portraitAnim->AddGameObject(m_btnLobbyReadyPortrait);
    portraitAnim->AddAction(portraitHideAction);
    portraitAnim->StartAnimation();
}


void GameController::ShowLobbyButton()
{
    m_fontLevelName->Show();
    m_fontLevelGoals->Show();

    hdAnimation* landscapeAnim = hdAnimationController::CreateAnimation(this);
    hdVectorAction *landscapeShowAction = new hdVectorAction();
    landscapeShowAction->SetDuration(0.5f);
    landscapeShowAction->SetDestination(hdVec3(m_PixelScreenWidth/2.0f,
                                               m_btnLobbyReadyLandscape->GetWorldCenter().y, 0.0f));

    landscapeAnim->AddGameObject(m_btnLobbyReadyLandscape);
    landscapeAnim->AddAction(landscapeShowAction);
    landscapeAnim->StartAnimation();

    hdAnimation* portraitAnim = hdAnimationController::CreateAnimation(this);
    hdVectorAction *portraitShowAction = new hdVectorAction();
    portraitShowAction->SetDuration(0.5f);
    portraitShowAction->SetDestination(hdVec3(m_PixelScreenWidth/2.0f,
                                              m_btnLobbyReadyLandscape->GetWorldCenter().y, 0.0f));

    portraitAnim->AddGameObject(m_btnLobbyReadyPortrait);
    portraitAnim->AddAction(portraitShowAction);
    portraitAnim->StartAnimation();
}


void GameController::ShowFinishLevelButton()
{
    if (m_showFinishButtonAnim == NULL)
    {
        m_showFinishButtonAnim = hdAnimationController::CreateAnimation(this, false);
        hdVectorAction *showAction = new hdVectorAction();
        showAction->SetDuration(0.5f);
#ifdef IPHONE_BUILD
        showAction->SetDestination(hdVec3(m_PixelScreenWidth/2.0f, 39.0f, 0.0f));
#else
        showAction->SetDestination(hdVec3(m_PixelScreenWidth/2.0f, 60.0f, 0.0f));
#endif
        m_showFinishButtonAnim->AddGameObject(m_btnFinishLevel);
        m_showFinishButtonAnim->AddAction(showAction);
        m_showFinishButtonAnim->StartAnimation();
    }
}


void GameController::ShowFailRestartButton()
{
    m_failRestartAnim = hdAnimationController::CreateAnimation(this, true);
    hdVectorAction *showAction = new hdVectorAction();
    showAction->SetDuration(0.5f);
#ifdef IPHONE_BUILD
    showAction->SetDestination(hdVec3(m_PixelScreenWidth/2.0f, 39.0f, 0.0f));
#else
    showAction->SetDestination(hdVec3(m_PixelScreenWidth/2.0f, 60.0f, 0.0f));
#endif
    m_failRestartAnim->AddGameObject(m_btnFailRestart);
    m_failRestartAnim->AddAction(showAction);
    m_failRestartAnim->StartAnimation();
}


void GameController::AnimateLostBlockNotification(Block* block)
{
    hdVec2 screenPos;
    hdVec3 aa;

    Polygon* notification = new Polygon(NULL, "Interface/Textures/imgBlockLost.png");
    notification->SetRelative(block);
    notification->SetAlpha(0.0f);
    notification->SetUpdateCallback(this, GameController::HandleNotificationUpdateCallback);

    ConvertInterfaceToScreen(screenPos, block->GetWorldCenter().x, block->GetWorldCenter().y);
    aa.Set(screenPos.x - 35.0f, screenPos.y + 30.0f, 0.0f);
    notification->SetAs2DBox(aa, aa+hdVec3(70, 35, 0));

    if (-1 == m_notificationPolygons->Add(notification))
    {
        hdPrintf("Could not add notification to polygon list: not enough room in list.\n");
        delete notification;
        return;
    }
    hdPrintf("[F] AnimateLostBlockNotification: %d\n", notification);

    AnimateFadeInOut(notification, 0.5f, 0.75f, 0.5f);

    hdAnimation* funcAnim = hdAnimationController::CreateAnimation(this);
    hdFunctionAction<Polygon>* funcAction = new hdFunctionAction<Polygon>();
    funcAction->SetDuration(1.75f);
    funcAction->SetFunctionObject(notification);
    funcAction->SetFunction(&Polygon::DoUpdateCallback);
    funcAnim->AddGameObject(notification);
    funcAnim->AddAction(funcAction);
    funcAnim->SetFinishedCallback(this, GameController::LostBlockNotificationCallback);
    funcAnim->StartAnimation();
}


/*
 * Position the notification relative to the block it is
 * talking about.
 */
void GameController::HandleNotificationUpdateCallback(void *handler, void *notification)
{
    hdVec2 screenPos;
    hdVec2 aa,bb;

    /* Volatile - both the notification and handler may go away at any time */
    if (handler == NULL || notification == NULL) return;

    GameController *self = (GameController *)handler;
    Polygon* note = (Polygon *)notification;
    Block* block = (Block *)note->GetRelative();

    if (NULL == note->GetWorld()) return;

    if (block != NULL)
    {
        self->ConvertInterfaceToScreen(screenPos, block->GetWorldCenter().x, block->GetWorldCenter().y);
        aa.Set(screenPos.x - 35.0f, screenPos.y + 30.0f);
        bb.Set(aa.x + 70, aa.y + 35);

        note->RemoveAllPoints();
        note->AddPoint(aa.x, aa.y);
        note->AddPoint(aa.x, bb.y);
        note->AddPoint(bb.x, bb.y);
        note->AddPoint(bb.x, aa.y);
    }
}


void GameController::LostBlockNotificationCallback(void *handler, hdAnimation *anim)
{
    // Need to avoid dangling pointer problems here otherwise program will crash horribly.
    if (handler == NULL || anim == NULL) return;

    hdPrintf("[F] LostBlockNotificationCallback\n");

    GameController *self = (GameController *)handler;
    if (anim->GetGameObjectCount() == 0) return;

    Polygon* poly = (Polygon*)(anim->GetGameObject(0));
    if (poly == NULL) return;

    self->m_notificationPolygons->Remove(poly);
}


void GameController::AnimateFragmentsMessage(int count)
{
    m_fragmentsText->SetTextFormatted("%d", count);

    if (!isAnimatingFragmentsMessage)
    {
        m_fragmentsBoxAnim = AnimateSlideInOut(m_fragmentsBox, e_screenEdgeLeft, 2.0f);
        m_fragmentsBoxAnim->SetFinishedCallback(this, GameController::FragmentsMessageCallback);

        AnimateFadeInOut(m_fragmentsText, 0.5f, 1.5f, 0.25f);
        isAnimatingFragmentsMessage=true;
    }
}


void GameController::FragmentsMessageCallback(void *handler, hdAnimation *anim)
{
    if (handler == NULL || anim == NULL) return;
    GameController *self = (GameController *)handler;
    self->isAnimatingFragmentsMessage = false;
}


void GameController::AnimateLostMessage(int count)
{
    m_lostMessage->SetTextFormatted("Total Lost: %d", count);

    if (!isAnimatingLostMessage)
    {
        m_lostMessageBoxAnim = AnimateSlideInOut(m_lostMessageBox, e_screenEdgeRight, 2.0f);
        m_lostMessageBoxAnim->SetFinishedCallback(this, GameController::LostMessageCallback);

        // Just in/out the text
        AnimateFadeInOut(m_lostMessage, 0.5f, 1.5f, 0.25f);
        isAnimatingLostMessage=true;
    }
}


void GameController::LostMessageCallback(void *handler, hdAnimation *anim)
{
    if (handler == NULL || anim == NULL) return;
    GameController *self = (GameController *)handler;
    self->isAnimatingLostMessage = false;
}


hdAnimation* GameController::AnimateSlideInOut(hdGameObject *messageBox, e_screenEdge side, const float duration)
{
    hdAABB boxAABB;
    hdVectorAction *slideIn;
    hdPauseAction *pauseAction;
    hdVectorAction *slideOut;
    hdAnimation *slideAnimation;
    const float slideDuration = 0.25f;

    slideAnimation = hdAnimationController::CreateAnimation(this);

    // Come in from right side
    // Final position is where right edge is 10px from right edge of the message polygon
    boxAABB =  messageBox->GetAABB();
    slideIn = new hdVectorAction();
    if (side == e_screenEdgeRight)
    {
        slideIn->SetDestination(hdVec3(m_PixelScreenWidth - 5.0f - (0.5f * boxAABB.Width()),
                                       messageBox->GetWorldCenter().y, 0.0f));
    }
    else if (side == e_screenEdgeLeft)
    {
        slideIn->SetDestination(hdVec3(5.0f + (0.5f * boxAABB.Width()),
                                       messageBox->GetWorldCenter().y, 0.0f));
    }
    slideIn->SetDuration(slideDuration);

    // Hold
    pauseAction = new hdPauseAction();
    pauseAction->SetDuration(duration);

    // Slide out
    slideOut = new hdVectorAction();
    if (side == e_screenEdgeRight)
    {
        slideOut->SetDestination(hdVec3(m_PixelScreenWidth + 5.0f + (0.5f * boxAABB.Width()),
                                        messageBox->GetWorldCenter().y, 0.0f));
    }
    else if (side == e_screenEdgeLeft)
    {
        slideOut->SetDestination(hdVec3(-(5.0f + (0.5f * boxAABB.Width())),
                                        messageBox->GetWorldCenter().y, 0.0f));
    }
    slideOut->SetDuration(slideDuration);

    slideAnimation->AddAction(slideOut);
    slideAnimation->AddAction(pauseAction);
    slideAnimation->AddAction(slideIn);

    slideAnimation->AddGameObject(messageBox);
    slideAnimation->StartAnimation();

    return slideAnimation;
}


void GameController::AnimateCompletionNotification(const float duration, hdGameObject* notification)
{
    if (m_completionSlideAnim != NULL)
    {
        m_completionSlideAnim->StopAnimationWithCompletion();
    }

    if (m_completionFadeAnim != NULL)
    {
        m_completionFadeAnim->StopAnimationWithCompletion();
    }

    notification->Show();
    m_completionMessageBox->Show();
    m_completionSlideAnim = AnimateSlideInOut(m_completionMessageBox, e_screenEdgeRight, 2.0f);
    m_completionFadeAnim = AnimateFadeInOut(notification, 0.5f, 1.5f, 0.15f);
    m_completionFadeAnim->SetFinishedCallback(this, GameController::MultiplierMessageCallback);
}


void GameController::CompletionNotificationCallback(void *handler, hdAnimation *anim)
{
    if (handler == NULL || anim == NULL) return;
    if (anim->GetGameObjectCount() == 0) return;

    hdGameObject* f = static_cast<hdGameObject *>(anim->GetGameObject(0));
    f->Hide();
}


/*
 * The animation creates a font polygon at the block's center
 *
 * The font polygon rises slowly and fades out after one second.
 */
void GameController::AnimatePointsMessage(const hdVec3& pos, int points, float scale)
{
    hdVec2 screenPos;
    ConvertInterfaceToScreen(screenPos, pos.x, pos.y);

    hdFontPolygon* pointsFont = new hdFontPolygon(GAME_CONTROLLER_POINTS_FONT, "", NULL, screenPos.x, screenPos.y, 160.0f, 40.0f);
    pointsFont->SetTextFormatted("%d", points);

    pointsFont->SetTint(1.0f, 1.0f, hdClamp(scale-0.25f, 0.5f, 1.0f), 1.0f);
    pointsFont->SetAlpha(0.0f);
    pointsFont->SetScale(hdClamp(scale+0.25f, 1.0f, 2.0f));
    pointsFont->AlignLeft();
    m_animPolygons->Add(pointsFont);

    hdAnimation* transAnim = hdAnimationController::CreateAnimation(this);
    hdVectorAction* moveTo = new hdVectorAction();
    moveTo->SetDuration(1.7f);
    moveTo->SetDestination(hdVec3(pointsFont->GetWorldCenter().x, screenPos.y + 100.0f, 0.0f));

    transAnim->AddGameObject(pointsFont);
    transAnim->AddAction(moveTo);
    transAnim->SetFinishedCallback(this, GameController::PointsMessageCallback);
    transAnim->StartAnimation();

    AnimateFadeInOut(pointsFont, 0.25f, 0.75f, 0.5f);
}


void GameController::PointsMessageCallback(void *handler, hdAnimation *anim)
{
    if (handler == NULL || anim == NULL) return;
    GameController *self = (GameController *)handler;
    if (anim->GetGameObjectCount() == 0) return;

    hdFontPolygon* f = static_cast<hdFontPolygon *>(anim->GetGameObject(0));
    self->m_animPolygons->Remove(f);
}


void GameController::AnimateMidairChainMessage(Block *block, int points)
{
    midAirChain->SetTextFormatted("+%d", points * 250);
    multiplierFont->SetTextFormatted("%dx", points);

    float tint[4];
    UtilGenerateColor(tint, points % 16, 16);
    multiplierFont->SetTint(tint[0], tint[1], tint[2], 1.0f);

    if (!isAnimatingMidairChain)
    {
        midAirComboBox->SetAlpha(1.0f);
        midAirComboBox->Show();
        m_midAirNoteAnim = AnimateSlideInOut(midAirComboBox, e_screenEdgeLeft, 2.0f);

        // Just fade in/out the text
        hdAnimation* fadeAnim = AnimateFadeInOut(midAirChain, 0.5f, 1.5f, 0.15f);
        fadeAnim->AddGameObject(multiplierFont);
        fadeAnim->SetFinishedCallback(this, GameController::MultiplierMessageCallback);
        isAnimatingMidairChain = true;
    }
}


void GameController::MultiplierMessageCallback(void *handler, hdAnimation *anim)
{
    if (handler == NULL || anim == NULL) return;
    GameController *self = (GameController *)handler;
    self->isAnimatingMidairChain = false;
}


/*
 * Multiplier message
 */
void GameController::AnimateMultiplierMessage(int multiplier)
{
    multiplierFont->SetTextFormatted("%dx", multiplier);

    float tint[4];
    UtilGenerateColor(tint, multiplier % 16, 16);

    multiplierFont->SetTint(tint[0], tint[1], tint[2], 1.0f);
}


void GameController::AnimateIcon(const char *iconTexture, const hdVec3& gamePosition,
                                 int position, bool drifts,
                                 float xWidth, float duration)
{
#define DRAW_ICONS_IN_GAMEWORLD 1
#if DRAW_ICONS_IN_GAMEWORLD == 1
    hdVec2 origin, interfaceValues;
    hdUIImage *icon;
    float xOffset;
    float padding;
    hdAnimation* transAnim;
    hdAnimation* fadeAnim;
    hdVectorAction* moveTo;

    ConvertScreenToInterface(interfaceValues, xWidth, 0.5f * xWidth);
    ConvertScreenToInterface(origin, 0, 0);
    interfaceValues.x = interfaceValues.x - origin.x;
    interfaceValues.y = interfaceValues.y - origin.y;

    padding = 0.5f * interfaceValues.x;

    xOffset = -padding;
    if (position == ICON_POSITION_LEFT)
    {
        xOffset = interfaceValues.x + padding;
    }
    if (position == ICON_POSITION_CENTER)
    {
        xOffset = 0.5f * interfaceValues.x;
    }

    if (NULL == (icon = new hdUIImage(iconTexture, NULL, gamePosition.x-xOffset, gamePosition.y,
                                      interfaceValues.x, interfaceValues.x/2.0f)))
    {
        hdPrintf("[GameController::AnimateIcon] Couldn't find texture for icon %s", iconTexture);
        return;
    }
    icon->SetAlpha(0.0f);
    m_gameWorldAnimPolygons->Add(icon);

    if (drifts)
    {
        transAnim = hdAnimationController::CreateAnimation(this);
        moveTo = new hdVectorAction();
        moveTo->SetDuration(duration);
        moveTo->SetDestination(hdVec3(icon->GetWorldCenter().x, gamePosition.y + interfaceValues.y, 0.0f));

        transAnim->AddGameObject(icon);
        transAnim->AddAction(moveTo);
        //transAnim->SetFinishedCallback(this, GameController::AnimateIconCallback);
        transAnim->StartAnimation();
    }

    fadeAnim = AnimateFadeInOut(icon, 0.1f, duration - 0.3f, 0.30f);
    fadeAnim->SetFinishedCallback(this, GameController::AnimateIconCallback);
#else
    hdVec2 screenPos;
    hdUIImage *icon;
    float xOffset;
    hdAnimation* transAnim;
    hdAnimation* fadeAnim;
    hdVectorAction* moveTo;

    ConvertInterfaceToScreen(screenPos, gamePosition.x, gamePosition.y);

    xOffset = oppositeSide ? xWidth : 0.0f;

    if (NULL == (icon = new hdUIImage(iconTexture, NULL, screenPos.x-xOffset, screenPos.y, xWidth, xWidth/2.0f)))
    {
        hdPrintf("[GameController::AnimateIcon] Couldn't find texture for icon %s", iconTexture);
        return;
    }
    icon->SetAlpha(0.0f);
    m_animPolygons->Add(icon);

    if (drifts)
    {
        transAnim = hdAnimationController::CreateAnimation(this);
        moveTo = new hdVectorAction();
        moveTo->SetDuration(duration);
        moveTo->SetDestination(hdVec3(icon->GetWorldCenter().x, screenPos.y + 100.0f, 0.0f));

        transAnim->AddGameObject(icon);
        transAnim->AddAction(moveTo);
        transAnim->StartAnimation();
    }

    fadeAnim = AnimateFadeInOut(icon, 0.1f, duration - 0.3f, 0.30f);
    fadeAnim->SetFinishedCallback(this, GameController::AnimateIconCallback);
#endif
}


void GameController::AnimateIconCallback(void *handler, hdAnimation *anim)
{
#if 1
    if (handler == NULL || anim == NULL) return;
    GameController *self = (GameController *)handler;
    if (anim->GetGameObjectCount() == 0) return;

    hdUIImage* f = static_cast<hdUIImage *>(anim->GetGameObject(0));
    self->m_gameWorldAnimPolygons->Remove(f);
#else
    if (handler == NULL || anim == NULL) return;
    GameController *self = (GameController *)handler;
    if (anim->GetGameObjectCount() == 0) return;

    hdUIImage* f = static_cast<hdUIImage *>(anim->GetGameObject(0));
    self->m_animPolygons->Remove(f);
#endif
}


void GameController::AnimateRiskyChainMessage(Block *block, int points)
{
}


void GameController::RiskyChainMessageCallback(void *handler, hdAnimation *anim)
{
}


void GameController::AnimateLevelClearNotification()
{
}


void GameController::LevelClearNotificationCallback(void *handler, hdAnimation *anim)
{
}


void GameController::AnimateGoalMessage(const totemGoal& goal, hdGameObject *messageBox)
{
    AnimateGoalMessage(goal.description, messageBox, 2.0f);
}


void GameController::AnimateGoalMessage(const char *description, hdGameObject *messageBox, const float duration)
{
    hdVectorAction* boxMoveIn;
    hdPauseAction* boxMovePauseAction;
    hdVectorAction* boxMoveOut;

    hdVectorAction *textMoveIn;
    hdPauseAction *textMovePauseAction;
    hdVectorAction *textMoveOut;

    if (m_goalAnim != NULL)
    {
        m_goalAnim->StopAnimationWithCompletion();
    }

    if (m_goalTextAnim != NULL)
    {
        m_goalTextAnim->StopAnimationWithCompletion();
    }

    if (m_goalFadeAnim != NULL)
    {
        m_goalFadeAnim->StopAnimationWithCompletion();
    }

    messageBox->Show();
    m_goalDescriptionMessageText->Show();
    m_goalDescriptionMessageText->SetText(description);
    
    m_goalFadeAnim = AnimateFadeInOut(messageBox, 0.25f, duration, 0.25f);
    
    /* Goal box moves */
    boxMoveIn = new hdVectorAction();
    boxMoveIn->SetDuration(0.25f);
    boxMoveIn->SetDestination(hdVec3(m_PixelScreenWidth/2.0f, (m_PixelScreenHeight/2.0f), 0.0f));
    
    boxMovePauseAction = new hdPauseAction();
    boxMovePauseAction->SetDuration(duration);
    
    boxMoveOut = new hdVectorAction();
    boxMoveOut->SetDuration(0.25f);
    boxMoveOut->SetDestination(hdVec3(m_PixelScreenWidth+ (m_PixelScreenHeight/2.0f), 
                                      m_PixelScreenHeight, 0.0f));
    
    m_goalAnim = hdAnimationController::CreateAnimation(this);
    m_goalAnim->SetFinishedCallback(this, GameController::GoalMessageCallback);
    m_goalAnim->AddGameObject(messageBox);
    m_goalAnim->AddAction(boxMoveOut);
    m_goalAnim->AddAction(boxMovePauseAction);
    m_goalAnim->AddAction(boxMoveIn);
    m_goalAnim->StartAnimation();
    
    /* text moves */
    textMoveIn = new hdVectorAction();
    textMoveIn->SetDuration(0.25f);
    if (messageBox == m_goalFailedMessageBox)
    {
        textMoveIn->SetDestination(hdVec3(m_PixelScreenWidth/2.0f, 
                                          (m_PixelScreenHeight/2.0f) - 5.0f, 
                                          0.0f));
    }
    else
    {
        textMoveIn->SetDestination(hdVec3(m_PixelScreenWidth/2.0f, 
                                          (m_PixelScreenHeight/2.0f) - 38.0f, 
                                          0.0f));
    }
    
    textMovePauseAction = new hdPauseAction();
    textMovePauseAction->SetDuration(duration);
    
    textMoveOut = new hdVectorAction();
    textMoveOut->SetDuration(0.25f);
    textMoveOut->SetDestination(hdVec3(m_PixelScreenWidth + (m_PixelScreenHeight/2.0f), 
                                       m_PixelScreenHeight - 40.0f, 
                                       0.0f));
    
    m_goalTextAnim = hdAnimationController::CreateAnimation(this);
    m_goalTextAnim->SetFinishedCallback(this, GameController::GoalMessageCallback);
    m_goalTextAnim->AddGameObject(m_goalDescriptionMessageText);
    m_goalTextAnim->AddAction(textMoveOut);
    m_goalTextAnim->AddAction(textMovePauseAction);
    m_goalTextAnim->AddAction(textMoveIn);
    m_goalTextAnim->StartAnimation();
}



void GameController::GoalMessageCallback(void *handler, hdAnimation *anim)
{
    if (handler == NULL || anim == NULL) return;
    if (anim->GetGameObjectCount() == 0) return;
    
    hdGameObject* a = static_cast<hdGameObject *>(anim->GetGameObject(0));
    hdGameObject* b = static_cast<hdGameObject *>(anim->GetGameObject(1));
    
    if (a != NULL)
        a->Hide();
    
    if (b != NULL) 
        b->Hide();
}


void GameController::AnimateFailNotification()
{
    hdFontPolygon* failFont = new hdFontPolygon(GAME_CONTROLLER_LARGE_FONT, "Fail", NULL, 
                                                m_interfaceProjection->GetWorldCenter().x, 
                                                m_interfaceProjection->GetWorldCenter().y, 
                                                160.0f, 40.0f);
    
    failFont->SetTint(1.0f, 0.0f, 0.0f, 1.0f);
    failFont->SetAlpha(0.0f);
    failFont->AlignCenter();
    m_animPolygons->Add(failFont);
    
    hdAnimation* fadeAnim = AnimateFadeInOut(failFont, 0.25f, 0.75f, 0.5f);
    
    fadeAnim->SetFinishedCallback(this, GameController::FailNotificationCallback);
}


void GameController::FailNotificationCallback(void *handler, hdAnimation *anim)
{
    if (handler == NULL || anim == NULL) return;	
    GameController *self = (GameController *)handler;
    if (anim->GetGameObjectCount() == 0) return;
    
    hdFontPolygon* f = (hdFontPolygon*)(anim->GetGameObject(0));
    self->m_animPolygons->Remove(f);
}
