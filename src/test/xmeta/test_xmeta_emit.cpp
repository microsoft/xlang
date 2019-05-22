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

const MethodAttributes interface_method_event_attributes()
{
    MethodAttributes result{};
    result.Access(MemberAccess::Public);
    result.Final(true);
    result.Virtual(true);
    result.HideBySig(true);
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

const EventAttributes event_attributes_no_flags()
{
    EventAttributes result{};
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
    std::vector<std::string> paths = { "Foundation.xmeta" };
    xmeta_idl_reader reader{ assembly_name, paths };
    reader.read(test_idl);
    REQUIRE(reader.get_num_syntax_errors() == 0);
    REQUIRE(reader.get_num_semantic_errors() == 0);
    return reader.save_to_memory();
}

template<typename T>
auto find_type_by_name(table<T> const& type, std::string name, std::string namespace_name)
{
    auto const& ref = std::find_if(type.begin(), type.end(), [name, namespace_name](auto&& t)
    {
        return t.TypeName() == name && t.TypeNamespace() == namespace_name;
    });
    REQUIRE(ref.TypeName() == name);
    REQUIRE(ref.TypeNamespace() == namespace_name);
    return ref;
}

TEST_CASE("Assemblies metadata")
{
    constexpr char assembly_name[] = "test_idl";
    constexpr char common_assembly_ref[] = "mscorlib";

    std::istringstream test_idl{ R"(
    )" };
    xlang::meta::reader::database db{ run_and_save_to_memory(test_idl, assembly_name) };

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

    auto const& enum_type = find_type_by_name<TypeDef>(db.TypeDef, "Color", "Windows.Test");
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

    REQUIRE(db.TypeDef.size() == TYPE_DEF_OFFSET + 1);
    REQUIRE(db.Param.size() == 5); // return type + two formal parameters + two from delegate constructor parameter
    REQUIRE(db.MethodDef.size() == 2); // One constructor and one invoke method
    auto const& delegate_type = find_type_by_name<TypeDef>(db.TypeDef, "testdelegate", "Windows.Test");
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
    auto const& e1_ref = find_type_by_name<TypeRef>(db.TypeRef, "e1", "Windows.Test");
    find_type_by_name<TypeRef>(db.TypeRef, "d1", "A");

    auto const& e1 = find_type_by_name<TypeDef>(db.TypeDef, "e1", "Windows.Test");
    auto const& d1 = find_type_by_name<TypeDef>(db.TypeDef, "d1", "A");
    test_enum_type_properties(e1);
    test_delegate_type_properties(d1);

    // Check that type Ref is cottrect
    {
        auto const& delegate_invoke = d1.MethodList().first[1];
        REQUIRE(delegate_invoke.Parent().TypeName() == "d1");
        auto const& delegate_sig = delegate_invoke.Signature();
        REQUIRE(std::get<coded_index<TypeDefOrRef>>(delegate_sig.ReturnType().Type().Type()).TypeRef() == e1_ref);
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
    auto const& e1_ref = find_type_by_name<TypeRef>(db.TypeRef, "e1", "Windows.Test");
    find_type_by_name<TypeRef>(db.TypeRef, "d1", "Windows.Test");

    auto const e1 = find_type_by_name<TypeDef>(db.TypeDef, "e1", "Windows.Test");
    auto const d1 = find_type_by_name<TypeDef>(db.TypeDef, "d1", "Windows.Test");
    test_enum_type_properties(e1);
    test_delegate_type_properties(d1);
    // Check that type Ref is cottrect
    {
        auto const& delegate_invoke = d1.MethodList().first[1];
        REQUIRE(delegate_invoke.Parent().TypeName() == "d1");
        auto const& delegate_sig = delegate_invoke.Signature();
        REQUIRE(std::get<coded_index<TypeDefOrRef>>(delegate_sig.ReturnType().Type().Type()).TypeRef() == e1_ref);
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

    auto const& struct0 = find_type_by_name<TypeDef>(db.TypeDef, "S", "N");
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
    
    find_type_by_name<TypeRef>(db.TypeRef, "S0", "N");
    auto const& s1 = find_type_by_name<TypeRef>(db.TypeRef, "S1", "N");
    auto const& e0 = find_type_by_name<TypeRef>(db.TypeRef, "E0", "N");

    auto const& struct0 = find_type_by_name<TypeDef>(db.TypeDef, "S0", "N");
    test_struct_type_properties(struct0);

    REQUIRE(db.Field.size() == 3);
    {
        auto const& struct_field = struct0.FieldList().first[0];
        auto const& struct_field_sig = struct_field.Signature();
        REQUIRE(struct_field.Flags().value == struct_field_attributes().value);
        REQUIRE(std::get<coded_index<TypeDefOrRef>>(struct_field_sig.Type().Type()).TypeRef() == s1);
    }
    {
        auto const& struct_field = struct0.FieldList().first[1];
        auto const& struct_field_sig = struct_field.Signature();
        REQUIRE(struct_field.Flags().value == struct_field_attributes().value);
        REQUIRE(std::get<coded_index<TypeDefOrRef>>(struct_field_sig.Type().Type()).TypeRef() == e0);
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
    auto const& E1_ref = find_type_by_name<TypeRef>(db.TypeRef, "E1", "N");
    auto const& ICombo_ref = find_type_by_name<TypeRef>(db.TypeRef, "IComboBox", "N");
    auto const& S1_ref = find_type_by_name<TypeRef>(db.TypeRef, "S1", "N");
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
    find_type_by_name<TypeRef>(db.TypeRef, "IComboBox", "N");
    find_type_by_name<TypeRef>(db.TypeRef, "E1", "N");
    auto const& S1_ref = find_type_by_name<TypeRef>(db.TypeRef, "S1", "N");

    REQUIRE(db.TypeDef.size() == TYPE_DEF_OFFSET + 3);
    auto const& combo_def = find_type_by_name<TypeDef>(db.TypeDef, "IComboBox", "N");
    test_interface_type_properties(combo_def);

    REQUIRE(size(combo_def.PropertyList()) == 1);
    auto const& property_list = combo_def.PropertyList();

    auto const& property1 = property_list.first[0];
    REQUIRE(property1.Name() == "property1");
    REQUIRE(property1.Flags().value == method_attributes_no_flags().value);
    
    REQUIRE(db.MethodSemantics.size() == 2);
    for (auto const& sem : property1.MethodSemantic())
    {
        REQUIRE(sem.Association().Property() == property1);
        auto const& assoc = sem.Association().type();
        REQUIRE(assoc == HasSemantics::Property);
        auto const& property_sig = sem.Association().Property().Type();
        REQUIRE(std::get<coded_index<TypeDefOrRef>>(property_sig.Type().Type()).TypeRef() == S1_ref);
        REQUIRE(sem.Association().Property().Name() == property1.Name());
    }
    auto const& get_method = property1.MethodSemantic().first[0].Method();
    REQUIRE(get_method.Name() == "get_property1");
    {
        auto const& sig = get_method.Signature();
        REQUIRE(std::get<coded_index<TypeDefOrRef>>(sig.ReturnType().Type().Type()).TypeRef() == S1_ref);
        REQUIRE(size(sig.Params()) == 0);
        REQUIRE(get_method.Flags().value == interface_method_property_attributes().value);
    }
    auto const& set_method = property1.MethodSemantic().first[1].Method();
    REQUIRE(set_method.Name() == "put_property1");
    {
        auto const& sig = set_method.Signature();
        REQUIRE(!sig.ReturnType());
        REQUIRE(size(sig.Params()) == 1);
        auto const& param_sig = sig.Params().first[0];
        REQUIRE(std::get<coded_index<TypeDefOrRef>>(param_sig.Type().Type()).TypeRef() == S1_ref);
        REQUIRE(set_method.Flags().value == interface_method_property_attributes().value);
    }
}

TEST_CASE("Interface event metadata")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            delegate void StringListEvent(Int32 sender);
            interface IComboBox 
            {
                event StringListEvent Changed;
            }
        }
    )" };
    std::string assembly_name = "testidl";
    xlang::meta::reader::database db{ run_and_save_to_memory(test_idl, assembly_name) };
    
    REQUIRE(db.AssemblyRef.size() == 2);
    auto const& iter = std::find_if(db.AssemblyRef.begin(), db.AssemblyRef.end(), [](auto&& assembly)
    {
        return assembly.Name() == "xlang_foundation";
    });
    REQUIRE(iter.Name() == "xlang_foundation");

    REQUIRE(db.TypeRef.size() == TYPE_REF_OFFSET + 3);
    find_type_by_name<TypeRef>(db.TypeRef, "IComboBox", "N");
    auto const& event_registration_token_ref = find_type_by_name<TypeRef>(db.TypeRef, "EventRegistrationToken", "Foundation");
    auto const& string_list_event_ref = find_type_by_name<TypeRef>(db.TypeRef, "StringListEvent", "N");

    REQUIRE(db.TypeDef.size() == TYPE_DEF_OFFSET + 2);
    auto const& combo = find_type_by_name<TypeDef>(db.TypeDef, "IComboBox", "N");
    test_interface_type_properties(combo);

    REQUIRE(size(combo.EventList()) == 1);
    auto const& event_list = combo.EventList();


    auto const& event1 = event_list.first[0];
    REQUIRE(event1.Name() == "Changed");
    REQUIRE(event1.EventFlags().value == event_attributes_no_flags().value);

    REQUIRE(db.MethodSemantics.size() == 2);
    for (auto const& sem : event1.MethodSemantic())
    {
        REQUIRE(sem.Association().Event() == event1);
        auto const& assoc = sem.Association().type();
        REQUIRE(assoc == HasSemantics::Event);
        auto const& event_sig = sem.Association().Event().EventType();
        REQUIRE(event_sig.TypeRef().TypeName() == string_list_event_ref.TypeName());
        REQUIRE(sem.Association().Event().Name() == event1.Name());
    }
    auto const& add_method = event1.MethodSemantic().first[0].Method();
    REQUIRE(add_method.Name() == "add_Changed");
    {
        auto const& sig = add_method.Signature();
        REQUIRE(std::get<coded_index<TypeDefOrRef>>(sig.ReturnType().Type().Type()).TypeRef() == event_registration_token_ref);
        REQUIRE(size(sig.Params()) == 1);
        auto const& param_sig = sig.Params().first[0];
        REQUIRE(std::get<coded_index<TypeDefOrRef>>(param_sig.Type().Type()).TypeRef() == string_list_event_ref);
        REQUIRE(add_method.Flags().value == interface_method_event_attributes().value);
    }
    auto const& remove_method = event1.MethodSemantic().first[1].Method();
    REQUIRE(remove_method.Name() == "remove_Changed");
    {
        auto const& sig = remove_method.Signature();
        REQUIRE(!sig.ReturnType());
        REQUIRE(size(sig.Params()) == 1);
        auto const& param_sig = sig.Params().first[0];
        REQUIRE(std::get<coded_index<TypeDefOrRef>>(param_sig.Type().Type()).TypeRef() == event_registration_token_ref);
        REQUIRE(remove_method.Flags().value == interface_method_event_attributes().value);
    }
}

TEST_CASE("Interface type metadata")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            interface IControl requires M.IControl3
            {
                void Paint();
            }
    
            interface IComboBox requires IControl, IControl2
            {
                Int32 property1;
            }

            interface IControl2 requires M.IControl3
            {
                void Paint2();
            }
        }
        namespace M
        {
            interface IControl3
            {
                void Paint3();
            }
        }

    )" };
    std::string assembly_name = "testidl";
    xlang::meta::reader::database db{ run_and_save_to_memory(test_idl, assembly_name) };

    REQUIRE(db.TypeRef.size() == TYPE_REF_OFFSET + 4);
    find_type_by_name<TypeRef>(db.TypeRef, "IComboBox", "N");
    auto const& control3_ref = find_type_by_name<TypeRef>(db.TypeRef, "IControl3", "M");
    auto const& control2_ref = find_type_by_name<TypeRef>(db.TypeRef, "IControl2", "N");
    auto const& control_ref = find_type_by_name<TypeRef>(db.TypeRef, "IControl", "N");

    REQUIRE(db.TypeDef.size() == TYPE_DEF_OFFSET + 4);
    auto const& combo = find_type_by_name<TypeDef>(db.TypeDef, "IComboBox", "N");
    auto const& c1 = find_type_by_name<TypeDef>(db.TypeDef, "IControl", "N");
    auto const& c2 = find_type_by_name<TypeDef>(db.TypeDef, "IControl2", "N");

    test_interface_type_properties(combo);

    REQUIRE(db.InterfaceImpl.size() == 5);
    auto const& impls =  combo.InterfaceImpl();
    REQUIRE(size(impls) == 3);
    {
        auto const& iter = std::find_if(impls.first, impls.second, [control3_ref](auto&& type_ref)
        {
            return type_ref.Interface().TypeRef() == control3_ref;
        });
        REQUIRE(iter.Interface().TypeRef() == control3_ref);
    }
    {
        auto const& iter = std::find_if(impls.first, impls.second, [control2_ref](auto&& type_ref)
        {
            return type_ref.Interface().TypeRef() == control2_ref;
        });
        REQUIRE(iter.Interface().TypeRef() == control2_ref);
    }
    {
        auto const& iter = std::find_if(impls.first, impls.second, [control_ref](auto&& type_ref)
        {
            return type_ref.Interface().TypeRef() == control_ref;
        });
        REQUIRE(iter.Interface().TypeRef() == control_ref);
    }

    auto const& c1_impls = c1.InterfaceImpl();
    {
        REQUIRE(size(c1_impls) == 1);
        auto const& iter = std::find_if(c1_impls.first, c1_impls.second, [control3_ref](auto&& type_ref)
        {
            return type_ref.Interface().TypeRef() == control3_ref;
        });
        REQUIRE(iter.Interface().TypeRef() == control3_ref);
    }

    auto const& c2_impls = c2.InterfaceImpl();
    {
        REQUIRE(size(c2_impls) == 1);
        auto const& iter = std::find_if(c2_impls.first, c2_impls.second, [control3_ref](auto&& type_ref)
        {
            return type_ref.Interface().TypeRef() == control3_ref;
        });
        REQUIRE(iter.Interface().TypeRef() == control3_ref);
    }
}