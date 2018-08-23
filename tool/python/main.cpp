#include "pch.h"
#include "strings.h"

using namespace std::chrono;
using namespace std::experimental::filesystem;
using namespace std::string_view_literals;
using namespace xlang;
using namespace xlang::meta::reader;
using namespace xlang::text;
using namespace xlang::cmd;

auto get_dotted_name_segments(std::string_view ns)
{
    std::vector<std::string_view> segments;
    size_t pos = 0;
    do
    {
        auto new_pos = ns.find('.', pos);
        if (new_pos == std::string_view::npos)
        {
            segments.push_back(ns.substr(pos));
            return segments;
        }

        segments.push_back(ns.substr(pos, new_pos - pos));
        pos = new_pos + 1;
    } while (true);
};

MethodSemantics get_method_semantics(TypeDef const& type, MethodDef const& method)
{
    for (auto const& prop : type.PropertyList())
    {
        for (auto const& semantic : prop.MethodSemantic())
        {
            if (semantic.Method() == method)
            {
                return semantic;
            }
        }
    }

    for (auto const& event : type.EventList())
    {
        for (auto const& semantic : event.MethodSemantic())
        {
            if (semantic.Method() == method)
            {
                return semantic;
            }
        }
    }

    return {};
};

struct method_signature
{
    using param_t = std::pair<Param, ParamSig>;

    explicit method_signature(MethodDef const& method) :
        m_method(method.Signature()),
        m_params(method.ParamList())
    {
        if (m_method.ReturnType() && m_params.first != m_params.second && m_params.first.Sequence() == 0)
        {
            m_return = m_params.first;
            ++m_params.first;
        }

        XLANG_ASSERT(meta::reader::size(m_params) == m_method.Params().size());
    }

    std::vector<param_t> params() const noexcept
    {
        std::vector<param_t> _params{};

        for (uint32_t i{}; i != m_method.Params().size(); ++i)
        {
            _params.emplace_back(m_params.first + i, m_method.Params()[i]);
        }

        return _params;
    }

    auto const& return_signature() const noexcept
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
            name = "generic_return_param_name";
        }

        return name;
    }

    bool has_params() const noexcept
    {
        return m_method.Params().size();
    }

private:

    MethodDefSig m_method;
    std::pair<Param, Param> m_params;
    Param m_return;
};

bool is_exclusive_to(TypeDef const& type)
{
    return bool{ get_attribute(type, "Windows.Foundation.Metadata", "ExclusiveToAttribute") };
}

template <typename...T> struct overloaded : T... { using T::operator()...; };
template <typename...T> overloaded(T...)->overloaded<T...>;

struct writer : writer_base<writer>
{
    using writer_base<writer>::write;

    std::string type_namespace;
    int32_t indent{ 0 };

    struct indent_guard
    {
        explicit indent_guard(writer& w) noexcept : _writer(w)
        {
            _writer.indent++;
        }

        ~indent_guard()
        {
            _writer.indent--;
        }

    private:
        writer & _writer;
    };

    struct bool_guard
    {
        explicit bool_guard(bool& value) noexcept : _value(value)
        {
            _original_value = value;
            value = true;
        }

        ~bool_guard()
        {
            _value = _original_value;
        }

    private:
        bool & _value;
        bool _original_value;
    };

    template <typename... Args>
    void write_indented(std::string_view const& value, Args const&... args)
    {
        if (indent == 0)
        {
            write(value, args...);
        }
        else
        {
            auto indentation = std::string(indent * 4, ' ');
            if (back() == '\n')
            {
                write(indentation);
            }

            auto pos = value.find_last_of('\n', value.size() - 2);
            if (pos == std::string_view::npos)
            {
                write(value, args...);
            }
            else
            {
                std::string value_{ value };

                while (pos != std::string::npos)
                {
                    value_.insert(pos + 1, indentation);
                    pos = value_.find_last_of('\n', pos - 1);
                }

                write(value_, args...);
            }
        }
    }

