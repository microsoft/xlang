#pragma once

#include <string_view>
#include <vector>

#include "base_model.h"

namespace xlang::xmeta
{
    struct method_modifiers
    {
        bool is_protected = false;
        bool is_override = false;
        bool is_static = false;
    };

    struct method_model : base_model
    {
        method_model(std::string_view const& id, size_t decl_line, xmeta_type const& return_type, method_modifiers const& mods) :
            base_model{ id, decl_line },
            modifiers{ mods },
            return_type{ return_type }
        { }
        method_model() = delete;

        method_modifiers modifiers;
        xmeta_type return_type;
        std::vector<std::string_view> type_parameters;
        std::vector<formal_parameter_t> formal_parameters;
    };
}
