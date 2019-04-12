#include "symbol_table_helpers.h"

namespace xlang::xmeta
{
    void symbol_table_helper::write_error(size_t decl_line, std::string_view const& msg)
    {
        std::cerr << "Semantic error (line " << decl_line << "): " << msg << std::endl;
        semantic_error_exists = true;
    }

    void symbol_table_helper::write_class_member_dup_modifier_error(size_t decl_line, std::string_view const& mod_name, std::string_view const& member_type, std::string_view const& id)
    {
        std::ostringstream oss;
        oss << "Modifier '" << mod_name << "' defined multiple times on " << member_type << " '" << id << "'";
        write_error(decl_line, oss.str());
    }

    void symbol_table_helper::write_class_dup_modifier_error(size_t decl_line, std::string_view const& mod_name, std::string_view const& class_name)
    {
        std::ostringstream oss;
        oss << "Modifier '" << mod_name << "' defined multiple times on class '" << cur_namespace_body->get_full_namespace_name() << "." << class_name << "'";
        write_error(decl_line, oss.str());
    }

    void symbol_table_helper::write_enum_member_name_error(size_t decl_line, std::string_view const& invalid_name, std::string_view const& enum_name)
    {
        std::ostringstream oss;
        oss << "Enum member '" << invalid_name << "' already defined in enum '" << cur_namespace_body->get_full_namespace_name() << "." << enum_name << "'";
        write_error(decl_line, oss.str());
    }

    void symbol_table_helper::write_enum_member_expr_ref_error(size_t decl_line, std::string_view const& invalid_name, std::string_view const& enum_name)
    {
        std::ostringstream oss;
        oss << "Enum member '" << invalid_name << "' not defined in enum '" << cur_namespace_body->get_full_namespace_name() << "." << enum_name << "'";
        write_error(decl_line, oss.str());
    }

    void symbol_table_helper::write_enum_const_expr_range_error(size_t decl_line, std::string_view const& invalid_expr, std::string_view const& enum_name)
    {
        std::ostringstream oss;
        oss << "Constant expression '" << invalid_expr << "' not in range of enum '" << cur_namespace_body->get_full_namespace_name() << "." << enum_name << "'";
    }

    void symbol_table_helper::write_event_dup_modifier_error(size_t decl_line, std::string_view const& mod_name, std::string_view const& event_name)
    {
        std::ostringstream oss;
        oss << "Modifier '" << mod_name << "' defined multiple times on enum '" << cur_namespace_body->get_full_namespace_name() << "." << event_name << "'";
        write_error(decl_line, oss.str());
    }

    void symbol_table_helper::write_namespace_name_error(size_t decl_line, std::string_view const& invalid_name, std::string_view const& original_name)
    {
        std::ostringstream oss;
        oss << "Namespace name '" << invalid_name << "' invalid. There already exists a namespace '" << original_name << "', and names cannot differ only by case.";
        write_error(decl_line, oss.str());
    }

    void symbol_table_helper::write_namespace_member_name_error(size_t decl_line, std::string_view const& invalid_name)
    {
        std::ostringstream oss;
        oss << "Member name '" << invalid_name << "' already defined in namespace '" << cur_namespace_body->get_full_namespace_name() << "'";
        write_error(decl_line, oss.str());
    }

    void symbol_table_helper::write_using_alias_directive_name_error(size_t decl_line, std::string_view const& invalid_name)
    {
        std::ostringstream oss;
        oss << "Using alias directive '" << invalid_name << "' already defined in namespace '" << cur_namespace_body->get_full_namespace_name() << "'";
        write_error(decl_line, oss.str());
    }

    std::string to_lower_copy(std::string_view sv)
    {
        std::string s{ sv };
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        return s;
    }

    struct check_ns_name
    {
        check_ns_name(std::string_view name) : new_name{ name } { }
        bool operator()(std::pair<std::string_view, std::shared_ptr<namespace_model>> const& v) const
        {
            auto old_name = v.first;
            return to_lower_copy(std::string(new_name)) == to_lower_copy(std::string(old_name)) && new_name != old_name;
        }
    private:
        std::string_view new_name;
    };

    // Pushes a namespace to the current namespace scope, and adds it to the symbol table if necessary.
    void symbol_table_helper::push_namespace(const std::string& name, const size_t& decl_line)
    {
        if (cur_namespace_body != nullptr)
        {
            auto &child_namespaces = cur_namespace_body->containing_namespace->child_namespaces;

            // Namespace names can't differ only by case.
            auto it1 = std::find_if(child_namespaces.begin(), child_namespaces.end(), check_ns_name(name));
            if (it1 != child_namespaces.end())
            {
                // Semantically invalid by check 4 for namespace members.
                write_namespace_name_error(decl_line, name, it1->first);
                return;
            }

            auto it2 = child_namespaces.find(name);
            if (it2 == child_namespaces.end())
            {
                child_namespaces[name] = std::make_shared<namespace_model>(name, decl_line, cur_namespace_body->containing_namespace /* parent ns */);
            }
            cur_namespace_body = std::make_shared<namespace_body_model>();
            cur_namespace_body->containing_namespace = child_namespaces[name];
            child_namespaces[name]->ns_bodies.emplace_back(cur_namespace_body);
        }
        else
        {
            auto it1 = std::find_if(namespaces.begin(), namespaces.end(), check_ns_name(name));
            if (it1 != namespaces.end())
            {
                // Semantically invalid by check 4 for namespace members.
                write_namespace_name_error(decl_line, name, it1->second->id);
                return;
            }
            auto it2 = namespaces.find(name);
            if (it2 == namespaces.end())
            {
                namespaces[name] = std::make_shared<namespace_model>(name, decl_line, nullptr /* No parent */);
            }
            cur_namespace_body = std::make_shared<namespace_body_model>();
            cur_namespace_body->containing_namespace = namespaces[name];
            namespaces[name]->ns_bodies.emplace_back(cur_namespace_body);
        }
    }

    // Pops a namespace from the namespace scope.
    void symbol_table_helper::pop_namespace()
    {
        if (cur_namespace_body != nullptr)
        {
            if (cur_namespace_body->containing_namespace->parent_namespace != nullptr)
            {
                cur_namespace_body = cur_namespace_body->containing_namespace->parent_namespace->ns_bodies.back();
            }
            else
            {
                cur_namespace_body = nullptr;
            }
        }
    }
}
