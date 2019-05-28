#include "xmeta_models.h"
#include "xmeta_emit.h"
#include <winrt/base.h>
#include <locale>
#include <codecvt>
#include <string>
#include <comutil.h>
#include <iostream>
#include <codecvt>
#include <locale>
#include <variant>

using namespace winrt;
using namespace xlang::meta::reader;
using namespace xlang::meta::writer;
using namespace xlang::xmeta;

// TODO: TypeRefEmitter
// method impl
// class/interface impl

namespace
{
    std::wstring s2ws(const std::string& as)
    {
        return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}.from_bytes(as);
    }

    // Useful helpers deal with meta::reader types, so use them primarily and translate to/from cor tokens for IMetaDataEmit and friends.
    mdModule to_token(Module module)
    {
        return module.index() + 1 | mdtModule;
    }

    mdTypeRef to_token(TypeRef type_ref)
    {
        return type_ref.index() + 1 | mdtTypeRef;
    }

    mdAssembly to_token(Assembly assembly)
    {
        return assembly.index() + 1 | mdtAssembly;
    }

    mdAssemblyRef to_token(AssemblyRef assembly_ref)
    {
        return assembly_ref.index() + 1 | mdtAssemblyRef;
    }

    Module to_Module(mdModule token)
    {
        XLANG_ASSERT(TypeFromToken(token) == mdtModule);
        return { nullptr, RidFromToken(token) - 1 };
    }

    TypeRef to_TypeRef(mdTypeRef token)
    {
        XLANG_ASSERT(TypeFromToken(token) == mdtTypeRef);
        return { nullptr, RidFromToken(token) - 1 };
    }

    TypeDef to_TypeDef(mdTypeDef token)
    {
        XLANG_ASSERT(TypeFromToken(token) == mdtTypeDef);
        return { nullptr, RidFromToken(token) - 1 };
    }

    Assembly to_Assembly(mdAssembly token)
    {
        XLANG_ASSERT(TypeFromToken(token) == mdtAssembly);
        return { nullptr, RidFromToken(token) - 1 };
    }

    AssemblyRef to_AssemblyRef(mdAssemblyRef token)
    {
        XLANG_ASSERT(TypeFromToken(token) == mdtAssemblyRef);
        return { nullptr, RidFromToken(token) - 1 };
    }

    ElementType to_ElementType(enum_semantics arg)
    {
        switch (arg)
        {
        case enum_semantics::Int8:
            return ElementType::I1;
        case enum_semantics::UInt8:
            return ElementType::U1;
        case enum_semantics::Int16:
            return ElementType::I2;
        case enum_semantics::UInt16:
            return ElementType::U2;
        case enum_semantics::Int32:
            return ElementType::I4;
        case enum_semantics::UInt32:
            return ElementType::U4;
        case enum_semantics::Int64:
            return ElementType::I8;
        case enum_semantics::UInt64:
            return ElementType::U8;
        default:
            XLANG_ASSERT(false);
            return ElementType::Void;
        }
    }

    ElementType to_ElementType(simple_type arg)
    {
        switch (arg)
        {
        case simple_type::String:
            return ElementType::String;
        case simple_type::Int8:
            return ElementType::I1;
        case simple_type::UInt8:
            return ElementType::U1;
        case simple_type::Int16:
            return ElementType::I2;
        case simple_type::UInt16:
            return ElementType::U2;
        case simple_type::Int32:
            return ElementType::I4;
        case simple_type::UInt32:
            return ElementType::U4;
        case simple_type::Int64:
            return ElementType::I8;
        case simple_type::UInt64:
            return ElementType::U8;
        case simple_type::Char16:
            return ElementType::Char;
        case simple_type::Single:
            return ElementType::R4;
        case simple_type::Double:
            return ElementType::R8;
        case simple_type::Boolean:
            return ElementType::Boolean;
        default:
            XLANG_ASSERT(false);
            return ElementType::Void;
        }
    }

    std::variant<std::string, simple_type, object_type> to_simple_type_or_id(model_ref<type_semantics> const& semantic_type)
    {
        assert(semantic_type.is_resolved());
        type_semantics const& ts = semantic_type.get_resolved_target();
        if (std::holds_alternative<std::shared_ptr<xlang::meta::reader::TypeDef>>(ts))
        {
            auto const& type = std::get<std::shared_ptr<xlang::meta::reader::TypeDef>>(ts);
            return std::string(type->TypeNamespace()) + "." + std::string(type->TypeName());
        }
        if (std::holds_alternative<std::shared_ptr<class_model>>(ts))
        {
            return std::get<std::shared_ptr<class_model>>(ts)->get_fully_qualified_id();
        }
        if (std::holds_alternative<std::shared_ptr<delegate_model>>(ts))
        {
            return std::get<std::shared_ptr<delegate_model>>(ts)->get_fully_qualified_id();
        }
        if (std::holds_alternative<std::shared_ptr<enum_model>>(ts))
        {
            return std::get<std::shared_ptr<enum_model>>(ts)->get_fully_qualified_id();
        }
        if (std::holds_alternative<std::shared_ptr<interface_model>>(ts))
        {
            return std::get<std::shared_ptr<interface_model>>(ts)->get_fully_qualified_id();
        }
        if (std::holds_alternative<std::shared_ptr<struct_model>>(ts))
        {
            return std::get<std::shared_ptr<struct_model>>(ts)->get_fully_qualified_id();
        }
        if (std::holds_alternative<simple_type>(ts))
        {
            return std::get<simple_type>(ts);
        }
        if (std::holds_alternative<object_type>(ts))
        {
            return std::get<object_type>(ts);
        }
        assert(false);
        return "";
    }

    std::string remove_extension(const std::string& filename) {
        size_t lastdot = filename.find_last_of(".");
        if (lastdot == std::string::npos) return filename;
        return filename.substr(0, lastdot);
    }

}

