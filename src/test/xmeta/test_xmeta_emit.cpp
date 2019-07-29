#include "pch.h"
#include "test_metadata_infrastructure.h"

void ValidateTypesInMetadata(std::istringstream & testIdl, std::vector<std::shared_ptr<ExpectedType>> const& fileTypes)
{
    xlang::meta::reader::database db{ run_and_save_to_memory(testIdl, "testidl") };

    auto expectedType(fileTypes.begin());
    REQUIRE(fileTypes.size() == db.TypeDef.size() - 1);

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
                    }, 
                    interface_method_attributes()
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
            },
            std::vector<ExpectedMethod> {
                ExpectedMethod("get_property1", "S1", {}, interface_method_property_attributes()),
                ExpectedMethod("put_property1", ElementType::Void, { ExpectedParam{"value", "S1"} }, interface_method_property_attributes())
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
            },
            std::vector<ExpectedMethod> {
                ExpectedMethod("add_Changed", "Foundation.EventRegistrationToken", { ExpectedParam{"value", "StringListEvent"} }, interface_method_event_attributes()),
                ExpectedMethod("remove_Changed", ElementType::Void, { ExpectedParam{"value", "Foundation.EventRegistrationToken"} }, interface_method_event_attributes())
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
                ExpectedMethod("Paint3", ElementType::Void, {}, interface_method_attributes())
            }
        ),
        std::make_unique<ExpectedInterface>("IComboBox", std::vector<std::string> {"M.IControl3", "IControl", "IControl2"},
            std::vector<ExpectedProperty> {
                ExpectedProperty("property1", ElementType::I4)
            }
        ),
        std::make_unique<ExpectedInterface>("IControl", std::vector<std::string> {"M.IControl3"},
            std::vector<ExpectedMethod> {
                ExpectedMethod("Paint", ElementType::Void, {}, interface_method_attributes())
            }
        ),
        std::make_unique<ExpectedInterface>("IControl2", std::vector<std::string> {"M.IControl3"},
            std::vector<ExpectedMethod> {
                ExpectedMethod("Paint2", ElementType::Void, {}, interface_method_attributes())
            }
        )
    };

    ValidateTypesInMetadata(test_idl, fileTypes);
}

TEST_CASE("Interface type metadata")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            interface IControl
            {
                void Paint();
            }
    
            interface IComboBox requires M.IControl3, IControl
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
                ExpectedMethod("Paint3", ElementType::Void, {}, interface_method_attributes())
            }
        ),
        std::make_unique<ExpectedInterface>("IComboBox", std::vector<std::string> {"M.IControl3", "IControl"},
            std::vector<ExpectedProperty> {
                ExpectedProperty("property1", ElementType::I4)
            }
        ),
        std::make_unique<ExpectedInterface>("IControl",
            std::vector<ExpectedMethod> {
                ExpectedMethod("Paint", ElementType::Void, {}, interface_method_attributes())
            }
        )
    };

    ValidateTypesInMetadata(test_idl, fileTypes);
}

TEST_CASE("Runtime class constructor metadata")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            runtimeclass C1
            {
                C1();

                C1(Int32 param1, Int32 param2);
            }
        }
    )" };

    ExpectedClass c1{ "C1", {}, {}, {},
        { 
            { ExpectedMethod{ ".ctor", ElementType::Void, { ExpectedParam{ "param1", ElementType::I4 }, ExpectedParam{ "param2", ElementType::I4 } }, constructor_method_attributes(), method_impl_runtime_attributes() }, "" },
            { ExpectedMethod{ ".ctor", ElementType::Void, {}, constructor_method_attributes(), method_impl_runtime_attributes() }, "" }
        },
        class_type_attributes()
    };

    ExpectedInterface ic1{ "IC1",
        { ExpectedMethod{ ".ctor", ElementType::Void, {}, constructor_method_attributes() } }
    };

    ExpectedInterface ic1f{ "IC1Factory",
        { ExpectedMethod{ "CreateInstance", "C1", { ExpectedParam{ "param1", ElementType::I4 }, ExpectedParam{ "param2", ElementType::I4 } }, constructor_method_attributes() } }
    };

    std::vector<std::shared_ptr<ExpectedType>> fileTypes =
    {
        std::make_unique<ExpectedClass>(c1),
        std::make_unique<ExpectedInterface>(ic1),
        std::make_unique<ExpectedInterface>(ic1f)
    };
    ValidateTypesInMetadata(test_idl, fileTypes);
}


