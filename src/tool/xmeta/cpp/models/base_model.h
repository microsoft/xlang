#pragma once

#include <string_view>
#include <variant>
#include <vector>

namespace xlang::xmeta
{
    struct base_model
    {
        base_model(std::string_view const& id, size_t decl_line) : id{ id }, decl_line{ decl_line } { }
        base_model() = delete;

        std::string_view id;
        size_t decl_line;
    };

    enum class simple_type
    {
        // TODO: rename
        Boolean,
        String,
        Int8,
        Int16,
        Int32,
        Int64,
        Uint8,
        Uint16,
        Uint32,
        Uint64,
        Char16,
        Guid,
        Single,
        Double,
        Void
    };

    struct type
    {
        bool is_array;
        std::variant<simple_type, std::string> type_id;
        std::vector<type> type_arguments;
    };

    // TODO: rename to parameter_semantics
    enum class parameter_semantics
    {
        in,
        const_ref,
        out
    };

    struct formal_parameter_t
    {
        parameter_semantics modifier;
        type type;
        std::string_view id;
    };
}
