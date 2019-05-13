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

// All the flags use in the metadata representation
const TypeAttributes interface_type_attributes()
{
    TypeAttributes result{};
    result.Visibility(TypeVisibility::Public);
    result.Semantics(TypeSemantics::Interface);
    result.Abstract(true);
    result.WindowsRuntime(true);
    return result;
}

const MethodAttributes interface_method_attributes()
{
    MethodAttributes result{};
    result.Access(MemberAccess::Public);
    result.Virtual(true);
    result.HideBySig(true);
    result.Abstract(true);
    result.Layout(VtableLayout::NewSlot);
    return result;
}

const MethodAttributes interface_method_property_attributes()
{
    MethodAttributes result{};
    result.Access(MemberAccess::Public);
    result.Virtual(true);
    result.HideBySig(true);
    result.Abstract(true);
    result.Layout(VtableLayout::NewSlot);
    result.SpecialName(true);
    return result;
}

const TypeAttributes struct_type_attributes()
{
    TypeAttributes result{};
    result.Visibility(TypeVisibility::Public);
    result.Sealed(true);
    result.WindowsRuntime(true);
    result.Layout(TypeLayout::SequentialLayout);
    return result;
}

const FieldAttributes struct_field_attributes()
{
    FieldAttributes result{};
    result.Access(MemberAccess::Public);
    return result;
}

const TypeAttributes enum_type_attributes()
{
    TypeAttributes result{};
    result.Visibility(TypeVisibility::Public);
    result.Sealed(true);
    result.WindowsRuntime(true);
    result.Layout(TypeLayout::AutoLayout);
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

const MethodAttributes delegate_constructor_attributes()
{
    MethodAttributes result{};
    result.RTSpecialName(true);
    result.SpecialName(true);
    result.HideBySig(true);
    result.Access(MemberAccess::Private);
    return result;
}

const MethodAttributes delegate_invoke_attributes()
{
    MethodAttributes result{};
    result.SpecialName(true);
    result.HideBySig(true);
    result.Virtual(true);
    result.Access(MemberAccess::Public);
    return result;
}

const MethodAttributes method_attributes_no_flags()
{
    MethodAttributes result{};
    return result;
}

const MethodImplAttributes delegate_method_impl_attribtes()
{
    MethodImplAttributes result{};
    result.CodeType(CodeType::Runtime);
    return result;
}

const FieldAttributes enum_value_field_attributes()
{
    FieldAttributes result{};
    result.RTSpecialName(true);
    result.SpecialName(true);
    result.Access(MemberAccess::Private);
    return result;
}

const FieldAttributes enum_fields_attributes()
{
    FieldAttributes result{};
    result.Static(true);
    result.Literal(true);
    result.HasDefault(true);
    result.Access(MemberAccess::Public);
    return result;
}

const ParamAttributes param_attributes_no_flags()
{
    ParamAttributes result{};
    return result;
}

const ParamAttributes param_attributes_in_flag()
{
    ParamAttributes result{};
    result.In(true);
    return result;
}

const ParamAttributes param_attributes_out_flag()
{
    ParamAttributes result{};
    result.Out(true);
    return result;
}

// In-depth checking of type properties of structs
void test_interface_type_properties(TypeDef const& interface_type)
{
    auto const& interface_flags = interface_type.Flags();
    REQUIRE(interface_flags.value == interface_type_attributes().value);
    REQUIRE(!interface_type.Extends());
}

// In-depth checking of type properties of structs
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

// In-depth checking of type properties of enums
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

    auto const& fields = enum_type.FieldList();
    {
        auto const& value_field = fields.first[0];
        REQUIRE(value_field.Flags().value == enum_value_field_attributes().value);
        REQUIRE(value_field.Name() == "value__");
        REQUIRE(!value_field.Constant());
    }
}

