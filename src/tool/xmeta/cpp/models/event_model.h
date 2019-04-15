#pragma once

#include <string_view>

#include "base_model.h"

namespace xlang::xmeta
{
    enum class event_modifier_t
    {
        none,
        protected_event,
        static_event
    };

    struct event_model : base_model
    {
        event_model(std::string_view const& id, size_t decl_line, event_modifier_t mod, xmeta_type const& type, bool accessors_declared) :
            base_model{ id, decl_line },
            event_modifier{ mod },
            event_type{ type },
            event_accessors_declared{ accessors_declared }
        { }
        event_model() = delete;

        event_modifier_t event_modifier;
        xmeta_type event_type;
        bool event_accessors_declared;

        // TODO: Add attributes to 'add' and 'remove' accessors.
    };
}
