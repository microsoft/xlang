#include "pch.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string_view>

#include "antlr4-runtime.h"
#include "xmeta_idl_reader.h"
#include "ast_to_st_listener.h"

using namespace antlr4;
using namespace xlang::xmeta;

constexpr method_modifier default_method_modifier = { false, false, false };
constexpr method_modifier static_method_modifier = { false, true, false };
constexpr property_modifier default_property_modifier = { false, false };
constexpr property_modifier static_property_modifier = { false, true };
constexpr event_modifier default_event_modifier = { false, false };
constexpr event_modifier static_event_modifier = { false, true };

auto find_namespace(xmeta_idl_reader & reader, std::string name)
{
    auto namespaces = reader.get_namespaces();
    auto it = namespaces.find(name);
    REQUIRE(it != namespaces.end());
    return it->second;
}

auto find_namespace_body(xmeta_idl_reader & reader, std::string name, int index)
{
    auto const& namespaces = reader.get_namespaces();
    auto const& it = namespaces.find(name);
    REQUIRE(it != namespaces.end());
    auto const& ns_bodies = it->second->get_namespace_bodies();
    return ns_bodies[index];
}

struct ExpectedClassRef
{
    std::string qualified_name;
    ExpectedClassRef(std::string_view id) : qualified_name{ id } {}
};

struct ExpectedInterfaceRef
{
    std::string qualified_name;
    ExpectedInterfaceRef(std::string_view id) : qualified_name{ id } {}
};

struct ExpectedEnumRef
{
    std::string qualified_name;
    ExpectedEnumRef(std::string_view id) : qualified_name{ id } {}
};

struct ExpectedDelegateRef
{
    std::string qualified_name;
    ExpectedDelegateRef(std::string_view id) : qualified_name{ id } {}
};

struct ExpectedStructRef
{
    std::string qualified_name;
    ExpectedStructRef(std::string_view id) : qualified_name{ id } {}
};

struct ExpectedTypeRefModel
{
   std::variant<ExpectedClassRef,
        ExpectedInterfaceRef,
        ExpectedEnumRef,
        ExpectedDelegateRef,
        ExpectedStructRef,
        fundamental_type,
        object_type> type;

   ExpectedTypeRefModel(ExpectedClassRef const& type)
       : type{ type } {}

   ExpectedTypeRefModel(ExpectedInterfaceRef const& type)
       : type{ type } {}

    ExpectedTypeRefModel(ExpectedDelegateRef const& type)
        : type{ type } {}

    ExpectedTypeRefModel(ExpectedEnumRef const& type)
        : type{ type } {}

    ExpectedTypeRefModel(ExpectedStructRef const& type)
        : type{ type } {}

    ExpectedTypeRefModel(object_type const& type)
        : type{ type } {}

    ExpectedTypeRefModel(fundamental_type const& type)
        : type{ type } {}

    void VerifyType(type_ref const& actual)
    {
        REQUIRE(actual.get_semantic().is_resolved());
        auto const& target = actual.get_semantic().get_resolved_target();
        if (std::holds_alternative<std::shared_ptr<xlang::meta::reader::TypeDef>>(target))
        {
            auto const& typedef_model = std::get<std::shared_ptr<xlang::meta::reader::TypeDef>>(target);
            std::string actual_fully_qualified_id = std::string(typedef_model->TypeNamespace()) + "." + std::string(typedef_model->TypeName());
            //TODO: interface and runtimeclass checks
            if (typedef_model->is_delegate())
            {
                REQUIRE(std::holds_alternative<ExpectedDelegateRef>(type));
                auto const& model = std::get<ExpectedDelegateRef>(type);
                REQUIRE(actual_fully_qualified_id == model.qualified_name);
            }
            if (typedef_model->is_enum())
            {
                REQUIRE(std::holds_alternative<ExpectedEnumRef>(type));
                auto const& model = std::get<ExpectedEnumRef>(type);
                REQUIRE(actual_fully_qualified_id == model.qualified_name);
            }
            if (typedef_model->is_struct())
            {
                REQUIRE(std::holds_alternative<ExpectedStructRef>(type));
                auto const& model = std::get<ExpectedStructRef>(type);
                REQUIRE(actual_fully_qualified_id == model.qualified_name);
            }
        }
        if (std::holds_alternative<std::shared_ptr<class_model>>(target))
        {
            REQUIRE(std::holds_alternative<ExpectedClassRef>(type));
            auto const& model = std::get<ExpectedClassRef>(type);
            auto const& actual_model = std::get<std::shared_ptr<class_model>>(target);
            REQUIRE(actual_model->get_qualified_name() == model.qualified_name);
        }
        if (std::holds_alternative<std::shared_ptr<interface_model>>(target))
        {
            REQUIRE(std::holds_alternative<ExpectedInterfaceRef>(type));
            auto const& model = std::get<ExpectedInterfaceRef>(type);
            auto const& actual_model = std::get<std::shared_ptr<interface_model>>(target);
            REQUIRE(actual_model->get_qualified_name() == model.qualified_name);
        }
        if (std::holds_alternative<std::shared_ptr<delegate_model>>(target))
        {
            REQUIRE(std::holds_alternative<ExpectedDelegateRef>(type));
            auto const& model = std::get<ExpectedDelegateRef>(type);
            auto const& actual_model = std::get<std::shared_ptr<delegate_model>>(target);
            REQUIRE(actual_model->get_qualified_name() == model.qualified_name);
        }
        if (std::holds_alternative<std::shared_ptr<struct_model>>(target))
        {
            REQUIRE(std::holds_alternative<ExpectedStructRef>(type));
            auto const& model = std::get<ExpectedStructRef>(type);
            auto const& actual_model = std::get<std::shared_ptr<struct_model>>(target);
            REQUIRE(actual_model->get_qualified_name() == model.qualified_name);
        }
        if (std::holds_alternative<std::shared_ptr<enum_model>>(target))
        {
            REQUIRE(std::holds_alternative<ExpectedEnumRef>(type));
            auto const& model = std::get<ExpectedEnumRef>(type);
            auto const& actual_model = std::get<std::shared_ptr<enum_model>>(target);
            REQUIRE(actual_model->get_qualified_name() == model.qualified_name);
        }
        if (std::holds_alternative<fundamental_type>(target))
        {
            REQUIRE(std::holds_alternative<fundamental_type>(type));
            auto const& model = std::get<fundamental_type>(type);
            auto const& actual_model = std::get<fundamental_type>(target);
            REQUIRE(actual_model == model);
        }
        if (std::holds_alternative<object_type>(target))
        {
            REQUIRE(std::holds_alternative<object_type>(type));
        }
    }
};

struct ExpectedFormalParameterModel
{
    std::string id;
    parameter_semantics sem;
    ExpectedTypeRefModel type;
    ExpectedFormalParameterModel(std::string const& name, parameter_semantics const& sem, ExpectedTypeRefModel const& type)
        : id{ name }, sem{ sem }, type{ type } {}

    void VerifyType(std::shared_ptr<formal_parameter_model> const& actual)
    {
        REQUIRE(actual->get_name() == id);
        REQUIRE(actual->get_semantic() == sem);
        REQUIRE(actual->get_type().get_semantic().is_resolved());
        type.VerifyType(actual->get_type());
    }

    void VerifyType(formal_parameter_model const& actual)
    {
        REQUIRE(actual.get_name() == id);
        REQUIRE(actual.get_semantic() == sem);
        REQUIRE(actual.get_type().get_semantic().is_resolved());
        type.VerifyType(actual.get_type());
    }
};

struct ExpectedMethodModel
{
    std::string id;
    method_modifier sem;
    std::optional<ExpectedTypeRefModel> return_type;
    std::vector<ExpectedFormalParameterModel> params;

    ExpectedMethodModel() {};

    ExpectedMethodModel(std::string const& name, method_modifier const& sem, std::optional<ExpectedTypeRefModel> const& return_type, std::vector<ExpectedFormalParameterModel> params)
        : id{ name }, sem{ sem }, return_type{ return_type }, params{ params } {}

    void VerifyType(std::shared_ptr<method_model> const& actual)
    {
        REQUIRE(actual->get_name() == id);
        REQUIRE(actual->get_modifier().is_static == sem.is_static);
        REQUIRE(actual->get_return_type()->get_semantic().is_resolved());

        if (return_type == std::nullopt)
        {
            REQUIRE(actual->get_return_type() == std::nullopt);
        }
        else
        {
            REQUIRE(actual->get_return_type()->get_semantic().is_resolved());
            return_type->VerifyType(*actual->get_return_type());
        }

        auto const& actual_params = actual->get_formal_parameters();
        REQUIRE(actual_params.size() == params.size());
        for (size_t i = 0; i < params.size(); i++)
        {
            params[i].VerifyType(actual_params[i]);
        }
    }
};

struct ExpectedPropertyModel
{
    std::string id;
    property_modifier sem;
    ExpectedTypeRefModel type;

    ExpectedMethodModel get_method;
    std::optional<ExpectedMethodModel> set_method;

    ExpectedPropertyModel(std::string const& name, 
            property_modifier const& sem, 
            ExpectedTypeRefModel const& type, 
            ExpectedMethodModel const& get_method,
            std::optional<ExpectedMethodModel> const& set_method)
        : id{ name }, sem{ sem }, type{ type }, get_method{ get_method }, set_method{ set_method } {}

    void VerifyType(std::shared_ptr<property_model> const& actual)
    {
        REQUIRE(actual->get_name() == id);
        REQUIRE(actual->get_modifier().is_static == sem.is_static);
        REQUIRE(actual->get_type().get_semantic().is_resolved());
        type.VerifyType(actual->get_type());

        REQUIRE(actual->get_get_method());
        get_method.VerifyType(actual->get_get_method());

        if (actual->get_set_method())
        {
            set_method->VerifyType(actual->get_set_method());
        }
        else
        {
            REQUIRE(set_method == std::nullopt);
        }
    }
};

struct ExpectedEventModel
{
    std::string id;
    event_modifier sem;
    ExpectedTypeRefModel type;

    ExpectedMethodModel add_method;
    ExpectedMethodModel remove_method;

    ExpectedEventModel(std::string const& name,
        event_modifier const& sem,
        ExpectedTypeRefModel const& type)
        : id{ name }, sem{ sem }, type{ type }
    {
        method_modifier method_sem;
        if (sem.is_static)
        {
            method_sem.is_static = true;
        }
        add_method = { "add_" + id, method_sem, ExpectedTypeRefModel{ ExpectedStructRef{ "Foundation.EventRegistrationToken" } }, {
            ExpectedFormalParameterModel{ "TODO:findname", parameter_semantics::in, type } } };

        remove_method = { "remove_" + id, method_sem, std::nullopt, {
            ExpectedFormalParameterModel{ "TODO:findname", parameter_semantics::in, ExpectedTypeRefModel{ ExpectedStructRef{ "Foundation.EventRegistrationToken" } } } } };
    }

    void VerifyType(std::shared_ptr<event_model> const& actual)
    {
        REQUIRE(actual->get_name() == id);
        REQUIRE(actual->get_modifier().is_static == sem.is_static);
        REQUIRE(actual->get_type().get_semantic().is_resolved());
        type.VerifyType(actual->get_type());

        REQUIRE(actual->get_add_method());
        add_method.VerifyType(actual->get_add_method());
        REQUIRE(actual->get_remove_method());
        remove_method.VerifyType(actual->get_remove_method());
    }

    auto const& get_add_method()
    {
        return add_method;
    }

    auto const& get_remove_method()
    {
        return remove_method;
    }
};

struct ExpectedClassModel
{
    std::string id;
    std::string qualified_name;
    std::vector<ExpectedMethodModel> methods;
    std::vector<ExpectedPropertyModel> properties;
    std::vector<ExpectedEventModel> events;
    std::optional<ExpectedTypeRefModel> class_base;
    std::vector<ExpectedTypeRefModel> interface_bases;

