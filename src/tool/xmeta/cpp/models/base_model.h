#pragma once

#include <string>
#include <string_view>

namespace xlang::xmeta
{
    struct base_model
    {
        base_model(std::string_view const& id, size_t decl_line) : m_id{ std::string(id) }, m_decl_line{ decl_line } { }
        base_model() = delete;

        auto const& get_id() const
        {
            return m_id;
        }

        auto const& get_decl_line() const
        {
            return m_decl_line;
        }

    private:
        std::string m_id;
        size_t m_decl_line;
    };
}
