#pragma once

#include <algorithm>
#include <memory>
#include <string>
#include <string_view>
#include <map>
#include <vector>
#include <variant>

#include "xlang_error.h"
#include "meta_reader.h"
#include "model_ref.h"

namespace xlang::xmeta
{
    struct namespace_body_model;
    struct namespace_model;
    struct namespace_member_model;

    struct using_alias_directive_model;
    struct using_namespace_directive_model;

    struct class_model;
    struct enum_model;
    struct interface_model;
    struct struct_model;
    struct delegate_model;

    struct property_model;
    struct method_model;
    struct event_model;

    struct formal_parameter_model;
    struct symbol_table;
    struct compilation_unit;
    struct type_ref;

    enum class semantic_error
    {
        passed = 0,
        symbol_exists,
        accessor_exists,
        method_cannot_be_overloaded
    };


    enum class method_association
    {
        None,
        Property,
        Event,
        Constructor
    };

    enum class fundamental_type
    {
        Boolean,
        String,
        Int8,
        Int16,
        Int32,
        Int64,
        UInt8,
        UInt16,
        UInt32,
        UInt64,
        Char16,
        Single,
        Double,
    };

    enum class enum_type
    {
        Int8,
        UInt8,
        Int16,
        UInt16,
        Int32,
        UInt32,
        Int64,
        UInt64
    };

    struct object_type {};

    using type_category = std::variant<std::monostate,
        std::shared_ptr<class_model>,
        std::shared_ptr<enum_model>,
        std::shared_ptr<interface_model>,
        std::shared_ptr<struct_model>,
        std::shared_ptr<delegate_model>,
        std::shared_ptr<xlang::meta::reader::TypeDef>,
        std::less<>>;

    using type_semantics = std::variant<
        std::shared_ptr<class_model>,
        std::shared_ptr<enum_model>,
        std::shared_ptr<interface_model>,
        std::shared_ptr<struct_model>,
        std::shared_ptr<delegate_model>,
        std::shared_ptr<xlang::meta::reader::TypeDef>,
        fundamental_type,
        object_type>;

    struct base_model
    {
        base_model() = delete;
        base_model(std::string_view const& name, size_t decl_line, std::string_view const& assembly_name) :
            m_name{ name },
            m_decl_line{ decl_line },
            m_assembly_name{ assembly_name }
        { }

        auto const& get_name() const noexcept
        {
            return m_name;
        }

        auto get_decl_line() const noexcept
        {
            return m_decl_line;
        }

        auto const& get_assembly_name() const noexcept
        {
            return m_assembly_name;
        }

    private:
        std::string m_name;
        size_t m_decl_line;
        std::string_view m_assembly_name;
    };

    template<class T>
    inline auto get_by_name(std::vector<std::shared_ptr<T>> const& v, std::string_view const& name)
    {
        auto same_id = [&](std::shared_ptr<T> const& t) { return t->get_name() == name; };
        return std::find_if(v.begin(), v.end(), same_id);
    }

    template<class T>
    inline auto get_by_name(std::map<std::string_view, std::shared_ptr<T>, std::less<>> const& m, std::string_view const& name)
    {
        return m.find(name);
    }

    template<class T>
    inline bool contains_id(T const& v, std::string_view const& name)
    {
        return get_by_name(v, name) != v.end();
    }
}
