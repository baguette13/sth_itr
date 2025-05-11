#include "DiallingState.hpp"
#include "ConnectedState.hpp"
#include "TalkingState.hpp"
#include "NotConnectedState.hpp"
#include <sstream>
#include <cstdlib>

namespace ue
{

const std::chrono::milliseconds DiallingState::CALL_TIMEOUT{60000}; // 60 seconds timeout

DiallingState::DiallingState(Context &context)
    : BaseState(context, "DiallingState")
{
    logger.logInfo("Dialling");
    showDialView();
    
    // Start timer for call request timeout
    context.timer.startTimer(CALL_TIMEOUT);
}

DiallingState::~DiallingState()
{
    context.timer.stopTimer();
}

void DiallingState::showDialView()
{
    // Display dialing instructions in the text mode
    auto& textMode = context.user.showViewTextMode();
    textMode.setText("Enter phone number to call\nThen press green button to dial");
    
    // Get the call mode interface from the user port
    auto& callMode = context.user.setCallMode();
    
    // Clear any previous text in the call mode fields
    callMode.clearIncomingText();
    callMode.clearOutgoingText();
    
    // Set accept callback (green button) - this will send the call request
    context.user.setAcceptCallback([this]() {
        logger.logInfo("Call request initiated by user");
        sendCallRequest();
    });
    
    // Set reject callback (red button) - this will cancel the dialing
    context.user.setRejectCallback([this]() {
        logger.logInfo("Dialing canceled by user");
        cancelCall();
    });
}

void DiallingState::sendCallRequest()
{
    // Get the phone number from the outgoing text field
    auto& dialMode = context.user.setCallMode();
    std::string phoneNumberText = dialMode.getOutgoingText();
    logger.logDebug("Phone number entered: ", phoneNumberText);
    
    // Convert string to phone number
    unsigned long numberValue = 0;
    try {
        if (!phoneNumberText.empty()) {
            numberValue = std::stoul(phoneNumberText);
        }
    } catch (const std::exception& ex) {
        logger.logError("Invalid phone number format: ", ex.what());
    }
    
    recipientPhoneNumber = common::PhoneNumber{numberValue};
    
    if (recipientPhoneNumber.value == 0)
    {
        logger.logError("Invalid recipient phone number");
        context.user.showViewTextMode().setText("Invalid phone number");
        context.setState<ConnectedState>();
        return;
    }
    
    // Send call request message to BTS
    logger.logInfo("Sending call request to: ", recipientPhoneNumber);
    context.user.showViewTextMode().setText("Calling: " + to_string(recipientPhoneNumber));
    
    context.bts.sendCallRequest(recipientPhoneNumber);
}

void DiallingState::handleCallAccepted(common::PhoneNumber from)
{
    // Call was accepted by the recipient
    if (from == recipientPhoneNumber)
    {
        logger.logInfo("Call accepted by: ", from);
        context.setState<TalkingState>(from);
    }
}

void DiallingState::handleCallDropped(common::PhoneNumber from)
{
    // Call was dropped by the recipient
    if (from == recipientPhoneNumber)
    {
        logger.logInfo("Call rejected by: ", from);
        context.user.showViewTextMode().setText("Call rejected by: " + to_string(from));
        context.setState<ConnectedState>();
    }
}

void DiallingState::handleUnknownRecipient()
{
    // Recipient is not available
    logger.logInfo("Unknown recipient: ", recipientPhoneNumber);
    context.user.showViewTextMode().setText("User not available: " + to_string(recipientPhoneNumber));
    context.setState<ConnectedState>();
}

void DiallingState::handleTimeout()
{
    // Call request timeout
    logger.logInfo("Call request timed out");
    context.user.showViewTextMode().setText("Call timed out");
    
    // Send call dropped to clean up on BTS side if needed
    if (recipientPhoneNumber.value != 0)
    {
        context.bts.sendCallDropped(recipientPhoneNumber);
    }
    
    context.setState<ConnectedState>();
}

void DiallingState::handleDisconnected()
{
    context.setState<NotConnectedState>();
}

void DiallingState::handleHomeClicked()
{
    cancelCall();
}

void DiallingState::cancelCall()
{
    // If we've already sent a call request, send call dropped message
    if (recipientPhoneNumber.value != 0)
    {
        context.bts.sendCallDropped(recipientPhoneNumber);
    }
    
    context.setState<ConnectedState>();
}

}