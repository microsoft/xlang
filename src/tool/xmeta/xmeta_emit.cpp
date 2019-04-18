#include "xmeta_emit.h"
#include <winrt/base.h>
#include <locale>
#include <codecvt>
#include <string>

using namespace winrt;

namespace xlang::xmeta
{
    // Hard-coded the mscorlib strong name.
    BYTE s_mscorlibStrongNameBlob[] = { 0xb7, 0x7a, 0x5c, 0x56, 0x19, 0x34, 0xe0, 0x89 };

    void xmeta_emit::initialize() 
    {
        // Getting the meta data dispenser
        check_hresult(CoInitialize(NULL));
    
        // Windows only for now
        check_hresult(CoCreateInstance(CLSID_CorMetaDataDispenser,
            0,
            CLSCTX_INPROC_SERVER,
            IID_IMetaDataDispenser,
            (void **)&m_metadata_dispenser));


        check_hresult(m_metadata_dispenser->DefineScope(
                CLSID_CorMetaDataRuntime,
                0,
                IID_IMetaDataAssemblyEmit,
                (IUnknown **)&m_metadata_assembly_emitter));


        //check_hresult(m_metadata_assembly_emitter->QueryInterface(IID_IMetaDataEmit2, (void **)&m_metadata_emitter));
        //check_hresult(m_metadata_emitter->QueryInterface(IID_IMetaDataImport, (void **)&m_metadata_import));
            // Defining the mscorlib assemblyref
        define_assembly();
        define_common_reference_assembly();
    }

    void xmeta_emit::uninitialize()
    {
        if (m_metadata_dispenser != nullptr)
        {
            m_metadata_dispenser->Release();
        }
        if (m_metadata_assembly_emitter != nullptr)
        {
            m_metadata_assembly_emitter->Release();
        }
        if (m_metadata_emitter != nullptr)
        {
            m_metadata_emitter->Release();
        }
        CoUninitialize();
    }

    void xmeta_emit::saveToFile()
    {
        m_metadata_emitter->Save((m_assembly_name + L".xmeta").c_str(), 0);
    }

    void xmeta_emit::define_assembly()
    {
        check_hresult(m_metadata_assembly_emitter->DefineAssembly(
            NULL,
            0,
            SHA1_HASH_ALGO,
            m_assembly_name.c_str(),
            &(s_genericMetadata),
            0,
            &token_assembly));
    }

