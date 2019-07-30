#include "pch.h"
#include "test_model_infrastructure.h"

using namespace antlr4;
using namespace xlang::xmeta;
using namespace xlang::meta::reader;

// 1 will print error, 0 will not
#define PRINT_ERROR_FLAG 1

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

TEST_CASE("Duplicate Namespaces")
{
    std::istringstream test_idl(R"(
        namespace N { }
        namespace N { }
    )");

    xmeta_idl_reader reader{ "" };
    reader.read(test_idl, PRINT_ERROR_FLAG);
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
    reader.read(test_idl, PRINT_ERROR_FLAG);
    REQUIRE(reader.get_num_syntax_errors() == 0);
    REQUIRE(reader.get_num_semantic_errors() == 3);
    REQUIRE(reader.error_exists(idl_error::DUPLICATE_NAMESPACE_MEMBER, "E", 5));
    REQUIRE(reader.error_exists(idl_error::DUPLICATE_NAMESPACE_MEMBER, "D", 8));
    REQUIRE(reader.error_exists(idl_error::DUPLICATE_NAMESPACE_MEMBER, "S", 11));
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
    reader.read(test_idl, PRINT_ERROR_FLAG);
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
    reader.read(implicit_dependency_error_idl, PRINT_ERROR_FLAG);
    REQUIRE(reader.get_num_syntax_errors() == 0);
    REQUIRE(reader.get_num_semantic_errors() == 1);
    REQUIRE(reader.error_exists(idl_error::CIRCULAR_ENUM_FIELD, "e_member_3", 8));
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
    reader.read(explicit_dependency_error_idl, PRINT_ERROR_FLAG);
    REQUIRE(reader.get_num_syntax_errors() == 0);
    REQUIRE(reader.get_num_semantic_errors() == 1);
    REQUIRE(reader.error_exists(idl_error::CIRCULAR_ENUM_FIELD, "e_member_1", 6));
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
    reader.read(test_idl, PRINT_ERROR_FLAG);
    REQUIRE(reader.get_num_syntax_errors() == 0);

    ExpectedEnumModel E{ "E", "N.E" , enum_type::Int32, {} };
    ExpectedEnumModel F{ "F", "N.F" , enum_type::Int32, {} };
    ExpectedDelegateModel D1{ "D1", "N.D1", ExpectedTypeRefModel{ ElementType::I4 }, {
        ExpectedFormalParameterModel{ "i", parameter_semantics::in, ExpectedTypeRefModel{ ElementType::I4 } },
        ExpectedFormalParameterModel{ "d", parameter_semantics::in, ExpectedTypeRefModel{ ElementType::R8 } },
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
    reader.read(struct_test_idl, PRINT_ERROR_FLAG);
    REQUIRE(reader.get_num_syntax_errors() == 0);

    ExpectedStructModel expected_struct{ "S", "N.S" , { 
        { ExpectedTypeRefModel{ ElementType::Boolean } , "field_1" },
        { ExpectedTypeRefModel{ ElementType::String } , "field_2" },
        { ExpectedTypeRefModel{ ElementType::I2 } , "field_3" },
        { ExpectedTypeRefModel{ ElementType::I4 } , "field_4" },
        { ExpectedTypeRefModel{ ElementType::I8 } , "field_5" },
        { ExpectedTypeRefModel{ ElementType::U1 } , "field_6" },
        { ExpectedTypeRefModel{ ElementType::U2 } , "field_7" },
        { ExpectedTypeRefModel{ ElementType::U4 } , "field_8" },
        { ExpectedTypeRefModel{ ElementType::Char } , "field_9" },
        { ExpectedTypeRefModel{ ElementType::R4 } , "field_10" },
        { ExpectedTypeRefModel{ ElementType::R8 } , "field_11" }
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
        reader.read(struct_test_idl, PRINT_ERROR_FLAG);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() > 0);
        REQUIRE(reader.error_exists(idl_error::CIRCULAR_STRUCT_FIELD, "N.S0", 4));
        REQUIRE(reader.error_exists(idl_error::CIRCULAR_STRUCT_FIELD, "N.S1", 8));
        REQUIRE(reader.error_exists(idl_error::CIRCULAR_STRUCT_FIELD, "N.S2", 12));
        REQUIRE(reader.error_exists(idl_error::CIRCULAR_STRUCT_FIELD, "N.S3", 16));
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
        reader.read(struct_test_idl, PRINT_ERROR_FLAG);
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
    reader.read(struct_test_idl, PRINT_ERROR_FLAG);
    REQUIRE(reader.get_num_syntax_errors() == 0);
    REQUIRE(reader.get_num_semantic_errors() == 1);
    REQUIRE(reader.error_exists(idl_error::DUPLICATE_FIELD_ID, "field_1", 7));
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
    reader.read(test_idl, PRINT_ERROR_FLAG);
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
    reader.read(struct_test_idl, PRINT_ERROR_FLAG);
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
    reader.read(test_idl, PRINT_ERROR_FLAG);
    REQUIRE(reader.get_num_syntax_errors() == 0);

    ExpectedStructModel S1{ "S1", "A.S1", {} };

    ExpectedEnumModel E1{ "E1", "B.C.E1" , enum_type::Int32, {} };

    ExpectedDelegateModel D1{ "D1", "N.D1", ExpectedTypeRefModel{ ElementType::Boolean }, {
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
    reader.read(test_idl, PRINT_ERROR_FLAG);
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
        reader.read(test_idl, PRINT_ERROR_FLAG);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 0);

        ExpectedMethodModel Paint{ "Paint", default_method_modifier, std::nullopt, {} };
        ExpectedMethodModel Draw{ "Draw", default_method_modifier, ExpectedTypeRefModel{ ElementType::I4 }, {
            ExpectedFormalParameterModel{ "i", parameter_semantics::in, ExpectedTypeRefModel{ ElementType::I4 } },
            ExpectedFormalParameterModel{ "d", parameter_semantics::in, ExpectedTypeRefModel{ ElementType::I4 } }
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
    reader.read(test_idl, PRINT_ERROR_FLAG);
    REQUIRE(reader.get_num_syntax_errors() == 0);
    REQUIRE(reader.get_num_semantic_errors() == 0);

    ExpectedMethodModel Paint{ "Paint", default_method_modifier, std::nullopt, {} };
    ExpectedMethodModel Paint2{ "Paint", default_method_modifier, std::nullopt, {
        ExpectedFormalParameterModel{ "p1", parameter_semantics::in, ExpectedTypeRefModel{ ElementType::I4 } }
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
    reader.read(test_idl, PRINT_ERROR_FLAG);
    REQUIRE(reader.get_num_syntax_errors() == 0);
    REQUIRE(reader.get_num_semantic_errors() == 0);

    ExpectedMethodModel Paint{ "Paint", default_method_modifier, ExpectedTypeRefModel{ ElementType::I8 }, {} };
    ExpectedMethodModel Paint2{ "Paint", default_method_modifier, ExpectedTypeRefModel{ ElementType::I8 }, {
        ExpectedFormalParameterModel{ "p1", parameter_semantics::in, ExpectedTypeRefModel{ ElementType::I4 } }
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
    reader.read(test_idl, PRINT_ERROR_FLAG);
    REQUIRE(reader.get_num_syntax_errors() == 0);
    REQUIRE(reader.get_num_semantic_errors() == 0);

    ExpectedMethodModel Paint{ "Paint", default_method_modifier, ExpectedTypeRefModel{ ExpectedStructRef{ "N.s1" } }, {} };
    ExpectedMethodModel Paint2{ "Paint", default_method_modifier, ExpectedTypeRefModel{ ExpectedStructRef{ "N.s1" } }, {
        ExpectedFormalParameterModel{ "p1", parameter_semantics::in, ExpectedTypeRefModel{ ElementType::I4 } }
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
        reader.read(test_idl, PRINT_ERROR_FLAG);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 2);
        REQUIRE(reader.error_exists(idl_error::CANNOT_OVERLOAD_METHOD, "Paint", 7));
        REQUIRE(reader.error_exists(idl_error::CANNOT_OVERLOAD_METHOD, "Paint", 12));
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
        reader.read(test_idl, PRINT_ERROR_FLAG);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 2);
        REQUIRE(reader.error_exists(idl_error::CANNOT_OVERLOAD_METHOD, "Paint", 7));
        REQUIRE(reader.error_exists(idl_error::CANNOT_OVERLOAD_METHOD, "Paint", 12));
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
        reader.read(test_idl, PRINT_ERROR_FLAG);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 2);
        REQUIRE(reader.error_exists(idl_error::CANNOT_OVERLOAD_METHOD, "Paint", 7));
        REQUIRE(reader.error_exists(idl_error::CANNOT_OVERLOAD_METHOD, "Paint", 12));
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
        reader.read(test_idl, PRINT_ERROR_FLAG);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 2);
        REQUIRE(reader.error_exists(idl_error::CANNOT_OVERLOAD_METHOD, "Paint", 11));
        REQUIRE(reader.error_exists(idl_error::CANNOT_OVERLOAD_METHOD, "Paint", 16));
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
        reader.read(test_idl, PRINT_ERROR_FLAG);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 2);
        REQUIRE(reader.error_exists(idl_error::CANNOT_OVERLOAD_METHOD, "Paint", 11));
        REQUIRE(reader.error_exists(idl_error::CANNOT_OVERLOAD_METHOD, "Paint", 16));
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
    reader.read(test_idl, PRINT_ERROR_FLAG);
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
    reader.read(test_idl, PRINT_ERROR_FLAG);
    REQUIRE(reader.get_num_syntax_errors() == 0);

    ExpectedMethodModel get_property1{ "get_property1", default_method_modifier, ExpectedTypeRefModel{ ElementType::I4 }, {} };
    ExpectedMethodModel set_property1{ "put_property1", default_method_modifier, std::nullopt,
        { ExpectedFormalParameterModel{ "value", parameter_semantics::in, ExpectedTypeRefModel{ ElementType::I4 } } } };
    ExpectedPropertyModel property1{ "property1",
        default_property_modifier,
        ExpectedTypeRefModel{ ElementType::I4 },
        get_property1,
        set_property1
    };

    ExpectedMethodModel get_property2{ "get_property2", default_method_modifier, ExpectedTypeRefModel{ ElementType::I4 }, {} };
    ExpectedPropertyModel property2{ "property2",
        default_property_modifier,
        ExpectedTypeRefModel{ ElementType::I4 },
        get_property2,
        std::nullopt
    };

    ExpectedMethodModel get_property3{ "get_property3", default_method_modifier, ExpectedTypeRefModel{ ElementType::I4 }, {} };
    ExpectedMethodModel set_property3{ "put_property3", default_method_modifier, std::nullopt,
        { ExpectedFormalParameterModel{ "value", parameter_semantics::in, ExpectedTypeRefModel{ ElementType::I4 } } } };
    ExpectedPropertyModel property3{ "property3",
        default_property_modifier,
        ExpectedTypeRefModel{ ElementType::I4 },
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
    ExpectedMethodModel get_property1{ "get_property1", default_method_modifier, ExpectedTypeRefModel{ ElementType::I4 }, {} };
    ExpectedMethodModel set_property1{ "put_property1", default_method_modifier, std::nullopt,
        { ExpectedFormalParameterModel{ "value", parameter_semantics::in, ExpectedTypeRefModel{ ElementType::I4 } } } };
    ExpectedPropertyModel property1{ "property1",
        default_property_modifier,
        ExpectedTypeRefModel{ ElementType::I4 },
        get_property1,
        set_property1
    };
    ExpectedMethodModel get_property2{ "get_property2", default_method_modifier, ExpectedTypeRefModel{ ElementType::I4 }, {} };
    ExpectedMethodModel set_property2{ "put_property2", default_method_modifier, std::nullopt,
        { ExpectedFormalParameterModel{ "value", parameter_semantics::in, ExpectedTypeRefModel{ ElementType::I4 } } } };
    ExpectedPropertyModel property2{ "property2",
        default_property_modifier,
        ExpectedTypeRefModel{ ElementType::I4 },
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
        reader.read(test_idl, PRINT_ERROR_FLAG);
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
        reader.read(test_idl, PRINT_ERROR_FLAG);
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
        reader.read(test_idl, PRINT_ERROR_FLAG);
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
        reader.read(test_idl, PRINT_ERROR_FLAG);
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

TEST_CASE("Invalid property accessor set only test")
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
            reader.read(test_set_only_idl, PRINT_ERROR_FLAG);
            REQUIRE(reader.get_num_syntax_errors() == 0);
            REQUIRE(reader.get_num_semantic_errors() == 1);
            REQUIRE(reader.error_exists(idl_error::INVALID_PROPERTY_ACCESSOR, "property1", 6));
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
            reader.read(test_set_only_idl, PRINT_ERROR_FLAG);
            REQUIRE(reader.get_num_syntax_errors() == 0);
            //TODO: This is only reporting one error due to the synthesized interface
            // Make this only report 1
            REQUIRE(reader.get_num_semantic_errors() == 2);
            REQUIRE(reader.error_exists(idl_error::INVALID_PROPERTY_ACCESSOR, "property3", 6));
            REQUIRE(reader.error_exists(idl_error::INVALID_PROPERTY_ACCESSOR, "property3", 6));
        }
    }
}

TEST_CASE("Invalid property accessor multiple getters test")
{
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
            reader.read(test_double_get_idl, PRINT_ERROR_FLAG);
            REQUIRE(reader.get_num_syntax_errors() == 0);
            REQUIRE(reader.get_num_semantic_errors() == 1);
            REQUIRE(reader.error_exists(idl_error::DUPLICATE_PROPERTY_ACCESSOR, "property1", 6));
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
            reader.read(test_double_get_idl, PRINT_ERROR_FLAG);
            REQUIRE(reader.get_num_syntax_errors() == 0);
            REQUIRE(reader.get_num_semantic_errors() >= 1);
            REQUIRE(reader.error_exists(idl_error::INVALID_OR_DUPLICATE_PROPERTY_ACCESSOR, "property1", 7));
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
            reader.read(test_double_get_idl, PRINT_ERROR_FLAG);
            REQUIRE(reader.get_num_syntax_errors() == 0);
            REQUIRE(reader.get_num_semantic_errors() == 1);
            REQUIRE(reader.error_exists(idl_error::DUPLICATE_PROPERTY_ACCESSOR, "property1", 6));
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
            reader.read(test_double_get_idl, PRINT_ERROR_FLAG);
            REQUIRE(reader.get_num_syntax_errors() == 0);
            REQUIRE(reader.get_num_semantic_errors() >= 1);
            REQUIRE(reader.error_exists(idl_error::INVALID_OR_DUPLICATE_PROPERTY_ACCESSOR, "property1", 7));
        }
    }
}
TEST_CASE("Invalid property accessor multiple setters test")
{
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
            reader.read(test_double_set_idl, PRINT_ERROR_FLAG);
            REQUIRE(reader.get_num_syntax_errors() == 0);
            REQUIRE(reader.get_num_semantic_errors() == 1);
            REQUIRE(reader.error_exists(idl_error::DUPLICATE_PROPERTY_ACCESSOR, "property1", 6));
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
            reader.read(test_double_set_idl, PRINT_ERROR_FLAG);
            REQUIRE(reader.get_num_syntax_errors() == 0);
            REQUIRE(reader.get_num_semantic_errors() == 1);
            REQUIRE(reader.error_exists(idl_error::DUPLICATE_PROPERTY_ACCESSOR, "property1", 6));
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
            reader.read(test_double_set_idl, PRINT_ERROR_FLAG);
            REQUIRE(reader.get_num_syntax_errors() == 0);
            REQUIRE(reader.get_num_semantic_errors() >= 1);
            REQUIRE(reader.error_exists(idl_error::INVALID_OR_DUPLICATE_PROPERTY_ACCESSOR, "property1", 7));
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
            reader.read(test_double_set_idl, PRINT_ERROR_FLAG);
            REQUIRE(reader.get_num_syntax_errors() == 0);
            REQUIRE(reader.get_num_semantic_errors() >= 1);
            REQUIRE(reader.error_exists(idl_error::INVALID_OR_DUPLICATE_PROPERTY_ACCESSOR, "property1", 7));
        }
    }
}

TEST_CASE("Invalid property accessor miscellaneous test")
{
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
            reader.read(test_three_acessor_idl, PRINT_ERROR_FLAG);
            REQUIRE(reader.get_num_syntax_errors() == 0);
            REQUIRE(reader.get_num_semantic_errors() == 1);
            REQUIRE(reader.error_exists(idl_error::INVALID_PROPERTY_ACCESSOR, "property1", 6));
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
            reader.read(test_three_acessor_idl, PRINT_ERROR_FLAG);
            REQUIRE(reader.get_num_syntax_errors() == 0);
            REQUIRE(reader.get_num_semantic_errors() == 1);
            REQUIRE(reader.error_exists(idl_error::INVALID_PROPERTY_ACCESSOR, "property1", 6));
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
            reader.read(test_add_and_remove_idl, PRINT_ERROR_FLAG);
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
            reader.read(test_add_and_remove_idl, PRINT_ERROR_FLAG);
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
        reader.read(test_set_only_idl, PRINT_ERROR_FLAG);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() >= 1);
        REQUIRE(reader.error_exists(idl_error::DUPLICATE_TYPE_MEMBER_ID, "property1", 7));
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
        reader.read(test_set_only_idl, PRINT_ERROR_FLAG);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() >= 1);
        REQUIRE(reader.error_exists(idl_error::DUPLICATE_TYPE_MEMBER_ID, "property1", 7));
    }
    {
        std::istringstream test_set_only_idl{ R"(
            namespace N
            {
                runtimeclass c1
                {
                    Int32 property4 { get; };
                    Int64 property4 { set; };
                }
            }
        )" };

        xmeta_idl_reader reader{ "" };
        reader.read(test_set_only_idl, PRINT_ERROR_FLAG);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() >= 1);
        REQUIRE(reader.error_exists(idl_error::DUPLICATE_TYPE_MEMBER_ID, "property4", 7));
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
    reader.read(test_idl, PRINT_ERROR_FLAG);
    REQUIRE(reader.get_num_syntax_errors() == 0);

    ExpectedMethodModel get_property1{ "get_property1", default_method_modifier, ExpectedTypeRefModel{ ElementType::I4 }, {} };
    ExpectedMethodModel set_property1{ "put_property1", default_method_modifier, std::nullopt,
        { ExpectedFormalParameterModel{ "value", parameter_semantics::in, ExpectedTypeRefModel{ ElementType::I4 } } } };
    ExpectedPropertyModel property1{ "property1",
        default_property_modifier,
        ExpectedTypeRefModel{ ElementType::I4 },
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
    reader.read(test_idl, PRINT_ERROR_FLAG);
    REQUIRE(reader.get_num_syntax_errors() == 0);

    ExpectedMethodModel get_property1{ "get_property1", default_method_modifier, ExpectedTypeRefModel{ ExpectedStructRef{ "N.S1" } }, {} };
    ExpectedMethodModel set_property1{ "put_property1", default_method_modifier, std::nullopt,
        { ExpectedFormalParameterModel{ "value", parameter_semantics::in, ExpectedTypeRefModel{ ExpectedStructRef{ "N.S1" } } } } };
    ExpectedPropertyModel property1{ "property1",
        default_property_modifier,
        ExpectedTypeRefModel{ ExpectedStructRef{ "N.S1" } },
        get_property1,
        set_property1
    };

    ExpectedMethodModel get_property2{ "get_property2", default_method_modifier, ExpectedTypeRefModel{ ExpectedEnumRef{ "M.E2" } }, {} };
    ExpectedMethodModel set_property2{ "put_property2", default_method_modifier, std::nullopt,
        { ExpectedFormalParameterModel{ "value", parameter_semantics::in, ExpectedTypeRefModel{ ExpectedEnumRef{ "M.E2" } } } } };
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
    reader.read(test_idl, PRINT_ERROR_FLAG);
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
        reader.read(test_idl, PRINT_ERROR_FLAG);
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
        reader.read(test_idl, PRINT_ERROR_FLAG);
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
            reader.read(test_idl, PRINT_ERROR_FLAG);
            REQUIRE(reader.get_num_syntax_errors() == 0);
            REQUIRE(reader.get_num_semantic_errors() == 1);
            REQUIRE(reader.error_exists(idl_error::DUPLICATE_TYPE_MEMBER_ID, "Changed", 8));
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
            reader.read(test_idl, PRINT_ERROR_FLAG);
            REQUIRE(reader.get_num_syntax_errors() == 0);
            REQUIRE(reader.get_num_semantic_errors() == 1);
            REQUIRE(reader.error_exists(idl_error::DUPLICATE_TYPE_MEMBER_ID, "Changed", 8));
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
            reader.read(test_idl, PRINT_ERROR_FLAG);
            REQUIRE(reader.get_num_syntax_errors() == 0);
            REQUIRE(reader.get_num_semantic_errors() == 1);
            REQUIRE(reader.error_exists(idl_error::DUPLICATE_TYPE_MEMBER_ID, "Changed", 8));
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
            reader.read(test_idl, PRINT_ERROR_FLAG);
            REQUIRE(reader.get_num_syntax_errors() == 0);
            REQUIRE(reader.get_num_semantic_errors() == 1);
            REQUIRE(reader.error_exists(idl_error::DUPLICATE_TYPE_MEMBER_ID, "Changed", 8));
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
        reader.read(test_idl, PRINT_ERROR_FLAG);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 1);
        REQUIRE(reader.error_exists(idl_error::DUPLICATE_TYPE_MEMBER_ID, "Changed", 8));
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
        reader.read(test_idl, PRINT_ERROR_FLAG);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 1);
        REQUIRE(reader.error_exists(idl_error::DUPLICATE_TYPE_MEMBER_ID, "Changed", 8));
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
        reader.read(test_idl, PRINT_ERROR_FLAG);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 2);
        REQUIRE(reader.error_exists(idl_error::CONFLICTING_EVENT_ACCESSOR_METHODS, "Changed", 8));
        REQUIRE(reader.error_exists(idl_error::CANNOT_OVERLOAD_METHOD, "add_Changed", 9));
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
        reader.read(test_idl, PRINT_ERROR_FLAG);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 2);
        REQUIRE(reader.error_exists(idl_error::CONFLICTING_EVENT_ACCESSOR_METHODS, "Changed", 8));
        REQUIRE(reader.error_exists(idl_error::CANNOT_OVERLOAD_METHOD, "remove_Changed", 9));
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
        reader.read(test_idl, PRINT_ERROR_FLAG);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 2);
        REQUIRE(reader.error_exists(idl_error::INVALID_OR_DUPLICATE_PROPERTY_ACCESSOR, "property1", 7));
        REQUIRE(reader.error_exists(idl_error::CANNOT_OVERLOAD_METHOD, "put_property1", 8));
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
        reader.read(test_idl, PRINT_ERROR_FLAG);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 2);
        REQUIRE(reader.error_exists(idl_error::INVALID_OR_DUPLICATE_PROPERTY_ACCESSOR, "property1", 7));
        REQUIRE(reader.error_exists(idl_error::CANNOT_OVERLOAD_METHOD, "put_property1", 8));
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
    reader.read(test_idl, PRINT_ERROR_FLAG);
    REQUIRE(reader.get_num_syntax_errors() == 0);
    REQUIRE(reader.get_num_semantic_errors() > 0);
    REQUIRE(reader.error_exists(idl_error::CIRCULAR_INTERFACE_INHERITANCE, "N.i1", 4));
    REQUIRE(reader.error_exists(idl_error::CIRCULAR_INTERFACE_INHERITANCE, "N.ITextBox", 8));
    REQUIRE(reader.error_exists(idl_error::CIRCULAR_INTERFACE_INHERITANCE, "N.IListBox", 12));
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
        reader.read(test_idl, PRINT_ERROR_FLAG);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 1);
        REQUIRE(reader.error_exists(idl_error::CANNOT_OVERLOAD_METHOD, "Paint", 4));
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
        reader.read(test_idl, PRINT_ERROR_FLAG);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 1);
        REQUIRE(reader.error_exists(idl_error::CANNOT_OVERLOAD_METHOD, "get_value", 4));
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
        reader.read(test_idl, PRINT_ERROR_FLAG);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 1);
        REQUIRE(reader.error_exists(idl_error::CANNOT_OVERLOAD_METHOD, "get_value", 4));
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
        reader.read(test_idl, PRINT_ERROR_FLAG);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 1);
        REQUIRE(reader.error_exists(idl_error::CONFLICTING_INHERITANCE_MEMBER, "value", 4));
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
        reader.read(test_idl, PRINT_ERROR_FLAG);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 1);
        REQUIRE(reader.error_exists(idl_error::CONFLICTING_INHERITANCE_MEMBER, "value", 5));
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
        reader.read(test_idl, PRINT_ERROR_FLAG);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 1);
        REQUIRE(reader.error_exists(idl_error::CONFLICTING_INHERITANCE_MEMBER, "value", 5));
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
        reader.read(test_idl, PRINT_ERROR_FLAG);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 1);
        REQUIRE(reader.error_exists(idl_error::CONFLICTING_INHERITANCE_MEMBER, "value", 6));
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
    reader.read(test_idl, PRINT_ERROR_FLAG);
    REQUIRE(reader.get_num_syntax_errors() == 0);
    REQUIRE(reader.get_num_semantic_errors() == 5);
    REQUIRE(reader.error_exists(idl_error::UNRESOLVED_TYPE, "N.fakebase", 4));
    REQUIRE(reader.error_exists(idl_error::UNRESOLVED_TYPE, "N.StringListEvent", 6));
    REQUIRE(reader.error_exists(idl_error::UNRESOLVED_TYPE, "N.FakeObject", 7));
    REQUIRE(reader.error_exists(idl_error::UNRESOLVED_TYPE, "N.FakeObject2", 8));
    REQUIRE(reader.error_exists(idl_error::UNRESOLVED_TYPE, "N.FakeObject", 8));
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
    reader.read(test_idl, PRINT_ERROR_FLAG);
    REQUIRE(reader.get_num_syntax_errors() == 0);
    REQUIRE(reader.get_num_semantic_errors() > 0);
    REQUIRE(reader.error_exists(idl_error::CIRCULAR_INTERFACE_INHERITANCE, "N.IListBox", 12));
    REQUIRE(reader.error_exists(idl_error::CIRCULAR_INTERFACE_INHERITANCE, "N.ITextBox", 8));
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
        reader.read(test_idl, PRINT_ERROR_FLAG);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() > 0);
        REQUIRE(reader.error_exists(idl_error::CANNOT_OVERLOAD_METHOD, "Paint", 4));
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
        reader.read(test_idl, PRINT_ERROR_FLAG);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 1);
        REQUIRE(reader.error_exists(idl_error::CANNOT_OVERLOAD_METHOD, "get_value", 4));
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
        reader.read(test_idl, PRINT_ERROR_FLAG);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 1);
        REQUIRE(reader.error_exists(idl_error::CANNOT_OVERLOAD_METHOD, "get_value", 4));
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
        reader.read(test_idl, PRINT_ERROR_FLAG);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 1);
        REQUIRE(reader.error_exists(idl_error::CONFLICTING_INHERITANCE_MEMBER, "value", 4));
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
        reader.read(test_idl, PRINT_ERROR_FLAG);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 1);
        REQUIRE(reader.error_exists(idl_error::CONFLICTING_INHERITANCE_MEMBER, "value", 5));
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
        reader.read(test_idl, PRINT_ERROR_FLAG);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 1);
        REQUIRE(reader.error_exists(idl_error::CONFLICTING_INHERITANCE_MEMBER, "value", 5));
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
        reader.read(test_idl, PRINT_ERROR_FLAG);
        REQUIRE(reader.get_num_syntax_errors() == 0);
        REQUIRE(reader.get_num_semantic_errors() == 1);
        REQUIRE(reader.error_exists(idl_error::CONFLICTING_INHERITANCE_MEMBER, "value", 6));
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
    reader.read(test_idl, PRINT_ERROR_FLAG);
    REQUIRE(reader.get_num_syntax_errors() == 0);
    //TODO: This is only reporting one error due to the synthesized interface
    // Make this only report 5
    REQUIRE(reader.get_num_semantic_errors() >= 5);
    REQUIRE(reader.error_exists(idl_error::UNRESOLVED_TYPE, "N.fakebase", 4));
    REQUIRE(reader.error_exists(idl_error::UNRESOLVED_TYPE, "N.StringListEvent", 6));
    REQUIRE(reader.error_exists(idl_error::UNRESOLVED_TYPE, "N.FakeObject", 7));
    REQUIRE(reader.error_exists(idl_error::UNRESOLVED_TYPE, "N.FakeObject2", 8));
    REQUIRE(reader.error_exists(idl_error::UNRESOLVED_TYPE, "N.FakeObject", 8));
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
    reader.read(test_idl, PRINT_ERROR_FLAG);
    REQUIRE(reader.get_num_syntax_errors() == 0);

    ExpectedMethodModel m1{ "m1", default_method_modifier, ExpectedTypeRefModel{ ElementType::I4 }, {
        ExpectedFormalParameterModel{ "s", parameter_semantics::in, ExpectedTypeRefModel{ ElementType::String } } } };
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
    reader.read(test_idl, PRINT_ERROR_FLAG);
    REQUIRE(reader.get_num_syntax_errors() == 0);

    ExpectedMethodModel m0s{ "m0s", static_method_modifier, std::nullopt, {} };
    ExpectedMethodModel m1s{ "m1s", static_method_modifier, ExpectedTypeRefModel{ ElementType::I4 }, {
        ExpectedFormalParameterModel{ "s", parameter_semantics::in, ExpectedTypeRefModel{ ElementType::String } } } };
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
    ExpectedMethodModel get_p1{ "get_p1", static_method_modifier, ExpectedTypeRefModel{ ElementType::I4 }, {} };
    ExpectedMethodModel set_p1{ "put_p1", static_method_modifier, std::nullopt, {
        ExpectedFormalParameterModel{ "value", parameter_semantics::in, ExpectedTypeRefModel{ ElementType::I4 } } } };

    ExpectedPropertyModel p1{ "p1", static_property_modifier, ExpectedTypeRefModel{ ElementType::I4 }, get_p1, set_p1 };

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
        reader.read(test_idl, PRINT_ERROR_FLAG);
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
        reader.read(test_idl, PRINT_ERROR_FLAG);
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
        reader.read(test_idl, PRINT_ERROR_FLAG);
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
        reader.read(test_idl, PRINT_ERROR_FLAG);
        REQUIRE(reader.get_num_syntax_errors() == 0);

        ExpectedPropertyModel p1g{ "p1", static_property_modifier, ExpectedTypeRefModel{ ElementType::I4 }, get_p1, std::nullopt };

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
    reader.read(test_idl, PRINT_ERROR_FLAG);
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
    reader.read(test_idl, PRINT_ERROR_FLAG);
    REQUIRE(reader.get_num_syntax_errors() == 0);
    REQUIRE(reader.get_num_semantic_errors() == 3);
    REQUIRE(reader.error_exists(idl_error::STATIC_MEMBER_ONLY, "m0s", 7));
    REQUIRE(reader.error_exists(idl_error::STATIC_MEMBER_ONLY, "p1", 8));
    REQUIRE(reader.error_exists(idl_error::STATIC_MEMBER_ONLY, "e1", 9));
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
    ExpectedMethodModel m1{ "m1", static_method_modifier, ExpectedTypeRefModel{ ElementType::I4 }, {
        ExpectedFormalParameterModel{ "s", parameter_semantics::in, ExpectedTypeRefModel{ ElementType::String } } } };
    
    std::vector<std::string> paths = { "Foundation.xmeta" };
    xmeta_idl_reader reader{ "" , paths };
    reader.read(test_idl, PRINT_ERROR_FLAG);
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
    ExpectedMethodModel m1{ "m1", default_method_modifier, ExpectedTypeRefModel{ ElementType::I4 }, {
        ExpectedFormalParameterModel{ "s", parameter_semantics::in, ExpectedTypeRefModel{ ElementType::String } } } };

    ExpectedEventModel e1{ "e1", default_event_modifier, ExpectedTypeRefModel{ ExpectedDelegateRef{ "N.StringListEvent" } } };

    ExpectedMethodModel get_p1{ "get_p1", default_method_modifier, ExpectedTypeRefModel{ ElementType::I4 }, {} };
    ExpectedMethodModel set_p1{ "put_p1", default_method_modifier, std::nullopt, {
        ExpectedFormalParameterModel{ "value", parameter_semantics::in, ExpectedTypeRefModel{ ElementType::I4 } } } };
    ExpectedPropertyModel p1{ "p1", default_property_modifier, ExpectedTypeRefModel{ ElementType::I4 }, get_p1, set_p1 };
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
        reader.read(test_idl, PRINT_ERROR_FLAG);
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
        reader.read(test_idl, PRINT_ERROR_FLAG);
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
    ExpectedMethodModel m1{ "m1", static_method_modifier, ExpectedTypeRefModel{ ElementType::I4 }, {
        ExpectedFormalParameterModel{ "s", parameter_semantics::in, ExpectedTypeRefModel{ ElementType::String } } } };

    ExpectedEventModel e1{ "e1", static_event_modifier, ExpectedTypeRefModel{ ExpectedDelegateRef{ "N.StringListEvent" } } };

    ExpectedMethodModel get_p1{ "get_p1", static_method_modifier, ExpectedTypeRefModel{ ElementType::I4 }, {} };
    ExpectedMethodModel set_p1{ "put_p1", static_method_modifier, std::nullopt, {
        ExpectedFormalParameterModel{ "value", parameter_semantics::in, ExpectedTypeRefModel{ ElementType::I4 } } } };
    ExpectedPropertyModel p1{ "p1", static_property_modifier, ExpectedTypeRefModel{ ElementType::I4 }, get_p1, set_p1 };
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
        reader.read(test_idl, PRINT_ERROR_FLAG);
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
        reader.read(test_idl, PRINT_ERROR_FLAG);
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
                Area(Int32 width);
                Area(Int32 width, Int32 height);
            }
        }
    )" };

    std::vector<std::string> paths = { "Foundation.xmeta" };
    xmeta_idl_reader reader{ "" , paths };
    reader.read(test_idl, PRINT_ERROR_FLAG);
    REQUIRE(reader.get_num_syntax_errors() == 0);

    ExpectedMethodModel Area{ ".ctor", default_method_modifier, std::nullopt, {} };
    ExpectedMethodModel Area2{ ".ctor", default_method_modifier, std::nullopt, {
        ExpectedFormalParameterModel{ "width", parameter_semantics::in, ExpectedTypeRefModel{ ElementType::I4 } }
    } };
    ExpectedMethodModel Area3{ ".ctor", default_method_modifier, std::nullopt, {
        ExpectedFormalParameterModel{ "width", parameter_semantics::in, ExpectedTypeRefModel{ ElementType::I4 } },
        ExpectedFormalParameterModel{ "height", parameter_semantics::in, ExpectedTypeRefModel{ ElementType::I4 } }
    } };


    ExpectedMethodModel Areaf{ "CreateInstance", default_method_modifier, ExpectedTypeRefModel{ ExpectedClassRef{ "N.Area" } }, {
        ExpectedFormalParameterModel{ "width", parameter_semantics::in, ExpectedTypeRefModel{ ElementType::I4 } }
    } };
    ExpectedMethodModel Areaf2{ "CreateInstance2", default_method_modifier, ExpectedTypeRefModel{ ExpectedClassRef{ "N.Area" } }, {
        ExpectedFormalParameterModel{ "width", parameter_semantics::in, ExpectedTypeRefModel{ ElementType::I4 } },
        ExpectedFormalParameterModel{ "height", parameter_semantics::in, ExpectedTypeRefModel{ ElementType::I4 } }
    } };

    ExpectedClassModel c1Area{ "Area", "N.Area",
        { Area, Area2, Area3 },
        {},
        {},
        std::nullopt,
        {}
    };

    ExpectedInterfaceModel syn_c1{ "IAreaFactory", "N.IAreaFactory",
        { Areaf, Areaf2 },
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
    ExpectedMethodModel m1{ "m1", default_method_modifier, ExpectedTypeRefModel{ ElementType::I4 }, {
        ExpectedFormalParameterModel{ "s", parameter_semantics::in, ExpectedTypeRefModel{ ElementType::String } } } };

    ExpectedEventModel e1{ "e1", default_event_modifier, ExpectedTypeRefModel{ ExpectedDelegateRef{ "N.StringListEvent" } } };

    ExpectedMethodModel get_p1{ "get_p1", default_method_modifier, ExpectedTypeRefModel{ ElementType::I4 }, {} };
    ExpectedMethodModel set_p1{ "put_p1", default_method_modifier, std::nullopt, {
        ExpectedFormalParameterModel{ "value", parameter_semantics::in, ExpectedTypeRefModel{ ElementType::I4 } } } };
    ExpectedPropertyModel p1{ "p1", default_property_modifier, ExpectedTypeRefModel{ ElementType::I4 }, get_p1, set_p1 };
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
        reader.read(test_idl, PRINT_ERROR_FLAG);
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
        reader.read(test_idl, PRINT_ERROR_FLAG);
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
    reader.read(test_idl, PRINT_ERROR_FLAG);
    REQUIRE(reader.get_num_syntax_errors() == 0);
    REQUIRE(reader.get_num_semantic_errors() == 0);
}