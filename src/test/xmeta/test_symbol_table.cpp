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
        namespace n { }
        namespace n { }
    )");

    xmeta_idl_reader reader{ "" };
    REQUIRE(reader.read(test_idl) == 0);

    auto namespaces = reader.get_namespaces();
    auto it = namespaces.find("n");
    REQUIRE(it != namespaces.end());
    auto ns = it->second;
    REQUIRE(ns->get_namespace_bodies().size() == 2);
    REQUIRE(ns->get_namespace_bodies()[0]->get_containing_namespace() == ns);
    REQUIRE(ns->get_namespace_bodies()[1]->get_containing_namespace() == ns);
}

TEST_CASE("Enum test")
{
    std::istringstream test_idl{ R"(
        namespace n
        {
            enum e
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
    auto it = namespaces.find("n");
    REQUIRE(it != namespaces.end());
    auto ns = it->second;
    REQUIRE(ns->get_namespace_bodies().size() == 1);
    auto ns_body = ns->get_namespace_bodies()[0];

    auto enums = ns_body->get_enums();
    REQUIRE(enums.find("e") != enums.end());
    auto const& enum_members = enums["e"]->get_members();
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
    REQUIRE((val1.is_resolved() && val1.get_resolved_target<int32_t>() == 0));
    REQUIRE((val2.is_resolved() && val2.get_resolved_target<int32_t>() == 3));
    REQUIRE((val3.is_resolved() && val3.get_resolved_target<int32_t>() == 4));
    REQUIRE((val4.is_resolved() && val4.get_resolved_target<int32_t>() == 0x21));
    REQUIRE((val5.is_resolved() && val5.get_resolved_target<int32_t>() == 0x21));
}

TEST_CASE("Enum circular dependency")
{
    std::istringstream implicit_dependency_error_idl{ R"(
        namespace n
        {
            enum e
            {
                e_member_1 = e_member_3,
                e_member_2,
                e_member_3
            }
        }
    )" };
    std::istringstream explicit_dependency_error_idl{ R"(
        namespace n
        {
            enum e
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
