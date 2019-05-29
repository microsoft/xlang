#include <iostream>

#include "xmeta_idl_reader.h"
#include "ast_to_st_listener.h"
#include "xlang_model_walker.h"
#include "xmeta_emit.h"

#if defined(_WIN32)
#include <Windows.h>
#include <mscoree.h>
#include <cor.h>
#include <winrt/base.h>
#endif

#pragma execution_character_set("utf-8")

using namespace antlr4;
using namespace winrt;
using namespace xlang::xmeta;

std::string_view remove_extension(std::string_view const filename) {
    size_t lastdot = filename.find_last_of(".");
    if (lastdot == std::string::npos) return filename;
    return filename.substr(0, lastdot);
}

std::string_view remove_path(std::string_view const filename) {
    size_t lastdot = filename.find_last_of("\\");
    if (lastdot == std::string::npos) return filename;
    return filename.substr(lastdot + 1, filename.length());
}

// TODO: #350 https://github.com/Microsoft/xlang/issues/350. This is very WIP.
int main(int argc, const char* argv[])
{
    std::ifstream stream;
    stream.open(argv[1]);
    std::vector<std::string> imports;
    xmeta_idl_reader reader{ "", imports};
    reader.read(stream);
    reader.save_to_current_path();
}
