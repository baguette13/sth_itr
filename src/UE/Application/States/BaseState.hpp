#pragma once

#include "IEventsHandler.hpp"
#include "Logger/PrefixedLogger.hpp"
#include "Context.hpp"

namespace ue
{

class BaseState : public IEventsHandler
{
public:
    BaseState(Context& context, const std::string& name);
    ~BaseState() override;

    // ITimerEventsHandler interface
    void handleTimeout() override;

    // IBtsEventsHandler interface
    void handleSib(common::BtsId btsId) override;
    void handleAttachAccept() override;
    void handleAttachReject() override;
    virtual void handleDisconnected();
    void handleSms(common::PhoneNumber from, const std::string& text) override;
    void handleCallRequest(common::PhoneNumber from) override;
    void handleCallAccepted(common::PhoneNumber from) override;
    void handleCallDropped(common::PhoneNumber from) override;
    void handleCallTalk(common::PhoneNumber from, const std::string& text) override;
    void handleUnknownRecipient() override;
    
    // IUserEventsHandler interface
    void handleHomeClicked() override;
    void handleSmsComposeClicked() override;
    void handleSmsViewClicked() override;

protected:
    Context& context;
    common::PrefixedLogger logger;
};

}
