#pragma once

#include <string_view>

#include "base_model.h"

namespace xlang::xmeta
{
    struct property_modifier_t
    {
        bool is_protected = false;
        bool is_static = false;
    };

    struct property_model : base_model
    {
        property_model(std::string_view const& id, size_t decl_line, property_modifier_t const& mod, xmeta_type const& type, bool get_declared, bool set_declared) :
            base_model{ id, decl_line },
            modifier{ mod },
            type{ type },
            get_declared{ get_declared },
            set_declared{ set_declared }
        { }
        property_model() = delete;

        property_modifier_t modifier;
        xmeta_type type;
        bool get_declared;
        bool set_declared;
        //TODO: link to methods
    };
}