    void write(ElementType type)
    {
        switch (type)
        {
        case ElementType::Boolean:
            write("_ct.c_bool");
            break;
        case ElementType::Char:
            write("_ct.c_wchar");
            break;
        case ElementType::I1:
            write("_ct.c_byte");
            break;
        case ElementType::U1:
            write("_ct.c_ubyte");
            break;
        case ElementType::I2:
            write("_ct.c_short");
            break;
        case ElementType::U2:
            write("_ct.c_ushort");
            break;
        case ElementType::I4:
            write("_ct.c_int");
            break;
        case ElementType::U4:
            write("_ct.c_uint");
            break;
        case ElementType::I8:
            write("_ct.c_longlong");
            break;
        case ElementType::U8:
            write("_ct.c_ulonglong");
            break;
        case ElementType::R4:
            write("_ct.c_float");
            break;
        case ElementType::R8:
            write("_ct.c_double");
            break;
        case ElementType::String:
            write("_HSTRING");
            break;
        case ElementType::Object:
            write("_ct.c_void_p");
            break;
        default:
            throw_invalid("element type not impl");
        }
    }

    void write(TypeDef const& type)
    {
        switch (get_category(type))
        {
        case category::struct_type:
            throw_invalid("write_com_type struct not impl");
        case category::enum_type:
            throw_invalid("write_com_type enum not impl");
            break;
        case category::class_type:
        case category::delegate_type:
        case category::interface_type:
            write("_ct.c_void_p");
            break;
        default:
            throw_invalid("invalid type category");
        }
    }

    void write(TypeRef const& type)
    {
        auto const& ns = type.TypeNamespace();
        auto const& name = type.TypeName();
        if (ns == "System"sv && name == "Guid")
        {
            write("_GUID");
        }
        else
        {
            auto td = find(type);
            write(td);
        }
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
            throw_invalid("write TypeSpec not impl");
        }
    }

    void write(TypeSig const& sig)
    {
        std::visit(overloaded{
            [&](ElementType type) { write(type); },
            [&](coded_index<TypeDefOrRef> type) { write(type); },
            [&](GenericTypeIndex) { throw_invalid("write GenericTypeIndex not impl"); },
            [&](GenericTypeInstSig) { throw_invalid("write GenericTypeInstSig not impl"); },
            }, sig.Type());
    }

    void write(RetTypeSig const& sig)
    {
        write(sig.Type());
    }

    void write(ParamSig const& sig)
    {
        write(sig.Type());
    }
};

void write_comment_typesig(writer& w, TypeSig const& sig)
{
    auto write_type_name = [&w](std::string_view const& ns, std::string_view const& name)
    {
        if (w.type_namespace == ns)
        {
            w.write(name);
        }
        else
        {
            w.write("%.%", ns, name);
        }
    };

    std::visit(overloaded{
        [&](ElementType type) {
            switch (type)
            {
            case ElementType::Boolean:
                w.write("Boolean");
                break;
            case ElementType::Char:
                w.write("Char");
                break;
            case ElementType::I1:
                w.write("Int8");
                break;
            case ElementType::U1:
                w.write("UInt8");
                break;
            case ElementType::I2:
                w.write("Int16");
                break;
            case ElementType::U2:
                w.write("UInt16");
                break;
            case ElementType::I4:
                w.write("Int32");
                break;
            case ElementType::U4:
                w.write("UInt32");
                break;
            case ElementType::I8:
                w.write("Int64");
                break;
            case ElementType::U8:
                w.write("UInt64");
                break;
            case ElementType::R4:
                w.write("Single");
                break;
            case ElementType::R8:
                w.write("Double");
                break;
            case ElementType::String:
                w.write("String");
                break;
            case ElementType::Object:
                w.write("Object");
                break;
            default:
                throw_invalid("write_comment_typesig element type not impl");
            }
        },
        [&](coded_index<TypeDefOrRef> type) {
            switch (type.type())
            {
            case TypeDefOrRef::TypeDef:
            {
                auto td = type.TypeDef();
                write_type_name(td.TypeNamespace(), td.TypeName());
                break;
            }
            case TypeDefOrRef::TypeRef:
            {
                auto tr = type.TypeRef();
                write_type_name(tr.TypeNamespace(), tr.TypeName());
                break;
            }
            case TypeDefOrRef::TypeSpec:
                throw_invalid("write_comment_typesig TypeSpec not impl");
            }
        },
        [&](GenericTypeIndex) { throw_invalid("write_comment_typesig GenericTypeIndex not impl"); },
        [&](GenericTypeInstSig) { throw_invalid("write_comment_typesig GenericTypeInstSig not impl"); },
        }, sig.Type());

}

