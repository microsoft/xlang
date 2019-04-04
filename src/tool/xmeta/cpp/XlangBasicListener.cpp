#include "XlangBasicListener.h"

void XlangBasicListener::enterNamespace_declaration(XlangParser::Namespace_declarationContext *ctx)
{
    namespaces.insert(ctx->IDENTIFIER()->getText());
}

void XlangBasicListener::enterExpression(XlangParser::ExpressionContext * ctx) {
    expressions.insert(ctx->getText());
 }