    ExpectedClassModel(std::string const& id,
        std::string const& qualified_name,
        std::vector<ExpectedMethodModel> const& methods,
        std::vector<ExpectedPropertyModel> const& properties,
        std::vector<ExpectedEventModel> const& events,
        std::optional<ExpectedTypeRefModel> class_base,
        std::vector<ExpectedTypeRefModel> const& interfaces_bases)
        : id{ id }, qualified_name{ qualified_name }, methods{ methods }, properties{ properties }, events{ events }, class_base{ class_base }, interface_bases{ interfaces_bases } {}

    void VerifyType(std::shared_ptr<class_model> const& actual)
    {
        REQUIRE(actual->get_name() == id);
        REQUIRE(actual->get_qualified_name() == qualified_name);
        auto const& actual_methods = actual->get_methods();
        REQUIRE(actual_methods.size() == methods.size());
        for (size_t i = 0; i < methods.size(); i++)
        {
            methods[i].VerifyType(actual_methods[i]);
        }

        auto const& actual_properties = actual->get_properties();
        REQUIRE(actual_properties.size() == properties.size());
        for (size_t i = 0; i < properties.size(); i++)
        {
            properties[i].VerifyType(actual_properties[i]);
        }

        auto const& actual_events = actual->get_events();
        REQUIRE(actual_events.size() == events.size());
        for (size_t i = 0; i < events.size(); i++)
        {
            events[i].VerifyType(actual_events[i]);
        }

        if (class_base == std::nullopt)
        {
            REQUIRE(actual->get_class_base_ref() == std::nullopt);
        }
        else
        {
            class_base->VerifyType(*actual->get_class_base_ref());
        }
        
        auto const& actual_interface_bases = actual->get_interface_bases();
        REQUIRE(actual_interface_bases.size() == interface_bases.size());
        for (size_t i = 0; i < interface_bases.size(); i++)
        {
            interface_bases[i].VerifyType(actual_interface_bases[i]);
        }
    }
};

struct ExpectedInterfaceModel
{
    std::string id;
    std::string qualified_name;
    std::vector<ExpectedMethodModel> methods;
    std::vector<ExpectedPropertyModel> properties;
    std::vector<ExpectedEventModel> events;
    std::vector<ExpectedTypeRefModel> interface_bases;

    ExpectedInterfaceModel(std::string const& id, 
            std::string const& qualified_name, 
            std::vector<ExpectedMethodModel> const& methods,
            std::vector<ExpectedPropertyModel> const& properties,
            std::vector<ExpectedEventModel> const& events,
            std::vector<ExpectedTypeRefModel> const& bases)
        : id{ id }, qualified_name{ qualified_name }, methods{ methods }, properties{ properties }, events{ events }, interface_bases{ bases } {}

    void VerifyType(std::shared_ptr<interface_model> const& actual)
    {
        REQUIRE(actual->get_name() == id);
        REQUIRE(actual->get_qualified_name() == qualified_name);
        auto const& actual_methods = actual->get_methods();
        REQUIRE(actual_methods.size() == methods.size());
        for (size_t i = 0; i < methods.size(); i++)
        {
            methods[i].VerifyType(actual_methods[i]);
        }

        auto const& actual_properties = actual->get_properties();
        REQUIRE(actual_properties.size() == properties.size());
        for (size_t i = 0; i < properties.size(); i++)
        {
            properties[i].VerifyType(actual_properties[i]);
        }

        auto const& actual_events = actual->get_events();
        REQUIRE(actual_events.size() == events.size());
        for (size_t i = 0; i < events.size(); i++)
        {
            events[i].VerifyType(actual_events[i]);
        }

        auto const& actual_interface_bases = actual->get_interface_bases();
        REQUIRE(actual_interface_bases.size() == interface_bases.size());
        for (size_t i = 0; i < interface_bases.size(); i++)
        {
            interface_bases[i].VerifyType(actual_interface_bases[i]);
        }
    }
};

struct ExpectedDelegateModel
{
    std::string id;
    std::string qualified_name;
    std::optional<ExpectedTypeRefModel> return_type;
    std::vector<ExpectedFormalParameterModel> params;

    ExpectedDelegateModel(std::string const& id, std::string const& qualified_name, std::optional<ExpectedTypeRefModel> const& return_type, std::vector<ExpectedFormalParameterModel> params)
        : id{ id }, qualified_name{ qualified_name }, return_type{ return_type }, params{ params } {}

    void VerifyType(std::shared_ptr<delegate_model> const& actual)
    {
        REQUIRE(actual->get_name() == id);
        REQUIRE(actual->get_qualified_name() == qualified_name);

        if (return_type == std::nullopt)
        {
            REQUIRE(actual->get_return_type() == std::nullopt);
        }
        else
        {
            REQUIRE(actual->get_return_type()->get_semantic().is_resolved());
            return_type->VerifyType(*actual->get_return_type());
        }

        auto const& actual_params = actual->get_formal_parameters();
        REQUIRE(actual_params.size() == params.size());
        for (size_t i = 0; i < params.size(); i++)
        {
            params[i].VerifyType(actual_params[i]);
        }
    }
};

struct ExpectedEnumModel
{
    std::string id;
    std::string qualified_name;
    std::vector<enum_member> fields;
    enum_type sem;

    ExpectedEnumModel(std::string const& id, std::string const& qualified_name, enum_type sem, std::vector<enum_member> fields)
        : id{ id }, qualified_name{ qualified_name }, fields{ fields }, sem{ sem } {}

    void VerifyType(std::shared_ptr<enum_model> const& actual)
    {
        REQUIRE(actual->get_name() == id);
        REQUIRE(actual->get_qualified_name() == qualified_name);
        REQUIRE(actual->get_type() == sem);
        auto & actual_members = actual->get_members();
        REQUIRE(actual_members.size() == fields.size());
        for (size_t i = 0; i < fields.size(); i++)
        {
            REQUIRE(actual_members.at(i) == fields.at(i));
        }
    }
};

struct ExpectedStructModel
{
    std::string id;
    std::string qualified_name;
    std::vector<std::pair<ExpectedTypeRefModel, std::string>> fields;

    ExpectedStructModel(std::string const& id, std::string const& qualified_name, std::vector<std::pair<ExpectedTypeRefModel, std::string>> fields)
        : id{ id }, qualified_name { qualified_name }, fields{ fields } {}

    void VerifyType(std::shared_ptr<struct_model> const& actual)
    {
        REQUIRE(actual->get_name() == id);
        REQUIRE(actual->get_qualified_name() == qualified_name);

        auto const& actual_fields = actual->get_fields();
        REQUIRE(actual_fields.size() == fields.size());
        for (size_t i = 0; i < fields.size(); i++)
        {
            REQUIRE(actual_fields.at(i).first.get_semantic().is_resolved());
            fields.at(i).first.VerifyType(actual_fields.at(i).first);
            REQUIRE(actual_fields.at(i).second == fields.at(i).second);
        }
    }
};

struct ExpectedNamespaceModel
{
    std::string id;
    std::string qualified_name;
    std::vector<ExpectedNamespaceModel> children;

    std::vector<ExpectedClassModel> classes;
    std::vector<ExpectedInterfaceModel> interfaces;
    std::vector<ExpectedStructModel> structs;
    std::vector<ExpectedEnumModel> enums;
    std::vector<ExpectedDelegateModel> delegates;

    ExpectedNamespaceModel(std::string const& id, 
            std::string const& qualified_name, 
            std::vector<ExpectedNamespaceModel> namespaces, 
            std::vector<std::variant<ExpectedEnumModel, ExpectedStructModel, ExpectedDelegateModel, ExpectedInterfaceModel, ExpectedClassModel>> declarations)
        : id{ id }, qualified_name{ qualified_name }, children { namespaces } 
    {
        for (auto & declaration : declarations)
        {
            if (std::holds_alternative<ExpectedClassModel>(declaration))
            {
                classes.push_back(std::get<ExpectedClassModel>(declaration));
            }
            if (std::holds_alternative<ExpectedStructModel>(declaration))
            {
                structs.push_back(std::get<ExpectedStructModel>(declaration));
            }
            if (std::holds_alternative<ExpectedEnumModel>(declaration))
            {
                enums.push_back(std::get<ExpectedEnumModel>(declaration));
            }
            if (std::holds_alternative<ExpectedDelegateModel>(declaration))
            {
                delegates.push_back(std::get<ExpectedDelegateModel>(declaration));
            }
            if (std::holds_alternative<ExpectedInterfaceModel>(declaration))
            {
                interfaces.push_back(std::get<ExpectedInterfaceModel>(declaration));
            }
        }
    }

    void VerifyType(std::shared_ptr<namespace_model> const& actual)
    {
        REQUIRE(actual->get_name() == id);
        REQUIRE(actual->get_qualified_name() == qualified_name);

        for (auto const& actual_bodies : actual->get_namespace_bodies())
        {
            auto const& actual_class = actual_bodies->get_classes();
            for (auto expected_class : classes)
            {
                auto const& it = actual_class.find(expected_class.id);
                REQUIRE(it != actual_class.end());
                expected_class.VerifyType(it->second);
            }

            auto const& actual_interface = actual_bodies->get_interfaces();
            for (auto expected_interface : interfaces)
            {
                auto const& it = actual_interface.find(expected_interface.id);
                REQUIRE(it != actual_interface.end());
                expected_interface.VerifyType(it->second);
            }
            auto const& actual_structs = actual_bodies->get_structs();
            for (auto expected_struct : structs)
            {
                auto const& it = actual_structs.find(expected_struct.id);
                REQUIRE(it != actual_structs.end());
                expected_struct.VerifyType(it->second);
            }

            auto const& actual_delegates = actual_bodies->get_delegates();
            for (auto expected_delegate : delegates)
            {
                auto const& it = actual_delegates.find(expected_delegate.id);
                REQUIRE(it != actual_delegates.end());
                expected_delegate.VerifyType(it->second);
            }

            auto const& actual_enums = actual_bodies->get_enums();
            for (auto epected_enum : enums)
            {
                auto const& it = actual_enums.find(epected_enum.id);
                REQUIRE(it != actual_enums.end());
                epected_enum.VerifyType(it->second);
            }
        }

        auto const& actual_child_namespaces = actual->get_child_namespaces();
        for (auto & child : children)
        {
            auto const& it = actual_child_namespaces.find(child.id);
            REQUIRE(it != actual_child_namespaces.end());
            child.VerifyType(it->second);
        }
    }
};


TEST_CASE("Duplicate Namespaces")
{
    std::istringstream test_idl(R"(
        namespace N { }
        namespace N { }
    )");

    xmeta_idl_reader reader{ "" };
    reader.read(test_idl);
    REQUIRE(reader.get_num_syntax_errors() == 0);
    auto ns = find_namespace(reader, "N");
    REQUIRE(ns->get_namespace_bodies().size() == 2);
    REQUIRE(ns->get_namespace_bodies()[0]->get_containing_namespace() == ns);
    REQUIRE(ns->get_namespace_bodies()[1]->get_containing_namespace() == ns);
}

TEST_CASE("Multiple definition error test")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            enum E {}
            enum E {}

            delegate void D();
            delegate void D();

            struct S {}
            struct S {}
        }
    )" };

    xmeta_idl_reader reader{ "" };
    reader.read(test_idl);
    REQUIRE(reader.get_num_syntax_errors() == 0);
    REQUIRE(reader.get_num_semantic_errors() == 3);
}

TEST_CASE("Enum test")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            enum E
            {
                e_member_1,
                e_member_2 = 3,
                e_member_3,
                e_member_4 = e_member_5,
                e_member_5 = 0x21
            }
        }
    )" };

    xmeta_idl_reader reader{ "" };
    reader.read(test_idl);
    REQUIRE(reader.get_num_syntax_errors() == 0);
    
    ExpectedEnumModel expected_enum{ "E", "N.E" , enum_type::Int32, { 
        enum_member{ "e_member_1", 0 },
        enum_member{ "e_member_2", 3 },
        enum_member{ "e_member_3", 4 },
        enum_member{ "e_member_4", 0x21 },
        enum_member{ "e_member_5", 0x21 }
    } };;
    ExpectedNamespaceModel expected_namespace{ "N", "N", {}, { expected_enum } };
    expected_namespace.VerifyType(find_namespace(reader, "N"));
}

