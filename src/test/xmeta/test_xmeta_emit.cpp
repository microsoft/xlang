#include "pch.h"
#include <iostream>

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
using namespace xlang::meta::reader;
using namespace std;

// In-depth checking of type properties of enums
const TypeAttributes enum_type_attributes()
{
    TypeAttributes result{};
    result.Visibility(TypeVisibility::Public);
    result.Sealed(true);
    result.WindowsRuntime(true);
    return result;
}

TEST_CASE("Assemblies metadata") 
{
    std::string assembly_name = "testidl";
    std::string common_assembly_ref = "mscorlib";
    std::shared_ptr<xlang::xmeta::xmeta_emit> emitter = std::make_shared<xlang::xmeta::xmeta_emit>(assembly_name);
    emitter->initialize();

    xlang::meta::writer::pe_writer writer;
    writer.add_metadata(emitter->save_to_memory());

    xlang::meta::reader::database db{ writer.save_to_memory() };

    REQUIRE(db.Assembly.size() == 1);
    REQUIRE(db.Assembly[0].Name() == assembly_name);
    REQUIRE(db.AssemblyRef.size() == 1);
    REQUIRE(db.AssemblyRef[0].Name() == common_assembly_ref);
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

    auto const& enum_type = db.TypeDef[1];
    REQUIRE(enum_type.TypeName() == "Color");
    // REQUIRE(enum_type.TypeNamespace() == "Xmeta.Test"); // TODO: populate metadata with namespace

    auto const& fields = enum_type.FieldList();
    REQUIRE(size(fields) == 4); // # of enumerators plus one for the value

    {
        auto const& value_field = fields.first[0];
        REQUIRE(value_field.Flags().value == (fdRTSpecialName | fdSpecialName | fdPrivate));
        REQUIRE(value_field.Name() == "value__");
        REQUIRE(!value_field.Constant());
    }

    const std::string_view enum_names[3] = { "Red", "Green", "Blue" };
    const int32_t enum_values[3] = { 0, 1, 2 };
    for (size_t i = 1; i < size(fields); i++)
    {
        auto const& enum_field = fields.first[i];
        REQUIRE(enum_field.Flags().value == (fdHasDefault | fdLiteral | fdStatic | fdPublic));
        REQUIRE(enum_field.Name() == enum_names[i - 1]);
        REQUIRE(enum_field.Constant().ValueInt32() == enum_values[i - 1]);

        auto const& field_sig = enum_field.Signature();
        REQUIRE(empty(field_sig.CustomMod()));
        REQUIRE(field_sig.get_CallingConvention() == CallingConvention::Field);
        REQUIRE(!field_sig.Type().is_szarray());

        auto const& coded_type = std::get<coded_index<TypeDefOrRef>>(field_sig.Type().Type());
        REQUIRE(coded_type.type() == TypeDefOrRef::TypeRef);

        auto const& type_ref = coded_type.TypeRef();
        REQUIRE(type_ref.TypeName() == "Color");
        // REQUIRE(type_ref.TypeNamespace() == "Xmeta.Test"); // TODO: populate metadata with namespace
        REQUIRE(type_ref.ResolutionScope().type() == ResolutionScope::Module);
    }

    emitter->uninitialize();
}

