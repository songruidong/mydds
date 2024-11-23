#include "communicationtype.h"
std::string toString(CommunicationType type) {
    switch (type) {
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