TEST_CASE("Enum circular implicit dependency")
{
    std::istringstream implicit_dependency_error_idl{ R"(
        namespace N
        {
            enum E
            {
                e_member_1 = e_member_3,
                e_member_2,
                e_member_3
            }
        }
    )" };

    xmeta_idl_reader reader{ "" };
    reader.read(implicit_dependency_error_idl);
    REQUIRE(reader.get_num_syntax_errors() == 0);
    REQUIRE(reader.get_num_semantic_errors() == 1);
}

TEST_CASE("Enum circular explicit dependency")
{
    std::istringstream explicit_dependency_error_idl{ R"(
        namespace N
        {
            enum E
            {
                e_member_1 = e_member_2,
                e_member_2 = e_member_1
            }
        }
    )" };

    xmeta_idl_reader reader{ "" };
    reader.read(explicit_dependency_error_idl);
    REQUIRE(reader.get_num_syntax_errors() == 0);
    REQUIRE(reader.get_num_semantic_errors() == 1);
}

TEST_CASE("Delegate test")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            enum E { }
            enum F{}
            delegate Int32 D1(Int32 i, Double d, E e);
            delegate void D2();
        }
    )" };

    xmeta_idl_reader reader{ "" };
    reader.read(test_idl);
    REQUIRE(reader.get_num_syntax_errors() == 0);

    ExpectedEnumModel E{ "E", "N.E" , enum_type::Int32, {} };
    ExpectedEnumModel F{ "F", "N.F" , enum_type::Int32, {} };
    ExpectedDelegateModel D1{ "D1", "N.D1", ExpectedTypeRefModel{ fundamental_type::Int32 }, {
        ExpectedFormalParameterModel{ "i", parameter_semantics::in, ExpectedTypeRefModel{ fundamental_type::Int32 } },
        ExpectedFormalParameterModel{ "d", parameter_semantics::in, ExpectedTypeRefModel{ fundamental_type::Double } },
        ExpectedFormalParameterModel{ "e", parameter_semantics::in, ExpectedTypeRefModel{ ExpectedEnumRef { E.qualified_name } } },
    } };
    ExpectedDelegateModel D2{ "D2", "N.D2", std::nullopt, {} };

    ExpectedNamespaceModel expected{ "N", "N", {}, { E, F, D1, D2 } };
    expected.VerifyType(find_namespace(reader, "N"));
}

TEST_CASE("Struct test")
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

    xmeta_idl_reader reader{ "" };
    reader.read(struct_test_idl);
    REQUIRE(reader.get_num_syntax_errors() == 0);

    ExpectedStructModel expected_struct{ "S", "N.S" , { 
        { ExpectedTypeRefModel{ fundamental_type::Boolean } , "field_1" },
        { ExpectedTypeRefModel{ fundamental_type::String } , "field_2" },
        { ExpectedTypeRefModel{ fundamental_type::Int16 } , "field_3" },
        { ExpectedTypeRefModel{ fundamental_type::Int32 } , "field_4" },
        { ExpectedTypeRefModel{ fundamental_type::Int64 } , "field_5" },
        { ExpectedTypeRefModel{ fundamental_type::UInt8 } , "field_6" },
        { ExpectedTypeRefModel{ fundamental_type::UInt16 } , "field_7" },
        { ExpectedTypeRefModel{ fundamental_type::UInt32 } , "field_8" },
        { ExpectedTypeRefModel{ fundamental_type::Char16 } , "field_9" },
        { ExpectedTypeRefModel{ fundamental_type::Single } , "field_10" },
        { ExpectedTypeRefModel{ fundamental_type::Double } , "field_11" }
    } };
    ExpectedNamespaceModel expected_namespace{ "N", "N", {}, { expected_struct } };
    expected_namespace.VerifyType(find_namespace(reader, "N"));
}

TEST_CASE("Struct circular test")
{
    {
        std::istringstream struct_test_idl{ R"(
            namespace N
            {
                struct S0
                {
                    S1 field_1;
                }
                struct S1
                {
                    S2 field_1;
                }
                struct S2
                {
                    S3 field_1;
                }
                struct S3
                {
                    S1 field_1;       
                }
            }
        )" };

        xmeta_idl_reader reader{ "" };
        reader.read(struct_test_idl);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() > 0);
    }
    {
        std::istringstream struct_test_idl{ R"(
            namespace N
            {
                struct S0
                {
                    S1 field_1;
                    S2 field_2;
                }
                struct S1
                {
                    S3 field_1;
                }
                struct S2
                {
                    S3 field_1;
                }
                struct S3
                {
                }
            }
        )" };
        xmeta_idl_reader reader{ "" };
        reader.read(struct_test_idl);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 0);
    }
}

TEST_CASE("Struct duplicate member test")
{
    std::istringstream struct_test_idl{ R"(
        namespace N
        {
            struct S0
            {
                Int32 field_1;
                Int32 field_1;
            }
        }
    )" };

    xmeta_idl_reader reader{ "" };
    reader.read(struct_test_idl);
    REQUIRE(reader.get_num_syntax_errors() == 0);
    REQUIRE(reader.get_num_semantic_errors() == 1);
}

TEST_CASE("Resolving delegates type ref test")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            struct S1
            {
            };

            enum E1
            {
            }

            delegate E1 D1(S1 param1, E1 param2);
        }
    )" };

    xmeta_idl_reader reader{ "" };
    reader.read(test_idl);
    REQUIRE(reader.get_num_syntax_errors() == 0);
    ExpectedStructModel S1{ "S1", "N.S1", {} };
    ExpectedEnumModel E1{ "E1", "N.E1" , enum_type::Int32, {} };
    ExpectedDelegateModel D1{ "D1", "N.D1", ExpectedTypeRefModel{ ExpectedEnumRef{ E1.qualified_name } }, {
        ExpectedFormalParameterModel{ "param1", parameter_semantics::in, ExpectedTypeRefModel{ ExpectedStructRef{ S1.qualified_name } } },
        ExpectedFormalParameterModel{ "param2", parameter_semantics::in, ExpectedTypeRefModel{ ExpectedEnumRef{ E1.qualified_name }  } },
    } };
    ExpectedNamespaceModel expected{ "N", "N", {}, { S1, E1, D1 } };
    expected.VerifyType(find_namespace(reader, "N"));
}

TEST_CASE("Resolving struct type ref test")
{
    std::istringstream struct_test_idl{ R"(
        namespace N
        {
            struct S1
            {
            };

            enum E1
            {
            }

            struct S2
            {
                S1 field_1;
                E1 field_2;
            };
        }
    )" };
    xmeta_idl_reader reader{ "" };
    reader.read(struct_test_idl);
    REQUIRE(reader.get_num_syntax_errors() == 0);
    ExpectedStructModel S1{ "S1", "N.S1", {} };
    ExpectedEnumModel E1{ "E1", "N.E1" , enum_type::Int32, {} };
    ExpectedStructModel S2{ "S2", "N.S2", {
        { ExpectedTypeRefModel{ ExpectedStructRef{ S1.qualified_name }} , "field_1" },
        { ExpectedTypeRefModel{ ExpectedEnumRef{ E1.qualified_name } } , "field_2" },
    } };
    ExpectedNamespaceModel expected{ "N", "N", {}, { S1, E1, S2 } };
    expected.VerifyType(find_namespace(reader, "N"));
}

TEST_CASE("Resolving type ref across namespaces test")
{
    std::istringstream test_idl{ R"(
        namespace A
        {
            struct S1
            {
            };
        }

        namespace N
        {
            delegate Boolean D1(A.S1 param1, B.C.E1 param2);
        }

        namespace B.C
        {
            enum E1
            {
            }
        }

    )" };

    xmeta_idl_reader reader{ "" };
    reader.read(test_idl);
    REQUIRE(reader.get_num_syntax_errors() == 0);

    ExpectedStructModel S1{ "S1", "A.S1", {} };

    ExpectedEnumModel E1{ "E1", "B.C.E1" , enum_type::Int32, {} };

    ExpectedDelegateModel D1{ "D1", "N.D1", ExpectedTypeRefModel{ fundamental_type::Boolean }, {
        ExpectedFormalParameterModel{ "param1", parameter_semantics::in, ExpectedTypeRefModel{ ExpectedStructRef{ S1.qualified_name } } },
        ExpectedFormalParameterModel{ "param2", parameter_semantics::in, ExpectedTypeRefModel{ ExpectedEnumRef{ E1.qualified_name } } } }
    };

    ExpectedNamespaceModel A{ "A", "A", {}, { S1 } };
    ExpectedNamespaceModel N{ "N", "N", {}, { D1 } };

    ExpectedNamespaceModel C{ "C", "B.C", {}, { E1 } };
    ExpectedNamespaceModel B{ "B", "B", { C }, { } };

    A.VerifyType(find_namespace(reader, "A"));
    N.VerifyType(find_namespace(reader, "N"));
    B.VerifyType(find_namespace(reader, "B"));
}

TEST_CASE("Interface base test")
{
    // This test case demonstrates that the interface bases of IComboBox does not contain ITest 
    // and it is able to resolved interface references across namespaces.
    std::istringstream test_idl{ R"(
        namespace N
        {
            interface ITest
            {
            }
            interface ITextBox requires ITest
            {
                void SetText(String text);
            }
            interface IComboBox requires ITextBox, M.IListBox {}
        }
        namespace M
        {
            interface IListBox
            {
               void SetItem(String items);
            }
        }
    )" };
    xmeta_idl_reader reader{ "" };
    reader.read(test_idl);
    REQUIRE(reader.get_num_syntax_errors() == 0);

    ExpectedInterfaceRef ITextBox{ "N.ITextBox" };
    ExpectedInterfaceRef IListBox{ "M.IListBox" };
    ExpectedInterfaceModel IComboBox{ "IComboBox", "N.IComboBox", {}, {}, {}, { ExpectedTypeRefModel{ ITextBox }, ExpectedTypeRefModel{ IListBox } } };
    ExpectedNamespaceModel N{ "N", "N", {}, { IComboBox } };
    N.VerifyType(find_namespace(reader, "N"));
}

TEST_CASE("Method test")
{
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                interface i1
                {
                    void Paint();
                    Int32 Draw(Int32 i, Int32 d);
                }
                runtimeclass c1
                {
                    void Paint();
                    Int32 Draw(Int32 i, Int32 d);
                }
            }
        )" };

        xmeta_idl_reader reader{ "" };
        reader.read(test_idl);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 0);

        ExpectedMethodModel Paint{ "Paint", default_method_modifier, std::nullopt, {} };
        ExpectedMethodModel Draw{ "Draw", default_method_modifier, ExpectedTypeRefModel{ fundamental_type::Int32 }, {
            ExpectedFormalParameterModel{ "i", parameter_semantics::in, ExpectedTypeRefModel{ fundamental_type::Int32 } },
            ExpectedFormalParameterModel{ "d", parameter_semantics::in, ExpectedTypeRefModel{ fundamental_type::Int32 } }
        } };
        ExpectedInterfaceModel i1{ "i1", "N.i1", { Paint, Draw }, {}, {}, {} };
        ExpectedClassModel c1{ "c1", "N.c1", { Paint, Draw }, {}, {}, std::nullopt, {} };

        ExpectedNamespaceModel N{ "N", "N", {}, { c1, i1 } };
        N.VerifyType(find_namespace(reader, "N"));
    }
}

