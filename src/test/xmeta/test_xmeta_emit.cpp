#include "pch.h"
#include <iostream>

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

constexpr int TYPE_DEF_OFFSET = 1; // Module
constexpr int TYPE_REF_OFFSET = 3; // System: Enum, Delegate, ValueType

// In-depth checking of type properties of enums
const TypeAttributes struct_type_attributes()
{
    TypeAttributes result{};
    result.Visibility(TypeVisibility::Public);
    result.Sealed(true);
    result.WindowsRuntime(true);
    result.Layout(TypeLayout::SequentialLayout);
    return result;
}

const TypeAttributes enum_type_attributes()
{
    TypeAttributes result{};
    result.Visibility(TypeVisibility::Public);
    result.Sealed(true);
    result.WindowsRuntime(true);
    return result;
}

const TypeAttributes delegate_type_attributes()
{
    TypeAttributes result{};
    result.Visibility(TypeVisibility::Public);
    result.Sealed(true);
    result.WindowsRuntime(true);
    return result;
}

void test_struct_type_properties(TypeDef const& struct_type)
{
    auto const& struct_flags = struct_type.Flags();
    REQUIRE(struct_flags.value == struct_type_attributes().value);
    REQUIRE(struct_type.is_struct());
    REQUIRE(empty(struct_type.MethodList()));
    REQUIRE(empty(struct_type.EventList()));
    REQUIRE(empty(struct_type.GenericParam()));
    REQUIRE(empty(struct_type.InterfaceImpl()));
    REQUIRE(empty(struct_type.MethodImplList()));
    REQUIRE(empty(struct_type.PropertyList()));
}

void test_enum_type_properties(TypeDef const& enum_type)
{
    auto const& enum_flags = enum_type.Flags();
    REQUIRE(enum_flags.value == enum_type_attributes().value); // TODO: Revisit reader flags to enable easy OR'ing together of values
    // These are basic things that should be true of all enums.
    // Offload these to a helper test method so we don't have to duplicate this level of paranoia everywhere.
    REQUIRE(enum_type.is_enum());
    REQUIRE(empty(enum_type.MethodList()));
    REQUIRE(empty(enum_type.EventList()));
    REQUIRE(empty(enum_type.GenericParam()));
    REQUIRE(empty(enum_type.InterfaceImpl()));
    REQUIRE(empty(enum_type.MethodImplList()));
    REQUIRE(empty(enum_type.PropertyList()));
}

void test_delegate_type_properties(TypeDef const& delegate_type)
{
    auto const& enum_flags = delegate_type.Flags();
    REQUIRE(enum_flags.value == enum_type_attributes().value);
    REQUIRE(delegate_type.is_delegate());
    REQUIRE(empty(delegate_type.EventList()));
    REQUIRE(empty(delegate_type.GenericParam())); // Not in the future
    REQUIRE(empty(delegate_type.InterfaceImpl()));
    REQUIRE(empty(delegate_type.MethodImplList()));
    REQUIRE(empty(delegate_type.PropertyList()));
}

std::vector<uint8_t> run_and_save_to_memory(std::istringstream & test_idl, std::string_view assembly_name)
{
    xmeta_idl_reader reader{ "" };
    REQUIRE(reader.read(test_idl) == 0);
    xlang::xmeta::xmeta_emit emitter(assembly_name);
    xlang::xmeta::xlang_model_walker walker(reader.get_namespaces(), emitter);

    walker.register_listener(emitter);
    walker.walk();

    xlang::meta::writer::pe_writer writer;
    writer.add_metadata(emitter.save_to_memory());

    return writer.save_to_memory();
}

