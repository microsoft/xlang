#include <iostream>

#include "antlr4-runtime.h"
#include "XlangParser.h"
#include "XlangLexer.h"

#include "xlang_model_walker.h"
#include "xmeta_emit.h"
#include "xmeta_models.h"
#include <windows.h>
#include <mscoree.h>
#include <cor.h>
#include <winrt/base.h>

#pragma execution_character_set("utf-8")

using namespace antlr4;
using namespace winrt;

int main(int argc, const char * argv[]) {

    //std::ifstream stream;
    //printf("Opening %s \n", argv[1]);
    //stream.open(argv[1]);

    //ANTLRInputStream input(stream);
    //XlangLexer lexer(&input);
    //CommonTokenStream tokens(&lexer);
    //XlangParser parser(&tokens);
    //parser.setBuildParseTree(true);

    //tree::ParseTree *tree = parser.xlang();
    //std::string s = tree->toStringTree(&parser);
    //std::cout << tree->toStringTree(&parser) << std::endl;

    //std::cout << parser.getNumberOfSyntaxErrors() << std::endl;

    std::vector<std::shared_ptr<xlang::xmeta::namespace_model>> v;


    xlang::xmeta::xlang_model_walker walker(v);
    std::shared_ptr<xlang::xmeta::xmeta_emit> emitter = std::make_shared<xlang::xmeta::xmeta_emit>(std::string("firsttest"));
    emitter->initialize();
    walker.register_listener(emitter);
    walker.walk();
    emitter->save_to_file();
    emitter->uninitialize();
    return 0;
}
