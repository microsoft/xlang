#pragma once

#include <assert.h>
#include <string_view>
#include <vector>

#include "event_model.h"
#include "interface_model.h"
#include "method_model.h"
#include "model_ref.h"
#include "class_or_interface_model.h"
#include "property_model.h"

namespace xlang::xmeta
{
    struct interface_model : class_or_interface_model
    {
        interface_model() = delete;
        interface_model(std::string_view const& id, size_t decl_line, std::string_view const& assembly_name, std::shared_ptr<namespace_body_model> const& containing_ns_body) :
            class_or_interface_model{ id, decl_line, assembly_name, containing_ns_body }
        { }

        void resolve(std::map<std::string, class_type_semantics> symbols)
        {
            class_or_interface_model::resolve(symbols);
        }
    };
}
