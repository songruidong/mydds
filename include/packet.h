#ifndef __PACKET_H__
#define __PACKET_H__

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>
#include "communicationtype.h"
#include "topic.hpp"
#include "utils.h"
// 基础打包解包接口
class Packet
{
  public:
    virtual std::vector<std::uint8_t> Pack() const                              = 0;
    virtual std::uint32_t Unpack(std::vector<std::uint8_t>::const_iterator begin,
                                 std::vector<std::uint8_t>::const_iterator end) = 0;
    virtual ~Packet()                                                           = default;
};

// DDS Header
class DDSHeader : public Packet
{
  public:
    std::uint8_t type;
    DDSHeader() = default;
    DDSHeader(std::uint8_t type) : type(type) {}
    virtual ~DDSHeader() = default;
    std::vector<std::uint8_t> Pack() const override { return {type}; }

    std::uint32_t Unpack(std::vector<std::uint8_t>::const_iterator begin,
                         std::vector<std::uint8_t>::const_iterator end) override
    {
        if (begin == end)
        {
            throw std::invalid_argument("DDSHeader: insufficient bytes for unpacking.");
        }
        type = *begin;
        return 1;
    }
};

// TopicName
class TopicName : public Packet
{
  public:
    std::uint32_t length;
    std::string name;
    TopicName() = default;
    TopicName(std::uint32_t length, std::string name) : length(length), name(name) {}
    virtual ~TopicName() = default;
    std::vector<std::uint8_t> Pack() const override
    {
        std::vector<std::uint8_t> ret = Util::pack_length(length);
        ret.insert(ret.end(), name.begin(), name.end());
        return ret;
    }

    std::uint32_t Unpack(std::vector<std::uint8_t>::const_iterator begin,
                         std::vector<std::uint8_t>::const_iterator end) override
    {
        if (begin == end)
        {
            throw std::invalid_argument("TopicName: insufficient bytes for unpacking.");
        }
        auto processed_len = Util::unpack_length(begin, end, this->length);
        if (std::distance(begin, end) < processed_len + static_cast<int>(this->length))
        {
            throw std::invalid_argument("TopicName: unpack length exceeds byte size.");
        }
        name = std::string(begin + processed_len, begin + processed_len + this->length);
        return processed_len + this->length;
    }
};

// TopicData
class TopicData : public Packet
{
  public:
    std::uint32_t length;
    std::string data;

    std::vector<std::uint8_t> Pack() const override
    {
        std::vector<std::uint8_t> ret = Util::pack_length(length);
        ret.insert(ret.end(), data.begin(), data.end());
        return ret;
    }

    std::uint32_t Unpack(std::vector<std::uint8_t>::const_iterator begin,
                         std::vector<std::uint8_t>::const_iterator end) override
    {
        if (begin == end)
        {
            throw std::invalid_argument("TopicData: insufficient bytes for unpacking.");
        }
        auto processed_len = Util::unpack_length(begin, end, this->length);
        if (std::distance(begin, end) < processed_len + static_cast<int>(this->length))
        {
            throw std::invalid_argument("TopicData: unpack length exceeds byte size.");
        }
        data = std::string(begin + processed_len, begin + processed_len + this->length);
        return processed_len + this->length;
    }
};

// TopicPacket
class TopicPacket : public Packet
{
  public:
    TopicName name;
    TopicData data;

    std::vector<std::uint8_t> Pack() const override
    {
        std::vector<std::uint8_t> ret        = name.Pack();
        std::vector<std::uint8_t> data_bytes = data.Pack();
        ret.insert(ret.end(), data_bytes.begin(), data_bytes.end());
        return ret;
    }

    std::uint32_t Unpack(std::vector<std::uint8_t>::const_iterator begin,
                         std::vector<std::uint8_t>::const_iterator end) override
    {
        auto namelen = name.Unpack(begin, end);
        auto datalen = data.Unpack(begin + namelen, end);
        return namelen + datalen;
    }
};

// PublishData
class PublishData : public Packet
{
  public:
    std::vector<TopicPacket> data;

    std::vector<std::uint8_t> Pack() const override
    {
        std::vector<std::uint8_t> ret;
        for (const auto &packet : data)
        {
            auto packed_packet = packet.Pack();
            ret.insert(ret.end(), packed_packet.begin(), packed_packet.end());
        }
        return ret;
    }

    std::uint32_t Unpack(std::vector<std::uint8_t>::const_iterator begin,
                         std::vector<std::uint8_t>::const_iterator end) override
    {
        std::uint32_t total_processed = 0;
        while (begin != end)
        {
            TopicPacket packet;
            auto processed = packet.Unpack(begin, end);
            data.push_back(packet);
            begin += processed;
            total_processed += processed;
        }

        return total_processed;
    }
};

// SubscribeData
class SubscribeData : public Packet
{
  public:
    std::vector<TopicName> data;

    std::vector<std::uint8_t> Pack() const override
    {
        std::vector<std::uint8_t> ret;
        for (const auto &topic : data)
        {
            auto packed_topic = topic.Pack();
            ret.insert(ret.end(), packed_topic.begin(), packed_topic.end());
        }
        return ret;
    }

