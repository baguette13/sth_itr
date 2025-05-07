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
    PhoneNumber to;      // recipient for sent messages
    std::string text;
    bool isRead = false;
    bool isSent = false; // flag to indicate if message was sent by this UE
};

class SmsDb
{
public:
    void addSms(PhoneNumber from, const std::string& text);
    void addSentSms(PhoneNumber to, const std::string& text);
    bool hasUnreadSms() const;
    std::vector<SmsMessage> getSmsMessages() const;
    std::vector<SmsMessage> getSentMessages() const;
    void markAsRead(size_t index);

private:
    std::vector<SmsMessage> messages;
};

}