void write_method_signature_comment(writer& w, MethodDef const& method)
{
    method_signature signature{ method };

    w.write_indented("# %(", method.Name());

    auto first = true;
    for (auto&& p : signature.params())
    {
        if (!first)
        {
            w.write(", ");
        }
        else
        {
            first = false;
        }

        write_comment_typesig(w, p.second.Type());
    }
    w.write(")");

    auto const& return_type = signature.return_signature();
    if (return_type)
    {
        w.write(" -> % ", bind<write_comment_typesig>(return_type.Type()));
    }

    w.write("\n");
}

void write_guid(writer& w, TypeDef const& type)
{
    using std::get;

    auto attribute = get_attribute(type, "Windows.Foundation.Metadata", "GuidAttribute");
    auto args = attribute.Value().FixedArgs();

    w.write_printf("_GUID(0x%08X, 0x%04X, 0x%04X, (0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X))",
        get<uint32_t>(get<ElemSig>(args[0].value).value),
        get<uint16_t>(get<ElemSig>(args[1].value).value),
        get<uint16_t>(get<ElemSig>(args[2].value).value),
        get<uint8_t>(get<ElemSig>(args[3].value).value),
        get<uint8_t>(get<ElemSig>(args[4].value).value),
        get<uint8_t>(get<ElemSig>(args[5].value).value),
        get<uint8_t>(get<ElemSig>(args[6].value).value),
        get<uint8_t>(get<ElemSig>(args[7].value).value),
        get<uint8_t>(get<ElemSig>(args[8].value).value),
        get<uint8_t>(get<ElemSig>(args[9].value).value),
        get<uint8_t>(get<ElemSig>(args[10].value).value));
}

void write_abi_function_param(writer& w, ParamSig const& sig)
{
    w.write(", %", sig);
}

void write_abi_function_return(writer& w, RetTypeSig const& sig)
{
    if (sig)
    {
        w.write(", _ct.POINTER(%)", sig);
    }
}

void write_abi_function(writer& w, MethodDef const& method, int32_t offset)
{
    w.write_indented("__% = _ct.WINFUNCTYPE(_ct.HRESULT%%)(%, '%')\n",
        method.Name(),
        bind_each<write_abi_function_param>(method.Signature().Params()),
        bind<write_abi_function_return>(method.Signature().ReturnType()),
        offset,
        method.Name());
}

void write_abi_interface(writer& w, TypeDef const& type, std::string_view iid_name)
{
    w.write_indented("% = %\n", iid_name, bind<write_guid>(type));

    // can't use bind_each here because offset needs to be updated for each function
    int32_t offset = 6;
    for (auto&& method : type.MethodList())
    {
        write_abi_function(w, method, offset++);
    }
    w.write("\n");
}

void write_method_def_param(writer& w, method_signature::param_t const& p)
{
    w.write(", ");
    w.write(p.first.Name());
}

