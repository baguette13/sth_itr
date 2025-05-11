#pragma once

#include "IUserPort.hpp"
#include "Logger/PrefixedLogger.hpp"
#include "IUeGui.hpp"
#include "Messages/PhoneNumber.hpp"

namespace ue
{

class UserPort : public IUserPort
{
public:
    UserPort(common::ILogger& logger, IUeGui& gui, common::PhoneNumber phoneNumber);
    void start(IUserEventsHandler& handler);
    void stop();

    void showNotConnected() override;
    void showConnecting() override;
    void showConnected() override;
    void showNewSms(bool present) override;
    void showSmsListView() override;
    void showSmsComposerView() override;
    IUeGui::IListViewMode& getListViewMode() override;
    IUeGui::ISmsComposeMode& getSmsComposeMode() override;
    IUeGui::ITextMode& showViewTextMode() override;
    IUeGui::ICallMode& setCallMode() override;
    void setAcceptCallback(IUeGui::Callback) override;
    void setRejectCallback(IUeGui::Callback) override;

private:
    common::PrefixedLogger logger;
    IUeGui& gui;
    common::PhoneNumber phoneNumber;
    IUserEventsHandler* handler = nullptr;
    
    void setMenuCallback();
};

}
