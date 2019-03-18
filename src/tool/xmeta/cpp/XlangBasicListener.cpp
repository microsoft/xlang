#include "XlangBasicListener.h"

void XlangBasicListener::enterNamespace_declaration(XlangParser::Namespace_declarationContext *ctx)
{
    namespaces.insert(ctx->IDENTIFIER()->getText());
}
