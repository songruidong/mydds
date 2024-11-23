#include "communicationtype.h"
#include "ioinfo.h"
std::string toString(CommunicationType type)
{
    switch (type)
    {
        case CommunicationType::Discover:
            return "Discover";
        case CommunicationType::Publish:
            return "Publish";
        case CommunicationType::Subscribe:
            return "Subscribe";
        default:
            return "Unknown";
    }
}
std::string toString(EventType type)
{
    switch (type)
    {
        case EventType::Multicast:
            return "Multicast";
        case EventType::Unicast:
            return "Unicast";
        case EventType::Read:
            return "Read";
        case EventType::Timer:
            return "Timer   ";
        default:
            return "Unknown";
    }
}