TEST_CASE("Method overloading test")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            interface i1
            {
                void Paint();
                void Paint(Int32 p1);
            }
            runtimeclass c1
            {
                void Paint();
                void Paint(Int32 p1);
            }
        }
    )" };

    xmeta_idl_reader reader{ "" };
    reader.read(test_idl);
    REQUIRE(reader.get_num_syntax_errors() == 0);
    REQUIRE(reader.get_num_semantic_errors() == 0);

    ExpectedMethodModel Paint{ "Paint", default_method_modifier, std::nullopt, {} };
    ExpectedMethodModel Paint2{ "Paint", default_method_modifier, std::nullopt, {
        ExpectedFormalParameterModel{ "p1", parameter_semantics::in, ExpectedTypeRefModel{ fundamental_type::Int32 } }
    } };

    ExpectedInterfaceModel i1{ "i1", "N.i1", { Paint, Paint2 }, {}, {}, {} };
    ExpectedClassModel c1{ "c1", "N.c1", { Paint, Paint2 }, {}, {}, std::nullopt, {} };

    ExpectedNamespaceModel N{ "N", "N", {}, { c1, i1 } };
    N.VerifyType(find_namespace(reader, "N"));
}

TEST_CASE("Method overloading test with simple types")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            interface i1
            {
                Int64 Paint();
                Int64 Paint(Int32 p1);
            }
            runtimeclass c1
            {
                Int64 Paint();
                Int64 Paint(Int32 p1);
            }
        }
    )" };

    xmeta_idl_reader reader{ "" };
    reader.read(test_idl);
    REQUIRE(reader.get_num_syntax_errors() == 0);
    REQUIRE(reader.get_num_semantic_errors() == 0);

    ExpectedMethodModel Paint{ "Paint", default_method_modifier, ExpectedTypeRefModel{ fundamental_type::Int64 }, {} };
    ExpectedMethodModel Paint2{ "Paint", default_method_modifier, ExpectedTypeRefModel{ fundamental_type::Int64 }, {
        ExpectedFormalParameterModel{ "p1", parameter_semantics::in, ExpectedTypeRefModel{ fundamental_type::Int32 } }
    } };

    ExpectedInterfaceModel i1{ "i1", "N.i1", { Paint, Paint2 }, {}, {}, {} };
    ExpectedClassModel c1{ "c1", "N.c1", { Paint, Paint2 }, {}, {}, std::nullopt, {} };

    ExpectedNamespaceModel N{ "N", "N", {}, { c1, i1 } };
    N.VerifyType(find_namespace(reader, "N"));
}

TEST_CASE("Method overloading test with type refs")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            struct s1
            {
            }
            interface i1
            {
                s1 Paint();
                s1 Paint(Int32 p1);
            }
            runtimeclass c1
            {
                s1 Paint();
                s1 Paint(Int32 p1);
            }
        }
    )" };

    xmeta_idl_reader reader{ "" };
    reader.read(test_idl);
    REQUIRE(reader.get_num_syntax_errors() == 0);
    REQUIRE(reader.get_num_semantic_errors() == 0);

    ExpectedMethodModel Paint{ "Paint", default_method_modifier, ExpectedTypeRefModel{ ExpectedStructRef{ "N.s1" } }, {} };
    ExpectedMethodModel Paint2{ "Paint", default_method_modifier, ExpectedTypeRefModel{ ExpectedStructRef{ "N.s1" } }, {
        ExpectedFormalParameterModel{ "p1", parameter_semantics::in, ExpectedTypeRefModel{ fundamental_type::Int32 } }
    } };

    ExpectedInterfaceModel i1{ "i1", "N.i1", { Paint, Paint2 }, {}, {}, {} };
    ExpectedClassModel c1{ "c1", "N.c1", { Paint, Paint2 }, {}, {}, std::nullopt, {} };

    ExpectedNamespaceModel N{ "N", "N", {}, { c1, i1 } };
    N.VerifyType(find_namespace(reader, "N"));
}

TEST_CASE("Method invalid overloading test")
{
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                interface i1
                {
                    Int32 Paint();
                    Int64 Paint();
                }
                runtimeclass c1
                {
                    Int32 Paint();
                    Int64 Paint();
                }
            }
        )" };

        xmeta_idl_reader reader{ "" };
        reader.read(test_idl);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 2);
    }
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                interface i1
                {
                    Int32 Paint();
                    Int32 Paint();
                }
                runtimeclass c1
                {
                    Int32 Paint();
                    Int32 Paint();
                }
            }
        )" };

        xmeta_idl_reader reader{ "" };
        reader.read(test_idl);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 2);
    }
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                interface i1
                {
                    void Paint();
                    Int32 Paint();
                }
                runtimeclass c1
                {
                    Int32 Paint();
                    void Paint();
                }
            }
        )" };

        xmeta_idl_reader reader{ "" };
        reader.read(test_idl);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 2);
    }
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                struct s1
                {
                }

                interface i1
                {
                    s1 Paint();
                    Int32 Paint();
                }
                runtimeclass c1
                {
                    Int32 Paint();
                    s1 Paint();
                }
            }
        )" };

        xmeta_idl_reader reader{ "" };
        reader.read(test_idl);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 2);
    }
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                struct s1
                {
                }

                interface i1
                {
                    s1 Paint();
                    void Paint();
                }
                runtimeclass c1
                {
                    void Paint();
                    S1 Paint();
                }
            }
        )" };

        xmeta_idl_reader reader{ "" };
        reader.read(test_idl);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 2);
    }

    // TODO: more examples testing arity on formal parameters
}

TEST_CASE("Resolving method type ref test")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            struct S1
            {
            };

            interface i1
            {
                E1 Draw(S1 p1, M.S2 p2);
            }

            runtimeclass c1
            {
                E1 Draw(S1 p1, M.S2 p2);
            }

            enum E1
            {
            }
        }
        namespace M
        {
            struct S2
            {
            };
        }
    )" };

    xmeta_idl_reader reader{ "" };
    reader.read(test_idl);
    REQUIRE(reader.get_num_syntax_errors() == 0);

    ExpectedMethodModel Draw{ "Draw", default_method_modifier, ExpectedTypeRefModel{ ExpectedEnumRef{ "N.E1" } }, {
        ExpectedFormalParameterModel{ "p1", parameter_semantics::in, ExpectedTypeRefModel{ ExpectedStructRef{ "N.S1" } } },
        ExpectedFormalParameterModel{ "p2", parameter_semantics::in, ExpectedTypeRefModel{ ExpectedStructRef{ "M.S2" } } }
    } };
    ExpectedClassModel c1{ "c1", "N.c1", { Draw }, {}, {}, std::nullopt, {} };
    ExpectedInterfaceModel i1{ "i1", "N.i1", { Draw }, {}, {}, {} };
    ExpectedNamespaceModel N{ "N", "N", {}, { i1, c1 } };
    N.VerifyType(find_namespace(reader, "N"));
}

TEST_CASE("Property method ordering test")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            interface i1
            {
                Int32 property1 { get; set; };
                Int32 property2 { get; };
                Int32 property3 { set; get; };
            }

            runtimeclass c1
            {
                Int32 property1 { get; set; };
                Int32 property2 { get; };
                Int32 property3 { set; get; };
            }
        }
    )" };

    xmeta_idl_reader reader{ "" };
    reader.read(test_idl);
    REQUIRE(reader.get_num_syntax_errors() == 0);

    ExpectedMethodModel get_property1{ "get_property1", default_method_modifier, ExpectedTypeRefModel{ fundamental_type::Int32 }, {} };
    ExpectedMethodModel set_property1{ "put_property1", default_method_modifier, std::nullopt,
        { ExpectedFormalParameterModel{ "TODO:findname", parameter_semantics::in, ExpectedTypeRefModel{ fundamental_type::Int32 } } } };
    ExpectedPropertyModel property1{ "property1",
        default_property_modifier,
        ExpectedTypeRefModel{ fundamental_type::Int32 },
        get_property1,
        set_property1
    };

    ExpectedMethodModel get_property2{ "get_property2", default_method_modifier, ExpectedTypeRefModel{ fundamental_type::Int32 }, {} };
    ExpectedPropertyModel property2{ "property2",
        default_property_modifier,
        ExpectedTypeRefModel{ fundamental_type::Int32 },
        get_property2,
        std::nullopt
    };

    ExpectedMethodModel get_property3{ "get_property3", default_method_modifier, ExpectedTypeRefModel{ fundamental_type::Int32 }, {} };
    ExpectedMethodModel set_property3{ "put_property3", default_method_modifier, std::nullopt,
        { ExpectedFormalParameterModel{ "TODO:findname", parameter_semantics::in, ExpectedTypeRefModel{ fundamental_type::Int32 } } } };
    ExpectedPropertyModel property3{ "property3",
        default_property_modifier,
        ExpectedTypeRefModel{ fundamental_type::Int32 },
        get_property3,
        set_property3
    };

    ExpectedInterfaceModel i1{ "i1", "N.i1", 
        { get_property1, set_property1, get_property2, set_property3, get_property3 },
        { property1, property2, property3 },
        {},
        {} };

    ExpectedClassModel c1{ "c1", "N.c1",
        { get_property1, set_property1, get_property2, set_property3, get_property3 },
        { property1, property2, property3 },
        {},
        std::nullopt,
        {} };

    ExpectedNamespaceModel N{ "N", "N", {}, { i1, c1 } };
    N.VerifyType(find_namespace(reader, "N"));
}

