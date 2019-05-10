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
    reader.read(test_idl);
    REQUIRE(reader.get_num_syntax_errors() == 0);

    auto namespaces = reader.get_namespaces();
    auto it = namespaces.find("N");
    REQUIRE(it != namespaces.end());
    auto ns = it->second;
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
    auto fields = struct1->get_fields();
    REQUIRE(fields.size() == 11);
    REQUIRE(fields[0].second == "field_1");
    auto type1 = fields[0].first.get_semantic();
    REQUIRE(std::get<simple_type>(type1.get_resolved_target()) == simple_type::Boolean);
    REQUIRE(fields[1].second == "field_2");
    auto type2 = fields[1].first.get_semantic();
    REQUIRE(std::get<simple_type>(type2.get_resolved_target()) == simple_type::String);
    REQUIRE(fields[2].second == "field_3");
    auto type3 = fields[2].first.get_semantic();
    REQUIRE(std::get<simple_type>(type3.get_resolved_target()) == simple_type::Int16);
    REQUIRE(fields[3].second == "field_4");
    auto type4 = fields[3].first.get_semantic();
    REQUIRE(std::get<simple_type>(type4.get_resolved_target()) == simple_type::Int32);
    REQUIRE(fields[4].second == "field_5");
    auto type5 = fields[4].first.get_semantic();
    REQUIRE(std::get<simple_type>(type5.get_resolved_target()) == simple_type::Int64);
    REQUIRE(fields[5].second == "field_6");
    auto type6 = fields[5].first.get_semantic();
    REQUIRE(std::get<simple_type>(type6.get_resolved_target()) == simple_type::UInt8);
    REQUIRE(fields[6].second == "field_7");
    auto type7 = fields[6].first.get_semantic();
    REQUIRE(std::get<simple_type>(type7.get_resolved_target()) == simple_type::UInt16);
    REQUIRE(fields[7].second == "field_8");
    auto type8 = fields[7].first.get_semantic();
    REQUIRE(std::get<simple_type>(type8.get_resolved_target()) == simple_type::UInt32);
    REQUIRE(fields[8].second == "field_9");
    auto type9 = fields[8].first.get_semantic();
    REQUIRE(std::get<simple_type>(type9.get_resolved_target()) == simple_type::Char16);
    REQUIRE(fields[9].second == "field_10");
    auto type10 = fields[9].first.get_semantic();
    REQUIRE(std::get<simple_type>(type10.get_resolved_target()) == simple_type::Single);
    REQUIRE(fields[10].second == "field_11");
    auto type11 = fields[10].first.get_semantic();
    REQUIRE(std::get<simple_type>(type11.get_resolved_target()) == simple_type::Double);
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
        REQUIRE(reader.get_num_semantic_errors() == 4);
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

TEST_CASE("Resolving delegates types test")
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

TEST_CASE("Resolving struct types test")
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

TEST_CASE("Resolving types across namespaces test")
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