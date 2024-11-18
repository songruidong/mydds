#ifndef __TOPICMANAGER_H__
#define __TOPICMANAGER_H__

#include <memory>
#include "topic.h"
#include "trie.h"

class TopicManager
{
  public:
    // 创建 Topic
    std::shared_ptr<Topic> insertTopic(const std::string &name, std::shared_ptr<Topic> topic)
    {
        // std::lock_guard<std::mutex> lock(manager_mutex); // 加锁
        if (topics.find(name).size() != 0)
        {
            throw std::runtime_error("Topic already exists: " + name);
        }
        topic->setName(name);
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
    // std::vector<std::string> listTopics() const
    // {
    //     // std::lock_guard<std::mutex> lock(manager_mutex); // 加锁
    //     std::vector<std::string> names;
    //     for (const auto &[name, _] : topics)
    //     {
    //         names.push_back(name);
    //     }
    //     return names;
    // }

  private:
    // mutable std::mutex manager_mutex;                                  // 保护 TopicManager 数据的互斥锁
    // std::unordered_map<std::string, std::shared_ptr<Topic>> topics;    // 使用智能指针管理 Topic
    Trie topics;
};

#endif  // __TOPICMANAGER_H__