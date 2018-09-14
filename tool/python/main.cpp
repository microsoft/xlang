#include "pch.h"
#include "strings.h"

using namespace std::chrono;
using namespace std::experimental::filesystem;
using namespace std::string_view_literals;
using namespace xlang;
using namespace xlang::meta::reader;
using namespace xlang::text;

// ----- general metadata helpers --------------------

template <typename T>
struct signature_handler_base
{
    void handle_class(TypeDef const& /*type*/) {}
    void handle_delegate(TypeDef const& /*type*/) {}
    void handle_enum(TypeDef const& /*type*/) {}
    void handle_guid(TypeRef const& /*type*/) {}
    void handle_interface(TypeDef const& /*type*/) {}
    void handle_struct(TypeDef const& /*type*/) {}

    void handle(TypeRef const& type)
    {
        auto ns = type.TypeNamespace();
        auto name = type.TypeName();

        if (name == "Guid" && ns == "System")
        {
            static_cast<T*>(this)->handle_guid(type);
        }
        else
        {
            static_cast<T*>(this)->handle(find_required(type));
        }
    }

    void handle(TypeDef const& type)
    {
        switch (get_category(type))
        {
        case category::class_type:
            static_cast<T*>(this)->handle_class(type);
            break;
        case category::delegate_type:
            static_cast<T*>(this)->handle_delegate(type);
            break;
        case category::interface_type:
            static_cast<T*>(this)->handle_interface(type);
            break;
        case category::enum_type:
            static_cast<T*>(this)->handle_enum(type);
            break;
        case category::struct_type:
            static_cast<T*>(this)->handle_struct(type);
            break;
        }
    }

    void handle(coded_index<TypeDefOrRef> const& type)
    {
        switch (type.type())
        {
        case TypeDefOrRef::TypeDef:
            static_cast<T*>(this)->handle(type.TypeDef());
            break;

        case TypeDefOrRef::TypeRef:
            static_cast<T*>(this)->handle(type.TypeRef());
            break;

        case TypeDefOrRef::TypeSpec:
            static_cast<T*>(this)->handle(type.TypeSpec().Signature().GenericTypeInst());
            break;
        }
    }

    void handle_start_generic() {}

    void handle_end_generic() {}

    void handle(GenericTypeInstSig const& type)
    {
        handle(type.GenericType());
        static_cast<T*>(this)->handle_start_generic();
        for (auto&& arg : type.GenericArgs())
        {
            handle(arg);
        }
        static_cast<T*>(this)->handle_end_generic();
    }

    void handle(ElementType /*type*/) {}

    void handle(GenericTypeIndex /*var*/) { }

    void handle(TypeSig const& signature)
    {
        visit(signature.Type(),
            [&](auto&& type)
        {
            static_cast<T*>(this)->handle(type);
        });
    }
};

struct method_signature
{
    using param_t = std::pair<Param, ParamSig const*>;

    explicit method_signature(MethodDef const& method) :
        m_method(method.Signature())
    {
        auto params = method.ParamList();

        if (m_method.ReturnType() && params.first != params.second && params.first.Sequence() == 0)
        {
            m_return = params.first;
            ++params.first;
        }

        for (uint32_t i{}; i != m_method.Params().size(); ++i)
        {
            m_params.emplace_back(params.first + i, m_method.Params().data() + i);
        }
    }

    std::vector<param_t>& params()
    {
        return m_params;
    }

    std::vector<param_t> const& params() const
    {
        return m_params;
    }

    auto const& return_signature() const
    {
        return m_method.ReturnType();
    }

    auto return_param_name() const
    {
        std::string_view name;

        if (m_return)
        {
            name = m_return.Name();
        }
        else
        {
            name = "winrt_impl_return_value";
        }

        return name;
    }

    bool has_params() const
    {
        return !m_params.empty();
    }

private:

    MethodDefSig m_method;
    std::vector<param_t> m_params;
    Param m_return;
};

bool is_exclusive_to(TypeDef const& type)
{
    return get_category(type) == category::interface_type && get_attribute(type, "Windows.Foundation.Metadata", "ExclusiveToAttribute");
}

bool is_flags_enum(TypeDef const& type)
{
    return get_category(type) == category::enum_type && get_attribute(type, "System", "FlagsAttribute");
}

inline auto get_name(MethodDef const& method)
{
    auto name = method.Name();

    if (method.SpecialName())
    {
        return name.substr(name.find('_') + 1);
    }

    return name;
}

// ----- Python projection specific metadata helpers --------------------

auto get_dotted_name_segments(std::string_view ns){    std::vector<std::string_view> segments;    size_t pos = 0;    do    {        auto new_pos = ns.find('.', pos);
        if (new_pos == std::string_view::npos)        {            segments.push_back(ns.substr(pos));            return std::move(segments);        }
        segments.push_back(ns.substr(pos, new_pos - pos));        pos = new_pos + 1;    } while (true);};

struct property_type
{
    MethodDef get;
    MethodDef set;
};

