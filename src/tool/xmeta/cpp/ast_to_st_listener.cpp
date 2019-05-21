#include "ast_to_st_listener.h"
#include "xmeta_idl_reader.h"
#include "models/xmeta_models.h"
#include "XlangParser.h"
#include "impl/base.h"

using namespace xlang::xmeta;

namespace
{
    static const std::map<std::string, enum_semantics> str_to_enum_type_map = {
        { "Int8", enum_semantics::Int8 },
        { "Int16", enum_semantics::Int16 },
        { "Int32", enum_semantics::Int32 },
        { "Int64", enum_semantics::Int64 },
        { "UInt8", enum_semantics::UInt8 },
        { "UInt16", enum_semantics::UInt16 },
        { "UInt32", enum_semantics::UInt32 },
        { "UInt64", enum_semantics::UInt64 },
    };

    enum_semantics str_to_enum_semantics(std::string const& val)
    {
        auto const iter = str_to_enum_type_map.find(val);
        if (iter == str_to_enum_type_map.end())
        {
            xlang::throw_invalid("Can't convert ", val, " to enum simple type");
        }
        return iter->second;
    }

    static const std::map<std::string, simple_type> str_to_simple_type_map = {
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

    simple_type str_to_simple_type(std::string const& val)
    {
        auto const iter = str_to_simple_type_map.find(val);
        if (str_to_simple_type_map.find(val) == str_to_simple_type_map.end())
        {
            xlang::throw_invalid("Can't convert ", val, " to simple type");
        }
        return iter->second;
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
}

ast_to_st_listener::ast_to_st_listener(compilation_unit & xlang_model, xlang_error_manager & error_manager) 
    : xlang_model{ xlang_model }, error_manager{ error_manager }
{ }

listener_error ast_to_st_listener::extract_type(XlangParser::Return_typeContext* rtc, std::optional<type_ref>& tr)
{
    assert(rtc);
    if (rtc->VOID_LXR())
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

void ast_to_st_listener::extract_formal_params(std::vector<XlangParser::Fixed_parameterContext*> const& ast_formal_params, std::variant<std::shared_ptr<xlang::xmeta::delegate_model>, std::shared_ptr<xlang::xmeta::method_model>> const& model)
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
            if (fixed_param->parameter_modifier()->CONST_LXR())
            {
                sem = parameter_semantics::const_ref;
            }
            else if (fixed_param->parameter_modifier()->REF())
            {
                sem = parameter_semantics::ref;
            }
            else if (fixed_param->parameter_modifier()->OUT_LXR())
            {
                sem = parameter_semantics::out;
            }
        }
        if (std::holds_alternative<std::shared_ptr<delegate_model>>(model))
        {
            std::get<std::shared_ptr<delegate_model>>(model)->add_formal_parameter(formal_parameter_model{ formal_param_name, decl_line, m_cur_assembly, sem, std::move(tr) });
        }
        else if (std::holds_alternative<std::shared_ptr<method_model>>(model))
        {
            std::get<std::shared_ptr<method_model>>(model)->add_formal_parameter(formal_parameter_model{ formal_param_name, decl_line, m_cur_assembly, sem, std::move(tr) });
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
             error_manager.write_enum_member_name_error(
                 decl_line,
                 enum_member_id,
                 new_enum->get_id(),
                 m_cur_namespace_body->get_containing_namespace()->get_fully_qualified_id());
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
                 error_manager.write_enum_circular_dependency(
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
         error_manager.write_enum_const_expr_range_error(
             decl_line,
             expr->getText(),
             enum_member_id,
             m_cur_namespace_body->get_containing_namespace()->get_fully_qualified_id());
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
        error_manager.write_enum_circular_dependency(
            e_member.get_decl_line(),
            e_member.get_id(),
            new_enum->get_id());
            return listener_error::failed;
    }
    dependents.emplace(e_member.get_id());
    auto const& ref_name = val.get_ref_name();
    if (!new_enum->member_exists(ref_name))
    {
        error_manager.write_enum_member_expr_ref_error(e_member.get_decl_line(), ref_name, new_enum->get_id(), 
            m_cur_namespace_body->get_containing_namespace()->get_fully_qualified_id());
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

listener_error ast_to_st_listener::extract_property_accessors(std::string const& property_id, 
    type_ref & tr, 
    size_t decl_line, 
    XlangParser::Property_accessorsContext* property_accessors, 
    std::shared_ptr<class_or_interface_model> const& model)
{
    std::shared_ptr<method_model> get_method = nullptr;
    std::shared_ptr<method_model> set_method = nullptr;
    if (property_accessors->property_accessor_method().size() > 0)
    {
        auto const& property_accessor_methods = property_accessors->property_accessor_method();
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
                get_method = std::make_shared<method_model>("get_" + property_id, property_accessor->GET()->getSymbol()->getLine(), m_cur_assembly, std::move(tr), method_association::Property);
                model->add_member(get_method);
            }
            else if (property_accessor->SET())
            {
                set_method = std::make_shared<method_model>("set_" + property_id, property_accessor->SET()->getSymbol()->getLine(), m_cur_assembly, std::move(std::nullopt), method_association::Property);
                parameter_semantics sem = parameter_semantics::in;
                set_method->add_formal_parameter(formal_parameter_model{ "TODO:findname", decl_line, m_cur_assembly, sem, std::move(tr) });
                model->add_member(set_method);
            }
        }
    }
    else // Implicity declaration
    {
        get_method = std::make_shared<method_model>("get_" + property_id, decl_line, m_cur_assembly, std::move(tr), method_association::Property);
        model->add_member(get_method);
        set_method = std::make_shared<method_model>("set_" + property_id, decl_line, m_cur_assembly, std::move(std::nullopt), method_association::Property);
        parameter_semantics sem = parameter_semantics::in;
        set_method->add_formal_parameter(formal_parameter_model{ "TODO:findname", decl_line, m_cur_assembly, sem, std::move(tr) });
        model->add_member(set_method);
    }


    auto prop_model = std::make_shared<property_model>(property_id, decl_line, m_cur_assembly, std::move(tr));
    prop_model->set_get_method(get_method);
    prop_model->set_set_method(set_method);
    model->add_member(prop_model);
    return listener_error::passed;
}


listener_error ast_to_st_listener::extract_event_saccessors(std::string const& event_id,
    type_ref & tr,
    size_t decl_line,
    XlangParser::Event_accessorsContext* property_accessors,
    std::shared_ptr<class_or_interface_model> const& model)
{
    type_ref event_registration{ "Foundation.EventRegistrationToken" };
    parameter_semantics sem = parameter_semantics::in;

    std::shared_ptr<method_model> add_method = std::make_shared<method_model>("add_" + event_id, decl_line, m_cur_assembly, std::move(event_registration), method_association::Event);
    add_method->add_formal_parameter(formal_parameter_model{ "TODO:findname", decl_line, m_cur_assembly, sem, std::move(tr) });

    std::shared_ptr<method_model> remove_method = std::make_shared<method_model>("remove_" + event_id, decl_line, m_cur_assembly, std::move(std::nullopt), method_association::Event);
    remove_method->add_formal_parameter(formal_parameter_model{ "TODO:findname", decl_line, m_cur_assembly, sem, std::move(event_registration) });

    model->add_member(add_method);
    model->add_member(remove_method);
    auto event = std::make_shared<event_model>(event_id, decl_line, m_cur_assembly, add_method, remove_method, std::move(tr));
    model->add_member(event);
    return listener_error::passed;
}

listener_error ast_to_st_listener::extract_event_accessors(XlangParser::Interface_event_declarationContext* interface_event, std::shared_ptr<class_or_interface_model> model)
{
    std::string event_id = interface_event->IDENTIFIER()->getText();
    type_ref event_registration{ "Foundation.EventRegistrationToken" };
    type_ref tr{ interface_event->type()->getText() };
    extract_type(interface_event->type(), tr);
    auto decl_line = interface_event->IDENTIFIER()->getSymbol()->getLine();
    parameter_semantics sem = parameter_semantics::in;

    std::shared_ptr<method_model> add_method = std::make_shared<method_model>("add_" + event_id, decl_line, m_cur_assembly, std::move(event_registration), method_association::Event);
    add_method->add_formal_parameter(formal_parameter_model{ "TODO:findname", decl_line, m_cur_assembly, sem, std::move(tr) });

    std::shared_ptr<method_model> remove_method = std::make_shared<method_model>("remove_" + event_id, decl_line, m_cur_assembly, std::move(std::nullopt), method_association::Event);
    remove_method->add_formal_parameter(formal_parameter_model{ "TODO:findname", decl_line, m_cur_assembly, sem, std::move(event_registration) });

    model->add_member(add_method);
    model->add_member(remove_method);
    auto event = std::make_shared<event_model>(event_id, decl_line, m_cur_assembly, add_method, remove_method, std::move(tr));
    model->add_member(event);
    return listener_error::passed;
}

void ast_to_st_listener::enterClass_declaration(XlangParser::Class_declarationContext * ctx)
{
    auto id = ctx->IDENTIFIER();
    std::string class_name{ id->getText() };
    auto decl_line = id->getSymbol()->getLine();

    std::string symbol = m_cur_namespace_body->get_containing_namespace()->get_fully_qualified_id() + "." + class_name;
    class_semantics sem;
    for (auto const& class_mod : ctx->class_modifiers())
    {
        if (class_mod->SEALED())
        {
            if (sem.is_sealed)
            {
                // report semantic error
            }
            sem.is_sealed = true;
        }
        if (class_mod->STATIC())
        {
            if (sem.is_static)
            {
                // report semantic error
            }
            sem.is_static = true;
        }
    }

    auto model = std::make_shared<class_model>(class_name, decl_line, m_cur_assembly, m_cur_namespace_body, sem);

    if (xlang_model.symbols.set_symbol(symbol, model) == compilation_error::symbol_exists)
    {
        error_manager.write_namespace_member_name_error(decl_line, class_name, m_cur_namespace_body->get_containing_namespace()->get_fully_qualified_id());
        return;
    }
    auto const& class_body = ctx->class_body();
    for (auto const& class_member_declarations : class_body->class_member_declarations())
    {
        for (auto const& class_member : class_member_declarations->class_member_declaration())
        {
            if (class_member->class_method_declaration())
            {
                auto class_method = class_member->class_method_declaration();
                std::string method_id = class_method->IDENTIFIER()->getText();

                std::optional<type_ref> tr = type_ref{ class_method->return_type()->getText() };
                extract_type(class_method->return_type(), tr);

                auto met_model = std::make_shared<method_model>(method_id, class_method->IDENTIFIER()->getSymbol()->getLine(), m_cur_assembly, std::move(tr), method_association::None);
                if (class_method->formal_parameter_list())
                {
                    extract_formal_params(class_method->formal_parameter_list()->fixed_parameter(), met_model);
                }
                model->add_member(met_model);
            }
            if (class_member->class_property_declaration())
            {
                auto const& class_property = class_member->class_property_declaration();
                type_ref tr{ class_property->type()->getText() };
                extract_type(class_property->type(), tr);
                extract_property_accessors(class_property->IDENTIFIER()->getText(), tr, class_property->IDENTIFIER()->getSymbol()->getLine(), class_property->property_accessors(), model);
            }
            if (class_member->class_event_declaration())
            {
                auto const& class_event = class_member->class_event_declaration();
                type_ref tr{ class_event->type()->getText() };
                extract_type(class_event->type(), tr);
                extract_event_saccessors(class_event->IDENTIFIER()->getText(), tr, class_event->IDENTIFIER()->getSymbol()->getLine(), class_event->event_accessors(), model);
            }
        }
    }
    if (ctx->class_base())
    {
        auto const& class_bases = ctx->class_base()->type_base();
        for (auto const& class_base : class_bases)
        {
            model->add_interface_base_ref(class_base->class_type()->getText());
        }
    }
    m_cur_namespace_body->add_class(model);
}

void ast_to_st_listener::enterInterface_declaration(XlangParser::Interface_declarationContext *ctx)
{
    auto id = ctx->IDENTIFIER();
    std::string interface_name{ id->getText() };
    auto decl_line = id->getSymbol()->getLine();

    std::string symbol = m_cur_namespace_body->get_containing_namespace()->get_fully_qualified_id() + "." + interface_name;
    auto model = std::make_shared<interface_model>(interface_name, decl_line, m_cur_assembly, m_cur_namespace_body);

    if (xlang_model.symbols.set_symbol(symbol, model) == compilation_error::symbol_exists)
    {
        error_manager.write_namespace_member_name_error(decl_line, interface_name, m_cur_namespace_body->get_containing_namespace()->get_fully_qualified_id());
        return;
    }
    auto const& interface_body = ctx->interface_body();
    for (auto const& interface_member : interface_body->interface_member_declaration())
    {
        if (interface_member->interface_method_declaration())
        {
            auto interface_method = interface_member->interface_method_declaration();
            std::string method_id = interface_method->IDENTIFIER()->getText();

            std::optional<type_ref> tr = type_ref{ interface_method->return_type()->getText() };
            extract_type(interface_method->return_type(), tr);

            auto met_model = std::make_shared<method_model>(method_id, interface_method->IDENTIFIER()->getSymbol()->getLine(), m_cur_assembly, std::move(tr), method_association::None);
            if (interface_method->formal_parameter_list())
            {
                extract_formal_params(interface_method->formal_parameter_list()->fixed_parameter(), met_model);
            }
            model->add_member(met_model);
        }
        if (interface_member->interface_property_declaration())
        {
            auto const& interface_property = interface_member->interface_property_declaration();
            type_ref tr{ interface_property->type()->getText() };
            extract_type(interface_property->type(), tr);
            extract_property_accessors(interface_property->IDENTIFIER()->getText(), tr, interface_property->IDENTIFIER()->getSymbol()->getLine(), interface_property->property_accessors(), model);
        }
        if (interface_member->interface_event_declaration())
        {
            auto const& interface_event = interface_member->interface_event_declaration();
            type_ref tr{ interface_event->type()->getText() };
            extract_type(interface_event->type(), tr);
            extract_event_saccessors(interface_event->IDENTIFIER()->getText(), tr, interface_event->IDENTIFIER()->getSymbol()->getLine(), interface_event->event_accessors(), model);
        }
    }
    if (ctx->interface_base())
    {
        auto const& interface_bases = ctx->interface_base()->type_base();
        for (auto const& interface_base : interface_bases)
        {
            model->add_interface_base_ref(interface_base->class_type()->getText());
        }
    }
    m_cur_namespace_body->add_interface(model);
}


void ast_to_st_listener::enterDelegate_declaration(XlangParser::Delegate_declarationContext* ctx)
{
    auto id = ctx->IDENTIFIER();
    std::string delegate_name{ id->getText() };
    auto decl_line = id->getSymbol()->getLine();
    std::string symbol = m_cur_namespace_body->get_containing_namespace()->get_fully_qualified_id() + "." + delegate_name;

    std::optional<type_ref> tr = type_ref{ ctx->return_type()->getText() };
    extract_type(ctx->return_type(), tr);
    auto dm = std::make_shared<delegate_model>(delegate_name, decl_line, m_cur_assembly, m_cur_namespace_body, std::move(tr));

    auto formal_params = ctx->formal_parameter_list();
    if (formal_params)
    {
        extract_formal_params(formal_params->fixed_parameter(), dm);
    }
    if (xlang_model.symbols.set_symbol(symbol, dm) == compilation_error::symbol_exists)
    {
        error_manager.write_namespace_member_name_error(decl_line, delegate_name, m_cur_namespace_body->get_containing_namespace()->get_fully_qualified_id());
        return;
    }
    m_cur_namespace_body->add_delegate(dm);
}

void ast_to_st_listener::enterEnum_declaration(XlangParser::Enum_declarationContext *ctx)
{
    auto id = ctx->IDENTIFIER();
    std::string enum_name{ id->getText() };
    auto decl_line = id->getSymbol()->getLine();
    enum_semantics type = enum_semantics::Int32;
    std::string symbol = m_cur_namespace_body->get_containing_namespace()->get_fully_qualified_id() + "." + enum_name;  
    if (ctx->enum_base())
    {
        assert(ctx->enum_base()->enum_integral_type() != nullptr);
        type = str_to_enum_semantics(ctx->enum_base()->enum_integral_type()->getText());
    }

    auto new_enum = std::make_shared<enum_model>(enum_name, decl_line, m_cur_assembly, m_cur_namespace_body, type);

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
    if (xlang_model.symbols.set_symbol(symbol, new_enum) == compilation_error::symbol_exists)
    {
        error_manager.write_namespace_member_name_error(decl_line, enum_name, 
            m_cur_namespace_body->get_containing_namespace()->get_fully_qualified_id());
        return;
    }
    m_cur_namespace_body->add_enum(new_enum);
}

void ast_to_st_listener::enterStruct_declaration(XlangParser::Struct_declarationContext *ctx)
{
    auto id = ctx->IDENTIFIER();
    auto decl_line = id->getSymbol()->getLine();
    std::string struct_name{ id->getText() };
    std::string symbol = m_cur_namespace_body->get_containing_namespace()->get_fully_qualified_id() + "." + struct_name;

    auto new_struct = std::make_shared<struct_model>(struct_name, decl_line, m_cur_assembly, m_cur_namespace_body);

    for (auto field : ctx->struct_body()->field_declaration())
    {
        std::string field_name{ field->IDENTIFIER()->getText() };
        if (new_struct->member_exists(field_name))
        {
            error_manager.write_struct_field_error(field->IDENTIFIER()->getSymbol()->getLine(), field_name, struct_name);
        }
        else
        {
            type_ref tr{ field->type()->getText() };
            extract_type(field->type(), tr);
            new_struct->add_field(std::pair(tr, field_name));
        }
    }
    if (xlang_model.symbols.set_symbol(symbol, new_struct) == compilation_error::symbol_exists)
    {
        error_manager.write_namespace_member_name_error(decl_line, struct_name, m_cur_namespace_body->get_containing_namespace()->get_fully_qualified_id());
        return;
    }
    m_cur_namespace_body->add_struct(new_struct);
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
            push_namespace(token, id->getSymbol()->getLine());
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
        pop_namespace();
    }
}


// Pushes a namespace to the current namespace scope, and adds it to the symbol table if necessary.
void ast_to_st_listener::push_namespace(std::string_view const& name, size_t decl_line)
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
                error_manager.write_namespace_member_name_error(decl_line, name, 
                    m_cur_namespace_body->get_containing_namespace()->get_fully_qualified_id());
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
        auto it1 = std::find_if(xlang_model.namespaces.begin(), xlang_model.namespaces.end(), invalid_ns_id(name));
        if (it1 != xlang_model.namespaces.end())
        {
            // Semantically invalid by check 4 for namespace members.
            error_manager.write_namespace_name_error(decl_line, name, it1->second->get_id());
            return;
        }

        auto it2 = xlang_model.namespaces.find(name);
        if (it2 == xlang_model.namespaces.end())
        {
            auto new_ns = std::make_shared<namespace_model>(name, decl_line, m_cur_assembly, nullptr /* No parent */);
            xlang_model.namespaces[new_ns->get_id()] = new_ns;
        }
        assert(xlang_model.namespaces.find(name) != xlang_model.namespaces.end());
        setup_cur_ns_body(xlang_model.namespaces.at(name));
    }
}

// Pops a namespace from the namespace scope.
void ast_to_st_listener::pop_namespace()
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
