#include <algorithm>
#include <stdexcept>

#include "ast_to_st_listener.h"
#include "symbol_table_helpers.h"
#include "models/class_model.h"
#include "models/delegate_model.h"
#include "models/enum_model.h"
#include "models/event_model.h"
#include "models/interface_model.h"
#include "models/method_model.h"
#include "models/namespace_model.h"
#include "models/property_model.h"
#include "models/struct_model.h"

using namespace xlang::xmeta;
extern bool semantic_error_exists;

std::map<std::string, enum_t> enum_t_antlr_to_xmeta_map
{
    { "Int8", enum_t::int8_enum },
    { "UInt8", enum_t::uint8_enum },
    { "Int16", enum_t::int16_enum },
    { "UInt16", enum_t::uint16_enum },
    { "Int32", enum_t::int32_enum },
    { "UInt32", enum_t::uint32_enum },
    { "Int64", enum_t::int64_enum },
    { "UInt64", enum_t::uint64_enum }
};

simple_type value_type_antlr_to_xmeta(XlangParser::Value_typeContext *vt)
{
    simple_type return_type = simple_type::boolean;
    if (vt->BOOLEAN())
    {
        return_type = simple_type::boolean_type;
    }
    else if (vt->STRING())
    {
        return_type = simple_type::string_type;
    }
    else if (vt->INT16())
    {
        return_type = simple_type::int16_type;
    }
    else if (vt->INT32())
    {
        return_type = simple_type::int32_type;
    }
    else if (vt->INT64())
    {
        return_type = simple_type::int64_type;
    }
    else if (vt->UINT8())
    {
        return_type = simple_type::uint8_type;
    }
    else if (vt->UINT16())
    {
        return_type = simple_type::uint16_type;
    }
    else if (vt->UINT32())
    {
        return_type = simple_type::uint32_type;
    }
    else if (vt->UINT64())
    {
        return_type = simple_type::uint64_type;
    }
    else if (vt->CHAR16())
    {
        return_type = simple_type::char16_type;
    }
    else if (vt->GUID())
    {
        return_type = simple_type::guid_type;
    }
    else if (vt->SINGLE())
    {
        return_type = simple_type::single_type;
    }
    else if (vt->DOUBLE())
    {
        return_type = simple_type::double_type;
    }
    return return_type;
}

xmeta_type type_antlr_to_xmeta(XlangParser::TypeContext *tc)
{
    xmeta_type xt;
    xt.is_void = false;
    if (tc->value_type())
    {
        xt.is_simple_type = true;
        xt.is_array = false;
        xt.simple_type = value_type_antlr_to_xmeta(tc->value_type());
    }
    else if (tc->class_type())
    {
        xt.is_simple_type = false;
        xt.is_array = false;
        xt.type_name = tc->class_type()->type_name()->namespace_or_type_name()->IDENTIFIER()->getText();
    }
    else if (tc->array_type())
    {
        auto value_type = tc->array_type()->non_array_type()->value_type();
        auto class_type = tc->array_type()->non_array_type()->class_type();
        if (value_type)
        {
            xt.is_simple_type = true;
            xt.is_array = true;
            xt.simple_type = value_type_antlr_to_xmeta(value_type);
        }
        else if (class_type)
        {
            xt.is_simple_type = false;
            xt.is_array = true;
            xt.type_name = class_type->type_name()->namespace_or_type_name()->IDENTIFIER()->getText();
        }
    }
    return xt;
}

void extract_type_params(std::vector<antlr4::tree::TerminalNode *> const& ast_type_params, std::vector<std::string_view> & st_type_params)
{
    for (auto ast_type_param : ast_type_params)
    {
        st_type_params.emplace_back(ast_type_param->getText());
    }
}

void extract_formal_params(std::vector<XlangParser::Fixed_parameterContext *> const& ast_formal_params, std::vector<formal_parameter_t> & st_formal_params)
{
    for (auto fixed_param : ast_formal_params)
    {
        formal_parameter_t formal_param;
        formal_param.id = fixed_param->IDENTIFIER()->getText();
        formal_param.type = type_antlr_to_xmeta(fixed_param->type());
        if (fixed_param->parameter_modifier() == nullptr)
        {
            formal_param.modifier = parameter_modifier_t::none;
        }
        else
        {
            if (fixed_param->parameter_modifier()->CONST())
            {
                formal_param.modifier = parameter_modifier_t::const_ref_parameter;
            }
            else if (fixed_param->parameter_modifier()->OUT())
            {
                formal_param.modifier = parameter_modifier_t::out_parameter;
            }
        }
        st_formal_params.emplace_back(formal_param);
    }
}