property_type get_property_methods(Property const& prop)
{
    MethodDef get_method{}, set_method{};

    for (auto&& method_semantic : prop.MethodSemantic())
    {
        auto semantic = method_semantic.Semantic();

        if (semantic.Getter())
        {
            get_method = method_semantic.Method();
        }
        else if (semantic.Setter())
        {
            set_method = method_semantic.Method();
        }
        else
        {
            throw_invalid("Properties can only have get and set methods");
        }
    }

    XLANG_ASSERT(get_method);

    if (set_method)
    {
        XLANG_ASSERT(get_method.Flags().Static() == set_method.Flags().Static());
    }

    return { get_method, set_method };
}

struct event_type
{
    MethodDef add;
    MethodDef remove;
};

event_type get_event_methods(Event const& evt)
{
    MethodDef add_method{}, remove_method{};

    for (auto&& method_semantic : evt.MethodSemantic())
    {
        auto semantic = method_semantic.Semantic();

        if (semantic.AddOn())
        {
            add_method = method_semantic.Method();
        }
        else if (semantic.RemoveOn())
        {
            remove_method = method_semantic.Method();
        }
        else
        {
            throw_invalid("Events can only have add and remove methods");
        }
    }

    XLANG_ASSERT(add_method);
    XLANG_ASSERT(remove_method);
    XLANG_ASSERT(add_method.Flags().Static() == remove_method.Flags().Static());

    return { add_method, remove_method };
}

auto get_constructors(TypeDef const& type)
{
    std::vector<MethodDef> constructors{};

    for (auto&& method : type.MethodList())
    {
        if (method.Flags().RTSpecialName() && method.Name() == ".ctor")
        {
            constructors.push_back(method);
        }
    }

    return std::move(constructors);
}

auto get_properties(TypeDef const& type, bool static_props)
{
    std::vector<Property> properties{};

    for (auto&& prop : type.PropertyList())
    {
        auto prop_methods = get_property_methods(prop);
        if (prop_methods.get.Flags().Static() == static_props)
        {
            properties.push_back(prop);
        }
    }

    return std::move(properties);
}

auto get_instance_properties(TypeDef const& type)
{
    return get_properties(type, false);
}

auto get_static_properties(TypeDef const& type)
{
    return get_properties(type, true);
}

auto get_events(TypeDef const& type, bool static_events)
{
    std::vector<Event> events{};

    for (auto&& event : type.EventList())
    {
        auto event_methods = get_event_methods(event);

        if (event_methods.add.Flags().Static() == static_events)
        {
            events.push_back(event);
        }
    }

    return std::move(events);
}

auto get_instance_events(TypeDef const& type)
{
    return get_events(type, false);
}

auto get_static_events(TypeDef const& type)
{
    return get_events(type, true);
}

auto get_methods(TypeDef const& type)
{
    std::vector<MethodDef> methods{};

    for (auto&& method : type.MethodList())
    {
        if (!(method.SpecialName() || method.Flags().RTSpecialName()))
        {
            methods.push_back(method);
        }
    }

    // TODO: static events
    return std::move(methods);
}

bool has_methods(TypeDef const& type)
{
    return get_methods(type).size() > 0 || get_static_properties(type).size() > 0 || get_static_events(type).size() > 0;
}

bool has_getsets(TypeDef const& type)
{
    return get_instance_properties(type).size() > 0 || get_instance_events(type).size() > 0;
}

bool has_finalizer(TypeDef const& type)
{
    auto category = get_category(type);
    return category == category::interface_type || (category == category::class_type && !type.Flags().Abstract());
}

enum class param_category
{
    in,
    out,
    pass_array,
    fill_array,
    receive_array,
};

auto get_param_category(method_signature::param_t const& param)
{
    if (param.second->Type().is_szarray())
    {
        if (param.first.Flags().In())
        {
            return param_category::pass_array;
        }
        else if (param.second->ByRef())
        {
            XLANG_ASSERT(param.first.Flags().Out());
            return param_category::fill_array;
        }
        else
        {
            XLANG_ASSERT(param.first.Flags().Out());
            return param_category::receive_array;
        }
    }
    else
    {
        if (param.first.Flags().In())
        {
            XLANG_ASSERT(!param.first.Flags().Out());
            return param_category::in;
        }
        else
        {
            XLANG_ASSERT(param.first.Flags().Out());
            return param_category::out;
        }
    }
}

bool is_in_param(method_signature::param_t const& param)
{
    auto category = get_param_category(param);

    if (category == param_category::fill_array)
    {
        throw_invalid("fill aray param not impl");
    }

    return (category == param_category::in || category == param_category::pass_array);
}

int count_in_param(std::vector<method_signature::param_t> const& params)
{
    int count{ 0 };

    for (auto&& param : params)
    {
        if (is_in_param(param))
        {
            count++;
        }
    }

    return count;
}

int count_out_param(std::vector<method_signature::param_t> const& params)
{
    int count{ 0 };

    for (auto&& param : params)
    {
        if (!is_in_param(param))
        {
            count++;
        }
    }

    return count;
}

// ----- Python code generation --------------------

