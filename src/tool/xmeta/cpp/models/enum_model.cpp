#include "enum_model.h"
#include "namespace_member_model.h"

namespace xlang::xmeta
{
    enum_value_semantics enum_member::get_resolved_value() const
    {
        assert(m_value.is_resolved());
        return std::visit(
            [&](auto&& value) -> enum_value_semantics
        {
            static_assert(std::is_integral_v<std::decay_t<decltype(value)>>);
            return value;
        },
            m_value.get_resolved_target());
    }

    std::errc enum_member::increment(enum_semantics type)
    {
        switch (type)
        {
        case enum_semantics::Int8:
            return increment<int8_t>();
        case enum_semantics::UInt8:
            return increment<uint8_t>();
        case enum_semantics::Int16:
            return increment<int16_t>();
        case enum_semantics::UInt16:
            return increment<uint16_t>();
        case enum_semantics::Int32:
            return increment<int32_t>();
        case enum_semantics::UInt32:
            return increment<uint32_t>();
        case enum_semantics::Int64:
            return increment<int64_t>();
        case enum_semantics::UInt64:
            return increment<uint64_t>();
        }
        return std::errc::invalid_argument;
    }

    std::errc enum_member::resolve_decimal_val(enum_semantics type)
    {
        assert(!m_value.is_resolved());
        return resolve_numeric_val(type, 10);
    }

    std::errc enum_member::resolve_hexadecimal_val(enum_semantics type)
    {
        assert(!m_value.is_resolved());
        return resolve_numeric_val(type, 16);
    }

    std::errc enum_member::resolve_numeric_val(enum_semantics type, int base) noexcept
    {
        switch (type)
        {
        case enum_semantics::Int8:
            return resolve_numeric_val<int8_t>(base);
        case enum_semantics::UInt8:
            return resolve_numeric_val<uint8_t>(base);
        case enum_semantics::Int16:
            return resolve_numeric_val<int16_t>(base);
        case enum_semantics::UInt16:
            return resolve_numeric_val<uint16_t>(base);
        case enum_semantics::Int32:
            return resolve_numeric_val<int32_t>(base);
        case enum_semantics::UInt32:
            return resolve_numeric_val<uint32_t>(base);
        case enum_semantics::Int64:
            return resolve_numeric_val<int64_t>(base);
        case enum_semantics::UInt64:
            return resolve_numeric_val<uint64_t>(base);
        }
        return std::errc::invalid_argument;
    }
}