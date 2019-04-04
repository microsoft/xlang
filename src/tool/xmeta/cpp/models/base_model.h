#pragma once

#include <string_view>
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

    enum class simple_t
    {
        boolean_type,
        string_type,
        int16_type,
        int32_type,
        int64_type,
        uint8_type,
        uint16_type,
        uint32_type,
        uint64_type,
        char16_type,
        guid_type,
        single_type,
        double_type,
    };

    struct xmeta_type
    {
        bool is_void;
        bool is_array;
        bool is_simple_type; // if (!is_void && !is_simple_type) { return type is struct, enum, class, interface, or delegate. }
        simple_t simple_type;
        std::string_view type_name;
        std::vector<xmeta_type> type_arguments;
    };

    enum class parameter_modifier_t
    {
        none,
        const_ref_parameter,
        out_parameter
    };

    struct formal_parameter_t
    {
        parameter_modifier_t modifier;
        xmeta_type type;
        std::string_view id;
    };
}
