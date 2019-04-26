#include "pch.h"
#include <iostream>

#include "XlangParser.h"
#include "XlangLexer.h"
#include "xlang_model_walker.h"
#include "xmeta_emit.h"
#include "xmeta_models.h"
#include "xlang_test_listener.h"
#include "meta_reader.h"
#include "meta_writer.h"
#include "xmeta_idl_reader.h"

using namespace antlr4;

#pragma execution_character_set("utf-8")

using namespace xlang::xmeta;
using namespace xlang::meta::reader;

// In-depth checking of type properties of enums
const TypeAttributes enum_type_attributes()
{
    TypeAttributes result{};
    result.Visibility(TypeVisibility::Public);
    result.Sealed(true);
    result.WindowsRuntime(true);
    return result;
}

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
}
//void test_delegate_type_properties(TypeDef const& delegate_type)
//{
//    auto const& enum_flags = delegate_type.Flags();
//    REQUIRE(enum_flags.value == enum_type_attributes().value); // TODO: Revisit reader flags to enable easy OR'ing together of values
//    // These are basic things that should be true of all enums.
//    // Offload these to a helper test method so we don't have to duplicate this level of paranoia everywhere.
//    REQUIRE(delegate_type.is_enum());
//    REQUIRE(empty(delegate_type.MethodList()));
//    REQUIRE(empty(delegate_type.EventList()));
//    REQUIRE(empty(delegate_type.GenericParam()));
//    REQUIRE(empty(delegate_type.InterfaceImpl()));
//    REQUIRE(empty(delegate_type.MethodImplList()));
//    REQUIRE(empty(delegate_type.PropertyList()));
//}


std::vector<uint8_t> run_and_save_to_memory(std::istringstream &test_idl, std::string_view assembly_name)
{
    xmeta_idl_reader reader{ "" };
    REQUIRE(reader.read(test_idl) == 0);
    xlang::xmeta::xmeta_emit emitter(assembly_name);
    xlang::xmeta::xlang_model_walker walker(reader.get_namespaces(), emitter);

    walker.register_listener(emitter);
    walker.walk();

    xlang::meta::writer::pe_writer writer;
    writer.add_metadata(emitter.save_to_memory());

    return writer.save_to_memory();
}

TEST_CASE("Assemblies metadata") 
{
    constexpr char assembly_name[] = "testidl";
    constexpr char common_assembly_ref[] = "mscorlib";
    xlang::xmeta::xmeta_emit emitter(assembly_name);

    xlang::meta::writer::pe_writer writer;
    writer.add_metadata(emitter.save_to_memory());

    xlang::meta::reader::database db{ writer.save_to_memory() };

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
    std::string assembly_name = "testidl";
    xlang::meta::reader::database db{ run_and_save_to_memory(test_idl, assembly_name) };

    REQUIRE(db.TypeDef.size() == 2);

    auto const& enum_type = db.TypeDef[1];
    REQUIRE(enum_type.TypeNamespace() == "Windows.Test");
    REQUIRE(enum_type.TypeName() == "Color");
    REQUIRE(enum_type.Flags().value == (tdPublic | tdSealed | tdClass | tdAutoLayout | tdWindowsRuntime));
    test_enum_type_properties(enum_type);

    auto const& fields = enum_type.FieldList();
    REQUIRE(size(fields) == 4); // # of enumerators plus one for the value

    {
        auto const& value_field = fields.first[0];
        REQUIRE(value_field.Flags().value == (fdRTSpecialName | fdSpecialName | fdPrivate));
        REQUIRE(value_field.Name() == "value__");
        REQUIRE(!value_field.Constant());
    }

    const std::string_view enum_names[3] = { "Red", "Green", "Blue" };
    const int32_t enum_values[3] = { 0, 1, 2 };
    for (size_t i = 1; i < size(fields); i++)
    {
        auto const& enum_field = fields.first[i];
        REQUIRE(enum_field.Flags().value == (fdHasDefault | fdLiteral | fdStatic | fdPublic));
        REQUIRE(enum_field.Name() == enum_names[i - 1]);
        REQUIRE(enum_field.Constant().ValueInt32() == enum_values[i - 1]);

        auto const& field_sig = enum_field.Signature();
        REQUIRE(empty(field_sig.CustomMod()));
        REQUIRE(field_sig.get_CallingConvention() == CallingConvention::Field);
        REQUIRE(!field_sig.Type().is_szarray());

        auto const& coded_type = std::get<coded_index<TypeDefOrRef>>(field_sig.Type().Type());
        REQUIRE(coded_type.type() == TypeDefOrRef::TypeRef);

        auto const& type_ref = coded_type.TypeRef();
        REQUIRE(type_ref.TypeName() == "Color");
        REQUIRE(type_ref.TypeNamespace() == "Windows.Test");
        REQUIRE(type_ref.ResolutionScope().type() == ResolutionScope::Module);
    }
}

