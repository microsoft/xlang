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

constexpr method_semantics default_method_semantics = { false, false, false };
constexpr method_semantics in_method_semantics = { false, true, false };

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

struct ExpectedEnumRef
{
    std::string fully_qualified_id;
    ExpectedEnumRef(std::string_view id) : fully_qualified_id{ id } {}
};

struct ExpectedDelegateRef
{
    std::string fully_qualified_id;
    ExpectedDelegateRef(std::string_view id) : fully_qualified_id{ id } {}
};

struct ExpectedStructRef
{
    std::string fully_qualified_id;
    ExpectedStructRef(std::string_view id) : fully_qualified_id{ id } {}
};

struct ExpectedTypeRefModel
{
   std::variant<ExpectedEnumRef,
        ExpectedDelegateRef,
        ExpectedStructRef,
        simple_type,
        object_type> type;

    ExpectedTypeRefModel(ExpectedDelegateRef const& type)
        : type{ type } {}

    ExpectedTypeRefModel(ExpectedEnumRef const& type)
        : type{ type } {}

    ExpectedTypeRefModel(ExpectedStructRef const& type)
        : type{ type } {}

    ExpectedTypeRefModel(object_type const& type)
        : type{ type } {}

    ExpectedTypeRefModel(simple_type const& type)
        : type{ type } {}

    void VerifyType(type_ref const& actual)
    {
        REQUIRE(actual.get_semantic().is_resolved());
        //std::get<simple_type>(method1_return_type.get_resolved_target());
        auto const& target = actual.get_semantic().get_resolved_target();
        //if (std::holds_alternative<std::shared_ptr<class_model>>(target))
        //{
        //    auto const& actual_model = std::get<std::shared_ptr<class_model>>(target);
        //    REQUIRE(actual_model->get_fully_qualified_id() == fully_qualified_id_or_type);
        //    REQUIRE(std::holds_alternative<object_type>(type));
        //}
        if (std::holds_alternative<std::shared_ptr<delegate_model>>(target))
        {
            REQUIRE(std::holds_alternative<ExpectedDelegateRef>(type));
            auto const& model = std::get<ExpectedDelegateRef>(type);
            auto const& actual_model = std::get<std::shared_ptr<delegate_model>>(target);
            REQUIRE(actual_model->get_fully_qualified_id() == model.fully_qualified_id);
        }
        if (std::holds_alternative<std::shared_ptr<struct_model>>(target))
        {
            REQUIRE(std::holds_alternative<ExpectedStructRef>(type));
            auto const& model = std::get<ExpectedStructRef>(type);
            auto const& actual_model = std::get<std::shared_ptr<struct_model>>(target);
            REQUIRE(actual_model->get_fully_qualified_id() == model.fully_qualified_id);
        }
        if (std::holds_alternative<std::shared_ptr<enum_model>>(target))
        {
            REQUIRE(std::holds_alternative<ExpectedEnumRef>(type));
            auto const& model = std::get<ExpectedEnumRef>(type);
            auto const& actual_model = std::get<std::shared_ptr<enum_model>>(target);
            REQUIRE(actual_model->get_fully_qualified_id() == model.fully_qualified_id);
        }
        if (std::holds_alternative<simple_type>(target))
        {
            REQUIRE(std::holds_alternative<simple_type>(type));
            auto const& model = std::get<simple_type>(type);
            auto const& actual_model = std::get<simple_type>(target);
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
        REQUIRE(actual->get_id() == id);
        REQUIRE(actual->get_semantic() == sem);
        REQUIRE(actual->get_type().get_semantic().is_resolved());
        type.VerifyType(actual->get_type());
    }

    void VerifyType(formal_parameter_model const& actual)
    {
        REQUIRE(actual.get_id() == id);
        REQUIRE(actual.get_semantic() == sem);
        REQUIRE(actual.get_type().get_semantic().is_resolved());
        type.VerifyType(actual.get_type());
    }
};

struct ExpectedMethodModel
{
    std::string id;
    method_semantics sem;
    std::optional<ExpectedTypeRefModel> return_type;
    std::vector<ExpectedFormalParameterModel> params;

    ExpectedMethodModel(std::string const& name, method_semantics const& sem, std::optional<ExpectedTypeRefModel> const& return_type, std::vector<ExpectedFormalParameterModel> params)
        : id{ name }, sem{ sem }, return_type{ return_type }, params{ params } {}

