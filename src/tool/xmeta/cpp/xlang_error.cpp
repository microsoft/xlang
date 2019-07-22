
#include "xlang_error.h"

namespace xlang::xmeta
{
    void xlang_error_manager::report_error(idl_error error, size_t decl_line)
    {
        error_list.push_back(error_model{ error, decl_line, "" });
        print_error(error_list.at(error_list.size() - 1));
    }

    void xlang_error_manager::report_error(idl_error error, size_t decl_line, std::string_view const& symbol)
    {
        error_list.push_back(error_model{ error, decl_line, std::string{ symbol } });
        print_error(error_list.at(error_list.size() - 1));
    }

    void xlang_error_manager::print_error(error_model const& model)
    {
        std::cerr << "error XIDL" << model.error_code << " : [line] " << model.decl_line <<  " [msg] " << errors_message.at(model.error_code);
        if (model.symbol != "")
        {
            std::cerr << " [symbol] " << model.symbol;
        }
        std::cerr << std::endl;
        m_num_semantic_errors++;
    }

    bool xlang_error_manager::error_exists(idl_error code, std::string symbol, size_t decl_line)
    {
        auto same_error = [&code, &symbol, &decl_line](error_model const& model)
        {
            return model.error_code == code && model.symbol == symbol && model.decl_line == decl_line;
        };
        return std::find_if(error_list.begin(), error_list.end(), same_error) != error_list.end();
    }
}
