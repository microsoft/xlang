#include <algorithm>
#include <stdexcept>

#include "ast_to_st_listener.h"
#include "xmeta_idl_reader.h"
#include "models/xmeta_models.h"
#include "XlangParser.h"

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

void extract_property_accessors(XlangParser::Property_accessorsContext *ctx, bool& get_declared, bool& set_declared, bool& get_declared_first)
{
    if (!ctx)
    {
        get_declared = true;
        set_declared = true;
        get_declared_first = true;
        return;
    }

    if (ctx->get_set_property_accessors())
    {
        get_declared = true;
        set_declared = true;
        get_declared_first = true;
    }
    else if (ctx->set_get_property_accessors())
    {
        get_declared = true;
        set_declared = true;
        get_declared_first = false;
    }
    else if (ctx->get_property_accessor())
    {
        get_declared = ctx->get_property_accessor() != nullptr;
        set_declared = false;
        get_declared_first = true;
    }
    else if (ctx->set_property_accessor())
    {
        get_declared_first = false;
        set_declared = ctx->set_property_accessor() != nullptr;
        get_declared_first = false;
    }
}

listener_error ast_to_st_listener::extract_type(XlangParser::TypeContext *tc, type_ref& tr)
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

void ast_to_st_listener::extract_to_existing_property(std::shared_ptr<property_model> const& prop, bool get_declared, bool set_declared, size_t decl_line, std::string_view const& container_name)
{
    if (prop->get_get_method() && get_declared)
    {
        m_reader.write_property_duplicate_get_error(decl_line, container_name, prop->get_id());
        return;
    }
    if (prop->get_set_method() && set_declared)
    {
        m_reader.write_property_duplicate_set_error(decl_line, container_name, prop->get_id());
        return;
    }
    if (get_declared)
    {
        prop->create_get_method();
    }
    if (set_declared)
    {
        prop->create_set_method();
    }
}

listener_error ast_to_st_listener::extract_type(XlangParser::Return_typeContext *rtc, std::optional<type_ref>& tr)
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

void ast_to_st_listener::extract_formal_params(std::vector<XlangParser::Fixed_parameterContext *> const& ast_formal_params, std::shared_ptr<delegate_model> const& dm)
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
        dm->add_formal_parameter(formal_parameter_model{ formal_param_name, decl_line, m_reader.m_cur_assembly, sem, std::move(tr) });
    }
}

template<class T>
listener_error ast_to_st_listener::extract_property_declaration(XlangParser::Property_declarationContext *ctx, std::shared_ptr<T> const& model)
{
    assert(ctx->property_identifier());
    assert(model != nullptr);

    auto id = ctx->property_identifier()->IDENTIFIER();
    auto decl_line = id->getSymbol()->getLine();
    std::string prop_name{ id->getText() };
    assert(ctx->type() != nullptr);
    type_ref tr{ ctx->type()->getText() };
    extract_type(ctx->type(), tr);
    bool is_array = ctx->type()->array_type() != nullptr;
    bool get_declared = false;
    bool set_declared = false;
    bool get_declared_first = true;
    extract_property_accessors(ctx->property_accessors(), get_declared, set_declared, get_declared_first);

    auto it = get_it(model->get_properties(), prop_name);
    if (it != model->get_properties().end())
    {
        std::shared_ptr<property_model> const& prop = *it;
        extract_to_existing_property(prop, get_declared, set_declared, decl_line, model->get_id());
        return listener_error::passed;
    }

    property_semantics sem;
    if (extract_property_semantic(ctx->property_modifier(), sem, decl_line, prop_name) == listener_error::failed)
    {
        return listener_error::failed;
    }

    auto pm = std::make_shared<property_model>(
        prop_name,
        decl_line,
        m_reader.get_cur_assembly(),
        std::move(tr),
        is_array,
        std::move(sem),
        get_declared,
        set_declared,
        get_declared_first);

    model->add_member(pm);

    return listener_error::passed;
}

