#pragma once

#include <algorithm>
#include <memory>
#include <string>
#include <string_view>
#include <map>
#include <vector>

namespace xlang::xmeta
{
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
