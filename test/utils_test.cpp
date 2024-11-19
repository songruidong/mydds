#include <gtest/gtest.h>
#include "utils.h"

TEST(UnpackLengthTest, NormalCases) {
    // 正常情况 1
    std::vector<uint8_t> bytes1 = {0x01};
    EXPECT_EQ(unpack_length(bytes1), 1);

    // 正常情况 2
    std::vector<uint8_t> bytes2 = {0x81, 0x01};
    EXPECT_EQ(unpack_length(bytes2), 129);

    // 正常情况 3
    std::vector<uint8_t> bytes3 = {0xFF, 0xFF, 0xFF, 0x7F};
    EXPECT_EQ(unpack_length(bytes3), 268435455);
}

TEST(UnpackLengthTest, InvalidLengthCases) {
    // 长度超过最大值
    std::vector<uint8_t> bytes4 = {0xFF, 0xFF, 0xFF, 0xFF};
    EXPECT_THROW(unpack_length(bytes4), std::invalid_argument);

    // 编码超过 4 字节
    std::vector<uint8_t> bytes5 = {0x80, 0x80, 0x80, 0x80, 0x01};
    EXPECT_THROW(unpack_length(bytes5), std::invalid_argument);
}

TEST(UnpackLengthTest, IncompleteEncodingCases) {
    // 不完整编码
    std::vector<uint8_t> bytes6 = {0x80};
    EXPECT_THROW(unpack_length(bytes6), std::invalid_argument);
}
