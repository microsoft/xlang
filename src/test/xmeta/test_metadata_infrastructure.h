
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string_view>

#include "antlr4-runtime.h"
#include "xmeta_idl_reader.h"
#include "ast_to_st_listener.h"

using namespace antlr4;

#pragma execution_character_set("utf-8")

using namespace xlang::xmeta;
using namespace xlang::meta::reader;

// All the flags use in the metadata representation
const TypeAttributes class_type_attributes()
{
    TypeAttributes result{};
    result.Visibility(TypeVisibility::Public);
    result.Semantics(TypeSemantics::Class);
    result.Sealed(true);
    result.Layout(TypeLayout::AutoLayout);
    result.WindowsRuntime(true);
    return result;
}

const TypeAttributes static_class_type_attributes()
{
    TypeAttributes result{};
    result.Visibility(TypeVisibility::Public);
    result.Semantics(TypeSemantics::Class);
    result.Sealed(true);
    result.Layout(TypeLayout::AutoLayout);
    result.WindowsRuntime(true);
    result.Abstract(true);
    return result;
}
//mdPublic | mdHideBySig | mdSpecialName | mdRTSpecialName
const MethodAttributes constructor_method_attributes()
{
    MethodAttributes result{};
    result.Access(MemberAccess::Public);
    result.HideBySig(true);
    result.SpecialName(true);
    result.RTSpecialName(true);
    return result;
}

const MethodAttributes class_method_attributes()
{
    MethodAttributes result{};
    result.Access(MemberAccess::Public);
    result.Virtual(true);
    result.HideBySig(true);
    result.Layout(VtableLayout::NewSlot);
    return result;
}

const MethodAttributes class_method_property_attributes()
{
    MethodAttributes result{};
    result.Access(MemberAccess::Public);
    result.Virtual(true);
    result.HideBySig(true);
    result.Layout(VtableLayout::NewSlot);
    result.SpecialName(true);
    return result;
}

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

const MethodImplAttributes method_impl_runtime_attributes()
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
    REQUIRE(delegate_constructor.ImplFlags().value == method_impl_runtime_attributes().value);
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
    ExpectedType() = delete;

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
    MethodAttributes flag;
    MethodImplAttributes implFlag{};

    ExpectedMethod(std::string const& expectedName, ElementType const& expectedReturnType, std::vector<ExpectedParam> expectedParams, MethodAttributes const& flag)
        : ExpectedType(expectedName), returnType(expectedReturnType), returnTypeRef(""), params(expectedParams), flag(flag) {}
    ExpectedMethod(std::string const& expectedName, std::string const& expectedReturnType, std::vector<ExpectedParam> expectedParams, MethodAttributes const& flag)
        : ExpectedType(expectedName), returnType(ElementType::End), returnTypeRef(expectedReturnType), params(expectedParams), flag(flag) {}

    ExpectedMethod(std::string const& expectedName, ElementType const& expectedReturnType, std::vector<ExpectedParam> expectedParams, MethodAttributes const& flag, MethodImplAttributes const&  implFlag)
        : ExpectedType(expectedName), returnType(expectedReturnType), returnTypeRef(""), params(expectedParams), flag(flag), implFlag{ implFlag } {}
    ExpectedMethod(std::string const& expectedName, std::string const& expectedReturnType, std::vector<ExpectedParam> expectedParams, MethodAttributes const& flag, MethodImplAttributes const&  implFlag)
        : ExpectedType(expectedName), returnType(ElementType::End), returnTypeRef(expectedReturnType), params(expectedParams), flag(flag), implFlag{ implFlag } {}

    void VerifyType(MethodDef const& method) const override
    {
        REQUIRE(method.Name() == name);
        // TODO: the flags are wonky right now too. Might be related to methodimpltable. Reenable once the fix is found
        // REQUIRE(method.Flags().value == flag.value);
        VerifyReturnType(returnType, returnTypeRef, method, method.Parent());

        REQUIRE(method.ImplFlags().value == implFlag.value);


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
                    VerifyReturnType(type, typeRef, method, prop.Parent());
                }
                else if (setMethodName == method.Name())
                {
                    foundSetMethod = true;
                    REQUIRE(!method.Signature().ReturnType());
                    REQUIRE(size(method.Signature().Params()) == 1);
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
                expectedMethod->VerifyType(method);
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

struct ExpectedClass : ExpectedType
{
    std::vector<std::pair<ExpectedMethod, std::string>> methods_and_implements;

    std::vector<ExpectedProperty> properties;
    std::vector<ExpectedEvent> events;
    std::vector<std::string> requires;
    std::string extends = "System.Object";
    TypeAttributes class_flag = class_type_attributes();

    ExpectedClass(std::string const& expectedName,
        std::vector<std::string> expectedRequires,
        std::vector<ExpectedProperty> expectedProperties,
        std::vector<ExpectedEvent> expectedEvents,
        std::vector<std::pair<ExpectedMethod, std::string>> expected_methods_and_implements)
        : ExpectedType(expectedName), requires(expectedRequires), properties(expectedProperties), events(expectedEvents), methods_and_implements(expected_methods_and_implements) {}

    ExpectedClass(std::string const& expectedName,
        std::vector<std::string> expectedRequires,
        std::vector<ExpectedProperty> expectedProperties,
        std::vector<ExpectedEvent> expectedEvents,
        std::vector<std::pair<ExpectedMethod, std::string>> expected_methods_and_implements,
        std::string const& extends)
        : ExpectedType(expectedName), requires(expectedRequires), properties(expectedProperties), events(expectedEvents), methods_and_implements(expected_methods_and_implements), extends(extends) {}

    ExpectedClass(std::string const& expectedName,
        std::vector<std::string> expectedRequires,
        std::vector<ExpectedProperty> expectedProperties,
        std::vector<ExpectedEvent> expectedEvents,
        std::vector<std::pair<ExpectedMethod, std::string>> expected_methods_and_implements,
        TypeAttributes const&  flag)
        : ExpectedType(expectedName), requires(expectedRequires), properties(expectedProperties), events(expectedEvents), methods_and_implements(expected_methods_and_implements), class_flag(flag) {}

    void VerifyType(TypeDef const& typeDef) const override
    {
        REQUIRE(typeDef.TypeName() == name);

        std::string actual_extend_name = std::string(typeDef.Extends().TypeRef().TypeNamespace()) + "." + std::string(typeDef.Extends().TypeRef().TypeName());
        REQUIRE(actual_extend_name == extends);
        REQUIRE(typeDef.Flags().value == class_flag.value);
        REQUIRE(empty(typeDef.FieldList()));

        REQUIRE(methods_and_implements.size() == size(typeDef.MethodList()));

        if (!methods_and_implements.empty())
        {
            {
                auto expectedMethod(methods_and_implements.begin());
                for (auto &&method : typeDef.MethodList())
                {
                    ExpectedMethod m = expectedMethod->first;
                    m.VerifyType(method);
                    ++expectedMethod;
                }
            }
            {
                auto expectedMethod(methods_and_implements.begin());
                for (auto &&method_impl : typeDef.MethodImplList())
                {
                    std::string expected_impl = expectedMethod->second;
                    if (expected_impl != "")
                    {
                        std::string actual = std::string(method_impl.MethodDeclaration().MemberRef().Class().TypeRef().TypeName())
                            + "." + std::string(method_impl.MethodDeclaration().MemberRef().Name());
                        REQUIRE(actual == expected_impl);
                        ++expectedMethod;
                    }
                }
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