    std::uint32_t Unpack(std::vector<std::uint8_t>::const_iterator begin,
                         std::vector<std::uint8_t>::const_iterator end) override
    {
        std::uint32_t total_processed = 0;
        while (begin != end)
        {
            TopicName topic;
            auto processed = topic.Unpack(begin, end);
            data.push_back(topic);
            begin += processed;
            total_processed += processed;
        }

        return total_processed;
    }
};

// DiscoverData
class DiscoverData : public Packet
{
  public:
    std::vector<TopicName> data;

    std::vector<std::uint8_t> Pack() const override
    {
        std::vector<std::uint8_t> ret;
        for (const auto &topic : data)
        {
            auto packed_topic = topic.Pack();
            ret.insert(ret.end(), packed_topic.begin(), packed_topic.end());
        }
        return ret;
    }

    std::uint32_t Unpack(std::vector<std::uint8_t>::const_iterator begin,
                         std::vector<std::uint8_t>::const_iterator end) override
    {
        std::uint32_t total_processed = 0;
        while (begin != end)
        {
            TopicName topic;
            auto processed = topic.Unpack(begin, end);
            data.push_back(topic);
            begin += processed;
            total_processed += processed;
        }

        return total_processed;
    }
};

// DDSPacket
class DDSPacket : public Packet
{
  public:
    DDSHeader header;
    std::variant<DiscoverData, PublishData, SubscribeData> DDSData;

    std::vector<std::uint8_t> Pack() const override
    {
        std::vector<std::uint8_t> ret = header.Pack();

        switch (DDSData.index())
        {
            case 0:  // DiscoverData
            {
                auto discover_data = std::get<DiscoverData>(DDSData).Pack();
                ret.insert(ret.end(), discover_data.begin(), discover_data.end());
                break;
            }
            case 1:  // PublishData
            {
                auto publish_data = std::get<PublishData>(DDSData).Pack();
                ret.insert(ret.end(), publish_data.begin(), publish_data.end());
                break;
            }
            case 2:  // SubscribeData
            {
                auto subscribe_data = std::get<SubscribeData>(DDSData).Pack();
                ret.insert(ret.end(), subscribe_data.begin(), subscribe_data.end());
                break;
            }
            default:
                throw std::invalid_argument("Unknown packet type");
        }

        return ret;
    }

    std::uint32_t Unpack(std::vector<std::uint8_t>::const_iterator begin,
                         std::vector<std::uint8_t>::const_iterator end) override
    {
        auto header_len = header.Unpack(begin, end);
        auto payload    = begin + header_len;
        switch (static_cast<CommunicationType>(header.type))
        {
            case CommunicationType::Discover:
            {
                DiscoverData disc_data;
                disc_data.Unpack(payload, end);
                DDSData = disc_data;
                break;
            }
            case CommunicationType::Publish:
            {
                PublishData pub_data;
                pub_data.Unpack(payload, end);
                DDSData = pub_data;
                break;
            }
            case CommunicationType::Subscribe:
            {
                SubscribeData sub_data;
                sub_data.Unpack(payload, end);
                DDSData = sub_data;
                break;
            }
            default:
                throw std::invalid_argument("Unknown packet type");
        }

        return std::distance(begin, end);
    }
    ~DDSPacket() = default;
    static std::vector<std::uint8_t> create_publish_packet(const std::vector<std::shared_ptr<Topic>> &topics)
    {
        // auto topic_ptr = topics[0];
        PublishData publish_data;
        for (auto &topic_ptr : topics)
        {
            TopicPacket topicpacket;
            topicpacket.name.name   = topic_ptr->getName();
            topicpacket.name.length = topic_ptr->getName().size();
            topicpacket.data.data   = topic_ptr->getData(0);
            topicpacket.data.length = topic_ptr->getData(0).length();
            publish_data.data.push_back(std::move(topicpacket));
        }
        DDSPacket packet;
        packet.header.type = static_cast<uint8_t>(CommunicationType::Publish);
        packet.DDSData     = publish_data;
        return packet.Pack();
    }
    static std::vector<std::uint8_t> create_subscribe_packet(const std::vector<std::shared_ptr<Topic>> &topics)
    {
        // auto topic_ptr = topics[0];
        DDSPacket packet;
        packet.header.type = static_cast<uint8_t>(CommunicationType::Subscribe);
        SubscribeData subscribe_data;
        for (auto &topic_ptr : topics)
        {
            // TopicName topicname = ;
            subscribe_data.data.push_back(TopicName(topic_ptr->getName().size(), topic_ptr->getName()));
        }
        packet.DDSData = subscribe_data;
        return packet.Pack();
    }
    static std::vector<std::uint8_t> create_discover_packet(const std::vector<std::shared_ptr<Topic>> &topics) {
        DDSPacket packet;
        packet.header.type = static_cast<uint8_t>(CommunicationType::Discover);
        DiscoverData discover_data;
        for (auto &topic_ptr : topics)
        {
            // TopicName topicname = ;
            discover_data.data.push_back(TopicName(topic_ptr->getName().size(), topic_ptr->getName()));
        }
        packet.DDSData = discover_data;
        return packet.Pack();
    }
};

#endif  // __PACKET_H__
