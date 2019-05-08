#pragma once

#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>

#include "antlr4-runtime.h"
#include "XlangParserBaseListener.h"
#include "xlang_model_listener.h"
#include "models/xmeta_models.h"
#include "xlang_error.h"

struct ast_to_st_listener;

namespace xlang::xmeta
{
    struct xmeta_idl_reader : public xlang_model_listener
    {
        xmeta_idl_reader(std::string_view const& idl_assembly_name) : m_xlang_model{ idl_assembly_name }
        {}

        friend struct ast_to_st_listener;

        void read(std::istream& idl_contents, bool disable_error_reporting = false);
        void read(std::istream& idl_contents, XlangParserBaseListener& listener, bool disable_error_reporting = false);
        void resolve();

        auto const& get_namespaces() const
        {
            return m_xlang_model.namespaces;
        }

        size_t get_num_semantic_errors()
        {
            return m_error_manager.get_num_of_semantic_errors();
        }

        size_t get_num_syntax_errors()
        {
            return m_error_manager.get_num_of_syntax_errors();
        }

        void listen_struct_model(std::shared_ptr<struct_model> const& model) final;
        void listen_delegate_model(std::shared_ptr<delegate_model> const& model) final;

    private:
        xlang_error_manager m_error_manager;
        compilation_unit m_xlang_model;
    };

    std::string copy_to_lower(std::string_view sv);
}
