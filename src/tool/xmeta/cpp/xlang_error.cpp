
#include "xlang_error.h"
namespace xlang::xmeta
{
    void xlang_error_manager::write_error(size_t decl_line, std::string_view const& msg)
    {
        std::cerr << "Semantic error (line " << decl_line << "): " << msg << std::endl;
        m_num_semantic_errors++;
    }

    void xlang_error_manager::write_unresolved_type_error(size_t decl_line, std::string symbol)
    {
        std::ostringstream oss;
        oss << "Unable to resolve type: " << symbol;
        write_error(decl_line, oss.str());
    }

    void xlang_error_manager::write_struct_field_error(size_t decl_line, std::string symbol)
    {
        std::ostringstream oss;
        oss << "Struct has circular fields: " << symbol;
        write_error(decl_line, oss.str());
    }

    void xlang_error_manager::write_struct_field_error(size_t decl_line, std::string_view const& invalid_name, std::string_view const& struct_name)
    {
        std::ostringstream oss;
        oss << "Struct member " << invalid_name << " already defined in struct " << struct_name;
        write_error(decl_line, oss.str());
    }

    void xlang_error_manager::write_enum_member_name_error(size_t decl_line, std::string_view const& invalid_name, std::string_view const& enum_name, std::string_view const& namespace_id)
    {
        std::ostringstream oss;
        oss << "Enum member '" << invalid_name << "' already defined in enum '" <<namespace_id << "." << enum_name << "'";
        write_error(decl_line, oss.str());
    }

    void xlang_error_manager::write_enum_member_expr_ref_error(size_t decl_line, std::string_view const& invalid_name, std::string_view const& enum_name, std::string_view const& namespace_id)
    {
        std::ostringstream oss;
        oss << "Enum member '" << invalid_name << "' not defined in enum '" << namespace_id << "." << enum_name << "'";
        write_error(decl_line, oss.str());
    }

    void xlang_error_manager::write_enum_circular_dependency(size_t decl_line, std::string_view const& invalid_member_id, std::string_view const& enum_name)
    {
        std::ostringstream oss;
        oss << "Enum '" << enum_name << "' has a circular depencency, starting at member '" << invalid_member_id << "'";
        write_error(decl_line, oss.str());
    }

    void xlang_error_manager::write_enum_const_expr_range_error(size_t decl_line, std::string_view const& invalid_expr, std::string_view const& enum_name, std::string_view const& namespace_id)
    {
        std::ostringstream oss;
        oss << "Constant expression '" << invalid_expr << "' not in range of enum '";
        oss << namespace_id << "." << enum_name << "'";
        write_error(decl_line, oss.str());
    }

    void xlang_error_manager::write_namespace_name_error(size_t decl_line, std::string_view const& invalid_name, std::string_view const& original_name)
    {
        std::ostringstream oss;
        oss << "Namespace name '" << invalid_name << "' invalid. There already exists a namespace '" << original_name << "', and names cannot differ only by case.";
        write_error(decl_line, oss.str());
    }

    void xlang_error_manager::write_namespace_member_name_error(size_t decl_line, std::string_view const& invalid_name, std::string_view const& namespace_id)
    {
        std::ostringstream oss;
        oss << "Member name '" << invalid_name << "' already defined in namespace '" << namespace_id << "'";
        write_error(decl_line, oss.str());
    }

    void xlang_error_manager::write_not_an_interface_error(size_t decl_line, std::string symbol)
    {
        std::ostringstream oss;
        oss << "Member name '" << symbol << "' is not an interface.";
        write_error(decl_line, oss.str());
    }

    void xlang_error_manager::write_not_a_delegate_error(size_t decl_line, std::string symbol)
    {
        std::ostringstream oss;
        oss << "Member name '" << symbol << "' is not an delegate.";
        write_error(decl_line, oss.str());
    }
}
