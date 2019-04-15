#include "xmeta_emit.hpp";
namespace xlang::xmeta
{
    void xmeta_emit::emit_metadata() {
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
            if (FAILED(hr)) {
                goto finish;
            }

            HRESULT hr = m_metadata_dispenser->DefineScope(
                CLSID_CorMetaDataRuntime,
                0,
                IID_IMetaDataAssemblyEmit,
                (IUnknown **)&m_metadata_assembly_emitter);
            if (FAILED(hr)) {
                goto finish;
            }

            m_metadata_assembly_emitter->QueryInterface(IID_IMetaDataEmit2, (void **)&m_metadata_emitter);
            if (FAILED(hr)) {
                goto finish;
            }

            define_assembly();

        
        }


        finish:
            m_metadata_dispenser->Release();
            m_metadata_assembly_emitter->Release();
            m_metadata_emitter->Release();
            CoUninitialize();
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
}



