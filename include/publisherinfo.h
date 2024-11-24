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
    PublisherInfo(std::string&& ipaddr, int&& port) : ipaddr(std::move(ipaddr)), port(port) {}
    PublisherInfo(std::pair<std::string, int> addr) : ipaddr(addr.first), port(addr.second) {}
    
    PublisherInfo(std::pair<std::string, int>&& addr) : ipaddr(std::move(addr.first)), port(addr.second) {}

    ~PublisherInfo() = default;
};


#endif // __PUBLISHERINFO_H__