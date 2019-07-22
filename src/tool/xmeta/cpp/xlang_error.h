#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <algorithm>

namespace xlang::xmeta
{
    enum idl_error
    {
        UNRESOLVED_TYPE,
        UNRESOLVED_REFERENCE,
        DUPLICATE_TYPE_MEMBER_ID,
        DUPLICATE_PROPERTY,
        DUPLICATE_EVENT,
        DUPLICATE_METHOD,
        DUPLICATE_FIELD_ID,
        DUPLICATE_ENUM_FIELD,
        TYPE_NOT_CLASS,
        TYPE_NOT_INTERFACE,
        TYPE_NOT_DELEGATE,
        CIRCULAR_INTERFACE_INHERITANCE,
        CIRCULAR_STRUCT_FIELD,
        CIRCULAR_ENUM_FIELD,
        ENUM_FIELD_OUT_OF_RANGE,
        INVALID_PROPERTY_ACCESSOR,
        DUPLICATE_PROPERTY_ACCESSOR,
        INVALID_OR_DUPLICATE_PROPERTY_ACCESSOR,
        CONFLICTING_EVENT_ACCESSOR_METHODS,
        INVALID_NAMESPACE_NAME,
        DUPLICATE_NAMESPACE_MEMBER,
        STATIC_MEMBER_ONLY,
        CANNOT_OVERLOAD_METHOD,
        CONFLICTING_INHERITANCE_MEMBER,
    };

    const std::map<idl_error, std::string> errors_message =
    {
        { UNRESOLVED_TYPE ,"Unable to resolve type."},
        { UNRESOLVED_REFERENCE, "Unable to resolve reference." },
        { DUPLICATE_TYPE_MEMBER_ID, "Duplicate type member identifier." },
        { DUPLICATE_PROPERTY, "Duplicate property member in type." },
        { DUPLICATE_EVENT, "Duplicate event member in type." },
        { DUPLICATE_METHOD, "Duplicate method member in type." },
        { DUPLICATE_FIELD_ID, "Duplicate field identifier." },
        { DUPLICATE_ENUM_FIELD, "Duplicate enum identifier." },
        { TYPE_NOT_CLASS, "Type is needs to be a runtime class type."},
        { TYPE_NOT_INTERFACE, "Type is needs to be an interface type."},
        { TYPE_NOT_DELEGATE, "Type is needs to be a delegate type."},
        { CIRCULAR_INTERFACE_INHERITANCE, "Class or Interface type has circular inheritance."},
        { CIRCULAR_STRUCT_FIELD, "Struct has circular field types. A struct cannot contain itself."},
        { CIRCULAR_ENUM_FIELD, "Enum field has circular dependency."},
        { ENUM_FIELD_OUT_OF_RANGE, "Enum expression not in range of valid enum expressions."},
        { INVALID_PROPERTY_ACCESSOR, "Invalid or missing property setter and getter."},
        { DUPLICATE_PROPERTY_ACCESSOR, "Duplicate property setter and getter."},
        { INVALID_OR_DUPLICATE_PROPERTY_ACCESSOR, "Duplicate or invalid identifier with property setter and getter."},
        { CONFLICTING_EVENT_ACCESSOR_METHODS, "Conflicting identifier with event setter and getter."},
        { INVALID_NAMESPACE_NAME, "Namespace name is invalid or duplicate."},
        { DUPLICATE_NAMESPACE_MEMBER, "Namespace member already defined"},
        { STATIC_MEMBER_ONLY, "Type member must be static"},
        { CANNOT_OVERLOAD_METHOD, "Cannot overload method"},
        { CONFLICTING_INHERITANCE_MEMBER, "Type member is conflicting with inherited members"}
    };

    struct error_model
    {
        idl_error error_code;
        size_t decl_line;
        std::string symbol;
    };

    class xlang_error_manager
    {
    public:
        void print_error(error_model const& model);

        void report_error(idl_error error, size_t decl_line);

        void report_error(idl_error error, size_t decl_line, std::string_view const& symbol);

        bool error_exists(idl_error code, std::string symbol, size_t decl_line);

        void disable_printing();

        size_t get_num_of_errors() const noexcept
        {
            return m_num_semantic_errors + m_num_syntax_errors; 
        }

        auto const& get_num_of_syntax_errors()
        {
            return m_num_syntax_errors;
        }

        auto const& get_num_of_semantic_errors()
        {
            return m_num_semantic_errors;
        }
        
        void increment_semantic_error_count()
        {
            m_num_semantic_errors++;
        }

        void set_num_syntax_error(size_t const& count)
        {
            m_num_syntax_errors = count;
        }

    private:
        size_t m_num_semantic_errors = 0;
        size_t m_num_syntax_errors = 0;
        std::vector<error_model> error_list;
        bool printing = true;
    };
}
