#pragma once

#include "generated/XlangParserBaseListener.h"

extern bool semantic_error_exists;

class ast_to_st_listener : public XlangParserBaseListener
{
public:
    void enterClass_declaration(XlangParser::Class_declarationContext *ctx) override;
    void exitClass_declaration(XlangParser::Class_declarationContext *ctx) override;

    void exitDelegate_declaration(XlangParser::Delegate_declarationContext *ctx) override;

    void exitEnum_declaration(XlangParser::Enum_declarationContext *ctx) override;

    void exitClass_event_declaration(XlangParser::Class_event_declarationContext *ctx) override;

    void enterInterface_declaration(XlangParser::Interface_declarationContext *ctx) override;
    void exitInterface_declaration(XlangParser::Interface_declarationContext *ctx) override;

    void exitClass_method_declaration(XlangParser::Class_method_declarationContext *ctx) override;

    void enterNamespace_declaration(XlangParser::Namespace_declarationContext *ctx) override;
    void exitNamespace_declaration(XlangParser::Namespace_declarationContext *ctx) override;

    void exitClass_property_declaration(XlangParser::Class_property_declarationContext *ctx) override;

    void enterStruct_declaration(XlangParser::Struct_declarationContext *ctx) override;
    void exitStruct_declaration(XlangParser::Struct_declarationContext *ctx) override;

    void exitUsing_alias_directive(XlangParser::Using_alias_directiveContext *ctx) override;

    void exitUsing_namespace_directive(XlangParser::Using_namespace_directiveContext *ctx) override;
};
