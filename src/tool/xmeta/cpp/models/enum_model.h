#pragma once

#include <algorithm>
#include <optional>
#include <string_view>
#include <vector>
#include <variant>

#include "base_model.h"
#include "model_ref.h"

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
        enum_member(std::string_view const& id, std::string_view const& str_val, enum_semantics type) :
            m_id{ id },
            m_value{ str_val }
        {
            assert(convert_value(type) == std::errc());
        }

        enum_member(std::string_view const& id, std::string_view const& str_val) :
            m_id{ id },
            m_value{ str_val }
        { }

        auto const& get_id() const noexcept
        {
            return m_id;
        }

        auto const& get_value() const noexcept
        {
            return m_value;
        }

        std::errc convert_value(enum_semantics type) noexcept
        {
            std::string_view str_val{ m_value.get_ref_name() };
            switch (type)
            {
            case enum_semantics::Int8:
                return extract_str_val<int8_t>(str_val);
            case enum_semantics::Uint8:
                return extract_str_val<uint8_t>(str_val);
            case enum_semantics::Int16:
                return extract_str_val<int16_t>(str_val);
            case enum_semantics::Uint16:
                return extract_str_val<uint16_t>(str_val);
            case enum_semantics::Int32:
                return extract_str_val<int32_t>(str_val);
            case enum_semantics::Uint32:
                return extract_str_val<uint32_t>(str_val);
            case enum_semantics::Int64:
                return extract_str_val<int64_t>(str_val);
            case enum_semantics::Uint64:
                return extract_str_val<uint64_t>(str_val);
            }
        }

    private:
        std::string m_id;
        model_ref<int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t> m_value;

        template<typename T>
        std::errc extract_str_val(std::string_view const& str_val)
        {
            T result;
            auto[p, ec] = std::from_chars(str_val.data(), str_val.data() + str_val.size(), result);
            if (ec == std::errc())
            {
                m_value = result;
            }
            return ec;
        }
    };

    struct enum_model : base_model
    {
        enum_model() = delete;
        enum_model(std::string_view const& id, size_t decl_line, std::string_view const& assembly_name, enum_semantics t) :
            base_model{ id, decl_line, assembly_name },
            m_type{ t }
        { }

        auto const& get_members() const noexcept
        {
            return m_members;
        }

        auto get_type() const noexcept
        {
            return m_type;
        }

        void add_member(std::string_view const& id, std::string_view const& str_val, bool is_numeric)
        {
            m_members.emplace_back(is_numeric ? enum_member{ id, str_val, m_type } : enum_member{ id, str_val });
        }

    private:
        std::vector<enum_member> m_members;
        enum_semantics m_type;
    };
}
