#include "../include/packet.h"
#include <gtest/gtest.h>
#include <iostream>

TEST(PacketTest, DDSHeaderPackUnpack) {
    DDSHeader header;
    header.type = 1;

    std::vector<uint8_t> packed = header.Pack();
    EXPECT_EQ(packed.size(), 1);
    EXPECT_EQ(packed[0], 1);

    DDSHeader unpackedHeader;
    unpackedHeader.Unpack(packed.begin(),packed.end());
    EXPECT_EQ(unpackedHeader.type, 1);
}

TEST(PacketTest, TopicNamePackUnpack) {
    TopicName topicName;
    topicName.length = 4;
    topicName.name = "Test";

    std::vector<uint8_t> packed = topicName.Pack();
    EXPECT_EQ(packed.size(), 5);

    TopicName unpackedTopicName;
    unpackedTopicName.Unpack(packed.begin(),packed.end());
    EXPECT_EQ(unpackedTopicName.length, 4);
    EXPECT_EQ(unpackedTopicName.name, "Test");
}

TEST(PacketTest, TopicDataPackUnpack) {
    TopicData topicData;
    topicData.length = 8;
    topicData.data = "TestData";

    std::vector<uint8_t> packed = topicData.Pack();
    GTEST_LOG_(INFO) << packed.size(); 
    EXPECT_EQ(packed.size(), 9);

    TopicData unpackedTopicData;
    unpackedTopicData.Unpack(packed.begin(),packed.end());
    EXPECT_EQ(unpackedTopicData.length, 8);
    EXPECT_EQ(unpackedTopicData.data, "TestData");
}

TEST(PacketTest, TopicPacketPackUnpack) {
    TopicPacket topicPacket;
    topicPacket.name.length = 5;
    topicPacket.name.name = "Test1";
    topicPacket.data.length = 10;
    topicPacket.data.data = "TestData10";

    std::vector<uint8_t> packed = topicPacket.Pack();
    EXPECT_EQ(packed.size(), 17);

    TopicPacket unpackedTopicPacket;
    unpackedTopicPacket.Unpack(packed.begin(),packed.end());
    EXPECT_EQ(unpackedTopicPacket.name.length, 5);
    EXPECT_EQ(unpackedTopicPacket.name.name, "Test1");
    EXPECT_EQ(unpackedTopicPacket.data.length, 10);
    EXPECT_EQ(unpackedTopicPacket.data.data, "TestData10");
}

TEST(PacketTest, PublishDataPackUnpack) {
    PublishData publishData;
    TopicPacket topicPacket1;
    topicPacket1.name.length = 5;
    topicPacket1.name.name = "Test1";
    topicPacket1.data.length = 10;
    topicPacket1.data.data = "TestData10";
    TopicPacket topicPacket2;
    topicPacket2.name.length = 5;
    topicPacket2.name.name = "Test2";
    topicPacket2.data.length = 10;
    topicPacket2.data.data = "TestData20";

    publishData.data.push_back(topicPacket1);
    publishData.data.push_back(topicPacket2);

    std::vector<uint8_t> packed = publishData.Pack();
    EXPECT_EQ(packed.size(), 34);

    PublishData unpackedPublishData;
    unpackedPublishData.Unpack(packed.begin(),packed.end());
    EXPECT_EQ(unpackedPublishData.data.size(), 2);
    EXPECT_EQ(unpackedPublishData.data[0].name.length, 5);
    EXPECT_EQ(unpackedPublishData.data[0].name.name, "Test1");
    EXPECT_EQ(unpackedPublishData.data[0].data.length, 10);
    EXPECT_EQ(unpackedPublishData.data[0].data.data, "TestData10");
    EXPECT_EQ(unpackedPublishData.data[1].name.length, 5);
    EXPECT_EQ(unpackedPublishData.data[1].name.name, "Test2");
    EXPECT_EQ(unpackedPublishData.data[1].data.length, 10);
    EXPECT_EQ(unpackedPublishData.data[1].data.data, "TestData20");
}

TEST(PacketTest, SubscribeDataPackUnpack) {
    SubscribeData subscribeData;
    TopicName topicName1;
    topicName1.length = 5;
    topicName1.name = "Test1";
    TopicName topicName2;
    topicName2.length = 5;
    topicName2.name = "Test2";

    subscribeData.data.push_back(topicName1);
    subscribeData.data.push_back(topicName2);

    std::vector<uint8_t> packed = subscribeData.Pack();
    EXPECT_EQ(packed.size(), 12);

    SubscribeData unpackedSubscribeData;
    unpackedSubscribeData.Unpack(packed.begin(),packed.end());
    EXPECT_EQ(unpackedSubscribeData.data.size(), 2);
    EXPECT_EQ(unpackedSubscribeData.data[0].length, 5);
    EXPECT_EQ(unpackedSubscribeData.data[0].name, "Test1");
    EXPECT_EQ(unpackedSubscribeData.data[1].length, 5);
    EXPECT_EQ(unpackedSubscribeData.data[1].name, "Test2");
}

TEST(PacketTest, DDSPacketPackUnpack) {
    DDSPacket ddsPacket;
    ddsPacket.header.type = 1;
    PublishData publishData;
    TopicPacket topicPacket1;
    topicPacket1.name.length = 5;
    topicPacket1.name.name = "Test1";
    topicPacket1.data.length = 10;
    topicPacket1.data.data = "TestData10";
    TopicPacket topicPacket2;
    topicPacket2.name.length = 5;
    topicPacket2.name.name = "Test2";
    topicPacket2.data.length = 10;
    topicPacket2.data.data = "TestData20";

    publishData.data.push_back(topicPacket1);
    publishData.data.push_back(topicPacket2);

    ddsPacket.DDSData = publishData;

    std::vector<uint8_t> packed = ddsPacket.Pack();
    EXPECT_EQ(packed.size(), 35);

    DDSPacket unpackedDDSPacket;
    unpackedDDSPacket.Unpack(packed.begin(),packed.end());
    EXPECT_EQ(unpackedDDSPacket.header.type, 1);
    EXPECT_EQ(std::holds_alternative<PublishData>(unpackedDDSPacket.DDSData),true);
    PublishData unpackedPublishData = std::get<PublishData>(unpackedDDSPacket.DDSData);
    EXPECT_EQ(unpackedPublishData.data.size(), 2);
    EXPECT_EQ(unpackedPublishData.data[0].name.length, 5);
    EXPECT_EQ(unpackedPublishData.data[0].name.name, "Test1");
    EXPECT_EQ(unpackedPublishData.data[0].data.length, 10);
    EXPECT_EQ(unpackedPublishData.data[0].data.data, "TestData10");
    EXPECT_EQ(unpackedPublishData.data[1].name.length, 5);
    EXPECT_EQ(unpackedPublishData.data[1].name.name, "Test2");
    EXPECT_EQ(unpackedPublishData.data[1].data.length, 10);
    EXPECT_EQ(unpackedPublishData.data[1].data.data, "TestData20");
}
