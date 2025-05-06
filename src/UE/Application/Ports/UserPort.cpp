#include "UserPort.hpp"
#include "UeGui/IListViewMode.hpp"

namespace ue
{

UserPort::UserPort(common::ILogger &logger, IUeGui &gui, common::PhoneNumber phoneNumber)
    : logger(logger, "[USER-PORT]"),
      gui(gui),
      phoneNumber(phoneNumber)
{}

void UserPort::start(IUserEventsHandler &handler)
{
    this->handler = &handler;
    gui.setTitle("Nokia " + to_string(phoneNumber));
    gui.setRejectCallback([this]() {
        if (this->handler) {
            this->handler->handleHomeClicked();
        }
    });
}

void UserPort::stop()
{
    handler = nullptr;
}

void UserPort::showNotConnected()
{
    gui.showNotConnected();
}

void UserPort::showConnecting()
{
    gui.showConnecting();
}

void UserPort::showConnected()
{
    IUeGui::IListViewMode& menu = gui.setListViewMode();
    menu.clearSelectionList();
    menu.addSelectionListItem("Compose SMS", "");
    menu.addSelectionListItem("View SMS", "");
    
    setMenuCallback();
}

void UserPort::showNewSms(bool present)
{
    gui.showNewSms(present);
}

void UserPort::showSmsListView()
{
    IUeGui::IListViewMode& menu = gui.setListViewMode();
    menu.clearSelectionList();
    // Messages will be populated by the Application class
    
    setMenuCallback();
}

void UserPort::showSmsComposerView()
{
    gui.setSmsComposeMode();
}

IUeGui::IListViewMode& UserPort::getListViewMode()
{
    return gui.setListViewMode();
}

void UserPort::setMenuCallback()
{
    gui.setAcceptCallback([this]() {
        auto selectedItem = gui.setListViewMode().getCurrentItemIndex();
        if (selectedItem.first && handler)
        {
            if (selectedItem.second == 0)
            {
                handler->handleSmsComposeClicked();
            }
            else if (selectedItem.second == 1)
            {
                handler->handleSmsViewClicked();
            }
        }
    });
}

}
