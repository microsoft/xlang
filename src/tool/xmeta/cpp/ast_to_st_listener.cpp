#include <algorithm>
#include <stdexcept>

#include "ast_to_st_listener.h"
#include "xmeta_idl_reader.h"
#include "models/xmeta_models.h"
#include "XlangParser.h"

using namespace xlang::xmeta;

enum_semantics str_to_enum_semantics(std::string const& val)
{
    const std::map<std::string, enum_semantics> str_to_enum_type_map = {
        { "Int8", enum_semantics::Int8 },
        { "Int16", enum_semantics::Int16 },
        { "Int32", enum_semantics::Int32 },
        { "Int64", enum_semantics::Int64 },
        { "UInt8", enum_semantics::UInt8 },
        { "UInt16", enum_semantics::UInt16 },
        { "UInt32", enum_semantics::UInt32 },
        { "UInt64", enum_semantics::UInt64 },
    };
    assert(str_to_enum_type_map.find(val) != str_to_enum_type_map.end());
    return str_to_enum_type_map.at(val);
}

simple_type str_to_simple_type(std::string const& val)
{
    const std::map<std::string, simple_type> str_to_simple_type_map = {
        { "Boolean", simple_type::Boolean },
        { "String", simple_type::String },
        { "Int8", simple_type::Int8 },
        { "Int16", simple_type::Int16 },
        { "Int32", simple_type::Int32 },
        { "Int64", simple_type::Int64 },
        { "UInt8", simple_type::UInt8 },
        { "UInt16", simple_type::UInt16 },
        { "UInt32", simple_type::UInt32 },
        { "UInt64", simple_type::UInt64 },
        { "Char16", simple_type::Char16 },
        { "Single", simple_type::Single },
        { "Double", simple_type::Double },
    };
    assert(str_to_simple_type_map.find(val) != str_to_simple_type_map.end());
    return str_to_simple_type_map.at(val);
}

ast_to_st_listener::ast_to_st_listener(xmeta_idl_reader& reader) :
    m_reader{ reader }
{ }

listener_error ast_to_st_listener::extract_type(XlangParser::Return_typeContext* rtc, std::optional<type_ref>& tr)
{
    assert(rtc);
    if (rtc->VOID())
    {
        tr = std::nullopt;
        return listener_error::passed;
    }
    assert(rtc->type());
    return extract_type(rtc->type(), *tr);
}

// Class types are to be resolved later in thse second pass
listener_error ast_to_st_listener::extract_type(XlangParser::TypeContext* tc, type_ref& tr)
{
    if (tc->value_type())
    {
        tr.set_semantic(str_to_simple_type(tc->getText()));
    }
    else if (tc->class_type())
    {
        if (tc->class_type()->OBJECT())
        {
            tr.set_semantic(object_type{});
        }
    }
    else if (tc->array_type())
    {
        assert(tc->array_type()->non_array_type());
        if (tc->array_type()->non_array_type()->value_type())
        {
            tr.set_semantic(str_to_simple_type(tc->getText()));
        }
        else if (tc->array_type()->non_array_type()->class_type())
        {
            if (tc->array_type()->non_array_type()->class_type()->OBJECT())
            {
                tr.set_semantic(object_type{});
            }
        }
    }
    return listener_error::passed;
}

void ast_to_st_listener::extract_formal_params(std::vector<XlangParser::Fixed_parameterContext*> const& ast_formal_params, 
    std::variant<std::shared_ptr<delegate_model>, std::shared_ptr<method_model>> const& model)
{
    for (auto fixed_param : ast_formal_params)
    {
        auto id = fixed_param->IDENTIFIER();
        std::string formal_param_name{ id->getText() };
        auto decl_line = id->getSymbol()->getLine();
        type_ref tr{ fixed_param->type()->getText() };
        extract_type(fixed_param->type(), tr);
        parameter_semantics sem = parameter_semantics::in;
        if (fixed_param->parameter_modifier() != nullptr)
        {
            if (fixed_param->parameter_modifier()->CONST())
            {
                sem = parameter_semantics::const_ref;
            }
            else if (fixed_param->parameter_modifier()->REF())
            {
                sem = parameter_semantics::ref;
            }
            else if (fixed_param->parameter_modifier()->OUT())
            {
                sem = parameter_semantics::out;
            }
        }
        if (std::holds_alternative<std::shared_ptr<delegate_model>>(model))
        {
            std::get<std::shared_ptr<delegate_model>>(model)->add_formal_parameter(formal_parameter_model{ formal_param_name, decl_line, m_reader.m_current_assembly, sem, std::move(tr) });
        }
        else if (std::holds_alternative<std::shared_ptr<method_model>>(model))
        {
            std::get<std::shared_ptr<method_model>>(model)->add_formal_parameter(formal_parameter_model{ formal_param_name, decl_line, m_reader.m_current_assembly, sem, std::move(tr) });
        }
    }
}

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

