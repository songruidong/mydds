#include <gtest/gtest.h>
#include "topicmanager.h"

TEST(TopicManagerTest, InsertTopic) {
    TopicManager manager;

    std::shared_ptr<Topic> topic1 = std::make_shared<Topic>("topic1");
    std::shared_ptr<Topic> insertedTopic1 = manager.insertTopic("topic1", topic1);

    // 验证插入成功
    EXPECT_EQ(insertedTopic1, topic1);

    // 尝试插入已存在的主题，应抛出异常
    std::shared_ptr<Topic> topic2 = std::make_shared<Topic>("topic1");
    EXPECT_THROW(manager.insertTopic("topic1", topic2), std::runtime_error);
}

TEST(TopicManagerTest, GetTopics) {
    TopicManager manager;

    std::shared_ptr<Topic> topic1 = std::make_shared<Topic>("topic1");
    manager.insertTopic("topic1", topic1);

    std::vector<std::shared_ptr<Topic>> topics = manager.getTopics("topic1");

    // 验证获取到的主题
    EXPECT_EQ(topics.size(), 1);
    EXPECT_EQ(topics[0], topic1);

    // 获取不存在的主题，应返回空向量
    std::vector<std::shared_ptr<Topic>> emptyTopics = manager.getTopics("topic2");
    EXPECT_EQ(emptyTopics.size(), 0);
}

TEST(TopicManagerTest, EraseTopic) {
    TopicManager manager;

    std::shared_ptr<Topic> topic1 = std::make_shared<Topic>("topic1");
    manager.insertTopic("topic1", topic1);

    manager.eraseTopic("topic1");

    // 验证主题已被删除
    std::vector<std::shared_ptr<Topic>> topics = manager.getTopics("topic1");
    EXPECT_EQ(topics.size(), 0);
}

// TEST(TopicManagerTest, ListTopics) {
//     TopicManager manager;

//     std::shared_ptr<Topic> topic1 = std::make_shared<Topic>("topic1");
//     std::shared_ptr<Topic> topic2 = std::make_shared<Topic>("topic2");

//     manager.insertTopic("topic1", topic1);
//     manager.insertTopic("topic2", topic2);

//     std::vector<std::string> topicNames = manager.listTopics();

//     // 验证列出的主题名称
//     EXPECT_EQ(topicNames.size(), 2);
//     EXPECT_TRUE(std::find(topicNames.begin(), topicNames.end(), "topic1")!= topicNames.end());
//     EXPECT_TRUE(std::find(topicNames.begin(), topicNames.end(), "topic2")!= topicNames.end());
// }
