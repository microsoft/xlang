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
constexpr int TYPE_REF_OFFSET = 4; // System: Enum, Delegate, ValueType

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

void VerifyTypeRefName(std::string const& expectedName, std::string_view currentNamespace, TypeRef const& typeRef)
{
    std::string combineName("");
    if (currentNamespace != typeRef.TypeNamespace())
    {
        combineName += typeRef.TypeNamespace();
        combineName += ".";
    }
    combineName += typeRef.TypeName();
    REQUIRE(combineName == expectedName);
}

void VerifyReturnType(ElementType const& expectedType, std::string const& expectedTypeRef, MethodDef const& method, TypeDef const& parent)
{
    auto const& methodSig = method.Signature();
    REQUIRE(method.ParamList().first[0].Name() == "returnVal");
    switch (expectedType)
    {
    case ElementType::Void:
        REQUIRE(!methodSig.ReturnType());
        break;
    case ElementType::End:
    {
        VerifyTypeRefName(expectedTypeRef, parent.TypeNamespace(), std::get<coded_index<TypeDefOrRef>>(methodSig.ReturnType().Type().Type()).TypeRef());
        break;
    }
    default:
        REQUIRE(std::get<ElementType>(methodSig.ReturnType().Type().Type()) == expectedType);
        break;
    }
}

struct ExpectedType
{
    std::string name;

    ExpectedType() {}
    ExpectedType(std::string const& typeName) : name(typeName) {}

    virtual void VerifyType(TypeDef const& /*typeDef*/) const {}
    virtual void VerifyType(MethodDef const& /*field*/) const {}
    virtual void VerifyType(Field const& /*field*/) const {}
    virtual void VerifyType(Property const& /*property*/) const {}
    virtual void VerifyType(Event const& /*event*/) const {}
    virtual void VerifyType(Param const& /*param*/, ParamSig const& /*paramSig*/, std::string_view const& /*parentNamespace*/) const {}
};

struct ExpectedStructField : ExpectedType
{
    ElementType type;
    std::string typeRef;

    ExpectedStructField(std::string const& expectedName, ElementType const& expectedType)
        : ExpectedType(expectedName), type(expectedType), typeRef("") {}
    ExpectedStructField(std::string const& expectedName, std::string const& expectedType)
        : ExpectedType(expectedName), type(ElementType::End), typeRef(expectedType) {}

    void VerifyType(Field const& field) const override
    {
        REQUIRE(field.Name() == name);
        REQUIRE(field.Flags().value == struct_field_attributes().value);

        if (type == ElementType::End)
        {
            VerifyTypeRefName(typeRef, field.Parent().TypeNamespace(), std::get<coded_index<TypeDefOrRef>>(field.Signature().Type().Type()).TypeRef());
        }
        else
        {
            REQUIRE(field.Signature().Type().element_type() == type);
        }
    }
};

struct ExpectedStruct : ExpectedType
{
    std::vector<ExpectedStructField> fields;

    ExpectedStruct(std::string const& expectedName, std::vector<ExpectedStructField> expectedFields) : ExpectedType(expectedName), fields(expectedFields) {}

    void VerifyType(TypeDef const& typeDef) const override
    {
        REQUIRE(typeDef.TypeName() == name);

        test_struct_type_properties(typeDef);

        auto structField(fields.begin());
        for (auto &&field : typeDef.FieldList())
        {
            structField->VerifyType(field);
            ++structField;
        }
    }
};

struct ExpectedEnumField : ExpectedType
{
    int32_t value;

    ExpectedEnumField(std::string const& expectedName, int32_t const& expectedValue) : ExpectedType(expectedName), value(expectedValue) {}

    void VerifyType(Field const& field) const override
    {
        REQUIRE(field.Name() == name);
        REQUIRE(field.Constant().ValueInt32() == value);
        REQUIRE(field.Flags().value == enum_fields_attributes().value);
    }
};

