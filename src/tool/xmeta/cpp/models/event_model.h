#pragma once

#include <string_view>

#include "base_model.h"

namespace xlang::xmeta
{
    struct event_modifier_t
    {
        bool is_protected = false;
        bool is_static = false;
    };

    struct event_model : base_model
    {
        event_model(std::string_view const& id, size_t decl_line, event_modifier_t const& mod, xmeta_type const& type, bool accessors_declared) :
            base_model{ id, decl_line },
            modifier{ mod },
            type{ type },
            event_accessors_declared{ accessors_declared }
        { }
        event_model() = delete;

        event_modifier_t modifier;
        xmeta_type type;
        bool event_accessors_declared;

        // TODO: Add attributes to 'add' and 'remove' accessors.
    };
}
