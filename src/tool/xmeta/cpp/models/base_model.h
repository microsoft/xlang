#pragma once

#include <string>
#include <string_view>

namespace xlang::xmeta
{
    struct base_model
    {
        base_model() = delete;
        base_model(std::string_view const& id, size_t decl_line, bool from_idl = true) :
            m_id{ id },
            m_decl_line{ decl_line },
            m_from_idl{ from_idl }
        { }

        auto const& get_id() const noexcept
        {
            return m_id;
        }

        auto get_decl_line() const noexcept
        {
            return m_decl_line;
        }

        auto from_idl() const noexcept
        {
            return m_from_idl;
        }

    private:
        std::string m_id;
        size_t m_decl_line;
        bool m_from_idl;
    };
}
