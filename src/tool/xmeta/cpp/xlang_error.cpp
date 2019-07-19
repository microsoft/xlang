
#include "xlang_error.h"
namespace xlang::xmeta
{
    void xlang_error_manager::report_error(idl_error error, size_t decl_line)
    {
        error_list.push_back(error_model{ error, decl_line, nullptr });
        print_error(error_list.at(error_list.size() - 1));
    }

    void xlang_error_manager::report_error(idl_error error, size_t decl_line, std::string_view const& symbol)
    {
        error_list.push_back(error_model{ error, decl_line, std::string{ symbol } });
        print_error(error_list.at(error_list.size() - 1));
    }

    void xlang_error_manager::print_error(error_model const& model)
    {
        std::cerr << "error XIDL" << model.error_code << " : [line] " << model.decl_line <<  " [msg] " << errors_message.at(model.error_code) << std::endl;
        m_num_semantic_errors++;
    }
}
