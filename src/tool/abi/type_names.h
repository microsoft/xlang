#pragma once

#include <string>

#include "common.h"
#include "meta_reader.h"
#include "namespace_iterator.h"

inline std::string clr_full_name(xlang::meta::reader::TypeDef const& type)
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
    inline void write_type_prefix(std::string& result, xlang::meta::reader::TypeDef const& type)
    {
        using namespace xlang::meta::reader;
        if ((get_category(type) == category::delegate_type) && (type.TypeNamespace() != collections_namespace))
        {
            // All delegates except those in the 'Windows.Foundation.Collections' namespace get an 'I' appended to the
            // front...
            result.push_back('I');
        }
    }

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
    if (!is_generic(type))
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
