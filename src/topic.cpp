#include "topic.hpp"

// 设置 Protobuf 数据，绑定一个 int key
template <typename ProtobufType>
void Topic::setData(int key, const ProtobufType &data)
{
    if (key < 0)
    {
        throw std::invalid_argument("Key cannot be negative.");
    }
    std::lock_guard<std::mutex> lock(topic_mutex); // 加锁
    storage[key] = data.SerializeAsString();
}

// 获取 Protobuf 数据，按 key 检索
template <typename ProtobufType>
ProtobufType Topic::getData(int key) const
{
    std::lock_guard<std::mutex> lock(topic_mutex); // 加锁
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
void Topic::removeData(int key)
{
    std::lock_guard<std::mutex> lock(topic_mutex); // 加锁
    storage.erase(key);
}

// 列出所有 keys
std::vector<int> Topic::listKeys() const
{
    std::lock_guard<std::mutex> lock(topic_mutex); // 加锁
    std::vector<int> keys;
    for (const auto &[key, _] : storage)
    {
        keys.push_back(key);
    }
    return keys;
}
