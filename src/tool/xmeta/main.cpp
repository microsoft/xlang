#include <iostream>

#include "antlr4-runtime.h"
#include "XlangParser.h"
#include "XlangLexer.h"

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

std::string remove_extension(const std::string& filename) {
    size_t lastdot = filename.find_last_of(".");
    if (lastdot == std::string::npos) return filename;
    return filename.substr(0, lastdot);
}

std::string remove_path(const std::string& filename) {
    size_t lastdot = filename.find_last_of("\\");
    if (lastdot == std::string::npos) return filename;
    return filename.substr(lastdot + 1, filename.length());
}

int main(int argc, const char* argv[])
{
    std::ifstream stream;
    stream.open(argv[1]);
    xmeta_idl_reader reader{ "" };
    reader.read(stream);

    std::string assembly_name = remove_path(remove_extension(std::string(argv[1])));
    xlang_model_walker walker(reader.get_namespaces());
    std::shared_ptr<xmeta_emit> emitter = std::make_shared<xmeta_emit>(assembly_name);

    emitter->initialize();
    walker.register_listener(emitter);
    walker.walk();
    emitter->save_to_file();
}