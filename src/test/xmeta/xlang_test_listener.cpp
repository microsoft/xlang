#include "pch.h"
#include "xlang_test_listener.h"

void xlang_test_listener::enterNamespace_declaration(XlangParser::Namespace_declarationContext *ctx)
{
    namespaces.insert(ctx->IDENTIFIER()->getText());
}

void xlang_test_listener::enterExpression(XlangParser::ExpressionContext * ctx)
{
    expressions.insert(ctx->getText());
}

void xlang_test_listener::enterEnum_declaration(XlangParser::Enum_declarationContext *ctx)
{
    enums.insert(ctx->IDENTIFIER()->getText());
}

void xlang_test_listener::enterEnum_member_declaration(XlangParser::Enum_member_declarationContext *ctx)
{
    enums.insert(ctx->enum_identifier()->getText());
    if (ctx->enum_expression() == NULL)
    {
        return;
    }
    XlangParser::Enum_expressionContext *enum_expression = ctx->enum_expression();
    if (enum_expression->enum_decimal_integer() != NULL)
    {
        enums.insert(enum_expression->enum_decimal_integer()->getText());
    }
    if (enum_expression->enum_expresssion_identifier() != NULL)
    {
        enums.insert(enum_expression->enum_expresssion_identifier()->getText());
    }
    if (enum_expression->enum_hexdecimal_integer() != NULL)
    {
        enums.insert(enum_expression->enum_hexdecimal_integer()->getText());
    }
}
