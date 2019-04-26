#include <assert.h>
#include <string.h>

#include "xmeta_idl_reader.h"
#include "ast_to_st_listener.h"
#include "XlangLexer.h"
#include "XlangParser.h"

namespace xlang::xmeta
{

    size_t xmeta_idl_reader::read(std::istream& idl_contents, bool disable_error_reporting)
    {
        ast_to_st_listener listener{ *this };
        return read(idl_contents, listener, disable_error_reporting);
    }

    size_t xmeta_idl_reader::read(std::istream& idl_contents, XlangParserBaseListener& listener, bool disable_error_reporting)
    {
        antlr4::ANTLRInputStream input{ idl_contents };
        XlangLexer lexer{ &input };
        antlr4::CommonTokenStream tokens{ &lexer };
        XlangParser parser{ &tokens };
        parser.setBuildParseTree(true);

        if (disable_error_reporting) {
            lexer.removeErrorListeners();
            parser.removeErrorListeners();
        }

        antlr4::tree::ParseTree *tree = parser.xlang();
        antlr4::tree::ParseTreeWalker::DEFAULT.walk(&listener, tree);
        return parser.getNumberOfSyntaxErrors();
    }

    struct invalid_ns_id
    {
        invalid_ns_id(std::string_view const& name) : new_name{ name } { }
        bool operator()(std::pair<std::string_view, std::shared_ptr<namespace_model>> const& v) const
        {
            auto old_name = v.first;
            return stricmp(new_name.data(), old_name.data()) == 0 && new_name != old_name;
        }
    private:
        std::string_view new_name;
    };

    // Pushes a namespace to the current namespace scope, and adds it to the symbol table if necessary.
    void xmeta_idl_reader::push_namespace(std::string_view const& name, size_t decl_line)
    {
        auto setup_cur_ns_body = [&](std::shared_ptr<namespace_model> const& child_ns)
        {
            assert(child_ns->get_id() == name);
            m_cur_namespace_body = std::make_shared<namespace_body_model>(child_ns);
            child_ns->add_namespace_body(m_cur_namespace_body);
        };
        if (m_cur_namespace_body != nullptr)
        {
            auto const& cur_ns = m_cur_namespace_body->get_containing_namespace();
            if (cur_ns->member_id_exists(name))
            {
                if (cur_ns->child_namespace_exists(name))
                {
                    setup_cur_ns_body(cur_ns->get_child_namespaces().at(name));
                }
                else
                {
                    write_namespace_member_name_error(decl_line, name);
                    return;
                }
            }
            else
            {
                if (!cur_ns->child_namespace_exists(name))
                {
                    cur_ns->add_child_namespace(std::make_shared<namespace_model>(name, decl_line, m_cur_assembly, cur_ns));
                }
                setup_cur_ns_body(cur_ns->get_child_namespaces().at(name));
            }
        }
        else
        {
            auto it1 = std::find_if(m_namespaces.begin(), m_namespaces.end(), invalid_ns_id(name));
            if (it1 != m_namespaces.end())
            {
                // Semantically invalid by check 4 for namespace members.
                write_namespace_name_error(decl_line, name, it1->second->get_id());
                return;
            }

            auto it2 = m_namespaces.find(name);
            if (it2 == m_namespaces.end())
            {
                auto new_ns = std::make_shared<namespace_model>(name, decl_line, m_cur_assembly, nullptr /* No parent */);
                m_namespaces[new_ns->get_id()] = new_ns;
            }
            assert(m_namespaces.find(name) != m_namespaces.end());
            setup_cur_ns_body(m_namespaces.at(name));
        }
    }

    // Pops a namespace from the namespace scope.
    void xmeta_idl_reader::pop_namespace()
    {
        if (m_cur_namespace_body != nullptr)
        {
            if (m_cur_namespace_body->get_containing_namespace()->get_parent_namespace() != nullptr)
            {
                m_cur_namespace_body = m_cur_namespace_body->get_containing_namespace()->get_parent_namespace()->get_namespace_bodies().back();
            }
            else
            {
                m_cur_namespace_body = nullptr;
            }
        }
    }

