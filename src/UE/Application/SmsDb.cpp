#include "SmsDb.hpp"

namespace ue
{

void SmsDb::addSms(PhoneNumber from, const std::string& text)
{
    SmsMessage message;
    message.from = from;
    message.to = PhoneNumber{};
    message.text = text;
    message.isRead = false;
    message.isSent = false;
    messages.push_back(message);
}

void SmsDb::addSentSms(PhoneNumber to, const std::string& text)
{
    SmsMessage message;
    message.from = PhoneNumber{};  // From field is empty for sent messages
    message.to = to;
    message.text = text;
    message.isRead = true;         // Sent messages are always "read"
    message.isSent = true;
    messages.push_back(message);
}

bool SmsDb::hasUnreadSms() const
{
    for (const auto& message : messages)
    {
        if (!message.isRead)
        {
            return true;
        }
    }
    return false;
}

std::vector<SmsMessage> SmsDb::getSmsMessages() const
{
    return messages;
}

std::vector<SmsMessage> SmsDb::getSentMessages() const
{
    std::vector<SmsMessage> sentMessages;
    for (const auto& message : messages)
    {
        if (message.isSent)
        {
            sentMessages.push_back(message);
        }
    }
    return sentMessages;
}

void SmsDb::markAsRead(size_t index)
{
    if (index < messages.size())
    {
        messages[index].isRead = true;
    }
}

}