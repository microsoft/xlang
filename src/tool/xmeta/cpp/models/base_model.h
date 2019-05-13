#pragma once

#include <algorithm>
#include <memory>
#include <string>
#include <string_view>
#include <map>
#include <vector>
#include <variant>

#include "xlang_error.h"

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

    struct symbol_table;
    struct compilation_unit;

    struct type_ref;

    using class_type_semantics = std::variant<std::monostate,
        std::shared_ptr<class_model>,
        std::shared_ptr<enum_model>,
        std::shared_ptr<interface_model>,
        std::shared_ptr<struct_model>,
        std::shared_ptr<delegate_model>, std::less<>>;

    struct base_model
    {
        base_model() = delete;
        base_model(std::string_view const& id, size_t decl_line, std::string_view const& assembly_name) :
            m_id{ id },
            m_decl_line{ decl_line },
            m_assembly_name{ assembly_name }
        { }

        auto const& get_id() const noexcept
        {
            return m_id;
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
        std::string m_id;
        size_t m_decl_line;
        std::string_view m_assembly_name;
    };

    template<class T>
    inline auto get_it(std::vector<std::shared_ptr<T>> const& v, std::string_view const& id)
    {
        auto same_id = [&](std::shared_ptr<T> const& t) { return t->get_id() == id; };
        return std::find_if(v.begin(), v.end(), same_id);
    }

    template<class T>
    inline auto get_it(std::map<std::string_view, std::shared_ptr<T>, std::less<>> const& m, std::string_view const& id)
    {
        return m.find(id);
    }

    template<class T>
    inline bool contains_id(T const& v, std::string_view const& id)
    {
        return get_it(v, id) != v.end();
    }
}
