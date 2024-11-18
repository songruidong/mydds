#include <gtest/gtest.h>
#include "topic.hpp"
#include "TemperatureData.pb.h"

TEST(TopicTest, SetAndGetData) {
    Topic topic("TestTopic");

    // 正常设置和获取数据
    int key = 1;
    TemperatureData data;
    data.set_location("TestLocation");
    data.set_temperature(25.5);
    topic.setData(key, data);
    TemperatureData retrievedData = topic.getData<TemperatureData>(key);
    EXPECT_EQ(retrievedData.location(), "TestLocation");
    EXPECT_EQ(retrievedData.temperature(), 25.5);

    // 异常情况：设置负数键
    try {
        topic.setData(-1, data);
        FAIL() << "Expected std::invalid_argument exception";
    } catch (const std::invalid_argument& e) {
        EXPECT_EQ(e.what(), std::string("Key cannot be negative."));
    }
}

TEST(TopicTest, RemoveData) {
    Topic topic("TestTopic");

    int key = 1;
    TemperatureData data;
    data.set_location("TestLocation");
    data.set_temperature(25.5);
    topic.setData(key, data);

    // 正常删除数据
    topic.removeData(key);
    try {
        topic.getData<TemperatureData>(key);
        FAIL() << "Expected std::runtime_error exception";
    } catch (const std::runtime_error& e) {
        EXPECT_EQ(e.what(), std::string("Key not found: 1"));
    }
}

TEST(TopicTest, ListKeys) {
    Topic topic("TestTopic");

    int key1 = 1;
    int key2 = 2;
    TemperatureData data1, data2;
    data1.set_location("TestLocation1");
    data1.set_temperature(25.5);
    data2.set_location("TestLocation2");
    data2.set_temperature(26.5);
    topic.setData(key1, data1);
    topic.setData(key2, data2);

    std::vector<int> keys = topic.listKeys();
    EXPECT_EQ(keys.size(), 2);
    EXPECT_TRUE(std::find(keys.begin(), keys.end(), key1)!= keys.end());
    EXPECT_TRUE(std::find(keys.begin(), keys.end(), key2)!= keys.end());
}

TEST(TopicTest, GetName) {
    Topic topic("TestTopic");

    std::string name = topic.getName();
    EXPECT_EQ(name, "TestTopic");
}
