#pragma once

namespace xlang
{
    void write_import_type(writer& w, TypeDef const& type)
    {
        if (is_exclusive_to(type))
        {
            return;
        }

        w.write("@ = _internal.@\n", type.TypeName(), type.TypeName());
    }

    void write_include(writer& w, std::string_view const& ns)
    {
        if (w.current_namespace != ns)
        {
            auto format = R"(#if __has_include("py.%.h")
#include "py.%.h"
#endif
)";
            w.write(format, ns, ns);
        }
        else
        {
            w.write("#include \"py.%.h\"\n", ns);
        }
    }

    void write_ns_init_function_name(writer& w, std::string_view const& ns)
    {
        w.write("initialize_%", get_impl_name(ns));
    }

    void write_param_name(writer& w, method_signature::param_t param)
    {
        w.write("param%", param.first.Sequence() - 1);
    }

    void write_pinterface_type_args(writer& w, GenericParam const& param)
    {
        w.write("typename %", param.Name());
    }

    void write_pinterface_type_arg_name(writer& w, GenericParam const& param)
    {
        w.write(param.Name());
    }

    void write_full_type(writer& w, TypeDef const& type)
    {
        w.write(type);

        if (is_ptype(type))
        {
            w.write("<%>", bind_list<write_pinterface_type_arg_name>(", ", type.GenericParam()));
        }
    }

    void write_py_category(writer& w, TypeDef const& type, std::string_view const& category)
    {
        w.write("    template <%> struct category<%> { using type = %; };\n", 
            bind_list<write_pinterface_type_args>(", ", type.GenericParam()),            
            bind<write_full_type>(type), 
            category);
    }

    void write_py_class_category(writer& w, TypeDef const& type)
    {
        write_py_category(w, type, "class_category");
    }

    void write_py_enum_category(writer& w, TypeDef const& type)
    {
        write_py_category(w, type, "enum_category");
    }

    void write_py_interface_category(writer& w, TypeDef const& type)
    {
        if (is_ptype(type))
        {
            write_py_category(w, type, "pinterface_category");
        }
        else
        {
            write_py_category(w, type, "interface_category");
        }
    }

    void write_setup_filenames(writer& w, std::string_view const& module_name, std::vector<std::string> const& namespaces)
    {
        XLANG_ASSERT(namespaces.size() > 0);

        for (auto&& ns : namespaces)
        {
            w.write("'%/src/py.%.cpp', ", settings.module, ns);
        }

        w.write("'%/src/%.cpp'", settings.module, module_name);
    }

    void write_winrt_wrapper(writer& w, TypeDef const& type)
    {
        if (is_ptype(type))
        {
            w.write("py::winrt_pinterface_wrapper<py@>", type.TypeName());
        }
        else if (get_category(type) == category::struct_type)
        {
            w.write("py::winrt_struct_wrapper<%>", type);
        }
        else
        {
            w.write("py::winrt_wrapper<%>", type);
        }
    }

    void write_getset_table(writer& w, TypeDef const& type)
    {
        XLANG_ASSERT(get_category(type) == category::struct_type);

        w.write("\nstatic PyGetSetDef @_getset[] = {\n", type.TypeName());

        for (auto&& field : type.FieldList())
        {
            // TODO: remove const_cast once pywinrt is updated to target Python 3.7. 
            //       pywinrt currently targeting 3.6 because that's the version that ships with VS 2017 v15.8
            //       https://github.com/python/cpython/commit/007d7ff73f4e6e65cfafd512f9c23f7b7119b803
            w.write("    { const_cast<char*>(\"%\"), (getter)@_get_%, (setter)@_set_%, nullptr, nullptr },\n",
                field.Name(),
                type.TypeName(), field.Name(),
                type.TypeName(), field.Name());
        }

        w.write("    { nullptr }\n};\n");
    }

    void write_method_table_flags(writer& w, std::vector<method_info> const& methods)
    {
        XLANG_ASSERT(methods.size() > 0);

        if (methods.size() == 1)
        {
            auto method = methods[0].method;

            if (is_get_method(method))
            {
                w.write("METH_NOARGS");
            }
            else if (is_put_method(method) || is_add_method(method) || is_remove_method(method))
            {
                w.write("METH_O");
            }
            else
            {
                w.write("METH_VARARGS");
            }

            if (method.Flags().Static())
            {
                w.write(" | METH_STATIC");
            }
        }
        else
        {
            w.write("METH_VARARGS");

            if (methods[0].method.Flags().Static())
            {
                w.write(" | METH_STATIC");
            }
        }
    }

    void write_method_table(writer& w, TypeDef const& type)
    {
        w.write("\nstatic PyMethodDef @_methods[] = {\n", type.TypeName());

        for (auto&&[name, overloads] : get_methods(type))
        {
            w.write("    { \"%\", (PyCFunction)@_%, %, nullptr },\n",
                name, type.TypeName(), name, bind<write_method_table_flags>(overloads));
        }

        if (!(is_ptype(type) || is_static_class(type)))
        {
            w.write("    { \"_from\", (PyCFunction)@__from, METH_O | METH_STATIC, nullptr },\n", type.TypeName());
        }

        w.write("    { nullptr }\n};\n");
    }

    void write_type_slot_table(writer& w, TypeDef const& type)
    {
        auto category = get_category(type);

        XLANG_ASSERT((category == category::class_type)
            || (category == category::interface_type)
            || (category == category::struct_type));

        w.write("\nstatic PyType_Slot @_Type_slots[] = \n{\n", type.TypeName());

        if (has_dealloc(type))
        {
            w.write("    { Py_tp_dealloc, @_dealloc },\n", type.TypeName());
        }

        w.write("    { Py_tp_new, @_new },\n", type.TypeName());

        if ((category == category::class_type) || (category == category::interface_type))
        {
            w.write("    { Py_tp_methods, @_methods },\n", type.TypeName());
        }

        if (category == category::struct_type)
        {
            w.write("    { Py_tp_getset, @_getset },\n", type.TypeName());
        }

        w.write("    { 0, nullptr },\n};\n");
    }

    void write_type_spec_size(writer& w, TypeDef const& type)
    {
        auto category = get_category(type);

        XLANG_ASSERT((category == category::class_type)
            || (category == category::interface_type)
            || (category == category::struct_type));

        if (is_static_class(type))
        {
            w.write("0");
        }
        else
        {
            w.write("sizeof(%)", bind<write_winrt_wrapper>(type));
        }
    }

    void write_type_spec(writer& w, TypeDef const& type)
    {
        auto format = R"(
static PyType_Spec @_Type_spec =
{
    "@",
    %,
    0,
    Py_TPFLAGS_DEFAULT,
    @_Type_slots
};
)";
        w.write(format, type.TypeName(), type.TypeName(), bind<write_type_spec_size>(type), type.TypeName());
    }

    struct out_param_init_writer : public signature_handler_base<out_param_init_writer>
    {
        using signature_handler_base<out_param_init_writer>::handle;

        writer& w;

        out_param_init_writer(writer& wr) : w(wr)
        {
        }

        void handle_class(TypeDef const& /*type*/) { w.write("nullptr"); }
        void handle_delegate(TypeDef const& /*type*/) { w.write("nullptr"); }
        void handle_interface(TypeDef const& /*type*/) { w.write("nullptr"); }
        void handle_struct(TypeDef const& /*type*/) { /* no init needed */ }

        void handle(ElementType /*type*/) { /* no init needed */ }
        void handle(GenericTypeInstSig const& /*type*/) { w.write("nullptr"); }
    };

    void write_out_param_init(writer& w, method_signature::param_t const& param)
    {
        // WinRT class, interface and delegate out params must be initialized as nullptr
        out_param_init_writer writer{ w };
        writer.handle(param.second->Type());
    }

    void write_param_declaration(writer& w, MethodDef const& method, method_signature::param_t const& param)
    {
        auto sequence = param.first.Sequence() - 1;

        switch (get_param_category(param))
        {
        case param_category::in:
            // if method is special (i.e. get/put/add/remove) but not RTSpecial (i.e. ctor)
            // treat the args value as a single value, not as a tuple
            if (method.SpecialName() && !method.Flags().RTSpecialName()) 
            {
                w.write("            auto param% = py::converter<%>::convert_to(args);\n", sequence, param.second->Type());
            }
            else
            {
                w.write("            auto param% = py::convert_to<%>(args, %);\n", sequence, param.second->Type(), sequence);
            }
            break;
        case param_category::out:
            w.write("            % param% { % };\n", param.second->Type(), sequence, bind<write_out_param_init>(param));
            break;
        case param_category::pass_array:
            w.write("            /*p*/ winrt::array_view<% const> param% { }; // TODO: Convert incoming python parameter\n", param.second->Type(), sequence);
            break;
        case param_category::fill_array:
            w.write("            /*f*/ winrt::array_view<%> param% { }; // TODO: Convert incoming python parameter\n", param.second->Type(), sequence);
            break;
        case param_category::receive_array:
            w.write("            /*r*/ winrt::com_array<%> param% { };\n", param.second->Type(), sequence);
            break;
        default:
            throw_invalid("write_param_conversion not impl");
        }
    }

    void write_method_overload_return(writer& w, method_signature const& signature)
    {
        if (signature.return_signature())
        {
            w.write("% return_value = ", signature.return_signature().Type());
        }
    }










    void write_method_overload_invoke_context(writer& w, TypeDef const& type, MethodDef const& method)
    {
        if (is_ptype(type))
        {
            w.write("obj.");
        }
        else if (method.Flags().Static())
        {
            w.write("%::", method.Parent());
        }
        else
        {
            w.write("self->obj.");
        }
    }

    auto get_cpp_method_name(MethodDef const& method)
    {
        auto name = method.Name();

        if (method.SpecialName())
        {
            return name.substr(name.find('_') + 1);
        }

        return name;
    }

    void write_method_overload_body(writer& w, TypeDef const& type, method_info const& info, method_signature signature)
    {
        auto guard{ w.push_generic_params(info.type_arguments) };

        if (get_param_category(signature.return_signature()) == param_category::receive_array)
        {
            w.write(R"(        // returning a ReceiveArray not impl
        return nullptr;
)");
            return;
        }

        w.write("        try\n        {\n");
        for (auto&& param : signature.params())
        {
            write_param_declaration(w, info.method, param);
        }

        if (signature.has_params())
        {
            w.write("\n");
        }

        w.write("            %%%(%);\n",
            bind<write_method_overload_return>(signature),
            bind<write_method_overload_invoke_context>(type, info.method),
            get_cpp_method_name(info.method),
            bind_list<write_param_name>(", ", signature.params()));

        if (signature.return_signature())
        {
            if (count_out_param(signature.params()) == 0)
            {
                auto format = R"(
            return py::convert(return_value);
)";
                w.write(format);
            }
            else
            {
                {
                    auto format = R"(
            PyObject* out_return_value = py::convert(return_value);
            if (!out_return_value) 
            { 
                return nullptr;
            };

)";
                    w.write(format);
                }

                int out_param_count = 1;
                std::string tuple_pack_param;

                for (auto&& param : signature.params())
                {
                    if (is_in_param(param))
                    {
                        continue;
                    }

                    out_param_count++;
                    auto sequence = param.first.Sequence() - 1;
                    tuple_pack_param.append(", ");
                    tuple_pack_param.append(w.write_temp("out%", sequence));

                    auto format = R"(            PyObject* out% = py::convert(param%);
            if (!out%) 
            {
                return nullptr;
            }

)";
                    w.write(format, sequence, sequence, sequence);
                }

                w.write("            return PyTuple_Pack(%, out_return_value%);\n", out_param_count, tuple_pack_param);
            }
        }
        else
        {
            w.write("            Py_RETURN_NONE;\n");
        }

        w.write(R"(        }
        catch (...)
        {
            return py::to_PyErr();
        }
)");
    }

    template <auto F>
    void write_method_overload(writer& w, TypeDef const& type, std::vector<method_info> const& overloads)
    {
        F(w, type, overloads[0]);

        if (overloads.size() == 1 && overloads[0].method.SpecialName())
        {
            auto overload = overloads[0];

            if (is_get_method(overloads[0].method))
            {
                w.write(R"(    if (args != nullptr)
    {
        PyErr_SetString(PyExc_TypeError, "arguments not supported for get methods");
        return nullptr;
    }
)");
            }

            method_signature signature{ overloads[0].method };
            write_method_overload_body(w, type, overloads[0], signature);
        }
        else
        {
            w.write("    Py_ssize_t arg_count = PyTuple_Size(args);\n\n");

            bool first{ true };
            for (auto&& overload : overloads)
            {
                method_signature signature{ overload.method };

                w.write("    ");

                if (first)
                {
                    first = false;
                }
                else
                {
                    w.write("else ");
                }

                auto format = R"(if (arg_count == %)
    {
)";
                w.write(format, count_in_param(signature.params()));
                write_method_overload_body(w, type, overload, signature);
                w.write("    }\n");
            }

            w.write(R"(    else if (arg_count == -1)
    {
        return nullptr; 
    }

    PyErr_SetString(PyExc_TypeError, "Invalid parameter count");
    return nullptr;
)");
        }

        w.write("}\n");
    }

    template <auto F>
    void write_methods(writer& w, TypeDef const& type)
    {
        for (auto&& [name, overloads]: get_methods(type))
        {   
            write_method_overload<F>(w, type, overloads);
        }
    }

    void write_type_query_interface(writer& w, TypeDef const& type)
    {
        if (is_ptype(type) || is_static_class(type))
        {
            return;
        }

        auto format = R"(
static PyObject* %__from(PyObject* /*unused*/, PyObject* arg)
{
    try
    {
        auto instance = py::converter<winrt::Windows::Foundation::IInspectable>::convert_to(arg);
        return py::convert(instance.as<%>());
    }
    catch (...)
    {
        return py::to_PyErr();
    }
}
)";

        w.write(format, type.TypeName(), type);
    }
    
    void write_type_method_decl_self_type(writer& w, TypeDef const& type, MethodDef const& method)
    {
        if (method.Flags().Static())
        {
            w.write("PyObject* /*unused*/");
        }
        else
        {
            w.write("%* self", bind<write_winrt_wrapper>(type));
        }
    }

    void write_type_function_decl(writer& w, TypeDef const& type, method_info const& info)
    {
        w.write("\nstatic PyObject* @_%(%, PyObject* args)\n{ \n",
            type.TypeName(),
            info.method.Name(),
            bind<write_type_method_decl_self_type>(type, info.method));
    }

    void write_type_functions(writer& w, TypeDef const& type)
    {
        if (is_ptype(type))
        {
            for (auto&&[name, overloads] : get_methods(type))
            {
                write_type_function_decl(w, type, overloads[0]);
                w.write("    return self->obj->%(args);\n}\n", name);
            }
        }
        else
        {
            write_methods<write_type_function_decl>(w, type);
        }
    }

    void write_winrt_type_specialization_native_type(writer& w, TypeDef const& type)
    {
        if (is_ptype(type))
        {
            w.write("py@", type.TypeName());
        }
        else
        {
            w.write("%", type);
        }
    }

    void write_winrt_type_specialization(writer& w, TypeDef const& type)
    {
        if (is_exclusive_to(type))
        {
            return;
        }

        auto format = R"(    template<>
    struct winrt_type<%>
    {
        static PyTypeObject* python_type;

        static PyTypeObject* get_python_type()
        {
            return python_type;
        }
    };

)";
        w.write(format, bind<write_winrt_type_specialization_native_type>(type));
    }

    void write_winrt_type_specialization_storage(writer& w, TypeDef const& type)
    {
        w.write("PyTypeObject* py::winrt_type<%>::python_type;\n", bind<write_winrt_type_specialization_native_type>(type));
    }

    void write_class_constructor_overload(writer& w, MethodDef const& method, method_signature const& signature)
    {
        w.write("        try\n        {\n");
        for (auto&& param : signature.params())
        {
            write_param_declaration(w, method, param);
        }

        auto format = R"(            % instance{ % };
            return py::wrap(instance, type);
        }
        catch (...)
        {
            return py::to_PyErr();
        }
)";
        w.write(format, method.Parent(), bind_list<write_param_name>(", ", signature.params()));
    }


    void write_class_constructor(writer& w, TypeDef const& type)
    {
        w.write("\nstatic PyObject* %_new(PyTypeObject* type, PyObject* args, PyObject* kwds)\n{\n", type.TypeName());

        auto constructors = get_constructors(type);

        if (is_static_class(type) || constructors.size() == 0)
        {
            auto format = R"(    PyErr_SetString(PyExc_TypeError, "% is not activatable");
    return nullptr;
)";
            w.write(format, type.TypeName());
        }
        else
        {
            w.write(R"(    if (kwds != nullptr)
    {
        PyErr_SetString(PyExc_TypeError, "keyword arguments not supported");
        return nullptr;
    }

)");
            w.write("    Py_ssize_t arg_count = PyTuple_Size(args);\n\n");

            bool first{ true };
            for (auto&& m : constructors)
            {
                method_signature signature{ m };

                w.write("    ");

                if (first)
                {
                    first = false;
                }
                else
                {
                    w.write("else ");
                }

                auto format = R"(if (arg_count == %)
    {
)";
                w.write(format, count_in_param(signature.params()));
                write_class_constructor_overload(w, m, signature);
                w.write("    }\n");
            }

            w.write(R"(    else if (arg_count == -1)
    {
        return nullptr; 
    }

    PyErr_SetString(PyExc_TypeError, "Invalid parameter count");
    return nullptr;
)");
        }

        w.write("}\n");
    }

    void write_class_dealloc(writer& w, TypeDef const& type)
    {
        XLANG_ASSERT(!is_ptype(type));

        if (!has_dealloc(type))
        {
            return;
        }
        
        auto format = R"(
static void @_dealloc(%* self)
{
    auto hash_value = std::hash<winrt::Windows::Foundation::IInspectable>{}(self->obj);
    py::wrapped_instance(hash_value, nullptr);
    self->obj = nullptr;
}
)";
        w.write(format, type.TypeName(), bind<write_winrt_wrapper>(type));
    }

    void write_class(writer& w, TypeDef const& type)
    {
        auto guard{ w.push_generic_params(type.GenericParam()) };

        w.write("\n// ----- % class --------------------\n", type.TypeName());
        write_winrt_type_specialization_storage(w, type);
        write_class_constructor(w, type);
        write_class_dealloc(w, type);
        write_type_query_interface(w, type);
        write_type_functions(w, type);
        write_method_table(w, type);
        write_type_slot_table(w, type);
        write_type_spec(w, type);
    }

















    void write_pinterface_decl(writer& w, TypeDef const& type)
    {
        if (!is_ptype(type))
            return;

        auto guard{ w.push_generic_params(type.GenericParam()) };

        w.write("\nstruct py@\n{\n", type.TypeName());
        w.write("    virtual ~py@() {};\n", type.TypeName());
        w.write("    virtual winrt::Windows::Foundation::IUnknown const& get_unknown() = 0;\n");
        w.write("    virtual std::size_t hash() = 0;\n\n");
            
        for (auto&& [method_name, overloads] : get_methods(type))
        {
            w.write("    virtual PyObject* %(PyObject* args) = 0;\n", method_name);
        }

        w.write("};\n");
    }

    void write_pinterface_method_decl(writer& w, TypeDef const&, method_info const& info)
    {
        w.write("\nPyObject* %(PyObject* args) override\n{\n", info.method.Name());
    }

    void write_pinterface_impl(writer& w, TypeDef const& type)
    {
        if (!is_ptype(type))
            return;

        auto guard{ w.push_generic_params(type.GenericParam()) };

        w.write("\ntemplate<%>\nstruct py@Impl : public py@\n{\n", bind_list<write_pinterface_type_args>(", ", type.GenericParam()), type.TypeName(), type.TypeName());

        w.write("py@Impl(%<%> o) : obj(o) {}\n", type.TypeName(), type, bind_list<write_pinterface_type_arg_name>(", ", type.GenericParam()));
        w.write("winrt::Windows::Foundation::IUnknown const& get_unknown() override { return obj; }\n");
        w.write("std::size_t hash() override { return py::get_instance_hash(obj); }\n");

        write_methods<write_pinterface_method_decl>(w, type);

        w.write("\n    %<%> obj{ nullptr };\n};\n", type, bind_list<write_pinterface_type_arg_name>(", ", type.GenericParam()));
    }

    void write_pinterface_type_mapper(writer& w, TypeDef const& type)
    {
        if (!is_ptype(type))
            return;

        auto format = R"(    template <%>
    struct pinterface_python_type<%<%>>
    {
        using abstract = ::py@;
        using concrete = ::py@Impl<%>;
    };

)";
        w.write(format, 
            bind_list<write_pinterface_type_args>(", ", type.GenericParam()),
            type, 
            bind_list<write_pinterface_type_arg_name>(", ", type.GenericParam()),
            type.TypeName(),
            type.TypeName(),
            bind_list<write_pinterface_type_arg_name>(", ", type.GenericParam()));
    }

    void write_interface_constructor(writer& w, TypeDef const& type)
    {
        XLANG_ASSERT(get_category(type) == category::interface_type);

        auto format = R"(
PyObject* @_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    PyErr_SetString(PyExc_TypeError, "@ interface is not activatable");
    return nullptr;
}
)";
        w.write(format, type.TypeName(), type.TypeName());
    }

    void write_interface_dealloc(writer& w, TypeDef const& type)
    {
        XLANG_ASSERT(get_category(type) == category::interface_type);

        auto format = R"(
static void @_dealloc(%* self)
{
    auto hash_value = %;
    py::wrapped_instance(hash_value, nullptr);
    self->obj%;
}
)";
        w.write(format, type.TypeName(), bind<write_winrt_wrapper>(type), 
            is_ptype(type)
            ? "self->obj->hash()"
            : "std::hash<winrt::Windows::Foundation::IInspectable>{}(self->obj)",
            is_ptype(type) ? ".release()" : " = nullptr");
    }

    void write_interface_pytypeobject(writer& w, TypeDef const& type)
    {
        if (is_ptype(type))
        {
            w.write("py@", type.TypeName());
        }
        else
        {
            w.write("%", type);
        }
    }

    void write_interface(writer& w, TypeDef const& type)
    {
        if (is_exclusive_to(type))
        {
            return;
        }

        auto guard{ w.push_generic_params(type.GenericParam()) };

        w.write("\n// ----- @ interface --------------------\n", type.TypeName());
        write_winrt_type_specialization_storage(w, type);
        write_interface_constructor(w, type);
        write_interface_dealloc(w, type);
        write_type_query_interface(w, type);
        write_type_functions(w, type);
        write_method_table(w, type);
        write_type_slot_table(w, type);
        write_type_spec(w, type);
    }








    void write_delegate_param(writer& w, Param const& p)
    {
        w.write("auto param%", p.Sequence());
    }

    void write_delegate(writer& w, TypeDef const& type)
    {
        auto guard{ w.push_generic_params(type.GenericParam()) };

        auto invoke = get_delegate_invoke(type);

        w.write("\n");

        if (is_ptype(type))
        {
            w.write("template <%>\n", bind_list<write_pinterface_type_args>(", ", type.GenericParam()));
        }

        auto format = R"(struct py@
{
    static % get(PyObject* callable)
    {
        if (PyFunction_Check(callable) == 0)
        {
            throw winrt::hresult_invalid_argument();
        }

        // TODO: How do I manage callable lifetime here?
        return [callable](%)
        {
)";
        w.write(format, 
            type.TypeName(),
            bind<write_full_type>(type),
            bind_list<write_delegate_param>(", ", invoke.ParamList()));

        for (auto&& p : invoke.ParamList())
        {
            w.write("            PyObject* pyObj% = py::convert(param%);\n", p.Sequence(), p.Sequence());
        }

        w.write("\n            PyObject* args = PyTuple_Pack(%", distance(invoke.ParamList()));
        for (auto&& p : invoke.ParamList())
        {
            w.write(", pyObj%", p.Sequence());
        }
        w.write(R"();

            // TODO: RAII for GILState
            PyGILState_STATE gstate;
            gstate = PyGILState_Ensure();
            PyObject_CallObject(callable, args);
            PyGILState_Release(gstate);
        };
    }
};
)");
    }

    void write_python_delegate_type(writer& w, TypeDef const& type)
    {
        w.write("::py@", type.TypeName());

        if (is_ptype(type))
        {
            w.write("<%>", bind_list<write_pinterface_type_arg_name>(", ", type.GenericParam()));
        }
    }

    void write_delegate_type_mapper(writer& w, TypeDef const& type)
    {
        auto format = R"(    template <%>
    struct delegate_python_type<%>
    {
        using type = %;
    };

)";
        w.write(format,
            bind_list<write_pinterface_type_args>(", ", type.GenericParam()),
            bind<write_full_type>(type),
            bind<write_python_delegate_type>(type));
    }









    std::string get_cpp_field_name(Field const& field)
    {
        auto type = field.Parent();
        std::string name{ field.Name() };
        
        static const std::set<std::string_view> custom_numerics = { "Matrix3x2", "Matrix4x4", "Plane", "Quaternion", "Vector2", "Vector3", "Vector4" };

        if ((type.TypeNamespace() == "Windows.Foundation.Numerics") && (custom_numerics.find(type.TypeName()) != custom_numerics.end()))
        {
            std::transform(name.begin(), name.end(), name.begin(), [](char c) {return static_cast<char>(::tolower(c)); });
        }

        return std::move(name);
    }

    void write_struct_converter_decl(writer& w, TypeDef const& type)
    {
        w.write_indented("template<>\nstruct converter<%>\n{\n", type);
        {
            writer::indent_guard g{ w };
            w.write_indented("static PyObject* convert(% instance) noexcept;\n", type);
            w.write_indented("static % convert_to(PyObject* obj);\n", type);
        }

        w.write_indented("};\n\n");
    }

    static const std::map<std::string_view, std::string_view> custom_foundation_convert_to = {
        { "DateTime", "new_value = winrt::Windows::Foundation::DateTime{ winrt::Windows::Foundation::TimeSpan { converter<int64_t>::convert_to(pyUniversalTime) } };\n" },
        { "EventRegistrationToken", "new_value.value = converter<int64_t>::convert_to(pyValue);\n" },
        { "HResult", "new_value = converter<int32_t>::convert_to(pyValue);\n" },
        { "TimeSpan", "new_value = winrt::Windows::Foundation::TimeSpan { converter<int64_t>::convert_to(pyDuration) };\n" }
    };

    void write_struct_convert_functions(writer& w, TypeDef const& type)
    {
        w.write_indented("\nPyObject* py::converter<%>::convert(% instance) noexcept\n{\n", type, type);
        {
            writer::indent_guard g{ w };
            w.write_indented("return py::wrap_struct<%>(instance, py::get_python_type<%>());\n", type, type);
        }
        w.write_indented("}\n\n");

        w.write_indented("% py::converter<%>::convert_to(PyObject* obj)\n{\n", type, type);
        {
            writer::indent_guard g{ w };
            w.write_indented("throw_if_pyobj_null(obj);\n");
            w.write_indented(R"(if (Py_TYPE(obj) == py::get_python_type<%>())
{
    return reinterpret_cast<py::winrt_struct_wrapper<%>*>(obj)->obj;
}

)", type, type);

            w.write_indented(R"(if (!PyDict_Check(obj))
{
    throw winrt::hresult_invalid_argument();
}

)");

            w.write_indented("% new_value{};\n", type);

            for (auto&& field : type.FieldList())
            {
                auto field_name = get_cpp_field_name(field);

                w.write_indented("PyObject* py% = PyDict_GetItemString(obj, \"%\");\n", field_name, field.Name());
                w.write_indented("if (!py%) { throw winrt::hresult_invalid_argument(); }\n", field_name);

                if (type.TypeNamespace() == "Windows.Foundation")
                {
                    auto convert_to = custom_foundation_convert_to.find(type.TypeName());
                    if (convert_to != custom_foundation_convert_to.end())
                    {
                        w.write_indented(convert_to->second);
                        continue;
                    }
                }

                w.write_indented("new_value.% = converter<%>::convert_to(py%);\n",
                    field_name,
                    field.Signature().Type(),
                    field_name);
            }

            w.write_indented("return new_value;\n");
        }
        w.write_indented("}\n\n");
    }


    struct struct_ctor_var_type_writer : public signature_handler_base<struct_ctor_var_type_writer>
    {
        using signature_handler_base<struct_ctor_var_type_writer>::handle;

        writer& w;

        struct_ctor_var_type_writer(writer& wr) : w(wr)
        {
        }

        void handle_struct(TypeDef const& /*type*/)
        {
            w.write("PyObject*");
        }

        void handle(ElementType type)
        {
            w.write(type);
        }
    };

    void write_struct_field_var_type(writer& w, Field const& field)
    {
        struct_ctor_var_type_writer writer{ w };
        writer.handle(field.Signature().Type());
    }

    struct struct_ctor_format_writer : public signature_handler_base<struct_ctor_format_writer>
    {
        using signature_handler_base<struct_ctor_format_writer>::handle;

        writer& w;

        struct_ctor_format_writer(writer& wr) : w(wr)
        {
        }

        void handle_struct(TypeDef const& /*type*/) 
        { 
            w.write("O");
        }

        void handle(ElementType type)
        {
            switch (type)
            {
            case ElementType::Boolean:
                w.write("p");
                break;
            // TODO: char, sbyte and byte struct support 
            //case ElementType::Char:
            //    w.write("wchar_t");
            //    break;
            //case ElementType::I1:
            //    w.write("int8_t");
            //    break;
            //case ElementType::U1:
            //    w.write("uint8_t");
            //    break;
            case ElementType::I2:
                w.write("h");
                break;
            case ElementType::U2:
                w.write("H");
                break;
            case ElementType::I4:
                w.write("i");
                break;
            case ElementType::U4:
                w.write("I");
                break;
            case ElementType::I8:
                w.write("L");
                break;
            case ElementType::U8:
                w.write("K");
                break;
            case ElementType::R4:
                w.write("f");
                break;
            case ElementType::R8:
                w.write("d");
                break;
            // TODO: 'u' format string was deprecated in Python 3.3. Need to move to a supported construct
            case ElementType::String:
               w.write("u");
               break;
            case ElementType::Object:
                throw_invalid("structs cannot contain ElementType::Object");
            default:
                throw_invalid("struct_ctor_format_writer element type not impl");
            }
        }
    };

    void write_struct_field_name(writer& w, Field const& field)
    {
        w.write("_%", field.Name());
    }

    void write_struct_field_format(writer& w, Field const& field)
    {
        struct_ctor_format_writer format_writer{ w };
        format_writer.handle(field.Signature().Type());
    }

    struct struct_ctor_var_init_writer : public signature_handler_base<struct_ctor_var_init_writer>
    {
        using signature_handler_base<struct_ctor_var_init_writer>::handle;

        writer& w;
        Field const& field;

        struct_ctor_var_init_writer(writer& wr, Field const& f) : w(wr), field(f)
        {
        }

        void handle_enum(TypeDef const& type)
        {
            w.write("static_cast<%>(%)", type, bind<write_struct_field_name>(field));
        }

        void handle_struct(TypeDef const& type)
        {
            w.write("py::converter<%>::convert_to(%)", type, bind<write_struct_field_name>(field));
        }

        void handle(ElementType)
        {
            write_struct_field_name(w, field);
        }
    };

    void write_struct_field_initalizer(writer& w, Field const& field)
    {
        struct_ctor_var_init_writer writer{ w, field };
        writer.handle(field.Signature().Type());
    }

    void write_struct_field_keyword(writer& w, Field const& field)
    {
        w.write("\"%\", ", field.Name());
    }

    void write_struct_field_parameter(writer& w, Field const& field)
    {
        w.write(", &%", bind<write_struct_field_name>(field));
    }

    void write_struct_constructor(writer& w, TypeDef const& type)
    {
        w.write_indented("PyObject* %_new(PyTypeObject* type, PyObject* args, PyObject* kwds)\n{\n", type.TypeName());
        {
            writer::indent_guard g{ w };

            auto format = R"(auto tuple_size = PyTuple_Size(args);
if ((tuple_size == 0) && (kwds == nullptr))
{
    try
    {
        % instance{};
        return py::wrap_struct(instance, type);
    }
    catch (...)
    {
        return py::to_PyErr();
    }
}

if ((tuple_size == 1) && (kwds == nullptr))
{
    auto arg = PyTuple_GetItem(args, 0);
    if (PyDict_Check(arg))
    {
        try
        {
            auto instance = py::converter<%>::convert_to(arg); 
            return py::wrap_struct(instance, type);
        }
        catch (...)
        {
            return py::to_PyErr();
        }
    }
}

)";
            w.write_indented(format, type, type);

            for (auto&& field : type.FieldList())
            {
                w.write_indented("% %{};\n", bind<write_struct_field_var_type>(field), bind<write_struct_field_name>(field));
            }

            w.write_indented("static char* kwlist[] = {%nullptr};\n\n", bind_each<write_struct_field_keyword>(type.FieldList()));

            w.write_indented("if (!PyArg_ParseTupleAndKeywords(args, kwds, \"%\", kwlist%))\n{\n    return nullptr;\n}\n",
                bind_each<write_struct_field_format>(type.FieldList()),
                bind_each<write_struct_field_parameter>(type.FieldList()));

            w.write_indented("\ntry\n{\n");
            {
                writer::indent_guard gg{ w };
                if (type.TypeNamespace() == "Windows.Foundation" && type.TypeName() == "DateTime")
                {
                    w.write_indented("winrt::Windows::Foundation::DateTime instance{ winrt::Windows::Foundation::TimeSpan{ _UniversalTime } };\n");
                }
                else
                {
                    w.write_indented("% instance{ % };\n", type, bind_list<write_struct_field_initalizer>(", ", type.FieldList()));
                }
                w.write_indented("return py::wrap_struct(instance, type);\n");
            }
            w.write_indented(R"(}
catch (...)
{
    return py::to_PyErr();
}
)");

        }
        w.write_indented("}\n");
    }

    static const std::map<std::string_view, std::string_view> custom_foundation_get_variable = {
        { "DateTime", "self->obj.time_since_epoch().count()" },
        { "EventRegistrationToken", "self->obj.value" },
        { "HResult", "self->obj"} ,
        { "TimeSpan", "self->obj.count()" }
    };

    void write_struct_property_get_variable(writer& w, Field const& field)
    {
        auto parent = field.Parent();
        if (parent.TypeNamespace() == "Windows.Foundation")
        {
            auto custom_var = custom_foundation_get_variable.find(parent.TypeName());
            if (custom_var != custom_foundation_get_variable.end())
            {
                w.write(custom_var->second);
                return;
            }
        }

        auto field_name = get_cpp_field_name(field);
        w.write("self->obj.%", field_name);
    }

    static const std::map<std::string_view, std::string_view> custom_foundation_set_variable = {
        { "DateTime", "self->obj" },
        { "EventRegistrationToken", "self->obj.value" },
        { "HResult", "self->obj"} ,
        { "TimeSpan", "self->obj" }
    };

    void write_struct_property_set_variable(writer& w, Field const& field)
    {
        auto parent = field.Parent();
        if (parent.TypeNamespace() == "Windows.Foundation")
        {
            auto custom_var = custom_foundation_set_variable.find(parent.TypeName());
            if (custom_var != custom_foundation_set_variable.end())
            {
                w.write(custom_var->second);
                return;
            }
        }

        auto field_name = get_cpp_field_name(field);
        w.write("self->obj.%", field_name);
    }

    static const std::map<std::string_view, std::string_view> custom_foundation_set_convert = {
        { "DateTime", "winrt::Windows::Foundation::DateTime{ winrt::Windows::Foundation::TimeSpan{ py::converter<int64_t>::convert_to(value) } }" },
        { "TimeSpan", "winrt::Windows::Foundation::TimeSpan{ py::converter<int64_t>::convert_to(value) }" }
    };

    void write_struct_property_set_convert(writer& w, Field const& field)
    {
        auto parent = field.Parent();
        if (parent.TypeNamespace() == "Windows.Foundation")
        {
            auto custom_convert = custom_foundation_set_convert.find(parent.TypeName());
            if (custom_convert != custom_foundation_set_convert.end())
            {
                w.write(custom_convert->second);
                return;
            }
        }

        w.write("py::converter<%>::convert_to(value)", field.Signature().Type());
    }

    void write_struct_property(writer& w, Field const& field)
    {
        auto field_name = get_cpp_field_name(field);

        {
            auto format = R"(
static PyObject* @_get_%(%* self, void* /*unused*/)
{
    try
    {
        return py::convert(%);
    }
    catch (...)
    {
        return py::to_PyErr();
    }
}
)";
            w.write(format,
                field.Parent().TypeName(),
                field.Name(),
                bind<write_winrt_wrapper>(field.Parent()),
                bind<write_struct_property_get_variable>(field));
        }

        {
            auto format = R"(
static int @_set_%(%* self, PyObject* value, void* /*unused*/)
{
    if (value == nullptr)
    {
        PyErr_SetString(PyExc_TypeError, "property delete not supported");
        return -1;
    }
    
    try
    {
        % = %;
        return 0;
    }
    catch (...)
    {
        return -1;
    }
}
)";
            w.write(format,
                field.Parent().TypeName(),
                field.Name(),
                bind<write_winrt_wrapper>(field.Parent()),
                bind<write_struct_property_set_variable>(field),
                bind<write_struct_property_set_convert>(field));
        }
    }

    void write_struct(writer& w, TypeDef const& type)
    {
        auto guard{ w.push_generic_params(type.GenericParam()) };

        w.write_indented("\n// ----- % struct --------------------\n", type.TypeName());
        write_winrt_type_specialization_storage(w, type);
        write_struct_convert_functions(w, type);
        write_struct_constructor(w, type);
        w.write_each<write_struct_property>(type.FieldList());
        write_getset_table(w, type);
        write_type_slot_table(w, type);
        write_type_spec(w, type);
    }













    void write_type_fromspec(writer& w, TypeDef const& type)
    {
        if (is_exclusive_to(type))
        {
            return;
        }

        auto winrt_type_param = is_ptype(type)
            ? w.write_temp("py@", type.TypeName())
            : w.write_temp("%", type);

        if (has_dealloc(type))
        {
            w.write_indented("\ntype_object = PyType_FromSpecWithBases(&@_Type_spec, bases);\n", type.TypeName());
        }
        else
        {
            w.write_indented("\ntype_object = PyType_FromSpec(&@_Type_spec);\n", type.TypeName());
        }


        auto format = R"(if (type_object == nullptr)
{
    return -1;
}
if (PyModule_AddObject(module, "@", type_object) != 0)
{
    Py_DECREF(type_object);
    return -1;
}
py::winrt_type<%>::python_type = reinterpret_cast<PyTypeObject*>(type_object);
type_object = nullptr;
)";
        w.write_indented(format,
            type.TypeName(),
            winrt_type_param);
    }

    void write_namespace_init(writer& w, filter const& f, std::string_view const& ns, cache::namespace_members const& members)
    {
        w.write("\n// ----- % Initialization --------------------\n", ns);
        auto format = R"(
int %(PyObject* module)
{
    PyObject* type_object{ nullptr };
    PyObject* bases = PyTuple_Pack(1, py::winrt_type<py::winrt_base>::python_type);
)";

        w.write_indented(format, bind<write_ns_init_function_name>(ns));
        {
            writer::indent_guard g{ w };

            f.bind_each<write_type_fromspec>(members.classes)(w);
            f.bind_each<write_type_fromspec>(members.interfaces)(w);
            f.bind_each<write_type_fromspec>(members.structs)(w);

            w.write_indented(R"(
Py_DECREF(bases);
return 0;
)");
        }
        w.write_indented("}\n");

        format = R"(
static PyModuleDef_Slot module_slots[] = {
    {Py_mod_exec, %},
    {0, nullptr}
};

PyDoc_STRVAR(module_doc, "Langworthy projection module.\n");

static struct PyModuleDef module_def = {
    PyModuleDef_HEAD_INIT,
    "%",
    module_doc,
    0,
    nullptr,
    module_slots,
    nullptr,
    nullptr,
    nullptr
};

PyMODINIT_FUNC
PyInit_%(void)
{
    return PyModuleDef_Init(&module_def);
}
)";
        auto segments = get_dotted_name_segments(ns);
        auto module_name = w.write_temp("_%_%", settings.module, bind_list("_", segments));
        w.write_indented(format, bind<write_ns_init_function_name>(ns), module_name, module_name);
    }


    void write_python_namespace_includes(writer& w, std::vector<std::string> const& namespaces)
    {
        for (auto&& ns : namespaces)
        {
            w.write("#include \"py.%.h\"\n", ns);
        }
    }
    
    void write_module_exec(writer& w)
    {
        {
            auto format = R"(
static int module_exec(PyObject* module)
{
    PyObject* type_object = PyType_FromSpec(&winrt_base_Type_spec);
    if (type_object == nullptr)
    {
        return -1;
    }
    if (PyModule_AddObject(module, "_winrt_base", type_object) != 0)
    {
        Py_DECREF(type_object);
        return -1;
    }
    py::winrt_type<py::winrt_base>::python_type = reinterpret_cast<PyTypeObject*>(type_object);
    type_object = nullptr;

)";
            w.write(format);
        }

        w.write("    return 0;\n}\n");
    }

    void write_module_slots(writer& w)
    {
        auto format = R"(
static PyModuleDef_Slot module_slots[] = {
    {Py_mod_exec, module_exec},
    {0, nullptr}
};
)";
        w.write(format);
    }

    void write_module_def(writer& w, std::string_view const& module_name)
    {
        auto format = R"(
PyDoc_STRVAR(module_doc, "Langworthy projection module.\n");

static struct PyModuleDef module_def = {
    PyModuleDef_HEAD_INIT,
    "%",
    module_doc,
    0,
    module_methods,
    module_slots,
    nullptr,
    nullptr,
    nullptr
};
)";
        w.write(format, module_name);
    }

    void write_module_init_func(writer& w, std::string_view const& module_name)
    {
        auto format = R"(
PyMODINIT_FUNC
PyInit_%(void)
{
    return PyModuleDef_Init(&module_def);
}
)";
        w.write(format, module_name);
    }
}