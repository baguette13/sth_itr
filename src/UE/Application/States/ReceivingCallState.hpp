#pragma once

#include "BaseState.hpp"

namespace ue
{

class ReceivingCallState : public BaseState
{
public:
    ReceivingCallState(Context& context, common::PhoneNumber caller);
    ~ReceivingCallState() override;

    // ITimerEventsHandler interface
    void handleTimeout() override;

    // IBtsEventsHandler interface
    void handleDisconnected() override;
    
    // IUserEventsHandler interface
    void handleHomeClicked() override;
    
private:
    common::PhoneNumber callerPhoneNumber;
    
    void showCallView();
    void acceptCall();
    void rejectCall();
};

}