namespace xlang::xmeta
{
    xmeta_emit::xmeta_emit(compilation_unit & model)
        : xlang_model{ model }
    {
        // Getting the meta data dispenser
        check_hresult(CoInitialize(nullptr));

        // Windows only for now
        check_hresult(CoCreateInstance(CLSID_CorMetaDataDispenser,
            nullptr,
            CLSCTX_INPROC,
            IID_IMetaDataDispenser,
            m_metadata_dispenser.put_void()));

        {
            variant_t version{ L"WindowsRuntime 1.4" };
            check_hresult(m_metadata_dispenser->SetOption(MetaDataRuntimeVersion, &version.GetVARIANT()));
        }

        check_hresult(m_metadata_dispenser->DefineScope(
            CLSID_CorMetaDataRuntime,
            0,
            IID_IMetaDataAssemblyEmit,
            reinterpret_cast<IUnknown**>(m_metadata_assembly_emitter.put_void())));

        check_hresult(m_metadata_assembly_emitter->QueryInterface(IID_IMetaDataEmit2, m_metadata_emitter.put_void()));
        check_hresult(m_metadata_emitter->QueryInterface(IID_IMetaDataImport, m_metadata_import.put_void()));
        // Defining the mscorlib assemblyref
        define_assembly();
        define_common_reference_assembly();

        mdModule token_current_module;
        check_hresult(m_metadata_import->GetModuleFromScope(&token_current_module));
        m_module = to_Module(token_current_module);
    };

    xmeta_emit::~xmeta_emit()
    {
        CoUninitialize();
    };


    // This doesn't really output in a PE format
    void xmeta_emit::save_to_file() const
    {
        m_metadata_emitter->Save((s2ws(remove_extension(xlang_model.m_assembly)) + L".xmeta").c_str(), 0);
    }

    std::vector<uint8_t> xmeta_emit::save_to_memory() const
    {
        DWORD save_size;
        m_metadata_emitter->GetSaveSize(CorSaveSize::cssAccurate, &save_size);
        std::vector<uint8_t> metadata;
        metadata.resize(save_size);
        check_hresult(m_metadata_emitter->SaveToMemory(metadata.data(), save_size));
        return metadata;
    }

    void xmeta_emit::define_assembly()
    {
        constexpr DWORD sha1_hash_algo{ 0x8004 };
        check_hresult(m_metadata_assembly_emitter->DefineAssembly(
            nullptr,
            0,
            sha1_hash_algo,
            s2ws(xlang_model.m_assembly).c_str(),
            &s_genericMetadata,
            afContentType_WindowsRuntime,
            &token_assembly));
        check_hresult(m_metadata_emitter->SetModuleProps(s2ws(xlang_model.m_assembly).c_str()));
    }

