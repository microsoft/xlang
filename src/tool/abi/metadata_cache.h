#pragma once

#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "meta_reader.h"
#include "task_group.h"
#include "type_name.h"
#include "type_names.h"

struct metadata_cache;

struct metadata_type
{
    virtual std::string_view clr_namespace() const = 0;
    virtual std::string_view clr_full_name() const = 0;
    virtual std::string_view mangled_name() const = 0;
    virtual std::string_view generic_param_mangled_name() const = 0;

    // NOTE: For easier validation, we try and generate header files as identical to MIDLRT as possible. The primary
    //       issue there is that the files MIDLRT generates are highly dependent on the structure of the .idl file
    //       being processed. Thus, it's more accurate to say that we try and mimic WINMDIDL, which orders many things
    //       alphabetically by its fully qualified "idl name."
    virtual std::string_view idl_name() const = 0;
};

inline bool operator<(metadata_type const& lhs, metadata_type const& rhs) noexcept
{
    return lhs.idl_name() < rhs.idl_name();
}

struct typedef_base : metadata_type
{
    typedef_base(xlang::meta::reader::TypeDef const& type) :
        m_type(type),
        m_clrFullName(::clr_full_name(type)),
        m_mangledName(::mangled_name<false>(type)),
        m_genericParamMangledName(::mangled_name<true>(type))
    {
    }

    virtual std::string_view clr_namespace() const override
    {
        return m_type.TypeNamespace();
    }

    virtual std::string_view clr_full_name() const override
    {
        return m_clrFullName;
    }

    virtual std::string_view mangled_name() const override
    {
        return m_mangledName;
    }

    virtual std::string_view generic_param_mangled_name() const override
    {
        // Only generic instantiations should be used as generic params
        XLANG_ASSERT(!is_generic(m_type));
        return m_genericParamMangledName;
    }

    xlang::meta::reader::TypeDef const& type() const noexcept
    {
        return m_type;
    }

protected:

    xlang::meta::reader::TypeDef m_type;

    // These strings are initialized by the base class
    std::string m_clrFullName;
    std::string m_mangledName;
    std::string m_genericParamMangledName;
};

struct enum_type final : typedef_base
{
    using typedef_base::typedef_base;

    virtual std::string_view idl_name() const override
    {
        // Enum names identically match their CLR name in idl files
        return m_clrFullName;
    }
};

struct struct_member
{
    xlang::meta::reader::Field field;
    metadata_type const* type;
};

struct struct_type final : typedef_base
{
    using typedef_base::typedef_base;

    virtual std::string_view idl_name() const override
    {
        // Struct names identically match their CLR name in idl files
        return m_clrFullName;
    }

    std::vector<struct_member> members;
};

struct delegate_type final : typedef_base
{
    delegate_type(xlang::meta::reader::TypeDef const& type) :
        typedef_base(type)
    {
        m_abiName.reserve(1 + type.TypeName().length());
        m_abiName.push_back('I');
        m_abiName += type.TypeName();
    }

    virtual std::string_view idl_name() const override
    {
        return m_abiName;
    }

private:

    std::string m_abiName;
};

struct interface_type final : typedef_base
{
    using typedef_base::typedef_base;

    virtual std::string_view idl_name() const override
    {
        // Interface names identically match their CLR name in idl files
        return m_clrFullName;
    }
};

struct class_type final : typedef_base
{
    using typedef_base::typedef_base;

    virtual std::string_view idl_name() const override
    {
        // Class names identically match their CLR name in idl files
        return m_clrFullName;
    }
};

struct generic_inst final : metadata_type
{
    generic_inst(metadata_type const* genericType, std::vector<metadata_type const*> genericParams) :
        m_genericType(genericType),
        m_genericParams(std::move(genericParams))
    {
        m_clrFullName = genericType->clr_full_name();
        m_clrFullName.push_back('<');

        m_mangledName = genericType->mangled_name();

        m_idlName = genericType->idl_name();
        m_idlName.push_back('<');

        std::string_view prefix;
        for (auto param : genericParams)
        {
            m_clrFullName += prefix;
            m_clrFullName += param->clr_full_name();

            m_mangledName.push_back('_');
            m_mangledName += param->generic_param_mangled_name();

            m_idlName += prefix;
            m_idlName += param->idl_name();
            prefix = ", ";
        }

        m_clrFullName.push_back('>');
        m_idlName.push_back('>');
    }

    virtual std::string_view clr_namespace() const override
    {
        return m_genericType->clr_namespace();
    }

    virtual std::string_view clr_full_name() const override
    {
        return m_clrFullName;
    }

    virtual std::string_view mangled_name() const override
    {
        return m_mangledName;
    }

    virtual std::string_view generic_param_mangled_name() const override
    {
        return m_mangledName;
    }

    metadata_type const* generic_type() const noexcept
    {
        return m_genericType;
    }

    std::vector<metadata_type const*> const& generic_params() const noexcept
    {
        return m_genericParams;
    }

    virtual std::string_view idl_name() const override
    {
        return m_idlName;
    }

private:

    metadata_type const* m_genericType;
    std::vector<metadata_type const*> m_genericParams;
    std::string m_clrFullName;
    std::string m_mangledName;
    std::string m_idlName;
};

