#include <gtest/gtest.h>
#include "packet.h"

TEST(PacketTest, DDSHeaderPackAndUnpack) {
    DDSHeader header;
    header.type = 1;

    std::vector<uint8_t> packed = header.Pack();
    EXPECT_EQ(packed.size(), 1);
    EXPECT_EQ(packed[0], 1);

    DDSHeader unpackedHeader;
    unpackedHeader.Unpack(packed);
    EXPECT_EQ(unpackedHeader.type, 1);
}

TEST(PacketTest, TopicNamePackAndUnpack) {
    TopicName topic;
    topic.length = 5;
    topic.name = "TestTopic";

    std::vector<uint8_t> packed = topic.Pack();

    TopicName unpackedTopic;
    unpackedTopic.Unpack(packed);

    EXPECT_EQ(unpackedTopic.length, 5);
    EXPECT_EQ(unpackedTopic.name, "TestTopic");
}

TEST(PacketTest, TopicDataPackAndUnpack) {
    TopicData data;
    data.length = 10;
    data.data = "TestData";

    std::vector<uint8_t> packed = data.Pack();

    TopicData unpackedData;
    unpackedData.Unpack(packed);

    EXPECT_EQ(unpackedData.length, 10);
    EXPECT_EQ(unpackedData.data, "TestData");
}

TEST(PacketTest, TopicPacketPackAndUnpack) {
    TopicPacket packet;
    packet.name.length = 5;
    packet.name.name = "TestTopic";
    packet.data.length = 10;
    packet.data.data = "TestData";

    std::vector<uint8_t> packed = packet.Pack();

    TopicPacket unpackedPacket;
    unpackedPacket.Unpack(packed);

    EXPECT_EQ(unpackedPacket.name.length, 5);
    EXPECT_EQ(unpackedPacket.name.name, "TestTopic");
    EXPECT_EQ(unpackedPacket.data.length, 10);
    EXPECT_EQ(unpackedPacket.data.data, "TestData");
}

TEST(PacketTest, PublishDataPackAndUnpack) {
    PublishData publishData;
    TopicPacket packet1;
    packet1.name.length = 5;
    packet1.name.name = "TestTopic1";
    packet1.data.length = 10;
    packet1.data.data = "TestData1";
    publishData.data.push_back(packet1);

    TopicPacket packet2;
    packet2.name.length = 6;
    packet2.name.name = "TestTopic2";
    packet2.data.length = 11;
    packet2.data.data = "TestData2";
    publishData.data.push_back(packet2);

    std::vector<uint8_t> packed = publishData.Pack();

    PublishData unpackedPublishData;
    unpackedPublishData.Unpack(packed);

    EXPECT_EQ(unpackedPublishData.data.size(), 2);

    EXPECT_EQ(unpackedPublishData.data[0].name.length, 5);
    EXPECT_EQ(unpackedPublishData.data[0].name.name, "TestTopic1");
    EXPECT_EQ(unpackedPublishData.data[0].data.length, 10);
    EXPECT_EQ(unpackedPublishData.data[0].data.data, "TestData1");

    EXPECT_EQ(unpackedPublishData.data[1].name.length, 6);
    EXPECT_EQ(unpackedPublishData.data[1].name.name, "TestTopic2");
    EXPECT_EQ(unpackedPublishData.data[1].data.length, 11);
    EXPECT_EQ(unpackedPublishData.data[1].data.data, "TestData2");
}

TEST(PacketTest, SubscribeDataPackAndUnpack) {
    SubscribeData subscribeData;
    TopicName topic1;
    topic1.length = 5;
    topic1.name = "TestTopic1";
    subscribeData.data.push_back(topic1);

    TopicName topic2;
    topic2.length = 6;
    topic2.name = "TestTopic2";
    subscribeData.data.push_back(topic2);

    std::vector<uint8_t> packed = subscribeData.Pack();

    SubscribeData unpackedSubscribeData;
    unpackedSubscribeData.Unpack(packed);

    EXPECT_EQ(unpackedSubscribeData.data.size(), 2);

    EXPECT_EQ(unpackedSubscribeData.data[0].length, 5);
    EXPECT_EQ(unpackedSubscribeData.data[0].name, "TestTopic1");

    EXPECT_EQ(unpackedSubscribeData.data[1].length, 6);
    EXPECT_EQ(unpackedSubscribeData.data[1].name, "TestTopic2");
}

TEST(PacketTest, DDSPacketPackAndUnpack) {
    DDSPacket packet;
    packet.header.type = 1;

    PublishData publishData;
    TopicPacket packet1;
    packet1.name.length = 5;
    packet1.name.name = "TestTopic1";
    packet1.data.length = 10;
    packet1.data.data = "TestData1";
    publishData.data.push_back(packet1);

    packet.DDSData = publishData;

    std::vector<uint8_t> packed = packet.Pack();

    DDSPacket unpackedPacket;
    unpackedPacket.Unpack(packed);

    EXPECT_EQ(unpackedPacket.header.type, 1);

    auto publishDataVariant = std::get<PublishData>(unpackedPacket.DDSData);

    EXPECT_EQ(publishDataVariant.data.size(), 1);

    EXPECT_EQ(publishDataVariant.data[0].name.length, 5);
    EXPECT_EQ(publishDataVariant.data[0].name.name, "TestTopic1");
    EXPECT_EQ(publishDataVariant.data[0].data.length, 10);
    EXPECT_EQ(publishDataVariant.data[0].data.data, "TestData1");
}
