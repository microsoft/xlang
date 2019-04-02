#pragma once

#include <string>

#include "base_model.h"

namespace xlang
{
    namespace xmeta
    {
        enum enum_type
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
            std::string id;
            uint64_t value;
        };

        class enum_model final : base_model
        {
        public:
            enum_model(const std::string &id, const size_t &decl_line, const enum_type &type) :
                base_model{ id, decl_line },
                type{ type }
            { }
            enum_type type;
            std::vector<enum_member> members;

        protected:
            enum_model() = delete;

        };
    }
}