// In-depth checking of type properties of delegates
void test_delegate_type_properties(TypeDef const& delegate_type)
{
    auto const& delegate_flag = delegate_type.Flags();
    REQUIRE(delegate_flag.value == delegate_type_attributes().value);
    REQUIRE(delegate_type.is_delegate());
    REQUIRE(empty(delegate_type.EventList()));
    REQUIRE(empty(delegate_type.InterfaceImpl()));
    REQUIRE(empty(delegate_type.MethodImplList()));
    REQUIRE(empty(delegate_type.PropertyList()));
    REQUIRE(size(delegate_type.MethodList()) == 2);

    // Constructor
    auto const& delegate_constructor = delegate_type.MethodList().first[0];
    REQUIRE(delegate_constructor.Name() == ".ctor");
    REQUIRE(!delegate_constructor.Signature().ReturnType());
    REQUIRE(size(delegate_constructor.Signature().Params()) == 2);
    REQUIRE(size(delegate_constructor.ParamList()) == 2);
    REQUIRE(delegate_constructor.ImplFlags().value == delegate_method_impl_attribtes().value);
    REQUIRE(delegate_constructor.Flags().value == delegate_constructor_attributes().value);
    auto const& delegate_constructor_sig = delegate_constructor.Signature();

    // Checking params
    {
        REQUIRE(delegate_constructor.ParamList().first[0].Name() == "object");
        REQUIRE(delegate_constructor.ParamList().first[0].Sequence() == 1);
        REQUIRE(delegate_constructor.ParamList().first[0].Flags().value == param_attributes_no_flags().value);
        auto const& delegate_param_sig = delegate_constructor_sig.Params().first[0];
        REQUIRE(std::holds_alternative<ElementType>(delegate_param_sig.Type().Type()));
        REQUIRE(std::get<ElementType>(delegate_param_sig.Type().Type()) == ElementType::Object);
    }
    {
        REQUIRE(delegate_constructor.ParamList().first[1].Name() == "method");
        REQUIRE(delegate_constructor.ParamList().first[1].Sequence() == 2);
        REQUIRE(delegate_constructor.ParamList().first[1].Flags().value == param_attributes_no_flags().value);
        auto const& delegate_param_sig = delegate_constructor_sig.Params().first[1];
        REQUIRE(std::holds_alternative<ElementType>(delegate_param_sig.Type().Type()));
        REQUIRE(std::get<ElementType>(delegate_param_sig.Type().Type()) == ElementType::I);
    }

    // Invoke method
    auto const& delegate_invoke = delegate_type.MethodList().first[1];
    REQUIRE(delegate_invoke.Name() == "Invoke");
    REQUIRE(delegate_invoke.Flags().value == delegate_invoke_attributes().value);
}

std::vector<uint8_t> run_and_save_to_memory(std::istringstream & test_idl, std::string_view assembly_name)
{
    xmeta_idl_reader reader{ "" };
    reader.read(test_idl);
    REQUIRE(reader.get_num_syntax_errors() == 0);
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
    test_enum_type_properties(enum_type);

    auto const& fields = enum_type.FieldList();
    REQUIRE(size(fields) == 4); // # of enumerators plus one for the value

    const std::string_view enum_names[3] = { "Red", "Green", "Blue" };
    const int32_t enum_values[3] = { 0, 1, 2 };
    for (size_t i = 1; i < size(fields); i++)
    {
        auto const& enum_field = fields.first[i];
        REQUIRE(enum_field.Flags().value == enum_fields_attributes().value);
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
            delegate Int16 testdelegate(Int32 c, out Int64 d);
        }
    )" };
    std::string assembly_name = "testidl";
    xlang::meta::reader::database db{ run_and_save_to_memory(test_idl, assembly_name) };
    
    auto const& delegate_type = db.TypeDef[1];

    REQUIRE(db.TypeDef.size() == TYPE_DEF_OFFSET + 1);
    REQUIRE(db.Param.size() == 5); // return type + two formal parameters + two from delegate constructor parameter
    REQUIRE(db.MethodDef.size() == 2); // One constructor and one invoke method
    REQUIRE(delegate_type.TypeNamespace() == "Windows.Test");
    REQUIRE(delegate_type.TypeName() == "testdelegate");
    test_delegate_type_properties(delegate_type);

    
    // Testing invoke method
    auto const& delegate_invoke = delegate_type.MethodList().first[1];
    // Checking return type
    auto const& delegate_sig = delegate_invoke.Signature();
    REQUIRE(std::holds_alternative<ElementType>(delegate_sig.ReturnType().Type().Type()));
    REQUIRE(std::get<ElementType>(delegate_sig.ReturnType().Type().Type()) == ElementType::I2);
    REQUIRE(delegate_invoke.ParamList().first[0].Name() == "returnVal");
    REQUIRE(delegate_invoke.ParamList().first[0].Sequence() == 0);

    // Checking params
    {
        REQUIRE(delegate_invoke.ParamList().first[1].Name() == "c");
        REQUIRE(delegate_invoke.ParamList().first[1].Sequence() == 1);
        REQUIRE(delegate_invoke.ParamList().first[1].Flags().value == param_attributes_in_flag().value);
        auto const& delegate_param_sig = delegate_sig.Params().first[0];
        REQUIRE(std::holds_alternative<ElementType>(delegate_param_sig.Type().Type()));
        REQUIRE(std::get<ElementType>(delegate_param_sig.Type().Type()) == ElementType::I4);
    }
    {
        REQUIRE(delegate_invoke.ParamList().first[2].Name() == "d");
        REQUIRE(delegate_invoke.ParamList().first[2].Sequence() == 2);
        REQUIRE(delegate_invoke.ParamList().first[2].Flags().value == param_attributes_out_flag().value);
        auto const& delegate_param_sig = delegate_sig.Params().first[1];
        REQUIRE(std::holds_alternative<ElementType>(delegate_param_sig.Type().Type()));
        REQUIRE(std::get<ElementType>(delegate_param_sig.Type().Type()) == ElementType::I8);
    }
}