void write_method_def(writer& w, MethodDef const& method, MethodSemantics const& semantics, method_signature const& signature, bool class_method = false)
{
    auto self_name = class_method ? "cls" : "self";

    if (semantics)
    {
        auto const& target = semantics.Association();
        auto const& semantic = semantics.Semantic();

        if (target.type() == HasSemantics::Property)
        {
            auto const& property = target.Property();

            if (semantic.Getter())
            {
                w.write_indented("^@%property\ndef %(%%):\n", 
                    class_method ? "_rt.class" : "",
                    property.Name(), 
                    self_name, 
                    bind_each<write_method_def_param>(signature.params()));
            }
            else if (semantic.Setter())
            {
                w.write_indented("^@%.setter\ndef %(%%):\n", property.Name(), property.Name(), self_name, bind_each<write_method_def_param>(signature.params()));
            }
            else
            {
                throw_invalid("write_method_def invalid property semantic");
            }
        }

        if (target.type() == HasSemantics::Event)
        {
            throw_invalid("write_method_def event semantic not impl");
        }
    }
    else
    {
        if (class_method)
        {
            w.write_indented("@classmethod\n");
        }

        w.write_indented("def %(%%):\n", 
            method.Name(), 
            self_name, 
            bind_each<write_method_def_param>(signature.params()));
    }
}





void write_method_invoke_params(writer& w, method_signature const& signature)
{
    for (auto&& p : signature.params())
    {
        w.write(", _%", p.first.Name());
    }

    if (signature.return_signature())
    {
        w.write(", _ct.byref(__retval)");
    }
}

void write_method_param_variable(writer& w, method_signature::param_t const& param)
{
    w.write_indented("_% = ", param.first.Name());

    std::visit(overloaded{
        [&](ElementType type) {
            switch (type)
            {
            case ElementType::Boolean:
            case ElementType::Char:
            case ElementType::I1:
            case ElementType::U1:
            case ElementType::I2:
            case ElementType::U2:
            case ElementType::I4:
            case ElementType::U4:
            case ElementType::I8:
            case ElementType::U8:
            case ElementType::R4:
            case ElementType::R8:
            case ElementType::String:
                w.write("%(%)\n", param.second, param.first.Name());
                break;
            default:
                throw_invalid("element type not impl");
            }
        },
        [&](coded_index<TypeDefOrRef> type) {
            switch (type.type())
            {
            case TypeDefOrRef::TypeDef:
            {
                auto td = type.TypeDef();
                w.write_indented("%._%__interface # TD %.%\n", param.first.Name(), td.TypeName(), td.TypeNamespace(), td.TypeName());
                break;
            }
            case TypeDefOrRef::TypeRef:
            {
                auto tr = type.TypeRef();
                auto td = tr.get_cache().find(tr.TypeNamespace(), tr.TypeName());
                w.write_indented("%._%__interface # TD from TR %.%\n", param.first.Name(), td.TypeName(), tr.TypeNamespace(), tr.TypeName());
                break;
            }
            case TypeDefOrRef::TypeSpec:
                throw_invalid("write_comment_typesig TypeSpec not impl");
            }
        },
        [&](GenericTypeIndex) { throw_invalid("write_comment_typesig GenericTypeIndex not impl"); },
        [&](GenericTypeInstSig) { throw_invalid("write_comment_typesig GenericTypeInstSig not impl"); },
            }, param.second.Type().Type());

}

void write_method_return_variable(writer& w, RetTypeSig const& return_sig)
{
    if (return_sig)
    {
        w.write_indented("__retval = %()\n", return_sig);
    }
}

void write_method_return(writer& w, RetTypeSig const& return_sig)
{
    auto write_typedef_return = [&w](TypeDef const& type)
    {
        if (w.type_namespace == type.TypeNamespace())
        {
            w.write_indented("return %(__retval)\n", type.TypeName());
        }
        else
        {
            w.write_indented("from % import % as __%\n", type.TypeNamespace(), type.TypeName(), type.TypeName());
            w.write_indented("return __%(__retval)\n", type.TypeName());
        }
    };

    if (return_sig)
    {
        std::visit(overloaded{
            [&](ElementType type) {
                switch (type)
                {
                case ElementType::Boolean:
                case ElementType::Char:
                case ElementType::I1:
                case ElementType::U1:
                case ElementType::I2:
                case ElementType::U2:
                case ElementType::I4:
                case ElementType::U4:
                case ElementType::I8:
                case ElementType::U8:
                case ElementType::R4:
                case ElementType::R8:
                    w.write_indented("return __retval.value\n");
                    break;
                case ElementType::String:
                    w.write_indented("return str(__retval)\n");
                    break;
                default:
                    throw_invalid("element type not impl");
                }
            },
            [&](coded_index<TypeDefOrRef> type) {
                switch (type.type())
                {
                case TypeDefOrRef::TypeDef:
                {
                    write_typedef_return(type.TypeDef());
                    break;
                }
                case TypeDefOrRef::TypeRef:
                {
                    auto tr = type.TypeRef();
                    auto td = tr.get_cache().find(tr.TypeNamespace(), tr.TypeName());
                    write_typedef_return(td);
                    break;
                }
                case TypeDefOrRef::TypeSpec:
                    throw_invalid("write_comment_typesig TypeSpec not impl");
                }
            },
            [&](GenericTypeIndex) { throw_invalid("write_comment_typesig GenericTypeIndex not impl"); },
            [&](GenericTypeInstSig) { throw_invalid("write_comment_typesig GenericTypeInstSig not impl"); },
            }, return_sig.Type().Type());
    }

    w.write("\n");
}

