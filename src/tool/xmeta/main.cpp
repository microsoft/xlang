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

// TODO: #350 https://github.com/Microsoft/xlang/issues/350
int main(int argc, const char* argv[])
{
    std::ifstream stream;
    stream.open(argv[1]);
    xmeta_idl_reader reader{ "" };
    reader.read(stream);

    std::string_view assembly_name = remove_path(argv[1]);
    xmeta_emit emitter(assembly_name);
    xlang_model_walker walker(reader.get_namespaces(), emitter);
    walker.walk();

    assembly_name = remove_extension(assembly_name);
    xlang::meta::writer::pe_writer writer;
    writer.add_metadata(emitter.save_to_memory());

    // Temporarily saving as a winmd to allow IL spy and dsm to work
    writer.save_to_file(std::filesystem::current_path().append(std::string(assembly_name) + ".winmd"));
}