TEST_CASE("Runtimeclass method metadata")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            struct S1 {}    
            runtimeclass C1 
            {
                S1 Draw(E1 param1);
                static S1 DrawStatic(E1 param1);
            }
            enum E1 {}  
        }
    )" };

    ExpectedClass c1{ "C1", {}, {}, {}, {
        { ExpectedMethod{ "Draw", "S1", { ExpectedParam{ "param1", "E1" } }, class_method_attributes()}, "IC1.Draw" }} 
    };
    ExpectedStruct s1{ "S1", std::vector<ExpectedStructField> {} };
    ExpectedInterface ic1{ "IC1",
        { ExpectedMethod{ "Draw", "S1", { ExpectedParam("param1", "E1") }, interface_method_attributes()}}
    };
    ExpectedInterface ic1_statics{ "IC1Statics",
        { ExpectedMethod{ "DrawStatic", "S1", { ExpectedParam("param1", "E1") }, interface_method_attributes()}}
    };
    ExpectedEnum e1{ "E1", {} };

    std::vector<std::shared_ptr<ExpectedType>> fileTypes =
    {
        std::make_unique<ExpectedClass>(c1),
        std::make_unique<ExpectedStruct>(s1),
        std::make_unique<ExpectedInterface>(ic1),
        std::make_unique<ExpectedInterface>(ic1_statics),
        std::make_unique<ExpectedEnum>(e1)
    };

    ValidateTypesInMetadata(test_idl, fileTypes);
}

TEST_CASE("Runtimeclass property metadata")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            struct S1 {}    
            runtimeclass C1 
            {
                S1 property1;
                static S1 property2;
            }
            enum E1 {}  
        }
    )" };

    ExpectedClass c1{ "C1", {}, 
        { ExpectedProperty{"property1", "S1"} },
        {}, 
        {
            { ExpectedMethod{ "get_property1", "S1", {}, class_method_property_attributes()}, "IC1.get_property1" },
            { ExpectedMethod{"put_property1", ElementType::Void, { ExpectedParam{"value", "S1"} }, class_method_property_attributes()}, "IC1.put_property1" }
        } 
    };
    ExpectedStruct s1{ "S1", std::vector<ExpectedStructField> {} };
    ExpectedInterface ic1{ "IC1",
        { ExpectedProperty("property1", "S1") }
    };
    ExpectedInterface ic1_statics{ "IC1Statics",
        { ExpectedProperty("property2", "S1") }
    };
    ExpectedEnum e1{ "E1", {} };


    std::vector<std::shared_ptr<ExpectedType>> fileTypes =
    {
        std::make_unique<ExpectedClass>(c1),
        std::make_unique<ExpectedStruct>(s1),
        std::make_unique<ExpectedInterface>(ic1),
        std::make_unique<ExpectedInterface>(ic1_statics),
        std::make_unique<ExpectedEnum>(e1)
    };

    ValidateTypesInMetadata(test_idl, fileTypes);
}

TEST_CASE("Runtimeclass event metadata")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            delegate void StringListEvent(Int32 sender);
            runtimeclass C1 
            {
                event StringListEvent Changed;
                static event StringListEvent Changed2;
            }
        }
    )" };

    ExpectedClass c1{ "C1", {}, {},
        { ExpectedEvent{ "Changed", "StringListEvent" } },
        {
            { ExpectedMethod("add_Changed", "Foundation.EventRegistrationToken", { ExpectedParam{"value", "StringListEvent"} }, interface_method_event_attributes()), "IC1.add_Changed" },
            { ExpectedMethod("remove_Changed", ElementType::Void, { ExpectedParam{"value", "Foundation.EventRegistrationToken"} }, interface_method_event_attributes()), "IC1.remove_Changed" }
        }
    };

    ExpectedInterface ic1{ "IC1",
        { ExpectedEvent{ "Changed", "StringListEvent"} }
    };
    ExpectedInterface ic1_static{ "IC1Statics",
        { ExpectedEvent{ "Changed2", "StringListEvent"} }
    };

    ExpectedDelegate d1{ "StringListEvent", ElementType::Void,
        { ExpectedParam{ "sender", ElementType::I4 } }
    };

    std::vector<std::shared_ptr<ExpectedType>> fileTypes =
    {
        std::make_unique<ExpectedClass>(c1),
        std::make_unique<ExpectedInterface>(ic1),
        std::make_unique<ExpectedInterface>(ic1_static),
        std::make_unique<ExpectedDelegate>(d1)
    };

    ValidateTypesInMetadata(test_idl, fileTypes);
}

