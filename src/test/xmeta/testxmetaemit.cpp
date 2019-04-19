#include "pch.h"
#include <iostream>
#include <string>
#include <filesystem>

#include "antlr4-runtime.h"
#include "XlangParser.h"
#include "XlangLexer.h"
#include "xlang_test_listener.h"
#include <Windows.h>

using namespace antlr4;

#pragma execution_character_set("utf-8")

TEST_CASE("Enum metadata") {
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
    emitter->saveToFile();
    emitter->uninitialize();
    return 0;
}
