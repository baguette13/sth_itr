#include "SmsDb.hpp"

namespace ue
{

void SmsDb::addSms(PhoneNumber from, const std::string& text)
{
    messages.push_back({from, text, false});
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

void SmsDb::markAsRead(size_t index)
{
    if (index < messages.size())
    {
        messages[index].isRead = true;
    }
}

}