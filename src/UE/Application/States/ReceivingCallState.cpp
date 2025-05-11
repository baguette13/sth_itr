#include "ReceivingCallState.hpp"
#include "ConnectedState.hpp"
#include "TalkingState.hpp"

namespace ue
{

ReceivingCallState::ReceivingCallState(Context &context, common::PhoneNumber caller)
    : BaseState(context, "ReceivingCallState"),
      callerPhoneNumber(caller)
{
    logger.logInfo("Incoming call from: ", callerPhoneNumber);
    showCallView();
    
    // Start 30-second timeout for call request
    context.timer.startTimer(std::chrono::milliseconds(30000));
}

ReceivingCallState::~ReceivingCallState()
{
    context.timer.stopTimer();
}

void ReceivingCallState::handleTimeout()
{
    logger.logInfo("Call request timed out");
    rejectCall();
}

void ReceivingCallState::handleDisconnected()
{
    context.setState<ConnectedState>();
}

void ReceivingCallState::handleHomeClicked()
{
    rejectCall();
}

void ReceivingCallState::showCallView()
{
    // Show incoming call screen
    context.user.showViewTextMode().setText("Incoming call from: " + to_string(callerPhoneNumber));
    
    // Set accept callback (green button)
    context.user.setAcceptCallback([this]() {
        logger.logInfo("Call accepted by user");
        acceptCall();
    });
    
    // Set reject callback (red button)
    context.user.setRejectCallback([this]() {
        logger.logInfo("Call rejected by user");
        rejectCall();
    });
}

void ReceivingCallState::acceptCall()
{
    // Send CallAccept message to BTS
    context.bts.sendCallAccepted(callerPhoneNumber);
    
    // Transition to talking state
    context.setState<TalkingState>(callerPhoneNumber);
}

void ReceivingCallState::rejectCall()
{
    // Send CallDropped message to BTS
    context.bts.sendCallDropped(callerPhoneNumber);
    
    // Return to connected state
    context.setState<ConnectedState>();
}

}