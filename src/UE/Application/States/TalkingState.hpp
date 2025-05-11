#pragma once

#include "BaseState.hpp"

namespace ue
{

class TalkingState : public BaseState
{
public:
    TalkingState(Context& context, common::PhoneNumber peer);

    // IBtsEventsHandler interface
    void handleDisconnected() override;
    void handleCallDropped(common::PhoneNumber from) override;
    void handleCallTalk(common::PhoneNumber from, const std::string& text) override;
    
    // IUserEventsHandler interface
    void handleHomeClicked() override;
    
private:
    common::PhoneNumber peerPhoneNumber;
    
    void showConnectedView();
};

}