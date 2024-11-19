#ifndef __PACKET_H__
#define __PACKET_H__

#include <cstdint>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>
#include "utils.h"

// 基础打包解包接口
class Packet
{
  public:
    virtual std::vector<std::uint8_t> Pack() const                       = 0;
    virtual std::uint32_t Unpack(const std::vector<std::uint8_t> &bytes) = 0;
    virtual ~Packet()                                                    = default;
};

// DDS Header
class DDSHeader : public Packet
{
  public:
    std::uint8_t type;

    std::vector<std::uint8_t> Pack() const override { return {type}; }

    std::uint32_t Unpack(const std::vector<std::uint8_t> &bytes) override
    {
        if (bytes.empty())
        {
            throw std::invalid_argument("DDSHeader: insufficient bytes for unpacking.");
        }
        type = bytes[0];
        return 1;
    }
};

// TopicName
class TopicName : public Packet
{
  public:
    std::uint32_t length;
    std::string name;

    std::vector<std::uint8_t> Pack() const override
    {
        std::vector<std::uint8_t> ret = Util::pack_length(length);
        ret.insert(ret.end(), name.begin(), name.end());
        return ret;
    }

    std::uint32_t Unpack(const std::vector<std::uint8_t> &bytes) override
    {
        if (bytes.empty())
        {
            throw std::invalid_argument("TopicName: insufficient bytes for unpacking.");
        }
        auto processed_len = Util::unpack_length(bytes, this->length);
        if (processed_len + this->length > bytes.size())
        {
            throw std::invalid_argument("TopicName: unpack length exceeds byte size.");
        }
        name = std::string(bytes.begin() + processed_len, bytes.begin() + processed_len + this->length);
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

    std::uint32_t Unpack(const std::vector<std::uint8_t> &bytes) override
    {
        if (bytes.empty())
        {
            throw std::invalid_argument("TopicData: insufficient bytes for unpacking.");
        }
        auto processed_len = Util::unpack_length(bytes, this->length);
        if (processed_len + this->length > bytes.size())
        {
            throw std::invalid_argument("TopicData: unpack length exceeds byte size.");
        }
        data = std::string(bytes.begin() + processed_len, bytes.begin() + processed_len + this->length);
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

    std::uint32_t Unpack(const std::vector<std::uint8_t> &bytes) override
    {
        auto namelen = name.Unpack(bytes);
        auto datalen = data.Unpack({bytes.begin() + namelen, bytes.end()});
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

    std::uint32_t Unpack(const std::vector<std::uint8_t> &bytes) override
    {
        std::uint32_t total_processed = 0;
        std::size_t i                 = 0;

        while (i < bytes.size())
        {
            TopicPacket packet;
            auto processed = packet.Unpack({bytes.begin() + i, bytes.end()});
            data.push_back(packet);
            i += processed;
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

    std::uint32_t Unpack(const std::vector<std::uint8_t> &bytes) override
    {
        std::uint32_t total_processed = 0;
        std::size_t i                 = 0;

        while (i < bytes.size())
        {
            TopicName topic;
            auto processed = topic.Unpack({bytes.begin() + i, bytes.end()});
            data.push_back(topic);
            i += processed;
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
    std::variant<PublishData, SubscribeData, std::monostate> DDSData;

    std::vector<std::uint8_t> Pack() const override
    {
        std::vector<std::uint8_t> ret = header.Pack();

        if (std::holds_alternative<PublishData>(DDSData))
        {
            auto publish_data = std::get<PublishData>(DDSData).Pack();
            ret.insert(ret.end(), publish_data.begin(), publish_data.end());
        }
        else if (std::holds_alternative<SubscribeData>(DDSData))
        {
            auto subscribe_data = std::get<SubscribeData>(DDSData).Pack();
            ret.insert(ret.end(), subscribe_data.begin(), subscribe_data.end());
        }

        return ret;
    }

    std::uint32_t Unpack(const std::vector<std::uint8_t> &bytes) override
    {
        auto header_len = header.Unpack(bytes);
        auto payload    = std::vector<std::uint8_t>(bytes.begin() + header_len, bytes.end());

        if (header.type == 1)
        {
            PublishData pub_data;
            pub_data.Unpack(payload);
            DDSData = pub_data;
        }
        else if (header.type == 2)
        {
            SubscribeData sub_data;
            sub_data.Unpack(payload);
            DDSData = sub_data;
        }
        else
        {
            DDSData = std::monostate{};
        }

        return header_len + payload.size();
    }
};

#endif  // __PACKET_H__
