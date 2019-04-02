#pragma once

#include <string_view>
#include <vector>

#include "base_model.h"

namespace xlang::xmeta
{
    typedef std::vector<std::pair<xmeta_type, std::string>> formal_params_t;

    enum class method_modifier_t
    {
        public_method,
        protected_method
    };

    struct method_model : base_model
    {
        method_model(std::string_view const& id, size_t decl_line, method_modifier_t mod, xmeta_type const& return_type) :
            base_model{ id, decl_line },
            access_modifier{ mod },
            return_type{ return_type }
        { }
        method_model() = delete;

        method_modifier_t access_modifier;
        xmeta_type return_type;
        std::vector<std::string_view> type_parameters;
        std::vector<formal_parameter_t> formal_parameters;
    };
}
