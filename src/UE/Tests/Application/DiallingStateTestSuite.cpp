#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "States/DiallingState.hpp"
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

class DiallingStateTestSuite : public Test
{
protected:
    const common::PhoneNumber RECIPIENT_NUMBER{234};
    NiceMock<common::ILoggerMock> loggerMock;
    NiceMock<IBtsPortMock> btsPortMock;
    NiceMock<IUserPortMock> userPortMock;
    NiceMock<ITimerPortMock> timerPortMock;
    NiceMock<ICallModeMock> callModeMock;
    NiceMock<ITextModeMock> textModeMock;
    NiceMock<IListViewModeMock> listViewModeMock;
    
    Context context{loggerMock, btsPortMock, userPortMock, timerPortMock};
    IUeGui::Callback acceptCallback;
    IUeGui::Callback rejectCallback;
    
    DiallingStateTestSuite()
    {
        ON_CALL(userPortMock, setCallMode()).WillByDefault(ReturnRef(callModeMock));
        ON_CALL(userPortMock, showViewTextMode()).WillByDefault(ReturnRef(textModeMock));
        ON_CALL(userPortMock, getListViewMode()).WillByDefault(ReturnRef(listViewModeMock));
        
        // Store callbacks for testing
        EXPECT_CALL(userPortMock, setAcceptCallback(_))
            .WillOnce(SaveArg<0>(&acceptCallback));
        EXPECT_CALL(userPortMock, setRejectCallback(_))
            .WillOnce(SaveArg<0>(&rejectCallback));
    }
};

TEST_F(DiallingStateTestSuite, shallShowDialScreenAndStartTimer)
{
    // expect
    EXPECT_CALL(userPortMock, setCallMode());
    EXPECT_CALL(callModeMock, clearIncomingText());
    EXPECT_CALL(callModeMock, clearOutgoingText());
    EXPECT_CALL(userPortMock, showViewTextMode());
    EXPECT_CALL(textModeMock, setText(_));
    EXPECT_CALL(timerPortMock, startTimer(_));
    
    // when
    DiallingState objectUnderTest{context};
}

TEST_F(DiallingStateTestSuite, shallSendCallRequestWhenUserAccepts)
{
    // given
    DiallingState objectUnderTest{context};
    ::testing::Mock::VerifyAndClearExpectations(&userPortMock);
    ::testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    ::testing::Mock::VerifyAndClearExpectations(&callModeMock);
    
    // expect
    EXPECT_CALL(userPortMock, setCallMode()).WillOnce(ReturnRef(callModeMock));
    EXPECT_CALL(callModeMock, getOutgoingText()).WillOnce(Return(std::to_string(RECIPIENT_NUMBER.value)));
    EXPECT_CALL(userPortMock, showViewTextMode()).WillOnce(ReturnRef(textModeMock));
    EXPECT_CALL(textModeMock, setText(HasSubstr("Calling: " + to_string(RECIPIENT_NUMBER))));
    EXPECT_CALL(btsPortMock, sendCallRequest(RECIPIENT_NUMBER));
    
    // when
    acceptCallback();
}

TEST_F(DiallingStateTestSuite, shallHandleInvalidPhoneNumber)
{
    // given
    DiallingState objectUnderTest{context};
    ::testing::Mock::VerifyAndClearExpectations(&userPortMock);
    ::testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    ::testing::Mock::VerifyAndClearExpectations(&callModeMock);
    
    // expect - no valid phone number entered
    EXPECT_CALL(userPortMock, setCallMode()).WillOnce(ReturnRef(callModeMock));
    EXPECT_CALL(callModeMock, getOutgoingText()).WillOnce(Return(""));
    EXPECT_CALL(userPortMock, showViewTextMode()).WillOnce(ReturnRef(textModeMock));
    EXPECT_CALL(textModeMock, setText(HasSubstr("Invalid phone number")));
    EXPECT_CALL(userPortMock, showConnected());
    
    // should not send call request
    EXPECT_CALL(btsPortMock, sendCallRequest(_)).Times(0);
    
    // when
    acceptCallback();
}

