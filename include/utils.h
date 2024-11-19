#include <cstdint>
#include <stdexcept>
#include <vector>
class Util
{
  public:
    static std::uint32_t unpack_length(const std::vector<std::uint8_t> &bytes, std::uint32_t &length)
    {
        length                   = 0;
        std::uint32_t multiplier = 1;

        constexpr std::uint32_t MAX_LENGTH = 268435455;
        constexpr std::size_t MAX_BYTES    = 4;

        for (std::size_t i = 0; i < bytes.size(); ++i)
        {
            std::uint8_t encoded_byte = bytes[i];
            length += (encoded_byte & 127) * multiplier;

            if (length > MAX_LENGTH)
            {
                throw std::invalid_argument("Decoded length exceeds maximum MQTT length.");
            }

            if ((encoded_byte & 128) == 0)
            {
                return i + 1;  // 解码完成
            }

            multiplier *= 128;

            if (i >= MAX_BYTES - 1)
            {
                throw std::invalid_argument("Variable-length encoding exceeds 4 bytes.");
            }
        }

        throw std::invalid_argument("Incomplete variable-length encoding.");
    }
    static std::vector<std::uint8_t> pack_length(std::uint32_t length)
    {
        constexpr std::uint32_t MAX_LENGTH = 268435455;  // MQTT 可变长度字段的最大值 (0xFFFFFFF)
        std::vector<std::uint8_t> encoded_bytes;

        if (length > MAX_LENGTH)
        {
            throw std::invalid_argument("Length exceeds maximum MQTT length.");
        }

        do
        {
            std::uint8_t encoded_byte = length % 128;
            length /= 128;

            // 如果还有更多的数字需要编码，设置最高位为 1
            if (length > 0)
            {
                encoded_byte |= 128;
            }

            encoded_bytes.push_back(encoded_byte);

        } while (length > 0);

        return encoded_bytes;
    }
};