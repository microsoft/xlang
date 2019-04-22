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
    std::string test_idl =
        "namespace Windows.Test \
        { \
            enum Color \
            { \
                Red, \
                Green, \
                Blue \
            } \
            enum Alignment \
            { \
                Center = 0, \
                Right = 1 \
            } \
            enum Permissions \
            { \
                None = 0x0000, \
                Camera = 0x0001, \
                Microphone = 0x0002, \
            } \
        }";

    std::string assembly_name = "testidl";
    xlang_test_listener listener;
    REQUIRE(setup_and_run_parser2(test_idl, listener) == 0);

    std::vector<std::shared_ptr<xlang::xmeta::namespace_model>> v;

    std::string ns_name("test");
    std::shared_ptr<namespace_model> ns(new namespace_model(ns_name, 0, assembly_name, nullptr));
    std::shared_ptr<namespace_body_model> ns_bm = std::make_shared<namespace_body_model>(namespace_body_model(ns));
    
    std::string enum_name("Color");
    std::shared_ptr<enum_model> enum_m(new enum_model(enum_name, 0, assembly_name, enum_semantics::Uint32));
    std::string enum_red("red");
    std::string enum_val("1");
    enum_m->add_member(enum_red, enum_val, true);

    ns_bm->add_enum(enum_m);
    ns->add_namespace_body(ns_bm);

    v.push_back(ns);

    xlang::xmeta::xlang_model_walker walker(v);
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
