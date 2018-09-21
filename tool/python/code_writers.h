#pragma once

namespace xlang
{
    void write_include(writer& w, std::string_view const& ns)
    {
        w.write("#include \"py.%.h\"\n", ns);
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
            w.write("'py.%.cpp', ", ns);
        }

        w.write("'%.cpp'", module_name);
    }

    void write_winrt_wrapper(writer& w, TypeDef const& type)
    {
        if (is_ptype(type))
        {
            w.write("py::winrt_pinterface_wrapper<py@>", type.TypeName());
        }
        else
        {
            w.write("py::winrt_wrapper<%>", type);
        }
    }

    void write_getset_table_prop_row(writer& w, Property const& prop)
    {
        auto property_methods = get_property_methods(prop);

        auto getter = w.write_temp("(getter)@_%", prop.Parent().TypeName(), property_methods.get.Name());
        auto setter = property_methods.set
            ? w.write_temp("(setter)@_%", prop.Parent().TypeName(), property_methods.set.Name())
            : w.write_temp("nullptr");

        // TODO: remove const_cast once pywinrt is updated to target Python 3.7. 
        //       pywinrt currently targeting 3.6 because that's the version that ships with VS 2017 v15.8
        //       https://github.com/python/cpython/commit/007d7ff73f4e6e65cfafd512f9c23f7b7119b803
        w.write("    { const_cast<char*>(\"%\"), %, %, nullptr, nullptr },\n",
            prop.Name(),
            getter,
            setter);
    }

    //void write_getset_table_row_event(writer& w, Event const& event)
    //{
    //    // TODO: remove const_cast once pywinrt is updated to target Python 3.7. 
    //    //       pywinrt currently targeting 3.6 because that's the version that ships with VS 2017 v15.8
    //    //       https://github.com/python/cpython/commit/007d7ff73f4e6e65cfafd512f9c23f7b7119b803
    //    w.write("    { const_cast<char*>(\"%\"), (getter)@_event_%, nullptr, nullptr, nullptr },\n", event.Name(),
    //        event.Parent().TypeName(), event.Name());
    //}

    void write_getset_table(writer& w, TypeDef const& type)
    {
        if (has_getsets(type))
        {
            w.write("\nstatic PyGetSetDef @_getset[] = {\n", type.TypeName());

            w.write_each<write_getset_table_prop_row>(get_instance_properties(type));
            //w.write_each<write_class_getset_table_row_event>(get_instance_events(type));

            w.write("    { nullptr }\n};\n");
        }
    }

    void write_method_table(writer& w, TypeDef const& type)
    {
        if (has_methods(type))
        {
            std::set<std::string_view> instance_methods{};
            std::set<std::string_view> static_methods{};

            w.write("\nstatic PyMethodDef @_methods[] = {\n", type.TypeName());

            for (auto&& method : get_methods(type))
            {
                auto is_static_method = method.Flags().Static();
                auto& set = is_static_method ? static_methods : instance_methods;

                if (set.find(method.Name()) == set.end())
                {
                    set.emplace(method.Name());

                    w.write("    { \"%\", (PyCFunction)@_%, METH_VARARGS%, nullptr },\n",
                        method.Name(), type.TypeName(), method.Name(), is_static_method ? " | METH_STATIC" : "");
                }
            }

            for (auto&& prop : get_static_properties(type))
            {
                w.write("    { \"%\", (PyCFunction)@_%, METH_VARARGS | METH_STATIC, nullptr },\n",
                    prop.Name(), type.TypeName(), prop.Name());
            }

            if (get_static_events(type).size() > 0)
            {
                throw_invalid("static events not impl");
            }

            //for (auto&& event : get_static_events(type))
            //{
            //    w.write("    { \"%\", (PyCFunction)%_%, METH_VARARGS | METH_STATIC, nullptr },\n",
            //        event.Name(), type.TypeName(), event.Name());
            //}

            w.write("    { nullptr }\n};\n");
        }
    }

    void write_type_slot_table(writer& w, TypeDef const& type)
    {
        auto format = R"(
static PyType_Slot @_Type_slots[] = 
{
    { Py_tp_base, nullptr }, // filled out in module init
    { Py_tp_new, @_new },
)";
        w.write(format, type.TypeName(), type.TypeName());

        if (has_dealloc(type))
        {
            w.write("    { Py_tp_dealloc, @_dealloc },\n", type.TypeName());
        }

        if (has_methods(type))
        {
            w.write("    { Py_tp_methods, @_methods },\n", type.TypeName());
        }

        if (has_getsets(type))
        {
            w.write("    { Py_tp_getset, @_getset },\n", type.TypeName());
        }

        if (distance(type.FieldList()) > 0)
        {
            w.write("    { Py_tp_members, @_members },\n", type.TypeName());
        }

        w.write("    { 0, nullptr },\n};\n");
    }

    void write_type_spec_size(writer& w, TypeDef const& type)
    {
        if (is_ptype(type))
        {
            w.write("sizeof(%)", bind<write_winrt_wrapper>(type));
            return;
        }

        auto category = get_category(type);

        if (category == category::class_type && type.Flags().Abstract())
        {
            w.write("0");
        }
        else
        {
            w.write("sizeof(%)", bind<write_winrt_wrapper>(type));
        }

        //if (category == category::struct_type)
        //{
        //    w.write("sizeof(%)", type.TypeName());
        //}
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

    void write_property_get_body(writer& w, std::string_view const& self, std::string_view const& prop_name)
    {
        auto format = R"(    try
    {
        auto return_value = %.%();
        return py::convert(return_value);
    }
    catch (...)
    {
        return py::to_PyErr();
    }
)";
        w.write(format, self, prop_name);
    }



    void write_out_param_init(writer& w, method_signature::param_t const& param)
    {
        // WinRT class, interface and delegate out params must be initialized as nullptr

        auto write_typedef = [&](TypeDef const& type)
        {
            switch (get_category(type))
            {
            case category::class_type:
            case category::delegate_type:
            case category::interface_type:
                w.write("nullptr");
            }
        };

        visit(param.second->Type().Type(),
            [&](coded_index<TypeDefOrRef> const& index)
        {
            switch (index.type())
            {
            case TypeDefOrRef::TypeDef:
                write_typedef(index.TypeDef());
                break;

            case TypeDefOrRef::TypeRef:
            {
                auto tr = index.TypeRef();
                if (tr.TypeNamespace() != "System" || tr.TypeName() != "Guid")
                {
                    write_typedef(find(tr));
                }
            }
            break;
            }
        },
            [&](ElementType) {},
            [&](GenericTypeInstSig const&) { w.write("nullptr"); },
            [&](GenericTypeIndex /*index*/) { throw_invalid("write GenericTypeIndex not impl"); });
    }

    void write_param_declaration(writer& w, method_signature::param_t const& param)
    {
        auto sequence = param.first.Sequence() - 1;

        switch (get_param_category(param))
        {
        case param_category::in:
            w.write("            auto param% = py::convert_to<%>(args, %);\n", sequence, param.second->Type(), sequence);
            break;
        case param_category::out:
            w.write("            % param% { % };\n", param.second->Type(), sequence, bind<write_out_param_init>(param));
            break;
        case param_category::pass_array:
            w.write("            /*p*/ winrt::array_view<% const> param% {}; //= py::convert_to<winrt::array_view<% const>>(args, %);\n", param.second->Type(), sequence, param.second->Type(), sequence);
            break;
        case param_category::receive_array:
            w.write("            /*r*/ winrt::array_view<%> param% { };\n", param.second->Type(), sequence);
            break;
        default:
            throw_invalid("write_param_conversion not impl");
        }
    }

    void write_class_method_overload_return(writer& w, method_signature const& signature)
    {
        if (signature.return_signature())
        {
            w.write("% return_value = ", signature.return_signature().Type());
        }
    }










    void write_class_method_overload_invoke_context(writer& w, MethodDef const& method)
    {
        if (is_ptype(method.Parent()))
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

    void write_class_method_overload(writer& w, MethodDef const& method, method_signature const& signature)
    {
        w.write("        try\n        {\n");
        w.write_each<write_param_declaration>(signature.params());

        if (signature.has_params())
        {
            w.write("\n");
        }

        w.write("            %%%(%);\n",
            bind<write_class_method_overload_return>(signature),
            bind<write_class_method_overload_invoke_context>(method),
            get_name(method),
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
    auto bind_method_overloads(writer& w, std::vector<MethodDef> const& methods)
    {
        w.write("    Py_ssize_t arg_count = PyTuple_Size(args);\n\n");

        bool first{ true };
        for (auto&& m : methods)
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
            F(w, m, signature);
            w.write("    }\n");
        }

        w.write(R"(    else if (arg_count == -1)
    {
        return nullptr; 
    }

    PyErr_SetString(PyExc_RuntimeError, "Invalid parameter count");
    return nullptr;
)");
    }

    template <auto F>
    void write_type_method(writer& w, TypeDef const& type, std::string_view const& method_name)
    {
        std::vector<MethodDef> methods{};

        for (auto&& method : get_methods(type))
        {
            if (method.Name() == method_name)
            {
                methods.push_back(method);
            }
        }

        XLANG_ASSERT(methods.size() > 0);
        bool static_method = methods[0].Flags().Static();

        if (methods.size() > 1)
        {
            // ensure all the methods found match the static flag of the first method
            XLANG_ASSERT(std::all_of(methods.begin(), methods.end(),
                [static_method](MethodDef const& m) { return m.Flags().Static() == static_method; }));
        }

        F(w, type, methods);
        bind_method_overloads<write_class_method_overload>(w, methods);
        w.write("}\n");
    }

    template <auto F>
    void write_type_methods(writer& w, TypeDef const& type)
    {
        std::set<std::string_view> method_set{};

        for (auto&& method : get_methods(type))
        {
            if (method_set.find(method.Name()) == method_set.end())
            {
                method_set.emplace(method.Name());
                write_type_method<F>(w, type, method.Name());
            }
        }
    }
    
    void write_class_method_decl(writer& w, TypeDef const& type, std::vector<MethodDef> const& methods)
    {
        auto method = *methods.begin();
        if (method.Flags().Static())
        {
            w.write("\nstatic PyObject* @_%(PyObject* /*unused*/, PyObject* args)\n{ \n", 
                type.TypeName(), 
                method.Name());
        }
        else
        {
            w.write("\nstatic PyObject* @_%(%* self, PyObject* args)\n{ \n", 
                type.TypeName(), 
                method.Name(), 
                bind<write_winrt_wrapper>(type));
        }
    }

    void write_class_methods(writer& w, TypeDef const& type)
    {
        XLANG_ASSERT(!is_ptype(type));

        write_type_methods<write_class_method_decl>(w, type);

        for (auto&& prop : get_static_properties(type))
        {
            std::vector<MethodDef> methods{};
            auto prop_methods = get_property_methods(prop);
            methods.push_back(prop_methods.get);

            if (prop_methods.set)
            {
                methods.push_back(prop_methods.set);
            }

            w.write("\nstatic PyObject* %_%(PyObject* /*unused*/, PyObject* args)\n{ \n", type.TypeName(), prop.Name());
            bind_method_overloads<write_class_method_overload>(w, methods);
            w.write("}\n");
        }
    }

    void write_class_constructor_overload(writer& w, MethodDef const& method, method_signature const& signature)
    {
        w.write("        try\n        {\n");

        w.write_each<write_param_declaration>(signature.params());

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
        w.write("\nPyObject* %_new(PyTypeObject* type, PyObject* args, PyObject* kwds)\n{\n", type.TypeName());

        auto constructors = get_constructors(type);
        if (type.Flags().Abstract() || constructors.size() == 0)
        {
            auto format = R"(    PyErr_SetString(PyExc_RuntimeError, "% is not activatable");
    return nullptr;
)";
            w.write(format, type.TypeName());
        }
        else
        {
            w.write(R"(    if (kwds != nullptr)
    {
        PyErr_SetString(PyExc_RuntimeError, "keyword arguments not supported");
        return nullptr;
    }

)");
            bind_method_overloads<write_class_constructor_overload>(w, constructors);
        }

        w.write("}\n");
    }

    void write_class_property(writer& w, Property const& prop)
    {
        XLANG_ASSERT(!is_ptype(prop.Parent()));

        auto property_methods = get_property_methods(prop);

        {
            auto format = R"(
static PyObject* @_%(%* self, void* /*unused*/)
{
%}
)";
            w.write(format, 
                prop.Parent().TypeName(),
                property_methods.get.Name(),
                bind<write_winrt_wrapper>(prop.Parent()),
                bind<write_property_get_body>("self->obj", prop.Name()));
        }

        if (property_methods.set)
        {
            auto format = R"(
static int @_%(%* self, PyObject* value, void* /*unused*/)
{
    if (value == nullptr)
    {
        PyErr_SetString(PyExc_RuntimeError, "property delete not supported");
        return -1;
    }
    
    try
    {
        auto param0 = py::convert_to<%>(value);
        self->obj.%(param0);
        return 0;
    }
    catch (...)
    {
        return -1;
    }
}
)";
            w.write(format,
                prop.Parent().TypeName(),
                property_methods.set.Name(),
                bind<write_winrt_wrapper>(prop.Parent()),
                prop.Type().Type(),
                prop.Name());
        }
    }

