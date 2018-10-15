#pragma once

#include "meta_reader.h"

struct generic_arg_stack
{
    using type = std::vector<std::vector<xlang::meta::reader::TypeSig>>;

    generic_arg_stack(type const& stack) :
        m_stack(&stack)
    {
        m_position = m_stack->size();
    }

    static generic_arg_stack empty()
    {
        static type empty;
        return generic_arg_stack(empty);
    }

    std::pair<xlang::meta::reader::TypeSig const&, generic_arg_stack> lookup(uint32_t index) const noexcept
    {
        // m_position points at the "current" type's generic parameters. When we're doing a lookup, we're doing so on a
        // previous type's generic parameters
        XLANG_ASSERT(m_position > 0);

        auto copy = *this;
        --copy.m_position;

        auto const& activeList = copy.current();
        XLANG_ASSERT(index < activeList.size());

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
