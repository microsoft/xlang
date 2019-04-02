#pragma once

#include <string_view>
#include <utility>
#include <vector>

#include "base_model.h"
#include "property_model.h"

namespace xlang::xmeta
{
    struct struct_model : base_model
    {
        struct_model(std::string_view const& id, size_t decl_line) : base_model{ id, decl_line } { }
        struct_model() = delete;

        // Members
        std::vector<std::pair<xmeta_type, std::string_view>> fields;
    };
}
