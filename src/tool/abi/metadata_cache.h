#pragma once

#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "meta_reader.h"
#include "task_group.h"
#include "type_names.h"

struct type_def;
struct generic_instantiation;
struct type_cache;
struct metadata_cache;

struct type_ref
{
    std::string_view clr_name;
    std::string_view mangled_name;
    std::string_view generic_param_mangled_name;
};

struct type_def
{
    xlang::meta::reader::TypeDef type;

    // Used for sorting and class/interface strings
    std::string clr_name;

    // Used for macros and C names
    std::string mangled_name;

    // Used for constructing names when used as a generic parameter
    std::string generic_param_mangled_name;

    type_def(xlang::meta::reader::TypeDef const& def) :
        type(def),
        clr_name(::clr_name(def)),
        mangled_name(::mangled_name<false>(def)),
        generic_param_mangled_name(::mangled_name<true>(def))
    {
    }
};

inline bool operator<(type_def const& lhs, type_def const& rhs) noexcept
{
    return lhs.clr_name < rhs.clr_name;
}

struct generic_instantiation
{
    xlang::meta::reader::GenericTypeInstSig signature;

    std::string clr_name;
    std::string mangled_name;
};

struct type_cache
{
    // Types defined in this namespace
    std::set<std::reference_wrapper<type_def const>> types;
    std::vector<std::reference_wrapper<type_def const>> enums;
    std::vector<std::reference_wrapper<type_def const>> structs;
    std::vector<std::reference_wrapper<type_def const>> delegates;
    std::vector<std::reference_wrapper<type_def const>> interfaces;
    std::vector<std::reference_wrapper<type_def const>> classes;

    // Dependencies of this namespace
    std::set<std::string_view> dependent_namespaces;
    std::map<std::string_view, generic_instantiation> generic_instantiations;
    std::set<std::reference_wrapper<type_def const>> external_dependencies;
    std::set<std::reference_wrapper<type_def const>> internal_dependencies;

    type_cache(metadata_cache const* cache) :
        m_cache(cache)
    {
    }

    type_cache merge(type_cache const& other) const;

private:
    friend struct metadata_cache;

    struct init_state
    {
        std::vector<std::vector<type_ref>> prev_param_stack;
    };

    void process_dependencies();
    void process_dependencies(xlang::meta::reader::TypeDef const& type, init_state& state);
    type_ref process_dependency(xlang::meta::reader::TypeSig const& type, init_state& state);
    type_ref process_dependency(xlang::meta::reader::coded_index<xlang::meta::reader::TypeDefOrRef> const& type, init_state& state);
    type_ref process_dependency(xlang::meta::reader::GenericTypeInstSig const& type, init_state& state);

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

    static void initialize_namespace(
        type_cache& target,
        std::map<std::string_view, type_def>& typeMap,
        xlang::meta::reader::cache::namespace_members const& members);

    std::map<std::string_view, std::map<std::string_view, type_def>> m_types;
};
