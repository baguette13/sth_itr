#pragma once

#include "Messages/PhoneNumber.hpp"
#include <string>
#include <vector>

namespace ue
{

using common::PhoneNumber;

struct SmsMessage
{
    PhoneNumber from;
    std::string text;
    bool isRead = false;
};

class SmsDb
{
public:
    void addSms(PhoneNumber from, const std::string& text);
    bool hasUnreadSms() const;
    std::vector<SmsMessage> getSmsMessages() const;
    void markAsRead(size_t index);

private:
    std::vector<SmsMessage> messages;
};

}