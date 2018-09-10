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
    void handle_class(TypeDef const& type) {}
    void handle_delegate(TypeDef const& type) {}
    void handle_enum(TypeDef const& type) {}
    void handle_guid(TypeRef const& type) {}
    void handle_interface(TypeDef const& type) {}
    void handle_struct(TypeDef const& type) {}

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

    void handle(GenericTypeInstSig const& type) {}

    void handle(ElementType type) {}

    void handle(GenericTypeIndex var) { }

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

// ----- Python projection specific metadata helpers --------------------

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
    
    return constructors;
}

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

    if (!get_method)
    {
        throw_invalid("Properties must have a get method");
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

    if (!add_method || !remove_method)
    {
        throw_invalid("Properties must have add and remove methods");
    }

    return { add_method, remove_method };
}

bool has_methods(TypeDef const& type)
{
    for (auto&& method : type.MethodList())
    {
        if (!method.SpecialName() && !method.Flags().RTSpecialName())
        {
            return true;
        }
    }

    return false;
}

bool has_properties(TypeDef const& type)
{
    return (distance(type.PropertyList()) > 0) || (distance(type.EventList()) > 0);
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

    std::string type_namespace;
    std::string module_name;
    std::set<TypeDef> converters{};

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
        default:
            throw_invalid("write_method_comment_type element type not impl");
        }
    }

    void write(GenericTypeIndex var)
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

    void handle_class(TypeDef const& type)
    {
        w.write("py::wrap");
    }

    void handle_interface(TypeDef const& type)
    {
        w.write("py::wrap");
    }

    void handle_enum(TypeDef const& type)
    {
        w.write("py::convert_enum_to");
    }

    void handle_guid(TypeRef const& type)
    {
        w.write("py::convert_to");
    }

    void handle(GenericTypeInstSig const& type) 
    {
        w.write("nullptr; // py::wrap GenericTypeInstSig TBD");
    }

    void handle(ElementType type)
    {
        if (type >= ElementType::Boolean && type <= ElementType::String)
        {
            w.write("py::convert_to");
        }
        else
        {
            throw_invalid("write ElementType not impl");
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

struct param_init : signature_handler_base<param_init>
{
    using signature_handler_base<param_init>::handle;

    writer& w;
    method_signature::param_t const& param;

    param_init(writer& w, method_signature::param_t const& param) : w(w), param(param)
    {
    }

    auto category()
    {
        return get_param_category(param);
    }

    auto sequence()
    {
        return std::to_string(param.first.Sequence() - 1);
    }

    void handle(ElementType type)
    {
        auto cat = category();

        if (cat == param_category::in && type >= ElementType::Boolean && type <= ElementType::String)
        {
            w.write("{ py::convert_from<%>(args, %) }", type, sequence());
        }
        else if (cat == param_category::out)
        {
        }
        else
        {
            throw_invalid("param_init::handle(ElementType) not impl");
        }
    }

    void handle_class(TypeDef const& type)
    {
        auto cat = category();
        
        if (cat == param_category::in)
        {
            w.write("{ py::unwrap<%>(args, %) }", type, sequence());
        }
        else if (cat == param_category::out)
        {
            w.write("{ nullptr }");
        }
        else
        {
            throw_invalid("param_init::handle_class category not impl");
        }
    }

    void handle_interface(TypeDef const& type)
    {
        auto cat = category();
        
        if (cat == param_category::in)
        {
            w.write("{ py::unwrap<%>(args, %) }", type, sequence());
        }
        else if (cat == param_category::out)
        {
            w.write("{ nullptr }");
        }
        else if (cat == param_category::pass_array)
        {
            w.write("{ /* % interface pass TBD */ nullptr }", type);
        }
        else
        {
            throw_invalid("param_init::handle_interface category not impl");
        }
    }

    static void write(writer& w, method_signature::param_t const& param)
    {
        param_init pi{ w, param };

        if (pi.category() == param_category::receive_array)
        {
            return;
        }

        pi.handle(param.second->Type());
    }
};

void write_param_name(writer& w, method_signature::param_t param)
{
    w.write("param%", param.first.Sequence() - 1);
}

void write_param_declaration(writer& w, std::pair<Param const&, ParamSig const*> const& param)
{
    std::string param_type;

    switch (get_param_category(param))
    {
    case param_category::in:
        param_type = w.write_temp("/* in */ %", param.second->Type());
        break;
    case param_category::out:
        param_type = w.write_temp("/* out*/ %", param.second->Type());
        break;
    case param_category::pass_array:
        param_type = w.write_temp("/*pass*/ winrt::array_view<% const>", param.second->Type());
        break;
    case param_category::receive_array:
        param_type = w.write_temp("/*recv*/ winrt::array_view<%>", param.second->Type());
        break;
    default:
        throw_invalid("write_param_conversion not impl");
    }

    w.write("            % %%;\n", param_type, bind<write_param_name>(param), bind<param_init::write>(param));
}

void write_class_method_overload(writer& w, MethodDef const& method, method_signature const& signature)
{
    w.write("        try\n        {\n");

    w.write_each<write_param_declaration>(signature.params());

    w.write("\n            ");
    if (signature.return_signature())
    {
        w.write("% return_value = ", signature.return_signature().Type());
    }
    if (method.Flags().Static())
    {
        w.write("%::", method.Parent());
    }
    else
    {
        w.write("self->obj.");
    }
    w.write("%(%);\n", method.Name(), bind_list<write_param_name>(", ", signature.params()));

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

void write_class_method(writer& w, TypeDef const& type, std::string_view const& method_name)
{
    std::vector<MethodDef> methods{};

    for (auto&& method : type.MethodList())
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

    w.write("\nstatic PyObject* %_%(% * %, PyObject* args)\n{\n", type.TypeName(), method_name,
        static_method ? "PyObject" : type.TypeName(), static_method ? "/*unused*/" : "self");
    bind_method_overloads<write_class_method_overload>(w, methods);
    w.write("}\n");
}

void write_class_methods(writer& w, TypeDef const& type)
{
    // I believe it is a hard rule of WinRT that you can't have instance and static members with the same name
    // email out to Larry Osterman to verify

    std::set<std::string_view> methods{};

    if (has_methods(type))
    {
        for (auto&& method : type.MethodList())
        {
            if (method.SpecialName() || method.Flags().RTSpecialName())
            {
                continue;
            }

            if (methods.find(method.Name()) == methods.end())
            {
                methods.emplace(method.Name());
                write_class_method(w, type, method.Name());
            }
        }
    }
}

void write_class_constructor_overload(writer& w, MethodDef const& method, method_signature const& signature)
{
    if (signature.has_params())
    {
        throw_invalid("custom activation not impl");
    }

    w.write(R"(        try
        {
            % instance{ % };
            return %(instance);
        }
        catch (...)
        {
            return py::to_PyErr();
        }
)", method.Parent(), bind_list<write_param_name>(", ", signature.params()), bind<convert_to_python::write_typedef>(method.Parent()));
}

void write_class_constructor(writer& w, TypeDef const& type)
{
    auto const& constructors = get_constructors(type);
    w.write("PyObject* %_constructor(PyTypeObject * type, PyObject* args, PyObject* kwds)\n{\n", type.TypeName());

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
    if (property_methods.get.Flags().Static() || (property_methods.set && property_methods.set.Flags().Static()))
        throw_invalid("write_class_properties static properties not impl");

    auto const& type_name = prop.Parent().TypeName();

    w.write(R"(
static PyObject* %_%(% * self, PyObject* /*unused*/)
{
    try
    {
        % return_value = self->obj.%();
        return %(return_value);
    }
    catch (...)
    {
        return py::to_PyErr();
    }
}
)", type_name, property_methods.get.Name(), type_name, prop.Type().Type(), prop.Name(), bind<convert_to_python::write_typesig>(prop.Type().Type()));

    if (property_methods.set)
    {
        throw_invalid("write_class_properties set_method not impl");

        //w.write("\nstatic PyObject* %_%(% * self, PyObject* args)\n{\n", type_name, set_method.Name(), type_name);
        //write_method_comment(w, set_method);
        //w.write("    return nullptr;\n");
        //w.write("}\n");
    }
}

void write_class_properties(writer& w, TypeDef const& type)
{
    if (distance(type.EventList()) > 0)
    {
        throw_invalid("write_class_properties for events not impl");
    }

    w.write_each<write_class_property>(type.PropertyList());
}

void write_class_property_table_row_prop(writer& w, Property const& prop)
{
    auto property_methods = get_property_methods(prop);

    if (property_methods.get.Flags().Static() || (property_methods.set && property_methods.set.Flags().Static()))
    {
        throw_invalid("write_class_property_table_row_prop static properties not impl");
    }

    auto const& getter = w.write_temp("(getter)%_%", prop.Parent().TypeName(), property_methods.get.Name());
    auto const& setter = property_methods.set
        ? w.write_temp("(setter)%_%", prop.Parent().TypeName(), property_methods.set.Name())
        : "nullptr";

    w.write("    { \"%\", %, %, nullptr, nullptr },\n", prop.Name(), getter, setter);
}

void write_class_property_table_row_event(writer& w, Event const& evt)
{
    throw_invalid("write_class_property_table_row_event not impl");
}

void write_class_property_table(writer& w, TypeDef const& type)
{
    if (has_properties(type))
    {
        w.write("\nstatic PyGetSetDef %_properties[] = {\n", type.TypeName());

        w.write_each<write_class_property_table_row_prop>(type.PropertyList());
        w.write_each<write_class_property_table_row_event>(type.EventList());

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

        for (auto&& method : type.MethodList())
        {
            if (method.SpecialName() || method.Flags().RTSpecialName())
            {
                continue;
            }

            auto static_method = method.Flags().Static();
            auto& set = static_method ? static_methods : instance_methods;

            if (set.find(method.Name()) == set.end())
            {
                set.emplace(method.Name());

                w.write("    { \"%\", (PyCFunction)%_%, METH_VARARGS%, nullptr },\n",
                    method.Name(), type.TypeName(), method.Name(), static_method ? " | METH_STATIC" : "");
            }
        }

        w.write("    { nullptr }\n};\n");
    }
}

void write_class_slot_table(writer& w, TypeDef const& type)
{
    w.write("\nstatic PyType_Slot %_Type_slots[] = {\n", type.TypeName());
    w.write("    { Py_tp_new, %_constructor},\n", type.TypeName());

    if (has_methods(type))
    {
        w.write("    { Py_tp_methods, %_methods},\n", type.TypeName());
    }

    if (has_properties(type))
    {
        w.write("    { Py_tp_getset, %_properties},\n", type.TypeName());
    }

    w.write("    { 0, nullptr },\n};\n");
}

void write_class_type_spec(writer& w, TypeDef const& type)
{
    auto const& basic_size = type.Flags().Abstract() ? "0"s : w.write_temp("sizeof(%)", type.TypeName());

    w.write(R"(
static PyType_Spec %_Type_spec = {
    "%.%",
    %,
    0,
    Py_TPFLAGS_DEFAULT,
    %_Type_slots
};
)", type.TypeName(), w.module_name, type.TypeName(), basic_size, type.TypeName());
}

void write_class_declaration(writer& w, TypeDef const& type)
{
    if (is_exclusive_to(type))
    {
        return;
    }

    w.write("\nstruct %\n{\n", type.TypeName());

    if (!(type.Flags().Abstract() && get_category(type) == category::class_type)) // aka if class is static
    {
        w.write("    PyObject_HEAD\n    % obj{ nullptr };\n", type);
    }

    w.write("    static PyObject* python_type;\n");
    w.write("};\n");
}

void write_class_trait_specialization(writer& w, TypeDef const& type)
{
    if (is_exclusive_to(type))
    {
        return;
    }

    w.write("    template <> struct py_type<%> { using type = %; };\n", type, type.TypeName());
}

void write_class(writer& w, TypeDef const& type)
{
    if (is_exclusive_to(type))
    {
        return;
    }

    auto guard{ w.push_generic_params(type.GenericParam()) };

    w.write("\n// ----- % --------------------\n", type.TypeName());
    w.write("PyObject* %::python_type;\n\n", type.TypeName());
    write_class_constructor(w, type);
    write_class_methods(w, type);
    write_class_properties(w, type);
    write_class_method_table(w, type);
    write_class_property_table(w, type);
    write_class_slot_table(w, type);
    write_class_type_spec(w, type);
}

void write_module_exec_type_init(writer& w, TypeDef const& type)
{
    w.write(R"(    %::python_type = PyType_FromSpec(&%_Type_spec);
    if (%::python_type == nullptr)
    {
        return -1;
    }
    if (PyModule_AddObject(module, "%", %::python_type) != 0)
    {
        return -1;
    }

)", type.TypeName(), type.TypeName(), type.TypeName(), type.TypeName(), type.TypeName());
}

void write_module_exec(writer& w, std::vector<TypeDef> const& classes, std::vector<TypeDef> const& interfaces)
{
    w.write(R"(
static int module_exec(PyObject* module)
{
)");

    w.write_each<write_module_exec_type_init>(classes);

    for (auto&& type : interfaces)
    {
        if (is_exclusive_to(type))
        {
            continue;
        }
        write_module_exec_type_init(w, type);
    }

    w.write(R"(    return 0;
}
)");
}

void write_module_init(writer& w, std::string_view const& module_name)
{
    w.write(R"(
static PyObject* initapartment(PyObject* self, PyObject* args)
{
    winrt::init_apartment();
    Py_RETURN_NONE;
}

static PyObject* uninitapartment(PyObject* self, PyObject* args)
{
    winrt::uninit_apartment();
    Py_RETURN_NONE;
}

PyDoc_STRVAR(module_doc, "Langworthy projection module.\n");

static PyMethodDef module_methods[]{
    { "init_apartment", initapartment, METH_NOARGS, "initialize the apartment" },
    { "uninit_apartment", uninitapartment, METH_NOARGS, "uninitialize the apartment" },
    { nullptr }
}; 

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

void write_setup_filename(writer& w, std::string_view const& filename)
{
    w.write("'%'", filename);
}

auto get_output(cmd::reader const& args)
{
    auto output{ absolute(args.value("output", "output")) };
    create_directories(output);
    output += path::preferred_separator;
    return output.string();
}

void print_usage()
{
    puts("Usage...");
}

int main(int const argc, char** argv)
{
    writer w;

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
        auto const output_folder = get_output(args);
        bool const verbose = args.exists("verbose");

        filter f(args.values("include"), args.values("exclude"));

        if (verbose)
        {
            for (auto&& db : c.databases())
            {
                w.write("input: %\n", db.path());
            }

            w.write("output: %\n", output_folder);
        }

        w.flush_to_console();

        std::string module_name{ "xlang" };
        std::vector<std::string> filenames{};
        task_group group;

        for (auto&& ns : c.namespaces())
        {
            if (!f.includes(ns.second))
            {
                continue;
            }

            std::string filename{ ns.first };
            filename.append(".cpp");
            filenames.emplace_back(filename);

            group.add([&]
            {
                {
                    writer w;
                    w.module_name = "_"s + module_name;
                    w.type_namespace = ns.first;
                    //w.debug_trace = true;

                    w.write_each<write_class>(ns.second.classes);
                    w.write_each<write_class>(ns.second.interfaces);
                    w.write("\n// ----- % Module --------------------\n", w.module_name);
                    write_module_exec(w, ns.second.classes, ns.second.interfaces);
                    write_module_init(w, w.module_name);

                    w.swap();
                    w.write(R"(// WARNING: Please don't edit this file. It was generated by Python/WinRT
#include <Python.h>
#include <winrt/%.h>
)", ns.first);
                    w.write("\n// ----- Python projection base functionallity --------------------\n");
                    w.write(strings::base_py);
                    w.write("\n// ----- Python Object Declarations --------------------\n");
                    w.write_each<write_class_declaration>(ns.second.classes);
                    w.write_each<write_class_declaration>(ns.second.interfaces);
                    w.write("\nnamespace py\n{\n");
                    w.write_each<write_class_trait_specialization>(ns.second.classes);
                    w.write_each<write_class_trait_specialization>(ns.second.interfaces);
                    w.write("}\n");

                    w.flush_to_file(output_folder / filename);
                }
            });
        }

        group.get();

        {
            writer w;
            w.module_name = "_"s + module_name;
            w.write(strings::setup, module_name, w.module_name, bind_list<write_setup_filename>(", ", filenames));
            w.flush_to_file(output_folder / "setup.py");
        }

        if (verbose)
        {
            w.write("time: %ms\n", duration_cast<duration<int64_t, std::milli>>(high_resolution_clock::now() - start).count());
        }
    }
    catch (std::exception const& e)
    {
        w.write("%\n", e.what());
        w.flush_to_console();
        getchar();
        return -1;
    }

    w.flush_to_console();
}