TEST_CASE("Assemblies metadata")
{
    constexpr char assembly_name[] = "testidl";
    constexpr char common_assembly_ref[] = "mscorlib";
    xlang::xmeta::xmeta_emit emitter(assembly_name);

    xlang::meta::writer::pe_writer writer;
    writer.add_metadata(emitter.save_to_memory());

    xlang::meta::reader::database db{ writer.save_to_memory() };

    REQUIRE(db.Assembly.size() == 1);
    REQUIRE(db.Assembly[0].Name() == assembly_name);
    REQUIRE(db.AssemblyRef.size() == 1);
    REQUIRE(db.AssemblyRef[0].Name() == common_assembly_ref);
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
    xlang::meta::reader::database db{ run_and_save_to_memory(test_idl, assembly_name) };

    REQUIRE(db.TypeDef.size() == TYPE_DEF_OFFSET + 1);

    auto const& enum_type = db.TypeDef[1];
    REQUIRE(enum_type.TypeNamespace() == "Windows.Test");
    REQUIRE(enum_type.TypeName() == "Color");
    REQUIRE(enum_type.Flags().value == (tdPublic | tdSealed | tdClass | tdAutoLayout | tdWindowsRuntime));
    test_enum_type_properties(enum_type);

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
        REQUIRE(type_ref.TypeNamespace() == "Windows.Test");
        REQUIRE(type_ref.ResolutionScope().type() == ResolutionScope::Module);
    }
}

TEST_CASE("Delegate metadata")
{
    std::istringstream test_idl{ R"(
        namespace Windows.Test
        {
            delegate Int32 testdelegate(Int32 c, Int32 d);
        }
    )" };
    std::string assembly_name = "testidl";
    xlang::meta::reader::database db{ run_and_save_to_memory(test_idl, assembly_name) };
    
    auto const& delegate_type = db.TypeDef[1];
    test_delegate_type_properties(delegate_type);
    REQUIRE(db.TypeDef.size() == TYPE_DEF_OFFSET + 1);
    REQUIRE(db.Param.size() == 5); // return type + two formal parameters + two from delegate constructor parameter
    REQUIRE(db.MethodDef.size() == 2); // One constructor and one invoke method
    REQUIRE(delegate_type.TypeNamespace() == "Windows.Test");
    REQUIRE(delegate_type.TypeName() == "testdelegate");
    REQUIRE(delegate_type.Flags().value == (tdPublic | tdSealed | tdClass | tdWindowsRuntime));

    // Testing constructor method
    auto const& delegate_constructor = db.MethodDef[0];
    REQUIRE(delegate_constructor.Name() == ".ctor");
    REQUIRE(delegate_constructor.Parent().TypeName() == "testdelegate");
    REQUIRE(!delegate_constructor.Signature().ReturnType());
    auto const& param = delegate_constructor.Signature().Params();
    auto const& test = param.first;

    // Testing invoke method
    auto const& delegate_invoke = db.MethodDef[1];
    REQUIRE(delegate_invoke.Name() == "Invoke");
    REQUIRE(delegate_invoke.Parent().TypeName() == "testdelegate");
    REQUIRE(delegate_invoke.Flags().value == (mdVirtual | fdSpecialName | mdHideBySig));
    auto const& delegate_sig = delegate_invoke.Signature();
    // Checking return type signatures
    REQUIRE(std::holds_alternative<ElementType>(delegate_sig.ReturnType().Type().Type()));
    REQUIRE(std::get<ElementType>(delegate_sig.ReturnType().Type().Type()) == ElementType::I4);
    // Checking paramater signatures
    for (auto const& delegate_param : delegate_sig.Params())
    {
        REQUIRE(std::holds_alternative<ElementType>(delegate_param.Type().Type()));
        REQUIRE(std::get<ElementType>(delegate_param.Type().Type()) == ElementType::I4);
    }
}