listener_error ast_to_st_listener::extract_property_semantic(std::vector<XlangParser::Property_modifierContext *> mods, property_semantics& sem, size_t decl_line, std::string_view const& id)
{
    for (auto property_mod : mods)
    {
        if (property_mod->PROTECTED())
        {
            if (sem.is_protected)
            {
                m_reader.write_duplicate_modifier_error(decl_line, "protected", id);
                return listener_error::failed;
            }
            sem.is_protected = true;
        }
        else if (property_mod->STATIC())
        {
            if (sem.is_static)
            {
                m_reader.write_duplicate_modifier_error(decl_line, "protected", id);
                return listener_error::failed;
            }
            sem.is_static = true;
        }
    }
    return listener_error::passed;
}


void ast_to_st_listener::enterClass_declaration(XlangParser::Class_declarationContext *ctx)
{
    auto id = ctx->IDENTIFIER();
    if (id)
    {
        std::string class_name{ id->getText() };
        std::string class_base{ "" };
        auto decl_line = id->getSymbol()->getLine();
        if (m_reader.get_cur_namespace_body()->type_id_exists(class_name))
        {
            // Semantically invalid by check 3 for namespace members.
            m_reader.write_namespace_member_name_error(decl_line, class_name);
            return;
        }

        class_semantics sem = class_semantics::sealed_instance_class;
        for (auto const& mod : ctx->class_modifier())
        {
            if (mod->UNSEALED())
            {
                sem = class_semantics::unsealed_class;
            }
            else if (mod->STATIC())
            {
                sem = class_semantics::static_class;
            }
        }
        m_reader.m_cur_class = std::make_shared<class_model>(
            class_name,
            decl_line,
            m_reader.get_cur_assembly(),
            m_reader.get_cur_namespace_body(),
            sem,
            ctx->class_base()
            ? ctx->class_base()->getText()
            : "");
        m_reader.m_cur_namespace_body->add_class(m_reader.m_cur_class);
    }
}

void ast_to_st_listener::exitClass_declaration(XlangParser::Class_declarationContext *ctx)
{
    m_reader.m_cur_class = nullptr;
}

void ast_to_st_listener::exitClass_property_declaration(XlangParser::Class_property_declarationContext *ctx)
{
    assert(ctx->property_declaration());
    extract_property_declaration(ctx->property_declaration(), m_reader.m_cur_class);
}

void ast_to_st_listener::exitDelegate_declaration(XlangParser::Delegate_declarationContext *ctx)
{
    auto id = ctx->IDENTIFIER();
    std::string delegate_name{ id->getText() };
    auto decl_line = id->getSymbol()->getLine();
    std::optional<type_ref> tr = type_ref{ ctx->return_type()->getText() };
    extract_type(ctx->return_type(), tr);
    auto dm = std::make_shared<delegate_model>(delegate_name, decl_line, m_reader.get_cur_assembly(), m_reader.get_cur_namespace_body(), std::move(tr));

    // TODO: Type params

    auto formal_params = ctx->formal_parameter_list();
    if (formal_params)
    {
        extract_formal_params(formal_params->fixed_parameter(), dm);
    }

    m_reader.m_cur_namespace_body->add_delegate(dm);
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

void ast_to_st_listener::enterInterface_declaration(XlangParser::Interface_declarationContext *ctx)
{
    auto id = ctx->IDENTIFIER();
    if (id)
    {
        std::string interface_name{ id->getText() };
        auto decl_line = id->getSymbol()->getLine();
        if (m_reader.m_cur_namespace_body->get_containing_namespace()->member_id_exists(interface_name))
        {
            // Semantically invalid by check 3 for namespace members.
            m_reader.write_namespace_member_name_error(decl_line, interface_name);
            return;
        }
        m_reader.m_cur_interface = std::make_shared<interface_model>(interface_name, decl_line, m_reader.get_cur_assembly(), m_reader.get_cur_namespace_body());
        m_reader.m_cur_namespace_body->add_interface(m_reader.m_cur_interface);
    }
}

void ast_to_st_listener::exitInterface_declaration(XlangParser::Interface_declarationContext *ctx)
{
    m_reader.m_cur_interface = nullptr;
}

void ast_to_st_listener::exitInterface_property_declaration(XlangParser::Interface_property_declarationContext *ctx)
{
    assert(ctx->property_declaration());
    extract_property_declaration(ctx->property_declaration(), m_reader.m_cur_interface);
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
