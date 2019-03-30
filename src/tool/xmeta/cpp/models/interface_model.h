#pragma once

#include <string>
#include <vector>

#include "base_model.h"
#include "event_model.h"
#include "method_model.h"
#include "property_model.h"

namespace xlang
{
    namespace xmeta
    {
        class interface_model final : public base_model
        {
        public:
            interface_model(const std::string &id, const size_t &decl_line) : base_model{ id, decl_line } { }

            std::vector<std::string> interface_base_list;
            std::vector<std::string> type_parameter_list;

            // Members
            std::vector<std::shared_ptr<event_model>> events;
            std::vector<std::shared_ptr<method_model>> methods;
            std::vector<std::shared_ptr<property_model>> properties;

        protected:
            interface_model() = delete;

        };
    }
}
