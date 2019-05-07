#include <assert.h>
#include <string.h>

#include "xmeta_idl_reader.h"
#include "ast_to_st_listener.h"
#include "XlangLexer.h"
#include "XlangParser.h"
#include "xlang_model_walker.h"

namespace xlang::xmeta
{
    void xmeta_idl_reader::read(std::istream& idl_contents, bool disable_error_reporting)
    {
        ast_to_st_listener listener{ *this };
        read(idl_contents, listener, disable_error_reporting);
    }

    void xmeta_idl_reader::read(std::istream& idl_contents, XlangParserBaseListener& listener, bool disable_error_reporting)
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
        resolve();
        m_error_manager.set_num_syntax_error(parser.getNumberOfSyntaxErrors());
    }

    void xmeta_idl_reader::resolve()
    {
        xlang_model_walker walker(m_namespaces, *this);
        walker.walk();
    }

    void xmeta_idl_reader::listen_struct_model(std::shared_ptr<struct_model> const& model) 
    {
        model->resolve(symbols, m_error_manager);
        model->has_circular_struct_declarations(symbols, m_error_manager);
    }

    void xmeta_idl_reader::listen_delegate_model(std::shared_ptr<delegate_model> const& model) 
    {
        model->resolve(symbols, m_error_manager);
    }
}