struct ExpectedEnum : ExpectedType
{
    std::vector<ExpectedEnumField> fields;

    ExpectedEnum(std::string const& expectedName, std::vector<ExpectedEnumField> expectedFields) : ExpectedType(expectedName), fields(expectedFields) {}

    void VerifyType(TypeDef const& typeDef) const override
    {
        REQUIRE(typeDef.TypeName() == name);

        test_enum_type_properties(typeDef);

        auto enumField(fields.begin());
        for (size_t i = 1; i < size(typeDef.FieldList()); i++)
        {
            enumField->VerifyType(typeDef.FieldList().first[i]);
            ++enumField;
        }
    }
};

struct ExpectedParam : ExpectedType
{
    ElementType type;
    std::string typeRef;

    ExpectedParam(std::string const& expectedName, ElementType const& expectedType)
        : ExpectedType(expectedName), type(expectedType), typeRef("") {}
    ExpectedParam(std::string const& expectedName, std::string const& expectedType)
        : ExpectedType(expectedName), type(ElementType::End), typeRef(expectedType) {}

    void VerifyType(Param const& param, ParamSig const& paramSig, std::string_view const& parentNamespace) const override
    {
        REQUIRE(param.Name() == name);
        if (type == ElementType::End)
        {
            VerifyTypeRefName(typeRef, parentNamespace, std::get<coded_index<TypeDefOrRef>>(paramSig.Type().Type()).TypeRef());
        }
        else
        {
            REQUIRE(std::get<ElementType>(paramSig.Type().Type()) == type);
        }
    }
};

struct ExpectedDelegate : ExpectedType
{
    ElementType returnType;
    std::string returnTypeRef;
    std::vector<ExpectedParam> params;

    ExpectedDelegate(std::string const& expectedName, ElementType const& expectedReturnType, std::vector<ExpectedParam> expectedParams)
        : ExpectedType(expectedName), returnType(expectedReturnType), returnTypeRef(""), params(expectedParams) {}
    ExpectedDelegate(std::string const& expectedName, std::string const& expectedReturnType, std::vector<ExpectedParam> expectedParams)
        : ExpectedType(expectedName), returnType(ElementType::End), returnTypeRef(expectedReturnType), params(expectedParams) {}

    void VerifyType(TypeDef const& typeDef) const override
    {
        REQUIRE(typeDef.TypeName() == name);

        test_delegate_type_properties(typeDef);

        auto const& invokeMethod = typeDef.MethodList().first[1];
        VerifyReturnType(returnType, returnTypeRef, invokeMethod, typeDef);

        auto param(params.begin());
        for (size_t i = 1; i < size(invokeMethod.ParamList()); i++)
        {
            param->VerifyType(invokeMethod.ParamList().first[i], invokeMethod.Signature().Params().first[i - 1], typeDef.TypeNamespace());
            ++param;
        }
    }
};

struct ExpectedMethod : ExpectedType
{
    ElementType returnType;
    std::string returnTypeRef;
    std::vector<ExpectedParam> params;

    ExpectedMethod(std::string const& expectedName, ElementType const& expectedReturnType, std::vector<ExpectedParam> expectedParams)
        : ExpectedType(expectedName), returnType(expectedReturnType), returnTypeRef(""), params(expectedParams) {}
    ExpectedMethod(std::string const& expectedName, std::string const& expectedReturnType, std::vector<ExpectedParam> expectedParams)
        : ExpectedType(expectedName), returnType(ElementType::End), returnTypeRef(expectedReturnType), params(expectedParams) {}

    void VerifyType(MethodDef const& method) const override
    {
        REQUIRE(method.Name() == name);

        VerifyReturnType(returnType, returnTypeRef, method, method.Parent());

        auto param(params.begin());
        for (size_t i = 1; i < size(method.ParamList()); i++)
        {
            param->VerifyType(method.ParamList().first[i], method.Signature().Params().first[i - 1], method.Parent().TypeNamespace());
            ++param;
        }
    }
};