TEST_F(DiallingStateTestSuite, shallTransitionToTalkingStateWhenCallAccepted)
{
    // given
    DiallingState objectUnderTest{context};
    ::testing::Mock::VerifyAndClearExpectations(&userPortMock);
    ::testing::Mock::VerifyAndClearExpectations(&callModeMock);
    
    // Set up the state by simulating a call request
    EXPECT_CALL(userPortMock, setCallMode()).WillOnce(ReturnRef(callModeMock));
    EXPECT_CALL(callModeMock, getOutgoingText()).WillOnce(Return(std::to_string(RECIPIENT_NUMBER.value)));
    EXPECT_CALL(userPortMock, showViewTextMode()).WillOnce(ReturnRef(textModeMock));
    EXPECT_CALL(textModeMock, setText(_));
    EXPECT_CALL(btsPortMock, sendCallRequest(_));
    acceptCallback();
    ::testing::Mock::VerifyAndClearExpectations(&userPortMock);
    ::testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    
    // For TalkingState constructor
    ON_CALL(userPortMock, setCallMode()).WillByDefault(ReturnRef(callModeMock));
    
    // expect for the transition to TalkingState
    EXPECT_CALL(userPortMock, setCallMode());
    EXPECT_CALL(callModeMock, clearIncomingText());
    EXPECT_CALL(callModeMock, clearOutgoingText());
    EXPECT_CALL(userPortMock, setAcceptCallback(_));
    EXPECT_CALL(userPortMock, setRejectCallback(_));
    EXPECT_CALL(timerPortMock, stopTimer());
    
    // when - call is accepted by recipient
    objectUnderTest.handleCallAccepted(RECIPIENT_NUMBER);
}

TEST_F(DiallingStateTestSuite, shallHandleCallDroppedByRecipient)
{
    // given
    DiallingState objectUnderTest{context};
    ::testing::Mock::VerifyAndClearExpectations(&userPortMock);
    ::testing::Mock::VerifyAndClearExpectations(&callModeMock);
    
    // Set up the state by simulating a call request
    EXPECT_CALL(userPortMock, setCallMode()).WillOnce(ReturnRef(callModeMock));
    EXPECT_CALL(callModeMock, getOutgoingText()).WillOnce(Return(std::to_string(RECIPIENT_NUMBER.value)));
    EXPECT_CALL(userPortMock, showViewTextMode()).WillOnce(ReturnRef(textModeMock));
    EXPECT_CALL(textModeMock, setText(_));
    EXPECT_CALL(btsPortMock, sendCallRequest(_));
    acceptCallback();
    ::testing::Mock::VerifyAndClearExpectations(&userPortMock);
    ::testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    
    // expect
    EXPECT_CALL(userPortMock, showViewTextMode()).WillOnce(ReturnRef(textModeMock));
    EXPECT_CALL(textModeMock, setText(HasSubstr("Call rejected by: " + to_string(RECIPIENT_NUMBER))));
    EXPECT_CALL(userPortMock, showConnected());
    EXPECT_CALL(timerPortMock, stopTimer());
    
    // when - call is rejected by recipient
    objectUnderTest.handleCallDropped(RECIPIENT_NUMBER);
}

TEST_F(DiallingStateTestSuite, shallHandleUnknownRecipient)
{
    // given
    DiallingState objectUnderTest{context};
    ::testing::Mock::VerifyAndClearExpectations(&userPortMock);
    ::testing::Mock::VerifyAndClearExpectations(&callModeMock);
    
    // Set up the state by simulating a call request
    EXPECT_CALL(userPortMock, setCallMode()).WillOnce(ReturnRef(callModeMock));
    EXPECT_CALL(callModeMock, getOutgoingText()).WillOnce(Return(std::to_string(RECIPIENT_NUMBER.value)));
    EXPECT_CALL(userPortMock, showViewTextMode()).WillOnce(ReturnRef(textModeMock));
    EXPECT_CALL(textModeMock, setText(_));
    EXPECT_CALL(btsPortMock, sendCallRequest(_));
    acceptCallback();
    ::testing::Mock::VerifyAndClearExpectations(&userPortMock);
    ::testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    
    // expect
    EXPECT_CALL(userPortMock, showViewTextMode()).WillOnce(ReturnRef(textModeMock));
    EXPECT_CALL(textModeMock, setText(HasSubstr("User not available")));
    EXPECT_CALL(userPortMock, showConnected());
    EXPECT_CALL(timerPortMock, stopTimer());
    
    // when - unknown recipient response received
    objectUnderTest.handleUnknownRecipient();
}