void check_simple_types(MethodDef method, std::string name, ElementType type)
{
    REQUIRE(method.Parent().TypeName() == name);
    auto const& delegate_sig = method.Signature();
    REQUIRE(std::get<ElementType>(delegate_sig.ReturnType().Type().Type()) == type);
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

    check_simple_types(db.TypeDef[TYPE_DEF_OFFSET + 0].MethodList().first[1], "d1", ElementType::String);
    check_simple_types(db.TypeDef[TYPE_DEF_OFFSET + 1].MethodList().first[1], "d2", ElementType::I1);
    check_simple_types(db.TypeDef[TYPE_DEF_OFFSET + 2].MethodList().first[1], "d3", ElementType::I2);
    check_simple_types(db.TypeDef[TYPE_DEF_OFFSET + 3].MethodList().first[1], "d4", ElementType::I4);
    check_simple_types(db.TypeDef[TYPE_DEF_OFFSET + 4].MethodList().first[1], "d5", ElementType::I8);
    check_simple_types(db.TypeDef[TYPE_DEF_OFFSET + 5].MethodList().first[1], "d6", ElementType::U1);
    check_simple_types(db.TypeDef[TYPE_DEF_OFFSET + 6].MethodList().first[1], "d7", ElementType::U2);
    check_simple_types(db.TypeDef[TYPE_DEF_OFFSET + 7].MethodList().first[1], "d8", ElementType::U4);
    check_simple_types(db.TypeDef[TYPE_DEF_OFFSET + 8].MethodList().first[1], "d9", ElementType::U8);
    check_simple_types(db.TypeDef[TYPE_DEF_OFFSET + 9].MethodList().first[1], "e1", ElementType::R4);
    check_simple_types(db.TypeDef[TYPE_DEF_OFFSET + 10].MethodList().first[1], "e2", ElementType::R8);
    check_simple_types(db.TypeDef[TYPE_DEF_OFFSET + 11].MethodList().first[1], "e3", ElementType::Char);
    check_simple_types(db.TypeDef[TYPE_DEF_OFFSET + 12].MethodList().first[1], "e4", ElementType::Boolean);

    auto const& void_return_method = db.TypeDef[TYPE_DEF_OFFSET + 13].MethodList().first[1];
    REQUIRE(void_return_method.Parent().TypeName() == "e5");
    REQUIRE(!void_return_method.Signature().ReturnType());
}

