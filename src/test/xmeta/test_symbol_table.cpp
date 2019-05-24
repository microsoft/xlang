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

struct ExpectedFormalParameterModel
{
    std::string id;
    parameter_semantics sem;
    type_ref type;
    ExpectedFormalParameterModel(std::string const& name, parameter_semantics const& sem, type_ref const& type)
        : id{ name }, sem{ sem }, type{ type } {}

    void VerifyType(std::shared_ptr<formal_parameter_model> const& actual)
    {
        REQUIRE(actual->get_id() == id);
        REQUIRE(actual->get_semantic() == sem);
        REQUIRE(actual->get_type() == type);
    }

    void VerifyType(formal_parameter_model const& actual)
    {
        REQUIRE(actual.get_id() == id);
        REQUIRE(actual.get_semantic() == sem);
        REQUIRE(actual.get_type() == type);
    }
};

struct ExpectedMethodModel
{
    std::string id;
    method_semantics sem;
    std::optional<type_ref> return_type;
    std::vector<ExpectedFormalParameterModel> params;

    ExpectedMethodModel(std::string const& name, method_semantics const& sem, std::optional<type_ref> const& return_type, std::vector<ExpectedFormalParameterModel> params)
        : id{ name }, sem{ sem }, return_type{ return_type }, params{ params } {}

    void VerifyType(std::shared_ptr<method_model> const& actual)
    {
        REQUIRE(actual->get_id() == id);
        REQUIRE(actual->get_semantic().is_static == sem.is_static);
        REQUIRE(actual->get_return_type() == return_type);

        auto const& actual_params = actual->get_formal_parameters();
        REQUIRE(actual_params.size() == params.size());
        for (size_t i = 0; i < params.size(); i++)
        {
            params[i].VerifyType(actual_params[i]);
        }
    }
};

struct ExepectedStructModel
{
    std::string fully_qualified_id;
    std::vector<std::pair<type_ref, std::string>> fields;

    ExepectedStructModel(std::string const& name, std::vector<std::pair<type_ref, std::string>> fields)
        : fully_qualified_id{ name }, fields{ fields } {}