TEST_CASE("Delegate metadata")
{
    std::istringstream test_idl{ R"(
        namespace Windows.Test
        {
            delegate Int32 testdelegate(Int32 c, Double d);
        }
    )" };
    std::string assembly_name = "testidl";

    std::map<std::string_view, std::shared_ptr<namespace_model>, std::less<>> v;
    std::string ns_name("Windows.Test");
    std::shared_ptr<namespace_model> ns(new namespace_model(ns_name, 0, assembly_name, nullptr));
    std::shared_ptr<namespace_body_model> ns_bm = std::make_shared<namespace_body_model>(namespace_body_model(ns));
    std::string delegate_name("testdelegate");
    std::shared_ptr<delegate_model> delegate_model_m = std::make_shared<delegate_model>(delegate_name, 0, assembly_name, ns_bm, type_ref(simple_type::Int32));
    formal_parameter_model param1("c", 0, assembly_name, parameter_semantics::in, type_ref(simple_type::Int32));
    formal_parameter_model param2("d", 0, assembly_name, parameter_semantics::in, type_ref(simple_type::Double));

    delegate_model_m->add_formal_parameter(std::move(param1));
    delegate_model_m->add_formal_parameter(std::move(param2));
    ns_bm->add_delegate(delegate_model_m);
    ns->add_namespace_body(ns_bm);

    v["test"] = ns;
    xlang::xmeta::xmeta_emit emitter(assembly_name);
    xlang::xmeta::xlang_model_walker walker(v, emitter);
    walker.walk();

    xlang::meta::writer::pe_writer writer;
    writer.add_metadata(emitter.save_to_memory());
    xlang::meta::reader::database db{ writer.save_to_memory() };

    REQUIRE(db.TypeDef.size() == 2);

    auto const& delegate_type = db.TypeDef[1];
    REQUIRE(delegate_type.TypeNamespace() == "Windows.Test");
    REQUIRE(delegate_type.TypeName() == "testdelegate");
    REQUIRE(delegate_type.Flags().value == (tdPublic | tdSealed | tdClass | tdWindowsRuntime));
    
    auto const& delegate_constructor = db.MethodDef[0];
    REQUIRE(delegate_constructor.Name() == ".ctor");
    REQUIRE(delegate_constructor.Parent().TypeName() == "testdelegate");
    auto const& coded_type = std::get<ElementType>(delegate_constructor.Signature().ReturnType().Type().Type());
    REQUIRE(std::holds_alternative<ElementType>(delegate_constructor.Signature().ReturnType().Type().Type()));
    //REQUIRE(coded_type == ElementType::Void);
    for (auto const& param : delegate_constructor.Signature().Params())
    {

        auto const& coded_type = std::get<ElementType>(param.Type().Type());
        REQUIRE(coded_type == ElementType::Void);
    }

    //db.Param;
    //auto const& delegate_invoke = db.MethodDef[1];
    //REQUIRE(delegate_constructor.Name() == "Invoke");
    //REQUIRE(delegate_constructor.Parent().TypeName() == "testdelegate");
    //REQUIRE(delegate_constructor.Flags == (mdVirtual | fdSpecialName | mdHideBySig));
    //REQUIRE(delegate_constructor.ImplFlags == miRuntime);


  /*  for (auto const& method : db.Param)
    {
        std::cout << method. << std::endl;
    }*/
}