    void xmeta_emit::define_common_reference_assembly()
    {
        // Hard-coded the mscorlib strong name.
        constexpr BYTE s_mscorlibStrongNameBlob[] = { 0xb7, 0x7a, 0x5c, 0x56, 0x19, 0x34, 0xe0, 0x89 };
        check_hresult(m_metadata_assembly_emitter->DefineAssemblyRef(
            s_mscorlibStrongNameBlob,
            sizeof(s_mscorlibStrongNameBlob),
            L"mscorlib",
            &s_genericMetadata,
            nullptr,
            0,
            afContentType_Default,
            &token_mscorlib));
        check_hresult(m_metadata_emitter->DefineTypeRefByName(token_mscorlib, L"System.Enum", &token_enum));
        check_hresult(m_metadata_emitter->DefineTypeRefByName(token_mscorlib, L"System.ValueType", &token_value_type));
        check_hresult(m_metadata_emitter->DefineTypeRefByName(token_mscorlib, L"System.MulticastDelegate", &token_delegate));
    }

    mdTypeDef xmeta_emit::define_type_def(std::string const& name, DWORD const& type_flag, mdToken token_extend, mdToken token_implements[])
    {
        auto wname = s2ws(name);
        mdTypeDef token_typedef;
        check_hresult(m_metadata_emitter->DefineTypeDef(
            wname.c_str(),
            type_flag,
            token_extend, 
            token_implements,
            &token_typedef));

        auto const& iter = type_references.find(name);
        if (iter == type_references.end())
        {
            mdTypeRef md_ref;
            m_metadata_emitter->DefineTypeRefByName(to_token(m_module), wname.c_str(), &md_ref);
            type_references.emplace(name, md_ref);
        }
        return token_typedef;
    }

    void xmeta_emit::listen_namespace_model(std::shared_ptr<namespace_model> const& /*model*/) {};
    
    void xmeta_emit::listen_class_model(std::shared_ptr<class_model> const& /*model*/) 
    {
        //mdTypeDef token_class_type_def = mdTokenNil;
        //mdTypeRef token_local_type_ref = mdTokenNil;

        //std::string class_name = model->get_id();
        //DWORD type_flag = runtimeclass_type_flag;
        //// MIDL3 will disable certain flags depending on these conditions
        ////if (pRuntimeClass->IsComposable())
        ////{
        ////    // Turn off the sealed flag
        ////    type_flag = dwTypeFlag & (~tdSealed);
        ////}

        //////  Empty runtime classes should appear with the abstract flag.
        ////if (pRuntimeClass->IsEmpty())
        ////{
        ////    type_flag = dwTypeFlag | tdAbstract;
        ////}

        //mdTypeDef implements[] = { mdTokenNil };

        //auto class_type_def = define_type_def(
        //    class_name,
        //    type_flag,
        //    mdTypeRefNil, // Extends (Going to be null until we find out if it is base class)
        //    implements // Implements (Going to be null until we find out if it is base class));
        //);

        //// TODO: Class base and interface implements

        //for (auto const& val : model->get_methods())
        //{
        //    define_method(val, token_class_type_def);
        //}
        //for (auto const& val : model->get_properties())
        //{
        //    define_property(val, token_class_type_def);
        //}
        //for (auto const& val : model->get_events())
        //{
        //    define_event(val, token_class_type_def);
        //}
    }

    void xmeta_emit::define_method(std::shared_ptr<method_model> const& model, DWORD const& method_flag, std::map<std::string_view, mdMethodDef> & method_defs, mdTypeDef const& token_def)
    {
        std::wstring method_name = s2ws(model->get_id());

        signature_blob method_sig = create_method_sig(model->get_return_type(), model->get_formal_parameters());

        mdMethodDef token_method_def;
        m_metadata_emitter->DefineMethod(
            token_def,
            method_name.c_str(),
            method_flag,
            method_sig.data(),
            method_sig.size(),
            0,
            0,
            &token_method_def);
        method_defs.emplace(model->get_id(), token_method_def);
        define_return(token_method_def);

        /* Define formal parameters */
        uint16_t index = 1;
        for (auto const& val : model->get_formal_parameters())
        {
            define_parameters(val, token_method_def, index);
            index++;
        }
    }

