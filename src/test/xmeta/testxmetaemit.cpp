#include <iostream>
#include "pch.h"

#include "antlr4-runtime.h"
#include "XlangParser.h"
#include "XlangLexer.h"

#include "xlang_model_walker.h"
#include "xmeta_emit.h"
#include "xmeta_models.h"
#include "xlang_test_listener.h"

#include "meta_reader.h"
#include "meta_writer.h"

using namespace antlr4;

#pragma execution_character_set("utf-8")

int setup_and_run_parser2(std::string const& idl, xlang_test_listener &listener, bool disable_error_reporting = false)
{
    ANTLRInputStream input(idl);
    XlangLexer lexer(&input);

    CommonTokenStream tokens(&lexer);
    XlangParser parser(&tokens);

    if (disable_error_reporting) {
        lexer.removeErrorListeners();
        parser.removeErrorListeners();
    }

    tree::ParseTree *tree = parser.xlang();
    tree::ParseTreeWalker::DEFAULT.walk(&listener, tree);
    return parser.getNumberOfSyntaxErrors();
}

TEST_CASE("Assembly name metadata") 
{
    std::string assembly_namme = "testidl";
    std::shared_ptr<xlang::xmeta::xmeta_emit> emitter = std::make_shared<xlang::xmeta::xmeta_emit>(assembly_namme);
    emitter->initialize();

    std::vector<uint8_t> metadata;
    emitter->save_to_memory(&metadata);

    xlang::meta::writer::pe_writer writer;
    writer.add_metadata(metadata);

    xlang::meta::reader::database db{ writer.save_to_memory() };

    REQUIRE(db.Assembly.size() == 1);
    REQUIRE(assembly_namme.compare(db.Assembly[0].Name()) == 0);

    emitter->uninitialize();
}

TEST_CASE("Enum metadata")
{
    //std::string test_idl =
    //    "namespace Windows.Test \
    //    { \
    //        enum Color \
    //        { \
    //            Red, \
    //            Green, \
    //            Blue \
    //        } \
    //        enum Alignment \
    //        { \
    //            Center = 0, \
    //            Right = 1 \
    //        } \
    //        enum Permissions \
    //        { \
    //            None = 0x0000, \
    //            Camera = 0x0001, \
    //            Microphone = 0x0002, \
    //        } \
    //    }";

    //xlang_test_listener listener;
    //REQUIRE(setup_and_run_parser2(test_idl, listener) == 0);

    //std::vector<std::shared_ptr<xlang::xmeta::namespace_model>> v;

    //xlang::xmeta::xlang_model_walker walker(v);
    //std::string assembly_namme = "testidl";
    //std::shared_ptr<xlang::xmeta::xmeta_emit> emitter = std::make_shared<xlang::xmeta::xmeta_emit>(assembly_namme);
    ////
    //emitter->initialize();
    //walker.register_listener(emitter);
    ////walker.walk();

    //std::vector<uint8_t> metadata;
    //emitter->save_to_memory(&metadata);

    //xlang::meta::writer::pe_writer writer;
    //writer.add_metadata(metadata);

    //xlang::meta::reader::database db{ writer.save_to_memory() };

    //REQUIRE(db.Assembly.size() == 1);
    //REQUIRE(assembly_namme.compare(db.Assembly[0].Name()) == 0);

    //emitter->uninitialize();
}
