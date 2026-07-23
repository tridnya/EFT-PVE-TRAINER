#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>
#include <string>

namespace runtime::mcp
{
    constexpr std::uint32_t protocol_magic = 0x504D4C56;
    constexpr std::uint16_t protocol_version = 1;
    constexpr std::size_t maximum_payload_size = 1024 * 1024;

    enum class Command : std::uint16_t
    {
        status = 1,
        list_assemblies = 2,
        find_classes = 3,
        inspect_class = 4,
        find_objects = 5,
        inspect_object = 6,
        read_field = 7,
        read_collection = 8,
        get_components = 9,
        get_hierarchy = 10,
        write_field = 11,
        read_memory = 12
    };

    enum class ValueKind : std::uint8_t
    {
        boolean = 1,
        signed_integer = 2,
        unsigned_integer = 3,
        floating_point = 4
    };

#pragma pack(push, 1)
    struct FrameHeader
    {
        std::uint32_t magic = protocol_magic;
        std::uint16_t version = protocol_version;
        Command command = Command::status;
        std::uint32_t request_id = 0;
        std::uint32_t payload_size = 0;
    };
#pragma pack(pop)

    static_assert(sizeof(FrameHeader) == 16);

    class BinaryReader
    {
    public:
        explicit BinaryReader(const std::span<const std::uint8_t> payload)
            : payload_(payload)
        {
        }

        template <typename T>
        bool read(T& value)
        {
            if (remaining() < sizeof(T))
            {
                valid_ = false;
                return false;
            }

            std::memcpy(&value, payload_.data() + offset_, sizeof(T));
            offset_ += sizeof(T);
            return true;
        }

        bool read_string(std::string& value)
        {
            std::uint32_t length = 0;
            if (!read(length) || length > remaining())
            {
                valid_ = false;
                return false;
            }

            value.assign(
                reinterpret_cast<const char*>(payload_.data() + offset_),
                length);
            offset_ += length;
            return true;
        }

        [[nodiscard]] bool valid() const
        {
            return valid_;
        }

        [[nodiscard]] std::size_t remaining() const
        {
            return payload_.size() - offset_;
        }

    private:
        std::span<const std::uint8_t> payload_;
        std::size_t offset_ = 0;
        bool valid_ = true;
    };
}
