#include "ConnectedState.hpp"
#include "NotConnectedState.hpp"
#include "ReceivingCallState.hpp"

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

void ConnectedState::handleCallRequest(common::PhoneNumber from)
{
    logger.logInfo("Received call request from: ", from);
    // Create a new PhoneNumber by value to ensure it's passed by value
    common::PhoneNumber caller = from;
    context.setState<ReceivingCallState>(caller);
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
    
    currentMessagesList = smsDb.getSmsMessages();
    auto& menu = context.user.getListViewMode();
    
    for (size_t i = 0; i < currentMessagesList.size(); ++i)
    {
        const auto& message = currentMessagesList[i];
        std::string label;
        if (message.isSent) {
            label = "To: " + to_string(message.to);
        } else {
            label = "From: " + to_string(message.from);
        }
        std::string tooltip = message.text;
        menu.addSelectionListItem(label, tooltip);
        
        // Mark message as read when displayed in list
        if (!message.isRead)
        {
            smsDb.markAsRead(i);
        }
    }
    
    // Set callback for when a message is selected from the list
    context.user.setAcceptCallback([this]() {
        auto& listView = context.user.getListViewMode();
        auto selectedItem = listView.getCurrentItemIndex();
        
        if (selectedItem.first && selectedItem.second < currentMessagesList.size()) {
            // Show the selected message content
            showSmsView(currentMessagesList[selectedItem.second]);
        }
    });
    
    refreshMessageIndicator();
}

void ConnectedState::showSmsComposerView()
{
    // First display the compose view
    context.user.showSmsComposerView();
    
    // Clear any previous text in the compose view
    auto& composeMode = context.user.getSmsComposeMode();
    composeMode.clearSmsText();
    
    // Set the callback for the accept button (green button)
    context.user.setAcceptCallback([this]() {
        this->handleSmsSend();
    });
}

void ConnectedState::handleSmsSend()
{
    // Get the phone number and text from the UI
    auto& composeMode = context.user.getSmsComposeMode();
    PhoneNumber recipient = composeMode.getPhoneNumber();
    std::string text = composeMode.getSmsText();
    
    logger.logInfo("Sending SMS to: ", recipient, ", text: ", text);
    
    // Send the SMS via BTS
    context.bts.sendSms(recipient, text);
    
    // Store the sent SMS
    smsDb.addSentSms(recipient, text);
    
    // Clear the SMS compose mode
    composeMode.clearSmsText();
    
    // Return to the main menu
    showMenuView();
}

void ConnectedState::showSmsView(const SmsMessage& message)
{
    auto& textMode = context.user.showViewTextMode();
    
    // Create header and content for the SMS view
    std::string header;
    if (message.isSent) {
        header = "Sent message to: " + to_string(message.to);
    } else {
        header = "Message from: " + to_string(message.from);
    }
    
    textMode.setText(header + "\n\n" + message.text);
    
    // Set callback to return to SMS list when user presses the green button
    context.user.setAcceptCallback([this]() {
        showSmsListView();
    });
    
    // Set callback to return to main menu when user presses the red button
    context.user.setRejectCallback([this]() {
        showMenuView();
    });
}

void ConnectedState::refreshMessageIndicator()
{
    bool hasUnread = smsDb.hasUnreadSms();
    context.user.showNewSms(hasUnread);
}

}
