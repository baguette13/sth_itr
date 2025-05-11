#pragma once

#include "BaseState.hpp"

namespace ue
{

class DiallingState : public BaseState
{
public:
    DiallingState(Context& context);
    ~DiallingState() override;

    // ITimerEventsHandler interface
    void handleTimeout() override;

    // IBtsEventsHandler interface
    void handleDisconnected() override;
    void handleCallAccepted(common::PhoneNumber from) override;
    void handleCallDropped(common::PhoneNumber from) override;
    void handleUnknownRecipient();
    
    // IUserEventsHandler interface
    void handleHomeClicked() override;

private:
    void showDialView();
    void sendCallRequest();
    void cancelCall();
    
    static const std::chrono::milliseconds CALL_TIMEOUT;
    common::PhoneNumber recipientPhoneNumber;
};

}