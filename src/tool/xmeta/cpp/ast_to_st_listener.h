#pragma once

#include <optional>
#include <string_view>
#include "XlangParserBaseListener.h"

#include "models/xmeta_models.h"

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

    void enterClass_declaration(XlangParser::Class_declarationContext *ctx) override;
    void exitClass_declaration(XlangParser::Class_declarationContext *ctx) override;

    void exitClass_property_declaration(XlangParser::Class_property_declarationContext *ctx) override;

    void exitDelegate_declaration(XlangParser::Delegate_declarationContext *ctx) override;

    void exitEnum_declaration(XlangParser::Enum_declarationContext *ctx) override;

    void enterInterface_declaration(XlangParser::Interface_declarationContext *ctx) override;
    void exitInterface_declaration(XlangParser::Interface_declarationContext *ctx) override;

    void exitInterface_property_declaration(XlangParser::Interface_property_declarationContext *ctx) override;

    void enterNamespace_declaration(XlangParser::Namespace_declarationContext *ctx) override;
    void exitNamespace_declaration(XlangParser::Namespace_declarationContext *ctx) override;

private:
    xlang::xmeta::xmeta_idl_reader& m_reader;

    listener_error extract_enum_member(XlangParser::Enum_member_declarationContext *ast_enum_member, std::shared_ptr<xlang::xmeta::enum_model> const& new_enum);
    void extract_formal_params(std::vector<XlangParser::Fixed_parameterContext *> const& ast_formal_params, std::shared_ptr<xlang::xmeta::delegate_model> const& dm);
    template<class T> listener_error extract_property_declaration(XlangParser::Property_declarationContext *ctx, std::shared_ptr<T> const& model);
    listener_error extract_property_semantic(std::vector<XlangParser::Property_modifierContext *> mods, xlang::xmeta::property_semantics& sem, size_t decl_line, std::string_view const& id);
    void extract_to_existing_property(std::shared_ptr<xlang::xmeta::property_model> const& prop, bool get_declared, bool set_declared, size_t decl_line, std::string_view const& container_name);
    listener_error extract_type(XlangParser::TypeContext *tc, xlang::xmeta::type_ref& tr);
    listener_error extract_type(XlangParser::Return_typeContext *rtc, std::optional<xlang::xmeta::type_ref>& tr);

    listener_error resolve_enum_val(xlang::xmeta::enum_member& member, std::shared_ptr<xlang::xmeta::enum_model> const& new_enum, std::set<std::string_view>& depentents);
};