struct writer : writer_base<writer>
{
    using writer_base<writer>::write;

#pragma region generic param handling
    std::vector<std::vector<std::string>> generic_param_stack;

    struct generic_param_guard
    {
        explicit generic_param_guard(writer* arg = nullptr)
            : owner(arg)
        {}

        ~generic_param_guard()
        {
            if (owner)
            {
                owner->generic_param_stack.pop_back();
            }
        }

        generic_param_guard(generic_param_guard && other)
            : owner(other.owner)
        {
            owner = nullptr;
        }

        generic_param_guard& operator=(generic_param_guard&& other)
        {
            owner = std::exchange(other.owner, nullptr);
            return *this;
        }

        generic_param_guard& operator=(generic_param_guard const&) = delete;
        writer* owner;
    };

    [[nodiscard]] auto push_generic_params(std::pair<GenericParam, GenericParam>&& params)
    {
        if (empty(params))
        {
            return generic_param_guard{ nullptr };
        }

        std::vector<std::string> names;

        for (auto&& param : params)
        {
            names.push_back(std::string{ param.Name() });
        }

        generic_param_stack.push_back(std::move(names));
        return generic_param_guard{ this };
    }

    [[nodiscard]] auto push_generic_params(GenericTypeInstSig const& signature)
    {
        std::vector<std::string> names;

        for (auto&& arg : signature.GenericArgs())
        {
            names.push_back(write_temp("%", arg));
        }

        generic_param_stack.push_back(std::move(names));
        return generic_param_guard{ this };
    }
#pragma endregion

    void write_value(bool value)
    {
        write(value ? "TRUE"sv : "FALSE"sv);
    }

    void write_value(char16_t value)
    {
        write_printf("%#0hx", value);
    }

    void write_value(int8_t value)
    {
        write_printf("%hhd", value);
    }

    void write_value(uint8_t value)
    {
        write_printf("%#0hhx", value);
    }

    void write_value(int16_t value)
    {
        write_printf("%hd", value);
    }

    void write_value(uint16_t value)
    {
        write_printf("%#0hx", value);
    }

    void write_value(int32_t value)
    {
        write_printf("%d", value);
    }

    void write_value(uint32_t value)
    {
        write_printf("%#0x", value);
    }

    void write_value(int64_t value)
    {
        write_printf("%lld", value);
    }

    void write_value(uint64_t value)
    {
        write_printf("%#0llx", value);
    }

    void write_value(float value)
    {
        write_printf("%f", value);
    }

    void write_value(double value)
    {
        write_printf("%f", value);
    }

    void write_value(std::string_view value)
    {
        write("\"%\"", value);
    }

    void write(Constant const& value)
    {
        switch (value.Type())
        {
        case ConstantType::Boolean:
            write_value(value.ValueBoolean());
            break;
        case ConstantType::Char:
            write_value(value.ValueChar());
            break;
        case ConstantType::Int8:
            write_value(value.ValueInt8());
            break;
        case ConstantType::UInt8:
            write_value(value.ValueUInt8());
            break;
        case ConstantType::Int16:
            write_value(value.ValueInt16());
            break;
        case ConstantType::UInt16:
            write_value(value.ValueUInt16());
            break;
        case ConstantType::Int32:
            write_value(value.ValueInt32());
            break;
        case ConstantType::UInt32:
            write_value(value.ValueUInt32());
            break;
        case ConstantType::Int64:
            write_value(value.ValueInt64());
            break;
        case ConstantType::UInt64:
            write_value(value.ValueUInt64());
            break;
        case ConstantType::Float32:
            write_value(value.ValueFloat32());
            break;
        case ConstantType::Float64:
            write_value(value.ValueFloat64());
            break;
        case ConstantType::String:
            write_value(value.ValueString());
            break;
        case ConstantType::Class:
            write("null");
            break;
        }
    }

    void write_code(std::string_view const& value)
    {
        for (auto&& c : value)
        {
            if (c == '.')
            {
                write("::");
            }
            else if (c == '`')
            {
                return;
            }
            else
            {
                write(c);
            }
        }
    }

    void write(TypeRef const& type)
    {
        auto ns = type.TypeNamespace();
        auto name = type.TypeName();

        if (name == "Guid" && ns == "System")
        {
            write("winrt::guid");
        }
        else
        {
            write(find_required(type));
        }
    }

    void write(TypeDef const& type)
    {
        write("winrt::@::@", type.TypeNamespace(), type.TypeName());
    }

    void write(coded_index<TypeDefOrRef> const& type)
    {
        switch (type.type())
        {
        case TypeDefOrRef::TypeDef:
            write(type.TypeDef());
            break;

        case TypeDefOrRef::TypeRef:
            write(type.TypeRef());
            break;

        case TypeDefOrRef::TypeSpec:
            write(type.TypeSpec().Signature().GenericTypeInst());
            break;
        }
    }

    void write(GenericTypeInstSig const& type)
    {
        write("%<%>", type.GenericType(), bind_list(", ", type.GenericArgs()));
    }

