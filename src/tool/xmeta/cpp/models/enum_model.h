#pragma once

#include <algorithm>
#include <string_view>

#include "base_model.h"

namespace xlang::xmeta
{
    enum class enum_t
    {
        int8_enum,
        uint8_enum,
        int16_enum,
        uint16_enum,
        int32_enum,
        uint32_enum,
        int64_enum,
        uint64_enum
    };

    struct enum_member
    {
        std::string_view id;
        int64_t signed_val;
        uint64_t unsigned_val;
        std::string_view value_id;
    };

    struct enum_model : base_model
    {
        enum_model(std::string_view const& id, size_t decl_line, enum_t type) :
            base_model{ id, decl_line },
            type{ type }
        { }
        enum_model() = delete;

        auto get_member(std::string_view const& member_id) const
        {
            return std::find_if(members.begin(), members.end(),
                [&member_id](enum_member const& em) { return em.id == member_id; });
        }

        bool is_signed() const
        {
            return type == enum_t::int8_enum ||
                type == enum_t::int16_enum ||
                type == enum_t::int32_enum ||
                type == enum_t::int64_enum;
        }

        bool within_range(int64_t val) const
        {
            if (type == enum_t::int8_enum)
            {
                return std::numeric_limits<int8_t>::min() <= val && val <= std::numeric_limits<int8_t>::max();
            }
            else if (type == enum_t::int16_enum)
            {
                return std::numeric_limits<int16_t>::min() <= val && val <= std::numeric_limits<int16_t>::max();
            }
            else if (type == enum_t::int32_enum)
            {
                return std::numeric_limits<int32_t>::min() <= val && val <= std::numeric_limits<int32_t>::max();
            }
            else if (type == enum_t::int64_enum)
            {
                return true;
            }
            return false;
        }

        bool within_range(uint64_t val) const
        {
            if (type == enum_t::uint8_enum)
            {
                return std::numeric_limits<uint8_t>::min() <= val && val <= std::numeric_limits<uint8_t>::max();
            }
            else if (type == enum_t::uint16_enum)
            {
                return std::numeric_limits<uint16_t>::min() <= val && val <= std::numeric_limits<uint16_t>::max();
            }
            else if (type == enum_t::uint32_enum)
            {
                return std::numeric_limits<uint32_t>::min() <= val && val <= std::numeric_limits<uint32_t>::max();
            }
            else if (type == enum_t::uint64_enum)
            {
                return true;
            }
            return false;
        }

        enum_t type;
        std::vector<enum_member> members;
    };
}
