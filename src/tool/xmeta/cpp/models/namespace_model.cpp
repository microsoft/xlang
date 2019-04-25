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
        m_classes[cm->get_id()] = cm;
    }

    void namespace_body_model::add_struct(std::shared_ptr<struct_model> const& sm)
    {
        m_structs[sm->get_id()] = sm;
    }

    void namespace_body_model::add_interface(std::shared_ptr<interface_model> const& im)
    {
        m_interfaces[im->get_id()] = im;
    }

    void namespace_body_model::add_enum(std::shared_ptr<enum_model> const& em)
    {
        m_enums[em->get_id()] = em;
    }

    void namespace_body_model::add_delegate(std::shared_ptr<delegate_model> const& dm)
    {
        m_delegates[dm->get_id()] = dm;
    }

    void namespace_body_model::add_using_alias_directive(std::shared_ptr<using_alias_directive_model> const& uad)
    {
        m_using_alias_directives[uad->get_id()] = uad;
    }

    void namespace_body_model::add_using_namespace_directive(std::shared_ptr<using_namespace_directive_model> const& und)
    {
        m_using_namespace_directives.emplace_back(std::move(und));
    }

    std::string copy_to_lower(std::string_view sv)
    {
        std::string s{ sv };
        std::locale loc;
        // ***NOTE*** ::tolower is temporary. This only works on the ASCII charset.
        // Comparing case in full UTF-8 is a larger discussion, and should first be documented in detail.
        std::transform(s.begin(), s.end(), s.begin(), [&](char c)
        {
            return static_cast<char>(::tolower(c));
        });
        return s;
    }

    // Checks if two member IDs are semantically the same.
    // namespace member IDs are case preserving but insensitive. This means if 'classA' is defined,
    // you cannot define another member 'cLaSsA'. You also cannot reference 'classA' using 'cLaSsA'
    // since case is preserved.
    template<typename T>
    struct same_member_id
    {
        same_member_id(std::string_view name) : new_name{ name } { }
        bool operator()(std::pair<std::string_view, std::shared_ptr<T>> const& v) const
        {
            auto old_name = v.first;
            return (copy_to_lower(std::string(new_name)) == copy_to_lower(std::string(old_name)) && new_name != old_name) ||
                new_name == old_name;
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

    std::string namespace_model::get_full_name() const
    {
        return m_parent_namespace != nullptr
            ? m_parent_namespace->get_full_name() + "." + get_id()
            : get_id();
    }
}