//    void write_class_event(writer& w, Event const& event)
//    {
//        auto format = R"(
//static PyObject* @_event_%(%* self, void* /*unused*/)
//{
//    try
//    {
//        // TODO: event handling
//        throw winrt::hresult_not_implemented();
//    }
//    catch (...)
//    {
//        return py::to_PyErr();
//    }
//}
//)";
//        w.write(format,
//            event.Parent().TypeName(),
//            event.Name(),
//            bind<write_winrt_wrapper>(event.Parent()));
//    }
//
//    void write_class_events(writer& w, TypeDef const& type)
//    {
//        w.write_each<write_class_event>(get_instance_events(type));
//    }

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
        w.write("PyTypeObject* py::winrt_type<%>::python_type;\n", type);        
        write_class_constructor(w, type);
        write_class_dealloc(w, type);
        write_class_methods(w, type);
        w.write_each<write_class_property>(get_instance_properties(type));
        //write_class_events(w, type);
        write_method_table(w, type);
        write_getset_table(w, type);
        write_type_slot_table(w, type);
        write_type_spec(w, type);
    }













    void write_pinterface_constructor(writer& w, TypeDef const& type)
    {
        XLANG_ASSERT(get_category(type) == category::interface_type);
        XLANG_ASSERT(is_ptype(type));

        auto format = R"(
PyObject* @_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    // TODO implement QI in this pinterface constructor
    PyErr_SetString(PyExc_RuntimeError, "@ is not activatable");
    return nullptr;
}
)";
        w.write(format, type.TypeName(), type.TypeName());
    }

    void write_pinterface_dealloc(writer& w, TypeDef const& type)
    {
        XLANG_ASSERT(has_dealloc(type));
        XLANG_ASSERT(is_ptype(type));
        auto format = R"(
static void @_dealloc(%* self)
{
    py::wrapped_instance(self->obj->hash(), nullptr);
    self->obj.release();
}
)";
        w.write(format, type.TypeName(), bind<write_winrt_wrapper>(type));
    }

    void write_pinterface_methods(writer& w, TypeDef const& type)
    {
        XLANG_ASSERT(is_ptype(type));

        std::set<std::string_view> method_set{};

        for (auto&& method : get_methods(type))
        {
            if (method_set.find(method.Name()) == method_set.end())
            {
                method_set.emplace(method.Name());

                auto format = R"(
static PyObject* @_%(%* self, PyObject* args)
{
    return self->obj->%(args);
}
)";
                w.write(format,
                    type.TypeName(),
                    method.Name(),
                    bind<write_winrt_wrapper>(type),
                    method.Name());
            }
        }
    }

    void write_pinterface_property(writer& w, Property const& prop)
    {
        XLANG_ASSERT(is_ptype(prop.Parent()));

        auto property_methods = get_property_methods(prop);

        {
            auto format = R"(
static PyObject* @_%(%* self, void* /*unused*/)
{
    return self->obj->%();
}
)";
            w.write(format,
                prop.Parent().TypeName(),
                property_methods.get.Name(),
                bind<write_winrt_wrapper>(prop.Parent()),
                property_methods.get.Name());
        }

        if (property_methods.set)
        {
            auto format = R"(
static int @_%(%* self, PyObject* value, void* /*unused*/)
{
    return self->obj->%();
}
)";
            w.write(format,
                prop.Parent().TypeName(),
                property_methods.get.Name(),
                bind<write_winrt_wrapper>(prop.Parent()),
                property_methods.set.Name());
        }
    }

    void write_pinterface(writer& w, TypeDef const& type)
    {
        auto guard{ w.push_generic_params(type.GenericParam()) };

        w.write("\n// ----- @ parameterized interface --------------------\n", type.TypeName());
        w.write("PyTypeObject* py::winrt_type<py@>::python_type;\n", type.TypeName());

        write_pinterface_constructor(w, type);
        write_pinterface_dealloc(w, type);
        write_pinterface_methods(w, type);
        w.write_each<write_pinterface_property>(get_instance_properties(type));
        //write_class_events(w, type);
        write_method_table(w, type);
        write_getset_table(w, type);
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
        w.write("    virtual std::size_t hash() = 0;\n\n");
            
        for (auto&& method : get_methods(type))
        {
            w.write("    virtual PyObject* %(PyObject* args) = 0;\n", method.Name());
        }

        for (auto&& prop : type.PropertyList())
        {
            auto prop_methods = get_property_methods(prop);

            w.write("    virtual PyObject* %() = 0;\n", prop_methods.get.Name());
            if (prop_methods.set)
            {
                w.write("    virtual int %(PyObject* arg) = 0;\n", prop_methods.set.Name());
            }
        }

        // TODO interface events

        w.write("};\n");
    }

    void write_pinterface_method_decl(writer& w, TypeDef const& type, std::vector<MethodDef> const& methods)
    {
        auto method = *methods.begin();
        w.write("\nPyObject* %(PyObject* args) override\n{\n", method.Name());
    }

    void write_pinterface_impl(writer& w, TypeDef const& type)
    {
        if (!is_ptype(type))
            return;

        auto guard{ w.push_generic_params(type.GenericParam()) };

        w.write("\ntemplate<%>\nstruct py@Impl : public py@\n{\n", bind_list<write_pinterface_type_args>(", ", type.GenericParam()), type.TypeName(), type.TypeName());

        w.write("py@Impl(%<%> o) : obj(o) {}\n", type.TypeName(), type, bind_list<write_pinterface_type_arg_name>(", ", type.GenericParam()));
        w.write("std::size_t hash() override { return py::get_instance_hash(obj); }\n");

        write_type_methods<write_pinterface_method_decl>(w, type);

        for (auto&& prop : type.PropertyList())
        {
            auto prop_methods = get_property_methods(prop);

            {
                auto format = R"(
PyObject* %() override
{
%}
)";
                w.write(format, prop_methods.get.Name(), bind<write_property_get_body>("obj", prop.Name()));
            }
            if (prop_methods.set)
            {
                w.write("    int set_%(PyObject* arg) override { return -1; }; // pinterface prop set not impl \n", prop.Name());
            }
        }

        // TODO interface events

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
        XLANG_ASSERT(!is_ptype(type));

        auto format = R"(