TEST_CASE("Runtimeclass requires interfacebase metadata")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            delegate void StringListEvent(Int32 sender);
            interface I1
            {
                event StringListEvent Changed;
                void m0();
                Int32 property1;
            }

            runtimeclass C1 requires I1
            {
            }
        }
    )" };

    ExpectedClass c1{ "C1",
        { "I1" },
        { ExpectedProperty{ "property1" , ElementType::I4 } },
        { ExpectedEvent{ "Changed", "StringListEvent" } },
        {
            { ExpectedMethod("add_Changed", "Foundation.EventRegistrationToken", { ExpectedParam{"value", "StringListEvent"} }, interface_method_event_attributes()), "I1.add_Changed" },
            { ExpectedMethod("remove_Changed", ElementType::Void, { ExpectedParam{"value", "Foundation.EventRegistrationToken"} }, interface_method_event_attributes()), "I1.remove_Changed" },
            { ExpectedMethod("m0", ElementType::Void, {}, class_method_attributes()), "I1.m0" },
            { ExpectedMethod("get_property1", ElementType::I4, {}, class_method_property_attributes()), "I1.get_property1" },
            { ExpectedMethod("put_property1", ElementType::Void, { ExpectedParam{"value", ElementType::I4} }, class_method_property_attributes()), "I1.put_property1" }
        }
    };

    ExpectedInterface i1{ "I1",
            { ExpectedProperty("property1", ElementType::I4) },
            { ExpectedEvent("Changed", "StringListEvent") },
            {
                ExpectedMethod("add_Changed", "Foundation.EventRegistrationToken", { ExpectedParam{"value", "StringListEvent"} }, interface_method_event_attributes()),
                ExpectedMethod("remove_Changed", ElementType::Void, { ExpectedParam{"value", "Foundation.EventRegistrationToken"} }, interface_method_event_attributes()),
                ExpectedMethod("m0", ElementType::Void, {}, interface_method_attributes()),
                ExpectedMethod("get_property1", ElementType::I4, {}, interface_method_property_attributes()),
                ExpectedMethod("put_property1", ElementType::Void, { ExpectedParam{"value", ElementType::I4} }, interface_method_property_attributes())
            }
    };

    ExpectedDelegate d1{ "StringListEvent", ElementType::Void,
       { ExpectedParam("sender", ElementType::I4) }
    };

    std::vector<std::shared_ptr<ExpectedType>> fileTypes =
    {
        std::make_unique<ExpectedClass>(c1),
        std::make_unique<ExpectedInterface>(i1),
        std::make_unique<ExpectedDelegate>(d1)
    };
    ValidateTypesInMetadata(test_idl, fileTypes);
}

TEST_CASE("Runtimeclass extend class base metadata")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            runtimeclass C0
            {
            }

            runtimeclass C1 : C0
            {
                Int32 Draw(Int32 param1);
            }
        }
    )" };

    ExpectedClass c0{ "C0", {}, {}, {}, {}};

    ExpectedClass c1{ "C1", {}, {}, {}, {
        { ExpectedMethod{ "Draw", ElementType::I4, { ExpectedParam{ "param1", ElementType::I4 } }, class_method_attributes() }, "IC1.Draw" },
    }, "N.C0"};

    ExpectedInterface ic1{ "IC1",
        { ExpectedMethod{ "Draw", ElementType::I4, { ExpectedParam{ "param1", ElementType::I4 } }, interface_method_attributes() } }
};

    std::vector<std::shared_ptr<ExpectedType>> fileTypes =
    {
        std::make_unique<ExpectedClass>(c0),
        std::make_unique<ExpectedClass>(c1),
        std::make_unique<ExpectedInterface>(ic1)
    };
    ValidateTypesInMetadata(test_idl, fileTypes);
}

TEST_CASE("Static class metadata")
{
    std::istringstream test_idl{ R"(
        namespace N
        {
            static runtimeclass C1
            {
                static Int32 Draw(Int32 param1);
            }
        }
    )" };

    ExpectedClass c1{ "C1", {}, {}, {}, {}, static_class_type_attributes() };

    ExpectedInterface ic1{ "IC1Statics",
        { ExpectedMethod{ "Draw", ElementType::I4, { ExpectedParam{ "param1", ElementType::I4 } }, interface_method_attributes() } } 
    };

    std::vector<std::shared_ptr<ExpectedType>> fileTypes =
    {
        std::make_unique<ExpectedClass>(c1),
        std::make_unique<ExpectedInterface>(ic1)
    };
    ValidateTypesInMetadata(test_idl, fileTypes);
}

TEST_CASE("Put breakpoint here to see test output before closing")
{
    return;
}