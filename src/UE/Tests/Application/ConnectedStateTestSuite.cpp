#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "States/ConnectedState.hpp"
#include "States/DiallingState.hpp"
#include "Context.hpp"
#include "Mocks/ILoggerMock.hpp"
#include "Mocks/IBtsPortMock.hpp"
#include "Mocks/IUserPortMock.hpp"
#include "Mocks/ITimerPortMock.hpp"
#include "Mocks/IUeGuiMock.hpp"

namespace ue
{
using namespace ::testing;

class ConnectedStateTestSuite : public Test
{
protected:
    NiceMock<common::ILoggerMock> loggerMock;
    NiceMock<IBtsPortMock> btsPortMock;
    NiceMock<IUserPortMock> userPortMock;
    NiceMock<ITimerPortMock> timerPortMock;
    NiceMock<ISmsComposeModeMock> smsComposeModeMock;
    NiceMock<IListViewModeMock> listViewModeMock;
    
    Context context{loggerMock, btsPortMock, userPortMock, timerPortMock};
    IUeGui::Callback acceptCallback;
    
    ConnectedStateTestSuite()
    {
        ON_CALL(userPortMock, getSmsComposeMode()).WillByDefault(ReturnRef(smsComposeModeMock));
        ON_CALL(userPortMock, getListViewMode()).WillByDefault(ReturnRef(listViewModeMock));
        // Allow showConnected to be called during constructor
        EXPECT_CALL(userPortMock, showConnected()).Times(AnyNumber());
    }
};

TEST_F(ConnectedStateTestSuite, shallShowSmsComposerViewWhenSmsComposeClicked)
{
    // given
    ConnectedState objectUnderTest{context};
    
    // expect
    EXPECT_CALL(userPortMock, showSmsComposerView());
    EXPECT_CALL(userPortMock, setAcceptCallback(_));
    
    // when
    objectUnderTest.handleSmsComposeClicked();
}

TEST_F(ConnectedStateTestSuite, shallSendSmsWhenAcceptCallbackCalled)
{
    // given
    ConnectedState objectUnderTest{context};
    
    // Setup for SMS composition
    EXPECT_CALL(userPortMock, showSmsComposerView());
    EXPECT_CALL(userPortMock, setAcceptCallback(_))
        .WillOnce(SaveArg<0>(&acceptCallback));
    objectUnderTest.handleSmsComposeClicked();
    ::testing::Mock::VerifyAndClearExpectations(&userPortMock);
    
    // Setup for SMS sending
    const common::PhoneNumber recipient{124};
    const std::string smsText = "Test SMS";
    
    EXPECT_CALL(smsComposeModeMock, getPhoneNumber()).WillOnce(Return(recipient));
    EXPECT_CALL(smsComposeModeMock, getSmsText()).WillOnce(Return(smsText));
    EXPECT_CALL(btsPortMock, sendSms(recipient, smsText));
    EXPECT_CALL(smsComposeModeMock, clearSmsText());
    EXPECT_CALL(userPortMock, showConnected()); // Return to main menu
    
    // when
    acceptCallback();
}

TEST_F(ConnectedStateTestSuite, shallTransitionToReceivingCallStateOnIncomingCallRequest)
{
    // given
    ConnectedState objectUnderTest{context};
    const common::PhoneNumber CALLER_NUMBER{123};
    
    // These expectations are needed because ConnectedState will transition to ReceivingCallState
    // which will call these methods in its constructor
    NiceMock<ITextModeMock> textModeMock;
    ON_CALL(userPortMock, showViewTextMode()).WillByDefault(ReturnRef(textModeMock));
    
    // expect
    EXPECT_CALL(userPortMock, showViewTextMode());
    EXPECT_CALL(userPortMock, setAcceptCallback(_));
    EXPECT_CALL(userPortMock, setRejectCallback(_));
    EXPECT_CALL(timerPortMock, startTimer(_));
    
    // when
    objectUnderTest.handleCallRequest(CALLER_NUMBER);
    
    // Transition to ReceivingCallState happens in the implementation
    // We can't directly verify the state change in this test structure,
    // but the implementation should call context.setState<ReceivingCallState>(CALLER_NUMBER)
}

TEST_F(ConnectedStateTestSuite, shallTransitionToDiallingStateWhenDialClicked)
{
    // given
    ConnectedState objectUnderTest{context};
    
    // Setup for menu selection - use handleHomeClicked which will call showMenuView internally
    EXPECT_CALL(userPortMock, showConnected());
    EXPECT_CALL(userPortMock, getListViewMode()).WillOnce(ReturnRef(listViewModeMock));
    EXPECT_CALL(listViewModeMock, clearSelectionList());
    EXPECT_CALL(listViewModeMock, addSelectionListItem(_, _)).Times(3);  // Three menu items
    EXPECT_CALL(userPortMock, setAcceptCallback(_)).WillOnce(SaveArg<0>(&acceptCallback));
    
    // Trigger menu view by calling handleHomeClicked
    objectUnderTest.handleHomeClicked();
    ::testing::Mock::VerifyAndClearExpectations(&userPortMock);
    
    // These expectations are needed because ConnectedState will transition to DiallingState
    // which will call these methods in its constructor
    NiceMock<ICallModeMock> callModeMock;
    NiceMock<ITextModeMock> textModeMock;
    ON_CALL(userPortMock, setCallMode()).WillByDefault(ReturnRef(callModeMock));
    ON_CALL(userPortMock, showViewTextMode()).WillByDefault(ReturnRef(textModeMock));
    
    // expect for DiallingState constructor
    EXPECT_CALL(userPortMock, setCallMode());
    EXPECT_CALL(callModeMock, clearIncomingText());
    EXPECT_CALL(callModeMock, clearOutgoingText());
    EXPECT_CALL(userPortMock, showViewTextMode());
    EXPECT_CALL(textModeMock, setText(_));
    EXPECT_CALL(userPortMock, setAcceptCallback(_));
    EXPECT_CALL(userPortMock, setRejectCallback(_));
    EXPECT_CALL(timerPortMock, startTimer(_));
    
    // Setup the expected index for dial option
    EXPECT_CALL(listViewModeMock, getCurrentItemIndex())
        .WillOnce(Return(std::make_pair(true, 2))); // Dial is the third menu item (index 2)
    
    // when - selecting "Dial" from menu
    acceptCallback();
}

}