TEST_CASE("Property method ordering different line test")
{
    ExpectedMethodModel get_property1{ "get_property1", default_method_modifier, ExpectedTypeRefModel{ fundamental_type::Int32 }, {} };
    ExpectedMethodModel set_property1{ "put_property1", default_method_modifier, std::nullopt,
        { ExpectedFormalParameterModel{ "TODO:findname", parameter_semantics::in, ExpectedTypeRefModel{ fundamental_type::Int32 } } } };
    ExpectedPropertyModel property1{ "property1",
        default_property_modifier,
        ExpectedTypeRefModel{ fundamental_type::Int32 },
        get_property1,
        set_property1
    };
    ExpectedMethodModel get_property2{ "get_property2", default_method_modifier, ExpectedTypeRefModel{ fundamental_type::Int32 }, {} };
    ExpectedMethodModel set_property2{ "put_property2", default_method_modifier, std::nullopt,
        { ExpectedFormalParameterModel{ "TODO:findname", parameter_semantics::in, ExpectedTypeRefModel{ fundamental_type::Int32 } } } };
    ExpectedPropertyModel property2{ "property2",
        default_property_modifier,
        ExpectedTypeRefModel{ fundamental_type::Int32 },
        get_property2,
        set_property2
    };
    ExpectedMethodModel draw{ "draw", default_method_modifier, std::nullopt, {} };
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                interface i1
                {
                    Int32 property1 { get; };
                    Int32 property1 { set; };
                }
                runtimeclass c1
                {
                    Int32 property1 { get; };
                    Int32 property1 { set; };
                }
            }
        )" };

        xmeta_idl_reader reader{ "" };
        reader.read(test_idl);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        ExpectedInterfaceModel i1{ "i1", "N.i1",
            { get_property1, set_property1 },
            { property1 },
            {},
            {} };
        ExpectedClassModel c1{ "c1", "N.c1",
            { get_property1, set_property1 },
            { property1 },
            {},
            std::nullopt,
            {} };
        ExpectedNamespaceModel N{ "N", "N", {}, { i1, c1 } };
        N.VerifyType(find_namespace(reader, "N"));
    }
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                interface i1
                {
                    Int32 property1 { set; };
                    Int32 property1 { get; };
                }
                runtimeclass c1
                {
                    Int32 property1 { set; };
                    Int32 property1 { get; };
                }
            }
        )" };
        xmeta_idl_reader reader{ "" };
        reader.read(test_idl);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        ExpectedInterfaceModel i1{ "i1", "N.i1",
            { set_property1, get_property1 },
            { property1 },
            {},
            {} };
        ExpectedClassModel c1{ "c1", "N.c1",
            { set_property1, get_property1 },
            { property1 },
            {},
            std::nullopt,
            {} };
        ExpectedNamespaceModel N{ "N", "N", {}, { i1, c1 } };
        N.VerifyType(find_namespace(reader, "N"));
    }
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                interface i1
                {
                    Int32 property1 { get; };
                    Int32 property2 { set; };
                    void draw();
                    Int32 property1 { set; };
                    Int32 property2 { get; };
                }
                runtimeclass c1
                {
                    Int32 property1 { get; };
                    Int32 property2 { set; };
                    void draw();
                    Int32 property1 { set; };
                    Int32 property2 { get; };
                }
            }
        )" };
        xmeta_idl_reader reader{ "" };
        reader.read(test_idl);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        ExpectedInterfaceModel i1{ "i1", "N.i1",
            { get_property1, set_property2, draw, set_property1, get_property2 },
            { property1, property2 },
            {},
            {} };
        ExpectedClassModel c1{ "c1", "N.c1",
            { get_property1, set_property2, draw, set_property1, get_property2 },
            { property1, property2 },
            {},
            std::nullopt,
            {} };
        ExpectedNamespaceModel N{ "N", "N", {}, { i1, c1 } };
        N.VerifyType(find_namespace(reader, "N"));
    }
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                delegate void StringListEvent(Int32 sender);
                interface i1
                {
                    Int32 property1 { get; };
                    Int32 property2 { set; };
                    void draw();
                    Int32 property1 { set; };
                    event StringListEvent Changed;
                    Int32 property2 { get; };
                }
                runtimeclass c1
                {
                    Int32 property1 { get; };
                    Int32 property2 { set; };
                    void draw();
                    Int32 property1 { set; };
                    event StringListEvent Changed;
                    Int32 property2 { get; };
                }
            }
        )" };
        std::vector<std::string> paths = { "Foundation.xmeta" };
        xmeta_idl_reader reader{ "" , paths };
        reader.read(test_idl, true);
        REQUIRE(reader.get_num_syntax_errors() == 0);

        ExpectedEventModel Changed{ "Changed", default_event_modifier, ExpectedTypeRefModel{ ExpectedDelegateRef{ "N.StringListEvent" } } };

        ExpectedInterfaceModel i1{ "i1", "N.i1",
            { get_property1, set_property2, draw, set_property1, Changed.get_add_method(), Changed.get_remove_method(), get_property2 },
            { property1, property2 },
            { Changed },
            {}
        };
        ExpectedClassModel c1{ "c1", "N.c1",
            { get_property1, set_property2, draw, set_property1, Changed.get_add_method(), Changed.get_remove_method(), get_property2 },
            { property1, property2 },
            { Changed },
            std::nullopt,
            {}
        };
        ExpectedNamespaceModel N{ "N", "N", {}, { i1, c1 } };
        N.VerifyType(find_namespace(reader, "N"));
    }
}

TEST_CASE("Invalid property accessor test")
{
    {
        {
            std::istringstream test_set_only_idl{ R"(
                namespace N
                {
                    interface i1
                    {
                        Int32 property1 { set; };
                    }
                }
            )" };

            xmeta_idl_reader reader{ "" };
            reader.read(test_set_only_idl);
            REQUIRE(reader.get_num_syntax_errors() == 0);
            REQUIRE(reader.get_num_semantic_errors() == 1);
        }
        {
            std::istringstream test_set_only_idl{ R"(
                namespace N
                {
                    runtimeclass c1
                    {
                        Int32 property3 { set; };
                    }
                }
            )" };

            xmeta_idl_reader reader{ "" };
            reader.read(test_set_only_idl);
            REQUIRE(reader.get_num_syntax_errors() == 0);
            //TODO: This is only reporting one error due to the synthesized interface
            // Make this only report 1
            REQUIRE(reader.get_num_semantic_errors() == 2); 
        }
    }

    // multiple getters
    {
        {
            std::istringstream test_double_get_idl{ R"(
                namespace N
                {
                    interface i1
                    {
                        Int32 property1 { get; get; };
                    }
                }
            )" };

            xmeta_idl_reader reader{ "" };
            reader.read(test_double_get_idl);
            REQUIRE(reader.get_num_syntax_errors() == 0);
            REQUIRE(reader.get_num_semantic_errors() == 1);
        }
        {
            std::istringstream test_double_get_idl{ R"(
                namespace N
                {
                    interface i1
                    {
                        Int32 property1 { get; };
                        Int32 property1 { get; };
                    }
                }
            )" };

            xmeta_idl_reader reader{ "" };
            reader.read(test_double_get_idl);
            REQUIRE(reader.get_num_syntax_errors() == 0);
            REQUIRE(reader.get_num_semantic_errors() >= 1);
        }
        {
            std::istringstream test_double_get_idl{ R"(
                namespace N
                {
                    runtimeclass c1
                    {
                        Int32 property1 { get; get; };
                    }
                }
            )" };

            xmeta_idl_reader reader{ "" };
            reader.read(test_double_get_idl);
            REQUIRE(reader.get_num_syntax_errors() == 0);
            REQUIRE(reader.get_num_semantic_errors() == 1);
        }
        {
            std::istringstream test_double_get_idl{ R"(
                namespace N
                {
                    runtimeclass c1
                    {
                        Int32 property1 { get; };
                        Int32 property1 { get; };
                    }
                }
            )" };

            xmeta_idl_reader reader{ "" };
            reader.read(test_double_get_idl);
            REQUIRE(reader.get_num_syntax_errors() == 0);
            REQUIRE(reader.get_num_semantic_errors() >= 1);
        }
    }
    // multiple setters
    {
        {
            std::istringstream test_double_set_idl{ R"(
                namespace N
                {
                    interface i1
                    {
                        Int32 property1 { set; set; };
                    }
                }
            )" };

            xmeta_idl_reader reader{ "" };
            reader.read(test_double_set_idl);
            REQUIRE(reader.get_num_syntax_errors() == 0);
            REQUIRE(reader.get_num_semantic_errors() == 1);
        }
        {
            std::istringstream test_double_set_idl{ R"(
                namespace N
                {
                    runtimeclass c1
                    {
                        Int32 property1 { set; set; };
                    }
                }
            )" };

            xmeta_idl_reader reader{ "" };
            reader.read(test_double_set_idl);
            REQUIRE(reader.get_num_syntax_errors() == 0);
            REQUIRE(reader.get_num_semantic_errors() == 1);
        }
    }
    {
        {
            std::istringstream test_double_set_idl{ R"(
                namespace N
                {
                    interface i1
                    {
                        Int32 property1 { set; };
                        Int32 property1 { set; };
                    }
                }
            )" };

            xmeta_idl_reader reader{ "" };
            reader.read(test_double_set_idl);
            REQUIRE(reader.get_num_syntax_errors() == 0);
            REQUIRE(reader.get_num_semantic_errors() >= 1);
        }
        {
            std::istringstream test_double_set_idl{ R"(
                namespace N
                {
                    runtimeclass c1
                    {
                        Int32 property1 { set; };
                        Int32 property1 { set; };
                    }
                }
            )" };

            xmeta_idl_reader reader{ "" };
            reader.read(test_double_set_idl);
            REQUIRE(reader.get_num_syntax_errors() == 0);
            REQUIRE(reader.get_num_semantic_errors() >= 1);
        }
    }
    {
        {
            std::istringstream test_three_acessor_idl{ R"(
                namespace N
                {
                    interface i1
                    {
                        Int32 property1 { set; get; get; };
                    }
                }
            )" };

            xmeta_idl_reader reader{ "" };
            reader.read(test_three_acessor_idl);
            REQUIRE(reader.get_num_syntax_errors() == 0);
            REQUIRE(reader.get_num_semantic_errors() == 1);
        }
        {
            std::istringstream test_three_acessor_idl{ R"(
                namespace N
                {
                    runtimeclass c1
                    {
                        Int32 property1 { set; get; get; };
                    }
                }
            )" };

            xmeta_idl_reader reader{ "" };
            reader.read(test_three_acessor_idl);
            REQUIRE(reader.get_num_syntax_errors() == 0);
            REQUIRE(reader.get_num_semantic_errors() == 1);
        }
    }
    {
        {
            std::istringstream test_add_and_remove_idl{ R"(
                namespace N
                {
                    interface i1
                    {
                        Int32 property1 { get; add; };
                        Int32 property2 { get; remove; };
                    }
                }
            )" };

            xmeta_idl_reader reader{ "" };
            reader.read(test_add_and_remove_idl);
            REQUIRE(reader.get_num_syntax_errors() == 2);
            REQUIRE(reader.get_num_semantic_errors() == 0);
        }
        {
            std::istringstream test_add_and_remove_idl{ R"(
                namespace N
                {
                    runtimeclass c1
                    {
                        Int32 property1 { get; add; };
                        Int32 property2 { get; remove; };
                    }
                }
            )" };

            xmeta_idl_reader reader{ "" };
            reader.read(test_add_and_remove_idl);
            REQUIRE(reader.get_num_syntax_errors() == 2);
            REQUIRE(reader.get_num_semantic_errors() == 0);
        }
    }
}

TEST_CASE("Duplicate property id test")
{
    {
        std::istringstream test_set_only_idl{ R"(
            namespace N
            {
                interface i1
                {
                    Int32 property1 { get; };
                    Int64 property1 { get; };
                }
            }
        )" };

        xmeta_idl_reader reader{ "" };
        reader.read(test_set_only_idl);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() >= 1);
    }
    {
        std::istringstream test_set_only_idl{ R"(
            namespace N
            {
                runtimeclass c1
                {
                    Int32 property1 { get; };
                    Int64 property1 { get; };
                }
            }
        )" };

        xmeta_idl_reader reader{ "" };
        reader.read(test_set_only_idl);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() >= 1);
    }
}

TEST_CASE("Property implicit accessors test")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            interface i1
            {
                Int32 property1;
            }
            runtimeclass c1
            {
                Int32 property1;
            }
        }
    )" };

    xmeta_idl_reader reader{ "" };
    reader.read(test_idl);
    REQUIRE(reader.get_num_syntax_errors() == 0);

    ExpectedMethodModel get_property1{ "get_property1", default_method_modifier, ExpectedTypeRefModel{ fundamental_type::Int32 }, {} };
    ExpectedMethodModel set_property1{ "put_property1", default_method_modifier, std::nullopt,
        { ExpectedFormalParameterModel{ "TODO:findname", parameter_semantics::in, ExpectedTypeRefModel{ fundamental_type::Int32 } } } };
    ExpectedPropertyModel property1{ "property1",
        default_property_modifier,
        ExpectedTypeRefModel{ fundamental_type::Int32 },
        get_property1,
        set_property1
    };

    ExpectedInterfaceModel i1{ "i1", "N.i1",
        { get_property1, set_property1 },
        { property1 },
        {},
        {} 
    };

    ExpectedClassModel c1{ "c1", "N.c1",
        { get_property1, set_property1 },
        { property1 },
        {},
        std::nullopt,
        {}
    };

    ExpectedNamespaceModel N{ "N", "N", {}, { i1, c1 } };
    N.VerifyType(find_namespace(reader, "N"));
}

