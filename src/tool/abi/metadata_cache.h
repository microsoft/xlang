#pragma once

#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "meta_reader.h"
#include "task_group.h"
#include "type_names.h"

struct type_cache;
struct generic_instantiation;
struct namespace_cache;
struct metadata_cache;

struct type_cache
{
    xlang::meta::reader::TypeDef type_def;

    // Used for sorting and class/interface strings
    std::string clr_name;

    // Used for function/generic arguments
    std::string cpp_name;

    // Used for macros and C names
    std::string mangled_name;

    // Used for constructing names when used as a generic parameter
    std::string generic_param_mangled_name;

    type_cache(xlang::meta::reader::TypeDef const& def) :
        type_def(def),
        clr_name(::clr_name(def)),
        cpp_name(::cpp_name(def)),
        mangled_name(::mangled_name<false>(def)),
        generic_param_mangled_name(::mangled_name<true>(def))
    {
    }
};

inline bool operator<(type_cache const& lhs, type_cache const& rhs) noexcept
{
    return lhs.clr_name < rhs.clr_name;
}

struct generic_param
{

};

struct generic_instantiation
{
    xlang::meta::reader::GenericTypeInstSig signature;

    std::string clr_name;
    std::string mangled_name;
};

struct namespace_cache
{
    std::string_view name;

    // Types defined in this namespace
    std::map<std::string_view, type_cache> types;
    std::vector<std::reference_wrapper<type_cache const>> enums;
    std::vector<std::reference_wrapper<type_cache const>> structs;
    std::vector<std::reference_wrapper<type_cache const>> delegates;
    std::vector<std::reference_wrapper<type_cache const>> interfaces;
    std::vector<std::reference_wrapper<type_cache const>> classes;

    // Dependencies of this namespace
    std::set<std::string_view> dependent_namespaces;
    std::set<std::reference_wrapper<type_cache const>> external_dependencies;
    std::set<std::reference_wrapper<type_cache const>> internal_dependencies;

    namespace_cache(metadata_cache const* cache, std::string_view name) :
        m_cache(cache),
        name(name)
    {
    }

    type_cache const* try_find(std::string_view typeName) const noexcept
    {
        auto itr = types.find(typeName);
        if (itr != types.end())
        {
            return &itr->second;
        }

        return nullptr;
    }

    type_cache const& find(std::string_view typeName) const
    {
        if (auto ptr = try_find(typeName))
        {
            return *ptr;
        }

        xlang::throw_invalid("Could not find type '", typeName, "' in namespace '", name, "'");
    }

private:
    friend struct metadata_cache;

    struct init_state
    {
        std::set<std::string_view> generic_types;
        std::vector<std::vector<xlang::meta::reader::TypeSig>> generic_param_stack;
    };

    void process_dependencies();
    void process_dependencies(xlang::meta::reader::TypeDef const& type, init_state& state);
    void process_dependency(xlang::meta::reader::TypeSig const& type, init_state& state);
    void process_dependency(xlang::meta::reader::coded_index<xlang::meta::reader::TypeDefOrRef> const& type, init_state& state);
    void process_dependency(xlang::meta::reader::GenericTypeInstSig const& type, init_state& state);

    metadata_cache const* m_cache;
};

struct metadata_cache
{
    std::map<std::string_view, namespace_cache> namespaces;

    metadata_cache(xlang::meta::reader::cache const& c);

    type_cache const* try_find(std::string_view ns, std::string_view name) const noexcept
    {
        auto itr = namespaces.find(ns);
        if (itr != namespaces.end())
        {
            return itr->second.try_find(name);
        }

        return nullptr;
    }

    type_cache const& find(std::string_view ns, std::string_view name) const
    {
        if (auto ptr = try_find(ns, name))
        {
            return *ptr;
        }

        xlang::throw_invalid("Could not find type '", name, "' in namespace '", ns, "'");
    }

private:

    static void initialize_namespace(namespace_cache& target, xlang::meta::reader::cache::namespace_members const& members);
};