TEST_CASE("Parameter signature class reference type metadata across namespace")
{
    std::istringstream test_idl{ R"(
        namespace Windows.Test
        {
            enum e1
            {
            }
        }
        namespace A
        {
            delegate Windows.Test.e1 d1();
        }
    )" };
    std::string assembly_name = "testidl";
    xlang::meta::reader::database db{ run_and_save_to_memory(test_idl, assembly_name) };
    REQUIRE(db.MethodDef.size() == 2);
    REQUIRE(db.TypeRef.size() == TYPE_REF_OFFSET + 2);
    REQUIRE(db.TypeDef.size() == TYPE_DEF_OFFSET + 2);
    REQUIRE(db.TypeRef[TYPE_REF_OFFSET + 1].TypeName() == "e1");
    REQUIRE(db.TypeRef[TYPE_REF_OFFSET].TypeName() == "d1");

    auto const e1 = db.TypeDef[TYPE_DEF_OFFSET + 1];
    auto const d1 = db.TypeDef[TYPE_DEF_OFFSET];
    REQUIRE(e1.TypeName() == "e1");
    REQUIRE(d1.TypeName() == "d1");
    test_enum_type_properties(e1);
    test_delegate_type_properties(d1);

    // Check that type Ref is cottrect
    {
        auto const& delegate_invoke = d1.MethodList().first[1];
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
            enum e1
            {
            }
            delegate e1 d1();
        }
    )" };
    std::string assembly_name = "testidl";
    xlang::meta::reader::database db{ run_and_save_to_memory(test_idl, assembly_name) };
    REQUIRE(db.MethodDef.size() == 2);
    REQUIRE(db.TypeRef.size() == TYPE_REF_OFFSET + 2);
    REQUIRE(db.TypeDef.size() == TYPE_DEF_OFFSET + 2);
    REQUIRE(db.TypeRef[TYPE_REF_OFFSET].TypeName() == "e1");
    REQUIRE(db.TypeRef[TYPE_REF_OFFSET + 1].TypeName() == "d1");

    auto const e1 = db.TypeDef[TYPE_DEF_OFFSET];
    auto const d1 = db.TypeDef[TYPE_DEF_OFFSET + 1];
    REQUIRE(e1.TypeName() == "e1");
    REQUIRE(d1.TypeName() == "d1");
    test_enum_type_properties(e1);
    test_delegate_type_properties(d1);
    // Check that type Ref is cottrect
    {
        auto const& delegate_invoke = d1.MethodList().first[1];
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

    auto const struct0 = db.TypeDef[TYPE_DEF_OFFSET];
    REQUIRE(db.TypeRef[TYPE_REF_OFFSET].TypeName() == "S");
    REQUIRE(struct0.TypeName() == "S");
    REQUIRE(struct0.TypeNamespace() == "N");
    test_struct_type_properties(struct0);

    for (size_t i = 0; i < size(struct0.FieldList()) ; i++)
    {
        auto const& struct_field = struct0.FieldList().first[i];
        REQUIRE(struct_field.Parent().TypeName() == "S");
        REQUIRE(struct_field.Flags().value == struct_field_attributes().value);
        auto const& struct_field_sig = struct_field.Signature();
        REQUIRE(struct_field_sig.Type().element_type() == elements_to_check[i]);
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

    REQUIRE(db.TypeRef[TYPE_REF_OFFSET + 0].TypeName() == "S0");
    REQUIRE(db.TypeRef[TYPE_REF_OFFSET + 1].TypeName() == "S1");
    REQUIRE(db.TypeRef[TYPE_REF_OFFSET + 2].TypeName() == "E0");

    auto const& struct0 = db.TypeDef[TYPE_DEF_OFFSET];
    REQUIRE(struct0.TypeName() == "S0");
    REQUIRE(struct0.TypeNamespace() == "N");
    test_struct_type_properties(struct0);

    REQUIRE(db.Field.size() == 3);
    {
        auto const& struct_field = struct0.FieldList().first[0];
        REQUIRE(struct_field.Parent().TypeName() == db.TypeDef[1].TypeName());
        auto const& struct_field_sig = struct_field.Signature();
        REQUIRE(struct_field.Flags().value == struct_field_attributes().value);
        REQUIRE(std::get<coded_index<TypeDefOrRef>>(struct_field_sig.Type().Type()).TypeRef() == db.TypeRef[4]);
    }
    {
        auto const& struct_field = struct0.FieldList().first[1];
        REQUIRE(struct_field.Parent().TypeName() == db.TypeDef[1].TypeName());
        auto const& struct_field_sig = struct_field.Signature();
        REQUIRE(struct_field.Flags().value == struct_field_attributes().value);
        REQUIRE(std::get<coded_index<TypeDefOrRef>>(struct_field_sig.Type().Type()).TypeRef() == db.TypeRef[5]);
    }
}

TEST_CASE("Interface method metadata")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            struct S1 {}    
            interface IComboBox 
            {
                S1 Draw(E1 param1);
            }
            enum E1 {}  
        }
    )" };
    std::string assembly_name = "testidl";
    xlang::meta::reader::database db{ run_and_save_to_memory(test_idl, assembly_name) };

    REQUIRE(db.TypeRef.size() == TYPE_REF_OFFSET + 3);

    auto const& E1_ref = db.TypeRef[TYPE_REF_OFFSET + 2];
    auto const& ICombo_ref = db.TypeRef[TYPE_REF_OFFSET + 1];
    auto const& S1_ref = db.TypeRef[TYPE_REF_OFFSET + 0];
    REQUIRE(E1_ref.TypeName() == "E1");
    REQUIRE(ICombo_ref.TypeName() == "IComboBox");
    REQUIRE(S1_ref.TypeName() == "S1");

    REQUIRE(db.TypeDef.size() == TYPE_DEF_OFFSET + 3);
    REQUIRE(db.TypeDef[TYPE_DEF_OFFSET + 1].TypeName() == "IComboBox");
    auto const& combo = db.TypeDef[TYPE_DEF_OFFSET + 1];
    test_interface_type_properties(combo);

    REQUIRE(size(combo.MethodList()) == 1);
    auto const& method_list = combo.MethodList();

    REQUIRE(method_list.first[0].Name() == "Draw");
    REQUIRE(method_list.first[0].Flags().value == interface_method_attributes().value);
    {
        auto const& Draw_sig = method_list.first[0].Signature();
        REQUIRE(std::get<coded_index<TypeDefOrRef>>(Draw_sig.ReturnType().Type().Type()).TypeRef() == S1_ref);
        auto const& Draw_param_sig = Draw_sig.Params().first[0];
        REQUIRE(std::get<coded_index<TypeDefOrRef>>(Draw_param_sig.Type().Type()).TypeRef() == E1_ref);
    }
}