PyObject* @_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    if (kwds != nullptr)
    {
        PyErr_SetString(PyExc_RuntimeError, "keyword arguments not supported");
        return nullptr;
    }

    Py_ssize_t arg_count = PyTuple_Size(args);

    if (arg_count == 1)
    {
        try
        {
            auto param0 = py::convert_to<%>(args, 0);
            return py::wrap(param0, type);
        }
        catch (...)
        {
            return py::to_PyErr();
        }
    }
    else if (arg_count == -1)
    {
        return nullptr; 
    }

    PyErr_SetString(PyExc_RuntimeError, "Invalid parameter count");
    return nullptr;
}
)";
        w.write(format, type.TypeName(), type);
    }

    void write_interface(writer& w, TypeDef const& type)
    {
        if (is_exclusive_to(type))
        {
            return;
        }

        if (is_ptype(type))
        {
            write_pinterface(w, type);
            return;
        }

        auto guard{ w.push_generic_params(type.GenericParam()) };

        w.write("\n// ----- @ interface --------------------\n", type.TypeName());
        w.write("PyTypeObject* py::winrt_type<%>::python_type;\n", type);

        write_interface_constructor(w, type);
        write_class_dealloc(w, type);
        write_class_methods(w, type);
        w.write_each<write_class_property>(get_instance_properties(type));
        //write_class_events(w, type);
        write_method_table(w, type);
        write_getset_table(w, type);
        write_type_slot_table(w, type);
        write_type_spec(w, type);
    }























