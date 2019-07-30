
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string_view>

#include "antlr4-runtime.h"
#include "xmeta_idl_reader.h"
#include "ast_to_st_listener.h"

using namespace antlr4;
using namespace xlang::xmeta;
using namespace xlang::meta::reader;

constexpr method_modifier default_method_modifier = { false, false, false };
constexpr method_modifier static_method_modifier = { false, true, false };
constexpr property_modifier default_property_modifier = { false, false };
constexpr property_modifier static_property_modifier = { false, true };
constexpr event_modifier default_event_modifier = { false, false };
constexpr event_modifier static_event_modifier = { false, true };

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
        ElementType,
        object_type> type;

    bool is_araray;

    ExpectedTypeRefModel(ExpectedClassRef const& type, bool is_array = false)
        : type{ type }, is_araray{ is_array } {}

    ExpectedTypeRefModel(ExpectedInterfaceRef const& type, bool is_array = false)
        : type{ type }, is_araray{ is_array } {}

    ExpectedTypeRefModel(ExpectedDelegateRef const& type, bool is_array = false)
        : type{ type }, is_araray{ is_array } {}

    ExpectedTypeRefModel(ExpectedEnumRef const& type, bool is_array = false)
        : type{ type }, is_araray{ is_array } {}

    ExpectedTypeRefModel(ExpectedStructRef const& type, bool is_array = false)
        : type{ type }, is_araray{ is_array } {}

    ExpectedTypeRefModel(object_type const& type, bool is_array = false)
        : type{ type }, is_araray{ is_array } {}

    ExpectedTypeRefModel(ElementType const& type, bool is_array = false)
        : type{ type }, is_araray{ is_array } {}

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
        if (std::holds_alternative<ElementType>(target))
        {
            REQUIRE(std::holds_alternative<ElementType>(type));
            auto const& model = std::get<ElementType>(type);
            auto const& actual_model = std::get<ElementType>(target);
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
            ExpectedFormalParameterModel{ "value", parameter_semantics::in, type } } };

        remove_method = { "remove_" + id, method_sem, std::nullopt, {
            ExpectedFormalParameterModel{ "value", parameter_semantics::in, ExpectedTypeRefModel{ ExpectedStructRef{ "Foundation.EventRegistrationToken" } } } } };
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
        : id{ id }, qualified_name{ qualified_name }, fields{ fields } {}

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
        : id{ id }, qualified_name{ qualified_name }, children{ namespaces }
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

struct ExpectedAttributeTypeModel
{
    std::string id;
    std::map<std::string, ExpectedTypeRefModel> named_parameter;
    std::vector<ExpectedTypeRefModel> positonal_parameter;

    ExpectedAttributeTypeModel(std::string const& id,
        std::map<std::string, ExpectedTypeRefModel> named_parameter,
        std::vector<ExpectedTypeRefModel> positonal_parameter)
        : id{ id }, named_parameter{ named_parameter }, positonal_parameter{ positonal_parameter } {}

    void VerifyType(std::shared_ptr<attribute_type_model> const& actual)
    {
        REQUIRE(actual->get_name() == id);

        auto const& actual_named_parameter = actual->get_named_parameters();
        REQUIRE(actual_named_parameter.size() == named_parameter.size());
        for (auto const&[name, ref] : actual_named_parameter)
        {
            REQUIRE(ref.get_semantic().is_resolved());
            named_parameter.at(name).VerifyType(ref);
        }

        auto const& actual_positional_parameter = actual->get_positonal_parameters();
        for (size_t i = 0; i < positonal_parameter.size(); i++)
        {
            REQUIRE(actual_positional_parameter.at(i).get_semantic().is_resolved());
            positonal_parameter.at(i).VerifyType(actual_positional_parameter.at(i));
        }
    }
};