TEST_CASE("Parameter signature simple type metadata")
{
    std::istringstream test_idl{ R"(
        namespace Windows.Test
        {
            delegate String d1();
            delegate Int8 d2();
            delegate Int16 d3();
            delegate Int32 d4();
            delegate Int64 d5();
            delegate UInt8 d6();
            delegate UInt16 d7();
            delegate UInt32 d8();
            delegate UInt64 d9();
            delegate Single e1();
            delegate Double e2();
            delegate Char16 e3();
            delegate Boolean e4();
            delegate void e5();
        }
    )" };
    std::string assembly_name = "testidl";
    xlang::meta::reader::database db{ run_and_save_to_memory(test_idl, assembly_name) };
    REQUIRE(db.MethodDef.size() == 28); // Each delegate defines two method defs
    // Testing invoke method
    {
        auto const& delegate_invoke = db.MethodDef[1];
        REQUIRE(delegate_invoke.Parent().TypeName() == "d1");
        auto const& delegate_sig = delegate_invoke.Signature();
        REQUIRE(std::get<ElementType>(delegate_sig.ReturnType().Type().Type()) == ElementType::String);
    }
    {
        auto const& delegate_invoke = db.MethodDef[3];
        REQUIRE(delegate_invoke.Parent().TypeName() == "d2");
        auto const& delegate_sig = delegate_invoke.Signature();
        REQUIRE(std::holds_alternative<ElementType>(delegate_sig.ReturnType().Type().Type()));
        REQUIRE(std::get<ElementType>(delegate_sig.ReturnType().Type().Type()) == ElementType::I1);
    }
    {
        auto const& delegate_invoke = db.MethodDef[5];
        REQUIRE(delegate_invoke.Parent().TypeName() == "d3");
        auto const& delegate_sig = delegate_invoke.Signature();
        REQUIRE(std::holds_alternative<ElementType>(delegate_sig.ReturnType().Type().Type()));
        REQUIRE(std::get<ElementType>(delegate_sig.ReturnType().Type().Type()) == ElementType::I2);
    }
    {
        auto const& delegate_invoke = db.MethodDef[7];
        REQUIRE(delegate_invoke.Parent().TypeName() == "d4");
        auto const& delegate_sig = delegate_invoke.Signature();
        REQUIRE(std::holds_alternative<ElementType>(delegate_sig.ReturnType().Type().Type()));
        REQUIRE(std::get<ElementType>(delegate_sig.ReturnType().Type().Type()) == ElementType::I4);
    }
    {
        auto const& delegate_invoke = db.MethodDef[9];
        REQUIRE(delegate_invoke.Parent().TypeName() == "d5");
        auto const& delegate_sig = delegate_invoke.Signature();
        REQUIRE(std::holds_alternative<ElementType>(delegate_sig.ReturnType().Type().Type()));
        REQUIRE(std::get<ElementType>(delegate_sig.ReturnType().Type().Type()) == ElementType::I8);
    }
    {
        auto const& delegate_invoke = db.MethodDef[11];
        REQUIRE(delegate_invoke.Parent().TypeName() == "d6");
        auto const& delegate_sig = delegate_invoke.Signature();
        REQUIRE(std::holds_alternative<ElementType>(delegate_sig.ReturnType().Type().Type()));
        REQUIRE(std::get<ElementType>(delegate_sig.ReturnType().Type().Type()) == ElementType::U1);
    }
    {
        auto const& delegate_invoke = db.MethodDef[13];
        REQUIRE(delegate_invoke.Parent().TypeName() == "d7");
        auto const& delegate_sig = delegate_invoke.Signature();
        REQUIRE(std::holds_alternative<ElementType>(delegate_sig.ReturnType().Type().Type()));
        REQUIRE(std::get<ElementType>(delegate_sig.ReturnType().Type().Type()) == ElementType::U2);
    }
    {
        auto const& delegate_invoke = db.MethodDef[15];
        REQUIRE(delegate_invoke.Parent().TypeName() == "d8");
        auto const& delegate_sig = delegate_invoke.Signature();
        REQUIRE(std::holds_alternative<ElementType>(delegate_sig.ReturnType().Type().Type()));
        REQUIRE(std::get<ElementType>(delegate_sig.ReturnType().Type().Type()) == ElementType::U4);
    }
    {
        auto const& delegate_invoke = db.MethodDef[17];
        REQUIRE(delegate_invoke.Parent().TypeName() == "d9");
        auto const& delegate_sig = delegate_invoke.Signature();
        REQUIRE(std::holds_alternative<ElementType>(delegate_sig.ReturnType().Type().Type()));
        REQUIRE(std::get<ElementType>(delegate_sig.ReturnType().Type().Type()) == ElementType::U8);
    }
    {
        auto const& delegate_invoke = db.MethodDef[19];
        REQUIRE(delegate_invoke.Parent().TypeName() == "e1");
        auto const& delegate_sig = delegate_invoke.Signature();
        REQUIRE(std::holds_alternative<ElementType>(delegate_sig.ReturnType().Type().Type()));
        REQUIRE(std::get<ElementType>(delegate_sig.ReturnType().Type().Type()) == ElementType::R4);
    }
    {
        auto const& delegate_invoke = db.MethodDef[21];
        REQUIRE(delegate_invoke.Parent().TypeName() == "e2");
        auto const& delegate_sig = delegate_invoke.Signature();
        REQUIRE(std::holds_alternative<ElementType>(delegate_sig.ReturnType().Type().Type()));
        REQUIRE(std::get<ElementType>(delegate_sig.ReturnType().Type().Type()) == ElementType::R8);
    }
    {
        auto const& delegate_invoke = db.MethodDef[23];
        REQUIRE(delegate_invoke.Parent().TypeName() == "e3");
        auto const& delegate_sig = delegate_invoke.Signature();
        REQUIRE(std::holds_alternative<ElementType>(delegate_sig.ReturnType().Type().Type()));
        REQUIRE(std::get<ElementType>(delegate_sig.ReturnType().Type().Type()) == ElementType::Char);
    }
    {
        auto const& delegate_invoke = db.MethodDef[25];
        REQUIRE(delegate_invoke.Parent().TypeName() == "e4");
        auto const& delegate_sig = delegate_invoke.Signature();
        REQUIRE(std::holds_alternative<ElementType>(delegate_sig.ReturnType().Type().Type()));
        REQUIRE(std::get<ElementType>(delegate_sig.ReturnType().Type().Type()) == ElementType::Boolean);
    }
    {
        auto const& delegate_invoke = db.MethodDef[27];
        REQUIRE(delegate_invoke.Parent().TypeName() == "e5");
        auto const& delegate_sig = delegate_invoke.Signature();
        REQUIRE(!delegate_invoke.Signature().ReturnType());
    }
}

