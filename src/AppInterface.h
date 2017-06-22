/*
 * Copyright (c) 2017 Hackdirt Ltd.
 * Author: David Petrie (david@davidpetrie.com)
 *
 * License: https://www.gnu.org/licenses/gpl-3.0.en.html
 */

#ifndef APP_INTERFACE_H
#define APP_INTERFACE_H

#include <hdk/game.h>
#include "PreferencesView.h"

#define LEVELPICKER_NEW_LEVEL_MESSAGE_TAG 6001
#define LEVELPICKER_LEVEL_PREV_TAG 500

#define LEVEL_CONTROLLER_TITLE_FONT CONFIG_TITLE_FONT
#define LEVEL_CONTROLLER_MESSAGE_FONT CONFIG_MESSAGE_FONT
#define LEVEL_CONTROLLER_TINY_FONT CONFIG_TINY_FONT

class AppInterface : public hdInterfaceController
{
public:
    AppInterface(const hdInterfaceController* parentController);

    ~AppInterface();

    void DrawAppInterface();

    void RefreshLayout();


protected:
    void DrawInternalInterface();

    static void HideMessageClickCallback(void *handler, void *sender);

    static void SkipUpCallback(void *handler, void *sender);

    static void PrevUpCallback(void *handler, void *sender);

    static void MessageQueueUpClicked(void *handler, void *sender);

    static void GenericButtonDownCallback(void *handler, void *sender);

    virtual void Game_ShowMessage();

    virtual void Game_HideMessage();

    void FindAndShowQueueMessage();

    /* Messaging Functions */
    void LoadScriptMessageForTag(const int tag);

    void LoadScriptMessageForTag(const int tag, const bool addToQueue);

    void LoadScriptMessageForTagAndLevel(const int tag, const int levelId, const bool addToQueue);

    void LoadScriptMessageForTagAndLevel(const int tag, const int levelId, const bool addToQueue, const bool noShow);

    void SetMessage(const totemMessage *msg);

    void SetMessage(const hdTexture *texture);

    void ShowMessage();

    void HideMessage();

    void HideMessageHandleCallback();

    const bool IsShowingMessage() const;


    /*
     * Preferences
     */
    void ShowPreferences();

    void HidePreferences();

    static void ClosePreferencesCallback(void *handler, void *sender);

    /**********************************************************************/
    /* Messages */
    /*
     * Note: two versions of each, one for portrait mode, the other
     * for landscape mode.
     */
    /**********************************************************************/
    totemMessage* m_scriptMessages[10];

    int m_messageCount;

    int m_currentMessageIndex;

    hdTypedefList<totemMessage, 16> *m_messageTagQueue;

    const bool ShowNextScriptMessage();

    static void HideMessageCallback(void *handler, hdAnimation *anim);

    hdUIContainer * GetCurrentMessageContainer() const;

    void SetMessageTexture(const char *texturePrefix);

    void ShowMessageInterface(hdUIContainer *container, hdUIContainer *buttonContainer);

    void HideMessageInterface(hdUIContainer *container, hdUIContainer *buttonContainer);

    /* Title Message */
    hdUIImage *m_uiLandscapeTitleMessageBackground;

    hdFontPolygon *m_uiLandscapeTitleMessageFont;

    hdUIImage *m_uiPortraitTitleMessageBackground;

    hdFontPolygon *m_uiPortraitTitleMessageFont;

    /*
     * Preferences View
     */
    PreferencesView *m_prefsView;


    // Small Message queue button
    hdUIContainer *m_uiMessageQueueContainer;

    hdButton *m_uiMessageQueueButton;

    hdFontPolygon *m_uiMessageQueueCount;


    hdUIContainer *m_uiLandscapeMessageContainer;

    hdUIContainer *m_uiLandscapeMessageButtonContainer;

    hdUIContainer *m_uiPortraitMessageContainer;

    hdUIContainer *m_uiPortraitMessageButtonContainer;


    hdUIImage *m_uiLandscapeImageMessage;

    hdFontPolygon *m_uiLandscapeTextMessage;

    hdUIImage *m_uiLandscapeTextMessageBackground;

    hdButton *m_uiLandscapeMessageOKButton;

    hdButton *m_uiLandscapeMessageDoneButton;

    hdButton *m_uiLandscapeMessageSkipButton;

    hdButton *m_uiLandscapeMessagePrevButton;

    hdButton *m_uiLandscapeMessageNextButton;


    hdUIImage *m_uiPortraitImageMessage;

    hdFontPolygon *m_uiPortraitTextMessage;

    hdUIImage *m_uiPortraitTextMessageBackground;

    hdButton *m_uiPortraitMessageOKButton;

    hdButton *m_uiPortraitMessageDoneButton;

    hdButton *m_uiPortraitMessageSkipButton;

    hdButton *m_uiPortraitMessagePrevButton;

    hdButton *m_uiPortraitMessageNextButton;

    // Avatar Messages
    hdUIImage *m_uiLandscapeAvatar;

    hdUIImage *m_uiLandscapeAvatarChrome;

    hdFontPolygon *m_uiLandscapeAvatarMessageText;

    hdUIImage *m_uiLandscapeAvatarMessageBackground;

    hdUIImage *m_uiLandscapeAvatarMessageChrome;


    hdUIImage *m_glow;
    
    hdUIImage *m_uiPortraitAvatar;
    
    hdUIImage *m_uiPortraitAvatarChrome;
    
    hdFontPolygon *m_uiPortraitAvatarMessageText;
    
    hdUIImage *m_uiPortraitAvatarMessageBackground;
    
    hdUIImage *m_uiPortraitAvatarMessageChrome;

    int m_HACKHACKHACK_currentLevelId;

private:
    void InitAppInterface();
    
    hdSound *m_btnMessageClickDownSound;

    hdSound *m_btnMessageClickUpSound;

};


inline hdUIContainer * AppInterface::GetCurrentMessageContainer() const
{
    if (m_orientation == e_hdInterfaceOrientationLandscapeLeft ||
        m_orientation == e_hdInterfaceOrientationLandscapeRight)
    {
        return m_uiLandscapeMessageContainer;
    }
    else
    {
        return m_uiPortraitMessageContainer;
    }
}

#endif