    void xmeta_emit::define_property(std::shared_ptr<property_model> const& model, std::map<std::string_view, mdMethodDef> const& method_defs, mdTypeDef const& token_def)
    {
        std::wstring property_name = s2ws(model->get_id());
        mdMethodDef token_get_method = mdTokenNil;
        mdMethodDef token_set_method = mdTokenNil;
        mdTypeDef implements[] = { mdTokenNil };

        std::shared_ptr<method_model> get_method_model = model->get_get_method();
        assert(get_method_model != nullptr); //There must always be a get
        {
            auto const& iter = method_defs.find(get_method_model->get_id());
            assert(iter != method_defs.end());
            token_get_method = iter->second;
        }

        std::shared_ptr<method_model> set_method_model = model->get_set_method();
        if (set_method_model != nullptr)
        {
            auto const& iter = method_defs.find(set_method_model->get_id());
            assert(iter != method_defs.end());
            token_set_method = iter->second;
        }
        
        signature_blob property_sig;
        property_sig.add_signature(PropertySig{ create_type_sig(model->get_type()).value() });

        mdProperty token_property;
        m_metadata_emitter->DefineProperty(
            token_def,
            property_name.c_str(),
            0,
            property_sig.data(),
            property_sig.size(),
            (DWORD)-1, // c_plus_type_flag
            nullptr,
            0,
            token_set_method,
            token_get_method,
            implements,
            &token_property);
    }


    void xmeta_emit::define_event(std::shared_ptr<event_model> const& model, std::map<std::string_view, mdMethodDef> const& method_defs, mdTypeDef const& token_def)
    {
        std::wstring event_name = s2ws(model->get_id());
        mdMethodDef token_add_method = mdTokenNil;
        mdMethodDef token_remove_method = mdTokenNil;
        mdTypeDef implements[] = { mdTokenNil };

        std::shared_ptr<method_model> const& add_method_model = model->get_add_method();
        assert(add_method_model != nullptr); //There must always be a add
        {
            auto const& iter = method_defs.find(add_method_model->get_id());
            assert(iter != method_defs.end());
            token_add_method = iter->second;
        }

        std::shared_ptr<method_model> const& remove_method_model = model->get_remove_method();
        assert(remove_method_model != nullptr); //There must always be a remove
        {
            auto const& iter = method_defs.find(remove_method_model->get_id());
            assert(iter != method_defs.end());
            token_remove_method = iter->second;
        }
        auto const& target = model->get_type().get_semantic().get_resolved_target();
        assert(std::holds_alternative<std::shared_ptr<delegate_model>>(target));
        auto const& dm = std::get<std::shared_ptr<delegate_model>>(target);

        mdTypeRef event_type_ref = get_or_define_type_ref(dm->get_fully_qualified_id(), xlang_model.m_assembly);
        mdEvent token_event;
        m_metadata_emitter->DefineEvent(
            token_def,
            event_name.c_str(),
            0,
            event_type_ref,
            token_add_method,
            token_remove_method,
            mdTokenNil,
            implements,
            &token_event);
    }
    
