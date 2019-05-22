#pragma once

#include <iostream>
#include <vector>
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

        void write_type_member_exists_error(size_t decl_line, std::string member);

        void write_unresolved_type_error(size_t decl_line, std::string symbol);

        void write_struct_field_error(size_t decl_line, std::string symbol);

        void write_not_an_interface_error(size_t decl_line, std::string symbol);

        void write_not_a_delegate_error(size_t decl_line, std::string symbol);

        void write_struct_field_error(size_t decl_line, 
            std::string_view const& invalid_name, std::string_view const& struct_name);

        void write_enum_member_name_error(size_t decl_line, 
            std::string_view const& invalid_name, std::string_view const& enum_name, std::string_view const& namespace_id);

        void write_enum_member_expr_ref_error(size_t decl_line, 
            std::string_view const& invalid_name, std::string_view const& enum_name, std::string_view const& namespace_id);

        void write_enum_circular_dependency(size_t decl_line, 
            std::string_view const& invalid_member_id, std::string_view const& enum_name);

        void write_enum_const_expr_range_error(size_t decl_line, 
            std::string_view const& invalid_expr, std::string_view const& enum_name, std::string_view const& namespace_id);

        void write_namespace_name_error(size_t decl_line, 
            std::string_view const& invalid_name, std::string_view const& original_name);

        void write_namespace_member_name_error(size_t decl_line, 
            std::string_view const& invalid_name, std::string_view const& namespace_id);

        void write_property_accessor_error(size_t decl_line, std::string member);
        
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

        void set_num_syntax_error(size_t const& count)
        {
            m_num_syntax_errors = count;
        }

    private:
        size_t m_num_semantic_errors = 0;
        size_t m_num_syntax_errors = 0;

        /* 
            Prototyping how I would do the errors, I would store each type of error in these vectors.
            And then they will be used to print out the actual error statements later. These vectors would
            contain a struct with an error code, the symbol associated with it, and the decl line
            
            struct error_model
            {
                size error_level;
                size_t error_code;
                size_t decl_line;
                std::string symbol;
            }
            std::vector<error_model> errors; 
        */
    };
}
