#pragma once

#include <algorithm>
#include <assert.h>
#include <charconv>
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

    using enum_value_semantics = model_ref<int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t>;

    struct enum_member : base_model
    {
        enum_member() = delete;
        enum_member(std::string_view const& id, size_t decl_line, std::string_view const& str_val) :
            base_model{ id, decl_line, "" },
            m_value{ str_val }
        {
        }

        bool operator==(enum_member const& rhs)
        {
            return rhs.get_id() == get_id();
        }

        auto const& get_value() const noexcept
        {
            return m_value;
        }

        void set_value(enum_value_semantics const& value)
        {
            m_value = value;
        }

        std::errc increment(enum_semantics type)
        {
            switch (type)
            {
            case enum_semantics::Int8:
                return increment<int8_t>();
            case enum_semantics::Uint8:
                return increment<uint8_t>();
            case enum_semantics::Int16:
                return increment<int16_t>();
            case enum_semantics::Uint16:
                return increment<uint16_t>();
            case enum_semantics::Int32:
                return increment<int32_t>();
            case enum_semantics::Uint32:
                return increment<uint32_t>();
            case enum_semantics::Int64:
                return increment<int64_t>();
            case enum_semantics::Uint64:
                return increment<uint64_t>();
            }
            return std::errc::invalid_argument;
        }

        std::errc resolve_decimal_val(enum_semantics type)
        {
            assert(!m_value.is_resolved());
            return resolve_numeric_val(type, 10);
        }

        std::errc resolve_hexadecimal_val(enum_semantics type)
        {
            assert(!m_value.is_resolved());
            return resolve_numeric_val(type, 16);
        }

    private:
        enum_value_semantics m_value;

        std::errc resolve_numeric_val(enum_semantics type, int base) noexcept
        {
            switch (type)
            {
            case enum_semantics::Int8:
                return resolve_numeric_val<int8_t>(base);
            case enum_semantics::Uint8:
                return resolve_numeric_val<uint8_t>(base);
            case enum_semantics::Int16:
                return resolve_numeric_val<int16_t>(base);
            case enum_semantics::Uint16:
                return resolve_numeric_val<uint16_t>(base);
            case enum_semantics::Int32:
                return resolve_numeric_val<int32_t>(base);
            case enum_semantics::Uint32:
                return resolve_numeric_val<uint32_t>(base);
            case enum_semantics::Int64:
                return resolve_numeric_val<int64_t>(base);
            case enum_semantics::Uint64:
                return resolve_numeric_val<uint64_t>(base);
            }
        }

        // resolve_numeric_val resolves the m_value string to type T. It is required that T is one
        // of int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, or uint64_t.
        template<typename T>
        std::errc resolve_numeric_val(int base)
        {
            assert(!m_value.is_resolved());
            std::string_view str_val{ m_value.get_ref_name() };
            T result;
            auto[p, ec] = std::from_chars(str_val.data(), str_val.data() + str_val.size(), result, base);
            assert(ec != std::errc::invalid_argument);
            if (ec == std::errc())
            {
                m_value.resolve(result);
            }
            return ec;
        }

        template<typename T>
        std::errc increment()
        {
            auto val = m_value.get_resolved_target<T>();
            if (val == std::numeric_limits<T>::max())
            {
                return std::errc::result_out_of_range;
            }
            m_value.resolve<T>(val + 1);
            return std::errc();
        }
    };

    struct enum_model : base_model
    {
        enum_model() = delete;
        enum_model(std::string_view const& id, size_t decl_line, std::string_view const& assembly_name, enum_semantics t) :
            base_model{ id, decl_line, assembly_name },
            m_type{ t }
        { }

        auto& get_members() noexcept
        {
            return m_members;
        }

        auto const& get_member(std::string_view const& id)
        {
            assert(member_exists(id));
            auto same_name = [&id](enum_member const& em)
            {
                return em.get_id() == id;
            };
            return *std::find_if(m_members.begin(), m_members.end(), same_name);
        }

        auto get_type() const noexcept
        {
            return m_type;
        }

        void add_member(enum_member&& e_member)
        {
            assert(!member_exists(e_member.get_id()));
            m_members.emplace_back(std::move(e_member));
        }

        bool member_exists(std::string_view const& id) const
        {
            auto same_id = [&id](enum_member const& em)
            {
                return em.get_id() == id;
            };
            return std::find_if(m_members.begin(), m_members.end(), same_id) != m_members.end();
        }

    private:
        std::vector<enum_member> m_members;
        enum_semantics m_type;
    };
}