TEST_CASE("Parameter signature class reference type metadata across namespace")
{
    std::istringstream test_idl{ R"(
        namespace Windows.Test
        {
            enum MyClass
            {
            }
        }
        namespace A
        {
            delegate Windows.Test.MyClass d1();
        }
    )" };
    std::string assembly_name = "testidl";
    xlang::meta::reader::database db{ run_and_save_to_memory(test_idl, assembly_name) };

    REQUIRE(db.MethodDef.size() == 2);
    REQUIRE(db.TypeRef.size() == TYPE_REF_OFFSET + 2);
    REQUIRE(db.TypeDef.size() == TYPE_DEF_OFFSET + 2);

    REQUIRE(db.TypeRef[4].TypeName() == "MyClass");
    REQUIRE(db.TypeRef[3].TypeName() == "d1");

    // Testing invoke method
    {
        auto const& delegate_invoke = db.MethodDef[1];
        REQUIRE(delegate_invoke.Parent().TypeName() == "d1");
        auto const& delegate_sig = delegate_invoke.Signature();
        REQUIRE(std::get<coded_index<TypeDefOrRef>>(delegate_sig.ReturnType().Type().Type()).TypeRef() == db.TypeRef[4]);
    }
}

TEST_CASE("Parameter signature class reference type metadata")
{
    std::istringstream test_idl{ R"(
        namespace Windows.Test
        {
            enum MyClass
            {
            }
            delegate MyClass d1();
        }
    )" };
    std::string assembly_name = "testidl";
    xlang::meta::reader::database db{ run_and_save_to_memory(test_idl, assembly_name) };

    REQUIRE(db.MethodDef.size() == 2);
    REQUIRE(db.TypeRef.size() == TYPE_REF_OFFSET + 2);
    REQUIRE(db.TypeDef.size() == TYPE_DEF_OFFSET + 2);

    REQUIRE(db.TypeRef[3].TypeName() == "MyClass");
    REQUIRE(db.TypeRef[4].TypeName() == "d1");

    // Testing invoke method
    {
        auto const& delegate_invoke = db.MethodDef[1];
        REQUIRE(delegate_invoke.Parent().TypeName() == "d1");
        auto const& delegate_sig = delegate_invoke.Signature();
        REQUIRE(std::get<coded_index<TypeDefOrRef>>(delegate_sig.ReturnType().Type().Type()).TypeRef() == db.TypeRef[3]);
    }
}

