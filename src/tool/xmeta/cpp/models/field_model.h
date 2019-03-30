#pragma once

#include <string>
#include <vector>

#include "base_model.h"

namespace xlang
{
    namespace xmeta
    {
        class field_model final : public base_model
        {
        public:
            method_model(const std::string &id, const size_t &decl_line) : base_model{ id, decl_line } { }

            int arity;
            access_modifier_t access_modifier;
            bool is_partial;
            type return_type;
            std::vector<std::string> type_parameter_list;
            formal_params_t formal_parameter_list;

        protected:
            method_model() = delete;

        };
    }
}