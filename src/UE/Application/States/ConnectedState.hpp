#pragma once

#include "BaseState.hpp"
#include "SmsDb.hpp"

namespace ue
{

class ConnectedState : public BaseState
{
public:
    ConnectedState(Context& context);

    // IBtsEventsHandler interface
    void handleDisconnected() override;
    void handleSms(common::PhoneNumber from, const std::string& text) override;
    
    // IUserEventsHandler interface
    void handleHomeClicked() override;
    void handleSmsComposeClicked() override;
    void handleSmsViewClicked() override;
    
private:
    void showMenuView();
    void showSmsListView();
    void showSmsComposerView();
    void showSmsView(const SmsMessage& message);
    void handleSmsSend();
    void refreshMessageIndicator();
    
    SmsDb smsDb;
    std::vector<SmsMessage> currentMessagesList;
};

}
