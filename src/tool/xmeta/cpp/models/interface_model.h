#pragma once

#include <cassert>
#include <string_view>
#include <vector>

#include "base_model.h"
#include "event_model.h"
#include "interface_model.h"
#include "method_model.h"
#include "property_model.h"

namespace xlang::xmeta
{
    struct interface_model : base_model
    {
        interface_model(std::string_view const& id, size_t decl_line) : base_model{ id, decl_line } { }
        interface_model() = delete;

        void add_interface_base_ref(std::string_view const& interface_base_ref)
        {
            m_interface_base_refs.emplace_back(std::string(interface_base_ref));
        }

        void add_interface_base_ref(size_t index, std::shared_ptr<interface_model> interface_base_ref)
        {
            assert(index < m_interface_base_refs.size());
            m_interface_base_refs[index] = interface_base_ref;
        }

        void add_member(std::shared_ptr<property_model> const& member)
        {
            m_members.emplace_back(member);
        }

        void add_member(std::shared_ptr<method_model> const& member)
        {
            m_members.emplace_back(member);
        }

        void add_member(std::shared_ptr<event_model> const& member)
        {
            m_members.emplace_back(member);
        }

    private:
        std::vector<std::variant<std::string, std::shared_ptr<interface_model>>> m_interface_base_refs;
        // TODO: Add type parameters (generic types)

        std::vector<std::variant<
            std::shared_ptr<property_model>,
            std::shared_ptr<method_model>,
            std::shared_ptr<event_model>>> m_members;
    };
}
