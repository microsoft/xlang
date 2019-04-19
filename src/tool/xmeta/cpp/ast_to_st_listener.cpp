#include <algorithm>
#include <stdexcept>
#include "symbol_table.h"
#include "models/xmeta_models.h"
#include "ast_to_st_listener.h"

using namespace xlang::xmeta;
extern bool semantic_error_exists;

std::map<std::string, enum_semantics> enum_semantics_map
{
    { "Int8", enum_semantics::Int8 },
    { "UInt8", enum_semantics::Uint8 },
    { "Int16", enum_semantics::Int16 },
    { "UInt16", enum_semantics::Uint16 },
    { "Int32", enum_semantics::Int32 },
    { "UInt32", enum_semantics::Uint32 },
    { "Int64", enum_semantics::Int64 },
    { "UInt64", enum_semantics::Uint64 }
};

ast_to_st_listener::ast_to_st_listener(xmeta_symbol_table & symbol_table) :
    m_st{ symbol_table }
{ }

bool ast_to_st_listener::extract_enum_member(XlangParser::Enum_member_declarationContext *ast_enum_member, std::shared_ptr<enum_model> const& new_enum)
{
     auto const& enum_member_id = ast_enum_member->enum_identifier()->getText();
     auto decl_line = ast_enum_member->enum_identifier()->IDENTIFIER()->getSymbol()->getLine();
     auto expr = ast_enum_member->enum_expression();
     auto ec = std::errc();
     if (expr)
     {
         if (new_enum->member_exists(enum_member_id))
         {
             m_st.write_enum_member_name_error(
                 decl_line,
                 enum_member_id,
                 new_enum->get_id());
             return false;
         }
         std::string str_val = expr->getText();
         if (expr->enum_decimal_integer() || expr->enum_hexdecimal_integer())
         {
             if (expr->enum_hexdecimal_integer())
             {
                 assert(str_val.size() > 2);
                 assert(str_val.compare(0, 2, "0x") == 0);
                 str_val.erase(0, 2);
             }
             enum_member e_member{ enum_member_id, decl_line, str_val };
             if (expr->enum_decimal_integer())
             {
                 ec = e_member.resolve_decimal_val(new_enum->get_type());
             }
             else if (expr->enum_hexdecimal_integer())
             {
                 ec = e_member.resolve_hexadecimal_val(new_enum->get_type());
             }
             new_enum->add_member(std::move(e_member));
         }
         else if (expr->enum_expresssion_identifier())
         {
             // Only handles implicit dependence. Explicit dependence is checked and resolved in resolve_enum_val.
             new_enum->add_member(enum_member{ enum_member_id, decl_line, str_val });
             return true;
         }
     }
     else
     {
         // No expression implies implicit dependence.
         if (new_enum->get_members().size() == 0)
         {
             enum_member e_member{ enum_member_id, decl_line, "0" };
             e_member.resolve_decimal_val(new_enum->get_type());
             new_enum->add_member(std::move(e_member));
         }
         else
         {
             enum_member e_member{ enum_member_id, decl_line, "" };
             auto const& val = new_enum->get_members().back().get_value();
             if (!val.is_resolved() && val.get_ref_name() == e_member.get_id())
             {
                 m_st.write_enum_circular_dependency(
                     e_member.get_decl_line(),
                     e_member.get_id(),
                     new_enum->get_id());
                 return false;
             }
             e_member.set_value(val);
             ec = e_member.increment(new_enum->get_type());
             new_enum->add_member(std::move(e_member));
         }
     }

     if (ec == std::errc::result_out_of_range)
     {
         m_st.write_enum_const_expr_range_error(
             decl_line,
             expr->getText(),
             enum_member_id);
         return false;
     }
     else
     {
         assert(ec == std::errc());
     }
     return true;
}

bool ast_to_st_listener::resolve_enum_val(enum_member& e_member, std::shared_ptr<enum_model> const& new_enum, std::set<std::string_view>& dependents)
{
    auto const& val = e_member.get_value();
    if (val.is_resolved())
    {
        return true;
    }
    else
    {
        if (dependents.find(e_member.get_id()) != dependents.end())
        {
            m_st.write_enum_circular_dependency(
                e_member.get_decl_line(),
                e_member.get_id(),
                new_enum->get_id());
                return false;
        }
        dependents.emplace(e_member.get_id());
        auto const& ref_name = val.get_ref_name();
        if (!new_enum->member_exists(ref_name))
        {
            m_st.write_enum_member_expr_ref_error(e_member.get_decl_line(), ref_name, new_enum->get_id());
            return false;
        }
        auto ref_member = new_enum->get_member(ref_name);
        if (!resolve_enum_val(ref_member, new_enum, dependents))
        {
            return false;
        }
        e_member.set_value(ref_member.get_value());
    }
    return true;
}

void ast_to_st_listener::exitEnum_declaration(XlangParser::Enum_declarationContext *ctx)
{
    auto id = ctx->IDENTIFIER();
    auto decl_line = id->getSymbol()->getLine();
    enum_semantics type = enum_semantics::Int32;

    if (ctx->enum_base())
    {
        assert(ctx->enum_base()->enum_integral_type() != nullptr);
        type = enum_semantics_map[ctx->enum_base()->enum_integral_type()->getText()];
    }

    std::shared_ptr<enum_model> new_enum = std::make_shared<enum_model>(id->getText(), decl_line, m_st.get_cur_assembly(), type);

    for (auto field : ctx->enum_body()->enum_member_declaration())
    {
        if (!extract_enum_member(field, new_enum))
        {
            return;
        }
    }

    for (enum_member& e_member : new_enum->get_members())
    {
        if (e_member.get_value().is_resolved())
        {
            continue;
        }
        std::set<std::string_view> dependents;
        if (!resolve_enum_val(e_member, new_enum, dependents))
        {
            return;
        }
    }

    m_st.get_cur_namespace_body()->add_enum(new_enum);
}

void ast_to_st_listener::enterNamespace_declaration(XlangParser::Namespace_declarationContext *ctx)
{
    auto id = ctx->IDENTIFIER();
    if (id)
    {
        std::string token;
        std::istringstream tokenStream(id->getText());
        while (std::getline(tokenStream, token, '.'))
        {
            m_st.push_namespace(token, id->getSymbol()->getLine());
        }
    }
}

void ast_to_st_listener::exitNamespace_declaration(XlangParser::Namespace_declarationContext *ctx)
{
    auto id = ctx->IDENTIFIER();
    auto const& id_text = id->getText();
    size_t num_of_ns = std::count(id_text.begin(), id_text.end(), '.') + 1;
    for (size_t i = 0; i < num_of_ns; ++i)
    {
        m_st.pop_namespace();
    }
}
