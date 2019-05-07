#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>

namespace xlang::xmeta
{
    class xlang_error_manager
    {
    public:
        void write_error(size_t decl_line, std::string_view const& msg);
        void write_redeclaration_error(std::string symbol, size_t decl_line);
        void write_unresolved_type_error(std::string symbol, size_t decl_line);
        void write_struct_field_error(std::string symbol, size_t decl_line);
        void write_enum_member_name_error(size_t decl_line, std::string_view const& invalid_name, std::string_view const& enum_name, std::string_view const& namespace_id);
        void write_enum_member_expr_ref_error(size_t decl_line, std::string_view const& invalid_name, std::string_view const& enum_name, std::string_view const& namespace_id);
        void write_enum_circular_dependency(size_t decl_line, std::string_view const& invalid_member_id, std::string_view const& enum_name);
        void write_enum_const_expr_range_error(size_t decl_line, std::string_view const& invalid_expr, std::string_view const& enum_name, std::string_view const& namespace_id);
        void write_namespace_name_error(size_t decl_line, std::string_view const& invalid_name, std::string_view const& original_name);
        void write_namespace_member_name_error(size_t decl_line, std::string_view const& invalid_name, std::string_view const& namespace_id);
        
        size_t get_num_of_errors() const noexcept
        {
            return m_num_semantic_errors + m_num_syntax_errors; 
        }

        auto const& get_num_of_syntax_errors()
        {
            return m_num_syntax_errors;
        }

        auto const& get_num_of_semantic_errors()
        {
            return m_num_semantic_errors;
        }
        
        void increment_semantic_error_count()
        {
            m_num_semantic_errors++;
        }

    private:
        size_t m_num_semantic_errors = 0;
        size_t m_num_syntax_errors = 0;
    };
}