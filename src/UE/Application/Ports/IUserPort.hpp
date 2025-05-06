#pragma once

#include "UeGui/IListViewMode.hpp"

namespace ue
{

class IUserEventsHandler
{
public:
    virtual ~IUserEventsHandler() = default;
    
    virtual void handleHomeClicked() = 0;
    virtual void handleSmsComposeClicked() = 0;
    virtual void handleSmsViewClicked() = 0;
};

class IUserPort
{
public:
    virtual ~IUserPort() = default;

    virtual void showNotConnected() = 0;
    virtual void showConnecting() = 0;
    virtual void showConnected() = 0;
    virtual void showNewSms(bool present) = 0;
    virtual void showSmsListView() = 0;
    virtual void showSmsComposerView() = 0;
    virtual IUeGui::IListViewMode& getListViewMode() = 0;
};

}
