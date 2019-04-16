#include "xmeta_emit.h";

namespace xlang::xmeta
{
    void xmeta_emit::initialize() {
        // Getting the meta data dispenser
        HRESULT hr;
        if (SUCCEEDED(CoInitialize(NULL)))
        {
            // Windows only for now
            hr = CoCreateInstance(CLSID_CorMetaDataDispenser,
                0,
                CLSCTX_INPROC_SERVER,
                IID_IMetaDataDispenser,
                (void **)&m_metadata_dispenser);
            if (FAILED(hr)) 
            {
                goto finish;
            }

            HRESULT hr = m_metadata_dispenser->DefineScope(
                CLSID_CorMetaDataRuntime,
                0,
                IID_IMetaDataAssemblyEmit,
                (IUnknown **)&m_metadata_assembly_emitter);

            if (FAILED(hr)) 
            {
                goto finish;
            }

            m_metadata_assembly_emitter->QueryInterface(IID_IMetaDataEmit2, (void **)&m_metadata_emitter);
            if (FAILED(hr)) 
            {
                goto finish;
            }

            define_assembly();
        }
        return;

    finish:
        m_metadata_dispenser->Release();
        m_metadata_assembly_emitter->Release();
        m_metadata_emitter->Release();
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
            &tokenAssembly));
    }

    HRESULT xmeta_emit::define_assembly_ref()
    {
        return 0;
    }

    HRESULT xmeta_emit::define_type_def()
    {
        return 0;
    }

    void xmeta_emit::listen_namespace_model(std::shared_ptr<namespace_model> const& model) {};
    void xmeta_emit::listen_class_model(std::shared_ptr<class_model> const& model) {};
    void xmeta_emit::listen_struct_model(std::shared_ptr<struct_model> const& model) {};
    void xmeta_emit::listen_interface_model(std::shared_ptr<interface_model> const& model) {};
    void xmeta_emit::listen_enum_model(std::shared_ptr<enum_model> const& model) {};
    void xmeta_emit::listen_delegate_model(std::shared_ptr<delegate_model> const& model) {};

    void xmeta_emit::listen_method_model(std::shared_ptr<method_model> const& model) {};
    void xmeta_emit::listen_property_model(std::shared_ptr<property_model> const& model) {};
    void xmeta_emit::listen_event_model(std::shared_ptr<event_model> const& model) {};
}



