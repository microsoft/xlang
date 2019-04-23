#include <iostream>

#include "antlr4-runtime.h"
#include "XlangParser.h"
#include "XlangLexer.h"

#include "xmeta_idl_reader.h"
#include "ast_to_st_listener.h"

#if defined(_WIN32)
#include <Windows.h>
#include <mscoree.h>
#include <cor.h>
#include <winrt/base.h>
#endif

#pragma execution_character_set("utf-8")

using namespace antlr4;
using namespace winrt;

int main(int argc, const char* argv[])
{
    return 0;
}
