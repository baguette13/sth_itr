#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "States/TalkingState.hpp"
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

class TalkingStateTestSuite : public Test
{
protected:
    const common::PhoneNumber PEER_NUMBER{123};
    NiceMock<common::ILoggerMock> loggerMock;
    StrictMock<IBtsPortMock> btsPortMock;
    StrictMock<IUserPortMock> userPortMock;
    NiceMock<ITimerPortMock> timerPortMock; // Using NiceMock since timer isn't used in TalkingState
    StrictMock<ICallModeMock> callModeMock;
    StrictMock<ITextModeMock> textModeMock;
    
    Context context{loggerMock, btsPortMock, userPortMock, timerPortMock};
    IUeGui::Callback acceptCallback;
    IUeGui::Callback rejectCallback;
    
    TalkingStateTestSuite()
    {
        ON_CALL(userPortMock, setCallMode()).WillByDefault(ReturnRef(callModeMock));
        ON_CALL(userPortMock, showViewTextMode()).WillByDefault(ReturnRef(textModeMock));
        
        // Expect call mode setup during construction
        EXPECT_CALL(userPortMock, setCallMode());
        EXPECT_CALL(callModeMock, clearIncomingText());
        EXPECT_CALL(callModeMock, clearOutgoingText());
        
        // Store callbacks for testing
        EXPECT_CALL(userPortMock, setAcceptCallback(_))
            .WillOnce(SaveArg<0>(&acceptCallback));
        EXPECT_CALL(userPortMock, setRejectCallback(_))
            .WillOnce(SaveArg<0>(&rejectCallback));
    }
};

TEST_F(TalkingStateTestSuite, shallSetupCallInterface)
{
    // given - setup is done in constructor
    
    // when
    TalkingState objectUnderTest{context, PEER_NUMBER};
}

TEST_F(TalkingStateTestSuite, shallSendCallTalkOnAcceptWithText)
{
    // given
    const std::string TALK_TEXT = "Hello, this is a test call!";
    TalkingState objectUnderTest{context, PEER_NUMBER};
    ::testing::Mock::VerifyAndClearExpectations(&userPortMock);
    ::testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    ::testing::Mock::VerifyAndClearExpectations(&callModeMock);
    
    // expect
    EXPECT_CALL(userPortMock, setCallMode());
    EXPECT_CALL(callModeMock, getOutgoingText()).WillOnce(Return(TALK_TEXT));
    EXPECT_CALL(btsPortMock, sendCallTalk(PEER_NUMBER, TALK_TEXT));
    EXPECT_CALL(userPortMock, setCallMode());
    EXPECT_CALL(callModeMock, clearOutgoingText());
    
    // when
    acceptCallback();
}

TEST_F(TalkingStateTestSuite, shallNotSendCallTalkOnAcceptWithEmptyText)
{
    // given
    const std::string EMPTY_TEXT = "";
    TalkingState objectUnderTest{context, PEER_NUMBER};
    ::testing::Mock::VerifyAndClearExpectations(&userPortMock);
    ::testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    ::testing::Mock::VerifyAndClearExpectations(&callModeMock);
    
    // expect
    EXPECT_CALL(userPortMock, setCallMode());
    EXPECT_CALL(callModeMock, getOutgoingText()).WillOnce(Return(EMPTY_TEXT));
    // No call to sendCallTalk should happen
    EXPECT_CALL(btsPortMock, sendCallTalk(_, _)).Times(0);
    EXPECT_CALL(callModeMock, clearOutgoingText()).Times(0); // Nothing to clear
    
    // when
    acceptCallback();
}

TEST_F(TalkingStateTestSuite, shallEndCallOnReject)
{
    // given
    TalkingState objectUnderTest{context, PEER_NUMBER};
    ::testing::Mock::VerifyAndClearExpectations(&userPortMock);
    ::testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    
    // expect
    EXPECT_CALL(btsPortMock, sendCallDropped(PEER_NUMBER));
    
    // when
    rejectCallback();
    
    // Verify state transition (would require a mock for the setState method in Context)
    // In a real test we could check if context.state is now an instance of ConnectedState
}

TEST_F(TalkingStateTestSuite, shallEndCallOnHomeButton)
{
    // given
    TalkingState objectUnderTest{context, PEER_NUMBER};
    ::testing::Mock::VerifyAndClearExpectations(&userPortMock);
    ::testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    
    // expect
    EXPECT_CALL(btsPortMock, sendCallDropped(PEER_NUMBER));
    
    // when
    objectUnderTest.handleHomeClicked();
    
    // Verify state transition (would require a mock for the setState method in Context)
    // In a real test we could check if context.state is now an instance of ConnectedState
}

TEST_F(TalkingStateTestSuite, shallHandleReceivedCallTalkMessages)
{
    // given
    const std::string RECEIVED_TEXT = "Can you hear me?";
    TalkingState objectUnderTest{context, PEER_NUMBER};
    ::testing::Mock::VerifyAndClearExpectations(&userPortMock);
    ::testing::Mock::VerifyAndClearExpectations(&callModeMock);
    
    // expect
    EXPECT_CALL(userPortMock, setCallMode());
    EXPECT_CALL(callModeMock, appendIncomingText(RECEIVED_TEXT));
    
    // when
    objectUnderTest.handleCallTalk(PEER_NUMBER, RECEIVED_TEXT);
}

TEST_F(TalkingStateTestSuite, shallIgnoreCallTalkFromWrongNumber)
{
    // given
    const common::PhoneNumber WRONG_NUMBER{456};
    const std::string RECEIVED_TEXT = "Can you hear me?";
    TalkingState objectUnderTest{context, PEER_NUMBER};
    ::testing::Mock::VerifyAndClearExpectations(&userPortMock);
    ::testing::Mock::VerifyAndClearExpectations(&callModeMock);
    
    // expect - no calls to appendIncomingText
    EXPECT_CALL(callModeMock, appendIncomingText(_)).Times(0);
    
    // when
    objectUnderTest.handleCallTalk(WRONG_NUMBER, RECEIVED_TEXT);
}

TEST_F(TalkingStateTestSuite, shallHandleCallDroppedFromPeer)
{
    // given
    TalkingState objectUnderTest{context, PEER_NUMBER};
    ::testing::Mock::VerifyAndClearExpectations(&userPortMock);
    ::testing::Mock::VerifyAndClearExpectations(&textModeMock);
    
    // expect
    EXPECT_CALL(userPortMock, showViewTextMode());
    EXPECT_CALL(textModeMock, setText(HasSubstr("Call ended by: " + to_string(PEER_NUMBER))));
    
    // when
    objectUnderTest.handleCallDropped(PEER_NUMBER);
    
    // Verify state transition (would require a mock for the setState method in Context)
    // In a real test we could check if context.state is now an instance of ConnectedState
}

TEST_F(TalkingStateTestSuite, shallIgnoreCallDroppedFromWrongNumber)
{
    // given
    const common::PhoneNumber WRONG_NUMBER{456};
    TalkingState objectUnderTest{context, PEER_NUMBER};
    ::testing::Mock::VerifyAndClearExpectations(&userPortMock);
    
    // expect - no calls to showViewTextMode
    EXPECT_CALL(userPortMock, showViewTextMode()).Times(0);
    
    // when
    objectUnderTest.handleCallDropped(WRONG_NUMBER);
}

}