struct ExpectedProperty : ExpectedType
{
    ElementType type;
    std::string typeRef;

    ExpectedProperty(std::string const& expectedName, ElementType const& expectedType)
        : ExpectedType(expectedName), type(expectedType), typeRef("") {}
    ExpectedProperty(std::string const& expectedName, std::string const& expectedType)
        : ExpectedType(expectedName), type(ElementType::End), typeRef(expectedType) {}

    void VerifyType(Property const& prop) const override
    {
        REQUIRE(prop.Name() == name);
        REQUIRE(prop.Flags().value == method_attributes_no_flags().value);

        if (type == ElementType::End)
        {
            VerifyTypeRefName(typeRef, prop.Parent().TypeNamespace(), std::get<coded_index<TypeDefOrRef>>(prop.Type().Type().Type()).TypeRef());
        }
        else
        {
            REQUIRE(prop.Type().Type().element_type() == type);
        }

        bool foundGetMethod = false;
        bool foundSetMethod = false;
        std::string getMethodName = "get_";
        std::string setMethodName = "put_";
        getMethodName += prop.Name();
        setMethodName += prop.Name();
        for (auto &&method : prop.Parent().MethodList())
        {
            if (method.Flags().SpecialName() == true)
            {
                if (getMethodName == method.Name())
                {
                    foundGetMethod = true;
                    REQUIRE(size(method.Signature().Params()) == 0);
                    REQUIRE(method.Flags().value == interface_method_property_attributes().value);
                    VerifyReturnType(type, typeRef, method, prop.Parent());
                }
                else if (setMethodName == method.Name())
                {
                    foundSetMethod = true;
                    REQUIRE(!method.Signature().ReturnType());
                    REQUIRE(size(method.Signature().Params()) == 1);
                    REQUIRE(method.Flags().value == interface_method_property_attributes().value);
                    if (type == ElementType::End)
                    {
                        VerifyTypeRefName(typeRef, method.Parent().TypeNamespace(), std::get<coded_index<TypeDefOrRef>>(method.Signature().Params().first[0].Type().Type()).TypeRef());
                    }
                    else
                    {
                        REQUIRE(method.Signature().Params().first[0].Type().element_type() == type);
                    }
                }
            }
        }
        REQUIRE(foundGetMethod == true);
        REQUIRE(foundSetMethod == true);
    }
};

struct ExpectedEvent : ExpectedType
{
    std::string typeRef;

    ExpectedEvent(std::string const& expectedName, std::string const& expectedType)
        : ExpectedType(expectedName), typeRef(expectedType) {}

    void VerifyType(Event const& event) const override
    {
        REQUIRE(event.Name() == name);
        REQUIRE(event.EventFlags().value == event_attributes_no_flags().value);

        VerifyTypeRefName(typeRef, event.Parent().TypeNamespace(), event.EventType().TypeRef());

        bool foundAddMethod = false;
        bool foundRemoveMethod = false;
        std::string addMethodName = "add_";
        std::string removeMethodName = "remove_";
        addMethodName += event.Name();
        removeMethodName += event.Name();
        for (auto &&method : event.Parent().MethodList())
        {
            if (method.Flags().SpecialName() == true)
            {
                if (addMethodName == method.Name())
                {
                    foundAddMethod = true;
                    REQUIRE(size(method.Signature().Params()) == 1);
                    std::string expectedTypeName(std::get<coded_index<TypeDefOrRef>>(method.Signature().Params().first[0].Type().Type()).TypeRef().TypeName());
                    VerifyTypeRefName(expectedTypeName, event.Parent().TypeNamespace(), event.EventType().TypeRef());
                    REQUIRE(method.Flags().value == interface_method_event_attributes().value);
                    VerifyReturnType(ElementType::End, "Foundation.EventRegistrationToken", method, event.Parent());
                }
                else if (removeMethodName == method.Name())
                {
                    foundRemoveMethod = true;
                    REQUIRE(size(method.Signature().Params()) == 1);
                    REQUIRE(!method.Signature().ReturnType());
                    REQUIRE(method.Flags().value == interface_method_event_attributes().value);
                    VerifyTypeRefName("Foundation.EventRegistrationToken", event.Parent().TypeNamespace(), std::get<coded_index<TypeDefOrRef>>(method.Signature().Params().first[0].Type().Type()).TypeRef());
                }
            }
        }
        REQUIRE(foundAddMethod == true);
        REQUIRE(foundRemoveMethod == true);
    }
};

