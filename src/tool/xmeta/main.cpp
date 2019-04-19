#include <iostream>

#include "antlr4-runtime.h"
#include "XlangParser.h"
#include "XlangLexer.h"

#include "ast_to_st_listener.h"
#include "symbol_table.h"

#if defined(_WIN32)
#include <Windows.h>
#endif

#pragma execution_character_set("utf-8")

using namespace antlr4;

int main(int argc, const char* argv[])
{
    std::ifstream stream;
    std::string_view idl_file_name{ argv[1] };
    printf("Opening %s \n", idl_file_name);
    stream.open(argv[1]);

    ANTLRInputStream input{ stream };
    XlangLexer lexer{ &input };
    CommonTokenStream tokens{ &lexer };
    XlangParser parser{ &tokens };
    parser.setBuildParseTree(true);

    tree::ParseTree *tree = parser.xlang();

    xlang::xmeta::xmeta_symbol_table st{ idl_file_name };
    tree::ParseTreeWalker walker{};
    std::shared_ptr<ast_to_st_listener> listener = std::make_shared<ast_to_st_listener>(st);
    walker.walk(listener.get(), tree);
    std::string s = tree->toStringTree(&parser);
    std::cout << tree->toStringTree(&parser) << std::endl;

    std::cout << parser.getNumberOfSyntaxErrors() << std::endl;

    // This gets the token stream and prints out all the tokens corresponding to the file
    TokenStream * ts = parser.getTokenStream();
    for (size_t i = 0; i < ts->size(); i++)
    {
        std::cout << ts->get(i)->getType() << std::endl;
    }
}
