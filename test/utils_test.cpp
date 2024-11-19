#include <gtest/gtest.h>
#include "../include/utils.h"

TEST(UtilTest, UnpackLength) {
    // 正常情况
    std::vector<uint8_t> bytes1 = {0x08};
    uint32_t length1;
    EXPECT_EQ(Util::unpack_length(bytes1, length1), 1);
    EXPECT_EQ(length1, 8);

    // 超过最大长度
    std::vector<uint8_t> bytes2 = {0xFF, 0xFF, 0xFF, 0xFF, 0x7F};
    uint32_t length2;
    EXPECT_THROW(Util::unpack_length(bytes2, length2), std::invalid_argument);

    // 编码不完整
    std::vector<uint8_t> bytes3 = {0x80};
    uint32_t length3;
    EXPECT_THROW(Util::unpack_length(bytes3, length3), std::invalid_argument);
}

TEST(UtilTest, PackLength) {
    // 正常情况
    uint32_t length1 = 127;
    std::vector<uint8_t> encoded_bytes1 = Util::pack_length(length1);
    EXPECT_EQ(encoded_bytes1.size(), 1);

    // 超过最大长度
    uint32_t length2 = 268435456;
    EXPECT_THROW(Util::pack_length(length2), std::invalid_argument);
}