void ast_to_st_listener::enterInterface_declaration(XlangParser::Interface_declarationContext *ctx)
{
    auto id = ctx->IDENTIFIER();
    std::string interface_name{ id->getText() };
    auto decl_line = id->getSymbol()->getLine();

    std::string symbol = m_reader.get_cur_namespace_body()->get_containing_namespace()->get_fully_qualified_id() + "." + interface_name;
    if (m_reader.type_declaration_exists(symbol))
    {
        m_reader.write_redeclaration_error(symbol, decl_line);
        return;
    }

    auto model = std::make_shared<interface_model>(interface_name, decl_line, m_reader.get_cur_assembly(), m_reader.get_cur_namespace_body());
    
    auto const& interface_body = ctx->interface_body();
    for (auto const& interface_member : interface_body->interface_member_declaration())
    {
        if (interface_member->interface_method_declaration())
        {
            auto interface_method = interface_member->interface_method_declaration();
            std::string method_id = interface_method->IDENTIFIER()->getText();

            std::optional<type_ref> tr = type_ref{ interface_method->return_type()->getText() };
            extract_type(interface_method->return_type(), tr);

            auto met_model = std::make_shared<method_model>(method_id, interface_method->IDENTIFIER()->getSymbol()->getLine(), m_reader.get_cur_assembly(), std::move(tr));
            if (interface_method->formal_parameter_list())
            {
                extract_formal_params(interface_method->formal_parameter_list()->fixed_parameter(), met_model);
            }
            model->add_member(met_model);
        }
        if (interface_member->interface_property_declaration())
        {
            extract_property_accessors(interface_member->interface_property_declaration(), model);
        }
        if (interface_member->interface_event_declaration())
        {
            extract_event_accessors(interface_member->interface_event_declaration(), model);
        }
    }
    if (ctx->interface_base())
    {
        auto const& interface_bases = ctx->interface_base()->type_list()->type_base();
        for (auto const& interface_base : interface_bases)
        {
            model->add_interface_base_ref(interface_base->type()->getText());
        }
    }
    m_reader.m_cur_namespace_body->add_interface(model);
}

listener_error ast_to_st_listener::extract_property_accessors(XlangParser::Interface_property_declarationContext* interface_property, std::shared_ptr<class_or_interface_model> model)
{
    std::string property_id = interface_property->IDENTIFIER()->getText();
    type_ref tr{ interface_property->type()->getText() };
    extract_type(interface_property->type(), tr);
    auto decl_line = interface_property->IDENTIFIER()->getSymbol()->getLine();

    std::shared_ptr<method_model> get_method = nullptr;
    std::shared_ptr<method_model> set_method = nullptr;
    if (interface_property->property_accessors()->property_accessor_method().size() > 0)
    {
        auto const& property_accessor_methods = interface_property->property_accessors()->property_accessor_method();
        if (property_accessor_methods.size() == 1)
        {
            if (!property_accessor_methods[0]->GET())
            {
                // WRITE SEMANTIC ERROR. Always need a get property.
                return listener_error::failed;
            }
        }
        else if (property_accessor_methods.size() == 2)
        {
            if ((property_accessor_methods[0]->GET() && property_accessor_methods[1]->GET())
                || (property_accessor_methods[0]->SET() && property_accessor_methods[1]->SET()))
            {
                // WRITE SEMANTIC ERROR
                return listener_error::failed;
            }
        }
        else if (property_accessor_methods.size() > 2)
        {
            // THIS PARSER BE TRIPPING :O
            return listener_error::failed;
        }

        for (auto const& property_accessor : property_accessor_methods)
        {
            if (property_accessor->GET())
            {
                get_method = std::make_shared<method_model>("get_" + property_id, property_accessor->GET()->getSymbol()->getLine(), m_reader.get_cur_assembly(), std::move(tr));
                model->add_member(get_method);
            }
            else if (property_accessor->SET())
            {
                set_method = std::make_shared<method_model>("set_" + property_id, property_accessor->SET()->getSymbol()->getLine(), m_reader.get_cur_assembly(), std::move(tr));
                model->add_member(set_method);
            }
        }
    } 
    else // Implicity declaration
    {
        get_method = std::make_shared<method_model>("get_" + property_id, decl_line, m_reader.get_cur_assembly(), std::move(tr));
        model->add_member(get_method);
        set_method = std::make_shared<method_model>("set_" + property_id, decl_line, m_reader.get_cur_assembly(), std::move(tr));
        model->add_member(set_method);    
    }
    

    auto prop_model = std::make_shared<property_model>(property_id, decl_line, m_reader.get_cur_assembly(), std::move(tr));
    prop_model->set_get_method(get_method);
    prop_model->set_set_method(set_method);
    model->add_member(prop_model);
    return listener_error::passed;
}

