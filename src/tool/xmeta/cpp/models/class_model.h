#pragma once

#include <memory>
#include <string_view>
#include <vector>
#include <variant>

#include "base_model.h"
#include "interface_model.h"
#include "method_model.h"
#include "property_model.h"
#include "event_model.h"
#include "class_or_interface_member_model.h"

namespace xlang::xmeta
{
    // Note: All classes are implicitly public, and explicitly specifying this is not allowed.
    // TODO: rename class_semantics
    enum class class_modifier_t
    {
        none,
        unsealed_class,
        static_class
    };

    struct class_model : base_model
    {
        class_model(std::string_view const& id, size_t decl_line, class_modifier_t const& mod) :
            base_model{ id, decl_line },
            modifier{ mod }
        { }
        class_model() = delete;

        std::string_view class_base_id;
        std::shared_ptr<class_model> class_base;
        std::vector<std::string_view> interface_bases;
        std::vector<std::string_view> type_parameters;
        class_modifier_t modifier;
        std::vector<std::shared_ptr<std::variant<property_model, method_model, event_model>>> members;
    };
}
