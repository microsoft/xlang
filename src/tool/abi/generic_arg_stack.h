#pragma once

#include "meta_reader.h"

struct generic_arg_stack
{
    using type = std::vector<std::vector<xlang::meta::reader::TypeSig>>;

    generic_arg_stack(type const& stack) noexcept :
        m_stack(&stack),
        m_position(stack.size())
    {
    }

    static generic_arg_stack empty() noexcept
    {
        static type empty;
        return generic_arg_stack(empty);
    }

    std::pair<xlang::meta::reader::TypeSig const&, generic_arg_stack> lookup(uint32_t index) const
    {
        // m_position points at the "current" type's generic parameters. When we're doing a lookup, we're doing so on a
        // previous type's generic parameters
        if (m_position == 0)
        {
            XLANG_ASSERT(false); // Typically a programming error
            xlang::throw_invalid("Type signature references a non-existant generic argument");
        }

        auto copy = *this;
        --copy.m_position;

        auto const& activeList = copy.current();
        if (index >= activeList.size())
        {
            XLANG_ASSERT(false); // Typically a programming error
            xlang::throw_invalid("Type signature references a generic argument that is out of range");
        }

        return { activeList[index], copy };
    }

private:

    std::vector<xlang::meta::reader::TypeSig> const& current() const noexcept
    {
        XLANG_ASSERT(m_position < m_stack->size());
        return (*m_stack)[m_position];
    }

    type const* m_stack;
    std::size_t m_position;
};
