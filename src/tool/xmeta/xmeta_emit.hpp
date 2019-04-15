#pragma once
#include <namespace_model.h>

#include <vector>
#include <mscoree.h>
#include <cor.h>

#define RETURN_IF_FAILED(hr)    if(FAILED(hr))  return (hr)
#define SHA1_HASH_ALGO                  0x8004         // ECMA 335 II.23.1.1

namespace xlang::xmeta
{
    class xmeta_emit
    {
    public:
        xmeta_emit(std::vector<namespace_model> const& namespaces, std::wstring const& assembly_name) 
            : m_namespaces(namespaces), m_assembly_name(assembly_name)
        { };

        void emit_metadata();

    private:
        std::wstring m_assembly_name;
        std::vector<namespace_model> m_namespaces;
        IMetaDataDispenserEx *m_metadata_dispenser;
        IMetaDataAssemblyEmit *m_metadata_assembly_emitter;
        IMetaDataEmit2 *m_metadata_emitter;

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
    };
}