TEST_F(DiallingStateTestSuite, shallHandleTimeout)
{
    // given
    DiallingState objectUnderTest{context};
    ::testing::Mock::VerifyAndClearExpectations(&userPortMock);
    ::testing::Mock::VerifyAndClearExpectations(&callModeMock);
    
    // Set up the state by simulating a call request
    EXPECT_CALL(userPortMock, setCallMode()).WillOnce(ReturnRef(callModeMock));
    EXPECT_CALL(callModeMock, getOutgoingText()).WillOnce(Return(std::to_string(RECIPIENT_NUMBER.value)));
    EXPECT_CALL(userPortMock, showViewTextMode()).WillOnce(ReturnRef(textModeMock));
    EXPECT_CALL(textModeMock, setText(_));
    EXPECT_CALL(btsPortMock, sendCallRequest(_));
    acceptCallback();
    ::testing::Mock::VerifyAndClearExpectations(&userPortMock);
    ::testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    
    // expect
    EXPECT_CALL(userPortMock, showViewTextMode()).WillOnce(ReturnRef(textModeMock));
    EXPECT_CALL(textModeMock, setText(HasSubstr("Call timed out")));
    EXPECT_CALL(btsPortMock, sendCallDropped(RECIPIENT_NUMBER));
    EXPECT_CALL(userPortMock, showConnected());
    EXPECT_CALL(timerPortMock, stopTimer());
    
    // when
    objectUnderTest.handleTimeout();
}

TEST_F(DiallingStateTestSuite, shallCancelCallWhenUserRejects)
{
    // given
    DiallingState objectUnderTest{context};
    ::testing::Mock::VerifyAndClearExpectations(&userPortMock);
    ::testing::Mock::VerifyAndClearExpectations(&callModeMock);
    
    // Set up the state by simulating a call request
    EXPECT_CALL(userPortMock, setCallMode()).WillOnce(ReturnRef(callModeMock));
    EXPECT_CALL(callModeMock, getOutgoingText()).WillOnce(Return(std::to_string(RECIPIENT_NUMBER.value)));
    EXPECT_CALL(userPortMock, showViewTextMode()).WillOnce(ReturnRef(textModeMock));
    EXPECT_CALL(textModeMock, setText(_));
    EXPECT_CALL(btsPortMock, sendCallRequest(_));
    acceptCallback();
    ::testing::Mock::VerifyAndClearExpectations(&userPortMock);
    ::testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    
    // expect
    EXPECT_CALL(btsPortMock, sendCallDropped(RECIPIENT_NUMBER));
    EXPECT_CALL(userPortMock, showConnected());
    EXPECT_CALL(timerPortMock, stopTimer());
    
    // when
    rejectCallback();
}

TEST_F(DiallingStateTestSuite, shallCancelCallOnHomeButton)
{
    // given
    DiallingState objectUnderTest{context};
    ::testing::Mock::VerifyAndClearExpectations(&userPortMock);
    ::testing::Mock::VerifyAndClearExpectations(&callModeMock);
    
    // Set up the state by simulating a call request
    EXPECT_CALL(userPortMock, setCallMode()).WillOnce(ReturnRef(callModeMock));
    EXPECT_CALL(callModeMock, getOutgoingText()).WillOnce(Return(std::to_string(RECIPIENT_NUMBER.value)));
    EXPECT_CALL(userPortMock, showViewTextMode()).WillOnce(ReturnRef(textModeMock));
    EXPECT_CALL(textModeMock, setText(_));
    EXPECT_CALL(btsPortMock, sendCallRequest(_));
    acceptCallback();
    ::testing::Mock::VerifyAndClearExpectations(&userPortMock);
    ::testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    
    // expect
    EXPECT_CALL(btsPortMock, sendCallDropped(RECIPIENT_NUMBER));
    EXPECT_CALL(userPortMock, showConnected());
    EXPECT_CALL(timerPortMock, stopTimer());
    
    // when
    objectUnderTest.handleHomeClicked();
}

}