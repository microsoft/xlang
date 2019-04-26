#include <iostream>

#include "xmeta_idl_reader.h"
#include "ast_to_st_listener.h"

#pragma execution_character_set("utf-8")

using namespace antlr4;

int main(int argc, const char* argv[])
{
    std::ifstream stream;
    printf("Opening %s \n", argv[1]);
    stream.open(argv[1]);

    xlang::xmeta::xmeta_idl_reader reader{ argv[1] };
    size_t num_syntax_errors = reader.read(stream);
    if (num_syntax_errors)
    {
        return EXIT_FAILURE;
    }
}
