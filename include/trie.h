#ifndef __TRIE_H__
#define __TRIE_H__

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include "topic.hpp"

class TrieNode;

class Trie
{
public:
    Trie();
    ~Trie();
    void insert(const std::string &path, std::shared_ptr<Topic> topic_ptr);
    void erase(const std::string &path);
    std::vector<std::shared_ptr<Topic>> find(const std::string &pattern);

private:
    std::shared_ptr<TrieNode> root;
    std::vector<std::string> split(const std::string_view &path, char delimiter);
    void find_helper(std::shared_ptr<TrieNode> node,
                     const std::vector<std::string> &tokens,
                     size_t index,
                     const std::string &current_path,
                     std::vector<std::shared_ptr<Topic>> &matches);
    bool erase_helper(std::shared_ptr<TrieNode> node,
                      const std::vector<std::string> &tokens,
                      size_t index);
};

#endif // __TRIE_H__