    void VerifyType(std::shared_ptr<method_model> const& actual)
    {
        REQUIRE(actual->get_id() == id);
        REQUIRE(actual->get_semantic().is_static == sem.is_static);
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
    property_semantics sem;
    ExpectedTypeRefModel type;

    ExpectedMethodModel get_method;
    std::optional<ExpectedMethodModel> set_method;

    ExpectedPropertyModel(std::string const& name, 
            property_semantics const& sem, 
            ExpectedTypeRefModel const& type, 
            ExpectedMethodModel const& get_method,
            std::optional<ExpectedMethodModel> const& set_method)
        : id{ name }, sem{ sem }, type{ type }, get_method{ get_method }, set_method{ set_method } {}

    void VerifyType(std::shared_ptr<property_model> const& actual)
    {
        REQUIRE(actual->get_id() == id);
        REQUIRE(actual->get_semantic().is_static == sem.is_static);
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
    event_semantics sem;
    ExpectedTypeRefModel type;

    ExpectedMethodModel add_method;
    ExpectedMethodModel remove_method;

    ExpectedEventModel(std::string const& name,
        event_semantics const& sem,
        ExpectedTypeRefModel const& type,
        ExpectedMethodModel const& add_method,
        ExpectedMethodModel const& remove_method)
        : id{ name }, sem{ sem }, type{ type }, add_method{ add_method }, remove_method{ remove_method } {}

    void VerifyType(std::shared_ptr<event_model> const& actual)
    {
        REQUIRE(actual->get_id() == id);
        REQUIRE(actual->get_semantic().is_static == sem.is_static);
        REQUIRE(actual->get_type().get_semantic().is_resolved());
        type.VerifyType(actual->get_type());

        REQUIRE(actual->get_add_method());
        add_method.VerifyType(actual->get_add_method());
        REQUIRE(actual->get_remove_method());
        remove_method.VerifyType(actual->get_remove_method());
    }
};

struct ExpectedInterfaceModel
{
    std::string id;
    std::string fully_qualified_id;
    std::vector<ExpectedMethodModel> methods;
    std::vector<ExpectedPropertyModel> properties;
    std::vector<ExpectedInterfaceModel> bases;

    ExpectedInterfaceModel(std::string const& id, 
            std::string const& fully_qualified_id, 
            std::vector<ExpectedMethodModel> const& methods,
            std::vector<ExpectedPropertyModel> const& properties,
            std::vector<ExpectedInterfaceModel> const& bases)
        : id{ id }, fully_qualified_id{ fully_qualified_id }, methods{ methods }, properties{ properties }, bases{ bases } {}

    void VerifyType(std::shared_ptr<interface_model> const& actual)
    {
        REQUIRE(actual->get_id() == id);
        REQUIRE(actual->get_fully_qualified_id() == fully_qualified_id);
        auto const& actual_methods = actual->get_methods();
        REQUIRE(actual_methods.size() == methods.size());
        for (size_t i = 0; i < methods.size(); i++)
        {
            methods[i].VerifyType(actual_methods[i]);
        }
    }
};

struct ExpectedDelegateModel
{
    std::string id;
    std::string fully_qualified_id;
    std::optional<ExpectedTypeRefModel> return_type;
    std::vector<ExpectedFormalParameterModel> params;

    ExpectedDelegateModel(std::string const& id, std::string const& fully_qualified_id, std::optional<ExpectedTypeRefModel> const& return_type, std::vector<ExpectedFormalParameterModel> params)
        : id{ id }, fully_qualified_id{ fully_qualified_id }, return_type{ return_type }, params{ params } {}

    void VerifyType(std::shared_ptr<delegate_model> const& actual)
    {
        REQUIRE(actual->get_id() == id);
        REQUIRE(actual->get_fully_qualified_id() == fully_qualified_id);

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
    std::string fully_qualified_id;
    std::vector<enum_member> fields;
    enum_semantics sem;

    ExpectedEnumModel(std::string const& id, std::string const& fully_qualified_id, enum_semantics sem, std::vector<enum_member> fields)
        : id{ id }, fully_qualified_id{ fully_qualified_id }, fields{ fields }, sem{ sem } {}

    void VerifyType(std::shared_ptr<enum_model> const& actual)
    {
        REQUIRE(actual->get_id() == id);
        REQUIRE(actual->get_fully_qualified_id() == fully_qualified_id);
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
    std::string fully_qualified_id;
    std::vector<std::pair<ExpectedTypeRefModel, std::string>> fields;

    ExpectedStructModel(std::string const& id, std::string const& fully_qualified_id, std::vector<std::pair<ExpectedTypeRefModel, std::string>> fields)
        : id{ id }, fully_qualified_id { fully_qualified_id }, fields{ fields } {}

    void VerifyType(std::shared_ptr<struct_model> const& actual)
    {
        REQUIRE(actual->get_id() == id);
        REQUIRE(actual->get_fully_qualified_id() == fully_qualified_id);

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
    std::string fully_qualified_id;
    std::vector<ExpectedNamespaceModel> children;

    std::vector<ExpectedInterfaceModel> interfaces;
    std::vector<ExpectedStructModel> structs;
    std::vector<ExpectedEnumModel> enums;
    std::vector<ExpectedDelegateModel> delegates;

    ExpectedNamespaceModel(std::string const& id, 
            std::string const& fully_qualified_id, 
            std::vector<ExpectedNamespaceModel> namespaces, 
            std::vector<std::variant<ExpectedEnumModel, ExpectedStructModel, ExpectedDelegateModel, ExpectedInterfaceModel>> declarations)
        : id{ id }, fully_qualified_id{ fully_qualified_id }, children { namespaces } 
    {
        for (auto & declaration : declarations)
        {
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
        REQUIRE(actual->get_id() == id);
        REQUIRE(actual->get_fully_qualified_id() == fully_qualified_id);

        for (auto const& actual_bodies : actual->get_namespace_bodies())
        {
            auto const& actual_interfaces = actual_bodies->get_interfaces();
            
            for (auto const& inter : actual_interfaces)
            {
                std::cout << inter.second->get_id() << std::endl;
            }
            for (auto expected_interface : interfaces)
            {
                std::cout << "finding" << expected_interface.id << std::endl;
                auto const& it = actual_interfaces.find(expected_interface.id);
                REQUIRE(it != actual_interfaces.end());
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
    
    ExpectedEnumModel expected_enum{ "E", "N.E" , enum_semantics::Int32, { 
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

    ExpectedEnumModel E{ "E", "N.E" , enum_semantics::Int32, {} };
    ExpectedEnumModel F{ "F", "N.F" , enum_semantics::Int32, {} };
    ExpectedDelegateModel D1{ "D1", "N.D1", ExpectedTypeRefModel{ simple_type::Int32 }, {
        ExpectedFormalParameterModel{ "i", parameter_semantics::in, ExpectedTypeRefModel{ simple_type::Int32 } },
        ExpectedFormalParameterModel{ "d", parameter_semantics::in, ExpectedTypeRefModel{ simple_type::Double } },
        ExpectedFormalParameterModel{ "e", parameter_semantics::in, ExpectedTypeRefModel{ ExpectedEnumRef { E.fully_qualified_id } } },
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
        { ExpectedTypeRefModel{ simple_type::Boolean } , "field_1" },
        { ExpectedTypeRefModel{ simple_type::String } , "field_2" },
        { ExpectedTypeRefModel{ simple_type::Int16 } , "field_3" },
        { ExpectedTypeRefModel{ simple_type::Int32 } , "field_4" },
        { ExpectedTypeRefModel{ simple_type::Int64 } , "field_5" },
        { ExpectedTypeRefModel{ simple_type::UInt8 } , "field_6" },
        { ExpectedTypeRefModel{ simple_type::UInt16 } , "field_7" },
        { ExpectedTypeRefModel{ simple_type::UInt32 } , "field_8" },
        { ExpectedTypeRefModel{ simple_type::Char16 } , "field_9" },
        { ExpectedTypeRefModel{ simple_type::Single } , "field_10" },
        { ExpectedTypeRefModel{ simple_type::Double } , "field_11" }
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
    ExpectedEnumModel E1{ "E1", "N.E1" , enum_semantics::Int32, {} };
    ExpectedDelegateModel D1{ "D1", "N.D1", ExpectedTypeRefModel{ ExpectedEnumRef{ E1.fully_qualified_id } }, {
        ExpectedFormalParameterModel{ "param1", parameter_semantics::in, ExpectedTypeRefModel{ ExpectedStructRef{ S1.fully_qualified_id } } },
        ExpectedFormalParameterModel{ "param2", parameter_semantics::in, ExpectedTypeRefModel{ ExpectedEnumRef{ E1.fully_qualified_id }  } },
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
    ExpectedEnumModel E1{ "E1", "N.E1" , enum_semantics::Int32, {} };
    ExpectedStructModel S2{ "S2", "N.S2", {
        { ExpectedTypeRefModel{ ExpectedStructRef{ S1.fully_qualified_id }} , "field_1" },
        { ExpectedTypeRefModel{ ExpectedEnumRef{ E1.fully_qualified_id } } , "field_2" },
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

    ExpectedEnumModel E1{ "E1", "B.C.E1" , enum_semantics::Int32, {} };

    ExpectedDelegateModel D1{ "D1", "N.D1", ExpectedTypeRefModel{ simple_type::Boolean }, {
        ExpectedFormalParameterModel{ "param1", parameter_semantics::in, ExpectedTypeRefModel{ ExpectedStructRef{ S1.fully_qualified_id } } },
        ExpectedFormalParameterModel{ "param2", parameter_semantics::in, ExpectedTypeRefModel{ ExpectedEnumRef{ E1.fully_qualified_id } } } }
    };

    ExpectedNamespaceModel A{ "A", "A", {}, { S1 } };
    ExpectedNamespaceModel N{ "N", "N", {}, { D1 } };

    ExpectedNamespaceModel C{ "C", "B.C", {}, { E1 } };
    ExpectedNamespaceModel B{ "B", "B", { C }, { } };

    A.VerifyType(find_namespace(reader, "A"));
    N.VerifyType(find_namespace(reader, "N"));
    B.VerifyType(find_namespace(reader, "B"));
}

// Disabling and coming back later
// TODO: fix base problem once we have attributes to specify which interface becomes the base
/*
DefaultRyan 20 hours ago  Member
This case seems odd for two reasons :

I didn't think xlang was supporting multiple interface inheritance. And multiple "requires" isn't supported 
without some sort of attribute specifying which required interface becomes the base for inheritance purposes.
This makes IComboBox.Paint() ambiguous.This should require some sort of disambiguation on the method.
*/
TEST_CASE("Interface base test", "[!hide]")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            interface IControl
            {
                void Paint();
            }
            interface ITextBox requires IControl
            {
                void SetText(String text);
            }
            interface IListBox requires IControl
            {
               void SetItem(String items);
            }
            interface IComboBox requires ITextBox, IListBox {}
        }
    )" };

    xmeta_idl_reader reader{ "" };
    reader.read(test_idl);
    REQUIRE(reader.get_num_syntax_errors() == 0);

    auto const& namespaces = reader.get_namespaces();
    auto const& it = namespaces.find("N");
    REQUIRE(it != namespaces.end());
    auto const& ns_bodies = it->second->get_namespace_bodies();
    REQUIRE(ns_bodies.size() == 1);
    auto const& interfaces = ns_bodies[0]->get_interfaces();
    REQUIRE(interfaces.size() == 4);

    auto const& combo = interfaces.at("IComboBox");
    auto const& combo_bases = combo->get_all_interface_bases();
    REQUIRE(combo_bases.size() == 3);
    REQUIRE(combo_bases.find(interfaces.at("ITextBox")) != combo_bases.end());
    REQUIRE(combo_bases.find(interfaces.at("IListBox")) != combo_bases.end());
    REQUIRE(combo_bases.find(interfaces.at("IControl")) != combo_bases.end());
    REQUIRE(combo->get_all_interface_bases().size() == 3);
}

TEST_CASE("Interface methods test")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            interface IControl
            {
                void Paint();
                Int32 Draw(Int32 i, Int32 d);
            }
        }
    )" };

    xmeta_idl_reader reader{ "" };
    reader.read(test_idl);
    REQUIRE(reader.get_num_syntax_errors() == 0);

    ExpectedMethodModel Paint{ "Paint", default_method_semantics, std::nullopt, {} };
    ExpectedMethodModel Draw{ "Draw", default_method_semantics, ExpectedTypeRefModel{ simple_type::Int32 }, {
        ExpectedFormalParameterModel{ "i", parameter_semantics::in, ExpectedTypeRefModel{ simple_type::Int32 } },
        ExpectedFormalParameterModel{ "d", parameter_semantics::in, ExpectedTypeRefModel{ simple_type::Int32 } }
    } };

    ExpectedInterfaceModel IControl{ "IControl", "N.IControl", { Paint, Draw }, {}, {} };
    ExpectedNamespaceModel N{ "N", "N", {}, { IControl } };
    N.VerifyType(find_namespace(reader, "N"));
}

TEST_CASE("Resolving interface method type ref test")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            struct S1
            {
            };

            interface IControl
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

    auto const& namespaces = reader.get_namespaces();
    auto const& it = namespaces.find("N");
    REQUIRE(it != namespaces.end());
    auto const& ns_bodies = it->second->get_namespace_bodies();
    REQUIRE(ns_bodies.size() == 1);
    auto const& interfaces = ns_bodies[0]->get_interfaces();
    REQUIRE(interfaces.size() == 1);

    REQUIRE(interfaces.find("IControl") != interfaces.end());
    auto const& model = interfaces.at("IControl");

    auto const& methods = model->get_methods();
    REQUIRE(methods[0]->get_id() == "Draw");

    auto const& return_type = methods[0]->get_return_type()->get_semantic();
    REQUIRE(return_type.is_resolved());
    REQUIRE(std::get<std::shared_ptr<enum_model>>(return_type.get_resolved_target())->get_id() == "E1");

    auto const& params = methods[0]->get_formal_parameters();
    {
        REQUIRE(params[0].get_id() == "p1");
        auto const& param_type = params[0].get_type().get_semantic();
        REQUIRE(param_type.is_resolved());
        REQUIRE(std::get<std::shared_ptr<struct_model>>(param_type.get_resolved_target())->get_id() == "S1");
    }
    {
        REQUIRE(params[1].get_id() == "p2");
        auto const& param_type = params[1].get_type().get_semantic();
        REQUIRE(param_type.is_resolved());
        REQUIRE(std::get<std::shared_ptr<struct_model>>(param_type.get_resolved_target())->get_id() == "S2");
    }
}

TEST_CASE("Interface property method ordering test")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            interface IControl
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

    auto const& namespaces = reader.get_namespaces();
    auto const& it = namespaces.find("N");
    REQUIRE(it != namespaces.end());
    auto const& ns_bodies = it->second->get_namespace_bodies();
    REQUIRE(ns_bodies.size() == 1);
    auto const& interfaces = ns_bodies[0]->get_interfaces();
    REQUIRE(interfaces.size() == 1);

    REQUIRE(interfaces.find("IControl") != interfaces.end());
    auto const& model = interfaces.at("IControl");

    auto const& properties = model->get_properties();
    REQUIRE(properties[0]->get_id() == "property1");
    REQUIRE(properties[1]->get_id() == "property2");
    REQUIRE(properties[2]->get_id() == "property3");
    {
        auto const& property_type = properties[0]->get_type().get_semantic();
        REQUIRE((property_type.is_resolved() && std::get<simple_type>(property_type.get_resolved_target()) == simple_type::Int32));
    }
    {
        auto const& property_type = properties[1]->get_type().get_semantic();
        REQUIRE((property_type.is_resolved() && std::get<simple_type>(property_type.get_resolved_target()) == simple_type::Int32));
    }
    {
        auto const& property_type = properties[2]->get_type().get_semantic();
        REQUIRE((property_type.is_resolved() && std::get<simple_type>(property_type.get_resolved_target()) == simple_type::Int32));
    }
    auto const& methods = model->get_methods();
    REQUIRE(methods.size() == 5);

    auto const& method0 = methods[0];
    REQUIRE(method0->get_id() == "get_property1");
    REQUIRE(properties[0]->get_get_method() == method0);
    REQUIRE(properties[0]->get_get_method()->get_id() == method0->get_id());
    auto method0_return_type = method0->get_return_type()->get_semantic();
    REQUIRE((method0_return_type.is_resolved() && std::get<simple_type>(method0_return_type.get_resolved_target()) == simple_type::Int32));

    auto const& method1 = methods[1];
    REQUIRE(method1->get_id() == "put_property1");
    REQUIRE(properties[0]->get_set_method() == method1);
    REQUIRE(properties[0]->get_set_method()->get_id() == method1->get_id());
    REQUIRE(!method1->get_return_type());

    auto const& method2 = methods[2];
    REQUIRE(method2->get_id() == "get_property2");
    REQUIRE(properties[1]->get_get_method() == method2);
    REQUIRE(properties[1]->get_get_method()->get_id() == method2->get_id());
    auto method2_return_type = method2->get_return_type()->get_semantic();
    REQUIRE((method2_return_type.is_resolved() && std::get<simple_type>(method2_return_type.get_resolved_target()) == simple_type::Int32));

    auto const& method3 = methods[3];
    REQUIRE(method3->get_id() == "put_property3");
    REQUIRE(properties[2]->get_set_method() == method3);
    REQUIRE(properties[2]->get_set_method()->get_id() == method3->get_id());
    REQUIRE(!method3->get_return_type());

    auto const& method4 = methods[4];
    REQUIRE(method4->get_id() == "get_property3");
    REQUIRE(properties[2]->get_get_method() == method4);
    REQUIRE(properties[2]->get_get_method()->get_id() == method4->get_id());
    auto method4_return_type = method4->get_return_type()->get_semantic();
    REQUIRE((method4_return_type.is_resolved() && std::get<simple_type>(method4_return_type.get_resolved_target()) == simple_type::Int32));
}

TEST_CASE("Interface property method ordering different line test")
{
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                interface IControl
                {
                    Int32 property1 { get; };
                    Int32 property1 { set; };
                }
            }
        )" };

        xmeta_idl_reader reader{ "" };
        reader.read(test_idl);
        REQUIRE(reader.get_num_syntax_errors() == 0);

        auto const& namespaces = reader.get_namespaces();
        auto const& it = namespaces.find("N");
        REQUIRE(it != namespaces.end());
        auto const& ns_bodies = it->second->get_namespace_bodies();
        REQUIRE(ns_bodies.size() == 1);
        auto const& interfaces = ns_bodies[0]->get_interfaces();
        REQUIRE(interfaces.size() == 1);

        REQUIRE(interfaces.find("IControl") != interfaces.end());
        auto const& model = interfaces.at("IControl");

        auto const& properties = model->get_properties();
        REQUIRE(properties.size() == 1);
        REQUIRE(properties[0]->get_id() == "property1");
        {
            auto const& property_type = properties[0]->get_type().get_semantic();
            REQUIRE((property_type.is_resolved() && std::get<simple_type>(property_type.get_resolved_target()) == simple_type::Int32));
        }
        auto const& methods = model->get_methods();
        REQUIRE(methods.size() == 2);
        auto const& method0 = methods[0];
        REQUIRE(method0->get_id() == "get_property1");
        REQUIRE(properties[0]->get_get_method() == method0);
        REQUIRE(properties[0]->get_get_method()->get_id() == method0->get_id());
        auto method0_return_type = method0->get_return_type()->get_semantic();
        REQUIRE((method0_return_type.is_resolved() && std::get<simple_type>(method0_return_type.get_resolved_target()) == simple_type::Int32));

        auto const& method1 = methods[1];
        REQUIRE(method1->get_id() == "put_property1");
        REQUIRE(properties[0]->get_set_method() == method1);
        REQUIRE(properties[0]->get_set_method()->get_id() == method1->get_id());
        REQUIRE(!method1->get_return_type());
    }
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                interface IControl
                {
                    Int32 property1 { set; };
                    Int32 property1 { get; };
                }
            }
        )" };

        xmeta_idl_reader reader{ "" };
        reader.read(test_idl);
        REQUIRE(reader.get_num_syntax_errors() == 0);

        auto const& namespaces = reader.get_namespaces();
        auto const& it = namespaces.find("N");
        REQUIRE(it != namespaces.end());
        auto const& ns_bodies = it->second->get_namespace_bodies();
        REQUIRE(ns_bodies.size() == 1);
        auto const& interfaces = ns_bodies[0]->get_interfaces();
        REQUIRE(interfaces.size() == 1);

        REQUIRE(interfaces.find("IControl") != interfaces.end());
        auto const& model = interfaces.at("IControl");

        auto const& properties = model->get_properties();
        REQUIRE(properties.size() == 1);
        REQUIRE(properties[0]->get_id() == "property1");
        {
            auto const& property_type = properties[0]->get_type().get_semantic();
            REQUIRE((property_type.is_resolved() && std::get<simple_type>(property_type.get_resolved_target()) == simple_type::Int32));
        }
        auto const& methods = model->get_methods();
        REQUIRE(methods.size() == 2);
        auto const& method0 = methods[0];
        REQUIRE(method0->get_id() == "put_property1");
        REQUIRE(properties[0]->get_set_method() == method0);
        REQUIRE(properties[0]->get_set_method()->get_id() == method0->get_id());
        REQUIRE(!method0->get_return_type());

        auto const& method1 = methods[1];
        REQUIRE(method1->get_id() == "get_property1");
        REQUIRE(properties[0]->get_get_method() == method1);
        REQUIRE(properties[0]->get_get_method()->get_id() == method1->get_id());
        auto method1_return_type = method1->get_return_type()->get_semantic();
        REQUIRE((method1_return_type.is_resolved() && std::get<simple_type>(method1_return_type.get_resolved_target()) == simple_type::Int32));
    }
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                interface IControl
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
        auto const& namespaces = reader.get_namespaces();
        auto const& it = namespaces.find("N");
        auto const& ns_bodies = it->second->get_namespace_bodies();
        auto const& interfaces = ns_bodies[0]->get_interfaces();
        REQUIRE(interfaces.find("IControl") != interfaces.end());
        auto const& model = interfaces.at("IControl");
        auto const& methods = model->get_methods();
        REQUIRE(methods.size() == 5);
        REQUIRE(methods[0]->get_id() == "get_property1");
        REQUIRE(methods[1]->get_id() == "put_property2");
        REQUIRE(methods[2]->get_id() == "draw");
        REQUIRE(methods[3]->get_id() == "put_property1");
        REQUIRE(methods[4]->get_id() == "get_property2");
    }
}

TEST_CASE("Interface property method name collision test")
{
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                interface IControl
                {
                    void get_property1();
                    Int32 property1 { get; set; };
                    void put_property1();
                }
            }
        )" };