    void xmeta_idl_reader::reset(std::string_view const& assembly_name)
    {
        m_assembly_names.clear();
        m_assembly_names.emplace_back(assembly_name);
        m_cur_assembly = m_assembly_names.back();
        m_namespaces.clear();
        m_cur_namespace_body = nullptr;
        m_cur_class = nullptr;
        m_cur_interface = nullptr;
        m_cur_struct = nullptr;
        m_num_semantic_errors = 0;
    }

    void xmeta_idl_reader::write_error(size_t decl_line, std::string_view const& msg)
    {
        std::cerr << "Semantic error (line " << decl_line << "): " << msg << std::endl;
        m_num_semantic_errors++;
    }

    void xmeta_idl_reader::write_enum_member_name_error(size_t decl_line, std::string_view const& invalid_name, std::string_view const& enum_name)
    {
        std::ostringstream oss;
        oss << "Enum member '" << invalid_name << "' already defined in enum '" << get_cur_ns_name() << "." << enum_name << "'";
        write_error(decl_line, oss.str());
    }

    void xmeta_idl_reader::write_enum_member_expr_ref_error(size_t decl_line, std::string_view const& invalid_name, std::string_view const& enum_name)
    {
        std::ostringstream oss;
        oss << "Enum member '" << invalid_name << "' not defined in enum '" << get_cur_ns_name() << "." << enum_name << "'";
        write_error(decl_line, oss.str());
    }

    void xmeta_idl_reader::write_enum_circular_dependency(size_t decl_line, std::string_view const& invalid_member_id, std::string_view const& enum_name)
    {
        std::ostringstream oss;
        oss << "Enum '" << enum_name << "' has a circular depencency, starting at member '" << invalid_member_id << "'";
        write_error(decl_line, oss.str());
    }

    void xmeta_idl_reader::write_enum_const_expr_range_error(size_t decl_line, std::string_view const& invalid_expr, std::string_view const& enum_name)
    {
        std::ostringstream oss;
        oss << "Constant expression '" << invalid_expr << "' not in range of enum '";
        oss << get_cur_ns_name() << "." << enum_name << "'";
        write_error(decl_line, oss.str());
    }

    void xmeta_idl_reader::write_namespace_name_error(size_t decl_line, std::string_view const& invalid_name, std::string_view const& original_name)
    {
        std::ostringstream oss;
        oss << "Namespace name '" << invalid_name << "' invalid. There already exists a namespace '" << original_name << "', and names cannot differ only by case.";
        write_error(decl_line, oss.str());
    }

    void xmeta_idl_reader::write_namespace_member_name_error(size_t decl_line, std::string_view const& invalid_name)
    {
        std::ostringstream oss;
        oss << "Member name '" << invalid_name << "' already defined in namespace '" << get_cur_ns_name() << "'";
        write_error(decl_line, oss.str());
    }

    void xmeta_idl_reader::write_property_duplicate_get_error(size_t decl_line, std::string_view const& container_name, std::string_view const& invalid_name)
    {
        std::ostringstream oss;
        oss << "Property '" << get_cur_ns_name() << "." << container_name << "." << invalid_name << "' defines get more than once.";
        write_error(decl_line, oss.str());
    }

    void xmeta_idl_reader::write_property_duplicate_set_error(size_t decl_line, std::string_view const& container_name, std::string_view const& invalid_name)
    {
        std::ostringstream oss;
        oss << "Property '" << get_cur_ns_name() << "." << container_name << "." << invalid_name << "' defines set more than once.";
        write_error(decl_line, oss.str());
    }

    void xmeta_idl_reader::write_duplicate_modifier_error(size_t decl_line, std::string_view const& modifier_name, std::string_view const& member_name)
    {
        std::ostringstream oss;
        oss << "'" << modifier_name << "' modifier defined twice on member '" << get_cur_ns_name() << "." << member_name;
        write_error(decl_line, oss.str());
    }

    std::string copy_to_lower(std::string_view sv)
    {
        std::string s{ sv };
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        return s;
    }

    inline std::string const xmeta_idl_reader::get_cur_ns_name() const
    {
        return m_cur_namespace_body->get_containing_namespace()->get_fully_qualified_id();
    }
}

