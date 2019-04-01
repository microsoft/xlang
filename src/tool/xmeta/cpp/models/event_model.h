#pragma once

#include "base_model.h"

namespace xlang
{
    namespace xmeta
    {
        enum event_modifier_t
        {
            no_event_modifier,
            proected_event,
            static_event
        };

        class event_model final : public base_model
        {
        public:
            event_model(const std::string &id, const size_t &decl_line) : base_model{ id, decl_line } { }

            event_modifier_t event_modifier;
            xmeta_type event_type;
            bool event_accessors_declared;

            // TODO: Add attributes to 'add' and 'remove' accessors.

        protected:
            event_model() = delete;

        };
    }
}
