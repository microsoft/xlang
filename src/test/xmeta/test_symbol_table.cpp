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


TEST_CASE("Duplicate Namespaces")
{
    std::istringstream test_idl(R"(
        namespace N { }
        namespace N { }
    )");

    xmeta_idl_reader reader{ "" };
    REQUIRE(reader.read(test_idl) == 0);

    auto namespaces = reader.get_namespaces();
    auto it = namespaces.find("N");
    REQUIRE(it != namespaces.end());
    auto ns = it->second;
    REQUIRE(ns->get_namespace_bodies().size() == 2);
    REQUIRE(ns->get_namespace_bodies()[0]->get_containing_namespace() == ns);
    REQUIRE(ns->get_namespace_bodies()[1]->get_containing_namespace() == ns);
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
    REQUIRE(reader.read(test_idl) == 0);

    auto namespaces = reader.get_namespaces();
    auto it = namespaces.find("N");
    REQUIRE(it != namespaces.end());
    auto ns = it->second;
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

TEST_CASE("Enum circular dependency")
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
    REQUIRE(reader.read(implicit_dependency_error_idl) == 0);
    REQUIRE(reader.get_num_semantic_errors() == 1);
    reader.reset("");
    REQUIRE(reader.read(explicit_dependency_error_idl) == 0);
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
    REQUIRE(reader.read(test_idl) == 0);

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
    REQUIRE((!del0_formal_param_2_type.is_resolved() && del0_formal_param_2_type.get_ref_name() == "E"));

    REQUIRE(delegates.find("D2") != delegates.end());
    REQUIRE(delegates.at("D2")->get_return_type() == std::nullopt);
    REQUIRE(delegates.at("D2")->get_formal_parameters().empty());
}