struct ExpectedInterface : ExpectedType
{
    std::vector<ExpectedMethod> methods;
    std::vector<ExpectedProperty> properties;
    std::vector<ExpectedEvent> events;
    std::vector<std::string> requires;

    ExpectedInterface(std::string const& expectedName, std::vector<std::string> expectedRequires, std::vector<ExpectedProperty> expectedProperties, std::vector<ExpectedEvent> expectedEvents, std::vector<ExpectedMethod> expectedMethods)
        : ExpectedType(expectedName), requires(expectedRequires), properties(expectedProperties), events(expectedEvents), methods(expectedMethods) {}
    ExpectedInterface(std::string const& expectedName, std::vector<std::string> expectedRequires, std::vector<ExpectedProperty> expectedProperties, std::vector<ExpectedEvent> expectedEvents)
        : ExpectedType(expectedName), requires(expectedRequires), properties(expectedProperties), events(expectedEvents), methods({}) {}
    ExpectedInterface(std::string const& expectedName, std::vector<std::string> expectedRequires, std::vector<ExpectedProperty> expectedProperties, std::vector<ExpectedMethod> expectedMethods)
        : ExpectedType(expectedName), requires(expectedRequires), properties(expectedProperties), events({}), methods(expectedMethods) {}
    ExpectedInterface(std::string const& expectedName, std::vector<std::string> expectedRequires, std::vector<ExpectedEvent> expectedEvents, std::vector<ExpectedMethod> expectedMethods)
        : ExpectedType(expectedName), requires(expectedRequires), properties({}), events(expectedEvents), methods(expectedMethods) {}
    ExpectedInterface(std::string const& expectedName, std::vector<std::string> expectedRequires, std::vector<ExpectedProperty> expectedProperties)
        : ExpectedType(expectedName), requires(expectedRequires), properties(expectedProperties), events({}), methods({}) {}
    ExpectedInterface(std::string const& expectedName, std::vector<std::string> expectedRequires, std::vector<ExpectedEvent> expectedEvents)
        : ExpectedType(expectedName), requires(expectedRequires), properties({}), events(expectedEvents), methods({}) {}
    ExpectedInterface(std::string const& expectedName, std::vector<std::string> expectedRequires, std::vector<ExpectedMethod> expectedMethods)
        : ExpectedType(expectedName), requires(expectedRequires), properties({}), events({}), methods(expectedMethods) {}

    ExpectedInterface(std::string const& expectedName, std::vector<ExpectedProperty> expectedProperties, std::vector<ExpectedEvent> expectedEvents, std::vector<ExpectedMethod> expectedMethods)
        : ExpectedType(expectedName), requires({}), properties(expectedProperties), events(expectedEvents), methods(expectedMethods) {}
    ExpectedInterface(std::string const& expectedName, std::vector<ExpectedProperty> expectedProperties, std::vector<ExpectedEvent> expectedEvents)
        : ExpectedType(expectedName), requires({}), properties(expectedProperties), events(expectedEvents), methods({}) {}
    ExpectedInterface(std::string const& expectedName, std::vector<ExpectedProperty> expectedProperties, std::vector<ExpectedMethod> expectedMethods)
        : ExpectedType(expectedName), requires({}), properties(expectedProperties), events({}), methods(expectedMethods) {}
    ExpectedInterface(std::string const& expectedName, std::vector<ExpectedEvent> expectedEvents, std::vector<ExpectedMethod> expectedMethods)
        : ExpectedType(expectedName), requires({}), properties({}), events(expectedEvents), methods(expectedMethods) {}
    ExpectedInterface(std::string const& expectedName, std::vector<ExpectedProperty> expectedProperties)
        : ExpectedType(expectedName), requires({}), properties(expectedProperties), events({}), methods({}) {}
    ExpectedInterface(std::string const& expectedName, std::vector<ExpectedEvent> expectedEvents)
        : ExpectedType(expectedName), requires({}), properties({}), events(expectedEvents), methods({}) {}
    ExpectedInterface(std::string const& expectedName, std::vector<ExpectedMethod> expectedMethods)
        : ExpectedType(expectedName), requires({}), properties({}), events({}), methods(expectedMethods) {}

