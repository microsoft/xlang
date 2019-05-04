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

    void enterInterface_declaration(XlangParser::Interface_declarationContext *ctx) override;
    void enterDelegate_declaration(XlangParser::Delegate_declarationContext* ctx) override;
    void enterEnum_declaration(XlangParser::Enum_declarationContext *ctx) override;
    void enterStruct_declaration(XlangParser::Struct_declarationContext *ctx) override;

    void enterNamespace_declaration(XlangParser::Namespace_declarationContext *ctx) override;
    void exitNamespace_declaration(XlangParser::Namespace_declarationContext *ctx) override;

private:
    xlang::xmeta::xmeta_idl_reader& m_reader;

    listener_error extract_enum_member(XlangParser::Enum_member_declarationContext *ast_enum_member, std::shared_ptr<xlang::xmeta::enum_model> const& new_enum);
    listener_error resolve_enum_val(xlang::xmeta::enum_member & member, std::shared_ptr<xlang::xmeta::enum_model> const& new_enum, std::set<std::string_view>& depentents);
  
    listener_error extract_type(XlangParser::TypeContext* tc, xlang::xmeta::type_ref& tr);
    listener_error extract_type(XlangParser::Return_typeContext* rtc, std::optional<xlang::xmeta::type_ref>& tr);
    void extract_formal_params(std::vector<XlangParser::Fixed_parameterContext*> const& ast_formal_params, 
        std::variant<std::shared_ptr<xlang::xmeta::delegate_model>, std::shared_ptr<xlang::xmeta::method_model>> const& model);

    listener_error extract_property_accessors(XlangParser::Interface_property_declarationContext* interface_property,
        std::shared_ptr<xlang::xmeta::class_or_interface_model> model);
};
