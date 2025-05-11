#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Application.hpp"
#include "Mocks/ILoggerMock.hpp"
#include "Mocks/IBtsPortMock.hpp"
#include "Mocks/IUserPortMock.hpp"
#include "Mocks/ITimerPortMock.hpp"
#include "Messages/PhoneNumber.hpp"
#include <memory>

namespace ue
{
using namespace ::testing;

struct ApplicationTestSuite : Test
{
    const common::PhoneNumber PHONE_NUMBER{112};
    const common::BtsId BTS_ID{1024};
    NiceMock<common::ILoggerMock> loggerMock;
    StrictMock<IBtsPortMock> btsPortMock;
    StrictMock<IUserPortMock> userPortMock;
    StrictMock<ITimerPortMock> timerPortMock;

    Expectation showNotConnected = EXPECT_CALL(userPortMock, showNotConnected());
    Application objectUnderTest{PHONE_NUMBER,
                                loggerMock,
                                btsPortMock,
                                userPortMock,
                                timerPortMock};
};

struct ApplicationNotConnectedTestSuite : ApplicationTestSuite
{
    void sendAttachRequestOnSib();
};

void ApplicationNotConnectedTestSuite::sendAttachRequestOnSib()
{
    using namespace std::chrono_literals;
    EXPECT_CALL(btsPortMock, sendAttachRequest(BTS_ID));
    EXPECT_CALL(timerPortMock, startTimer(500ms));
    EXPECT_CALL(userPortMock, showConnecting());
    objectUnderTest.handleSib(BTS_ID);
}

TEST_F(ApplicationNotConnectedTestSuite, shallSendAttachRequestOnSib)
{
    sendAttachRequestOnSib();
}

class ApplicationConnectingTestSuite : public ApplicationTestSuite
{
protected:
    NiceMock<IListViewModeMock> listViewModeMock;
    
    ApplicationConnectingTestSuite()
    {
        // Allow states to transition properly during connect
        ON_CALL(userPortMock, getListViewMode()).WillByDefault(ReturnRef(listViewModeMock));
        
        // Setup initial state for test - connecting
        // Using handleSib to transition to connecting state
        EXPECT_CALL(btsPortMock, sendAttachRequest(BTS_ID));
        EXPECT_CALL(timerPortMock, startTimer(_));
        EXPECT_CALL(userPortMock, showConnecting());
        objectUnderTest.handleSib(BTS_ID);
        ::testing::Mock::VerifyAndClearExpectations(&userPortMock);
        ::testing::Mock::VerifyAndClearExpectations(&btsPortMock);
        ::testing::Mock::VerifyAndClearExpectations(&timerPortMock);
    }
};

TEST_F(ApplicationConnectingTestSuite, shallConnectOnAttachAccept)
{
    EXPECT_CALL(timerPortMock, stopTimer());
    EXPECT_CALL(userPortMock, showConnected());
    
    // These are called internally by ConnectedState constructor
    EXPECT_CALL(userPortMock, getListViewMode()).Times(AnyNumber()).WillRepeatedly(ReturnRef(listViewModeMock));
    EXPECT_CALL(userPortMock, setAcceptCallback(_)).Times(AnyNumber());
    EXPECT_CALL(listViewModeMock, clearSelectionList()).Times(AnyNumber());
    EXPECT_CALL(listViewModeMock, addSelectionListItem(_, _)).Times(AnyNumber());
    
    objectUnderTest.handleAttachAccept();
}

TEST_F(ApplicationConnectingTestSuite, shallDisConnectOnAttachReject)
{
    EXPECT_CALL(timerPortMock, stopTimer());
    EXPECT_CALL(userPortMock, showNotConnected());
    objectUnderTest.handleAttachReject();
}

TEST_F(ApplicationConnectingTestSuite, shallDisConnectOnTimeout)
{
    EXPECT_CALL(userPortMock, showNotConnected());
    objectUnderTest.handleTimeout();
}

}