TEST_CASE("Interface property metadata")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            struct S1 {}    
            interface IComboBox 
            {
                S1 property1;
            }
            enum E1 {}  
        }
    )" };
    std::string assembly_name = "testidl";
    xlang::meta::reader::database db{ run_and_save_to_memory(test_idl, assembly_name) };

    REQUIRE(db.TypeRef.size() == TYPE_REF_OFFSET + 3);

    auto const& E1_ref = db.TypeRef[TYPE_REF_OFFSET + 2];
    auto const& ICombo_ref = db.TypeRef[TYPE_REF_OFFSET + 1];
    auto const& S1_ref = db.TypeRef[TYPE_REF_OFFSET + 0];
    REQUIRE(E1_ref.TypeName() == "E1");
    REQUIRE(ICombo_ref.TypeName() == "IComboBox");
    REQUIRE(S1_ref.TypeName() == "S1");

    REQUIRE(db.TypeDef.size() == TYPE_DEF_OFFSET + 3);
    REQUIRE(db.TypeDef[TYPE_DEF_OFFSET + 1].TypeName() == "IComboBox");
    auto const& combo = db.TypeDef[TYPE_DEF_OFFSET + 1];
    test_interface_type_properties(combo);

    REQUIRE(size(combo.PropertyList()) == 1);
    auto const& property_list = combo.PropertyList();

    auto const& property1 = property_list.first[0];
    REQUIRE(property1.Name() == "property1");
    REQUIRE(property1.Flags().value == method_attributes_no_flags().value);
    
    REQUIRE(db.MethodSemantics.size() == 2);
    for (size_t i = 0; i < db.MethodSemantics.size(); i++)
    {
        auto const& assoc = db.MethodSemantics[i].Association().type();
        REQUIRE(assoc == HasSemantics::Property);
        auto const& property_sig = db.MethodSemantics[i].Association().Property().Type();
        REQUIRE(std::get<coded_index<TypeDefOrRef>>(property_sig.Type().Type()).TypeRef() == S1_ref);
        REQUIRE(db.MethodSemantics[i].Association().Property().Name() == "property1");
    }
    auto const& get_method = db.MethodSemantics[0].Method();
    REQUIRE(get_method.Name() == "get_property1");
    {
        auto const& sig = get_method.Signature();
        REQUIRE(std::get<coded_index<TypeDefOrRef>>(sig.ReturnType().Type().Type()).TypeRef() == S1_ref);
        REQUIRE(size(sig.Params()) == 0);
    }
    auto const& set_method = db.MethodSemantics[1].Method();
    REQUIRE(set_method.Name() == "set_property1");
    {
        auto const& sig = set_method.Signature();
        REQUIRE(!sig.ReturnType());
        REQUIRE(size(sig.Params()) == 1);
        auto const& param_sig = sig.Params().first[0];
        REQUIRE(std::get<coded_index<TypeDefOrRef>>(param_sig.Type().Type()).TypeRef() == S1_ref);
    }
}

