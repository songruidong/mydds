#ifndef __TOPICMANAGER_H__
#define __TOPICMANAGER_H__

#include <memory>
#include "topic.h"
#include "trie.h"

class TopicManager
{
public:
    // 创建 Topic
    std::shared_ptr<Topic> insertTopic(const std::string &name, std::shared_ptr<Topic> topic);

    // 获取 Topic
    std::vector<std::shared_ptr<Topic>> getTopics(const std::string &name);

    // 删除 Topic
    void eraseTopic(const std::string &name);

    // // 列出所有 Topic 名称
    // std::vector<std::string> listTopics() const;

private:
    // mutable std::mutex manager_mutex;                                  // 保护 TopicManager 数据的互斥锁
    // std::unordered_map<std::string, std::shared_ptr<Topic>> topics;    // 使用智能指针管理 Topic
    Trie topics;
};

#endif // __TOPICMANAGER_H__