    void xmeta_emit::define_common_reference_assembly()
    {
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


    void xmeta_emit::listen_namespace_model(std::shared_ptr<namespace_model> const& model) {};
    
    void xmeta_emit::listen_class_model(std::shared_ptr<class_model> const& model) 
    {
        mdTypeDef token_class_type_def = mdTokenNil;
        mdTypeRef token_local_type_ref = mdTokenNil;

        LPCWSTR class_name = s2ws(model->get_id()).c_str();
        DWORD type_flag = runtimeclass_type_flag;
        // MIDL3 will disable certain flags depending on these conditions
        //if (pRuntimeClass->IsComposable())
        //{
        //    // Turn off the sealed flag
        //    type_flag = dwTypeFlag & (~tdSealed);
        //}

        ////  Empty runtime classes should appear with the abstract flag.
        //if (pRuntimeClass->IsEmpty())
        //{
        //    type_flag = dwTypeFlag | tdAbstract;
        //}

        check_hresult(m_metadata_emitter->DefineTypeDef(
            class_name,
            type_flag,
            mdTokenNil, // Extends (Going to be null until we find out if it is base class)
            mdTokenNil, // Extends (Going to be null until we find out if it is base class)
            &token_class_type_def));

        for (auto const& val : model->get_methods())
        {
            define_class_method(val, token_class_type_def);
        }
        for (auto const& val : model->get_properties())
        {
            define_class_property(val, token_class_type_def);
        }
        for (auto const& val : model->get_events())
        {
            define_class_event(val, token_class_type_def);
        }
    }

    void xmeta_emit::define_class_method(std::shared_ptr<method_model> const& model, mdTypeDef const& token_class_def) 
    {
        LPCWSTR method_name = s2ws(model->get_id()).c_str();

        DWORD method_flag;
        mdMethodDef token_method_def;
        //m_metadata_emitter->DefineMethod(
        //    token_class_def,
        //    method_name,
        //    ,
        //    ,
        //    miRuntime,
        //    &token_method_def);

        /* Define return value */
        mdParamDef token_return; // To be used for attributes later
        std::string return_name = type_semantics_to_string(model->get_return_type()->get_semantic());
        check_hresult(m_metadata_emitter->DefineParam(
            token_method_def,
            0,  // Index Zero represents the return value.
            s2ws(return_name).c_str(),
            0,  // return tyes have no flag set
            (DWORD)-1,  // Ignore dwCPlusTypeFlag
            nullptr,    // No constant value
            0,
            &token_return));

        /* Define formal parameters */
        int index = 1;
        for (auto const& val : model->get_formal_parameters())
        {
            define_method_parameter(val, token_method_def, index);
            index++;
        }

    }

    void xmeta_emit::define_class_property(std::shared_ptr<property_model> const& model, mdTypeDef const& token_class_def) 
    {
        LPCWSTR property_name = s2ws(model->get_id()).c_str();
        
        mdMethodDef token_get_method = mdTokenNil;
        mdMethodDef token_set_method = mdTokenNil;
        DWORD property_flag = 0;
        DWORD c_plus_type_flag;
        std::shared_ptr<method_model> get_method_model = model->get_get_method();
        if (get_method_model != nullptr) //TODO: This case is not suppose to happen
        {
            LPCWSTR get_method_name = s2ws(get_method_model->get_id()).c_str();
            PCCOR_SIGNATURE pv_sig_blob = NULL;
            ULONG cb_sig_blob = 0;
            DWORD impl_flag = 0;
            m_metadata_emitter->DefineMethod(
                token_class_def,
                get_method_name,
                property_flag,
                pv_sig_blob,
                cb_sig_blob,
                miRuntime,
                impl_flag,
                &token_get_method);

        }
        std::shared_ptr<method_model> set_method_model = model->get_set_method();
        if (set_method_model != nullptr)
        {
            LPCWSTR set_method_name = s2ws(set_method_model->get_id()).c_str();
            PCCOR_SIGNATURE pv_sig_blob = NULL;
            ULONG cb_sig_blob = 0;
            DWORD impl_flag = 0;
            m_metadata_emitter->DefineMethod(
                token_class_def,
                set_method_name,
                property_flag,
                pv_sig_blob,
                cb_sig_blob,
                miRuntime,
                impl_flag,
                &token_set_method);
        }

        mdProperty token_property;
        //m_metadata_emitter->DefineProperty(
        //    token_class,
        //    property_name,
        //    property_flag,
        //    , // pvSig
        //    , // cbsig
        //    , // c_plus_type_flag
        //    nullptr,
        //    0,
        //    token_set_method,
        //    token_get_method,
        //    mdTokenNil,
        //    &token_property);
    }

    void xmeta_emit::define_class_event(std::shared_ptr<event_model> const& model, mdTypeDef const& token_class_def)
    {
        LPCWSTR event_name = s2ws(model->get_id()).c_str();

        mdMethodDef token_add_method = mdTokenNil;
        mdMethodDef token_remove_method = mdTokenNil;
        DWORD event_flag = 0;
        DWORD c_plus_type_flag;
        std::shared_ptr<method_model> add_method_model = model->get_add_method();
        if (add_method_model != nullptr) //TODO: This case is not suppose to happen
        {
            LPCWSTR get_method_name = s2ws(add_method_model->get_id()).c_str();
            PCCOR_SIGNATURE pv_sig_blob = NULL;
            ULONG cb_sig_blob = 0;
            DWORD impl_flag = 0;
            m_metadata_emitter->DefineMethod(
                token_class_def,
                get_method_name,
                event_flag,
                pv_sig_blob,
                cb_sig_blob,
                miRuntime,
                impl_flag,
                &token_add_method);

        }
        std::shared_ptr<method_model> remove_method_model = model->get_remove_method();
        if (remove_method_model != nullptr)
        {
            LPCWSTR set_method_name = s2ws(remove_method_model->get_id()).c_str();
            PCCOR_SIGNATURE pv_sig_blob = NULL;
            ULONG cb_sig_blob = 0;
            DWORD impl_flag = 0;
            m_metadata_emitter->DefineMethod(
                token_class_def,
                set_method_name,
                event_flag,
                pv_sig_blob,
                cb_sig_blob,
                miRuntime,
                impl_flag,
                &token_remove_method);
        }

        mdProperty token_event;
        //m_metadata_emitter->DefineEvent(
        //    token_class_def,
        //    event_name,
        //    property_flag,
        //    , // pvSig
        //    , // cbsig
        //    , // c_plus_type_flag
        //    nullptr,
        //    0,
        //    token_add_method,
        //    token_remove_method,
        //    mdTokenNil,
        //    &token_event);
    }
    
    void xmeta_emit::listen_struct_model(std::shared_ptr<struct_model> const& model) 
    {
        LPCWSTR struct_name = s2ws(model->get_id()).c_str();
        mdTypeDef token_struct_type_def;
        check_hresult(m_metadata_emitter->DefineTypeDef(struct_name, struct_type_flag, token_value_type, mdTokenNil, &token_struct_type_def));

        for (std::pair<type_ref, std::string> const& field : model->get_fields())
        {
            mdFieldDef token_field;
            //m_metadata_emitter->DefineField(
            //    token_struct_type_def, 
            //    s2ws(field.second).c_str(), 
            //    fdPublic, 
            //    ,
            //    ,
            //    ELEMENT_TYPE_END,
            //    nullptr,
            //    0,
            //    &token_field);

        }
    }
    
    void xmeta_emit::listen_interface_model(std::shared_ptr<interface_model> const& model) 
    {
        mdTypeDef token_interface_type_def = mdTokenNil;
        mdTypeRef token_local_type_ref = mdTokenNil;

        LPCWSTR interface_name = s2ws(model->get_id()).c_str();

        DWORD type_flag = interface_type_flag;
        //if (pInterface->HasExclusiveToAttribute())
        //{
        //    // Mark this type as NotPublic.
        //    type_flag &= ~tdVisibilityMask;
        //    type_flag |= tdNotPublic;
        //}

        check_hresult(m_metadata_emitter->DefineTypeDef(
            interface_name,
            type_flag,
            mdTokenNil, // Extends (Going to be null until we find out if it is base class)
            mdTokenNil, // Extends (Going to be null until we find out if it is base class)
            &token_interface_type_def));

        for (auto const& val : model->get_methods())
        {
            define_interface_method(val, token_interface_type_def);
        }
        for (auto const& val : model->get_properties())
        {
            define_interface_property(val, token_interface_type_def);
        }
        for (auto const& val : model->get_events())
        {
            define_interface_event(val, token_interface_type_def);
        }
    }


    void xmeta_emit::define_interface_method(std::shared_ptr<method_model> const& model, mdTypeDef const& class_td)
    {
    }

    void xmeta_emit::define_interface_property(std::shared_ptr<property_model> const& model, mdTypeDef const& class_td)
    {
    }

    void xmeta_emit::define_interface_event(std::shared_ptr<event_model> const& model, mdTypeDef const& class_td)
    {

    }

    void xmeta_emit::listen_enum_model(std::shared_ptr<enum_model> const& model) 
    {
        LPCWSTR enum_name = s2ws(model->get_id()).c_str();

        mdTypeDef token_enum_type_def;
        check_hresult(m_metadata_emitter->DefineTypeDef(enum_name, enum_type_flag, 
                                                    token_enum, mdTokenNil, &token_enum_type_def));
        

        for (enum_member const& enum_member : model->get_members())
        {
            std::string enum_member_name = enum_member.get_id();
            auto enum_member_val = enum_member.get_value();
     /*       m_metadata_emitter->DefineField(
                token_enum_type_def,
                enum_member_name.c_str(),

            )*/
        }
    }
    
    void xmeta_emit::listen_delegate_model(delegate_model const& model) 
    {
        LPCWSTR enum_name = s2ws(model.get_id()).c_str();

        mdTypeDef token_delegate_type_def;
        check_hresult(m_metadata_emitter->DefineTypeDef(enum_name, delegate_type_flag,
                                                    token_delegate, mdTokenNil, &token_delegate_type_def));

        /* Define return value */
        mdParamDef token_return; // To be used for attributes later
        std::string return_name = type_semantics_to_string(model.get_return_type()->get_semantic());
        check_hresult(m_metadata_emitter->DefineParam(
            token_delegate_type_def,
            0,  // Index Zero represents the return value.
            s2ws(return_name).c_str(),
            0,  // return tyes have no flag set
            (DWORD)-1,  // Ignore dwCPlusTypeFlag
            nullptr,    // No constant value
            0,
            &token_return));

        /* Define formal parameters */
        int index = 1;
        for (auto const& val : model.get_formal_parameters())
        {
            define_method_parameter(val, token_delegate_type_def, index);
            index++;
        }


    }


    void xmeta_emit::define_method_parameter(formal_parameter_model const& model, mdMethodDef const& token_method_def, int parameter_index)
    {
        LPCWSTR param_name = s2ws(model.get_id()).c_str();

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
        throw EXCEPTION_ACCESS_VIOLATION;
        mdParamDef token_param_def; //To be used for attributes later
        check_hresult(m_metadata_emitter->DefineParam(
            token_method_def,
            0, 
            param_name,
            param_flags,
            (DWORD) - 1,
            nullptr,
            0,
            &token_param_def));
    }
    
    inline std::wstring s2ws(const std::string& as)
    {
        wchar_t* buf = new wchar_t[as.size() * 2 + 2];
        swprintf(buf, L"%S", as.c_str());
        std::wstring rval = buf;
        delete[] buf;
        return rval;
    }

    inline std::string type_semantics_to_string(model_ref<type_semantics> const& semantic_type)
    {
        type_semantics ts = semantic_type.get_resolved_target<type_semantics>();
        if (std::holds_alternative<std::shared_ptr<class_model>>(ts))
        {
            return std::get<std::shared_ptr<class_model>>(ts)->get_id();
        }
        if (std::holds_alternative<std::shared_ptr<enum_model>>(ts))
        {
            return std::get<std::shared_ptr<enum_model>>(ts)->get_id();
        }
        if (std::holds_alternative<std::shared_ptr<interface_model>>(ts))
        {
            return std::get<std::shared_ptr<interface_model>>(ts)->get_id();
        }
        if (std::holds_alternative<std::shared_ptr<struct_model>>(ts))
        {
            return std::get<std::shared_ptr<struct_model>>(ts)->get_id();
        }
        if (std::holds_alternative<simple_type>(ts))
        {
            switch (std::get<simple_type>(ts))
            {
            case simple_type::Boolean:
                return std::string("Boolean");
            case simple_type::String:
                return std::string("String");
            case simple_type::Int8:
                return std::string("Int8");
            case simple_type::Int16:
                return std::string("Int16");
            case simple_type::Int32:
                return std::string("Int32");
            case simple_type::Int64:
                return std::string("Int64");
            case simple_type::Uint8:
                return std::string("Uint8");
            case simple_type::Uint16:
                return std::string("Uint16");
            case simple_type::Uint32:
                return std::string("Uint32");
            case simple_type::Char16:
                return std::string("Char16");
            case simple_type::Guid:
                return std::string("Guid");
            case simple_type::Single:
                return std::string("Single");
            case simple_type::Double:
                return std::string("Double");
            }
        }
        if (std::holds_alternative<object_type>(ts))
        {
            return std::string("Object");
        }
        return nullptr;
    }

}



