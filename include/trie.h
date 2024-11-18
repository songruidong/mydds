#ifndef __TRIE_H__
#define __TRIE_H__

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include "topic.h"
// TrieNode: 字典树的节点
class TrieNode
{
  public:
    TrieNode() {}

    // bool is_end_of_word;  // 当前节点是否是一个完整路径的结束
    std::shared_ptr<Topic> topic_ptr;
    std::unordered_map<std::string, std::shared_ptr<TrieNode>> children;  // 子节点
};

// Trie: 支持通配符的字典树
class Trie
{
  public:
    Trie() : root(std::make_shared<TrieNode>()) {}

    // 插入一个路径，例如 "sensor/temperature/room1"
    void insert(const std::string &path, std::shared_ptr<Topic> topic_ptr)
    {
        auto node                       = root;
        std::vector<std::string> tokens = split(path, '/');
        for (const auto &token : tokens)
        {
            if (node->children.find(token) == node->children.end())
            {
                node->children[token] = std::make_shared<TrieNode>();
            }
            node = node->children[token];
        }
        node->topic_ptr = topic_ptr;
    }

    // 删除一个路径，例如 "sensor/temperature/room1"
    void erase(const std::string &path)
    {
        std::vector<std::string> tokens = split(path, '/');
        erase_helper(root, tokens, 0);
    }

    // 查找匹配通配符的路径，例如 "sensor/+/room1" 或 "sensor/#"
    std::vector<std::shared_ptr<Topic>> find(const std::string &pattern)
    {
        std::vector<std::shared_ptr<Topic>> matches;
        std::vector<std::string> tokens = split(pattern, '/');
        find_helper(root, tokens, 0, "", matches);
        return matches;
    }

  private:
    std::shared_ptr<TrieNode> root;

    std::vector<std::string> split(const std::string_view &path, char delimiter)
    {
        std::vector<std::string> tokens;
        size_t start = 0, end = 0;
        while ((end = path.find(delimiter, start)) != std::string::npos)
        {
            tokens.emplace_back(path.substr(start, end - start));  // 直接切片
            start = end + 1;
        }
        if (start < path.size())
        {
            tokens.emplace_back(path.substr(start));  // 添加最后一个部分
        }
        return tokens;
    }

    // 递归查找通配符匹配
    void find_helper(std::shared_ptr<TrieNode> node,
                     const std::vector<std::string> &tokens,
                     size_t index,
                     const std::string &current_path,
                     std::vector<std::shared_ptr<Topic>> &matches)
    {
        // 如果已经匹配完所有层级
        if (index == tokens.size())
        {
            if (node->topic_ptr)
            {
                matches.push_back(node->topic_ptr);
            }
            return;
        }

        const std::string &token = tokens[index];

        if (token == "+")
        {
            // "+" 匹配当前层级的所有节点
            for (const auto &[child_name, child_node] : node->children)
            {
                find_helper(child_node, tokens, index + 1,
                            current_path.empty() ? child_name : current_path + "/" + child_name, matches);
            }
        }
        else if (token == "#")
        {
            // "#" 匹配当前和后续所有层级
            find_helper(node, tokens, tokens.size(), current_path, matches);  // 当前路径就是匹配结果
            for (const auto &[child_name, child_node] : node->children)
            {
                find_helper(child_node, tokens, index,
                            current_path.empty() ? child_name : current_path + "/" + child_name, matches);
            }
        }
        else
        {
            // 普通字符匹配
            if (node->children.find(token) != node->children.end())
            {
                find_helper(node->children[token], tokens, index + 1,
                            current_path.empty() ? token : current_path + "/" + token, matches);
            }
        }
    }

    // 递归删除节点
    bool erase_helper(std::shared_ptr<TrieNode> node,
                      const std::vector<std::string> &tokens,
                      size_t index)
    {
        if (index == tokens.size())
        {
            // 到达路径末尾，清理 topic_ptr
            if (node->topic_ptr)
            {
                node->topic_ptr.reset();  // 清理指针
            }
            // 如果没有子节点，返回 true，表示可以删除当前节点
            return node->children.empty();
        }

        const std::string &token = tokens[index];
        if (node->children.find(token) != node->children.end())
        {
            auto child_node = node->children[token];
            if (erase_helper(child_node, tokens, index + 1))
            {
                // 如果子节点可以删除，则从当前节点的子节点列表中移除
                node->children.erase(token);
            }
        }

        // 如果当前节点没有 topic_ptr 且没有子节点，返回 true
        return node->children.empty() && !node->topic_ptr;
    }
};

#endif  // __TRIE_H__