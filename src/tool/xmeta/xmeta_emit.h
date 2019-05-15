#pragma once
#include "xmeta_models.h"
#include "xlang_model_listener.h"
#include <winrt/base.h>
#include <vector>
#include <mscoree.h>
#include <cor.h>
#include <meta_reader.h>
#include <meta_writer.h>
#include <string_view>

namespace xlang::xmeta
{
    inline std::string type_semantics_to_string(model_ref<type_semantics> const& semantic_type);

    class xmeta_emit : public xlang_model_listener
    {
    public:
        explicit xmeta_emit(compilation_unit & assembly_name);
        ~xmeta_emit();

        void save_to_file() const;
        std::vector<uint8_t> save_to_memory() const;

        void listen_namespace_model(std::shared_ptr<namespace_model> const& model) final;
        void listen_class_model(std::shared_ptr<class_model> const& model) final;
        void listen_struct_model(std::shared_ptr<struct_model> const& model) final;
        void listen_interface_model(std::shared_ptr<interface_model> const& model) final;
        void listen_enum_model(std::shared_ptr<enum_model> const& model) final;
        void listen_delegate_model(std::shared_ptr<delegate_model> const& model) final;

    private:
        compilation_unit & xlang_model;
        winrt::com_ptr<IMetaDataDispenserEx> m_metadata_dispenser;
        winrt::com_ptr<IMetaDataAssemblyEmit> m_metadata_assembly_emitter;
        winrt::com_ptr<IMetaDataEmit2> m_metadata_emitter;
        winrt::com_ptr<IMetaDataImport> m_metadata_import;

        std::map<std::string, mdTypeRef> type_references;
        std::map<std::string, mdAssemblyRef> assembly_references;
        meta::reader::Module m_module;

        mdAssembly token_assembly;
        mdAssemblyRef token_mscorlib;
        mdTypeRef token_enum;
        mdTypeRef token_value_type;
        mdTypeRef token_delegate;
        mdTypeRef token_event_registration;

        void define_assembly();
        void define_common_reference_assembly();
        mdTypeDef define_type_def(std::string const& name, DWORD const& type_flag, mdToken token_extend, mdToken token_implements[]);
        
        void define_method(std::shared_ptr<method_model> const& model, DWORD const& method_flag, std::map<std::string_view, mdMethodDef> & method_references, mdTypeDef const& token_def);
        void define_property(std::shared_ptr<property_model> const& model, std::map<std::string_view, mdMethodDef> const& method_references, mdTypeDef const& token_def);
        void define_event(std::shared_ptr<event_model> const& model, std::map<std::string_view, mdMethodDef> const& method_references, mdTypeDef const& token_def);

        mdParamDef define_return(mdTypeDef const& type_def);
        void define_parameters(formal_parameter_model const& model, mdMethodDef const& token_method, uint16_t parameter_index);

        std::optional<xlang::meta::reader::TypeSig> create_type_sig(std::optional<type_ref> const& ref);
        xlang::meta::writer::signature_blob create_method_sig(std::optional<type_ref> const& return_type_ref, std::vector<formal_parameter_model> const& formal_parameters);

        mdTypeRef get_or_define_type_ref(std::string const& ref_name);

        // A generic assembly metadata struct.
        static constexpr ASSEMBLYMETADATA s_genericMetadata =
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
    };
}
