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
#include "xmeta_idl_reader.h"

using namespace antlr4;

#pragma execution_character_set("utf-8")

using namespace xlang::xmeta;

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
    return parser.getNumberOfSyntaxErrors();
}

TEST_CASE("Assembly name metadata") 
{
    std::string assembly_name = "testidl";
    std::shared_ptr<xlang::xmeta::xmeta_emit> emitter = std::make_shared<xlang::xmeta::xmeta_emit>(assembly_name);
    emitter->initialize();

    xlang::meta::writer::pe_writer writer;
    writer.add_metadata(emitter->save_to_memory());

    xlang::meta::reader::database db{ writer.save_to_memory() };

    REQUIRE(db.Assembly.size() == 1);
    REQUIRE(assembly_name.compare(db.Assembly[0].Name()) == 0);

    emitter->uninitialize();
}

TEST_CASE("Enum metadata")
{
    std::istringstream test_idl{ R"(
        namespace Windows.Test
        {
            enum Color
            {
                Red,
                Green,
                Blue
            }
        }
    )" };
    std::string assembly_name = "testidl";
    xmeta_idl_reader reader{ "" };
    REQUIRE(reader.read(test_idl) == 0);
    xlang::xmeta::xlang_model_walker walker(reader.get_namespaces());

    std::shared_ptr<xlang::xmeta::xmeta_emit> emitter = std::make_shared<xlang::xmeta::xmeta_emit>(assembly_name);

    emitter->initialize();
    walker.register_listener(emitter);
    walker.walk();

    xlang::meta::writer::pe_writer writer;
    writer.add_metadata(emitter->save_to_memory());
    xlang::meta::reader::database db{ writer.save_to_memory() };

    REQUIRE(db.TypeDef.size() == 2);
    REQUIRE(db.TypeDef[1].TypeName().compare("Color") == 0);
    emitter->uninitialize();
}
