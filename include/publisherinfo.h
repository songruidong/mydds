#ifndef __PUBLISHERINFO_H__
#define __PUBLISHERINFO_H__


#include <string>
class PublisherInfo
{
  public:
    std::string ipaddr;
    int port;
    PublisherInfo() = default;
    PublisherInfo(std::string ipaddr, int port) : ipaddr(ipaddr), port(port) {}
    ~PublisherInfo() = default;
};


#endif // __PUBLISHERINFO_H__