TEST_CASE("Interface type metadata")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            delegate void StringListEvent(Int32 sender);

            interface IControl
            {
                void Paint();
            }
    
            interface IComboBox requires IControl {
                S1 Draw(S1 param1);
                event StringListEvent Changed;
                S1 property1;
            }

            struct S1
            {
            }
        }
    )" };
    std::string assembly_name = "testidl";
    xlang::meta::reader::database db{ run_and_save_to_memory(test_idl, assembly_name) };

    REQUIRE(db.TypeRef.size() == TYPE_REF_OFFSET + 4);
    auto const& S1_ref = db.TypeRef[TYPE_REF_OFFSET + 0];
    auto const& IControl_ref = db.TypeRef[TYPE_REF_OFFSET + 1];
    auto const& ICombo_ref = db.TypeRef[TYPE_REF_OFFSET + 2];
    auto const& StringListEvent_ref = db.TypeRef[TYPE_REF_OFFSET + 3];
    REQUIRE(S1_ref.TypeName() == "S1");
    REQUIRE(IControl_ref.TypeName() == "IControl");
    REQUIRE(ICombo_ref.TypeName() == "IComboBox");
    REQUIRE(StringListEvent_ref.TypeName() == "StringListEvent");

    REQUIRE(db.TypeDef.size() == TYPE_DEF_OFFSET + 4);
    REQUIRE(db.TypeDef[TYPE_DEF_OFFSET + 1].TypeName() == "IComboBox");
    REQUIRE(db.TypeDef[TYPE_DEF_OFFSET + 2].TypeName() == "IControl");
    auto const& combo = db.TypeDef[TYPE_DEF_OFFSET + 1];
    auto const& control = db.TypeDef[TYPE_DEF_OFFSET + 2];
    test_interface_type_properties(combo);
    test_interface_type_properties(control);

    REQUIRE(size(combo.MethodList()) == 5);
    REQUIRE(size(combo.PropertyList()) == 1);
    REQUIRE(size(combo.EventList()) == 1);
    REQUIRE(size(combo.InterfaceImpl()) == 1);
    auto const& method_list = combo.MethodList();

    REQUIRE(method_list.first[0].Name() == "Draw");
    REQUIRE(method_list.first[0].Flags().value == interface_method_attributes().value);
    {
        auto const& Draw_sig = method_list.first[0].Signature();
        REQUIRE(std::get<coded_index<TypeDefOrRef>>(Draw_sig.ReturnType().Type().Type()).TypeRef() == S1_ref);
        auto const& Draw_param_sig = Draw_sig.Params().first[0];
        REQUIRE(std::get<coded_index<TypeDefOrRef>>(Draw_param_sig.Type().Type()).TypeRef() == S1_ref);
    }
}