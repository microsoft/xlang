#include "XlangParserBaseListener.h"
#include <set>

class xlangtestlistener : public XlangParserBaseListener
{
public:
    void enterNamespace_declaration(XlangParser::Namespace_declarationContext *ctx) override;
    void enterExpression(XlangParser::ExpressionContext *ctx) override;
    void enterEnum_member_declaration(XlangParser::Enum_member_declarationContext *ctx) override;
    std::set<std::string> namespaces;
    std::set<std::string> expressions;
    std::set<std::string> enums;
};