    void xmeta_emit::listen_interface_model(std::shared_ptr<interface_model> const& model)
    {
        auto const& interface_name = model->get_fully_qualified_id();
        
        std::vector<mdTypeRef> implements;
        for (auto const& interface_ref : model->get_interface_bases())
        {
            auto const& val = std::get<std::shared_ptr<interface_model>>(interface_ref.get_semantic().get_resolved_target());
            implements.emplace_back(get_or_define_type_ref(val->get_fully_qualified_id(), xlang_model.m_assembly));
        }
        implements.emplace_back(mdTokenNil);
        
        //TODO: When we have attributes, if it has a single ExclusiveToAttribute. Then it must not be tdPublic. 
        //NOTE: tdWindowsRuntime flag maybe removed later to indicate that this is not WinRT
        static constexpr DWORD interface_type_flag = tdInterface | tdPublic | tdAbstract | tdWindowsRuntime;
        auto token_interface_type_def = define_type_def(interface_name, interface_type_flag, mdTokenNil, implements.data());
        std::map<std::string_view, mdMethodDef> method_defs;
        for (auto const& val : model->get_methods())
        {
            if (val->get_method_association() == method_association::None)
            {
                static constexpr DWORD method_flag = mdPublic | mdVirtual | mdHideBySig | mdAbstract | mdNewSlot /*| mdInstance*/;
                define_method(val, method_flag, method_defs, token_interface_type_def);
            }
            else if (val->get_method_association() == method_association::Property)
            {
                static constexpr DWORD method_flag = mdPublic | mdVirtual | mdHideBySig | mdNewSlot | mdAbstract | mdSpecialName;
                define_method(val, method_flag, method_defs, token_interface_type_def);
            }
            else if (val->get_method_association() == method_association::Event)
            {
                static constexpr DWORD method_flag = mdPublic | mdFinal | mdVirtual | mdHideBySig | mdNewSlot | mdSpecialName;
                define_method(val, method_flag, method_defs, token_interface_type_def);
            }
            else
            {
                assert(false);
            }
        }
        for (auto const& val : model->get_properties())
        {
            define_property(val, method_defs, token_interface_type_def);
        }
        for (auto const& val : model->get_events())
        {
            define_event(val, method_defs, token_interface_type_def);
        }
    }

    void xmeta_emit::listen_struct_model(std::shared_ptr<struct_model> const& model) 
    {
        auto const& struct_name = model->get_fully_qualified_id();
        //NOTE: tdWindowsRuntime flag maybe removed later to indicate that this is not WinRT
        static constexpr DWORD struct_type_flag = tdPublic | tdSealed | tdClass | tdSequentialLayout | tdWindowsRuntime;
        mdTypeDef implements[] = { mdTokenNil };
        auto token_struct_type_def = define_type_def(struct_name, struct_type_flag, token_value_type, implements);

        static constexpr DWORD struct_field_flag = fdPublic;
        for (std::pair<type_ref, std::string> const& struct_member : model->get_fields())
        {
            assert(struct_member.first.get_semantic().is_resolved());
            auto const& field_name = s2ws(struct_member.second);

            signature_blob field_signature;
            field_signature.add_signature(FieldSig{ create_type_sig(struct_member.first).value() });
            mdFieldDef field_token;
            check_hresult(m_metadata_emitter->DefineField(token_struct_type_def,
                field_name.c_str(),
                struct_field_flag,
                field_signature.data(),
                field_signature.size(),
                ELEMENT_TYPE_END,
                nullptr,
                0,
                &field_token));
        }
    }

    void xmeta_emit::listen_enum_model(std::shared_ptr<enum_model> const& model) 
    {
        auto const& type_name = model->get_fully_qualified_id();
        //NOTE: tdWindowsRuntime flag maybe removed later to indicate that this is not WinRT
        static constexpr DWORD enum_type_flag = tdPublic | tdSealed | tdClass | tdAutoLayout | tdWindowsRuntime;
        mdTypeDef implements[] = { mdTokenNil };
        auto token_enum_type_def = define_type_def(type_name, enum_type_flag, token_enum, implements);
        
        static constexpr DWORD enum_value_flag = fdRTSpecialName | fdSpecialName | fdPrivate;
        ElementType const underlying_type = to_ElementType(model->get_type());
        signature_blob value_signature;
        value_signature.add_signature(FieldSig{ TypeSig{underlying_type} });
        mdFieldDef field_token;
        check_hresult(m_metadata_emitter->DefineField(token_enum_type_def,
            L"value__",
            enum_value_flag,
            value_signature.data(),
            value_signature.size(),
            ELEMENT_TYPE_END,
            nullptr,
            0,
            &field_token));

        auto iter = type_references.find(type_name);
        if (iter == type_references.end())
        {
            throw_invalid("Failed to find TypeRef for: " + type_name);
        }
        TypeRef const& enum_type_ref = to_TypeRef(iter->second);

        static constexpr DWORD enumerator_flag = fdHasDefault | fdLiteral | fdStatic | fdPublic;
        signature_blob enumerator_signature;
        enumerator_signature.add_signature(FieldSig{ TypeSig{ElementType::ValueType, enum_type_ref.coded_index<TypeDefOrRef>()} });

        for (enum_member const& enum_member : model->get_members())
        {
            call(enum_member.get_resolved_value(), [&](auto const& val)
                {
                    using val_type = std::decay_t<decltype(val)>;
                    static_assert(std::is_integral_v<val_type>);
                    auto const& name = s2ws(enum_member.get_id());
                    check_hresult(m_metadata_emitter->DefineField(token_enum_type_def,
                        name.c_str(),
                        enumerator_flag,
                        enumerator_signature.data(),
                        enumerator_signature.size(),
                        static_cast<DWORD>(underlying_type),
                        &val,
                        static_cast<ULONG>(sizeof(val_type)),
                        &field_token));
                });
        }
    }
    
