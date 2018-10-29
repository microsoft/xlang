#pragma once

#include <string>

#include "meta_reader.h"
#include "namespace_iterator.h"

inline std::string clr_name(xlang::meta::reader::TypeDef const& type)
{
    std::string result;
    result.reserve(type.TypeNamespace().length() + type.TypeName().length() + 1);
    result += type.TypeNamespace();
    result += '.';
    result += type.TypeName();
    return result;
}

namespace details
{
    inline void write_type_prefix(std::string& result, xlang::meta::reader::category typeCategory)
    {
        if (typeCategory == xlang::meta::reader::category::delegate_type)
        {
            result += 'I';
        }
    }

    inline void write_type_prefix(std::string& result, xlang::meta::reader::TypeDef const& type)
    {
        write_type_prefix(result, xlang::meta::reader::get_category(type));
    }
}

namespace details
{
    template <bool IsGenericParam>
    void write_mangled_name(std::string& result, std::string_view name)
    {
        result.reserve(result.length() + name.length());
        for (auto ch : name)
        {
            if (ch == '.')
            {
                result += IsGenericParam ? "__C" : "_C";
            }
            else if (ch == '_')
            {
                result += IsGenericParam ? "__z" : "__";
            }
            else if (ch == '`')
            {
                result += '_';
            }
            else
            {
                result += ch;
            }
        }
    }
}

template <bool IsGenericParam>
inline std::string mangled_name(xlang::meta::reader::TypeDef const& type)
{
    std::string result;
    if (distance(type.GenericParam()) == 0)
    {
        details::write_mangled_name<IsGenericParam>(result, type.TypeNamespace());
        result += IsGenericParam ? "__C" : "_C";
    }
    else
    {
        // Generic types don't have the namespace included in the mangled name
        result += "__F";
    }

    details::write_type_prefix(result, type);
    details::write_mangled_name<IsGenericParam>(result, type.TypeName());
    return result;
}



#if 0
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
        if (pos < ns.length())
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

        XLANG_ASSERT(pos < name.length());
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

        return *thisPos - *otherPos;
    }

    struct iterator
    {
        type_name const* target;
        std::ptrdiff_t index;

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
        return iterator{ this, 0 };
    }

    iterator cend() const noexcept
    {
        return end();
    }
};

bool operator==(type_name const& lhs, type_name const& rhs) noexcept
{
    return lhs.compare(rhs) == 0;
}

bool operator!=(type_name const& lhs, type_name const& rhs) noexcept
{
    return lhs.compare(rhs) != 0;
}

bool operator<(type_name const& lhs, type_name const& rhs) noexcept
{
    return lhs.compare(rhs) < 0;
}

bool operator<=(type_name const& lhs, type_name const& rhs) noexcept
{
    return lhs.compare(rhs) <= 0;
}

bool operator>(type_name const& lhs, type_name const& rhs) noexcept
{
    return lhs.compare(rhs) > 0;
}

bool operator>=(type_name const& lhs, type_name const& rhs) noexcept
{
    return lhs.compare(rhs) >= 0;
}
#endif
