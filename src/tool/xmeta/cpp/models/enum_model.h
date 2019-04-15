#pragma once

#include <algorithm>
#include <optional>
#include <string_view>
#include <vector>
#include <variant>

#include "base_model.h"

namespace xlang::xmeta
{
    enum class enum_semantics
    {
        Int8,
        Uint8,
        Int16,
        Uint16,
        Int32,
        Uint32,
        Int64,
        Uint64
    };

    using enum_member_semantics = std::variant<
        int8_t,
        uint8_t,
        int16_t,
        uint16_t,
        int32_t,
        uint32_t,
        int64_t,
        uint64_t>;

    struct enum_member
    {
        enum_member() = delete;
        enum_member(std::string_view const& id, std::optional<enum_member_semantics> val);
        enum_member(std::string_view const& id, std::string_view const& expression_id);

        auto const& get_id() const noexcept;
        auto const& get_value() const noexcept;
        auto const& get_expression_id() const noexcept;

        void set_value(enum_member_semantics const& val) noexcept;

    private:
        std::string m_id;
        std::optional<enum_member_semantics> m_value;

        // Temporarily stores ID referenced in an enum assignment.
        std::string m_expression_id;
    };

    struct enum_model : base_model
    {
        enum_model() = delete;
        enum_model(std::string_view const& id, size_t decl_line, std::string_view const& assembly_name, enum_semantics t);

        auto const& get_members() const noexcept;
        auto const& get_type() const noexcept;

        std::errc assign_value(std::string_view const& member_id, std::string_view const& str_val) noexcept;
        void add_member(std::string_view const& id, std::optional<enum_member_semantics> val);
        void add_member(std::string_view const& id, std::string_view const& expression_id);

    private:
        std::vector<enum_member> m_members;
        enum_semantics m_type;
    };
}