    void VerifyType(TypeDef const& typeDef) const override
    {
        REQUIRE(typeDef.TypeName() == name);

        test_interface_type_properties(typeDef);

        if (!methods.empty())
        {
            auto expectedMethod(methods.begin());
            for (auto &&method : typeDef.MethodList())
            {
                if (method.Flags().SpecialName() == false)
                {
                    expectedMethod->VerifyType(method);
                }
                ++expectedMethod;
            }
        }

        if (!properties.empty())
        {
            auto expectedProperty(properties.begin());
            for (auto &&prop : typeDef.PropertyList())
            {
                expectedProperty->VerifyType(prop);
                ++expectedProperty;
            }
        }

        if (!events.empty())
        {
            auto expectedEvent(events.begin());
            for (auto &&event : typeDef.EventList())
            {
                expectedEvent->VerifyType(event);
                ++expectedEvent;
            }
        }

        if (!requires.empty())
        {
            auto expectedRequire(requires.begin());
            for (auto &&require : typeDef.InterfaceImpl())
            {
                VerifyTypeRefName(*expectedRequire, typeDef.TypeNamespace(), require.Interface().TypeRef());
                ++expectedRequire;
            }
        }

        return;
    }
};

void ValidateTypesInMetadata(std::istringstream & testIdl, std::vector<std::shared_ptr<ExpectedType>> const& fileTypes)
{
    xlang::meta::reader::database db{ run_and_save_to_memory(testIdl, "testidl") };

    auto expectedType(fileTypes.begin());
    // Start at 1 to skip over <Module> element
    for (size_t i = 1; i < db.TypeDef.size(); i++)
    {
        (*expectedType)->VerifyType(db.TypeDef[i]);
        ++expectedType;
    }
    REQUIRE(expectedType == fileTypes.end());
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

    std::vector<std::shared_ptr<ExpectedType>> fileTypes =
    {
        std::make_unique<ExpectedEnum>("Color",
            std::vector<ExpectedEnumField> {
                ExpectedEnumField("Red", 0),
                ExpectedEnumField("Green", 1),
                ExpectedEnumField("Blue", 2)
            }
        )
    };

    ValidateTypesInMetadata(test_idl, fileTypes);
}

