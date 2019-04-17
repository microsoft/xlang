#pragma once
#include "xmeta_models.h"
#include "xlang_model_listener.h"

#include <vector>
#include <mscoree.h>
#include <cor.h>

#define RETURN_IF_FAILED(hr)    if(FAILED(hr))  return (hr)
#define SHA1_HASH_ALGO                  0x8004         // ECMA 335 II.23.1.1

namespace xlang::xmeta
{
    inline std::wstring s2ws(const std::string& as);
    class xmeta_emit : public xlang_model_listener
    {
    public:
        xmeta_emit(std::wstring const& assembly_name) 
            : m_assembly_name(assembly_name)
        { };

        HRESULT initialize();
        void uninitialize();
        void saveToFile();

        void listen_namespace_model(std::shared_ptr<namespace_model> const& model) override;
        void listen_class_model(std::shared_ptr<class_model> const& model) override;
        void listen_struct_model(std::shared_ptr<struct_model> const& model) override;
        void listen_interface_model(std::shared_ptr<interface_model> const& model) override;
        void listen_enum_model(std::shared_ptr<enum_model> const& model) override;
        void listen_delegate_model(std::shared_ptr<delegate_model> const& model) override;

        void listen_method_model(std::shared_ptr<method_model> const& model) override;
        void listen_property_model(std::shared_ptr<property_model> const& model) override;
        void listen_event_model(std::shared_ptr<event_model> const& model) override;

    private:
        std::wstring m_assembly_name;
        IMetaDataDispenserEx *m_metadata_dispenser = nullptr;
        IMetaDataAssemblyEmit *m_metadata_assembly_emitter = nullptr;
        IMetaDataEmit2 *m_metadata_emitter = nullptr;

        mdAssembly token_assembly;
        mdAssemblyRef token_mscorlib;
        mdTypeRef token_enum;

        // A generic assembly metadata struct.
        const ASSEMBLYMETADATA s_genericMetadata =
        {
            // usMajorVersion - Unspecified major version
            0xFF,
            // usMinorVersion - Unspecified minor version
            0xFF,
            // usRevisionNumber - Unspecified revision number
            0xFF,
            // usBuildNumber - Unspecified build number
            0xFF,
            // szLocale - locale indepedence
            nullptr,
            // cbLocale
            0,
            // rProcessor - Processor independence
            nullptr,
            // ulProcessor
            0,
            // rOS - OS independence
            nullptr,
            // ulOS
            0,
        };

        // Windows specific methods
        HRESULT define_assembly();
        HRESULT define_common_reference_assembly();


        static const DWORD dw_enum_typeflag = tdPublic | tdSealed | tdClass | tdAutoLayout | tdWindowsRuntime;  // Flag: Public | Sealed | Class | AutoLayout | WindowsRuntime
    };
}