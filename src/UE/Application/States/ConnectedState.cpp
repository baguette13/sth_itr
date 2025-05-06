#include "ConnectedState.hpp"
#include "NotConnectedState.hpp"

namespace ue
{

ConnectedState::ConnectedState(Context &context)
    : BaseState(context, "ConnectedState")
{
    context.user.showConnected();
}

void ConnectedState::handleDisconnected()
{
    context.setState<NotConnectedState>();
}

void ConnectedState::handleSms(common::PhoneNumber from, const std::string& text)
{
    logger.logInfo("Received SMS from: ", from, ", text: ", text);
    smsDb.addSms(from, text);
    refreshMessageIndicator();
}

void ConnectedState::handleHomeClicked()
{
    showMenuView();
}

void ConnectedState::handleSmsComposeClicked()
{
    showSmsComposerView();
}

void ConnectedState::handleSmsViewClicked()
{
    showSmsListView();
}

void ConnectedState::showMenuView()
{
    context.user.showConnected();
}

void ConnectedState::showSmsListView()
{
    context.user.showSmsListView();
    
    auto messages = smsDb.getSmsMessages();
    auto& menu = context.user.getListViewMode();
    
    for (size_t i = 0; i < messages.size(); ++i)
    {
        const auto& message = messages[i];
        std::string label = "From: " + to_string(message.from);
        std::string tooltip = message.text;
        menu.addSelectionListItem(label, tooltip);
        
        // Mark message as read when displayed in list
        if (!message.isRead)
        {
            smsDb.markAsRead(i);
        }
    }
    
    refreshMessageIndicator();
}

void ConnectedState::showSmsComposerView()
{
    context.user.showSmsComposerView();
}

void ConnectedState::refreshMessageIndicator()
{
    bool hasUnread = smsDb.hasUnreadSms();
    context.user.showNewSms(hasUnread);
}

}