TEST_CASE("Property test")
{
    std::istringstream property_test_idl{ R"(
        namespace N
        {
            interface I
            {
                Int32 property_1;
                Int32 property_2 { get; }
                Int32 property_3 { set; };
                Int32 property_2 { set; };
                Int32 property_3 { get; };
                Int32 property_4 { get; set; };
                Int32 property_5 { set; get; }
            }

            runtimeclass C
            {
                Int32 property_1;
                Int32 property_2 { get; }
                Int32 property_3 { set; };
                Int32 property_2 { set; };
                Int32 property_3 { get; };
                Int32 property_4 { get; set; };
                Int32 property_5 { set; get; }
            }
        }
    )" };

    std::istringstream incorrect_property_test_idl{ R"(
        namespace N
        {
            interface I
            {
                Int32 property_1;
                Int32 property_1 { get; }
                Int32 property_2 { get; set; };
                Int32 property_2 { set; };
            }
        }
    )" };

    xmeta_idl_reader reader{ "" };
    REQUIRE(reader.read(property_test_idl) == 0);

    auto namespaces = reader.get_namespaces();
    auto it = namespaces.find("N");
    REQUIRE(it != namespaces.end());
    auto ns = it->second;
    REQUIRE(ns->get_namespace_bodies().size() == 1);
    auto ns_body = ns->get_namespace_bodies()[0];

    auto interfaces = ns_body->get_interfaces();
    REQUIRE(interfaces.size() == 1);
    REQUIRE(interfaces.find("I") != interfaces.end());
    auto iface = interfaces.at("I");
    auto properties = iface->get_properties();
    REQUIRE(properties.size() == 5);
    REQUIRE(properties[0]->get_id() == "property_1");
    REQUIRE(properties[1]->get_id() == "property_2");
    REQUIRE(properties[2]->get_id() == "property_3");
    REQUIRE(properties[3]->get_id() == "property_4");
    REQUIRE(properties[4]->get_id() == "property_5");
    REQUIRE(properties[0]->get_get_method() != nullptr);
    REQUIRE(properties[0]->get_get_method()->get_id() == "get_property_1");
    REQUIRE(properties[0]->get_set_method() != nullptr);
    REQUIRE(properties[0]->get_set_method()->get_id() == "set_property_1");
    REQUIRE(properties[1]->get_get_method() != nullptr);
    REQUIRE(properties[1]->get_set_method() != nullptr);
    REQUIRE(properties[1]->get_get_method()->get_id() == "get_property_2");
    REQUIRE(properties[1]->get_set_method()->get_id() == "set_property_2");
    REQUIRE(properties[1]->is_get_method_declared_first());
    REQUIRE(properties[2]->get_get_method() != nullptr);
    REQUIRE(properties[2]->get_set_method() != nullptr);
    REQUIRE(properties[2]->get_get_method()->get_id() == "get_property_3");
    REQUIRE(properties[2]->get_set_method()->get_id() == "set_property_3");
    REQUIRE(!properties[2]->is_get_method_declared_first());
    REQUIRE(properties[3]->get_get_method() != nullptr);
    REQUIRE(properties[3]->get_set_method() != nullptr);
    REQUIRE(properties[3]->get_get_method()->get_id() == "get_property_4");
    REQUIRE(properties[3]->get_set_method()->get_id() == "set_property_4");
    REQUIRE(properties[3]->is_get_method_declared_first());
    REQUIRE(properties[4]->get_get_method() != nullptr);
    REQUIRE(properties[4]->get_set_method() != nullptr);
    REQUIRE(properties[4]->get_get_method()->get_id() == "get_property_5");
    REQUIRE(properties[4]->get_set_method()->get_id() == "set_property_5");
    REQUIRE(!properties[4]->is_get_method_declared_first());

    auto classes = ns_body->get_classes();
    REQUIRE(classes.size() == 1);
    REQUIRE(classes.find("C") != classes.end());
    auto c = classes.at("C");
    properties = c->get_properties();
    REQUIRE(properties.size() == 5);
    REQUIRE(properties[0]->get_id() == "property_1");
    REQUIRE(properties[1]->get_id() == "property_2");
    REQUIRE(properties[2]->get_id() == "property_3");
    REQUIRE(properties[3]->get_id() == "property_4");
    REQUIRE(properties[4]->get_id() == "property_5");
    REQUIRE(properties[0]->get_get_method() != nullptr);
    REQUIRE(properties[0]->get_get_method()->get_id() == "get_property_1");
    REQUIRE(properties[0]->get_set_method() != nullptr);
    REQUIRE(properties[0]->get_set_method()->get_id() == "set_property_1");
    REQUIRE(properties[1]->get_get_method() != nullptr);
    REQUIRE(properties[1]->get_set_method() != nullptr);
    REQUIRE(properties[1]->get_get_method()->get_id() == "get_property_2");
    REQUIRE(properties[1]->get_set_method()->get_id() == "set_property_2");
    REQUIRE(properties[1]->is_get_method_declared_first());
    REQUIRE(properties[2]->get_get_method() != nullptr);
    REQUIRE(properties[2]->get_set_method() != nullptr);
    REQUIRE(properties[2]->get_get_method()->get_id() == "get_property_3");
    REQUIRE(properties[2]->get_set_method()->get_id() == "set_property_3");
    REQUIRE(!properties[2]->is_get_method_declared_first());
    REQUIRE(properties[3]->get_get_method() != nullptr);
    REQUIRE(properties[3]->get_set_method() != nullptr);
    REQUIRE(properties[3]->get_get_method()->get_id() == "get_property_4");
    REQUIRE(properties[3]->get_set_method()->get_id() == "set_property_4");
    REQUIRE(properties[3]->is_get_method_declared_first());
    REQUIRE(properties[4]->get_get_method() != nullptr);
    REQUIRE(properties[4]->get_set_method() != nullptr);
    REQUIRE(properties[4]->get_get_method()->get_id() == "get_property_5");
    REQUIRE(properties[4]->get_set_method()->get_id() == "set_property_5");
    REQUIRE(!properties[4]->is_get_method_declared_first());

    reader.reset("");
    REQUIRE(reader.read(incorrect_property_test_idl) == 0);
    REQUIRE(reader.get_num_semantic_errors() == 2);
}