    void write(ElementType type)
    {
        switch (type)
        {
        case ElementType::Boolean:
            write("bool");
            break;
        case ElementType::Char:
            write("wchar_t");
            break;
        case ElementType::I1:
            write("int8_t");
            break;
        case ElementType::U1:
            write("uint8_t");
            break;
        case ElementType::I2:
            write("int16_t");
            break;
        case ElementType::U2:
            write("uint16_t");
            break;
        case ElementType::I4:
            write("int32_t");
            break;
        case ElementType::U4:
            write("uint32_t");
            break;
        case ElementType::I8:
            write("int64_t");
            break;
        case ElementType::U8:
            write("uint64_t");
            break;
        case ElementType::R4:
            write("float");
            break;
        case ElementType::R8:
            write("double");
            break;
        case ElementType::String:
            write("winrt::hstring");
            break;
        case ElementType::Object:
            write("winrt::Windows::Foundation::IInspectable");
            break;
        default:
            throw_invalid("write_method_comment_type element type not impl");
        }
    }

    void write(GenericTypeIndex /*var*/)
    {
        throw_invalid("write GenericTypeIndex not impl");
    }

    void write(TypeSig const& signature)
    {
        visit(signature.Type(),
            [&](auto&& type)
        {
            write(type);
        });
    }

    void write_header_comment()
    {
        write(R"(// WARNING: Please don't edit this file. It was generated by Python/WinRT

)");
    }
};

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

        w.write(R"(if (arg_count == %)
    {
)", count_in_param(signature.params()));
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

struct separator
{
    writer& w;
    bool first{ true };

    void operator()()
    {
        if (first)
        {
            first = false;
        }
        else
        {
            w.write(", ");
        }
    }
};

struct convert_to_python : signature_handler_base<convert_to_python>
{
    using signature_handler_base<convert_to_python>::handle;

    writer& w;

    convert_to_python(writer& wr) : w(wr)
    {
    }

    void handle_class(TypeDef const& /*type*/)
    {
        w.write("py::wrap");
    }

    void handle_interface(TypeDef const& /*type*/)
    {
        w.write("py::wrap");
    }

    void handle_enum(TypeDef const& /*type*/)
    {
        w.write("py::convert_enum_to");
    }

    void handle_struct(TypeDef const& /*type*/)
    {
        // TODO: write convert_to function for structs
        w.write("nullptr; /* convert_to_python handle_struct */ //");
    }

    void handle_guid(TypeRef const& /*type*/)
    {
        throw_invalid("convert_to_python handle_guid not implemented");
    }

    void handle(GenericTypeInstSig const& /*type*/)
    {
        w.write("py::wrap");
    }

    void handle(ElementType type)
    {
        if (type >= ElementType::Boolean && type <= ElementType::String)
        {
            w.write("py::convert_primitive_to");
        }
        else if (type >= ElementType::Object)
        {
            w.write("py::wrap");
        }
        else
        {
            throw_invalid("convert_to_python::handle(ElementType");
        }
    }

    static void write_typedef(writer& w, TypeDef const& type)
    {
        convert_to_python ctw{ w };
        ctw.handle(type);
    }

    static void write_typesig(writer& w, TypeSig const& sig)
    {
        convert_to_python ctw{ w };
        ctw.handle(sig);
    }

    static void write_param(writer& w, method_signature::param_t const& param)
    {
        if (get_param_category(param) == param_category::receive_array)
        {
            w.write("nullptr; // receive_array convert_to not impl");
        }
        else
        {
            write_typesig(w, param.second->Type());
        }
    }
};

void write_param_name(writer& w, method_signature::param_t param)
{
    w.write("param%", param.first.Sequence() - 1);
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

        case TypeDefOrRef::TypeSpec:
            w.write("nullptr");
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
        w.write("            auto param% = py::convert_from<%>(args, %);\n", sequence, param.second->Type(), sequence);
        break;
    case param_category::out:
        w.write("            % param% { % };\n", param.second->Type(), sequence, bind<write_out_param_init>(param));
        break;
    case param_category::pass_array:
        w.write("            /*p*/ winrt::array_view<% const> param% {}; //= py::convert_from<winrt::array_view<% const>>(args, %);\n", param.second->Type(), sequence, param.second->Type(), sequence);
        break;
    case param_category::receive_array:        w.write("            /*r*/ winrt::array_view<%> param% { };\n", param.second->Type(), sequence);
        break;
    default:
        throw_invalid("write_param_conversion not impl");
    }

}

void write_class_method_overload_return(writer& w, method_signature const& signature)
{
    if (signature.return_signature())
    {
        w.write("auto return_value = ");
    }
}

