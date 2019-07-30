#include "ast_to_st_listener.h"
#include "xmeta_idl_reader.h"
#include "models/xmeta_models.h"
#include "XlangParser.h"
#include "impl/base.h"

using namespace xlang::xmeta;

namespace
{
    static const std::map<std::string, enum_type> str_to_enum_type_map = {
        { "Int8", enum_type::Int8 },
        { "Int16", enum_type::Int16 },
        { "Int32", enum_type::Int32 },
        { "Int64", enum_type::Int64 },
        { "UInt8", enum_type::UInt8 },
        { "UInt16", enum_type::UInt16 },
        { "UInt32", enum_type::UInt32 },
        { "UInt64", enum_type::UInt64 },
    };

    enum_type str_to_enum_semantics(std::string const& val)
    {
        auto const iter = str_to_enum_type_map.find(val);
        if (iter == str_to_enum_type_map.end())
        {
            xlang::throw_invalid("Can't convert ", val, " to enum simple type");
        }
        return iter->second;
    }

    static const std::map<std::string, xlang::meta::reader::ElementType> str_to_simple_type_map = {
        { "Boolean", xlang::meta::reader::ElementType::Boolean },
        { "String", xlang::meta::reader::ElementType::String },
        { "Int8", xlang::meta::reader::ElementType::I1 },
        { "Int16", xlang::meta::reader::ElementType::I2 },
        { "Int32", xlang::meta::reader::ElementType::I4 },
        { "Int64", xlang::meta::reader::ElementType::I8 },
        { "UInt8", xlang::meta::reader::ElementType::U1 },
        { "UInt16", xlang::meta::reader::ElementType::U2 },
        { "UInt32", xlang::meta::reader::ElementType::U4 },
        { "UInt64", xlang::meta::reader::ElementType::U8 },
        { "Char16", xlang::meta::reader::ElementType::Char },
        { "Single", xlang::meta::reader::ElementType::R4 },
        { "Double", xlang::meta::reader::ElementType::R8 },
    };

