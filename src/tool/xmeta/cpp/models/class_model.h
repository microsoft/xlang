#pragma once

#include <string>
#include <vector>

#include "base_model.h"
#include "interface_model.h"
#include "method_model.h"
#include "property_model.h"
#include "event_model.h"

namespace xlang
{
    namespace xmeta
    {
        // Note: All classes are implicitly public, and explicitly specifying this is not allowed.
        enum class_modifier_t
        {
            none,
            sealed_class,
            static_class
        };

        class class_model final : public base_model
        {
        public:
            class_model(const std::string &id, const size_t &decl_line) : base_model{ id, decl_line } { }

            std::string class_base_id;
            std::shared_ptr<class_model> class_base;
            std::vector<std::string> interface_base_list;
            std::vector<std::string> type_parameter_list;
            std::vector<formal_parameter_t> formal_parameter_list;

            // Members
            std::vector<std::shared_ptr<event_model>> events;
            std::vector<std::shared_ptr<method_model>> methods;
            std::vector<std::shared_ptr<property_model>> properties;

        protected:
            class_model() = delete;

        };
    }
}
