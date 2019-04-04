#pragma once

#include <string_view>

#include "base_model.h"

namespace xlang::xmeta
{
    enum class enum_type
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
        uint64_t value;
    };

    struct enum_model : base_model
    {
        enum_model(std::string_view const& id, size_t decl_line, enum_type type) :
            base_model{ id, decl_line },
            type{ type }
        { }
        enum_model() = delete;

        enum_type type;
        std::vector<enum_member> members;
    };
}
