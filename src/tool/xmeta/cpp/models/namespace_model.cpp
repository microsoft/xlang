#if defined(_WIN32)
#include <string.h>
#endif

#include <string_view>

#include "namespace_model.h"

#include "using_directive_models.h"

#include "class_model.h"
#include "delegate_model.h"
#include "enum_model.h"
#include "interface_model.h"
#include "property_model.h"
#include "struct_model.h"

namespace xlang::xmeta
{
    void namespace_body_model::add_class(std::shared_ptr<class_model> const& cm)
    {
        auto [x, succeeded] = m_classes.insert(std::pair(std::string_view{ cm->get_id() }, cm));
        assert(succeeded);
    }

    void namespace_body_model::add_struct(std::shared_ptr<struct_model> const& sm)
    {
        auto [x, succeeded] = m_structs.insert(std::pair(std::string_view{ sm->get_id() }, sm));
        assert(succeeded);
    }

    void namespace_body_model::add_interface(std::shared_ptr<interface_model> const& im)
    {
        auto [x, succeeded] = m_interfaces.insert(std::pair(std::string_view{ im->get_id() }, im));
        assert(succeeded);
    }

    void namespace_body_model::add_enum(std::shared_ptr<enum_model> const& em)
    {
        auto [x, succeeded] = m_enums.insert(std::pair(std::string_view{ em->get_id() }, em));
        assert(succeeded);
    }

    void namespace_body_model::add_delegate(std::shared_ptr<delegate_model> const& dm)
    {
        auto [x, succeeded] = m_delegates.insert(std::pair(std::string_view{ dm->get_id() }, dm));
        assert(succeeded);
    }

    void namespace_body_model::add_using_alias_directive(std::shared_ptr<using_alias_directive_model> const& uad)
    {
        auto [x, succeeded] = m_using_alias_directives.insert(std::pair(std::string_view{ uad->get_id() }, uad));
    }

    void namespace_body_model::add_using_namespace_directive(std::shared_ptr<using_namespace_directive_model> const& und)
    {
        m_using_namespace_directives.emplace_back(und);
    }

    // Checks if two member IDs are semantically the same.
    // namespace member IDs are case preserving but insensitive. This means if 'classA' is defined,
    // you cannot define another member 'cLaSsA'. You also cannot reference 'classA' using 'cLaSsA'
    // since case is preserved.
    template<typename T>
    struct same_member_id
    {
        same_member_id(std::string_view const& name) : new_name{ name } { }
        bool operator()(std::pair<std::string_view, std::shared_ptr<T>> const& v) const
        {
            auto old_name = v.first;
#if defined(_WIN32)
            return (stricmp(old_name.data(), new_name.data()) == 0 && new_name != old_name) ||
                new_name == old_name;
#endif
            return false; // Only works on windows for now.
        }
    private:
        std::string_view new_name;
    };

    inline bool namespace_body_model::type_id_exists(std::string_view const& member_id) const
    {
        return contains_id(m_classes, member_id) ||
            contains_id(m_structs, member_id) ||
            contains_id(m_interfaces, member_id) ||
            contains_id(m_enums, member_id) ||
            contains_id(m_delegates, member_id);
    }

    void namespace_model::add_child_namespace(std::shared_ptr<namespace_model> const& child)
    {
        assert(m_child_namespaces.find(child->get_id()) == m_child_namespaces.end());
        m_child_namespaces[child->get_id()] = child;
    }

    void namespace_model::add_namespace_body(std::shared_ptr<namespace_body_model> const& body)
    {
        m_namespace_bodies.emplace_back(body);
    }

    bool namespace_model::member_id_exists(std::string_view const& member_id) const
    {
        for (auto ns_body : m_namespace_bodies)
        {
            if (ns_body->type_id_exists(member_id))
            {
                return true;
            }
        }

        return contains_id(m_child_namespaces, member_id);
    }

    bool namespace_model::child_namespace_exists(std::string_view const& member_id) const
    {
        return m_child_namespaces.find(member_id) != m_child_namespaces.end();
    }

    std::string const& namespace_model::get_fully_qualified_id() const
    {
        if (!m_fully_qualified_id.has_value())
        {
            m_fully_qualified_id = m_parent_namespace == nullptr
                ? m_parent_namespace->get_fully_qualified_id() + "." + get_id()
                : get_id();
        }
        assert(m_fully_qualified_id.has_value());
        return m_fully_qualified_id.value();
    }
}