bool extract_enum_member(XlangParser::Enum_member_declarationContext *ast_enum_member, enum_model & new_enum)
{
    enum_member new_enum_member;
    new_enum_member.id = ast_enum_member->IDENTIFIER()->getText();
    if (new_enum.get_member(new_enum_member.id) != new_enum.members.end())
    {
        write_enum_member_name_error(new_enum.decl_line, new_enum_member.id, new_enum.id);
        return false;
    }
    auto expr = ast_enum_member->enum_expression();
    if (expr)
    {
        std::string str_val;
        if (expr->enum_decimal_integer())
        {
            str_val = expr->enum_decimal_integer()->getText();
            try
            {
                if (new_enum.is_signed())
                {
                    new_enum_member.signed_val = std::stoll(str_val);
                }
                else
                {
                    new_enum_member.unsigned_val = std::stoull(str_val);
                }
            }
            catch (std::out_of_range ex)
            {
                write_enum_const_expr_range_error(new_enum.decl_line, str_val, new_enum.id);
                return false;
            }
            catch (std::invalid_argument)
            {
                write_error(new_enum.decl_line, "Enum constant expression is invalid.");
                return false;
            }
        }
        else if (expr->enum_hexdecimal_integer())
        {
            str_val = expr->enum_hexdecimal_integer()->getText();
            try
            {
                if (!new_enum.is_signed())
                {
                    new_enum_member.unsigned_val = std::stoull(str_val, nullptr, 16);
                }
                else
                {
                    new_enum_member.signed_val = std::stoll(str_val, nullptr, 16);
                }
            }
            catch (std::out_of_range ex)
            {
                write_enum_const_expr_range_error(new_enum.decl_line, str_val, new_enum.id);
            }
            catch (std::invalid_argument)
            {
                write_error(new_enum.decl_line, "Enum constant expression is invalid.");
            }
        }
        else if (expr->enum_identifier())
        {
            new_enum_member.value_id = expr->enum_identifier()->IDENTIFIER()->getText();
        }

        if ((expr->enum_decimal_integer() || expr->enum_hexdecimal_integer()) && (
            (new_enum.is_signed() && !new_enum.within_range(new_enum_member.signed_val)) ||
            (!new_enum.is_signed() && !new_enum.within_range(new_enum_member.unsigned_val))))
        {
            write_enum_const_expr_range_error(new_enum.decl_line, str_val, new_enum.id);
            return false;
        }
    }
    new_enum.members.emplace_back(new_enum_member);
    return true;
}

void ast_to_st_listener::enterClass_declaration(XlangParser::Class_declarationContext *ctx)
{
    auto id = ctx->IDENTIFIER();
    if (id)
    {
        std::string_view class_name{ id->getText() };
        auto decl_line = id->getSymbol()->getLine();
        if (cur_namespace_body->member_id_exists(class_name))
        {
            // Semantically invalid by check 3 for namespace members.
            write_namespace_member_name_error(decl_line, class_name);
            return;
        }

        class_modifier_t mod = class_modifier_t::none;
        if (ctx->class_modifier())
        {
            if (ctx->class_modifier()->UNSEALED())
            {
                mod = class_modifier_t::unsealed_class;
            }
            else if (ctx->class_modifier()->STATIC())
            {
                mod = class_modifier_t::static_class;
            }
        }
        cur_class = std::make_shared<class_model>(class_name, decl_line, mod);
        cur_namespace_body->classes[class_name] = cur_class;
    }
}

void ast_to_st_listener::exitClass_declaration(XlangParser::Class_declarationContext *)
{
    cur_class = nullptr;
}

void ast_to_st_listener::exitDelegate_declaration(XlangParser::Delegate_declarationContext *ctx)
{
    auto id = ctx->IDENTIFIER();
    std::string_view delegate_name{ id->getText() };
    auto decl_line = id->getSymbol()->getLine();

    xmeta_type return_type;
    if (ctx->return_type()->VOID())
    {
        return_type.is_void = true;
    }
    else
    {
        return_type = type_antlr_to_xmeta(ctx->return_type()->type());
    }

    delegate_model dm{ delegate_name, decl_line, return_type };

    auto type_params = ctx->type_parameter_list();
    if (type_params)
    {
        extract_type_params(type_params->IDENTIFIER(), dm.type_parameters);
    }

    auto formal_params = ctx->formal_parameter_list();
    if (formal_params)
    {
        extract_formal_params(formal_params->fixed_parameter(), dm.formal_parameters);
    }

    cur_namespace_body->delegates.emplace_back(dm);
}

