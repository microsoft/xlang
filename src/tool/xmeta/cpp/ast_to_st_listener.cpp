#include <algorithm>
#include <stdexcept>

#include "ast_to_st_listener.h"
#include "xmeta_idl_reader.h"
#include "models/xmeta_models.h"

using namespace xlang::xmeta;

enum_semantics str_to_enum_semantics(std::string const& val)
{
    if (val == "Int8")
    {
        return enum_semantics::Int8;
    }
    else if (val == "UInt8")
    {
        return enum_semantics::UInt8;
    }
    else if (val == "Int16")
    {
        return enum_semantics::Int16;
    }
    else if (val == "UInt16")
    {
        return enum_semantics::UInt16;
    }
    else if (val == "Int32")
    {
        return enum_semantics::Int32;
    }
    else if (val == "UInt32")
    {
        return enum_semantics::UInt32;
    }
    else if (val == "Int64")
    {
        return enum_semantics::Int64;
    }
    else if (val == "UInt64")
    {
        return enum_semantics::UInt64;
    }
    assert(false);
    return enum_semantics::Int32;
}

ast_to_st_listener::ast_to_st_listener(xmeta_idl_reader& reader) :
    m_reader{ reader }
{ }


listener_error ast_to_st_listener::extract_enum_member(XlangParser::Enum_member_declarationContext *ast_enum_member, std::shared_ptr<enum_model> const& new_enum)
{
     auto const& enum_member_id = ast_enum_member->enum_identifier()->getText();
     auto decl_line = ast_enum_member->enum_identifier()->IDENTIFIER()->getSymbol()->getLine();
     auto expr = ast_enum_member->enum_expression();
     auto ec = std::errc();
     if (expr)
     {
         if (new_enum->member_exists(enum_member_id))
         {
             m_reader.write_enum_member_name_error(
                 decl_line,
                 enum_member_id,
                 new_enum->get_id());
             return listener_error::failed;
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
             return listener_error::passed;
         }
     }
     else
     {
         // No expression implies implicit dependence.
         if (new_enum->get_members().empty())
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
                 m_reader.write_enum_circular_dependency(
                     e_member.get_decl_line(),
                     e_member.get_id(),
                     new_enum->get_id());
                 return listener_error::failed;
             }
             e_member.set_value(val);
             if (e_member.get_value().is_resolved())
             {
                 ec = e_member.increment(new_enum->get_type());
             }
             new_enum->add_member(std::move(e_member));
         }
     }

     if (ec == std::errc::result_out_of_range)
     {
         m_reader.write_enum_const_expr_range_error(
             decl_line,
             expr->getText(),
             enum_member_id);
         return listener_error::failed;
     }
     else
     {
         assert(ec == std::errc());
     }
     return listener_error::passed;
}

listener_error ast_to_st_listener::resolve_enum_val(enum_member& e_member, std::shared_ptr<enum_model> const& new_enum, std::set<std::string_view>& dependents)
{
    auto const& val = e_member.get_value();
    if (val.is_resolved())
    {
        return listener_error::passed;
    }
    if (dependents.find(e_member.get_id()) != dependents.end())
    {
        m_reader.write_enum_circular_dependency(
            e_member.get_decl_line(),
            e_member.get_id(),
            new_enum->get_id());
            return listener_error::failed;
    }
    dependents.emplace(e_member.get_id());
    auto const& ref_name = val.get_ref_name();
    if (!new_enum->member_exists(ref_name))
    {
        m_reader.write_enum_member_expr_ref_error(e_member.get_decl_line(), ref_name, new_enum->get_id());
        return listener_error::failed;
    }
    auto ref_member = new_enum->get_member(ref_name);
    if (resolve_enum_val(ref_member, new_enum, dependents) == listener_error::failed)
    {
        return listener_error::failed;
    }
    e_member.set_value(ref_member.get_value());
    return listener_error::passed;
}

void ast_to_st_listener::exitEnum_declaration(XlangParser::Enum_declarationContext *ctx)
{
    auto id = ctx->IDENTIFIER();
    std::string enum_name{ id->getText() };
    auto decl_line = id->getSymbol()->getLine();
    enum_semantics type = enum_semantics::Int32;

    if (ctx->enum_base())
    {
        assert(ctx->enum_base()->enum_integral_type() != nullptr);
        type = str_to_enum_semantics(ctx->enum_base()->enum_integral_type()->getText());
    }

    std::shared_ptr<enum_model> new_enum = std::make_shared<enum_model>(id->getText(), decl_line, m_reader.get_cur_assembly(), m_reader.get_cur_namespace_body(), type);

    for (auto field : ctx->enum_body()->enum_member_declaration())
    {
        if (extract_enum_member(field, new_enum) == listener_error::failed)
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
        if (resolve_enum_val(e_member, new_enum, dependents) == listener_error::failed)
        {
            return;
        }
    }

    m_reader.m_cur_namespace_body->add_enum(new_enum);
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
            m_reader.push_namespace(token, id->getSymbol()->getLine());
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
        m_reader.pop_namespace();
    }
}