TEST_CASE("Delegate metadata")
{
    std::istringstream test_idl{ R"(
        namespace Windows.Test
        {
            delegate Int16 testdelegate(Int32 c, out Int64 d);
        }
    )" };

    std::vector<std::shared_ptr<ExpectedType>> fileTypes =
    {
        std::make_unique<ExpectedDelegate>("testdelegate", ElementType::I2,
            std::vector<ExpectedParam> {
                ExpectedParam("c", ElementType::I4),
                ExpectedParam("d", ElementType::I8)
            }
        )
    };

    ValidateTypesInMetadata(test_idl, fileTypes);
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

    std::vector<std::shared_ptr<ExpectedType>> fileTypes =
    {
        std::make_unique<ExpectedDelegate>("d1", ElementType::String, std::vector<ExpectedParam> {}),
        std::make_unique<ExpectedDelegate>("d2", ElementType::I1, std::vector<ExpectedParam> {}),
        std::make_unique<ExpectedDelegate>("d3", ElementType::I2, std::vector<ExpectedParam> {}),
        std::make_unique<ExpectedDelegate>("d4", ElementType::I4, std::vector<ExpectedParam> {}),
        std::make_unique<ExpectedDelegate>("d5", ElementType::I8, std::vector<ExpectedParam> {}),
        std::make_unique<ExpectedDelegate>("d6", ElementType::U1, std::vector<ExpectedParam> {}),
        std::make_unique<ExpectedDelegate>("d7", ElementType::U2, std::vector<ExpectedParam> {}),
        std::make_unique<ExpectedDelegate>("d8", ElementType::U4, std::vector<ExpectedParam> {}),
        std::make_unique<ExpectedDelegate>("d9", ElementType::U8, std::vector<ExpectedParam> {}),
        std::make_unique<ExpectedDelegate>("e1", ElementType::R4, std::vector<ExpectedParam> {}),
        std::make_unique<ExpectedDelegate>("e2", ElementType::R8, std::vector<ExpectedParam> {}),
        std::make_unique<ExpectedDelegate>("e3", ElementType::Char, std::vector<ExpectedParam> {}),
        std::make_unique<ExpectedDelegate>("e4", ElementType::Boolean, std::vector<ExpectedParam> {}),
        std::make_unique<ExpectedDelegate>("e5", ElementType::Void, std::vector<ExpectedParam> {})
    };

    ValidateTypesInMetadata(test_idl, fileTypes);
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

    std::vector<std::shared_ptr<ExpectedType>> fileTypes =
    {
        std::make_unique<ExpectedDelegate>("d1", "Windows.Test.e1", std::vector<ExpectedParam> {}),
        std::make_unique<ExpectedEnum>("e1", std::vector<ExpectedEnumField> {})
    };

    ValidateTypesInMetadata(test_idl, fileTypes);
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

    std::vector<std::shared_ptr<ExpectedType>> fileTypes =
    {
        std::make_unique<ExpectedEnum>("e1", std::vector<ExpectedEnumField> {}),
        std::make_unique<ExpectedDelegate>("d1", "e1", std::vector<ExpectedParam> {})
    };

    ValidateTypesInMetadata(test_idl, fileTypes);
}