listener_error ast_to_st_listener::extract_event_accessors(XlangParser::Interface_event_declarationContext* interface_event, std::shared_ptr<class_or_interface_model> model)
{
    std::string event_id = interface_event->IDENTIFIER()->getText();
    type_ref tr{ interface_event->type()->getText() };
    extract_type(interface_event->type(), tr);
    auto decl_line = interface_event->IDENTIFIER()->getSymbol()->getLine();
    std::shared_ptr<method_model> add_method = std::make_shared<method_model>("add_" + event_id, decl_line, m_reader.get_cur_assembly(), std::move(tr));
    std::shared_ptr<method_model> remove_method = std::make_shared<method_model>("remove_" + event_id, decl_line, m_reader.get_cur_assembly(), std::move(tr));

    model->add_member(add_method);
    model->add_member(remove_method);
    auto event = std::make_shared<event_model>(event_id, decl_line, m_reader.get_cur_assembly(), add_method, remove_method, std::move(tr));
    model->add_member(event);
    return listener_error::passed;
}

void ast_to_st_listener::enterDelegate_declaration(XlangParser::Delegate_declarationContext* ctx)
{
    auto id = ctx->IDENTIFIER();
    std::string delegate_name{ id->getText() };
    auto decl_line = id->getSymbol()->getLine();

    std::string symbol = m_reader.get_cur_namespace_body()->get_containing_namespace()->get_fully_qualified_id() + "." + delegate_name;
    if (m_reader.type_declaration_exists(symbol))
    {
        m_reader.write_redeclaration_error(symbol, decl_line);
        return;
    }

    std::optional<type_ref> tr = type_ref{ ctx->return_type()->getText() };
    extract_type(ctx->return_type(), tr);
    auto dm = std::make_shared<delegate_model>(delegate_name, decl_line, m_reader.get_cur_assembly(), m_reader.get_cur_namespace_body(), std::move(tr));

    auto formal_params = ctx->formal_parameter_list();
    if (formal_params)
    {
        extract_formal_params(formal_params->fixed_parameter(), dm);
    }

    m_reader.m_cur_namespace_body->add_delegate(dm);
    m_reader.symbols[symbol] = dm;
}

void ast_to_st_listener::enterEnum_declaration(XlangParser::Enum_declarationContext *ctx)
{
    auto id = ctx->IDENTIFIER();
    std::string enum_name{ id->getText() };
    auto decl_line = id->getSymbol()->getLine();
    enum_semantics type = enum_semantics::Int32;
    std::string symbol = m_reader.get_cur_namespace_body()->get_containing_namespace()->get_fully_qualified_id() + "." + enum_name;
    if (m_reader.type_declaration_exists(symbol))
    {
        m_reader.write_redeclaration_error(symbol, decl_line);
        return;
    }

    if (ctx->enum_base())
    {
        assert(ctx->enum_base()->enum_integral_type() != nullptr);
        type = str_to_enum_semantics(ctx->enum_base()->enum_integral_type()->getText());
    }

    auto new_enum = std::make_shared<enum_model>(enum_name, decl_line, m_reader.get_cur_assembly(), m_reader.get_cur_namespace_body(), type);

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
    m_reader.symbols[symbol] = new_enum;
}

void ast_to_st_listener::enterStruct_declaration(XlangParser::Struct_declarationContext *ctx)
{
    auto id = ctx->IDENTIFIER();
    auto decl_line = id->getSymbol()->getLine();
    std::string struct_name{ id->getText() };
    std::string symbol = m_reader.get_cur_namespace_body()->get_containing_namespace()->get_fully_qualified_id() + "." + struct_name;

    if (m_reader.type_declaration_exists(symbol))
    {
        // TODO: Reccord the semantic error and continue
        m_reader.write_redeclaration_error(symbol, decl_line);
        return;
    }

    auto new_struct = std::make_shared<struct_model>(struct_name, decl_line, m_reader.get_cur_assembly(), m_reader.get_cur_namespace_body());

    for (auto field : ctx->struct_body()->field_declaration())
    {
        type_ref tr{ field->type()->getText() };
        extract_type(field->type(), tr);
        new_struct->add_field(std::pair(tr, field->IDENTIFIER()->getText()));
    }

    m_reader.m_cur_namespace_body->add_struct(new_struct);
    m_reader.symbols[symbol] = new_struct;
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