TEST_CASE("Resolving property type ref test")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            struct S1
            {
            };

            interface i1
            {
                S1 property1;
                M.E2 property2;
            }

            runtimeclass c1
            {
                S1 property1;
                M.E2 property2;
            }
        }
        namespace M
        {
            enum E2
            {
            };
        }
    )" };

    xmeta_idl_reader reader{ "" };
    reader.read(test_idl);
    REQUIRE(reader.get_num_syntax_errors() == 0);

    ExpectedMethodModel get_property1{ "get_property1", default_method_modifier, ExpectedTypeRefModel{ ExpectedStructRef{ "N.S1" } }, {} };
    ExpectedMethodModel set_property1{ "put_property1", default_method_modifier, std::nullopt,
        { ExpectedFormalParameterModel{ "TODO:findname", parameter_semantics::in, ExpectedTypeRefModel{ ExpectedStructRef{ "N.S1" } } } } };
    ExpectedPropertyModel property1{ "property1",
        default_property_modifier,
        ExpectedTypeRefModel{ ExpectedStructRef{ "N.S1" } },
        get_property1,
        set_property1
    };

    ExpectedMethodModel get_property2{ "get_property2", default_method_modifier, ExpectedTypeRefModel{ ExpectedEnumRef{ "M.E2" } }, {} };
    ExpectedMethodModel set_property2{ "put_property2", default_method_modifier, std::nullopt,
        { ExpectedFormalParameterModel{ "TODO:findname", parameter_semantics::in, ExpectedTypeRefModel{ ExpectedEnumRef{ "M.E2" } } } } };
    ExpectedPropertyModel property2{ "property2",
        default_property_modifier,
        ExpectedTypeRefModel{ ExpectedEnumRef{ "M.E2" } },
        get_property2,
        set_property2
    };

    ExpectedInterfaceModel i1{ "i1", "N.i1",
        { get_property1, set_property1, get_property2, set_property2 },
        { property1, property2 },
        {},
        {}
    };

    ExpectedClassModel c1{ "c1", "N.c1",
        { get_property1, set_property1, get_property2, set_property2 },
        { property1, property2 },
        {},
        std::nullopt,
        {}
    };

    ExpectedNamespaceModel N{ "N", "N", {}, { i1, c1 } };
    N.VerifyType(find_namespace(reader, "N"));
}

TEST_CASE("Event test")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            delegate void StringListEvent(Int32 sender);
            interface i1
            {
                event StringListEvent Changed;
            }

            runtimeclass c1
            {
                event StringListEvent Changed;
            }
        }
    )" };
    std::vector<std::string> paths = { "Foundation.xmeta" };
    xmeta_idl_reader reader{ "" , paths};
    reader.read(test_idl, true);
    REQUIRE(reader.get_num_syntax_errors() == 0);

    ExpectedEventModel Changed{ "Changed", default_event_modifier, ExpectedTypeRefModel{ ExpectedDelegateRef{ "N.StringListEvent" } } };

    ExpectedInterfaceModel i1{ "i1", "N.i1",
        { Changed.get_add_method(), Changed.get_remove_method() },
        {},
        { Changed },
        {}
    };

    ExpectedClassModel c1{ "c1", "N.c1",
        { Changed.get_add_method(), Changed.get_remove_method() },
        {},
        { Changed },
         std::nullopt,
        {}
    };

    ExpectedNamespaceModel N{ "N", "N", {}, { c1, i1 } };
    N.VerifyType(find_namespace(reader, "N"));
}

TEST_CASE("Event explicit accessor not allowed test")
{
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                delegate void StringListEvent(Int32 sender);
                interface i1
                {
                    event StringListEvent Changed { add; remove; };
                }
            }
        )" };
        std::vector<std::string> paths = { "Foundation.xmeta" };
        xmeta_idl_reader reader{ "" , paths };
        reader.read(test_idl);
        REQUIRE(reader.get_num_syntax_errors() > 0);
    }
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                delegate void StringListEvent(Int32 sender);
                runtimeclass c1
                {
                    event StringListEvent Changed { add; remove; };
                }
            }
        )" };
        std::vector<std::string> paths = { "Foundation.xmeta" };
        xmeta_idl_reader reader{ "" , paths };
        reader.read(test_idl);
        REQUIRE(reader.get_num_syntax_errors() > 0);
    }
}

TEST_CASE("Duplicate event test")
{
    {
        {
            std::istringstream test_idl{ R"(
                namespace N
                {
                    delegate void StringListEvent(Int32 sender);
                    interface i1
                    {
                        event StringListEvent Changed;
                        event StringListEvent Changed;
                    }
                }
            )" };
            std::vector<std::string> paths = { "Foundation.xmeta" };
            xmeta_idl_reader reader{ "" , paths };
            reader.read(test_idl);
            REQUIRE(reader.get_num_syntax_errors() == 0);
            REQUIRE(reader.get_num_semantic_errors() == 2);
        }
        {
            std::istringstream test_idl{ R"(
                namespace N
                {
                    delegate void StringListEvent(Int32 sender);
                    runtimeclass c1
                    {
                        event StringListEvent Changed;
                        event StringListEvent Changed;
                    }
                }
            )" };
            std::vector<std::string> paths = { "Foundation.xmeta" };
            xmeta_idl_reader reader{ "" , paths };
            reader.read(test_idl);
            REQUIRE(reader.get_num_syntax_errors() == 0);
            REQUIRE(reader.get_num_semantic_errors() == 2);
        }
    }
    {
        {
            std::istringstream test_idl{ R"(
                namespace N
                {
                    delegate void StringListEvent(Int32 sender);
                    interface i1
                    {
                        event StringListEvent Changed;
                        event StringStackEvent Changed;
                    }
                    delegate void StringStackEvent(Int32 sender);
                }
            )" };
            std::vector<std::string> paths = { "Foundation.xmeta" };
            xmeta_idl_reader reader{ "" , paths };
            reader.read(test_idl);
            REQUIRE(reader.get_num_syntax_errors() == 0);
            REQUIRE(reader.get_num_semantic_errors() == 2);
        }
        {
            std::istringstream test_idl{ R"(
                namespace N
                {
                    delegate void StringListEvent(Int32 sender);
                    runtimeclass c1
                    {
                        event StringListEvent Changed;
                        event StringStackEvent Changed;
                    }
                    delegate void StringStackEvent(Int32 sender);
                }
            )" };
            std::vector<std::string> paths = { "Foundation.xmeta" };
            xmeta_idl_reader reader{ "" , paths };
            reader.read(test_idl);
            REQUIRE(reader.get_num_syntax_errors() == 0);
            REQUIRE(reader.get_num_semantic_errors() == 2);
        }
    }
}

TEST_CASE("Event and property name collision test")
{
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                delegate void StringListEvent(Int32 sender);
                interface i1
                {
                    event StringListEvent Changed;
                    Int32 Changed;
                }
            }
        )" };
        std::vector<std::string> paths = { "Foundation.xmeta" };
        xmeta_idl_reader reader{ "" , paths };
        reader.read(test_idl);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 1);
    }
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                delegate void StringListEvent(Int32 sender);
                runtimeclass c1
                {
                    event StringListEvent Changed;
                    Int32 Changed;
                }
            }
        )" };
        std::vector<std::string> paths = { "Foundation.xmeta" };
        xmeta_idl_reader reader{ "" , paths };
        reader.read(test_idl);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 1);
    }
}

TEST_CASE("Event and method name collision test")
{
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                delegate void StringListEvent(Int32 sender);
                interface i1
                {
                    Int32 remove_Changed();
                    event StringListEvent Changed;
                    void add_Changed();
                }
            }
        )" };
        std::vector<std::string> paths = { "Foundation.xmeta" };
        xmeta_idl_reader reader{ "" , paths };
        reader.read(test_idl);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 2);
    }
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                delegate void StringListEvent(Int32 sender);
                runtimeclass c1
                {
                    void add_Changed();
                    event StringListEvent Changed;
                    Int32 remove_Changed();
                }
            }
        )" };
        std::vector<std::string> paths = { "Foundation.xmeta" };
        xmeta_idl_reader reader{ "" , paths };
        reader.read(test_idl);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 2);
    }
}

TEST_CASE("Property method name collision test")
{
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                interface i1
                {
                    void get_property1();
                    Int32 property1 { get; set; };
                    void put_property1(Int32 i);
                }
            }
        )" };

        xmeta_idl_reader reader{ "" };
        reader.read(test_idl);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 2);
    }
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                runtimeclass c1
                {
                    void get_property1();
                    Int32 property1 { get; set; };
                    void put_property1(Int32 i);
                }
            }
        )" };

        xmeta_idl_reader reader{ "" };
        reader.read(test_idl);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 2);
    }
}

TEST_CASE("Interface circular inheritance test")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            interface i1 requires IListBox
            {
                void Paint();
            }
            interface ITextBox requires i1
            {
                void SetText(String text);
            }
            interface IListBox requires ITextBox
            {
               void SetItem(String items);
            }
        }
    )" };

    xmeta_idl_reader reader{ "" };
    reader.read(test_idl);
    REQUIRE(reader.get_num_syntax_errors() == 0);
    REQUIRE(reader.get_num_semantic_errors() > 0);
}

TEST_CASE("Interface member declared in inheritance test")
{
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                interface i1 requires IListBox
                {
                    void Paint();
                }
                interface IListBox
                {
                    void Paint();
                }
            }
        )" };

        xmeta_idl_reader reader{ "" };
        reader.read(test_idl);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 1);
    }
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                interface i1 requires IListBox
                {
                    Int32 value;
                }
                interface IListBox
                {
                    void get_value();
                }
            }
        )" };

        xmeta_idl_reader reader{ "" };
        reader.read(test_idl);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 1);
    }
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                interface i1 requires IListBox
                {
                    Int32 value;
                }
                interface IListBox
                {
                    void get_value();
                }
            }
        )" };

        xmeta_idl_reader reader{ "" };
        reader.read(test_idl);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 1);
    }
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                interface i1 requires IListBox
                {
                    Int32 value;
                }
                interface IListBox
                {
                    Int32 value;
                }
            }
        )" };

        xmeta_idl_reader reader{ "" };
        reader.read(test_idl);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 1);
    }
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                delegate void StringListEvent(Int32 sender);
                interface i1 requires IListBox
                {
                    Int32 value;
                }
                interface IListBox
                {
                    event StringListEvent value;
                }
            }
        )" };

        xmeta_idl_reader reader{ "" };
        reader.read(test_idl);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 1);
    }
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                delegate void StringListEvent(Int32 sender);
                interface i1 requires IListBox
                {
                    Int32 value;
                }
                interface IListBox
                {
                    event StringListEvent value;
                }
            }
        )" };

        xmeta_idl_reader reader{ "" };
        reader.read(test_idl);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 1);
    }
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                delegate void StringListEvent(Int32 sender);
                delegate void StringListEvent2(Int32 sender);
                interface i1 requires IListBox
                {
                    event StringListEvent value;
                }
                interface IListBox
                {
                    event StringListEvent2 value;
                }
            }
        )" };

        xmeta_idl_reader reader{ "" };
        reader.read(test_idl);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 1);
    }
}

TEST_CASE("Unresolved types interface test")
{
    // The unresolved type is in fakebase, FakeObject(both in obj and doSomething2), FakeOject2, and StringListEvent
    std::istringstream test_idl{ R"(
        namespace N
        {
            interface i1 requires fakebase
            {
                event StringListEvent Changed;
                FakeObject obj { get; set; };
                FakeObject doSomething2(FakeObject2 test);
            }
        }
    )" };
    std::vector<std::string> paths = { "Foundation.xmeta" };
    xmeta_idl_reader reader{ "" , paths };
    reader.read(test_idl);
    REQUIRE(reader.get_num_syntax_errors() == 0);
    REQUIRE(reader.get_num_semantic_errors() == 5);
}

TEST_CASE("Runtimeclass circular inheritance test")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            runtimeclass c1 requires IListBox
            {
                void Paint();
            }
            interface ITextBox requires IListBox
            {
                void SetText(String text);
            }
            interface IListBox requires ITextBox
            {
               void SetItem(String items);
            }
        }
    )" };

    xmeta_idl_reader reader{ "" };
    reader.read(test_idl);
    REQUIRE(reader.get_num_syntax_errors() == 0);
    REQUIRE(reader.get_num_semantic_errors() > 0);
}

