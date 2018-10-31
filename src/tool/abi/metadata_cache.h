#pragma once

#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "meta_reader.h"
#include "task_group.h"
#include "type_name.h"
#include "type_names.h"

struct metadata_type
{
    virtual std::string_view clr_namespace() const = 0;
    virtual std::string_view clr_full_name() const = 0;
    virtual std::string_view logical_name() const = 0;
    virtual std::string_view abi_name() const = 0;
    virtual std::string_view mangled_name() const = 0;
    virtual std::string_view generic_param_mangled_name() const = 0;
};

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

    virtual std::string_view logical_name() const override
    {
        return m_type.TypeName();
    }

    virtual std::string_view abi_name() const override
    {
        return m_type.TypeName();
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

    // These strings need to be initialized by the derived class
};

struct enum_type final : typedef_base
{
    enum_type(xlang::meta::reader::TypeDef const& type) :
        typedef_base(type),
        m_underlyingType(std::get<xlang::meta::reader::ElementType>(type.FieldList().first.Signature().Type().Type()))
    {
    }

    xlang::meta::reader::ElementType underlying_type() const noexcept
    {
        return m_underlyingType;
    }

private:

    xlang::meta::reader::ElementType m_underlyingType;
};

struct struct_type final : typedef_base
{
};

struct delegate_type final : typedef_base
{
    delegate_type(xlang::meta::reader::TypeDef const& type) :
        typedef_base(type)
    {
        m_typeName.push_back('I');
        m_typeName += type.TypeName();
    }

    virtual std::string_view logical_name() const override
    {
        return m_typeName;
    }

    virtual std::string_view abi_name() const override
    {
        return m_typeName;
    }

private:

    std::string m_typeName;
};

struct interface_type final : typedef_base
{
};

struct class_type final : typedef_base
{
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

    virtual std::string_view clr_namespace() const override
    {
        return {};
    }

    virtual std::string_view clr_full_name() const override
    {
        return m_clrName;
    }

    virtual std::string_view logical_name() const override
    {
        return m_logicalName;
    }

    virtual std::string_view abi_name() const override
    {
        return m_abiName;
    }

    virtual std::string_view mangled_name() const override
    {
        xlang::throw_invalid("ElementType values don't have mangled names");
    }

    virtual std::string_view generic_param_mangled_name() const override
    {
        return m_mangledName;
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

    virtual std::string_view clr_namespace() const override
    {
        // Currently all mapped types from the System namespace have no namespace
        return {};
    }

    virtual std::string_view clr_full_name() const override
    {
        return m_clrName;
    }

    virtual std::string_view logical_name() const override
    {
        return m_cppName;
    }

    virtual std::string_view abi_name() const override
    {
        return m_cppName;
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

private:

    std::string_view m_clrName;
    std::string_view m_cppName;
};


#if 0
struct metadata_cache;

struct generic_inst final : metadata_type
{
    generic_inst(
        metadata_type const* genericType,
        std::vector<metadata_type const*> genericParams,
        std::string clrName,
        std::string mangledName) :
        m_genericType(genericType),
        m_genericParams(std::move(genericParams)),
        m_clrName(std::move(clrName)),
        m_mangledName(std::move(mangledName))
    {
    }

    virtual std::string_view clr_namespace() const override
    {
        return m_genericType->clr_namespace();
    }

    virtual std::string_view clr_full_name() const override
    {
        return m_clrName;
    }

    virtual std::string_view cpp_type_name() const override
    {
        return m_mangledName;
    }

    virtual std::string_view mangled_name() const override
    {
        return m_mangledName;
    }

    virtual std::string_view generic_param_mangled_name() const override
    {
        return m_mangledName;
    }

    metadata_type const& generic_type() const noexcept
    {
        return *m_genericType;
    }

    std::vector<metadata_type const*> const& generic_params() const noexcept
    {
        return m_genericParams;
    }

private:

    metadata_type const* m_genericType;
    std::vector<metadata_type const*> m_genericParams;
    std::string m_clrName;
    std::string m_mangledName;
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
    // The namespace(s) that this object incorporates
    std::vector<std::string_view> included_namespaces;

    // Types defined in these namespace(s)
    std::set<std::reference_wrapper<type_def const>> types;
    std::vector<std::reference_wrapper<type_def const>> enums;
    std::vector<std::reference_wrapper<type_def const>> structs;
    std::vector<std::reference_wrapper<type_def const>> delegates;
    std::vector<std::reference_wrapper<type_def const>> interfaces;
    std::vector<std::reference_wrapper<type_def const>> classes;
    std::set<api_contract> contracts;

    // Dependencies of these namespace(s)
    std::set<std::string_view> dependent_namespaces;
    std::map<std::string_view, generic_inst> generic_instantiations;
    std::set<std::reference_wrapper<type_def const>> external_dependencies;
    std::set<std::reference_wrapper<type_def const>> internal_dependencies;

    type_cache(metadata_cache const* cache) :
        m_cache(cache)
    {
    }

    type_cache merge(type_cache const& other) const;

    metadata_cache const& metadata() const noexcept
    {
        return *m_cache;
    }

private:
    friend struct metadata_cache;

    struct init_state
    {
        // Used for GenericTypeIndex lookup
        generic_inst const* parent_generic_inst = nullptr;
    };

    void process_dependencies();
    void process_dependencies(xlang::meta::reader::TypeDef const& type, init_state& state);
    metadata_type const* process_dependency(xlang::meta::reader::TypeSig const& type, init_state& state);
    metadata_type const* process_dependency(xlang::meta::reader::coded_index<xlang::meta::reader::TypeDefOrRef> const& type, init_state& state);
    metadata_type const* process_dependency(xlang::meta::reader::GenericTypeInstSig const& type, init_state& state);

    metadata_cache const* m_cache;
};

struct metadata_cache
{
    std::map<std::string_view, type_cache> namespaces;

    metadata_cache(xlang::meta::reader::cache const& c);

    type_def const* try_find(std::string_view ns, std::string_view name) const noexcept
    {
        auto nsItr = m_types.find(ns);
        if (nsItr != m_types.end())
        {
            auto nameItr = nsItr->second.find(name);
            if (nameItr != nsItr->second.end())
            {
                return &nameItr->second;
            }
        }

        return nullptr;
    }

    type_def const& find(std::string_view ns, std::string_view name) const
    {
        if (auto ptr = try_find(ns, name))
        {
            return *ptr;
        }

        xlang::throw_invalid("Could not find type '", name, "' in namespace '", ns, "'");
    }

private:

    void initialize_namespace(
        type_cache& target,
        std::map<std::string_view, type_def>& typeMap,
        xlang::meta::reader::cache::namespace_members const& members);

    //xlang::meta::reader::cache const* m_cache;
    std::map<std::string_view, std::map<std::string_view, type_def>> m_types;
};
#endif