void write_invoke_method_body(writer& w, method_signature const& signature, std::string_view const& function_name, std::string_view const& variable_name)
{
    for (auto&& param : signature.params())
    {
        write_method_param_variable(w, param);
    }
    auto const& return_signature = signature.return_signature();
    
    write_method_return_variable(w, return_signature);
    
    w.write_indented("%(%%)\n", function_name, variable_name, bind<write_method_invoke_params>(signature));
    
    write_method_return(w, return_signature);
}

void write_class_activation(writer& w, CustomAttribute const& attr)
{
    auto const& sig = attr.Value();
    auto const& args = sig.FixedArgs();
    auto const& arg0 = std::get_if<ElemSig::SystemType>(&std::get<ElemSig>(args[0].value).value);

    if (arg0)
    {
        auto const& iface = attr.get_database().get_cache().find(arg0->name);

        if (!is_exclusive_to(iface))
        {
            throw_invalid("non exclusive activation interface not supported");
        }

        w.write_indented("# custom activation %.%\n", iface.TypeNamespace(), iface.TypeName());

        auto iid_name = w.write_temp("__iid_%", iface.TypeName());
        write_abi_interface(w, iface, iid_name);

        for (auto&& method : iface.MethodList())
        {
            method_signature signature{ method };
            write_method_signature_comment(w, method);

            w.write_indented("^@classmethod\ndef %(cls%):\n",
                method.Name(),
                bind_each<write_method_def_param>(signature.params()));
            writer::indent_guard g{ w };

            w.write_indented("with cls.__get_factory(cls.%) as __ptr:\n", iid_name);
            writer::indent_guard gg{ w };

            auto function_name = w.write_temp("cls.__%", method.Name());
            write_invoke_method_body(w, signature, function_name, "__ptr");
        }
    }
    else
    {
        throw_invalid("write_class_activatable_attribute default activation not impl");
    }
}

void write_class_static_interface(writer& w, CustomAttribute const& attr)
{
    auto const& sig = attr.Value();
    auto const& args = sig.FixedArgs();
    auto const& arg0 = std::get<ElemSig::SystemType>(std::get<ElemSig>(args[0].value).value);
    auto const& iface = attr.get_database().get_cache().find(arg0.name);

    if (!is_exclusive_to(iface))
    {
        throw_invalid("non exclusive static interface not supported");
    }

    w.write_indented("# static %\n", arg0.name);

    auto iid_name = w.write_temp("__iid_%", iface.TypeName());
    write_abi_interface(w, iface, iid_name);

    for (auto&& method : iface.MethodList())
    {
        auto const& semantics = get_method_semantics(iface, method);
        method_signature signature{ method };

        write_method_signature_comment(w, method);

        write_method_def(w, method, semantics, signature, true);
        writer::indent_guard g{ w };

        auto function_name = w.write_temp("cls.__%", method.Name());

        w.write_indented("with cls.__get_factory(cls.%) as __ptr:\n", iid_name);
        writer::indent_guard gg{ w };

        write_invoke_method_body(w, signature, function_name, "__ptr");
    }
}