    xlang::meta::reader::ElementType str_to_simple_type(std::string const& val)
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
        tr = type_ref{ tc->array_type()->non_array_type()->getText() };
        tr.set_is_array();
        if (tc->array_type()->non_array_type()->value_type())
        {
            tr.set_semantic(str_to_simple_type(tc->array_type()->non_array_type()->getText()));
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
            std::get<std::shared_ptr<delegate_model>>(model)->add_formal_parameter(formal_parameter_model{ formal_param_name, decl_line, xlang_model.m_assembly, sem, std::move(tr) });
        }
        else if (std::holds_alternative<std::shared_ptr<method_model>>(model))
        {
            std::get<std::shared_ptr<method_model>>(model)->add_formal_parameter(formal_parameter_model{ formal_param_name, decl_line, xlang_model.m_assembly, sem, std::move(tr) });
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
             error_manager.report_error(idl_error::DUPLICATE_ENUM_FIELD, decl_line, enum_member_id);
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
             if (!val.is_resolved() && val.get_ref_name() == e_member.get_name())
             {
                 error_manager.report_error(idl_error::CIRCULAR_ENUM_FIELD, e_member.get_decl_line(), e_member.get_name());
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
         error_manager.report_error(idl_error::ENUM_FIELD_OUT_OF_RANGE, decl_line, enum_member_id);
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
    if (dependents.find(e_member.get_name()) != dependents.end())
    {
        error_manager.report_error(idl_error::CIRCULAR_ENUM_FIELD, e_member.get_decl_line(), e_member.get_name());
        return listener_error::failed;
    }
    dependents.emplace(e_member.get_name());
    auto const& ref_name = val.get_ref_name();
    if (!new_enum->member_exists(ref_name))
    {
        error_manager.report_error(idl_error::UNRESOLVED_REFERENCE, e_member.get_decl_line(), ref_name);
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

listener_error ast_to_st_listener::extract_property_accessors(std::shared_ptr<property_model> const& prop_model,
    XlangParser::Property_accessorsContext* property_accessors,
    std::shared_ptr<class_or_interface_model> const& model)
{
    std::shared_ptr<property_model> check = model->get_property_by_name(prop_model->get_name());
    if (check != nullptr)
    {
        if (!(check->get_type() == prop_model->get_type()))
        {
            error_manager.report_error(idl_error::DUPLICATE_TYPE_MEMBER_ID, prop_model->get_decl_line(), prop_model->get_name());
            return listener_error::failed;
        }
    }

    std::shared_ptr<method_model> get_method = nullptr;
    std::shared_ptr<method_model> set_method = nullptr;
    type_ref tr = prop_model->get_type();

    method_modifier property_method_modifier;
    if (prop_model->get_modifier().is_static)
    {
        property_method_modifier.is_static = true;
    }


    if (property_accessors->property_accessor_method().size() > 0) // Explicit declaration
    {
        auto const& property_accessor_methods = property_accessors->property_accessor_method();
        if (property_accessor_methods.size() == 2)
        {
            // Can't have two of the same
            if ((property_accessor_methods[0]->GET() && property_accessor_methods[1]->GET())
                || (property_accessor_methods[0]->SET() && property_accessor_methods[1]->SET()))
            {
                error_manager.report_error(idl_error::DUPLICATE_PROPERTY_ACCESSOR, prop_model->get_decl_line(), prop_model->get_name());
                return listener_error::failed;
            }
        }
        else if (property_accessor_methods.size() > 2)
        {
            // Can't have more than two
            error_manager.report_error(idl_error::INVALID_PROPERTY_ACCESSOR, prop_model->get_decl_line(), prop_model->get_name());
            return listener_error::failed;
        }

        listener_error error = listener_error::passed;
        for (auto const& property_accessor : property_accessor_methods)
        {
            if (property_accessor->GET())
            {
                get_method = std::make_shared<method_model>("get_" + prop_model->get_name(), property_accessor->GET()->getSymbol()->getLine(), xlang_model.m_assembly, std::move(prop_model->get_type()), property_method_modifier, method_association::Property);
                if (get_method && model->add_member(get_method) == semantic_error::symbol_exists)
                {
                    error_manager.report_error(idl_error::INVALID_OR_DUPLICATE_PROPERTY_ACCESSOR, prop_model->get_decl_line(), prop_model->get_name());
                    error = listener_error::failed;
                }
            }
            else if (property_accessor->SET())
            {
                set_method = std::make_shared<method_model>("put_" + prop_model->get_name(), property_accessor->SET()->getSymbol()->getLine(), xlang_model.m_assembly, std::move(std::nullopt), property_method_modifier, method_association::Property);
                parameter_semantics sem = parameter_semantics::in;
                set_method->add_formal_parameter(formal_parameter_model{ "value", prop_model->get_decl_line(), xlang_model.m_assembly, sem, std::move(tr) });
                if (set_method && model->add_member(set_method) == semantic_error::symbol_exists)
                {
                    error_manager.report_error(idl_error::INVALID_OR_DUPLICATE_PROPERTY_ACCESSOR, prop_model->get_decl_line(), prop_model->get_name());
                    error = listener_error::failed;
                }
            }
        }
        if (error == listener_error::failed)
        {
            return error;
        }
    }
    else // Implicit declaration
    {
        get_method = std::make_shared<method_model>("get_" + prop_model->get_name(), prop_model->get_decl_line(), xlang_model.m_assembly, std::move(prop_model->get_type()), property_method_modifier, method_association::Property);

        set_method = std::make_shared<method_model>("put_" + prop_model->get_name(), prop_model->get_decl_line(), xlang_model.m_assembly, std::move(std::nullopt), property_method_modifier, method_association::Property);
        parameter_semantics sem = parameter_semantics::in;
        set_method->add_formal_parameter(formal_parameter_model{ "value", prop_model->get_decl_line(), xlang_model.m_assembly, sem, std::move(tr) });

        // Adding property methods to the model
        listener_error error = listener_error::passed;
        if (get_method && model->add_member(get_method) == semantic_error::symbol_exists)
        {
            error_manager.report_error(idl_error::INVALID_OR_DUPLICATE_PROPERTY_ACCESSOR, prop_model->get_decl_line(), prop_model->get_name());
            error = listener_error::failed;
        }
        if (set_method && model->add_member(set_method) == semantic_error::symbol_exists)
        {
            error_manager.report_error(idl_error::INVALID_OR_DUPLICATE_PROPERTY_ACCESSOR, prop_model->get_decl_line(), prop_model->get_name());
            error =  listener_error::failed;
        }
        if (error == listener_error::failed)
        {
            return error;
        }
    }
    
    // This enables declaration of the property's get and set on two different lines. 
    if (model->property_exists(prop_model))
    {
        auto const& existing_property = model->get_property_member(prop_model->get_name());

        // Check that the type is the same
        if (existing_property->get_type().get_semantic().get_ref_name() != prop_model->get_type().get_semantic().get_ref_name())
        {
            error_manager.report_error(idl_error::DUPLICATE_PROPERTY, prop_model->get_decl_line(), prop_model->get_name());
            return listener_error::failed;
        }

        // Set the get property if not declared
        if (existing_property->set_get_method(get_method) == semantic_error::accessor_exists)
        {
            error_manager.report_error(idl_error::DUPLICATE_PROPERTY_ACCESSOR, prop_model->get_decl_line(), prop_model->get_name());
            return listener_error::failed;
        }
        else
        {
            prop_model->set_get_method(get_method);
        }

        // Set the set property if not declared
        if (existing_property->set_set_method(set_method) == semantic_error::accessor_exists)
        {
            error_manager.report_error(idl_error::DUPLICATE_PROPERTY_ACCESSOR, prop_model->get_decl_line(), prop_model->get_name());
            return listener_error::failed;
        }
        else
        {
            prop_model->set_set_method(set_method);
        }
    }
    else
    {
        prop_model->set_get_method(get_method);
        prop_model->set_set_method(set_method);

        if (model->add_member(prop_model) == semantic_error::symbol_exists)
        {
            error_manager.report_error(idl_error::DUPLICATE_TYPE_MEMBER_ID, prop_model->get_decl_line(), prop_model->get_name());
            return listener_error::failed;
        }
    }

    return listener_error::passed;
}

listener_error ast_to_st_listener::extract_event_accessors(std::shared_ptr<event_model> const& event,
    std::shared_ptr<class_or_interface_model> const& model)
{
    if (model->member_exists(event->get_name()))
    {
        error_manager.report_error(idl_error::DUPLICATE_TYPE_MEMBER_ID, event->get_decl_line(), event->get_name());
        return listener_error::failed; 
    }
    
    type_ref event_registration{ "Foundation.EventRegistrationToken" };
    parameter_semantics param_sem = parameter_semantics::in;
    type_ref tr = event->get_type();

    method_modifier event_method_modifier;
    if (event->get_modifier().is_static)
    {
        event_method_modifier.is_static = true;
    }

    std::shared_ptr<method_model> add_method = std::make_shared<method_model>("add_" + event->get_name(), event->get_decl_line(), xlang_model.m_assembly, std::move(event_registration), event_method_modifier, method_association::Event);
    add_method->add_formal_parameter(formal_parameter_model{ "value", event->get_decl_line(), xlang_model.m_assembly, param_sem, std::move(tr) });

    std::shared_ptr<method_model> remove_method = std::make_shared<method_model>("remove_" + event->get_name(), event->get_decl_line(), xlang_model.m_assembly, std::move(std::nullopt), event_method_modifier, method_association::Event);
    remove_method->add_formal_parameter(formal_parameter_model{ "value", event->get_decl_line(), xlang_model.m_assembly, param_sem, std::move(event_registration) });

    assert(event->set_add_method(add_method) == semantic_error::passed);
    assert(event->set_remove_method(remove_method) == semantic_error::passed);

    listener_error error = listener_error::passed;
    if (model->add_member(add_method) == semantic_error::symbol_exists)
    {
        error_manager.report_error(idl_error::CONFLICTING_EVENT_ACCESSOR_METHODS, event->get_decl_line(), event->get_name());
        error = listener_error::failed;
    }
    if (model->add_member(remove_method) == semantic_error::symbol_exists)
    {
        error_manager.report_error(idl_error::CONFLICTING_EVENT_ACCESSOR_METHODS, event->get_decl_line(), event->get_name());
        error = listener_error::failed;
    }
    if (error == listener_error::failed)
    {
        return error;
    }
    if (model->add_member(event) == semantic_error::symbol_exists)
    {
        error_manager.report_error(idl_error::DUPLICATE_TYPE_MEMBER_ID, event->get_decl_line(), event->get_name());
        return listener_error::failed;
    }
    return listener_error::passed;
}

void ast_to_st_listener::enterClass_declaration(XlangParser::Class_declarationContext * ctx)
{
    auto id = ctx->IDENTIFIER();
    std::string class_name{ id->getText() };
    auto decl_line = id->getSymbol()->getLine();

    std::string symbol = m_cur_namespace_body->get_containing_namespace()->get_qualified_name() + "." + class_name;
    class_modifier class_sem;
    for (auto const& class_mod : ctx->class_modifiers())
    {
        if (class_mod->SEALED())
        {
            if (class_sem.is_sealed)
            {
                // report semantic error
            }
            class_sem.is_sealed = true;
        }
        if (class_mod->STATIC())
        {
            if (class_sem.is_static)
            {
                // report semantic error
            }
            class_sem.is_static = true;
        }
    }

    auto clss_model = std::make_shared<class_model>(class_name, decl_line, xlang_model.m_assembly, m_cur_namespace_body, class_sem);

    auto synthesized_interface = std::make_shared<interface_model>("I" + class_name, decl_line, xlang_model.m_assembly, m_cur_namespace_body, true);
    bool has_instance = false;

    auto synthesized_interface_factory = std::make_shared<interface_model>("I" + class_name + "Factory", decl_line, xlang_model.m_assembly, m_cur_namespace_body, true);
    bool has_factory = false;
    
    auto synthesized_interface_statics = std::make_shared<interface_model>("I" + class_name + "Statics", decl_line, xlang_model.m_assembly, m_cur_namespace_body, true);
    bool has_statics = false;

    
    // TODO: protected, override and composable are coming in a later update
    //auto synthesized_interface_protected = std::make_shared<interface_model>("I" + class_name + "Protected", decl_line, xlang_model.m_assembly, m_cur_namespace_body);
    //auto synthesized_interface_overrides = std::make_shared<interface_model>("I" + class_name + "Overrides", decl_line, xlang_model.m_assembly, m_cur_namespace_body);

    if (xlang_model.symbols.set_symbol(symbol, clss_model) == semantic_error::symbol_exists)
    {
        error_manager.report_error(idl_error::DUPLICATE_NAMESPACE_MEMBER, decl_line, class_name);
        return;
    }

    std::string create_instance = "CreateInstance";
    size_t create_instance_index = 1;
    auto const& class_body = ctx->class_body();
    for (auto const& class_member_declarations : class_body->class_member_declarations())
    {
        for (auto const& class_member : class_member_declarations->class_member_declaration())
        {
            if (class_member->class_constructor_declaration())
            {
                auto class_constructor = class_member->class_constructor_declaration();
                std::string constructor_id = class_constructor->IDENTIFIER()->getText();

                if (constructor_id != class_name)
                {
                    // TODO: semantic check that constructor name must be the same as class name.
                }

                method_modifier method_sem;
                if (class_constructor->class_constructor_modifier() && class_constructor->class_constructor_modifier()->PROTECTED())
                {
                    method_sem.is_protected = true;
                }

                auto constexpr constructor_semantic_name = ".ctor";
                auto constructor_model = std::make_shared<method_model>(constructor_semantic_name, class_constructor->IDENTIFIER()->getSymbol()->getLine(), xlang_model.m_assembly, std::move(std::nullopt), method_sem, method_association::Constructor);
                if (class_constructor->formal_parameter_list())
                {
                    extract_formal_params(class_constructor->formal_parameter_list()->fixed_parameter(), constructor_model);
                }

                if (class_constructor->formal_parameter_list() && class_constructor->formal_parameter_list()->fixed_parameter().size() > 0) // Non default constructor
                {
                    std::string factory_name;
                    // Synthesizing constructors
                    if (create_instance_index != 1)
                    {
                        factory_name = create_instance + std::to_string(create_instance_index);
                    }
                    else
                    {
                        factory_name = create_instance;
                    }
                    type_ref tr = type_ref{ class_name };
                    auto syn_constructor_model = std::make_shared<method_model>(factory_name, class_constructor->IDENTIFIER()->getSymbol()->getLine(), xlang_model.m_assembly, std::move(tr), method_sem, method_association::Constructor);
                    if (class_constructor->formal_parameter_list())
                    {
                        extract_formal_params(class_constructor->formal_parameter_list()->fixed_parameter(), syn_constructor_model);
                    }
                    synthesized_interface_factory->add_member(syn_constructor_model);
                    has_factory = true;
                    create_instance_index++;
                }
                else
                {
                    auto syn_constructor_model = std::make_shared<method_model>(constructor_semantic_name, class_constructor->IDENTIFIER()->getSymbol()->getLine(), xlang_model.m_assembly, std::move(std::nullopt), method_sem, method_association::Constructor);
                    if (class_constructor->formal_parameter_list())
                    {
                        extract_formal_params(class_constructor->formal_parameter_list()->fixed_parameter(), syn_constructor_model);
                    }
                    synthesized_interface->add_member(syn_constructor_model);
                    has_instance = true;
                }

                /* TODO: We don't need to update add any members to the class model, the class model can completely be void of methods 
                properties and events and the metadata emit can user the interfaces that are referenced in the class
                 to emit metadata */
                if (clss_model->add_member(constructor_model) == semantic_error::symbol_exists)
                {
                    error_manager.report_error(idl_error::DUPLICATE_TYPE_MEMBER_ID, constructor_model->get_decl_line(), constructor_model->get_name());
                }
            }
            if (class_member->class_method_declaration())
            {
                auto class_method = class_member->class_method_declaration();
                std::string method_id = class_method->IDENTIFIER()->getText();

                std::optional<type_ref> tr = type_ref{ class_method->return_type()->getText() };
                extract_type(class_method->return_type(), tr);

                // TODO: semantic checks
                method_modifier method_sem;
                for (auto const& method_modifier : class_method->method_modifier())
                {
                    if (method_modifier->OVERRIDABLE() || method_modifier->OVERRIDE())
                    {
                        if (method_sem.is_protected)
                        {
                            // REPORT semantic error
                        }
                        method_sem.is_overridable = true;
                    }
                    if (method_modifier->PROTECTED())
                    {
                        if (method_sem.is_protected)
                        {
                            // REPORT semantic error
                        }
                        method_sem.is_protected = true;
                    }
                    if (method_modifier->STATIC())
                    {
                        if (method_modifier->PROTECTED() || method_modifier->OVERRIDABLE() || method_modifier->OVERRIDE())
                        {
                            // REPORT static methods cannot be overrridable and protected
                        }
                        if (method_sem.is_static)
                        {
                            // REPORT semantic error
                        }
                        method_sem.is_static = true;
                    }
                }

                if (class_sem.is_static && !method_sem.is_static)
                {
                    // REPORT static class must only have static methods
                    error_manager.report_error(idl_error::STATIC_MEMBER_ONLY, class_method->IDENTIFIER()->getSymbol()->getLine(), method_id);
                }

                auto met_model = std::make_shared<method_model>(method_id, class_method->IDENTIFIER()->getSymbol()->getLine(), xlang_model.m_assembly, std::move(tr), method_sem, method_association::None);
                if (class_method->formal_parameter_list())
                {
                    extract_formal_params(class_method->formal_parameter_list()->fixed_parameter(), met_model);
                }

                auto syn_met_model = std::make_shared<method_model>(method_id, class_method->IDENTIFIER()->getSymbol()->getLine(), xlang_model.m_assembly, std::move(tr), method_sem, method_association::None);
                if (class_method->formal_parameter_list())
                {
                    extract_formal_params(class_method->formal_parameter_list()->fixed_parameter(), syn_met_model);
                }

                if (method_sem.is_static)
                {
                    synthesized_interface_statics->add_member(syn_met_model);
                    has_statics = true;
                }
                else
                {
                    synthesized_interface->add_member(syn_met_model);
                    has_instance = true;
                }

                /* TODO: We don't need to update add any members to the class model, the class model can completely be void of methods
                    properties and events and the metadata emit can user the interfaces that are referenced in the class
                     to emit metadata */
                if (clss_model->add_member(met_model) == semantic_error::symbol_exists)
                {
                    error_manager.report_error(idl_error::CANNOT_OVERLOAD_METHOD, met_model->get_decl_line(), met_model->get_name());
                }
            }
            if (class_member->class_property_declaration())
            {
                auto const& class_property = class_member->class_property_declaration();
                type_ref tr{ class_property->type()->getText() };
                extract_type(class_property->type(), tr);

                // TODO: semantic checks
                property_modifier property_sem;
                for (auto const& property_modifier : class_property->property_modifier())
                {
                    if (property_modifier->PROTECTED())
                    {
                        if (property_sem.is_protected)
                        {
                            // REPORT semantic error
                        }
                        property_sem.is_protected = true;
                    }
                    if (property_modifier->OVERRIDABLE())
                    {
                        // TODO: figure out what to do with overridable
                    }
                    if (property_modifier->STATIC())
                    {
                        if (property_modifier->PROTECTED() || property_modifier->OVERRIDABLE())
                        {
                            // REPORT static methods cannot be overrridable and protected
                        }
                        if (property_sem.is_static)
                        {
                            // REPORT semantic error
                        }
                        property_sem.is_static = true;
                    }
                }

                auto prop_model = std::make_shared<property_model>(class_property->IDENTIFIER()->getText(), class_property->IDENTIFIER()->getSymbol()->getLine(), xlang_model.m_assembly, property_sem, std::move(tr));

                if (class_sem.is_static && !property_sem.is_static)
                {
                    // REPORT static class must only have static methods
                    error_manager.report_error(idl_error::STATIC_MEMBER_ONLY, prop_model->get_decl_line(), prop_model->get_name());
                }

                /* TODO: We don't need to update add any members to the class model, the class model can completely be void of methods
                    properties and events and the metadata emit can user the interfaces that are referenced in the class
                     to emit metadata */
                if (extract_property_accessors(prop_model, class_property->property_accessors(), clss_model) == listener_error::passed)
                {
                    auto syn_prop_model = std::make_shared<property_model>(class_property->IDENTIFIER()->getText(), class_property->IDENTIFIER()->getSymbol()->getLine(), xlang_model.m_assembly, property_sem, std::move(tr));
                    if (property_sem.is_static)
                    {
                        extract_property_accessors(syn_prop_model, class_property->property_accessors(), synthesized_interface_statics);
                        has_statics = true;
                    }
                    else
                    {
                        extract_property_accessors(syn_prop_model, class_property->property_accessors(), synthesized_interface);
                        has_instance = true;
                    }
                }
            }
            if (class_member->class_event_declaration())
            {
                auto const& class_event = class_member->class_event_declaration();
                type_ref tr{ class_event->type()->getText() };
                extract_type(class_event->type(), tr);

                // TODO: semantic checks
                event_modifier event_sem;
                for (auto const& event_modifier : class_event->event_modifier())
                {
                    if (event_modifier->PROTECTED())
                    {
                        if (event_sem.is_protected)
                        {
                            // report semantic error
                        }
                        event_sem.is_protected = true;
                    }
                    if (event_modifier->STATIC())
                    {
                        if (event_sem.is_static)
                        {
                            // report semantic error
                        }
                        event_sem.is_static = true;
                    }
                }

                auto event = std::make_shared<event_model>(class_event->IDENTIFIER()->getText(), class_event->IDENTIFIER()->getSymbol()->getLine(), xlang_model.m_assembly, event_sem, std::move(tr));
                
                if (class_sem.is_static && !event_sem.is_static)
                {
                    // REPORT static class must only have static methods
                    error_manager.report_error(idl_error::STATIC_MEMBER_ONLY, event->get_decl_line(), event->get_name());
                }
                
                /* TODO: We don't need to update add any members to the class model, the class model can completely be void of methods
                    properties and events and the metadata emit can user the interfaces that are referenced in the class
                     to emit metadata */
                if (extract_event_accessors(event, clss_model) == listener_error::passed)
                {
                    auto syn_event_model = std::make_shared<event_model>(class_event->IDENTIFIER()->getText(), class_event->IDENTIFIER()->getSymbol()->getLine(), xlang_model.m_assembly, event_sem, std::move(tr));
                    if (event_sem.is_static)
                    {
                        extract_event_accessors(syn_event_model, synthesized_interface_statics);
                        has_statics = true;
                    }
                    else
                    {
                        extract_event_accessors(syn_event_model, synthesized_interface);
                        has_instance = true;
                    }
                }
            }
        }
    }
    if (ctx->class_base())
    {
        auto const& cls_base = ctx->class_base();
        if (cls_base->interface_base())
        {
            auto const& interface_basese = cls_base->interface_base()->type_base();
            for (auto const& interface_base : interface_basese)
            {
                clss_model->add_interface_base_ref(interface_base->class_type()->getText());
            }
        }
        if (cls_base->type_base())
        {
            clss_model->add_class_base_ref(cls_base->type_base()->class_type()->getText());
        }
    }
    if (has_instance)
    {
        clss_model->add_instance_interface_ref(synthesized_interface);
        m_cur_namespace_body->add_interface(synthesized_interface);
    }
    if (has_statics)
    {
        clss_model->add_static_interface_ref(synthesized_interface_statics);
        m_cur_namespace_body->add_interface(synthesized_interface_statics);
    }
    if (has_factory)
    {
        clss_model->add_factory_interface_ref(synthesized_interface_factory);
        m_cur_namespace_body->add_interface(synthesized_interface_factory);
    }

    m_cur_namespace_body->add_class(clss_model);
}

void ast_to_st_listener::enterInterface_declaration(XlangParser::Interface_declarationContext *ctx)
{
    auto id = ctx->IDENTIFIER();
    std::string interface_name{ id->getText() };
    auto decl_line = id->getSymbol()->getLine();

    std::string symbol = m_cur_namespace_body->get_containing_namespace()->get_qualified_name() + "." + interface_name;
    auto model = std::make_shared<interface_model>(interface_name, decl_line, xlang_model.m_assembly, m_cur_namespace_body);

    if (xlang_model.symbols.set_symbol(symbol, model) == semantic_error::symbol_exists)
    {
        error_manager.report_error(idl_error::DUPLICATE_NAMESPACE_MEMBER, decl_line, interface_name);
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

            auto met_model = std::make_shared<method_model>(method_id, interface_method->IDENTIFIER()->getSymbol()->getLine(), xlang_model.m_assembly, std::move(tr), method_association::None);
            if (interface_method->formal_parameter_list())
            {
                extract_formal_params(interface_method->formal_parameter_list()->fixed_parameter(), met_model);
            }
            if (model->add_member(met_model) == semantic_error::symbol_exists)
            {
                 error_manager.report_error(idl_error::CANNOT_OVERLOAD_METHOD, met_model->get_decl_line(), met_model->get_name());
            }
        }
        if (interface_member->interface_property_declaration())
        {
            auto const& interface_property = interface_member->interface_property_declaration();
            type_ref tr{ interface_property->type()->getText() };
            extract_type(interface_property->type(), tr);
            auto prop_model = std::make_shared<property_model>(interface_property->IDENTIFIER()->getText(), interface_property->IDENTIFIER()->getSymbol()->getLine(), xlang_model.m_assembly, xlang::xmeta::property_modifier(), std::move(tr));

            extract_property_accessors(prop_model, interface_property->property_accessors(), model);
        }
        if (interface_member->interface_event_declaration())
        {
            auto const& interface_event = interface_member->interface_event_declaration();
            type_ref tr{ interface_event->type()->getText() };
            extract_type(interface_event->type(), tr);

            auto event = std::make_shared<event_model>(interface_event->IDENTIFIER()->getText(), interface_event->IDENTIFIER()->getSymbol()->getLine(), xlang_model.m_assembly, xlang::xmeta::event_modifier(), std::move(tr));
            extract_event_accessors(event, model);
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
    std::string symbol = m_cur_namespace_body->get_containing_namespace()->get_qualified_name() + "." + delegate_name;

    std::optional<type_ref> tr = type_ref{ ctx->return_type()->getText() };
    extract_type(ctx->return_type(), tr);
    auto dm = std::make_shared<delegate_model>(delegate_name, decl_line, xlang_model.m_assembly, m_cur_namespace_body, std::move(tr));

    auto formal_params = ctx->formal_parameter_list();
    if (formal_params)
    {
        extract_formal_params(formal_params->fixed_parameter(), dm);
    }
    if (xlang_model.symbols.set_symbol(symbol, dm) == semantic_error::symbol_exists)
    {
        error_manager.report_error(idl_error::DUPLICATE_NAMESPACE_MEMBER, decl_line, delegate_name);
        return;
    }
    m_cur_namespace_body->add_delegate(dm);
}

void ast_to_st_listener::enterEnum_declaration(XlangParser::Enum_declarationContext *ctx)
{
    auto id = ctx->IDENTIFIER();
    std::string enum_name{ id->getText() };
    auto decl_line = id->getSymbol()->getLine();
    enum_type type = enum_type::Int32;
    std::string symbol = m_cur_namespace_body->get_containing_namespace()->get_qualified_name() + "." + enum_name;  
    if (ctx->enum_base())
    {
        assert(ctx->enum_base()->enum_integral_type() != nullptr);
        type = str_to_enum_semantics(ctx->enum_base()->enum_integral_type()->getText());
    }

    auto new_enum = std::make_shared<enum_model>(enum_name, decl_line, xlang_model.m_assembly, m_cur_namespace_body, type);

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
    if (xlang_model.symbols.set_symbol(symbol, new_enum) == semantic_error::symbol_exists)
    {
        error_manager.report_error(idl_error::DUPLICATE_NAMESPACE_MEMBER, decl_line, enum_name);
        return;
    }

    if (ctx->attributes())
    {
        XlangParser::AttributesContext * attrs = ctx->attributes();
        for (XlangParser::Attribute_sectionContext * attr_section : attrs->attribute_section())
        {
            for (XlangParser::AttributeContext * attr : attr_section->attribute_list()->attribute())
            {

                std::string attribute_name = attr->type_name()->getText();
                XlangParser::Attribute_argumentsContext * attribute_argument = attr->attribute_arguments();

                std::vector<attribute_member> positoned_parameter;
                std::map<std::string, attribute_member> named_parameter;

                for (XlangParser::Positional_argumentContext * positional_arg : attribute_argument->positional_argument_list()->positional_argument())
                {
                    positoned_parameter.emplace_back(attribute_member{ positional_arg->getText() });
                }
                for (XlangParser::Named_argumentContext * named_arg : attribute_argument->named_argument_list()->named_argument())
                {
                    named_parameter.emplace(named_arg->IDENTIFIER()->getText(), attribute_member{ named_arg->expression()->getText() });
                }
                std::shared_ptr<attribute_model> attr_model = attribute_model::create_attribute(xlang_model.attributes.get_attribute_type(attribute_name), positoned_parameter, named_parameter);
                new_enum->add_attribute(attr_model);
            }
        }
    }
    m_cur_namespace_body->add_enum(new_enum);
}

void ast_to_st_listener::enterStruct_declaration(XlangParser::Struct_declarationContext *ctx)
{
    auto id = ctx->IDENTIFIER();
    auto decl_line = id->getSymbol()->getLine();
    std::string struct_name{ id->getText() };
    std::string symbol = m_cur_namespace_body->get_containing_namespace()->get_qualified_name() + "." + struct_name;

    auto new_struct = std::make_shared<struct_model>(struct_name, decl_line, xlang_model.m_assembly, m_cur_namespace_body);

    for (auto field : ctx->struct_body()->field_declaration())
    {
        std::string field_name{ field->IDENTIFIER()->getText() };
        if (new_struct->member_exists(field_name))
        {
            error_manager.report_error(idl_error::DUPLICATE_FIELD_ID, field->IDENTIFIER()->getSymbol()->getLine(), field_name);
        }
        else
        {
            type_ref tr{ field->type()->getText() };
            extract_type(field->type(), tr);
            new_struct->add_field(std::pair(tr, field_name));
        }
    }
    if (xlang_model.symbols.set_symbol(symbol, new_struct) == semantic_error::symbol_exists)
    {
        error_manager.report_error(idl_error::DUPLICATE_NAMESPACE_MEMBER, decl_line, struct_name);
        return;
    }
    m_cur_namespace_body->add_struct(new_struct);
}

void ast_to_st_listener::enterAttribute_declaration(XlangParser::Attribute_declarationContext * ctx)
{ 
    auto id = ctx->IDENTIFIER();
    auto decl_line = id->getSymbol()->getLine();
    std::string attribute_name{ id->getText() };
    std::string symbol = m_cur_namespace_body->get_containing_namespace()->get_qualified_name() + "." + attribute_name;

    auto new_attribute = std::make_shared<attribute_type_model>(attribute_name, decl_line, xlang_model.m_assembly);
    if (ctx->attribute_body()->attribute_constructor_declaration() && ctx->attribute_body()->attribute_constructor_declaration()->attribute_parameter_list())
    {
        for (auto const& pos_parameter : ctx->attribute_body()->attribute_constructor_declaration()->attribute_parameter_list()->positional_parameter())
        {
            type_ref tr{ pos_parameter->type()->getText() };
            extract_type(pos_parameter->type(), tr);
            new_attribute->add_positonal_parameter(std::move(tr));
        }
    }

    for (auto const& field : ctx->attribute_body()->field_declaration())
    {
        std::string field_name{ field->IDENTIFIER()->getText() };

        type_ref tr{ field->type()->getText() };
        extract_type(field->type(), tr);
        new_attribute->add_named_parameter(field_name, std::move(tr));
    }

    if (xlang_model.attributes.define_attribute_type(attribute_name, new_attribute) == semantic_error::symbol_exists)
    {
        error_manager.report_error(idl_error::DUPLICATE_ATTRIBUTE, decl_line, attribute_name);
    }

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
        assert(child_ns->get_name() == name);
        m_cur_namespace_body = std::make_shared<namespace_body_model>(child_ns);
        child_ns->add_namespace_body(m_cur_namespace_body);
    };

    if (m_cur_namespace_body != nullptr)
    {
        auto const& cur_ns = m_cur_namespace_body->get_containing_namespace();
        if (cur_ns->member_exists(name))
        {
            if (cur_ns->child_namespace_exists(name))
            {
                setup_cur_ns_body(cur_ns->get_child_namespaces().at(name));
            }
            else
            {
                error_manager.report_error(idl_error::INVALID_NAMESPACE_NAME, decl_line, name);
                return;
            }
        }
        else
        {
            if (!cur_ns->child_namespace_exists(name))
            {
                cur_ns->add_child_namespace(std::make_shared<namespace_model>(name, decl_line, xlang_model.m_assembly, cur_ns));
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
            error_manager.report_error(idl_error::INVALID_NAMESPACE_NAME, decl_line, name);
            return;
        }

        auto it2 = xlang_model.namespaces.find(name);
        if (it2 == xlang_model.namespaces.end())
        {
            auto new_ns = std::make_shared<namespace_model>(name, decl_line, xlang_model.m_assembly, nullptr /* No parent */);
            xlang_model.namespaces[new_ns->get_name()] = new_ns;
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