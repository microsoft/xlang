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

            // Defining the mscorlib assemblyref
            RETURN_IF_FAILED(define_assembly());
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
    }


    void xmeta_emit::listen_namespace_model(std::shared_ptr<namespace_model> const& model) {};
    void xmeta_emit::listen_class_model(std::shared_ptr<class_model> const& model) {};
    void xmeta_emit::listen_struct_model(std::shared_ptr<struct_model> const& model) {};
    void xmeta_emit::listen_interface_model(std::shared_ptr<interface_model> const& model) {};

    void xmeta_emit::listen_enum_model(std::shared_ptr<enum_model> const& model) {
        std::string enum_name = model->get_id();

        mdTypeDef token_enum_type_def;
        m_metadata_emitter->DefineTypeDef(s2ws(enum_name).c_str(), dw_enum_typeflag, token_enum, mdTokenNil, &token_enum_type_def);
       
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

    void xmeta_emit::listen_method_model(std::shared_ptr<method_model> const& model) {};
    void xmeta_emit::listen_property_model(std::shared_ptr<property_model> const& model) {};
    void xmeta_emit::listen_event_model(std::shared_ptr<event_model> const& model) {};

    inline std::wstring s2ws(const std::string& as)
    {
        wchar_t* buf = new wchar_t[as.size() * 2 + 2];
        swprintf(buf, L"%S", as.c_str());
        std::wstring rval = buf;
        delete[] buf;
        return rval;
    }
}



