#include "trie.h"



Trie::Trie() : root(std::make_shared<TrieNode>()) {}

Trie::~Trie() {}

void Trie::insert(const std::string &path, std::shared_ptr<Topic> topic_ptr)
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

void Trie::erase(const std::string &path)
{
    std::vector<std::string> tokens = split(path, '/');
    erase_helper(root, tokens, 0);
}

std::vector<std::shared_ptr<Topic>> Trie::find(const std::string &pattern)
{
    std::vector<std::shared_ptr<Topic>> matches;
    std::vector<std::string> tokens = split(pattern, '/');
    find_helper(root, tokens, 0, "", matches);
    return matches;
}

std::vector<std::string> Trie::split(const std::string_view &path, char delimiter)
{
    std::vector<std::string> tokens;
    size_t start = 0, end = 0;
    while ((end = path.find(delimiter, start))!= std::string::npos)
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

void Trie::find_helper(std::shared_ptr<TrieNode> node,
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
                        current_path.empty()? child_name : current_path + "/" + child_name, matches);
        }
    }
    else if (token == "#")
    {
        // "#" 匹配当前和后续所有层级
        find_helper(node, tokens, tokens.size(), current_path, matches);  // 当前路径就是匹配结果
        for (const auto &[child_name, child_node] : node->children)
        {
            find_helper(child_node, tokens, index,
                        current_path.empty()? child_name : current_path + "/" + child_name, matches);
        }
    }
    else
    {
        // 普通字符匹配
        if (node->children.find(token)!= node->children.end())
        {
            find_helper(node->children[token], tokens, index + 1,
                        current_path.empty()? token : current_path + "/" + token, matches);
        }
    }
}

bool Trie::erase_helper(std::shared_ptr<TrieNode> node,
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
    if (node->children.find(token)!= node->children.end())
    {
        auto child_node = node->children[token];
        if (erase_helper(child_node, tokens, index + 1))
        {
            // 如果子节点可以删除，则从当前节点的子节点列表中移除
            node->children.erase(token);
        }
    }

    // 如果当前节点没有 topic_ptr 且没有子节点，返回 true
    return node->children.empty() &&!node->topic_ptr;
}
