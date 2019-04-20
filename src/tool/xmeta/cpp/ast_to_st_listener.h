#pragma once

#include <string_view>
#include "generated/XlangParserBaseListener.h"
#include "models/enum_model.h"

namespace xlang::xmeta
{
    struct xmeta_idl_reader;
}

enum class listener_error : bool
{
    passed = true,
    failed = false
};

struct ast_to_st_listener : XlangParserBaseListener
{
    ast_to_st_listener() = delete;
    ast_to_st_listener(xlang::xmeta::xmeta_idl_reader& reader);

    void exitEnum_declaration(XlangParser::Enum_declarationContext *ctx) override;

    void enterNamespace_declaration(XlangParser::Namespace_declarationContext *ctx) override;
    void exitNamespace_declaration(XlangParser::Namespace_declarationContext *ctx) override;

private:
    xlang::xmeta::xmeta_idl_reader& m_reader;


    listener_error extract_enum_member(XlangParser::Enum_member_declarationContext *ast_enum_member, std::shared_ptr<xlang::xmeta::enum_model> const& new_enum);
    listener_error resolve_enum_val(xlang::xmeta::enum_member & member, std::shared_ptr<xlang::xmeta::enum_model> const& new_enum, std::set<std::string_view>& depentents);
};
