#pragma once

#include <string>

#include "base_model.h"

namespace xlang
{
    namespace xmeta
    {
        enum enum_type
        {
            uint32_enum,
            int32_enum
        };

        class enum_model final : base_model
        {
        public:

        protected:
            enum_model() = delete;
            enum_model(enum_model const&) = delete;
            enum_model& operator=(enum_model const&) = delete;

            enum_model(const std::string &id, const size_t &decl_line) : base_model{ id, decl_line } { }

        private:
            enum_type type;
        };
    }
}