TEST_CASE("Struct simple type metadata")
{
    std::istringstream struct_test_idl{ R"(
        namespace N
        {
            struct S
            {
                Boolean field_1;
                String field_2;
                Int16 field_3;
                Int32 field_4;
                Int64 field_5;
                UInt8 field_6;
                UInt16 field_7;
                UInt32 field_8;
                Char16 field_9;
                Single field_10;
                Double field_11;
            };
        }
    )" };
    const std::vector<ElementType> elements_to_check 
        = { ElementType::Boolean, ElementType::String, ElementType::I2, 
        ElementType::I4, ElementType::I8, ElementType::U1, ElementType::U2, 
        ElementType::U4, ElementType::Char, ElementType::R4, ElementType::R8};
    std::string assembly_name = "testidl";
    xlang::meta::reader::database db{ run_and_save_to_memory(struct_test_idl, assembly_name) };
    
    REQUIRE(db.TypeRef.size() == TYPE_REF_OFFSET + 1);
    REQUIRE(db.TypeDef.size() == TYPE_DEF_OFFSET + 1);
    REQUIRE(db.Field.size() == 11);

    REQUIRE(db.TypeRef[3].TypeName() == "S");
    REQUIRE(db.TypeDef[1].TypeName() == "S");

    test_struct_type_properties(db.TypeDef[1]);

    for (size_t i = 0; i < db.Field.size(); i++)
    {
        auto const& struct_field = db.Field[i];
        REQUIRE(struct_field.Parent().TypeName() == "S");
        auto const& struct_field_sig = struct_field.Signature();
        REQUIRE(std::get<ElementType>(struct_field_sig.Type().Type()) == elements_to_check[i]);
    }
}

TEST_CASE("Struct class type metadata")
{
    std::istringstream struct_test_idl{ R"(
        namespace N
        {
            enum E0
            {
            }

            struct S0
            {
                S1 field_1;
                E0 field_2;
            };

            struct S1
            {
            }
        }
    )" };
    const std::vector<ElementType> elements_to_check
        = { ElementType::Boolean, ElementType::String, ElementType::I2,
        ElementType::I4, ElementType::I8, ElementType::U1, ElementType::U2,
        ElementType::U4, ElementType::Char, ElementType::R4, ElementType::R8 };
    std::string assembly_name = "testidl";
    xlang::meta::reader::database db{ run_and_save_to_memory(struct_test_idl, assembly_name) };

    REQUIRE(db.TypeRef.size() == TYPE_REF_OFFSET + 3);
    REQUIRE(db.TypeDef.size() == TYPE_DEF_OFFSET + 3);

    REQUIRE(db.TypeRef[3].TypeName() == "S0");
    REQUIRE(db.TypeRef[4].TypeName() == "S1");
    REQUIRE(db.TypeRef[5].TypeName() == "E0");

    REQUIRE(db.TypeDef[1].TypeName() == "S0");
    test_struct_type_properties(db.TypeDef[1]);

    REQUIRE(db.Field.size() == 3);
    {
        auto const& struct_field = db.Field[0];
        REQUIRE(struct_field.Parent().TypeName() == db.TypeDef[1].TypeName());
        auto const& struct_field_sig = struct_field.Signature();
        REQUIRE(std::get<coded_index<TypeDefOrRef>>(struct_field_sig.Type().Type()).TypeRef() == db.TypeRef[4]);
    }
    {
        auto const& struct_field = db.Field[1];
        REQUIRE(struct_field.Parent().TypeName() == db.TypeDef[1].TypeName());
        auto const& struct_field_sig = struct_field.Signature();
        REQUIRE(std::get<coded_index<TypeDefOrRef>>(struct_field_sig.Type().Type()).TypeRef() == db.TypeRef[5]);
    }
}