        xmeta_idl_reader reader{ "" };
        reader.read(test_idl);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 2);
    }
}

TEST_CASE("Interface invalid property accessor test")
{
    {
        std::istringstream test_set_only_idl{ R"(
            namespace N
            {
                interface IControl
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
    // multiple getters
    {
        std::istringstream test_double_get_idl{ R"(
            namespace N
            {
                interface IControl
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
                interface IControl
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
    // multiple setters
    {
        std::istringstream test_double_set_idl{ R"(
            namespace N
            {
                interface IControl
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
                interface IControl
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
        std::istringstream test_three_acessor_idl{ R"(
            namespace N
            {
                interface IControl
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
        std::istringstream test_add_and_remove_idl{ R"(
            namespace N
            {
                interface IControl
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

TEST_CASE("Interface duplicate property id test")
{
    {
        std::istringstream test_set_only_idl{ R"(
            namespace N
            {
                interface IControl
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

TEST_CASE("Interface property implicit accessors test")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            interface IControl
            {
                Int32 property1;
            }
        }
    )" };

    xmeta_idl_reader reader{ "" };
    reader.read(test_idl);
    REQUIRE(reader.get_num_syntax_errors() == 0);

    auto const& namespaces = reader.get_namespaces();
    auto const& it = namespaces.find("N");
    REQUIRE(it != namespaces.end());
    auto const& ns_bodies = it->second->get_namespace_bodies();
    REQUIRE(ns_bodies.size() == 1);
    auto const& interfaces = ns_bodies[0]->get_interfaces();
    REQUIRE(interfaces.size() == 1);

    REQUIRE(interfaces.find("IControl") != interfaces.end());
    auto const& model = interfaces.at("IControl");

    auto const& properties = model->get_properties();
    REQUIRE(properties[0]->get_id() == "property1");

    auto const& property_type = properties[0]->get_type().get_semantic();
    REQUIRE((property_type.is_resolved() && std::get<simple_type>(property_type.get_resolved_target()) == simple_type::Int32));

    auto const& methods = model->get_methods();
    REQUIRE(methods.size() == 2);

    auto const& method0 = methods[0];
    REQUIRE(method0->get_id() == "get_property1");
    auto method0_return_type = method0->get_return_type()->get_semantic();
    REQUIRE((method0_return_type.is_resolved() && std::get<simple_type>(method0_return_type.get_resolved_target()) == simple_type::Int32));

    auto const& method1 = methods[1];
    REQUIRE(method1->get_id() == "put_property1");
    REQUIRE(!method1->get_return_type());
}

TEST_CASE("Resolving Interface property type ref test")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            struct S1
            {
            };

            interface IControl
            {
                S1 property1;
                M.S2 property2;
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

    auto const& namespaces = reader.get_namespaces();
    auto const& it = namespaces.find("N");
    REQUIRE(it != namespaces.end());
    auto const& ns_bodies = it->second->get_namespace_bodies();
    REQUIRE(ns_bodies.size() == 1);
    auto const& interfaces = ns_bodies[0]->get_interfaces();
    REQUIRE(interfaces.size() == 1);

    REQUIRE(interfaces.find("IControl") != interfaces.end());
    auto const& model = interfaces.at("IControl");

    auto const& properties = model->get_properties();
    {
        REQUIRE(properties[0]->get_id() == "property1");
        auto const& property_type = properties[0]->get_type().get_semantic();
        REQUIRE(property_type.is_resolved());
        REQUIRE(std::get<std::shared_ptr<struct_model>>(property_type.get_resolved_target())->get_id() == "S1");
    }
    {
        REQUIRE(properties[1]->get_id() == "property2");
        auto const& property_type = properties[1]->get_type().get_semantic();
        REQUIRE(property_type.is_resolved());
        REQUIRE(std::get<std::shared_ptr<struct_model>>(property_type.get_resolved_target())->get_id() == "S2");
    }
}

TEST_CASE("Interface event test")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            delegate void StringListEvent(Int32 sender);
            interface IControl
            {
                event StringListEvent Changed;
            }
        }
    )" };
    std::vector<std::string> paths = { "Foundation.xmeta" };
    xmeta_idl_reader reader{ "" , paths};
    reader.read(test_idl, true);
    REQUIRE(reader.get_num_syntax_errors() == 0);

    auto const& namespaces = reader.get_namespaces();
    auto const& it = namespaces.find("N");
    REQUIRE(it != namespaces.end());
    auto const& ns_bodies = it->second->get_namespace_bodies();
    REQUIRE(ns_bodies.size() == 1);
    auto const& interfaces = ns_bodies[0]->get_interfaces();
    REQUIRE(interfaces.size() == 1);

    REQUIRE(interfaces.find("IControl") != interfaces.end());
    auto const& model = interfaces.at("IControl");

    auto const& events = model->get_events();
    REQUIRE(events[0]->get_id() == "Changed");

    auto const& property_type = events[0]->get_type().get_semantic();
    REQUIRE(property_type.is_resolved());
    REQUIRE(std::holds_alternative<std::shared_ptr<delegate_model>>(property_type.get_resolved_target()));
    REQUIRE(std::get<std::shared_ptr<delegate_model>>(property_type.get_resolved_target())->get_id() == "StringListEvent");
}

TEST_CASE("Interface event explicit accessor not allowed test")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            delegate void StringListEvent(Int32 sender);
            interface IControl
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

TEST_CASE("Interface duplicate event test")
{
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                delegate void StringListEvent(Int32 sender);
                interface IControl
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
        REQUIRE(reader.get_num_semantic_errors() == 1);
    }
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                delegate void StringListEvent(Int32 sender);
                interface IControl
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
        REQUIRE(reader.get_num_semantic_errors() == 1);
    }
}

TEST_CASE("Interface event and property name collision test")
{
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                delegate void StringListEvent(Int32 sender);
                interface IControl
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

TEST_CASE("Interface event and method name test")
{
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                delegate void StringListEvent(Int32 sender);
                interface IControl
                {
                    void remove_Changed();
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
}

TEST_CASE("Interface circular inheritance test")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            interface IControl requires IListBox
            {
                void Paint();
            }
            interface ITextBox requires IControl
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
                interface IControl requires IListBox
                {
                    void Paint();
                }
                interface IListBox requires ITextBox
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
                interface IControl requires IListBox
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
                interface IControl requires IListBox
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
                interface IControl requires IListBox
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
                interface IControl requires IListBox
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
                interface IControl requires IListBox
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
                interface IControl requires IListBox
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
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                interface IControl requires fakebase
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
}

TEST_CASE("Class methods test")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            runtimeclass c1
            {
                static void m0();
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
    auto const& ns_body = find_namespace_body(reader, "N", 0);
    auto const& classes = ns_body->get_classes();
    REQUIRE(classes.size() == 1);

    REQUIRE(classes.find("c1") != classes.end());
    auto const& model = classes.at("c1");

    auto const& methods = model->get_methods();
    REQUIRE(methods.size() == 4); 
    ExpectedMethodModel m0{ "m0", in_method_semantics, std::nullopt, {} };
    ExpectedMethodModel m1{ "m1", default_method_semantics, ExpectedTypeRefModel{ simple_type::Int32 }, { 
        ExpectedFormalParameterModel{ "s", parameter_semantics::in, ExpectedTypeRefModel{ simple_type::String } } } };
    m0.VerifyType(methods[0]);
    m1.VerifyType(methods[1]);

    auto const& method2 = methods[2];
    REQUIRE(method2->get_id() == "m2");
    REQUIRE(method2->get_return_type()->get_semantic().is_resolved());
    REQUIRE(std::holds_alternative<std::shared_ptr<struct_model>>(method2->get_return_type()->get_semantic().get_resolved_target()));
    REQUIRE(std::get<std::shared_ptr<struct_model>>(method2->get_return_type()->get_semantic().get_resolved_target())->get_fully_qualified_id() == "N.S1");

    auto const& method3 = methods[3];
    REQUIRE(method3->get_id() == "m3");
    REQUIRE(method3->get_return_type()->get_semantic().is_resolved());
    REQUIRE(std::holds_alternative<std::shared_ptr<struct_model>>(method3->get_return_type()->get_semantic().get_resolved_target()));
    REQUIRE(std::get<std::shared_ptr<struct_model>>(method3->get_return_type()->get_semantic().get_resolved_target())->get_fully_qualified_id() == "M.S1");
}

TEST_CASE("Class methods synthesized test")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            runtimeclass c1
            {
                static void m1();
                Int32 m2(String s);
                S1 m3();
                M.S1 m4();
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
}

/*
TEST_CASE("Interface property method ordering test")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            interface IControl
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

    auto const& namespaces = reader.get_namespaces();
    auto const& it = namespaces.find("N");
    REQUIRE(it != namespaces.end());
    auto const& ns_bodies = it->second->get_namespace_bodies();
    REQUIRE(ns_bodies.size() == 1);
    auto const& interfaces = ns_bodies[0]->get_interfaces();
    REQUIRE(interfaces.size() == 1);

    REQUIRE(interfaces.find("IControl") != interfaces.end());
    auto const& model = interfaces.at("IControl");

    auto const& properties = model->get_properties();
    REQUIRE(properties[0]->get_id() == "property1");
    REQUIRE(properties[1]->get_id() == "property2");
    REQUIRE(properties[2]->get_id() == "property3");
    {
        auto const& property_type = properties[0]->get_type().get_semantic();
        REQUIRE((property_type.is_resolved() && std::get<simple_type>(property_type.get_resolved_target()) == simple_type::Int32));
    }
    {
        auto const& property_type = properties[1]->get_type().get_semantic();
        REQUIRE((property_type.is_resolved() && std::get<simple_type>(property_type.get_resolved_target()) == simple_type::Int32));
    }
    {
        auto const& property_type = properties[2]->get_type().get_semantic();
        REQUIRE((property_type.is_resolved() && std::get<simple_type>(property_type.get_resolved_target()) == simple_type::Int32));
    }
    auto const& methods = model->get_methods();
    REQUIRE(methods.size() == 5);

    auto const& method0 = methods[0];
    REQUIRE(method0->get_id() == "get_property1");
    REQUIRE(properties[0]->get_get_method() == method0);
    REQUIRE(properties[0]->get_get_method()->get_id() == method0->get_id());
    auto method0_return_type = method0->get_return_type()->get_semantic();
    REQUIRE((method0_return_type.is_resolved() && std::get<simple_type>(method0_return_type.get_resolved_target()) == simple_type::Int32));

    auto const& method1 = methods[1];
    REQUIRE(method1->get_id() == "put_property1");
    REQUIRE(properties[0]->get_set_method() == method1);
    REQUIRE(properties[0]->get_set_method()->get_id() == method1->get_id());
    REQUIRE(!method1->get_return_type());

    auto const& method2 = methods[2];
    REQUIRE(method2->get_id() == "get_property2");
    REQUIRE(properties[1]->get_get_method() == method2);
    REQUIRE(properties[1]->get_get_method()->get_id() == method2->get_id());
    auto method2_return_type = method2->get_return_type()->get_semantic();
    REQUIRE((method2_return_type.is_resolved() && std::get<simple_type>(method2_return_type.get_resolved_target()) == simple_type::Int32));

    auto const& method3 = methods[3];
    REQUIRE(method3->get_id() == "put_property3");
    REQUIRE(properties[2]->get_set_method() == method3);
    REQUIRE(properties[2]->get_set_method()->get_id() == method3->get_id());
    REQUIRE(!method3->get_return_type());

    auto const& method4 = methods[4];
    REQUIRE(method4->get_id() == "get_property3");
    REQUIRE(properties[2]->get_get_method() == method4);
    REQUIRE(properties[2]->get_get_method()->get_id() == method4->get_id());
    auto method4_return_type = method4->get_return_type()->get_semantic();
    REQUIRE((method4_return_type.is_resolved() && std::get<simple_type>(method4_return_type.get_resolved_target()) == simple_type::Int32));
}

TEST_CASE("Interface property method ordering different line test")
{
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                interface IControl
                {
                    Int32 property1 { get; };
                    Int32 property1 { set; };
                }
            }
        )" };

        xmeta_idl_reader reader{ "" };
        reader.read(test_idl);
        REQUIRE(reader.get_num_syntax_errors() == 0);

        auto const& namespaces = reader.get_namespaces();
        auto const& it = namespaces.find("N");
        REQUIRE(it != namespaces.end());
        auto const& ns_bodies = it->second->get_namespace_bodies();
        REQUIRE(ns_bodies.size() == 1);
        auto const& interfaces = ns_bodies[0]->get_interfaces();
        REQUIRE(interfaces.size() == 1);

        REQUIRE(interfaces.find("IControl") != interfaces.end());
        auto const& model = interfaces.at("IControl");

        auto const& properties = model->get_properties();
        REQUIRE(properties.size() == 1);
        REQUIRE(properties[0]->get_id() == "property1");
        {
            auto const& property_type = properties[0]->get_type().get_semantic();
            REQUIRE((property_type.is_resolved() && std::get<simple_type>(property_type.get_resolved_target()) == simple_type::Int32));
        }
        auto const& methods = model->get_methods();
        REQUIRE(methods.size() == 2);
        auto const& method0 = methods[0];
        REQUIRE(method0->get_id() == "get_property1");
        REQUIRE(properties[0]->get_get_method() == method0);
        REQUIRE(properties[0]->get_get_method()->get_id() == method0->get_id());
        auto method0_return_type = method0->get_return_type()->get_semantic();
        REQUIRE((method0_return_type.is_resolved() && std::get<simple_type>(method0_return_type.get_resolved_target()) == simple_type::Int32));

        auto const& method1 = methods[1];
        REQUIRE(method1->get_id() == "put_property1");
        REQUIRE(properties[0]->get_set_method() == method1);
        REQUIRE(properties[0]->get_set_method()->get_id() == method1->get_id());
        REQUIRE(!method1->get_return_type());
    }
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                interface IControl
                {
                    Int32 property1 { set; };
                    Int32 property1 { get; };
                }
            }
        )" };

        xmeta_idl_reader reader{ "" };
        reader.read(test_idl);
        REQUIRE(reader.get_num_syntax_errors() == 0);

        auto const& namespaces = reader.get_namespaces();
        auto const& it = namespaces.find("N");
        REQUIRE(it != namespaces.end());
        auto const& ns_bodies = it->second->get_namespace_bodies();
        REQUIRE(ns_bodies.size() == 1);
        auto const& interfaces = ns_bodies[0]->get_interfaces();
        REQUIRE(interfaces.size() == 1);

        REQUIRE(interfaces.find("IControl") != interfaces.end());
        auto const& model = interfaces.at("IControl");

        auto const& properties = model->get_properties();
        REQUIRE(properties.size() == 1);
        REQUIRE(properties[0]->get_id() == "property1");
        {
            auto const& property_type = properties[0]->get_type().get_semantic();
            REQUIRE((property_type.is_resolved() && std::get<simple_type>(property_type.get_resolved_target()) == simple_type::Int32));
        }
        auto const& methods = model->get_methods();
        REQUIRE(methods.size() == 2);
        auto const& method0 = methods[0];
        REQUIRE(method0->get_id() == "put_property1");
        REQUIRE(properties[0]->get_set_method() == method0);
        REQUIRE(properties[0]->get_set_method()->get_id() == method0->get_id());
        REQUIRE(!method0->get_return_type());

        auto const& method1 = methods[1];
        REQUIRE(method1->get_id() == "get_property1");
        REQUIRE(properties[0]->get_get_method() == method1);
        REQUIRE(properties[0]->get_get_method()->get_id() == method1->get_id());
        auto method1_return_type = method1->get_return_type()->get_semantic();
        REQUIRE((method1_return_type.is_resolved() && std::get<simple_type>(method1_return_type.get_resolved_target()) == simple_type::Int32));
    }
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                interface IControl
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
        auto const& namespaces = reader.get_namespaces();
        auto const& it = namespaces.find("N");
        auto const& ns_bodies = it->second->get_namespace_bodies();
        auto const& interfaces = ns_bodies[0]->get_interfaces();
        REQUIRE(interfaces.find("IControl") != interfaces.end());
        auto const& model = interfaces.at("IControl");
        auto const& methods = model->get_methods();
        REQUIRE(methods.size() == 5);
        REQUIRE(methods[0]->get_id() == "get_property1");
        REQUIRE(methods[1]->get_id() == "put_property2");
        REQUIRE(methods[2]->get_id() == "draw");
        REQUIRE(methods[3]->get_id() == "put_property1");
        REQUIRE(methods[4]->get_id() == "get_property2");
    }
}

TEST_CASE("Interface property method name collision test")
{
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                interface IControl
                {
                    void get_property1();
                    Int32 property1 { get; set; };
                    void put_property1();
                }
            }
        )" };

        xmeta_idl_reader reader{ "" };
        reader.read(test_idl);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 2);
    }
}

TEST_CASE("Interface invalid property accessor test")
{
    {
        std::istringstream test_set_only_idl{ R"(
            namespace N
            {
                interface IControl
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
    // multiple getters
    {
        std::istringstream test_double_get_idl{ R"(
            namespace N
            {
                interface IControl
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
                interface IControl
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
    // multiple setters
    {
        std::istringstream test_double_set_idl{ R"(
            namespace N
            {
                interface IControl
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
                interface IControl
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
        std::istringstream test_three_acessor_idl{ R"(
            namespace N
            {
                interface IControl
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
        std::istringstream test_add_and_remove_idl{ R"(
            namespace N
            {
                interface IControl
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

TEST_CASE("Interface duplicate property id test")
{
    {
        std::istringstream test_set_only_idl{ R"(
            namespace N
            {
                interface IControl
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

TEST_CASE("Interface property implicit accessors test")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            interface IControl
            {
                Int32 property1;
            }
        }
    )" };

    xmeta_idl_reader reader{ "" };
    reader.read(test_idl);
    REQUIRE(reader.get_num_syntax_errors() == 0);

    auto const& namespaces = reader.get_namespaces();
    auto const& it = namespaces.find("N");
    REQUIRE(it != namespaces.end());
    auto const& ns_bodies = it->second->get_namespace_bodies();
    REQUIRE(ns_bodies.size() == 1);
    auto const& interfaces = ns_bodies[0]->get_interfaces();
    REQUIRE(interfaces.size() == 1);

    REQUIRE(interfaces.find("IControl") != interfaces.end());
    auto const& model = interfaces.at("IControl");

    auto const& properties = model->get_properties();
    REQUIRE(properties[0]->get_id() == "property1");

    auto const& property_type = properties[0]->get_type().get_semantic();
    REQUIRE((property_type.is_resolved() && std::get<simple_type>(property_type.get_resolved_target()) == simple_type::Int32));

    auto const& methods = model->get_methods();
    REQUIRE(methods.size() == 2);

    auto const& method0 = methods[0];
    REQUIRE(method0->get_id() == "get_property1");
    auto method0_return_type = method0->get_return_type()->get_semantic();
    REQUIRE((method0_return_type.is_resolved() && std::get<simple_type>(method0_return_type.get_resolved_target()) == simple_type::Int32));

    auto const& method1 = methods[1];
    REQUIRE(method1->get_id() == "put_property1");
    REQUIRE(!method1->get_return_type());
}

TEST_CASE("Resolving Interface property type ref test")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            struct S1
            {
            };

            interface IControl
            {
                S1 property1;
                M.S2 property2;
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

    auto const& namespaces = reader.get_namespaces();
    auto const& it = namespaces.find("N");
    REQUIRE(it != namespaces.end());
    auto const& ns_bodies = it->second->get_namespace_bodies();
    REQUIRE(ns_bodies.size() == 1);
    auto const& interfaces = ns_bodies[0]->get_interfaces();
    REQUIRE(interfaces.size() == 1);

    REQUIRE(interfaces.find("IControl") != interfaces.end());
    auto const& model = interfaces.at("IControl");

    auto const& properties = model->get_properties();
    {
        REQUIRE(properties[0]->get_id() == "property1");
        auto const& property_type = properties[0]->get_type().get_semantic();
        REQUIRE(property_type.is_resolved());
        REQUIRE(std::get<std::shared_ptr<struct_model>>(property_type.get_resolved_target())->get_id() == "S1");
    }
    {
        REQUIRE(properties[1]->get_id() == "property2");
        auto const& property_type = properties[1]->get_type().get_semantic();
        REQUIRE(property_type.is_resolved());
        REQUIRE(std::get<std::shared_ptr<struct_model>>(property_type.get_resolved_target())->get_id() == "S2");
    }
}

TEST_CASE("Interface event test")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            delegate void StringListEvent(Int32 sender);
            interface IControl
            {
                event StringListEvent Changed;
            }
        }
    )" };
    std::vector<std::string> paths = { "Foundation.xmeta" };
    xmeta_idl_reader reader{ "" , paths };
    reader.read(test_idl, true);
    REQUIRE(reader.get_num_syntax_errors() == 0);

    auto const& namespaces = reader.get_namespaces();
    auto const& it = namespaces.find("N");
    REQUIRE(it != namespaces.end());
    auto const& ns_bodies = it->second->get_namespace_bodies();
    REQUIRE(ns_bodies.size() == 1);
    auto const& interfaces = ns_bodies[0]->get_interfaces();
    REQUIRE(interfaces.size() == 1);

    REQUIRE(interfaces.find("IControl") != interfaces.end());
    auto const& model = interfaces.at("IControl");

    auto const& events = model->get_events();
    REQUIRE(events[0]->get_id() == "Changed");

    auto const& property_type = events[0]->get_type().get_semantic();
    REQUIRE(property_type.is_resolved());
    REQUIRE(std::holds_alternative<std::shared_ptr<delegate_model>>(property_type.get_resolved_target()));
    REQUIRE(std::get<std::shared_ptr<delegate_model>>(property_type.get_resolved_target())->get_id() == "StringListEvent");
}

TEST_CASE("Interface event explicit accessor not allowed test")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            delegate void StringListEvent(Int32 sender);
            interface IControl
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

TEST_CASE("Interface duplicate event test")
{
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                delegate void StringListEvent(Int32 sender);
                interface IControl
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
        REQUIRE(reader.get_num_semantic_errors() == 1);
    }
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                delegate void StringListEvent(Int32 sender);
                interface IControl
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
        REQUIRE(reader.get_num_semantic_errors() == 1);
    }
}

TEST_CASE("Interface event and property name collision test")
{
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                delegate void StringListEvent(Int32 sender);
                interface IControl
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

TEST_CASE("Interface event and method name test")
{
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                delegate void StringListEvent(Int32 sender);
                interface IControl
                {
                    void remove_Changed();
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
}

TEST_CASE("Interface circular inheritance test")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            interface IControl requires IListBox
            {
                void Paint();
            }
            interface ITextBox requires IControl
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
                interface IControl requires IListBox
                {
                    void Paint();
                }
                interface IListBox requires ITextBox
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
                interface IControl requires IListBox
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
                interface IControl requires IListBox
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
                interface IControl requires IListBox
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
                interface IControl requires IListBox
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
                interface IControl requires IListBox
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
                interface IControl requires IListBox
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
    {
        std::istringstream test_idl{ R"(
            namespace N
            {
                interface IControl requires fakebase
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
}
*/