TEST_CASE("Runtimeclass member declared in inheritance test")
{
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                runtimeclass c1 requires IListBox
                {
                    void Paint();
                }
                interface IListBox
                {
                    void Paint();
                }
            }
        )" };

        xmeta_idl_reader reader{ "" };
        reader.read(test_idl);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() > 0);
    }
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                runtimeclass c1 requires IListBox
                {
                    Int32 value;
                }
                interface IListBox
                {
                    void get_value();
                }
            }
        )" };

        xmeta_idl_reader reader{ "" };
        reader.read(test_idl);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 1);
    }
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                runtimeclass c1 requires IListBox
                {
                    Int32 value;
                }
                interface IListBox
                {
                    void get_value();
                }
            }
        )" };

        xmeta_idl_reader reader{ "" };
        reader.read(test_idl);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 1);
    }
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                runtimeclass c1 requires IListBox
                {
                    Int32 value;
                }
                interface IListBox
                {
                    Int32 value;
                }
            }
        )" };

        xmeta_idl_reader reader{ "" };
        reader.read(test_idl);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 1);
    }
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                delegate void StringListEvent(Int32 sender);
                runtimeclass c1 requires IListBox
                {
                    Int32 value;
                }
                interface IListBox
                {
                    event StringListEvent value;
                }
            }
        )" };

        xmeta_idl_reader reader{ "" };
        reader.read(test_idl);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 1);
    }
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                delegate void StringListEvent(Int32 sender);
                runtimeclass c1 requires IListBox
                {
                    Int32 value;
                }
                interface IListBox
                {
                    event StringListEvent value;
                }
            }
        )" };

        xmeta_idl_reader reader{ "" };
        reader.read(test_idl);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 1);
    }
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                delegate void StringListEvent(Int32 sender);
                delegate void StringListEvent2(Int32 sender);
                runtimeclass c1 requires IListBox
                {
                    event StringListEvent value;
                }
                interface IListBox
                {
                    event StringListEvent2 value;
                }
            }
        )" };

        xmeta_idl_reader reader{ "" };
        reader.read(test_idl);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 1);
    }
}

TEST_CASE("Unresolved types Runtimeclass test")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            runtimeclass c1 requires fakebase
            {
                event StringListEvent Changed;
                FakeObject obj { get; set; };
                FakeObject doSomething2(FakeObject2 test);
            }
        }
    )" };
    std::vector<std::string> paths = { "Foundation.xmeta" };
    xmeta_idl_reader reader{ "" , paths };
    reader.read(test_idl);
    REQUIRE(reader.get_num_syntax_errors() == 0);
    //TODO: This is only reporting one error due to the synthesized interface
    // Make this only report 5
    REQUIRE(reader.get_num_semantic_errors() >= 5);
}

TEST_CASE("Class methods synthesized test")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            runtimeclass c1
            {
                Int32 m1(String s);
                S1 m2();
                M.S1 m3();
            }

            struct S1
            {
            }
        }

        namespace M
        {
            struct S1
            {
            }
        }
    )" };
    xmeta_idl_reader reader{ "" };
    reader.read(test_idl);
    REQUIRE(reader.get_num_syntax_errors() == 0);

    ExpectedMethodModel m1{ "m1", default_method_modifier, ExpectedTypeRefModel{ fundamental_type::Int32 }, {
        ExpectedFormalParameterModel{ "s", parameter_semantics::in, ExpectedTypeRefModel{ fundamental_type::String } } } };
    ExpectedMethodModel m2{ "m2", default_method_modifier, ExpectedTypeRefModel{ ExpectedStructRef{ "N.S1" } }, {} };
    ExpectedMethodModel m3{ "m3", default_method_modifier, ExpectedTypeRefModel{ ExpectedStructRef{ "M.S1" } }, {} };

    ExpectedClassModel c1{ "c1", "N.c1",
        { m1, m2, m3 },
        {},
        {},
        std::nullopt,
        {}
    };

    ExpectedInterfaceModel syn_c1{ "Ic1", "N.Ic1",
        { m1, m2, m3 },
        {},
        {},
        {}
    };

    ExpectedNamespaceModel N{ "N", "N", {}, { c1, syn_c1 } };
    N.VerifyType(find_namespace(reader, "N"));
}

TEST_CASE("Class static method test")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            runtimeclass c1
            {
                static void m0s();
                static Int32 m1s(String s);
            }
        }
    )" };
    xmeta_idl_reader reader{ "" };
    reader.read(test_idl);
    REQUIRE(reader.get_num_syntax_errors() == 0);

    ExpectedMethodModel m0s{ "m0s", static_method_modifier, std::nullopt, {} };
    ExpectedMethodModel m1s{ "m1s", static_method_modifier, ExpectedTypeRefModel{ fundamental_type::Int32 }, {
        ExpectedFormalParameterModel{ "s", parameter_semantics::in, ExpectedTypeRefModel{ fundamental_type::String } } } };
    ExpectedClassModel c1{ "c1", "N.c1",
        { m0s, m1s },
        {},
        {},
        std::nullopt,
        {}
    };

    ExpectedInterfaceModel syn_c1{ "Ic1Statics", "N.Ic1Statics",
        { m0s, m1s },
        {},
        {},
        {}
    };

    ExpectedNamespaceModel N{ "N", "N", {}, { c1, syn_c1 } };
    N.VerifyType(find_namespace(reader, "N"));
}


TEST_CASE("Class static property test")
{
    ExpectedMethodModel get_p1{ "get_p1", static_method_modifier, ExpectedTypeRefModel{ fundamental_type::Int32 }, {} };
    ExpectedMethodModel set_p1{ "put_p1", static_method_modifier, std::nullopt, {
        ExpectedFormalParameterModel{ "TODO:findname", parameter_semantics::in, ExpectedTypeRefModel{ fundamental_type::Int32 } } } };

    ExpectedPropertyModel p1{ "p1", static_property_modifier, ExpectedTypeRefModel{ fundamental_type::Int32 }, get_p1, set_p1 };

    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                runtimeclass c1
                {
                    static Int32 p1;
                }
            }
        )" };
        xmeta_idl_reader reader{ "" };
        reader.read(test_idl);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        ExpectedClassModel c1{ "c1", "N.c1",
            { get_p1, set_p1 },
            { p1 },
            {},
            std::nullopt,
            {}
        };

        ExpectedInterfaceModel syn_c1{ "Ic1Statics", "N.Ic1Statics",
            { get_p1, set_p1 },
            { p1 },
            {},
            {}
        };

        ExpectedNamespaceModel N{ "N", "N", {}, { c1, syn_c1 } };
        N.VerifyType(find_namespace(reader, "N"));
    }

    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                runtimeclass c1
                {
                    static Int32 p1 { get; set; };
                }
            }
        )" };
        xmeta_idl_reader reader{ "" };
        reader.read(test_idl);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        ExpectedClassModel c1{ "c1", "N.c1",
            { get_p1, set_p1 },
            { p1 },
            {},
            std::nullopt,
            {}
        };

        ExpectedInterfaceModel syn_c1{ "Ic1Statics", "N.Ic1Statics",
            { get_p1, set_p1 },
            { p1 },
            {},
            {}
        };

        ExpectedNamespaceModel N{ "N", "N", {}, { c1, syn_c1 } };
        N.VerifyType(find_namespace(reader, "N"));
    }

    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                runtimeclass c1
                {
                    static Int32 p1 { get; } 
                    static Int32 p1 { set; };
                }
            }
        )" };
        xmeta_idl_reader reader{ "" };
        reader.read(test_idl);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        ExpectedClassModel c1{ "c1", "N.c1",
            { get_p1, set_p1 },
            { p1 },
            {},
            std::nullopt,
            {}
        };

        ExpectedInterfaceModel syn_c1{ "Ic1Statics", "N.Ic1Statics",
            { get_p1, set_p1 },
            { p1 },
            {},
            {}
        };

        ExpectedNamespaceModel N{ "N", "N", {}, { c1, syn_c1 } };
        N.VerifyType(find_namespace(reader, "N"));
    }

    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                runtimeclass c1
                {
                    static Int32 p1 { get; };
                }
            }
        )" };
        xmeta_idl_reader reader{ "" };
        reader.read(test_idl);
        REQUIRE(reader.get_num_syntax_errors() == 0);

        ExpectedPropertyModel p1g{ "p1", static_property_modifier, ExpectedTypeRefModel{ fundamental_type::Int32 }, get_p1, std::nullopt };

        ExpectedClassModel c1{ "c1", "N.c1",
            { get_p1 },
            { p1g },
            {},
            std::nullopt,
            {}
        };

        ExpectedInterfaceModel syn_c1{ "Ic1Statics", "N.Ic1Statics",
            { get_p1 },
            { p1g },
            {},
            {}
        };

        ExpectedNamespaceModel N{ "N", "N", {}, { c1, syn_c1 } };
        N.VerifyType(find_namespace(reader, "N"));
    }
}


TEST_CASE("Class static events test")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            delegate void StringListEvent(Int32 sender);
            runtimeclass c1
            {
                static event StringListEvent e1;
            }
        }
    )" };
    std::vector<std::string> paths = { "Foundation.xmeta" };
    xmeta_idl_reader reader{ "" , paths };
    reader.read(test_idl);
    REQUIRE(reader.get_num_syntax_errors() == 0);

    ExpectedEventModel e1{ "e1", static_event_modifier, ExpectedTypeRefModel{ ExpectedDelegateRef{ "N.StringListEvent" } } };
    ExpectedClassModel c1{ "c1", "N.c1",
        { e1.add_method, e1.remove_method },
        {},
        { e1 },
        std::nullopt,
        {}
    };

    ExpectedInterfaceModel syn_c1{ "Ic1Statics", "N.Ic1Statics",
        { e1.add_method, e1.remove_method },
        {},
        { e1 },
        {}
    };

    ExpectedNamespaceModel N{ "N", "N", {}, { c1, syn_c1 } };
    N.VerifyType(find_namespace(reader, "N"));
}

TEST_CASE("Static class has static members only test")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            delegate void StringListEvent(Int32 sender);
            static runtimeclass c1
            {
                void m0s();
                Int32 p1;
                event StringListEvent e1;
            }
        }
    )" };
    xmeta_idl_reader reader{ "" };
    reader.read(test_idl);
    REQUIRE(reader.get_num_syntax_errors() == 0);
    REQUIRE(reader.get_num_semantic_errors() == 3);
}

TEST_CASE("Runtime class synthensized interfaces test")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            runtimeclass c1
            {
                void m0();
                static Int32 m1(String s);
            }
        }
    )" };
    
    ExpectedMethodModel m0{ "m0", default_method_modifier, std::nullopt, {} };
    ExpectedMethodModel m1{ "m1", static_method_modifier, ExpectedTypeRefModel{ fundamental_type::Int32 }, {
        ExpectedFormalParameterModel{ "s", parameter_semantics::in, ExpectedTypeRefModel{ fundamental_type::String } } } };
    
    std::vector<std::string> paths = { "Foundation.xmeta" };
    xmeta_idl_reader reader{ "" , paths };
    reader.read(test_idl);
    REQUIRE(reader.get_num_syntax_errors() == 0);
    ExpectedClassModel c1{ "c1", "N.c1",
        { m0, m1 },
        {},
        {},
        std::nullopt,
        {}
    };

    ExpectedInterfaceModel syn_c1{ "Ic1", "N.Ic1",
        { m0 },
        {},
        {},
        {}
    };

    ExpectedInterfaceModel syn_static_c1{ "Ic1Statics", "N.Ic1Statics",
        { m1 },
        {},
        {},
        {}
    };

    ExpectedNamespaceModel N{ "N", "N", {}, { c1, syn_c1, syn_static_c1 } };

    N.VerifyType(find_namespace(reader, "N"));
}