    void xmeta_emit::listen_delegate_model(std::shared_ptr<delegate_model> const& model)
    {
        auto const& type_name = model->get_fully_qualified_id();
        //NOTE: tdWindowsRuntime flag maybe removed later to indicate that this is not WinRT
        static constexpr DWORD delegate_type_flag = tdPublic | tdSealed | tdClass | tdWindowsRuntime;
        mdTypeDef implements[] = { mdTokenNil };
        mdTypeDef token_delegate_type_def = define_type_def(type_name, delegate_type_flag, token_delegate, implements);

        auto iter = type_references.find(type_name);
        if (iter == type_references.end())
        {
            throw_invalid("Failed to find TypeRef for: " + type_name);
        }

        // Constructor
        static constexpr DWORD delegate_constructor_flag = mdPrivate | mdSpecialName | mdRTSpecialName | mdHideBySig;
        signature_blob delegate_constructor_sig;
        std::vector<ParamSig> param_types = { ParamSig{ TypeSig{ElementType::Object} }, ParamSig{ TypeSig{ElementType::I} } };
        delegate_constructor_sig.add_signature(MethodDefSig{ RetTypeSig{ std::nullopt }, param_types }); // nullopt means returntype is void
        mdMethodDef token_delegate_constructor_def;
        check_hresult(m_metadata_emitter->DefineMethod(
            token_delegate_type_def,
            L".ctor",
            delegate_constructor_flag,
            delegate_constructor_sig.data(),
            delegate_constructor_sig.size(),
            0,
            miRuntime,
            &token_delegate_constructor_def));

        mdParamDef constructor_param_1;
        check_hresult(m_metadata_emitter->DefineParam(
            token_delegate_constructor_def,
            1, 
            L"object",
            0,
            (DWORD)-1,
            nullptr,
            0,
            &constructor_param_1));

        mdParamDef constructor_param_2;
        check_hresult(m_metadata_emitter->DefineParam(
            token_delegate_constructor_def,
            2, 
            L"method",
            0,
            (DWORD)-1, 
            nullptr,
            0,
            &constructor_param_2));
        
        // Invoke
        signature_blob delegate_invoke_sig = create_method_sig(model->get_return_type(), model->get_formal_parameters());
         
        static constexpr DWORD delegate_invoke_flag = mdPublic | mdVirtual | mdSpecialName | mdHideBySig;
        mdMethodDef token_delegate_invoke_def;
        check_hresult(m_metadata_emitter->DefineMethod(
            token_delegate_type_def,
            L"Invoke",
            delegate_invoke_flag,
            delegate_invoke_sig.data(),
            delegate_invoke_sig.size(),
            0,
            miRuntime,
            &token_delegate_invoke_def));

        /** Defining parameters and return **/
        /* Define return value */
        // To be used for attributes later
        /* mdParamDef token_return = */define_return(token_delegate_invoke_def);

        /* Define formal parameters */
        uint16_t index = 1;
        for (auto const& val : model->get_formal_parameters())
        {
            define_parameters(val, token_delegate_invoke_def, index);
            index++;
        }
    }

    mdParamDef xmeta_emit::define_return(mdTypeDef const& type_def)
    {
        /* Define return value */
        mdParamDef token_param_return;
        check_hresult(m_metadata_emitter->DefineParam(
            type_def,
            0,  // Index Zero represents the return value.
            L"returnVal",
            0,  // return tyes have no flag set
            (DWORD)-1,  // Ignore dwCPlusTypeFlag
            nullptr,    // No constant value
            0,
            &token_param_return));
        return token_param_return;
    }

