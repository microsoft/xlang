#pragma once

#include <memory>
#include <string_view>
#include <vector>

#include "base_model.h"
#include "interface_model.h"
#include "method_model.h"
#include "property_model.h"
#include "event_model.h"

namespace xlang::xmeta
{
    // Note: All classes are implicitly public, and explicitly specifying this is not allowed.
    enum class class_modifier_t
    {
        none,
        sealed_class,
        static_class
    };

    struct class_model : base_model
    {
        class_model(std::string_view const& id, size_t decl_line) : base_model{ id, decl_line } { }
        class_model() = delete;

        std::string_view class_base_id;
        std::shared_ptr<class_model> class_base;
        std::vector<std::string_view> interface_bases;
        std::vector<std::string_view> type_parameters;
        std::vector<formal_parameter_t> formal_parameters;
        std::vector<std::string_view> generic_types;

        // Members
        std::vector<property_model> properties;
        std::vector<method_model> methods;
        std::vector<event_model> events;
    };
}
