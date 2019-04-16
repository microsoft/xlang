#pragma once

#include <memory>
#include <string>
#include <variant>

namespace xlang::xmeta
{
    // model_ref is meant to hold unresolved and then resolved reference state. When unresolved,
    // it holds a string representing the ID of the reference. Otherwise, it holds generic state,
    // which is typically a smart pointer to the type or member being referenced.
    template <typename T, typename... Ts>
    struct model_ref
    {
        model_ref() = delete;
        explicit model_ref(std::string_view const& name) :
            m_ref{ std::string(name) }
        { }

        bool is_resolved() const noexcept
        {
            return m_ref.index() != 0;
        }

        template<typename H>
        bool holds_type() const noexcept
        {
            return std::holds_alternative<H>(m_ref);
        }

        template<typename R>
        auto const& get_resolved_target() const noexcept
        {
            assert(std::holds_alternative<R>(m_ref));
            return std::get<R>(m_ref);
        }

        std::string const& get_ref_name() const noexcept
        {
            assert(!is_resolved());
            return std::get<std::string>(m_ref);
        }

        template<typename R>
        void resolve(R const& value) noexcept
        {
            m_ref = value;
        }

    private:
        std::variant<std::string, T, Ts...> m_ref;
    };
}
