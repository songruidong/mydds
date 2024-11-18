#include "topicmanager.h"

// 创建 Topic
std::shared_ptr<Topic> TopicManager::insertTopic(const std::string &name, std::shared_ptr<Topic> topic)
{
    // std::lock_guard<std::mutex> lock(manager_mutex); // 加锁
    if (topics.find(name).size()!= 0)
    {
        throw std::runtime_error("Topic already exists: " + name);
    }
    topic->setName(name);
    topics.insert(name, topic);
    return topic;
}

// 获取 Topic
std::vector<std::shared_ptr<Topic>> TopicManager::getTopics(const std::string &name)
{
    // std::lock_guard<std::mutex> lock(manager_mutex); // 加锁
    auto matches = topics.find(name);
    if (matches.size() == 0)
    {
        // throw std::runtime_error("Topic not found: " + name);
        return {};
    }
    return matches;
}

// 删除 Topic
void TopicManager::eraseTopic(const std::string &name)
{
    // std::lock_guard<std::mutex> lock(manager_mutex); // 加锁
    topics.erase(name);
}

// // 列出所有 Topic 名称
// std::vector<std::string> TopicManager::listTopics() const
// {
//     // std::lock_guard<std::mutex> lock(manager_mutex); // 加锁
//     std::vector<std::string> names;
//     for (const auto &[name, _] : topics)
//     {
//         names.push_back(name);
//     }
//     return names;
// }
