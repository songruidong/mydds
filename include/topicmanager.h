#ifndef __TOPICMANAGER_H__
#define __TOPICMANAGER_H__

#include <memory>
#include <unordered_map>
#include <vector>
#include "topic.hpp"
#include "trie.h"

class TopicManager
{
  public:
    // 创建 Topic
    std::shared_ptr<Topic> insertTopic(const std::string &name, std::shared_ptr<Topic> topic = nullptr)
    {
        // std::lock_guard<std::mutex> lock(manager_mutex); // 加锁
        if (topics.find(name).size() != 0)
        {
            return nullptr;
        }
        if (topic != nullptr)
        {
            topic->setName(name);
        }
        topics.insert(name, topic);
        return topic;
    }

    // 获取 Topic
    std::vector<std::shared_ptr<Topic>> getTopics(const std::string &name)
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
    void eraseTopic(const std::string &name)
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
    bool create_topic(const std::string &name, std::shared_ptr<Topic> topic = nullptr)
    {
        if (publishing_topics.count(name))
        {
            return false;
        }
        insertTopic(name, topic);
        return true;
    }

    // 发布 Topic
    bool create_publishTopic(const std::string &name, std::shared_ptr<Topic> topic = nullptr)
    {
        // std::lock_guard<std::mutex> lock(manager_mutex); // 加锁
        if (publishing_topics.count(name))
        {
            return false;
        }
        insertTopic(name, topic);
        publishing_topics[name] = topic;
        return true;
    }
    std::shared_ptr<Topic> create_subscribeTopic(const std::string &name, std::shared_ptr<Topic> topic)
    {
        // std::lock_guard<std::mutex> lock(manager_mutex); // 加锁
        if (subscribing_topics.count(name))
        {
            return nullptr;
        }
        insertTopic(name, topic);
        subscribing_topics[name] = topic;
        return topic;
    }
    std::vector<std::shared_ptr<Topic>> get_all_publishTopics()
    {
        std::vector<std::shared_ptr<Topic>> topics;
        topics.reserve(publishing_topics.size());
        for (const auto &pair : publishing_topics)
        {
            topics.push_back(pair.second);  // 提取 shared_ptr<Topic>
        }
        return topics;
    }

  private:
    // mutable std::mutex manager_mutex;                                  // 保护 TopicManager 数据的互斥锁
    // std::unordered_map<std::string, std::shared_ptr<Topic>> topics;    // 使用智能指针管理 Topic
    Trie topics;
    std::unordered_map<std::string, std::shared_ptr<Topic>> publishing_topics;
    std::unordered_map<std::string, std::shared_ptr<Topic>> subscribing_topics;
};

#endif  // __TOPICMANAGER_H__
