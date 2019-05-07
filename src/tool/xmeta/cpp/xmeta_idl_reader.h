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
        xmeta_idl_reader(std::string_view const& idl_assembly_name)
        {
            m_assembly = idl_assembly_name;
        }

        friend struct ast_to_st_listener;

        void read(std::istream& idl_contents, bool disable_error_reporting = false);
        void read(std::istream& idl_contents, XlangParserBaseListener& listener, bool disable_error_reporting = false);
        void resolve();

        auto const& get_namespaces() const
        {
            return m_namespaces;
        }

        auto const& get_symbols() const
        {
            return symbols;
        }

        size_t get_num_semantic_errors()
        {
            return m_error_manager.get_num_of_semantic_errors();
        }

        size_t get_num_syntax_errors()
        {
            return m_error_manager.get_num_of_syntax_errors();
        }

        bool set_symbol(std::string_view symbol, class_type_semantics const& class_type)
        {
            return symbols.insert(std::pair<std::string, class_type_semantics>(symbol, class_type)).second;
        }

        void listen_struct_model(std::shared_ptr<struct_model> const& model) final;
        void listen_delegate_model(std::shared_ptr<delegate_model> const& model) final;

    private:
        std::map<std::string_view, std::shared_ptr<namespace_model>, std::less<>> m_namespaces;

        std::string m_assembly;
        std::vector<std::string> m_imnported_assembly_names;
        xlang_error_manager m_error_manager;

        std::map<std::string, class_type_semantics> symbols;
    };

    std::string copy_to_lower(std::string_view sv);
}
