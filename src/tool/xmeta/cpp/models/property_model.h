#pragma once

#include <string_view>

#include "base_model.h"

namespace xlang::xmeta
{
    enum class property_modifier_t
    {
        none,
        protected_property,
        static_property
    };

    struct property_model : base_model
    {
        property_model(std::string_view const& id, size_t decl_line, property_modifier_t mod, xmeta_type const& type, bool get_declared, bool set_declared) :
            base_model{ id, decl_line },
            property_modifier{ mod },
            property_type{ type },
            get_declared{ get_declared },
            set_declared{ set_declared }
        { }
        property_model() = delete;

        property_modifier_t property_modifier;
        xmeta_type property_type;
        bool get_declared;
        bool set_declared;
    };
}
