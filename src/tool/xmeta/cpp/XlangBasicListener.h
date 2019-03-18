#include "generated/XlangParserBaseListener.h"
#include<set>

class XlangBasicListener : public XlangParserBaseListener
{
public:
    void enterNamespace_declaration(XlangParser::Namespace_declarationContext *ctx) override;
    std::set<std::string> namespaces;

};