void write_class_method_overload_invoke_context(writer& w, MethodDef const& method)
{
    if (method.Flags().Static())
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
            w.write("\n            return %(return_value);\n", bind<convert_to_python::write_typesig>(signature.return_signature().Type()));
        }
        else
        {
            w.write(R"(
            PyObject* out_return_value = %(return_value);
            if (!out_return_value) 
            { 
                return nullptr;
            };

)", bind<convert_to_python::write_typesig>(signature.return_signature().Type()));

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

                w.write(R"(            PyObject* out% = %(param%);
            if (!out%) 
            {
                return nullptr;
            }

)", sequence, bind<convert_to_python::write_param>(param), sequence, sequence);
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

void write_winrt_wrapper(writer& w, TypeDef const& type)
{
    w.write("py::winrt_wrapper<%>", type);
}

void write_class_method_self_param(writer& w, MethodDef const& method)
{
    if (method.Flags().Static())
    {
        w.write("PyObject* /*unused*/");
    }
    else
    {
        w.write("%* self", bind<write_winrt_wrapper>(method.Parent()));
    }
}

void write_class_method(writer& w, TypeDef const& type, std::string_view const& method_name)
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

    w.write("\nstatic PyObject* %_%(%, PyObject* args)\n{ \n", type.TypeName(), method_name, bind<write_class_method_self_param>(methods[0]));
    bind_method_overloads<write_class_method_overload>(w, methods);
    w.write("}\n");
}

void write_class_methods(writer& w, TypeDef const& type)
{
    // I believe it is a hard rule of WinRT that you can't have instance and static members with the same name
    // email out to Larry Osterman to verify

    std::set<std::string_view> methods{};

    for (auto&& method : get_methods(type))
    {
        if (methods.find(method.Name()) == methods.end())
        {
            methods.emplace(method.Name());
            write_class_method(w, type, method.Name());
        }
    }

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

    w.write(R"(            % instance{ % };
            return %(instance);
        }
        catch (...)
        {
            return py::to_PyErr();
        }
)", method.Parent(), bind_list<write_param_name>(", ", signature.params()), bind<convert_to_python::write_typedef>(method.Parent()));
}

void write_interface_constructor(writer& w, TypeDef const& type)
{
    XLANG_ASSERT(get_category(type) == category::interface_type);

    w.write(R"(PyObject* %_constructor(PyTypeObject* type, PyObject* args, PyObject* kwds)
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
            auto param0 = py::convert_from<%>(args, 0);
            return py::wrap(param0);
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
)", type.TypeName(), type);
}

