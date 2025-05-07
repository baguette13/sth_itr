#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "States/ConnectedState.hpp"
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

}