void ast_to_st_listener::exitEnum_declaration(XlangParser::Enum_declarationContext *ctx)
{
    auto id = ctx->IDENTIFIER();
    std::string_view enum_name{ id->getText() };
    auto decl_line = id->getSymbol()->getLine();
    enum_t type = enum_t::int32_enum;

    if (ctx->enum_base())
    {
        type = enum_t_antlr_to_xmeta_map[ctx->enum_base()->enum_integral_type()->getText()];
    }

    enum_model new_enum{ enum_name, decl_line, type };

    for (auto field : ctx->enum_body()->enum_member_declaration())
    {
        if (!extract_enum_member(field, new_enum))
        {
            return;
        }
    }

    cur_namespace_body->enums.emplace_back(new_enum);
}

void ast_to_st_listener::exitClass_event_declaration(XlangParser::Class_event_declarationContext *ctx)
{
    auto id = ctx->IDENTIFIER();
    std::string_view event_name{ id->getText() };
    auto decl_line = id->getSymbol()->getLine();
    bool event_accessors_declared = ctx->event_accessors() != nullptr;
    if (cur_namespace_body->member_id_exists(event_name))
    {
        // Semantically invalid by check 3 for namespace members.
        write_namespace_member_name_error(decl_line, event_name);
        return;
    }

    event_modifier_t mod;
    for (auto event_mod : ctx->event_modifier())
    {
        if (event_mod->PROTECTED())
        {
            if (mod.is_protected)
            {
                // Semantically invalid by check 6 for class members.
                write_event_dup_modifier_error(decl_line, "protected", event_name);
            }
            mod.is_protected = true;
        }
        else if (event_mod->STATIC())
        {
            if (mod.is_static)
            {
                // Semantically invalid by check 6 for class members.
                write_event_dup_modifier_error(decl_line, "protected", event_name);
            }
            mod.is_static = true;
        }
    }

    xmeta_type xt = type_antlr_to_xmeta(ctx->type());
    auto em = std::make_shared<event_model>(event_name, decl_line, mod, xt, event_accessors_declared);
    cur_class->members.emplace_back(em);
}

void ast_to_st_listener::enterInterface_declaration(XlangParser::Interface_declarationContext *ctx)
{
    auto id = ctx->IDENTIFIER();
    if (id)
    {
        std::string_view interface_name{ id->getText() };
        auto decl_line = id->getSymbol()->getLine();
        if (cur_namespace_body->member_id_exists(interface_name))
        {
            // Semantically invalid by check 3 for namespace members.
            write_namespace_member_name_error(decl_line, interface_name);
            return;
        }
        cur_interface = std::make_shared<interface_model>(interface_name, decl_line);
        cur_namespace_body->interfaces[interface_name] = cur_interface;
    }
}

void ast_to_st_listener::exitInterface_declaration(XlangParser::Interface_declarationContext *)
{
    cur_interface = nullptr;
}

void ast_to_st_listener::exitClass_method_declaration(XlangParser::Class_method_declarationContext *ctx)
{
    std::string_view method_name{ ctx->IDENTIFIER()->getText() };
    auto decl_line = ctx->IDENTIFIER()->getSymbol()->getLine();

    xmeta_type return_type;
    if (ctx->return_type()->VOID())
    {
        return_type.is_void = true;
        return_type.is_array = false;
        return_type.is_simple_type = false;
    }
    else
    {
        return_type = type_antlr_to_xmeta(ctx->return_type()->type());
    }

    method_modifiers modifiers;
    for (auto mmod : ctx->method_modifier())
    {
        if (mmod->PROTECTED())
        {
            if (modifiers.is_protected)
            {
                // Semantically invalid by check 4 for class members.
                write_class_member_dup_modifier_error(decl_line, "protected", "method", method_name);
                return;
            }
            modifiers.is_protected = true;
        }
        else if (mmod->STATIC())
        {
            if (modifiers.is_static)
            {
                // Semantically invalid by check 4 for class members.
                write_class_member_dup_modifier_error(decl_line, "static", "method", method_name);
                return;
            }
            if (modifiers.is_override)
            {
                write_error(decl_line, "'override' cannot be used with 'static'");
                return;
            }
            modifiers.is_static = true;
        }
        else if (mmod->OVERRIDE())
        {
            if (modifiers.is_override)
            {
                // Semantically invalid by check 4 for class members.
                write_class_member_dup_modifier_error(decl_line, "override", "method", method_name);
                return;
            }
            if (modifiers.is_static)
            {
                write_error(decl_line, "'override' cannot be used with 'static'");
                return;
            }
            modifiers.is_override = true;
        }
    }

    auto mm = std::make_shared<method_model>(method_name, decl_line, return_type, modifiers);

    auto type_params = ctx->type_parameter_list();
    if (type_params)
    {
        extract_type_params(type_params->IDENTIFIER(), mm->type_parameters);
    }

    auto formal_params = ctx->formal_parameter_list();
    if (formal_params)
    {
        extract_formal_params(formal_params->fixed_parameter(), mm->formal_parameters);
    }

    cur_class->members.emplace_back(mm);
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
    std::string_view id_text{ id->getText() };
    size_t num_of_ns = std::count(id_text.begin(), id_text.end(), '.') + 1;
    for (size_t i = 0; i < num_of_ns; ++i)
    {
        pop_namespace();
    }
}