void write_class_constructor(writer& w, TypeDef const& type)
{
    if (get_category(type) == category::interface_type)
    {
        write_interface_constructor(w, type);
        return;
    }

    w.write("PyObject* %_constructor(PyTypeObject* type, PyObject* args, PyObject* kwds)\n{\n", type.TypeName());

    auto constructors = get_constructors(type);
    if (type.Flags().Abstract() || constructors.size() == 0)
    {
        w.write(R"(    PyErr_SetString(PyExc_RuntimeError, "% is not activatable");
    return nullptr;
)", type.TypeName());
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
    auto property_methods = get_property_methods(prop);

    w.write(R"(
static PyObject* %_%(%* self, void* /*unused*/)
{
    try
    {
        auto return_value = self->obj.%();
        return %(return_value);
    }
    catch (...)
    {
        return py::to_PyErr();
    }
}
)", prop.Parent().TypeName(), property_methods.get.Name(), bind<write_winrt_wrapper>(prop.Parent()), prop.Name(), bind<convert_to_python::write_typesig>(prop.Type().Type()));

    if (property_methods.set)
    {
        w.write(R"(
static int %_%(%* self, PyObject* value, void* /*unused*/)
{
    if (value == nullptr)
    {
        PyErr_SetString(PyExc_RuntimeError, "property delete not supported");
        return -1;
    }
    
    try
    {
        auto param0 = py::convert_from<%>(value);
        auto return_value = self->obj.%();
        return 0;
    }
    catch (...)
    {
        return -1;
    }
}
)", prop.Parent().TypeName(), property_methods.set.Name(), bind<write_winrt_wrapper>(prop.Parent()), prop.Type().Type(), prop.Name());
    }
}

void write_class_properties(writer& w, TypeDef const& type)
{
    w.write_each<write_class_property>(get_instance_properties(type));
}

void write_class_event(writer& w, Event const& event)
{
    w.write(R"(
static PyObject* %_event_%(%* self, void* /*unused*/)
{
    try
    {
        // TODO: event handling
        throw winrt::hresult_not_implemented();
    }
    catch (...)
    {
        return py::to_PyErr();
    }
}
)", event.Parent().TypeName(), event.Name(), bind<write_winrt_wrapper>(event.Parent()));

}

void write_class_events(writer& w, TypeDef const& type)
{
    w.write_each<write_class_event>(get_instance_events(type));
}

void write_class_property_table_row_getter(writer& w, Property const& prop, property_type const& property_methods)
{
    w.write("(getter)%_%", prop.Parent().TypeName(), property_methods.get.Name());
}

void write_class_property_table_row_setter(writer& w, Property const& prop, property_type const& property_methods)
{
    if (property_methods.set)
    {
        w.write("(setter)%_%", prop.Parent().TypeName(), property_methods.set.Name());
    }
    else
    {
        w.write("nullptr");
    }
}

void write_class_getset_table_row_prop(writer& w, Property const& prop)
{
    auto property_methods = get_property_methods(prop);

    // TODO: remove const_cast once pywinrt is updated to target Python 3.7. 
    //       pywinrt currently targeting 3.6 because that's the version that ships with VS 2017 v15.8
    //       https://github.com/python/cpython/commit/007d7ff73f4e6e65cfafd512f9c23f7b7119b803
    w.write("    { const_cast<char*>(\"%\"), %, %, nullptr, nullptr },\n", prop.Name(),
        bind<write_class_property_table_row_getter>(prop, property_methods),
        bind<write_class_property_table_row_setter>(prop, property_methods));
}

void write_class_getset_table_row_event(writer& w, Event const& event)
{
    // TODO: remove const_cast once pywinrt is updated to target Python 3.7. 
    //       pywinrt currently targeting 3.6 because that's the version that ships with VS 2017 v15.8
    //       https://github.com/python/cpython/commit/007d7ff73f4e6e65cfafd512f9c23f7b7119b803
    w.write("    { const_cast<char*>(\"%\"), (getter)%_event_%, nullptr, nullptr, nullptr },\n", event.Name(),
        event.Parent().TypeName(), event.Name());
}

void write_class_property_table(writer& w, TypeDef const& type)
{
    if (has_getsets(type))
    {
        w.write("\nstatic PyGetSetDef %_properties[] = {\n", type.TypeName());

        w.write_each<write_class_getset_table_row_prop>(get_instance_properties(type));
        w.write_each<write_class_getset_table_row_event>(get_instance_events(type));

        w.write("    { nullptr }\n};\n");
    }
}

void write_class_method_table(writer& w, TypeDef const& type)
{
    if (has_methods(type))
    {
        std::set<std::string_view> instance_methods{};
        std::set<std::string_view> static_methods{};

        w.write("\nstatic PyMethodDef %_methods[] = {\n", type.TypeName());

        for (auto&& method : get_methods(type))
        {
            auto is_static_method = method.Flags().Static();
            auto& set = is_static_method ? static_methods : instance_methods;

            if (set.find(method.Name()) == set.end())
            {
                set.emplace(method.Name());

                w.write("    { \"%\", (PyCFunction)%_%, METH_VARARGS%, nullptr },\n",
                    method.Name(), type.TypeName(), method.Name(), is_static_method ? " | METH_STATIC" : "");
            }
        }

        for (auto&& prop : get_static_properties(type))
        {
            w.write("    { \"%\", (PyCFunction)%_%, METH_VARARGS | METH_STATIC, nullptr },\n",
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
    w.write("\nstatic PyType_Slot %_Type_slots[] = {\n", type.TypeName());
    w.write("    { Py_tp_new, %_constructor },\n", type.TypeName());
    if (has_finalizer(type))
    {
        w.write("    { Py_tp_finalize, py::finalize<%> },\n", type);
    }

    if (has_methods(type))
    {
        w.write("    { Py_tp_methods, %_methods },\n", type.TypeName());
    }

    if (has_getsets(type))
    {
        w.write("    { Py_tp_getset, %_properties },\n", type.TypeName());
    }

    if (distance(type.FieldList()) > 0)
    {
        w.write("    { Py_tp_members, %_members },\n", type.TypeName());
    }

    w.write("    { 0, nullptr },\n};\n");
}

void write_type_spec_size(writer& w, TypeDef const& type)
{
    auto category = get_category(type);

    if (category == category::class_type || category == category::interface_type)
    {
        if (type.Flags().Abstract())
        {
            w.write("0");
        }
        else
        {
            w.write("sizeof(%)", bind<write_winrt_wrapper>(type));
        }
    }

    if (category == category::struct_type)
    {
        w.write("sizeof(%)", type.TypeName());
    }
}

void write_type_spec(writer& w, TypeDef const& type)
{
    w.write(R"(
static PyType_Spec %_Type_spec = {
    "%",
    %,
    0,
    Py_TPFLAGS_DEFAULT,
    %_Type_slots
};
)", type.TypeName(), type.TypeName(), bind<write_type_spec_size>(type), type.TypeName());
}

void write_class(writer& w, TypeDef const& type)
{
    if (is_exclusive_to(type))
    {
        return;
    }

    auto guard{ w.push_generic_params(type.GenericParam()) };

    w.write("\n// ----- % --------------------\n", type.TypeName());

    w.write("PyObject* py::winrt_type<%>::python_type;\n\n", type);
    write_class_constructor(w, type);
    write_class_methods(w, type);
    write_class_properties(w, type);
    write_class_events(w, type);
    write_class_method_table(w, type);
    write_class_property_table(w, type);
    write_type_slot_table(w, type);
    write_type_spec(w, type);
}

void write_struct_constructor(writer& w, TypeDef const& type)
{
    w.write(R"(
PyObject* %_constructor(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    if (kwds != nullptr)
    {
        PyErr_SetString(PyExc_RuntimeError, "keyword arguments not supported");
        return nullptr;
    }

    Py_ssize_t arg_count = PyTuple_Size(args);

    if (arg_count == 0)
    {
        auto py_instance = PyObject_New(%, reinterpret_cast<PyTypeObject*>(py::winrt_type<%>::python_type));

        if (!py_instance)
        {
            return nullptr;
        }

        // PyObject_New doesn't call type's constructor, so manually initialize the contained struct
        std::memset(&(py_instance->value), 0, sizeof(%));
        return reinterpret_cast<PyObject*>(py_instance);
    }
    else if (arg_count == -1)
    {
        return nullptr; 
    }

    PyErr_SetString(PyExc_RuntimeError, "Invalid parameter count");
    return nullptr;

}
)", type.TypeName(), type.TypeName(), type, type);

    //w
    //

    //w.write("}\n");
}

void write_struct_type(writer& w, FieldSig const& signature)
{
    auto process_typedef = [&](TypeDef const& type)
    {
        auto category = get_category(type);
        if (category == category::enum_type)
        {
            if (is_flags_enum(type))
            {
                w.write("T_UINT");
            }
            else
            {
                w.write("T_INT");
            }
        }
        else if (category == category::struct_type)
        {
            throw_invalid("struct hierarchy not impl");
        }
        else
        {
            throw_invalid("invalid struct member");
        }
    };

    return visit(signature.Type().Type(),
        [&](coded_index<TypeDefOrRef> const& index)
    {
        switch (index.type())
        {
        case TypeDefOrRef::TypeDef:
            return process_typedef(index.TypeDef());
        case TypeDefOrRef::TypeRef:
        {
            auto tr = index.TypeRef();
            if (tr.TypeNamespace() == "System" & tr.TypeName() == "Guid")
            {
                throw_invalid("struct guid not impl");
            }
            if (tr.TypeNamespace() == "Windows.Foundation")
            {
                if (tr.TypeName() == "DateTime" || tr.TypeName() == "TimeSpan")
                {
                    w.write("T_LONGLONG");
                    return;
                }
            }
            return process_typedef(find(tr));
        }
        case TypeDefOrRef::TypeSpec:
            throw_invalid("struct TypeSpec not supported");
        }
    },
        [&](ElementType type)
    {
        switch (type)
        {
        case ElementType::I2:
            w.write("T_SHORT");
            break;
        case ElementType::U2:
            w.write("T_USHORT");
            break;
        case ElementType::I4:
            w.write("T_INT");
            break;
        case ElementType::U4:
            w.write("T_UINT");
            break;
        case ElementType::I8:
            w.write("T_LONGLONG");
            break;
        case ElementType::U8:
            w.write("T_ULONGLONG");
            break;
        case ElementType::R4:
            w.write("T_FLOAT");
            break;
        case ElementType::R8:
            w.write("T_DOUBLE");
            break;
        default:
            throw_invalid("struct element type not impl");
        }
    },
        [&](GenericTypeInstSig const&) { throw_invalid("struct GenericTypeInstSig not supported"); },
        [&](GenericTypeIndex) { throw_invalid("struct GenericTypeIndex not supported"); });
}

void write_struct(writer& w, TypeDef const& type)
{
    w.write("\n// ----- % --------------------\n", type.TypeName());
    w.write("PyObject* py::winrt_type<%>::python_type;\n", type);

    w.write(R"(
struct %
{
    PyObject_HEAD
    % value{};
};
)", type.TypeName(), type);

    write_struct_constructor(w, type);

    w.write(R"(
static PyMemberDef %_members[] = {
)", type.TypeName());

    // TODO: remove const_cast once pywinrt is updated to target Python 3.7. 
    //       pywinrt currently targeting 3.6 because that's the version that ships with VS 2017 v15.8
    //       https://github.com/python/cpython/commit/007d7ff73f4e6e65cfafd512f9c23f7b7119b803

    for (auto&& field : type.FieldList())
    {
        w.write("    { const_cast<char*>(\"%\"), %, offsetof(%, value.%), 0, nullptr },\n", field.Name(), bind<write_struct_type>(field.Signature()), type.TypeName(), field.Name());
    }

    w.write("    { nullptr },\n};\n");

    write_type_slot_table(w, type);
    write_type_spec(w, type);
}

void write_delegate(writer&, TypeDef const&)
{
    throw_invalid("projecting delegates not impl");
}

void write_module_exec_type_init(writer& w, TypeDef const& type)
{
    if (is_exclusive_to(type))
    {
        return;
    }

    w.write(R"(    py::winrt_type<%>::python_type = PyType_FromSpec(&%_Type_spec);
    if (py::winrt_type<%>::python_type == nullptr)
    {
        return -1;
    }
    if (PyModule_AddObject(module, "%", py::winrt_type<%>::python_type) != 0)
    {
        return -1;
    }

)", type, type.TypeName(), type, type.TypeName(), type);
}

void write_ns_init_function_name(writer& w, std::string_view const& ns)
{
    auto ns_segments = get_dotted_name_segments(ns);
    w.write("initialize_%", bind_list("_", ns_segments));
}

void write_ns_init(writer& w, std::string_view const& ns, cache::namespace_members const& ns_members)
{
    w.write(R"(
int %(PyObject* module)
{
)", bind<write_ns_init_function_name>(ns));

    w.write_each<write_module_exec_type_init>(ns_members.classes);

    for (auto&& type : ns_members.interfaces)
    {
        if (is_exclusive_to(type))
        {
            continue;
        }
        write_module_exec_type_init(w, type);
    }

    for (auto&& type : ns_members.structs)
    {
        write_module_exec_type_init(w, type);
    }

    w.write(R"(    return 0;
}
)");
}

void write_module_exec(writer& w, std::vector<std::string> const& namespaces)
{
    w.write(R"(
static int module_exec(PyObject* module)
{
)");

    for (auto&& ns : namespaces)
    {
        w.write(R"(    if (%(module) != 0)
    {
        return -1;
    }

)", bind<write_ns_init_function_name>(ns));
    }

    w.write(R"(    return 0;
}
)");
}

void write_module_init(writer& w, std::string_view const& module_name)
{
    w.write(R"(
PyDoc_STRVAR(module_doc, "Langworthy projection module.\n");

static PyModuleDef_Slot module_slots[] = {
    {Py_mod_exec, module_exec},
    {0, nullptr}
};

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

PyMODINIT_FUNC
PyInit_%(void)
{
    return PyModuleDef_Init(&module_def);
}
)", module_name, module_name);
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

auto get_output(cmd::reader const& args)
{
    auto output{ absolute(args.value("output", "output")) };
    create_directories(output);
    return output;
}

void print_usage()
{
    puts("Usage...");
}

int main(int const argc, char** argv)
{
    writer wc;

    try
    {
        auto start = high_resolution_clock::now();

        std::vector<cmd::option> options
        {
            // name, min, max
            { "input", 1 },
            { "output", 0, 1 },
            { "include", 0 },
            { "exclude", 0 },
            { "verbose", 0, 0 },
        };

        cmd::reader args{ argc, argv, options };

        if (!args)
        {
            print_usage();
            return 0;
        }

        cache c{ args.values("input") };
        auto const output = get_output(args);
        bool const verbose = args.exists("verbose");

        filter f(args.values("include"), args.values("exclude"));

        if (verbose)
        {
            for (auto&& db : c.databases())
            {
                wc.write("input: %\n", db.path());
            }

            wc.write("output: %\n", output.string());
        }

        wc.flush_to_console();

        std::string python_module_name{ "xlang" };
        std::string native_module_name{ "_" + python_module_name };

        std::vector<std::string> generated_namespaces{};
        task_group group;

        group.add([&]
        {
            writer w;
            w.write(strings::base_py);
            w.flush_to_file(output / "pybase.h");
        });

        for (auto&& ns : c.namespaces())
        {
            if (!f.includes(ns.second))
            {
                continue;
            }

            std::string fqns{ ns.first };
            auto h_filename = "py." + fqns + ".h";

            generated_namespaces.emplace_back(ns.first);

            group.add([&]
            {
                writer w;

                auto ns_segments = get_dotted_name_segments(ns.first);

                w.write_header_comment();
                w.write(R"(#include <pybase.h>
#include <winrt/%.h>

int %(PyObject* module);
)", ns.first, bind<write_ns_init_function_name>(ns.first));
                w.flush_to_file(output / h_filename);
            });

            group.add([&]
            {
                writer w;
                w.debug_trace = IsDebuggerPresent() != 0;

                w.write_header_comment();
                w.write("#include \"%\"\n", h_filename);

                w.write("%%%%",
                    f.bind_each<write_class>(ns.second.classes),
                    f.bind_each<write_class>(ns.second.interfaces),
                    f.bind_each<write_struct>(ns.second.structs),
                    f.bind_each<write_delegate>(ns.second.delegates));

                w.write("\n// ----- % Initialization --------------------\n", ns.first);
                w.write("\nint %(PyObject* module)\n{\n", bind<write_ns_init_function_name>(ns.first));
                w.write("%%%",
                    f.bind_each<write_module_exec_type_init>(ns.second.classes),
                    f.bind_each<write_module_exec_type_init>(ns.second.interfaces),
                    f.bind_each<write_module_exec_type_init>(ns.second.structs));
                w.write("    return 0;\n}\n");

                w.flush_to_file(output / ("py." + fqns + ".cpp"));
            });
        }

        group.get();

        {
            writer w;
            w.write_header_comment();

            for (auto&& ns : generated_namespaces)
            {
                w.write("#include \"py.%.h\"\n", ns);
            }

            w.write(strings::module_methods);
            write_module_exec(w, generated_namespaces);
            write_module_init(w, native_module_name);

            w.flush_to_file(output / (native_module_name + ".cpp"));
        }

        {
            writer w;

            w.write(strings::setup, python_module_name, native_module_name, bind<write_setup_filenames>(native_module_name, generated_namespaces));
            w.flush_to_file(output / "setup.py");
        }

        if (verbose)
        {
            wc.write("time: %ms\n", duration_cast<duration<int64_t, std::milli>>(high_resolution_clock::now() - start).count());
        }
    }
    catch (std::exception const& e)
    {
        wc.write("%\n", e.what());
        wc.flush_to_console();
        getchar();
        return -1;
    }

    wc.flush_to_console();
}
