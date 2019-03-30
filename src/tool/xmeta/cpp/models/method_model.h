#pragma once

#include <string>
#include <vector>

#include "base_model.h"

namespace xlang
{
    namespace xmeta
    {
        enum access_modifier_t
        {
            public_method,
            protected_method
        };

        typedef std::vector<std::pair<type, std::string>> formal_params_t;

        class method_model final : public base_model
        {
        public:
            method_model(const std::string &id, const size_t &decl_line) : base_model{ id, decl_line } { }

            int arity;
            access_modifier_t access_modifier;
            bool is_partial;
            type return_type;
            std::vector<std::string> type_parameter_list;
            std::vector<formal_parameter_t> formal_parameter_list;

        protected:
            method_model() = delete;

        };
    }
}