#pragma once

#include <string>
#include <string_view>

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
}
