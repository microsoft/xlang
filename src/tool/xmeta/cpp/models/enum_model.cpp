#include <cassert>
#include <charconv>

#include "enum_model.h"

namespace xlang::xmeta
{
    enum_member::enum_member(std::string_view const& id, std::optional<enum_member_semantics> val) :
        m_id{ id },
        m_value{ val }
    { }

    enum_member::enum_member(std::string_view const& id, std::string_view const& expression_id) :
        m_id{ id },
        m_expression_id{ expression_id }
    { }

    auto const& enum_member::get_id() const
    {
        return m_id;
    }

    auto const& enum_member::get_value() const
    {
        return m_value;
    }

    auto const& enum_member::get_expression_id() const
    {
        return m_expression_id;
    }

    void enum_member::set_value(enum_member_semantics const& val)
    {
        m_value = val;
    }

    enum_model::enum_model(std::string_view const& id, size_t decl_line, enum_semantics t) :
        base_model{ id, decl_line },
        m_type{ t }
    { }

    auto const& enum_model::get_members() const
    {
        return m_members;
    }

    auto const& enum_model::get_type() const
    {
        return m_type;
    }

    template<typename T>
    auto set_val(std::vector<enum_member>::iterator & it, std::string_view str_val)
    {
        T result;
        auto[p, ec] = std::from_chars(str_val.data(), str_val.data() + str_val.size(), result);
        if (ec == std::errc())
        {
            it->set_value(result);
        }
        return ec;
    }

    std::errc enum_model::assign_value(std::string_view const& member_id, std::string_view val)
    {
        auto same_member = [&member_id](enum_member const& em)
        {
            return em.get_id() == member_id;
        };
        auto it = std::find_if(m_members.begin(), m_members.end(), same_member);
        assert(it != m_members.end());
        switch (m_type)
        {
        case enum_semantics::Int8:
            return set_val<int8_t>(it, val);
        case enum_semantics::Uint8:
            return set_val<uint8_t>(it, val);
        case enum_semantics::Int16:
            return set_val<int16_t>(it, val);
        case enum_semantics::Uint16:
            return set_val<uint16_t>(it, val);
        case enum_semantics::Int32:
            return set_val<int32_t>(it, val);
        case enum_semantics::Uint32:
            return set_val<uint32_t>(it, val);
        case enum_semantics::Int64:
            return set_val<int64_t>(it, val);
        case enum_semantics::Uint64:
            return set_val<uint64_t>(it, val);
        }
    }

    void enum_model::add_member(std::string_view const& id, std::optional<enum_member_semantics> val)
    {
        m_members.emplace_back(enum_member{ id, val });
    }

    void enum_model::add_member(std::string_view const& id, std::string_view const& expression_id)
    {
        m_members.emplace_back(enum_member{ id, expression_id });
    }
}