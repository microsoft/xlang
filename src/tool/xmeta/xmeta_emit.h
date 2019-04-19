#pragma once
#include "xmeta_models.h"
#include "xlang_model_listener.h"
#include <winrt/base.h>
#include <vector>
#include <mscoree.h>
#include <cor.h>

#define RETURN_IF_FAILED(hr)    if(FAILED(hr))  return (hr)
#define SHA1_HASH_ALGO                  0x8004         // ECMA 335 II.23.1.1

namespace xlang::xmeta
{

    // Wide string from narrow string function
    inline std::wstring s2ws(const std::string& as);
    inline std::string type_semantics_to_string(model_ref<type_semantics> const& semantic_type);

    class xmeta_emit : public xlang_model_listener
    {
    public:
        xmeta_emit(std::string const& assembly_name) 
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
        void listen_delegate_model(delegate_model const& model) override;

        void define_method(std::shared_ptr<method_model> const& model, mdTypeDef const& token_def);
        void define_property(std::shared_ptr<property_model> const& model, mdTypeDef const& token_def);
        void define_event(std::shared_ptr<event_model> const& model, mdTypeDef const& token_def);

        void define_method_parameter(formal_parameter_model const& model, mdMethodDef const& token_method, int parameter_index);
    private:
        std::string m_assembly_name;
        winrt::com_ptr<IMetaDataDispenserEx> m_metadata_dispenser;
        winrt::com_ptr<IMetaDataAssemblyEmit> m_metadata_assembly_emitter = nullptr;
        winrt::com_ptr<IMetaDataEmit2> m_metadata_emitter = nullptr;
        winrt::com_ptr<IMetaDataImport> m_metadata_import = nullptr;

        mdAssembly token_assembly;
        mdAssemblyRef token_mscorlib;
        mdTypeRef token_enum;
        mdTypeRef token_value_type;
        mdTypeRef token_delegate;

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
        void define_assembly();
        void define_common_reference_assembly();


        static const DWORD enum_type_flag = tdPublic | tdSealed | tdClass | tdAutoLayout | tdWindowsRuntime;  // Flag: Public | Sealed | Class | AutoLayout | WindowsRuntime
        static const DWORD struct_type_flag = tdPublic | tdSealed | tdClass | tdSequentialLayout | tdWindowsRuntime; // Flags: Public | Sealed | Class |  Sequential
        static const DWORD runtimeclass_type_flag = tdPublic | tdSealed | tdClass | tdWindowsRuntime;                // Flags: class | public | sealed
        static const DWORD interface_type_flag = tdPublic | tdInterface | tdAbstract | tdWindowsRuntime;    // Flags: : Interface | Public | Abstract 
        static const DWORD delegate_type_flag = tdPublic | tdSealed | tdClass | tdWindowsRuntime;           // Flags: Public | Sealed | Class
    };
}