TEST_CASE("Runtime class synthensized instance interface test")
{
    ExpectedMethodModel m0{ "m0", default_method_modifier, std::nullopt, {} };
    ExpectedMethodModel m1{ "m1", default_method_modifier, ExpectedTypeRefModel{ fundamental_type::Int32 }, {
        ExpectedFormalParameterModel{ "s", parameter_semantics::in, ExpectedTypeRefModel{ fundamental_type::String } } } };

    ExpectedEventModel e1{ "e1", default_event_modifier, ExpectedTypeRefModel{ ExpectedDelegateRef{ "N.StringListEvent" } } };

    ExpectedMethodModel get_p1{ "get_p1", default_method_modifier, ExpectedTypeRefModel{ fundamental_type::Int32 }, {} };
    ExpectedMethodModel set_p1{ "put_p1", default_method_modifier, std::nullopt, {
        ExpectedFormalParameterModel{ "TODO:findname", parameter_semantics::in, ExpectedTypeRefModel{ fundamental_type::Int32 } } } };
    ExpectedPropertyModel p1{ "p1", default_property_modifier, ExpectedTypeRefModel{ fundamental_type::Int32 }, get_p1, set_p1 };
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                delegate void StringListEvent(Int32 sender);
                runtimeclass c1
                {
                    event StringListEvent e1;
                    void m0();
                    Int32 p1;
                    Int32 m1(String s);
                }
            }
        )" };
        std::vector<std::string> paths = { "Foundation.xmeta" };
        xmeta_idl_reader reader{ "" , paths };
        reader.read(test_idl);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        ExpectedClassModel c1{ "c1", "N.c1",
            { e1.add_method, e1.remove_method, m0, get_p1, set_p1, m1 },
            { p1 },
            { e1 },
            std::nullopt,
            {}
        };

        ExpectedInterfaceModel syn_c1{ "Ic1", "N.Ic1",
            { e1.add_method, e1.remove_method, m0, get_p1, set_p1, m1 },
            { p1 },
            { e1 },
            {}
        };

        ExpectedNamespaceModel N{ "N", "N", {}, { c1, syn_c1 } };

        N.VerifyType(find_namespace(reader, "N"));
    }

    // Testing different ordering
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                delegate void StringListEvent(Int32 sender);
                runtimeclass c1
                {
                    void m0();
                    Int32 p1 { get; };
                    Int32 m1(String s);
                    event StringListEvent e1;
                    Int32 p1 { set; };
                }
            }
        )" };
        std::vector<std::string> paths = { "Foundation.xmeta" };
        xmeta_idl_reader reader{ "" , paths };
        reader.read(test_idl);
        REQUIRE(reader.get_num_syntax_errors() == 0);

        ExpectedClassModel c1{ "c1", "N.c1",
            { m0, get_p1, m1, e1.add_method, e1.remove_method, set_p1 },
            { p1 },
            { e1 },
            std::nullopt,
            {}
                };

        ExpectedInterfaceModel syn_c1{ "Ic1", "N.Ic1",
            { m0, get_p1, m1, e1.add_method, e1.remove_method, set_p1 },
            { p1 },
            { e1 },
            {}
        };

        ExpectedNamespaceModel N{ "N", "N", {}, { c1, syn_c1 } };

        N.VerifyType(find_namespace(reader, "N"));
    }
}

TEST_CASE("Runtime class synthensized static interface test")
{
    ExpectedMethodModel m0{ "m0", static_method_modifier, std::nullopt, {} };
    ExpectedMethodModel m1{ "m1", static_method_modifier, ExpectedTypeRefModel{ fundamental_type::Int32 }, {
        ExpectedFormalParameterModel{ "s", parameter_semantics::in, ExpectedTypeRefModel{ fundamental_type::String } } } };

    ExpectedEventModel e1{ "e1", static_event_modifier, ExpectedTypeRefModel{ ExpectedDelegateRef{ "N.StringListEvent" } } };

    ExpectedMethodModel get_p1{ "get_p1", static_method_modifier, ExpectedTypeRefModel{ fundamental_type::Int32 }, {} };
    ExpectedMethodModel set_p1{ "put_p1", static_method_modifier, std::nullopt, {
        ExpectedFormalParameterModel{ "TODO:findname", parameter_semantics::in, ExpectedTypeRefModel{ fundamental_type::Int32 } } } };
    ExpectedPropertyModel p1{ "p1", static_property_modifier, ExpectedTypeRefModel{ fundamental_type::Int32 }, get_p1, set_p1 };
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                delegate void StringListEvent(Int32 sender);
                runtimeclass c1
                {
                    static event StringListEvent e1;
                    static void m0();
                    static Int32 p1;
                    static Int32 m1(String s);
                }
            }
        )" };
        std::vector<std::string> paths = { "Foundation.xmeta" };
        xmeta_idl_reader reader{ "" , paths };
        reader.read(test_idl);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        ExpectedClassModel c1{ "c1", "N.c1",
            { e1.add_method, e1.remove_method, m0, get_p1, set_p1, m1 },
            { p1 },
            { e1 },
            std::nullopt,
            {}
        };

        ExpectedInterfaceModel syn_c1{ "Ic1Statics", "N.Ic1Statics",
            { e1.add_method, e1.remove_method, m0, get_p1, set_p1, m1 },
            { p1 },
            { e1 },
            {}
        };

        ExpectedNamespaceModel N{ "N", "N", {}, { c1, syn_c1 } };

        N.VerifyType(find_namespace(reader, "N"));
    }

    // Testing different ordering
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                delegate void StringListEvent(Int32 sender);
                runtimeclass c1
                {
                    static void m0();
                    static Int32 p1 { get; };
                    static Int32 m1(String s);
                    static event StringListEvent e1;
                    static Int32 p1 { set; };
                }
            }
        )" };
        std::vector<std::string> paths = { "Foundation.xmeta" };
        xmeta_idl_reader reader{ "" , paths };
        reader.read(test_idl);
        REQUIRE(reader.get_num_syntax_errors() == 0);

        ExpectedClassModel c1{ "c1", "N.c1",
            { m0, get_p1, m1, e1.add_method, e1.remove_method, set_p1 },
            { p1 },
            { e1 },
            std::nullopt,
            {}
        };

        ExpectedInterfaceModel syn_c1{ "Ic1Statics", "N.Ic1Statics",
            { m0, get_p1, m1, e1.add_method, e1.remove_method, set_p1 },
            { p1 },
            { e1 },
            {}
        };

        ExpectedNamespaceModel N{ "N", "N", {}, { c1, syn_c1 } };

        N.VerifyType(find_namespace(reader, "N"));
    }
}

TEST_CASE("Runtime class constructor test")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            runtimeclass Area
            {
                Area(); // default constructor must use IActivationFactory
                Area(Int32 width, Int32 height);
            }
        }
    )" };

    std::vector<std::string> paths = { "Foundation.xmeta" };
    xmeta_idl_reader reader{ "" , paths };
    reader.read(test_idl);
    REQUIRE(reader.get_num_syntax_errors() == 0);

    ExpectedMethodModel Area{ ".ctor", default_method_modifier, std::nullopt, {} };
    ExpectedMethodModel Area2{ ".ctor", default_method_modifier, std::nullopt, {
        ExpectedFormalParameterModel{ "width", parameter_semantics::in, ExpectedTypeRefModel{ fundamental_type::Int32 } },
        ExpectedFormalParameterModel{ "height", parameter_semantics::in, ExpectedTypeRefModel{ fundamental_type::Int32 } }
    } };

    ExpectedClassModel c1Area{ "Area", "N.Area",
        { Area, Area2 },
        {},
        {},
        std::nullopt,
        {}
    };

    ExpectedInterfaceModel syn_c1{ "IAreaFactory", "N.IAreaFactory",
        { Area2 },
        {},
        {},
        {}
    };

    ExpectedNamespaceModel N{ "N", "N", {}, { c1Area, syn_c1 } };

    N.VerifyType(find_namespace(reader, "N"));
}

TEST_CASE("Runtime class require interface test")
{
    ExpectedMethodModel m0{ "m0", default_method_modifier, std::nullopt, {} };
    ExpectedMethodModel m1{ "m1", default_method_modifier, ExpectedTypeRefModel{ fundamental_type::Int32 }, {
        ExpectedFormalParameterModel{ "s", parameter_semantics::in, ExpectedTypeRefModel{ fundamental_type::String } } } };

    ExpectedEventModel e1{ "e1", default_event_modifier, ExpectedTypeRefModel{ ExpectedDelegateRef{ "N.StringListEvent" } } };

    ExpectedMethodModel get_p1{ "get_p1", default_method_modifier, ExpectedTypeRefModel{ fundamental_type::Int32 }, {} };
    ExpectedMethodModel set_p1{ "put_p1", default_method_modifier, std::nullopt, {
        ExpectedFormalParameterModel{ "TODO:findname", parameter_semantics::in, ExpectedTypeRefModel{ fundamental_type::Int32 } } } };
    ExpectedPropertyModel p1{ "p1", default_property_modifier, ExpectedTypeRefModel{ fundamental_type::Int32 }, get_p1, set_p1 };
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                delegate void StringListEvent(Int32 sender);
                interface i1
                {
                    event StringListEvent e1;
                    void m0();
                    Int32 p1;
                    Int32 m1(String s);
                }

                runtimeclass c1 requires i1
                {
                }
            }
        )" };
        std::vector<std::string> paths = { "Foundation.xmeta" };
        xmeta_idl_reader reader{ "" , paths };
        reader.read(test_idl);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        ExpectedClassModel c1{ "c1", "N.c1",
            {},
            {},
            {},
            std::nullopt,
            { ExpectedInterfaceRef{ "N.i1" } }
        };

        ExpectedInterfaceModel i1{ "i1", "N.i1",
            { e1.add_method, e1.remove_method, m0, get_p1, set_p1, m1 },
            { p1 },
            { e1 },
            {}
        };

        ExpectedNamespaceModel N{ "N", "N", {}, { c1, i1 } };

        N.VerifyType(find_namespace(reader, "N"));
    }

    // Testing different ordering
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                delegate void StringListEvent(Int32 sender);
                runtimeclass c1
                {
                    void m0();
                    Int32 p1 { get; };
                    Int32 m1(String s);
                    event StringListEvent e1;
                    Int32 p1 { set; };
                }
            }
        )" };
        std::vector<std::string> paths = { "Foundation.xmeta" };
        xmeta_idl_reader reader{ "" , paths };
        reader.read(test_idl);
        REQUIRE(reader.get_num_syntax_errors() == 0);

        ExpectedClassModel c1{ "c1", "N.c1",
            { m0, get_p1, m1, e1.add_method, e1.remove_method, set_p1 },
            { p1 },
            { e1 },
            std::nullopt,
            {}
        };

        ExpectedInterfaceModel syn_c1{ "Ic1", "N.Ic1",
            { m0, get_p1, m1, e1.add_method, e1.remove_method, set_p1 },
            { p1 },
            { e1 },
            {}
        };

        ExpectedNamespaceModel N{ "N", "N", {}, { c1, syn_c1 } };

        N.VerifyType(find_namespace(reader, "N"));
    }
}

TEST_CASE("Array test")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            struct test{}
            interface i1
            {
                test[] Paint();
            }
        }
    )" };

    xmeta_idl_reader reader{ "" };
    reader.read(test_idl);
    REQUIRE(reader.get_num_syntax_errors() == 0);
    REQUIRE(reader.get_num_semantic_errors() == 0);

    //ExpectedMethodModel Paint{ "Paint", default_method_modifier, ExpectedTypeRefModel{ ExpectedStructRef{ "N.s1" } }, {} };
    //ExpectedMethodModel Paint2{ "Paint", default_method_modifier, ExpectedTypeRefModel{ ExpectedStructRef{ "N.s1" } }, {
    //    ExpectedFormalParameterModel{ "p1", parameter_semantics::in, ExpectedTypeRefModel{ fundamental_type::Int32 } }
    //} };

    //ExpectedInterfaceModel i1{ "i1", "N.i1", { Paint, Paint2 }, {}, {}, {} };
    //ExpectedClassModel c1{ "c1", "N.c1", { Paint, Paint2 }, {}, {}, std::nullopt, {} };

    //ExpectedNamespaceModel N{ "N", "N", {}, { c1, i1 } };
    //N.VerifyType(find_namespace(reader, "N"));
}
