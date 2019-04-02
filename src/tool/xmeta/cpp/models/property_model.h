#pragma once

#include "base_model.h"

namespace xlang
{
    namespace xmeta
    {
        enum property_modifier_t
        {
            no_property_modifier,
            protected_property,
            static_property
        };

        class property_model final : public base_model
        {
        public:
            property_model(const std::string &id, const size_t &decl_line, const property_modifier_t &mod, const xmeta_type &type, bool get_declared, bool set_declared) :
                base_model{ id, decl_line },
                property_modifier{ mod },
                property_type{ type },
                get_declared{ get_declared },
                set_declared{ set_declared }
            { }

            property_modifier_t property_modifier;
            xmeta_type property_type;
            bool get_declared;
            bool set_declared;

        protected:
            property_model() = delete;

        };
    }
}
