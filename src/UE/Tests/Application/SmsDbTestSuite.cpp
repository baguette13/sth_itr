#include <gtest/gtest.h>
#include "SmsDb.hpp"

namespace ue
{
using namespace ::testing;

class SmsDbTestSuite : public Test
{
protected:
    SmsDb objectUnderTest;
};

TEST_F(SmsDbTestSuite, shallAddSms)
{
    // given
    const common::PhoneNumber sender{123};
    const std::string text = "Test Message";
    
    // when
    objectUnderTest.addSms(sender, text);
    auto messages = objectUnderTest.getSmsMessages();
    
    // then
    ASSERT_EQ(1u, messages.size());
    EXPECT_EQ(sender, messages[0].from);
    EXPECT_EQ(text, messages[0].text);
    EXPECT_FALSE(messages[0].isRead);
}

TEST_F(SmsDbTestSuite, shallReportUnreadMessages)
{
    // given
    const common::PhoneNumber sender{123};
    const std::string text = "Test Message";
    
    // when
    objectUnderTest.addSms(sender, text);
    
    // then
    EXPECT_TRUE(objectUnderTest.hasUnreadSms());
}

TEST_F(SmsDbTestSuite, shallMarkMessageAsRead)
{
    // given
    const common::PhoneNumber sender{123};
    const std::string text = "Test Message";
    objectUnderTest.addSms(sender, text);
    
    // when
    objectUnderTest.markAsRead(0);
    
    // then
    auto messages = objectUnderTest.getSmsMessages();
    ASSERT_EQ(1u, messages.size());
    EXPECT_TRUE(messages[0].isRead);
    EXPECT_FALSE(objectUnderTest.hasUnreadSms());
}

TEST_F(SmsDbTestSuite, shallHandleMultipleMessages)
{
    // given
    const common::PhoneNumber sender1{123};
    const common::PhoneNumber sender2{234};
    const std::string text1 = "Message 1";
    const std::string text2 = "Message 2";
    
    // when
    objectUnderTest.addSms(sender1, text1);
    objectUnderTest.addSms(sender2, text2);
    
    // then
    auto messages = objectUnderTest.getSmsMessages();
    ASSERT_EQ(2u, messages.size());
    EXPECT_EQ(sender1, messages[0].from);
    EXPECT_EQ(text1, messages[0].text);
    EXPECT_EQ(sender2, messages[1].from);
    EXPECT_EQ(text2, messages[1].text);
}

TEST_F(SmsDbTestSuite, shallIgnoreInvalidIndexWhenMarkingAsRead)
{
    // given
    const common::PhoneNumber sender{123};
    const std::string text = "Test Message";
    objectUnderTest.addSms(sender, text);
    
    // when - try to mark a non-existent message as read
    objectUnderTest.markAsRead(999);
    
    // then - nothing should change
    auto messages = objectUnderTest.getSmsMessages();
    ASSERT_EQ(1u, messages.size());
    EXPECT_FALSE(messages[0].isRead);
    EXPECT_TRUE(objectUnderTest.hasUnreadSms());
}

}