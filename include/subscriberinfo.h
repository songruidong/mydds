#ifndef __SUBSCRIBERINFO_H__
#define __SUBSCRIBERINFO_H__
#include <string>
class SubscriberInfo
{
  public:
    std::string ipaddr;
    int port;
    SubscriberInfo() = default;
    SubscriberInfo(std::string ipaddr, int port) : ipaddr(ipaddr), port(port) {}
    ~SubscriberInfo() = default;
};

#endif  // __SUBSCRIBERINFO_H__