void write_class_interface(writer& w, InterfaceImpl const& iface_impl)
{
    auto is_default_interface = get_attribute(iface_impl, "Windows.Foundation.Metadata", "DefaultAttribute");

    auto const& iface = find(iface_impl.Interface().TypeRef());
    auto is_exclusive_interface = is_exclusive_to(iface);

    w.write_indented("# %instance interface %.%\n",
        is_default_interface ? "default " : "",
        iface.TypeNamespace(), iface.TypeName());

    auto iid_name = is_default_interface ? "__iid" : w.write_temp("__iid_%", iface.TypeName());

    if (is_exclusive_interface)
    {
        write_abi_interface(w, iface, iid_name);
    }

    for (auto&& method : iface.MethodList())
    {
        auto const& semantics = get_method_semantics(iface, method);
        method_signature signature{ method };

        write_method_signature_comment(w, method);

        write_method_def(w, method, semantics, signature);
        writer::indent_guard g{ w };

        if (is_exclusive_interface)
        {
            w.write_indented("__cls = type(self)\n");

            if (is_default_interface)
            {
                w.write_indented("with self.__get_interface() as __ptr:\n");
            }
            else
            {
                w.write_indented("with self.__get_interface(__cls.%) as __ptr:\n", iid_name);
            }
           
            writer::indent_guard gg{ w };

            auto function_name = w.write_temp("__cls.__%", method.Name());
            write_invoke_method_body(w, signature, function_name, "__ptr");
        }
        else
        {
            // TODO: DRY this out
            if (w.type_namespace == iface.TypeNamespace())
            {
                w.write_indented("with self.__get_interface(%._iid) as __ptr:\n", iface.TypeName());
                writer::indent_guard gg{ w };

                w.write_indented("return %._invoke_%(__ptr%)\n", iface.TypeName(), method.Name(),
                    bind_each<write_method_def_param>(signature.params()));
            }
            else
            {
                w.write_indented("from % import % as %", iface.TypeNamespace(), iface.TypeName(), iface.TypeName());
                w.write_indented("with self.__get_interface(__%._iid) as __ptr:\n", iface.TypeName());
                writer::indent_guard gg{ w };

                w.write_indented("return __%._invoke_%(__ptr%)\n", iface.TypeName(), method.Name(),
                    bind_each<write_method_def_param>(signature.params()));
            }
        }
    }
}

void write_class(writer& w, TypeDef const& type)
{
    auto const& extends = type.Extends().TypeRef();
    if (extends.TypeNamespace() != "System"sv || extends.TypeName() != "Object"sv)
    {
        throw_invalid("write_class inheritance not implemented");
    }

    w.write_indented("class %():\n", type.TypeName());
    writer::indent_guard g{ w };

    w.write_indented(strings::class_get_factory, type.TypeNamespace(), type.TypeName());
    w.write_indented(strings::class_get_interface);
    w.write_indented(strings::class_new);
    w.write_indented(strings::class_init);

    for (auto&& attr : type.CustomAttribute())
    {
        auto const& name = attr.TypeNamespaceAndName();
        if (name.first == "Windows.Foundation.Metadata"sv)
        {
            if (name.second == "StaticAttribute"sv)
            {
                write_class_static_interface(w, attr);
            }

            if (name.second == "ActivatableAttribute"sv)
            {
                write_class_activation(w, attr);
            }
        }
    }

    for (auto&& iface : type.InterfaceImpl())
    {
        write_class_interface(w, iface);
    }

    w.write("\n");
}

