#include <gtest/gtest.h>
#include "trie.h"

TEST(TrieTest, InsertAndFind) {
    Trie trie;

    std::shared_ptr<Topic> topic1 = std::make_shared<Topic>("topic1");
    std::shared_ptr<Topic> topic2 = std::make_shared<Topic>("topic2");

    trie.insert("sensor/temperature/room1", topic1);
    trie.insert("sensor/humidity/room1", topic2);

    std::vector<std::shared_ptr<Topic>> matches = trie.find("sensor/temperature/room1");
    EXPECT_EQ(matches.size(), 1);
    EXPECT_EQ(matches[0], topic1);

    matches = trie.find("sensor/#");
    EXPECT_EQ(matches.size(), 2);
}

TEST(TrieTest, Erase) {
    Trie trie;

    std::shared_ptr<Topic> topic1 = std::make_shared<Topic>("topic1");

    trie.insert("sensor/temperature/room1", topic1);

    trie.erase("sensor/temperature/room1");

    std::vector<std::shared_ptr<Topic>> matches = trie.find("sensor/temperature/room1");
    EXPECT_EQ(matches.size(), 0);
}

TEST(TrieTest, WildcardFind) {
    Trie trie;

    std::shared_ptr<Topic> topic1 = std::make_shared<Topic>("topic1");
    std::shared_ptr<Topic> topic2 = std::make_shared<Topic>("topic2");

    trie.insert("sensor/temperature/room1", topic1);
    trie.insert("sensor/humidity/room1", topic2);

    std::vector<std::shared_ptr<Topic>> matches = trie.find("sensor/+/room1");
    EXPECT_EQ(matches.size(), 2);
}