struct element_type final : metadata_type
{
    element_type(
        std::string_view clrName,
        std::string_view logicalName,
        std::string_view abiName,
        std::string_view mangledName) :
        m_clrName(clrName),
        m_logicalName(logicalName),
        m_abiName(abiName),
        m_mangledName(mangledName)
    {
    }

    static element_type const& from_type(xlang::meta::reader::ElementType type);

    virtual std::string_view clr_namespace() const override
    {
        return {};
    }

    virtual std::string_view clr_full_name() const override
    {
        return m_clrName;
    }

    virtual std::string_view mangled_name() const override
    {
        xlang::throw_invalid("ElementType values don't have mangled names");
    }

    virtual std::string_view generic_param_mangled_name() const override
    {
        return m_mangledName;
    }

    virtual std::string_view idl_name() const override
    {
        // TODO: Not 100% accurate... E.g. many things are uppercase (DOUBLE, FLOAT, etc.) and INT64 vs. __int64
        return m_abiName;
    }

private:

    xlang::meta::reader::ElementType m_type;
    std::string_view m_clrName;
    std::string_view m_logicalName;
    std::string_view m_abiName;
    std::string_view m_mangledName;
};

struct system_type final : metadata_type
{
    system_type(std::string_view clrName, std::string_view cppName) :
        m_clrName(clrName),
        m_cppName(cppName)
    {
    }

    static system_type const& from_name(std::string_view typeName);

    virtual std::string_view clr_namespace() const override
    {
        // Currently all mapped types from the System namespace have no namespace
        return {};
    }

    virtual std::string_view clr_full_name() const override
    {
        return m_clrName;
    }

    virtual std::string_view mangled_name() const override
    {
        xlang::throw_invalid("ElementType values don't have mangled names");
    }

    virtual std::string_view generic_param_mangled_name() const override
    {
        // Currently all mangled names from the System namespace match their C++ name (i.e. no '_' etc. in the name)
        return m_cppName;
    }

    virtual std::string_view idl_name() const override
    {
        return m_cppName;
    }

private:

    std::string_view m_clrName;
    std::string_view m_cppName;
};

struct mapped_type final : metadata_type
{
    mapped_type(xlang::meta::reader::TypeDef const& type) :
        m_type(type),
        m_clrFullName(::clr_full_name(type))
    {
    }

    virtual std::string_view clr_namespace() const override
    {
        return m_type.TypeNamespace();
    }

    virtual std::string_view clr_full_name() const override
    {
        return m_clrFullName;
    }

    virtual std::string_view mangled_name() const override
    {
        // Currently no mapped type names have any characters that would make it so that the mangled name would be
        // different than the type name (i.e. no underscore, etc.)
        return m_type.TypeName();
    }

    virtual std::string_view generic_param_mangled_name() const override
    {
        return m_type.TypeName();
    }

    virtual std::string_view idl_name() const override
    {
        return m_type.TypeName();
    }

private:

    xlang::meta::reader::TypeDef m_type;
    std::string m_clrFullName;
};

struct api_contract
{
    type_name name;
    std::uint32_t current_version;
};

inline bool operator<(api_contract const& lhs, api_contract const& rhs)
{
    return lhs.name < rhs.name;
}

struct type_cache
{
    // Definitions
    std::vector<std::reference_wrapper<enum_type>> enums;
    std::vector<std::reference_wrapper<struct_type>> structs;
    std::vector<std::reference_wrapper<delegate_type>> delegates;
    std::vector<std::reference_wrapper<interface_type>> interfaces;
    std::vector<std::reference_wrapper<class_type>> classes;

    // Dependencies
    std::set<std::string_view> dependent_namespaces;
    std::map<std::string_view, generic_inst> generic_instantiations;
    std::set<std::reference_wrapper<typedef_base const>> external_dependencies;
    std::set<std::reference_wrapper<typedef_base const>> internal_dependencies;
};

struct namespace_types
{
    std::vector<enum_type> enums;
    std::vector<struct_type> structs;
    std::vector<delegate_type> delegates;
    std::vector<interface_type> interfaces;
    std::vector<class_type> classes;
    std::set<api_contract> contracts;
};

struct metadata_cache
{
    std::map<std::string_view, namespace_types> namespaces;

    metadata_cache(xlang::meta::reader::cache const& c);

    type_cache process_namespaces(std::initializer_list<std::string_view> targetNamespaces);

    metadata_type const* try_find(std::string_view typeNamespace, std::string_view typeName) const
    {
        if (typeNamespace == system_namespace)
        {
            return &system_type::from_name(typeName);
        }

        auto nsItr = m_typeTable.find(typeNamespace);
        if (nsItr != m_typeTable.end())
        {
            auto nameItr = nsItr->second.find(typeName);
            if (nameItr != nsItr->second.end())
            {
                return &nameItr->second;
            }
        }

        return nullptr;
    }

    metadata_type const& find(std::string_view typeNamespace, std::string_view typeName) const
    {
        if (auto ptr = try_find(typeNamespace, typeName))
        {
            return *ptr;
        }

        xlang::throw_invalid("Could not find type '", typeName, "' in namespace '", typeNamespace, "'");
    }

private:

    std::map<std::string_view, std::map<std::string_view, metadata_type const&>> m_typeTable;
};