void write_interface(writer& w, TypeDef const& type)
{
    if (is_exclusive_to(type))
    {
        return;
    }

    w.write_indented("class %():\n", type.TypeName());
    writer::indent_guard g{ w };

    w.write_indented(strings::class_new);
    w.write_indented(strings::class_init);

    write_abi_interface(w, type, "_iid");
    for (auto&& method : type.MethodList())
    {
        method_signature signature{ method };
        auto const& semantics = get_method_semantics(type, method);

        write_method_signature_comment(w, method);

        {
            w.write_indented("^@classmethod\ndef _invoke_%(cls, ptr%):\n", 
                method.Name(), 
                bind_each<write_method_def_param>(signature.params()));
            writer::indent_guard gg{ w };

            auto function_name = w.write_temp("cls.__%", method.Name());

            write_invoke_method_body(w, signature, function_name, "ptr");
        }

        {
            write_method_def(w, method, semantics, signature);
            writer::indent_guard gg{ w };

            w.write_indented("return type(self)._invoke_%(self.__interface%)\n\n", method.Name(), bind_each<write_method_def_param>(signature.params()));
        }
    }

    w.write("\n");
}

auto get_in(reader const& args)
{
    std::vector<std::string> files;

    auto add_directory = [&](auto&& path)
    {
        for (auto&& file : directory_iterator(path))
        {
            if (is_regular_file(file))
            {
                files.push_back(file.path().string());
            }
        }
    };

    for (auto&& path : args.values("input"))
    {
        if (is_directory(path))
        {
            add_directory(path);
        }
        else if (is_regular_file(path))
        {
            files.push_back(path);
        }
#if XLANG_PLATFORM_WINDOWS
        else if (path == "local")
        {
            std::array<char, MAX_PATH> local{};
            ExpandEnvironmentStringsA("%windir%\\System32\\WinMetadata", local.data(), static_cast<DWORD>(local.size()));
            add_directory(local.data());
        }
#endif
        else
        {
            throw_invalid("Path '", path, "' is not a file or directory");
        }
    }

    return files;
}

auto get_out(reader const& args)
{
    auto out{ absolute(args.value("output", "output")) };

    create_directories(out);
    out += path::preferred_separator;
    return out.string();
}

void print_usage()
{
    puts("Usage...");
}

void write_base_py(std::string const& output);

int main(int const argc, char** argv)
{
    writer w;

    try
    {
        auto start = high_resolution_clock::now();

        std::vector<option> options
        {
            // name, min, max
            { "input", 1 },
            { "output", 0, 1 },
            { "include", 0 },
            { "exclude", 0 },
            { "verbose", 0, 0 },
        };

        reader args{ argc, argv, options };

        if (!args)
        {
            print_usage();
            return 0;
        }

        cache c{ get_in(args) };
        auto const out = get_out(args);
        bool const verbose = args.exists("verbose");

        filter f(args.values("include"), args.values("exclude"));

        if (verbose)
        {
            std::for_each(c.databases().begin(), c.databases().end(), [&](auto&& db)
            {
                w.write("in: %\n", db.path());
            });

            w.write("out: %\n", out);
        }

        w.flush_to_console();
        task_group group;

        write_base_py(out);

        for (auto&& ns : c.namespaces())
        {
            if (!f.includes(ns.second))
            {
                continue;
            }

            group.add([&]
            {
                path ns_path{ out };
                for (auto const& segment : get_dotted_name_segments(ns.first))
                {
                    ns_path.append(segment.begin(), segment.end());
                }

                create_directories(ns_path);

                writer w;
                w.type_namespace = ns.first;
                //w.debug_trace = true;

                w.write(R"(
import ctypes as _ct
import winrt as _rt
import contextlib as _ctx
from hstring import HSTRING as _HSTRING
from guid import GUID as _GUID

)");
                w.write("%%",
                    f.bind_each<write_interface>(ns.second.interfaces),
                    f.bind_each<write_class>(ns.second.classes));

                w.flush_to_file((ns_path / "__init__.py").string());
            });
        }

        group.get();

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

void write_base_py(std::string const& out)
{
    {
        writer w;
        w.write(strings::guid);
        w.flush_to_file(out + "guid.py");
    }

    {
        writer w;
        w.write(strings::hstring);
        w.flush_to_file(out + "hstring.py");
    }

    {
        writer w;
        w.write(strings::winrt);
        w.flush_to_file(out + "winrt.py");
    }

    {
        writer w;
        w.write(strings::test);
        w.flush_to_file(out + "test.py");
    }
}