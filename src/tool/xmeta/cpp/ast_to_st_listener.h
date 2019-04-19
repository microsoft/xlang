#pragma once

#include "generated/XlangParserBaseListener.h"
#include "symbol_table.h"

#include <string_view>

using xlang::xmeta::xmeta_symbol_table;

struct xlang::xmeta::enum_model;
struct xlang::xmeta::enum_member;

struct ast_to_st_listener : XlangParserBaseListener
{
    ast_to_st_listener() = delete;
    ast_to_st_listener(xmeta_symbol_table & symbol_table);

    void exitEnum_declaration(XlangParser::Enum_declarationContext *ctx) override;

    void enterNamespace_declaration(XlangParser::Namespace_declarationContext *ctx) override;
    void exitNamespace_declaration(XlangParser::Namespace_declarationContext *ctx) override;

private:
    xmeta_symbol_table & m_st;

    bool extract_enum_member(XlangParser::Enum_member_declarationContext *ast_enum_member, std::shared_ptr<xlang::xmeta::enum_model> const& new_enum);
    bool resolve_enum_val(xlang::xmeta::enum_member & member, std::shared_ptr<xlang::xmeta::enum_model> const& new_enum, std::set<std::string_view>& depentents);
};