TEST_CASE("Struct simple type metadata")
{
    std::istringstream test_idl{ R"(
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

    std::vector<std::shared_ptr<ExpectedType>> fileTypes =
    {
        std::make_unique<ExpectedStruct>("S",
            std::vector<ExpectedStructField> {
                ExpectedStructField("field_1", ElementType::Boolean),
                ExpectedStructField("field_2", ElementType::String),
                ExpectedStructField("field_3", ElementType::I2),
                ExpectedStructField("field_4", ElementType::I4),
                ExpectedStructField("field_5", ElementType::I8),
                ExpectedStructField("field_6", ElementType::U1),
                ExpectedStructField("field_7", ElementType::U2),
                ExpectedStructField("field_8", ElementType::U4),
                ExpectedStructField("field_9", ElementType::Char),
                ExpectedStructField("field_10", ElementType::R4),
                ExpectedStructField("field_11", ElementType::R8)
            }
        )
    };

    ValidateTypesInMetadata(test_idl, fileTypes);
}

TEST_CASE("Struct class type metadata")
{
    std::istringstream test_idl{ R"(
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

    std::vector<std::shared_ptr<ExpectedType>> fileTypes =
    {
        std::make_unique<ExpectedStruct>("S0",
            std::vector<ExpectedStructField> {
                ExpectedStructField("field_1", "S1"),
                ExpectedStructField("field_2", "E0")
            }
        ),
        std::make_unique<ExpectedStruct>("S1", std::vector<ExpectedStructField> {}),
        std::make_unique<ExpectedEnum>("E0", std::vector<ExpectedEnumField>{})
    };

    ValidateTypesInMetadata(test_idl, fileTypes);
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

    std::vector<std::shared_ptr<ExpectedType>> fileTypes =
    {
        std::make_unique<ExpectedStruct>("S1", std::vector<ExpectedStructField> {}),
        std::make_unique<ExpectedInterface>("IComboBox",
            std::vector<ExpectedMethod> {
                ExpectedMethod("Draw", "S1",
                    std::vector<ExpectedParam> {
                        ExpectedParam("param1", "E1")
                    }
                )
            }
        ),
        std::make_unique<ExpectedEnum>("E1", std::vector<ExpectedEnumField>{})
    };

    ValidateTypesInMetadata(test_idl, fileTypes);
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

    std::vector<std::shared_ptr<ExpectedType>> fileTypes =
    {
        std::make_unique<ExpectedStruct>("S1", std::vector<ExpectedStructField> {}),
        std::make_unique<ExpectedInterface>("IComboBox",
            std::vector<ExpectedProperty> {
                ExpectedProperty("property1", "S1")
            }
        ),
        std::make_unique<ExpectedEnum>("E1", std::vector<ExpectedEnumField>{})
    };

    ValidateTypesInMetadata(test_idl, fileTypes);
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

    std::vector<std::shared_ptr<ExpectedType>> fileTypes =
    {
        std::make_unique<ExpectedInterface>("IComboBox",
            std::vector<ExpectedEvent> {
                ExpectedEvent("Changed", "StringListEvent")
            }
        ),
        std::make_unique<ExpectedDelegate>("StringListEvent", ElementType::Void,
            std::vector<ExpectedParam> {
                ExpectedParam("sender", ElementType::I4)
            }
        )
    };

    ValidateTypesInMetadata(test_idl, fileTypes);
}

TEST_CASE("Interface type metadata 2", "[!hide]")
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

    std::vector<std::shared_ptr<ExpectedType>> fileTypes =
    {
        std::make_unique<ExpectedInterface>("IControl3",
            std::vector<ExpectedMethod> {
                ExpectedMethod("Paint3", ElementType::Void, {})
            }
        ),
        std::make_unique<ExpectedInterface>("IComboBox", std::vector<std::string> {"M.IControl3", "IControl", "IControl2"},
            std::vector<ExpectedProperty> {
                ExpectedProperty("property1", ElementType::I4)
            }
        ),
        std::make_unique<ExpectedInterface>("IControl", std::vector<std::string> {"M.IControl3"},
            std::vector<ExpectedMethod> {
                ExpectedMethod("Paint", ElementType::Void, {})
            }
        ),
        std::make_unique<ExpectedInterface>("IControl2", std::vector<std::string> {"M.IControl3"},
            std::vector<ExpectedMethod> {
                ExpectedMethod("Paint2", ElementType::Void, {})
            }
        )
    };

    ValidateTypesInMetadata(test_idl, fileTypes);
}

TEST_CASE("Interface type metadata", "[!hide]")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            interface IControl requires M.IControl3
            {
                void Paint();
            }
    
            interface IComboBox requires IControl
            {
                Int32 property1;
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

    std::vector<std::shared_ptr<ExpectedType>> fileTypes =
    {
        std::make_unique<ExpectedInterface>("IControl3",
            std::vector<ExpectedMethod> {
                ExpectedMethod("Paint3", ElementType::Void, {})
            }
        ),
        std::make_unique<ExpectedInterface>("IComboBox", std::vector<std::string> {"M.IControl3", "IControl"},
            std::vector<ExpectedProperty> {
                ExpectedProperty("property1", ElementType::I4)
            }
        ),
        std::make_unique<ExpectedInterface>("IControl", std::vector<std::string> {"M.IControl3"},
            std::vector<ExpectedMethod> {
                ExpectedMethod("Paint", ElementType::Void, {})
            }
        )
    };

    ValidateTypesInMetadata(test_idl, fileTypes);
}

TEST_CASE("Put breakpoint here to see test output before closing")
{
    return;
}