void ast_to_st_listener::exitClass_property_declaration(XlangParser::Class_property_declarationContext *ctx)
{
    auto id = ctx->IDENTIFIER();
    auto decl_line = id->getSymbol()->getLine();
    std::string_view property_name{ id->getText() };
    auto type = type_antlr_to_xmeta(ctx->type());
    bool get_declared = false;
    bool set_declared = false;
    if (ctx->property_accessors())
    {
        get_declared = ctx->property_accessors()->GET() != nullptr;
        set_declared = ctx->property_accessors()->SET() != nullptr;
    }

    property_modifier_t mod;
    for (auto property_mod : ctx->property_modifier())
    {
        if (property_mod->PROTECTED())
        {
            if (mod.is_protected)
            {
                // TODO: ERROR
            }
            mod.is_protected = true;
        }
        else if (property_mod->STATIC())
        {
            if (mod.is_static)
            {
                // TODO: ERROR
            }
            mod.is_static = true;
        }
    }

    auto pm = std::make_shared<property_model>(property_name, decl_line, mod, type, get_declared, set_declared);
    cur_class->members.emplace_back(pm);
}

void ast_to_st_listener::enterStruct_declaration(XlangParser::Struct_declarationContext *ctx)
{
    auto id = ctx->IDENTIFIER();
    if (id)
    {
        std::string_view struct_name{ id->getText() };
        auto decl_line = id->getSymbol()->getLine();
        if (cur_namespace_body->member_id_exists(struct_name))
        {
            // Semantically invalid by check 3 for namespace members.
            write_namespace_member_name_error(decl_line, struct_name);
            return;
        }
        cur_struct = std::make_shared<struct_model>(struct_name, decl_line);
        cur_namespace_body->structs[struct_name] = cur_struct;
    }
}

void ast_to_st_listener::exitStruct_declaration(XlangParser::Struct_declarationContext *)
{
    cur_struct = nullptr;
}

void ast_to_st_listener::exitUsing_alias_directive(XlangParser::Using_alias_directiveContext *ctx)
{
    auto id = ctx->IDENTIFIER();
    std::string_view id_name{ id->getText() };
    auto ns_or_type_name = ctx->namespace_or_type_name()->IDENTIFIER()->getText();
    auto decl_line = id->getSymbol()->getLine();
    if (cur_namespace_body->using_alias_directives.find(id_name) !=
        cur_namespace_body->using_alias_directives.end())
    {
        // Semantically invalid by check 5 for namespace members.
        write_using_alias_directive_name_error(decl_line, id_name);
        return;
    }
    cur_namespace_body->using_alias_directives[id_name] = ns_or_type_name;
}

void ast_to_st_listener::exitUsing_namespace_directive(XlangParser::Using_namespace_directiveContext *ctx)
{
    auto id = ctx->namespace_name()->namespace_or_type_name()->IDENTIFIER();
    std::string_view id_name{ id->getText() };
    if (std::find_if(cur_namespace_body->using_namespace_directives.begin(), cur_namespace_body->using_namespace_directives.end(),
            [&id_name](std::string_view const& s) { return s == id_name; }) !=
        cur_namespace_body->using_namespace_directives.end())
    {
        // Semantically invalid by check 5 for namespace members.
        write_namespace_member_name_error(id->getSymbol()->getLine(), id_name);
        return;
    }
    cur_namespace_body->using_namespace_directives.emplace_back(id_name);
}