    void xmeta_emit::define_parameters(formal_parameter_model const& model, mdMethodDef const& token_method_def, uint16_t parameter_index)
    {
        std::wstring param_name = s2ws(model.get_id());

        DWORD param_flags = 0;
        if (parameter_index != 0)
        {
            if (model.get_semantic() == parameter_semantics::in)
            {
                param_flags |= pdIn;
            }
            if (model.get_semantic() == parameter_semantics::out)
            {
                param_flags |= pdOut;
            }
        }
        mdParamDef token_param_def; //To be used for attributes later
        check_hresult(m_metadata_emitter->DefineParam(
            token_method_def,
            parameter_index,
            param_name.c_str(),
            param_flags,
            (DWORD) - 1,
            nullptr,
            0,
            &token_param_def));
    }

    mdTypeRef xmeta_emit::get_or_define_type_ref(std::string const& ref_name, std::string const& assembly_ref)
    {
        mdToken token_assembly_ref;

        // Defining assembly ref if we haven't define one. If assembly ref is us, we don't need to do it.
        if (assembly_ref != xlang_model.m_assembly)
        {
            auto const& ref = assembly_references.find(assembly_ref);
            if (ref == assembly_references.end())
            {
                check_hresult(m_metadata_assembly_emitter->DefineAssemblyRef(
                    nullptr,
                    0,
                    s2ws(assembly_ref).c_str(),
                    &s_genericMetadata,
                    nullptr,
                    0,
                    afContentType_Default,
                    &token_assembly_ref));
                assembly_references.emplace(assembly_ref, token_assembly_ref);
            }
            else
            {
                token_assembly_ref = ref->second;
            }
        }
        else
        {
            token_assembly_ref = to_token(m_module);
        }

        // Defining our type ref if we haven't.
        auto const& iter = type_references.find(ref_name);
        if (iter == type_references.end())
        {
            mdTypeRef md_ref;
            m_metadata_emitter->DefineTypeRefByName(token_assembly_ref, s2ws(ref_name).c_str(), &md_ref);
            type_references.emplace(ref_name, md_ref);
            return md_ref;
        }
        return iter->second;
    }

    std::optional<TypeSig> xmeta_emit::create_type_sig(std::optional<type_ref> const& ref)
    {
        if (ref) // Return type is not void
        {
            std::variant<std::string, simple_type, object_type> semantic = to_simple_type_or_id(ref->get_semantic());
            if (std::holds_alternative<std::string>(semantic))
            {
                std::string return_name = std::get<std::string>(semantic);
                TypeRef type_ref;

                // if it holds a meta reader type def, this must be coming from another assembly. 
                if (std::holds_alternative<std::shared_ptr<xlang::meta::reader::TypeDef>>(ref->get_semantic().get_resolved_target()))
                {
                    auto const& type_def = std::get<std::shared_ptr<xlang::meta::reader::TypeDef>>(ref->get_semantic().get_resolved_target());
                    type_ref = to_TypeRef(get_or_define_type_ref(return_name, std::string(type_def->get_database().Assembly[0].Name())));
                }
                else
                {
                    type_ref = to_TypeRef(get_or_define_type_ref(return_name, xlang_model.m_assembly));
                }
                return TypeSig{ ElementType::ValueType, type_ref.coded_index<TypeDefOrRef>() };
            }
            else if (std::holds_alternative<object_type>(semantic))
            {
                return TypeSig{ ElementType::Object };
            }
            else // holds simple type
            {
                return TypeSig{ to_ElementType(std::get<simple_type>(semantic)) };
            }
        }
        return std::nullopt;
    };

    signature_blob xmeta_emit::create_method_sig(std::optional<type_ref> const& return_type_ref, std::vector<formal_parameter_model> const& formal_parameters)
    {
        signature_blob method_sig;
        std::vector<ParamSig> param_sigs;
        RetTypeSig return_sig = RetTypeSig{ create_type_sig(return_type_ref) };
        for (formal_parameter_model const& val : formal_parameters)
        {
            ParamSig param_sig = ParamSig{ create_type_sig(val.get_type()).value() };
            param_sigs.emplace_back(param_sig);
        }
        method_sig.add_signature(MethodDefSig{ return_sig, param_sigs });
        return method_sig;
    }
}



