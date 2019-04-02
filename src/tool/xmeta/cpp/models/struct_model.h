#pragma once

#include <string>
#include <vector>

#include "base_model.h"
#include "property_model.h"

namespace xlang
{
    namespace xmeta
    {
        typedef std::pair<xmeta_type, std::string> field_t;
        class struct_model final : public base_model
        {
        public:
            struct_model(const std::string &id, const size_t &decl_line) : base_model{ id, decl_line } { }

            // Members
            std::vector<field_t> fields;

        protected:
            struct_model() = delete;

        };
    }
}
