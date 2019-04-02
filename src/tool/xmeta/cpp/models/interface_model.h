#pragma once

#include <string_view>
#include <vector>

#include "base_model.h"
#include "event_model.h"
#include "method_model.h"
#include "property_model.h"

namespace xlang::xmeta
{
    struct interface_model : base_model
    {
        interface_model(std::string_view const& id, size_t decl_line) : base_model{ id, decl_line } { }
        interface_model() = delete;

        std::vector<std::string_view> interface_bases;
        std::vector<std::string_view> type_parameters;

        // Members
        std::vector<property_model> properties;
        std::vector<method_model> methods;
        std::vector<event_model> events;
    };
}
