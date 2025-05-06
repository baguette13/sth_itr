#pragma once

#include <gmock/gmock.h>
#include "Ports/IUserPort.hpp"
#include "UeGui/IListViewMode.hpp"
#include "IUeGuiMock.hpp"

namespace ue
{

class IUserEventsHandlerMock : public IUserEventsHandler
{
public:
    IUserEventsHandlerMock();
    ~IUserEventsHandlerMock() override;
    
    MOCK_METHOD(void, handleHomeClicked, (), (final));
    MOCK_METHOD(void, handleSmsComposeClicked, (), (final));
    MOCK_METHOD(void, handleSmsViewClicked, (), (final));
};

class IUserPortMock : public IUserPort
{
public:
    IUserPortMock();
    ~IUserPortMock() override;

    MOCK_METHOD(void, showNotConnected, (), (final));
    MOCK_METHOD(void, showConnecting, (), (final));
    MOCK_METHOD(void, showConnected, (), (final));
    MOCK_METHOD(void, showNewSms, (bool), (final));
    MOCK_METHOD(void, showSmsListView, (), (final));
    MOCK_METHOD(void, showSmsComposerView, (), (final));
    MOCK_METHOD(IUeGui::IListViewMode&, getListViewMode, (), (final));
};

}
