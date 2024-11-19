#include <gtest/gtest.h>
#include "topicmanager.h"

TEST(TopicManagerTest, InsertAndGetTopic) {
    TopicManager manager;

    std::shared_ptr<Topic> topic1 = std::make_shared<Topic>("Topic1");
    std::shared_ptr<Topic> insertedTopic1 = manager.insertTopic("Topic1", topic1);

    // 验证插入成功
    EXPECT_EQ(insertedTopic1, topic1);

    std::vector<std::shared_ptr<Topic>> topics = manager.getTopics("Topic1");

    // 验证获取到的主题与插入的主题一致
    EXPECT_EQ(topics.size(), 1);
    EXPECT_EQ(topics[0], topic1);
}

TEST(TopicManagerTest, EraseTopic) {
    TopicManager manager;

    std::shared_ptr<Topic> topic1 = std::make_shared<Topic>("Topic1");
    manager.insertTopic("Topic1", topic1);

    manager.eraseTopic("Topic1");

    std::vector<std::shared_ptr<Topic>> topics = manager.getTopics("Topic1");

    // 验证主题已被删除
    EXPECT_EQ(topics.size(), 0);
}

// 可以继续添加其他测试用例来覆盖更多场景
