#include "xmeta_emit.h";

#include <locale>
#include <codecvt>
#include <string>

namespace xlang::xmeta
{
    // Hard-coded the mscorlib strong name.
    BYTE s_mscorlibStrongNameBlob[] = { 0xb7, 0x7a, 0x5c, 0x56, 0x19, 0x34, 0xe0, 0x89 };

    HRESULT xmeta_emit::initialize() {
        // Getting the meta data dispenser
        if (SUCCEEDED(CoInitialize(NULL)))
        {
            // Windows only for now
            RETURN_IF_FAILED(CoCreateInstance(CLSID_CorMetaDataDispenser,
                0,
                CLSCTX_INPROC_SERVER,
                IID_IMetaDataDispenser,
                (void **)&m_metadata_dispenser));


            RETURN_IF_FAILED(m_metadata_dispenser->DefineScope(
                CLSID_CorMetaDataRuntime,
                0,
                IID_IMetaDataAssemblyEmit,
                (IUnknown **)&m_metadata_assembly_emitter));


            RETURN_IF_FAILED(m_metadata_assembly_emitter->QueryInterface(IID_IMetaDataEmit2, (void **)&m_metadata_emitter));
            RETURN_IF_FAILED( m_metadata_emitter->QueryInterface(IID_IMetaDataImport, (void **)&m_metadata_import));
            // Defining the mscorlib assemblyref
            RETURN_IF_FAILED(define_assembly());
            RETURN_IF_FAILED(define_common_reference_assembly());
        }
        return S_OK;
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

    HRESULT xmeta_emit::define_assembly()
    {
        RETURN_IF_FAILED(m_metadata_assembly_emitter->DefineAssembly(
            NULL,
            0,
            SHA1_HASH_ALGO,
            m_assembly_name.c_str(),
            &(s_genericMetadata),
            0,
            &token_assembly));
    }

    HRESULT xmeta_emit::define_common_reference_assembly()
    {
        RETURN_IF_FAILED(m_metadata_assembly_emitter->DefineAssemblyRef(
            s_mscorlibStrongNameBlob,
            sizeof(s_mscorlibStrongNameBlob),
            L"mscorlib",
            &s_genericMetadata,
            nullptr,
            0,
            afContentType_Default,
            &token_mscorlib));

        RETURN_IF_FAILED(m_metadata_emitter->DefineTypeRefByName(token_mscorlib, L"System.Enum", &token_enum));
        RETURN_IF_FAILED(m_metadata_emitter->DefineTypeRefByName(token_mscorlib, L"System.ValueType", &token_value_type));
    }


    void xmeta_emit::listen_namespace_model(std::shared_ptr<namespace_model> const& model) {};
    
    void xmeta_emit::listen_class_model(std::shared_ptr<class_model> const& model) {
        mdTypeDef token_class_type_def = mdTokenNil;
        mdTypeRef token_local_type_ref = mdTokenNil;

        LPCWSTR class_name = s2ws(model->get_id()).c_str();

        // MIDL3 will disable certain flags depending on these conditions
        //if (pRuntimeClass->IsComposable())
        //{
        //    // Turn off the sealed flag
        //    dwTypeFlag = dwTypeFlag & (~tdSealed);
        //}

        ////  Empty runtime classes should appear with the abstract flag.
        //if (pRuntimeClass->IsEmpty())
        //{
        //    dwTypeFlag = dwTypeFlag | tdAbstract;
        //}

        m_metadata_emitter->DefineTypeDef(
            class_name,
            c_dwRuntimeClassTypeFlag,
            mdTokenNil, // Extends (Going to be null until we find out if it is base class)
            mdTokenNil, // Extends (Going to be null until we find out if it is base class)
            &token_class_type_def);

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
    };
    
    void xmeta_emit::listen_struct_model(std::shared_ptr<struct_model> const& model) {
        LPCWSTR struct_name = s2ws(model->get_id()).c_str();
        mdTypeDef token_struct_type_def;
        m_metadata_emitter->DefineTypeDef(struct_name, c_dwStructTypeFlag, token_value_type, mdTokenNil, &token_struct_type_def);

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
    };
    
    void xmeta_emit::listen_interface_model(std::shared_ptr<interface_model> const& model) {};

    void xmeta_emit::listen_enum_model(std::shared_ptr<enum_model> const& model) {
        LPCWSTR enum_name = s2ws(model->get_id()).c_str();

        mdTypeDef token_enum_type_def;
        m_metadata_emitter->DefineTypeDef(enum_name, enum_type_flag, token_enum, mdTokenNil, &token_enum_type_def);
        

        for (enum_member const& enum_member : model->get_members())
        {
            std::string enum_member_name = enum_member.get_id();
            auto enum_member_val = enum_member.get_value();
     /*       m_metadata_emitter->DefineField(
                token_enum_type_def,
                enum_member_name.c_str(),

            )*/
        }
    };
    
    
    void xmeta_emit::listen_delegate_model(delegate_model const& model) {};

    void xmeta_emit::define_class_method(std::shared_ptr<method_model> const& model, mdTypeDef const& class_td) {
        LPCWSTR method_name = s2ws(model->get_id()).c_str();

        DWORD method_flag;
        mdMethodDef token_method_def;
        //m_metadata_emitter->DefineMethod(
        //    class_td,
        //    method_name,
        //    ,
        //    ,
        //    miRuntime,
        //    &token_method_def);
    };

    void xmeta_emit::define_class_property(std::shared_ptr<property_model> const& model, mdTypeDef const& token_class) {
        LPCWSTR property_name = s2ws(model->get_id()).c_str();
        std::shared_ptr<method_model> get_method_model = model->get_get_method();
        mdMethodDef token_get_method = mdTokenNil;
        mdMethodDef token_set_method = mdTokenNil;
        DWORD property_flag;
        DWORD c_plus_type_flag;
        if (get_method_model != nullptr) //TODO: This case is not suppose to happen
        {
            //m_metadata_emitter->DefineMethod(
            //    token_class,
            //    method_name,
            //    ,
            //    ,
            //    miRuntime,
            //    &token_get_method);
            
        }
        std::shared_ptr<method_model> set_method_model = model->get_set_method();
        if (set_method_model != nullptr)
        {
            //m_metadata_emitter->DefineMethod(
            //    token_class,
            //    method_name,
            //    ,
            //    ,
            //    miRuntime,
            //    &token_set_method);
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
    };
    
    void xmeta_emit::define_class_event(std::shared_ptr<event_model> const& model, mdTypeDef const& class_td) {
    
    };

    inline std::wstring s2ws(const std::string& as)
    {
        wchar_t* buf = new wchar_t[as.size() * 2 + 2];
        swprintf(buf, L"%S", as.c_str());
        std::wstring rval = buf;
        delete[] buf;
        return rval;
    }
}



