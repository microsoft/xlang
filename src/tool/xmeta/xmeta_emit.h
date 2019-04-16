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
    class xmeta_emit : public xlang_model_listener
    {
    public:
        xmeta_emit(std::wstring const& assembly_name) 
            : m_assembly_name(assembly_name)
        { };

        void initialize();
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

        mdAssembly tokenAssembly;

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
        HRESULT define_assembly_ref();
        HRESULT define_custom_attribute();
        HRESULT define_event();
        HRESULT define_field();
        HRESULT define_member_ref();
        HRESULT define_method();
        HRESULT define_method_impl();
        HRESULT define_param();
        HRESULT define_property();
        HRESULT define_type_def();
        HRESULT define_generic_param();

        // Not sure if we will use these
        HRESULT define_module_ref();       
        HRESULT define_nested_ref();
        HRESULT define_permission_set();
        HRESULT define_p_invoke_map();       
        HRESULT define_security_attribute_set();

        HRESULT define_type_ref_by_name();
        HRESULT define_user_string();

        static const DWORD dw_enum_typeflag = tdPublic | tdSealed | tdClass | tdAutoLayout | tdWindowsRuntime;  // Flag: Public | Sealed | Class | AutoLayout | WindowsRuntime
    };
}