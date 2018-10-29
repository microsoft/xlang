#pragma once

#include "meta_reader.h"

// Useful for comparing & ordering namespace + type name pairs as if the two names were one
struct type_name
{
    std::string_view ns;
    std::string_view name;

    constexpr std::string_view TypeNamespace() const noexcept
    {
        return ns;
    }

    constexpr std::string_view TypeName() const noexcept
    {
        return name;
    }

    constexpr bool has_namespace() const noexcept
    {
        return !ns.empty();
    }

    constexpr std::size_t size() const noexcept
    {
        return ns.length() + name.length() + (has_namespace() ? 1 : 0);
    }

    constexpr std::size_t length() const noexcept
    {
        return size();
    }

    constexpr char operator[](std::ptrdiff_t pos) const noexcept
    {
        XLANG_ASSERT(pos >= 0);
        if (pos < static_cast<std::ptrdiff_t>(ns.length()))
        {
            return ns[pos];
        }
        pos -= ns.length();

        if (has_namespace())
        {
            if (pos == 0)
            {
                return '.';
            }

            --pos;
        }

        XLANG_ASSERT(pos < static_cast<std::ptrdiff_t>(name.length()));
        return name[pos];
    }

    int compare(type_name const& other) const noexcept
    {
        auto [thisPos, otherPos] = std::mismatch(begin(), end(), other.begin(), other.end());
        if (thisPos == end())
        {
            if (otherPos == other.end())
            {
                return 0;
            }

            return -1;
        }
        else if (otherPos == other.end())
        {
            return 1;
        }

        return *otherPos - *thisPos;
    }

    struct iterator
    {
        type_name const* target;
        std::size_t index;

        using iterator_category = std::forward_iterator_tag; // Could be better, but only exists for comparison
        using value_type = char;
        using difference_type = std::ptrdiff_t;
        using pointer = void;
        using reference = char;

        friend constexpr bool operator!=(iterator const& lhs, iterator const& rhs) noexcept
        {
            XLANG_ASSERT(lhs.target == rhs.target);
            return lhs.index != rhs.index;
        }

        friend constexpr bool operator==(iterator const& lhs, iterator const& rhs) noexcept
        {
            return !(lhs != rhs);
        }

        char operator*() const noexcept
        {
            return (*target)[index];
        }

        iterator& operator++() noexcept
        {
            XLANG_ASSERT(index < target->length());
            ++index;
            return *this;
        }

        iterator operator++(int) noexcept
        {
            auto copy = *this;
            ++(*this);
            return copy;
        }
    };

    iterator begin() const noexcept
    {
        return iterator{ this, 0 };
    }

    iterator cbegin() const noexcept
    {
        return begin();
    }

    iterator end() const noexcept
    {
        return iterator{ this, length() };
    }

    iterator cend() const noexcept
    {
        return end();
    }
};

inline bool operator==(type_name const& lhs, type_name const& rhs) noexcept
{
    return lhs.compare(rhs) == 0;
}

inline bool operator!=(type_name const& lhs, type_name const& rhs) noexcept
{
    return lhs.compare(rhs) != 0;
}

inline bool operator<(type_name const& lhs, type_name const& rhs) noexcept
{
    return lhs.compare(rhs) < 0;
}

inline bool operator<=(type_name const& lhs, type_name const& rhs) noexcept
{
    return lhs.compare(rhs) <= 0;
}

inline bool operator>(type_name const& lhs, type_name const& rhs) noexcept
{
    return lhs.compare(rhs) > 0;
}

inline bool operator>=(type_name const& lhs, type_name const& rhs) noexcept
{
    return lhs.compare(rhs) >= 0;
}
