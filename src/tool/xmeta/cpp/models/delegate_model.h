#pragma once

#include <string>
#include <vector>

#include "base_model.h"

namespace xlang
{
    namespace xmeta
    {
        class delegate_model final : public base_model
        {
        public:
            delegate_model(const std::string &id, const size_t &decl_line) : base_model{ id, decl_line } { }

            xmeta_type return_type;
            std::vector<std::string> type_parameter_list;
            std::vector<formal_parameter_t> formal_parameter_list;

        protected:
            delegate_model() = delete;

        };
    }
}
