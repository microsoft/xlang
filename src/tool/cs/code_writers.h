#pragma once

namespace xlang
{
    void write_type_namespace(writer& w, std::string_view const& ns)
    {
        w.write("\nnamespace @ {\n", ns);
    }
    void write_close_namespace(writer& w)
    {
        w.write("\n}\n");
    }

    void write_enum_field(writer& w, Field const& field)
    {
        if (auto constant = field.Constant())
        {
            w.write("\n    @ = %,",
                field.Name(),
                *constant);
        }
    }
    void write_enum(writer& w, TypeDef const& type)
    {
        auto format = R"(
@public enum @ : % {%
}
)";
        auto fields = type.FieldList();
        auto flags_attr = get_attribute(type, "System", "FlagsAttribute");
        w.write(format,
            flags_attr ? "[Flags]\n" : "",
            type.TypeName(),
            fields.first.Signature().Type(),
            bind_each<write_enum_field>(fields));
    }

    void write_struct_field(writer& w, Field const& field)
    {
        w.write("\n    % @;", field.Signature().Type(), field.Name());
    }

    void write_struct(writer& w, TypeDef const& type)
    {
        auto format = R"(
public struct @ {%
}
)";
        w.write(format,
            type.TypeName(),
            bind_each<write_struct_field>(type.FieldList()));
    }

//     void write_method_param(writer& w, Param const& param)
//     {
//         w.write(", % %", param.Name(), param.Seq());
//     }

    template<typename T, typename U>
    auto zip(const T& t, const U& u)
    {
        auto tc = begin(t);
        auto te = end(t);
        auto uc = begin(u);
        auto ue = end(u);
        std::vector< std::pair< decltype(*tc), decltype(*uc) > > r;
        for (; tc != te && uc != ue; ++tc, ++uc)
        {
            r.emplace_back(*tc, *uc);
        }
        XLANG_ASSERT(tc==te && uc==ue);
        return r;
    }

    void write_method_params(writer& w, method_signature const& signature)
    {
        separator sep{ w };
        for (auto&&[param, param_signature] : signature.params())
        {
            sep();
            w.write("% %", "int"/*param_signature->Type()*/, param.Name());
        }
    }

    void write_interface_method(writer& w, MethodDef const& method)
    {
        auto format = R"---(
    public % @(%) {
        var vslot = ((void***)instance)[0][%];
        var func = Marshal.GetDelegateForFunctionPointer<@>((IntPtr)vslot);
        int hr = func(instance%);
        if (hr < 0)
            throw new Exception(hr);%
    })---";
        method_signature msig{ method };
        w.write(format,
            msig.return_signature(),
            method.Name(),
            bind<write_method_params>(msig),
            (int)method.RVA(),
            "DEL",
            ", arg1",
            ""
        );
    }

    void write_interface_required(writer& w, interface_info const& iface)
    {
        w.write("%, ", iface.type.TypeName());
    }

    void write_guid_value(writer& w, std::vector<FixedArgSig> const& args)
    {
        using std::get;

        w.write_printf("%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
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

    void write_interface(writer& w, TypeDef const& type)
    {
        auto req = get_required_interfaces(type);
        req.erase(req.begin(), req.begin() + 1);
        
        auto format = R"---(
[GuidAttribute("%")]
public unsafe partial class % : % {%
}
)---";
        auto guid = get_attribute(type, "Windows.Foundation.Metadata", "GuidAttribute");
        w.write(format,
            bind<write_guid_value>(guid.Value().FixedArgs()),
            type.TypeName(),
            bind_each<write_interface_required>(req),
            bind_each<write_interface_method>(type.MethodList()));
    }
}