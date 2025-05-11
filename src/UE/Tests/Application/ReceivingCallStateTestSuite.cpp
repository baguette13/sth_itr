#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "States/ReceivingCallState.hpp"
#include "States/ConnectedState.hpp"
#include "States/TalkingState.hpp"
#include "Context.hpp"
#include "Mocks/ILoggerMock.hpp"
#include "Mocks/IBtsPortMock.hpp"
#include "Mocks/IUserPortMock.hpp"
#include "Mocks/ITimerPortMock.hpp"
#include "Mocks/IUeGuiMock.hpp"

namespace ue
{
using namespace ::testing;

class ReceivingCallStateTestSuite : public Test
{
protected:
    const common::PhoneNumber CALLER_NUMBER{123};
    NiceMock<common::ILoggerMock> loggerMock;
    NiceMock<IBtsPortMock> btsPortMock;
    NiceMock<IUserPortMock> userPortMock;
    NiceMock<ITimerPortMock> timerPortMock;
    NiceMock<ITextModeMock> textModeMock;
    NiceMock<IListViewModeMock> listViewModeMock;
    NiceMock<ICallModeMock> callModeMock;
    
    Context context{loggerMock, btsPortMock, userPortMock, timerPortMock};
    IUeGui::Callback acceptCallback;
    IUeGui::Callback rejectCallback;
    
    ReceivingCallStateTestSuite()
    {
        ON_CALL(userPortMock, showViewTextMode()).WillByDefault(ReturnRef(textModeMock));
        ON_CALL(userPortMock, getListViewMode()).WillByDefault(ReturnRef(listViewModeMock));
        ON_CALL(userPortMock, setCallMode()).WillByDefault(ReturnRef(callModeMock));
        
        // Store callbacks for testing
        EXPECT_CALL(userPortMock, setAcceptCallback(_))
            .WillOnce(SaveArg<0>(&acceptCallback));
        EXPECT_CALL(userPortMock, setRejectCallback(_))
            .WillOnce(SaveArg<0>(&rejectCallback));
    }
};

TEST_F(ReceivingCallStateTestSuite, shallShowIncomingCallScreenAndStartTimer)
{
    // expect
    EXPECT_CALL(userPortMock, showViewTextMode());
    EXPECT_CALL(textModeMock, setText(HasSubstr("Incoming call from: " + to_string(CALLER_NUMBER))));
    EXPECT_CALL(timerPortMock, startTimer(_));
    
    // when
    ReceivingCallState objectUnderTest{context, CALLER_NUMBER};
}

TEST_F(ReceivingCallStateTestSuite, shallAcceptCallAndTransitionToTalkingState)
{
    // given
    ReceivingCallState objectUnderTest{context, CALLER_NUMBER};
    ::testing::Mock::VerifyAndClearExpectations(&userPortMock);
    ::testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    
    // expect
    EXPECT_CALL(btsPortMock, sendCallAccepted(CALLER_NUMBER));
    EXPECT_CALL(timerPortMock, stopTimer());
    
    // when
    acceptCallback();
    
    // Verify state transition (would require a mock for the setState method in Context)
    // In a real test we could check if context.state is now an instance of TalkingState
}

TEST_F(ReceivingCallStateTestSuite, shallRejectCallAndTransitionToConnectedState)
{
    // given
    ReceivingCallState objectUnderTest{context, CALLER_NUMBER};
    ::testing::Mock::VerifyAndClearExpectations(&userPortMock);
    ::testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    
    // expect
    EXPECT_CALL(btsPortMock, sendCallDropped(CALLER_NUMBER));
    EXPECT_CALL(userPortMock, showConnected()); // Add this expectation
    EXPECT_CALL(timerPortMock, stopTimer());
    
    // when
    rejectCallback();
}

TEST_F(ReceivingCallStateTestSuite, shallTimeoutAndRejectCall)
{
    // given
    ReceivingCallState objectUnderTest{context, CALLER_NUMBER};
    ::testing::Mock::VerifyAndClearExpectations(&userPortMock);
    ::testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    
    // expect
    EXPECT_CALL(btsPortMock, sendCallDropped(CALLER_NUMBER));
    EXPECT_CALL(userPortMock, showConnected()); // Add this expectation
    EXPECT_CALL(timerPortMock, stopTimer());
    
    // when
    objectUnderTest.handleTimeout();
}

TEST_F(ReceivingCallStateTestSuite, shallRejectCallOnHomeButton)
{
    // given
    ReceivingCallState objectUnderTest{context, CALLER_NUMBER};
    ::testing::Mock::VerifyAndClearExpectations(&userPortMock);
    ::testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    
    // expect
    EXPECT_CALL(btsPortMock, sendCallDropped(CALLER_NUMBER));
    EXPECT_CALL(userPortMock, showConnected()); // Add this expectation
    EXPECT_CALL(timerPortMock, stopTimer());
    
    // when
    objectUnderTest.handleHomeClicked();
    
    // Verify state transition (would require a mock for the setState method in Context)
    // In a real test we could check if context.state is now an instance of ConnectedState
}

}