    void VerifyType(std::shared_ptr<struct_model> const& actual)
    {
        REQUIRE(actual->get_fully_qualified_id() == fully_qualified_id);

        auto const& actual_fields = actual->get_fields();
        REQUIRE(actual_fields.size() == fields.size());
        for (size_t i = 0; i < fields.size(); i++)
        {
            REQUIRE(actual_fields.at(i).first == fields.at(i).first);
            REQUIRE(actual_fields.at(i).second == fields.at(i).second);
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

    auto ns = find_namespace(reader, "N");
    REQUIRE(ns->get_namespace_bodies().size() == 1);
    auto ns_body = ns->get_namespace_bodies()[0];

    auto enums = ns_body->get_enums();
    REQUIRE(enums.find("E") != enums.end());
    auto const& enum_members = enums["E"]->get_members();
    REQUIRE(enum_members.size() == 5);
    REQUIRE(enum_members[0].get_id() == "e_member_1");
    REQUIRE(enum_members[1].get_id() == "e_member_2");
    REQUIRE(enum_members[2].get_id() == "e_member_3");
    REQUIRE(enum_members[3].get_id() == "e_member_4");
    REQUIRE(enum_members[4].get_id() == "e_member_5");
    auto const& val1 = enum_members[0].get_value();
    auto const& val2 = enum_members[1].get_value();
    auto const& val3 = enum_members[2].get_value();
    auto const& val4 = enum_members[3].get_value();
    auto const& val5 = enum_members[4].get_value();
    REQUIRE((val1.is_resolved() && std::get<int32_t>(val1.get_resolved_target()) == 0));
    REQUIRE((val2.is_resolved() && std::get<int32_t>(val2.get_resolved_target()) == 3));
    REQUIRE((val3.is_resolved() && std::get<int32_t>(val3.get_resolved_target()) == 4));
    REQUIRE((val4.is_resolved() && std::get<int32_t>(val4.get_resolved_target()) == 0x21));
    REQUIRE((val5.is_resolved() && std::get<int32_t>(val5.get_resolved_target()) == 0x21));
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

    auto namespaces = reader.get_namespaces();
    auto it = namespaces.find("N");
    REQUIRE(it != namespaces.end());
    auto ns_bodies = it->second->get_namespace_bodies();
    REQUIRE(ns_bodies.size() == 1);
    auto delegates = ns_bodies[0]->get_delegates();
    REQUIRE(delegates.size() == 2);

    REQUIRE(delegates.find("D1") != delegates.end());
    REQUIRE(delegates.at("D1")->get_return_type() != std::nullopt);
    auto del0_return_type = delegates.at("D1")->get_return_type()->get_semantic();
    REQUIRE((del0_return_type.is_resolved() && std::get<simple_type>(del0_return_type.get_resolved_target()) == simple_type::Int32));
    auto del0_formal_params = delegates.at("D1")->get_formal_parameters();
    REQUIRE(del0_formal_params.size() == 3);
    auto del0_formal_param_0_type = del0_formal_params[0].get_type().get_semantic();
    auto del0_formal_param_1_type = del0_formal_params[1].get_type().get_semantic();
    auto del0_formal_param_2_type = del0_formal_params[2].get_type().get_semantic();
    REQUIRE(del0_formal_params[0].get_id() == "i");
    REQUIRE(del0_formal_params[1].get_id() == "d");
    REQUIRE(del0_formal_params[2].get_id() == "e");
    REQUIRE((del0_formal_param_0_type.is_resolved() && std::get<simple_type>(del0_formal_param_0_type.get_resolved_target()) == simple_type::Int32));
    REQUIRE((del0_formal_param_1_type.is_resolved() && std::get<simple_type>(del0_formal_param_1_type.get_resolved_target()) == simple_type::Double));
    REQUIRE(del0_formal_param_2_type.is_resolved());
    REQUIRE(std::get<std::shared_ptr<enum_model>>(del0_formal_param_2_type.get_resolved_target())->get_id() == "E");
    REQUIRE(delegates.find("D2") != delegates.end());
    REQUIRE(delegates.at("D2")->get_return_type() == std::nullopt);
    REQUIRE(delegates.at("D2")->get_formal_parameters().empty());
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

    ExepectedStructModel expected{ "N.S" , { 
        { type_ref{ simple_type::Boolean } , "field_1" },
        { type_ref{ simple_type::String } , "field_2" },
        { type_ref{ simple_type::Int16 } , "field_3" },
        { type_ref{ simple_type::Int32 } , "field_4" },
        { type_ref{ simple_type::Int64 } , "field_5" },
        { type_ref{ simple_type::UInt8 } , "field_6" },
        { type_ref{ simple_type::UInt16 } , "field_7" },
        { type_ref{ simple_type::UInt32 } , "field_8" },
        { type_ref{ simple_type::Char16 } , "field_9" },
        { type_ref{ simple_type::Single } , "field_10" },
        { type_ref{ simple_type::Double } , "field_11" }
    } };

    auto namespaces = reader.get_namespaces();
    auto it = namespaces.find("N");
    REQUIRE(it != namespaces.end());
    auto ns = it->second;
    REQUIRE(ns->get_namespace_bodies().size() == 1);
    auto ns_body = ns->get_namespace_bodies()[0];

    auto structs = ns_body->get_structs();
    REQUIRE(structs.size() == 1);
    REQUIRE(structs.find("S") != structs.end());
    auto struct1 = structs.at("S");
    expected.VerifyType(struct1);
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
    auto namespaces = reader.get_namespaces();
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

    auto namespaces = reader.get_namespaces();
    auto it = namespaces.find("N");
    REQUIRE(it != namespaces.end());
    auto ns = it->second;
    REQUIRE(ns->get_namespace_bodies().size() == 1);
    auto ns_body = ns->get_namespace_bodies()[0];

    auto delegates = ns_body->get_delegates();
    REQUIRE(delegates.size() == 1);
    REQUIRE(delegates.find("D1") != delegates.end());
    auto delegate1 = delegates.at("D1");
    auto return_semantic = delegate1->get_return_type()->get_semantic();
    REQUIRE(return_semantic.is_resolved());
    auto resolved_return_semantic = return_semantic.get_resolved_target();
    REQUIRE(std::holds_alternative<std::shared_ptr<enum_model>>(resolved_return_semantic));
    REQUIRE(std::get<std::shared_ptr<enum_model>>(resolved_return_semantic)->get_id() == "E1");

    {
        auto param = delegate1->get_formal_parameters()[0];
        param.get_type().get_semantic();
        REQUIRE(param.get_id() == "param1");
        auto type = param.get_type().get_semantic();
        REQUIRE(type.is_resolved());
        auto target = type.get_resolved_target();
        REQUIRE(std::holds_alternative<std::shared_ptr<struct_model>>(target));
        REQUIRE(std::get<std::shared_ptr<struct_model>>(target)->get_id() == "S1");
    }
    {
        auto param = delegate1->get_formal_parameters()[1];
        param.get_type().get_semantic();
        REQUIRE(param.get_id() == "param2");
        auto type = param.get_type().get_semantic();
        REQUIRE(type.is_resolved());
        auto target = type.get_resolved_target();
        REQUIRE(std::holds_alternative<std::shared_ptr<enum_model>>(target));
        REQUIRE(std::get<std::shared_ptr<enum_model>>(target)->get_id() == "E1");
    }
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

    auto namespaces = reader.get_namespaces();
    auto it = namespaces.find("N");
    REQUIRE(it != namespaces.end());
    auto ns = it->second;
    REQUIRE(ns->get_namespace_bodies().size() == 1);
    auto ns_body = ns->get_namespace_bodies()[0];

    auto structs = ns_body->get_structs();
    REQUIRE(structs.find("S2") != structs.end());
    auto struct2 = structs.at("S2");
    auto fields = struct2->get_fields();
    REQUIRE(fields.size() == 2);
    {
        REQUIRE(fields[0].second == "field_1");
        auto type = fields[0].first.get_semantic();
        REQUIRE(type.is_resolved());
        auto target = type.get_resolved_target();
        REQUIRE(std::holds_alternative<std::shared_ptr<struct_model>>(target));
        REQUIRE(std::get<std::shared_ptr<struct_model>>(target)->get_id() == "S1");
    }
    {
        REQUIRE(fields[1].second == "field_2");
        auto type = fields[1].first.get_semantic();
        REQUIRE(type.is_resolved());
        auto target = type.get_resolved_target();
        REQUIRE(std::holds_alternative<std::shared_ptr<enum_model>>(target));
        REQUIRE(std::get<std::shared_ptr<enum_model>>(target)->get_id() == "E1");

    }
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

    auto namespaces = reader.get_namespaces();
    auto it = namespaces.find("N");
    REQUIRE(it != namespaces.end());
    auto ns = it->second;
    REQUIRE(ns->get_namespace_bodies().size() == 1);
    auto ns_body = ns->get_namespace_bodies()[0];

    auto delegates = ns_body->get_delegates();
    REQUIRE(delegates.size() == 1);
    REQUIRE(delegates.find("D1") != delegates.end());
    auto delegate1 = delegates.at("D1");
    {
        auto param = delegate1->get_formal_parameters()[0];
        param.get_type().get_semantic();
        REQUIRE(param.get_id() == "param1");
        auto type = param.get_type().get_semantic();
        REQUIRE(type.is_resolved());
        auto target = type.get_resolved_target();
        REQUIRE(std::holds_alternative<std::shared_ptr<struct_model>>(target));
        REQUIRE(std::get<std::shared_ptr<struct_model>>(target)->get_id() == "S1");
    }
    {
        auto param = delegate1->get_formal_parameters()[1];
        param.get_type().get_semantic();
        REQUIRE(param.get_id() == "param2");
        auto type = param.get_type().get_semantic();
        REQUIRE(type.is_resolved());
        auto target = type.get_resolved_target();
        REQUIRE(std::holds_alternative<std::shared_ptr<enum_model>>(target));
        REQUIRE(std::get<std::shared_ptr<enum_model>>(target)->get_id() == "E1");
    }
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
    REQUIRE(methods.size() == 2);

    auto const& method0 = methods[0];
    REQUIRE(method0->get_id() == "Paint");
    REQUIRE(method0->get_return_type() == std::nullopt);
    REQUIRE(method0->get_formal_parameters().size() == 0);

    auto const& method1 = methods[1];
    REQUIRE(method1->get_id() == "Draw");
    auto const& method1_return_type = method1->get_return_type()->get_semantic();
    REQUIRE((method1_return_type.is_resolved() && std::get<simple_type>(method1_return_type.get_resolved_target()) == simple_type::Int32));
    auto const& method1_formal_parameters = method1->get_formal_parameters();
    REQUIRE(method1_formal_parameters.size() == 2);
    REQUIRE(method1_formal_parameters[0].get_id() == "i");
    REQUIRE(std::get<simple_type>(method1_formal_parameters[0].get_type().get_semantic().get_resolved_target()) == simple_type::Int32);
    REQUIRE(method1_formal_parameters[1].get_id() == "d");
    REQUIRE(std::get<simple_type>(method1_formal_parameters[1].get_type().get_semantic().get_resolved_target()) == simple_type::Int32);
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
    ExpectedMethodModel m1{ "m1", default_method_semantics, type_ref{ simple_type::Int32 }, { ExpectedFormalParameterModel{ "s", parameter_semantics::in, type_ref{ simple_type::String } } } };
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
