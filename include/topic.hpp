#ifndef __TOPIC_H__
#define __TOPIC_H__

#include <mutex>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>
class Topic
{
  public:
    explicit Topic(const std::string &name) : topic_name(name) {}
    Topic() = default;
    // 设置 Protobuf 数据，绑定一个 int key
    template <typename ProtobufType>
    void setData(int key, const ProtobufType &data)
    {
        if (key < 0)
        {
            throw std::invalid_argument("Key cannot be negative.");
        }
        // std::lock_guard<std::mutex> lock(topic_mutex);  // 加锁
        storage[key] = data.SerializeAsString();
    }

    // 获取 Protobuf 数据，按 key 检索
    template <typename ProtobufType>
    ProtobufType getData(int key) const
    {
        // std::lock_guard<std::mutex> lock(topic_mutex);  // 加锁
        auto it = storage.find(key);
        if (it == storage.end())
        {
            throw std::runtime_error("Key not found: " + std::to_string(key));
        }

        ProtobufType data;
        if (!data.ParseFromString(it->second))
        {
            throw std::runtime_error("Failed to parse Protobuf data!");
        }
        return data;
    }

    // 删除数据
    void removeData(int key)
    {
        // std::lock_guard<std::mutex> lock(topic_mutex);  // 加锁
        storage.erase(key);
    }

    // 列出所有 keys
    std::vector<int> listKeys() const
    {
        // std::lock_guard<std::mutex> lock(topic_mutex);  // 加锁
        std::vector<int> keys;
        for (const auto &[key, _] : storage)
        {
            keys.push_back(key);
        }
        return keys;
    }

    // 获取 Topic 名称
    const std::string &getName() const { return topic_name; }
    void setName(const std::string_view &name) { this->topic_name = name; }

  private:
    std::string topic_name;                        // Topic 名称
    // mutable std::mutex topic_mutex;                // 保护 Topic 数据的互斥锁
    std::unordered_map<int, std::string> storage;  // 存储 Protobuf 数据的哈希表
};

#endif  // __TOPIC_H__