//    void write_struct_constructor(writer& w, TypeDef const& type)
//    {
//        // TODO: replace stub constructor with real code
//        auto format = R"(
//PyObject* %_constructor(PyTypeObject* type, PyObject* args, PyObject* kwds)
//{
//    if (kwds != nullptr)
//    {
//        PyErr_SetString(PyExc_RuntimeError, "keyword arguments not supported");
//        return nullptr;
//    }
//
//    Py_ssize_t arg_count = PyTuple_Size(args);
//
//    if (arg_count == 0)
//    {
//        //auto py_instance = PyObject_New(%, reinterpret_cast<PyTypeObject*>(py::winrt_type<%>::python_type));
//
//        //if (!py_instance)
//        //{
//        //    return nullptr;
//        //}
//
//        //// PyObject_New doesn't call type's constructor, so manually initialize the contained struct
//        //std::memset(&(py_instance->value), 0, sizeof(%));
//        //return reinterpret_cast<PyObject*>(py_instance);
//
//        PyErr_SetNone(PyExc_NotImplementedError);
//        return nullptr;
//    }
//    else if (arg_count == -1)
//    {
//        return nullptr; 
//    }
//
//    PyErr_SetString(PyExc_RuntimeError, "Invalid parameter count");
//    return nullptr;
//}
//)";
//        w.write(format, type.TypeName(), type.TypeName(), type, type);
//    }
//
//    void write_struct_type(writer& w, FieldSig const& signature)
//    {
//        auto process_typedef = [&](TypeDef const& type)
//        {
//            auto category = get_category(type);
//            if (category == category::enum_type)
//            {
//                if (is_flags_enum(type))
//                {
//                    w.write("T_UINT");
//                }
//                else
//                {
//                    w.write("T_INT");
//                }
//            }
//            else if (category == category::struct_type)
//            {
//                throw_invalid("struct hierarchy not impl");
//            }
//            else
//            {
//                throw_invalid("invalid struct member");
//            }
//        };
//
//        return visit(signature.Type().Type(),
//            [&](coded_index<TypeDefOrRef> const& index)
//        {
//            switch (index.type())
//            {
//            case TypeDefOrRef::TypeDef:
//                return process_typedef(index.TypeDef());
//            case TypeDefOrRef::TypeRef:
//            {
//                auto tr = index.TypeRef();
//                if (tr.TypeNamespace() == "System" & tr.TypeName() == "Guid")
//                {
//                    throw_invalid("struct guid not impl");
//                }
//                if (tr.TypeNamespace() == "Windows.Foundation")
//                {
//                    if (tr.TypeName() == "DateTime" || tr.TypeName() == "TimeSpan")
//                    {
//                        w.write("T_LONGLONG");
//                        return;
//                    }
//                }
//                return process_typedef(find(tr));
//            }
//            case TypeDefOrRef::TypeSpec:
//                throw_invalid("struct TypeSpec not supported");
//            }
//        },
//            [&](ElementType type)
//        {
//            switch (type)
//            {
//            case ElementType::I2:
//                w.write("T_SHORT");
//                break;
//            case ElementType::U2:
//                w.write("T_USHORT");
//                break;
//            case ElementType::I4:
//                w.write("T_INT");
//                break;
//            case ElementType::U4:
//                w.write("T_UINT");
//                break;
//            case ElementType::I8:
//                w.write("T_LONGLONG");
//                break;
//            case ElementType::U8:
//                w.write("T_ULONGLONG");
//                break;
//            case ElementType::R4:
//                w.write("T_FLOAT");
//                break;
//            case ElementType::R8:
//                w.write("T_DOUBLE");
//                break;
//            default:
//                throw_invalid("struct element type not impl");
//            }
//        },
//            [&](GenericTypeInstSig const&) { throw_invalid("struct GenericTypeInstSig not supported"); },
//            [&](GenericTypeIndex) { throw_invalid("struct GenericTypeIndex not supported"); });
//    }
//
//    void write_struct(writer& w, TypeDef const& type)
//    {
//        auto guard{ w.push_generic_params(type.GenericParam()) };
//
//        w.write("\n// ----- % struct --------------------\n", type.TypeName());
//
//        auto format = R"(
//struct %
//{
//    PyObject_HEAD
//    % value{};
//};
//)";
//        w.write(format, type.TypeName(), type);
//
//        write_struct_constructor(w, type);
//
//        w.write("\nstatic PyMemberDef %_members[] = {\n", type.TypeName());
//
//        // TODO: remove const_cast once pywinrt is updated to target Python 3.7. 
//        //       pywinrt currently targeting 3.6 because that's the version that ships with VS 2017 v15.8
//        //       https://github.com/python/cpython/commit/007d7ff73f4e6e65cfafd512f9c23f7b7119b803
//
//        for (auto&& field : type.FieldList())
//        {
//            w.write("    { const_cast<char*>(\"%\"), %, offsetof(%, value.%), 0, nullptr },\n", field.Name(), bind<write_struct_type>(field.Signature()), type.TypeName(), field.Name());
//        }
//
//        w.write("    { nullptr },\n};\n");
//
//        write_type_slot_table(w, type);
//        write_type_spec(w, type);
//    }
//
//    void write_delegate(writer& w, TypeDef const& type)
//    {
//        auto guard{ w.push_generic_params(type.GenericParam()) };
//
//        w.write("\n// ----- % delegate --------------------\n", type.TypeName());
//    }
//void write_enum(writer& w, TypeDef const& type)
//{
//    auto guard{ w.push_generic_params(type.GenericParam()) };
//    w.write("\n// ----- % enum --------------------\n", type.TypeName());
//}













    void write_type_fromspec(writer& w, TypeDef const& type)
    {
        if (is_exclusive_to(type))
        {
            return;
        }

        auto winrt_type_param = is_ptype(type)
            ? w.write_temp("py@", type.TypeName())
            : w.write_temp("%", type);

        auto format = R"(
    @_Type_slots[0].pfunc = py::winrt_type<py::winrt_base>::python_type;
    type_object = PyType_FromSpec(&@_Type_spec);
    if (type_object == nullptr)
    {
        return -1;
    }
    if (PyModule_AddObject(module, "@", type_object) != 0)
    {
        return -1;
    }
    py::winrt_type<%>::python_type = reinterpret_cast<PyTypeObject*>(type_object);
)";
        w.write(format, 
            type.TypeName(), 
            type.TypeName(), 
            type.TypeName(),
            winrt_type_param);
    }

    void write_python_namespace_includes(writer& w, std::vector<std::string> const& namespaces)
    {
        for (auto&& ns : namespaces)
        {
            w.write("#include \"py.%.h\"\n", ns);
        }
    }
    
    void write_module_exec(writer& w, std::vector<std::string> const& namespaces)
    {
        {
            auto format = R"(
static int module_exec(PyObject* module)
{
    PyObject* type_object{ nullptr };
    type_object = PyType_FromSpec(&winrt_base_Type_spec);
    if (type_object == nullptr)
    {
        return -1;
    }
    if (PyModule_AddObject(module, "_winrt_base", type_object) != 0)
    {
        return -1;
    }
    py::winrt_type<py::winrt_base>::python_type = reinterpret_cast<PyTypeObject*>(type_object);

)";
            w.write(format);
        }

        for (auto&& ns : namespaces)
        {
            auto format = R"(    if (%(module) != 0)
    {
        return -1;
    }

)";
            w.write(format, bind<write_ns_init_function_name>(ns));
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