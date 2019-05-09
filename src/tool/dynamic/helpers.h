#pragma once

namespace fooxlang
{
    auto get_start_time()
    {
        return std::chrono::high_resolution_clock::now();
    }

    auto get_elapsed_time(std::chrono::time_point<std::chrono::high_resolution_clock> const& start)
    {
        return std::chrono::duration_cast<std::chrono::duration<int64_t, std::milli>>(std::chrono::high_resolution_clock::now() - start).count();
    }

    auto get_dotted_name_segments(std::string_view ns)
    {
        std::vector<std::string_view> segments;
        size_t pos = 0;

        while (true)
        {
            auto new_pos = ns.find('.', pos);

            if (new_pos == std::string_view::npos)
            {
                segments.push_back(ns.substr(pos));
                return std::move(segments);
            }

            segments.push_back(ns.substr(pos, new_pos - pos));
            pos = new_pos + 1;
        };
    };

    bool is_exclusive_to(xlang::meta::reader::TypeDef const& type)
    {
        return xlang::meta::reader::get_category(type) == xlang::meta::reader::category::interface_type 
            && xlang::meta::reader::get_attribute(type, "Windows.Foundation.Metadata", "ExclusiveToAttribute");
    }

    bool is_flags_enum(xlang::meta::reader::TypeDef const& type)
    {
        return xlang::meta::reader::get_category(type) == xlang::meta::reader::category::enum_type 
            && xlang::meta::reader::get_attribute(type, "System", "FlagsAttribute");
    }

    enum class fundamental_type
    {
        Boolean,
        Char,
        Int8,
        UInt8,
        Int16,
        UInt16,
        Int32,
        UInt32,
        Int64,
        UInt64,
        Float,
        Double,
        String,
    };

    struct generic_type_instance;
    struct object_type {};
    struct guid_type {};
    using type_definition = xlang::meta::reader::TypeDef;
    using generic_type_index = xlang::meta::reader::GenericTypeIndex;

    using type_semantics = std::variant<
        fundamental_type,
        object_type,
        guid_type,
        type_definition,
        generic_type_instance,
        generic_type_index>;

    struct generic_type_instance
    {
        type_definition generic_type;
        std::vector<type_semantics> generic_args{};
    };

    type_semantics get_type_semantics(xlang::meta::reader::TypeSig const& signature);

    type_semantics get_type_semantics(xlang::meta::reader::GenericTypeInstSig const& type)
    {
        auto generic_type_helper = [&type]()
        {
            switch (type.GenericType().type())
            {
            case xlang::meta::reader::TypeDefOrRef::TypeDef:
                return type.GenericType().TypeDef();
            case xlang::meta::reader::TypeDefOrRef::TypeRef:
                return xlang::meta::reader::find_required(type.GenericType().TypeRef());
            }

            xlang::throw_invalid("invalid TypeDefOrRef value for GenericTypeInstSig.GenericType");
        };

        auto gti = generic_type_instance{ generic_type_helper() };

        for (auto&& arg : type.GenericArgs())
        {
            gti.generic_args.push_back(get_type_semantics(arg));
        }

        return gti;
    }

    type_semantics get_type_semantics(xlang::meta::reader::coded_index<xlang::meta::reader::TypeDefOrRef> const& type)
    {
        switch (type.type())
        {
        case xlang::meta::reader::TypeDefOrRef::TypeDef:
            return type.TypeDef();
        case xlang::meta::reader::TypeDefOrRef::TypeRef:
        {
            auto type_ref = type.TypeRef();
            if (type_ref.TypeName() == "Guid" && type_ref.TypeNamespace() == "System")
            {
                return guid_type{};
            }

            return xlang::meta::reader::find_required(type_ref);
        }
        case xlang::meta::reader::TypeDefOrRef::TypeSpec:
            return get_type_semantics(type.TypeSpec().Signature().GenericTypeInst());
        }

        xlang::throw_invalid("TypeDefOrRef not supported");
    }

    namespace impl
    {
        template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
        template<class... Ts> overloaded(Ts...)->overloaded<Ts...>;
    }

    type_semantics get_type_semantics(xlang::meta::reader::TypeSig const& signature)
    {
        return std::visit(
            impl::overloaded{
            [](xlang::meta::reader::ElementType type) -> type_semantics
        {
            switch (type)
            {
            case xlang::meta::reader::ElementType::Boolean:
                return fundamental_type::Boolean;
            case xlang::meta::reader::ElementType::Char:
                return fundamental_type::Char;
            case xlang::meta::reader::ElementType::I1:
                return fundamental_type::Int8;
            case xlang::meta::reader::ElementType::U1:
                return fundamental_type::UInt8;
            case xlang::meta::reader::ElementType::I2:
                return fundamental_type::Int16;
            case xlang::meta::reader::ElementType::U2:
                return fundamental_type::UInt16;
            case xlang::meta::reader::ElementType::I4:
                return fundamental_type::Int32;
            case xlang::meta::reader::ElementType::U4:
                return fundamental_type::UInt32;
            case xlang::meta::reader::ElementType::I8:
                return fundamental_type::Int64;
            case xlang::meta::reader::ElementType::U8:
                return fundamental_type::UInt64;
            case xlang::meta::reader::ElementType::R4:
                return fundamental_type::Float;
            case xlang::meta::reader::ElementType::R8:
                return fundamental_type::Double;
            case xlang::meta::reader::ElementType::String:
                return fundamental_type::String;
            case xlang::meta::reader::ElementType::Object:
                return object_type{};
            }
            xlang::throw_invalid("element type not supported");
        },
            [](xlang::meta::reader::coded_index<xlang::meta::reader::TypeDefOrRef> type) -> type_semantics
        {
            return get_type_semantics(type);
        },
            [](xlang::meta::reader::GenericTypeIndex var) -> type_semantics { return generic_type_index{ var.index }; },
            [](xlang::meta::reader::GenericTypeInstSig sig) -> type_semantics { return get_type_semantics(sig); },
            [](xlang::meta::reader::GenericMethodTypeIndex) -> type_semantics { xlang::throw_invalid("Generic methods not supported"); }
            }, signature.Type());
    }

    auto get_typedef(type_semantics const& semantics)
    {
        return std::visit(
            impl::overloaded{
                [](type_definition type) { return type; },
                [](generic_type_instance type_instance) { return type_instance.generic_type; },
                [](auto) -> type_definition { xlang::throw_invalid("type doesn't contain typedef"); }
            }, semantics);
    };

    auto get_typedef(xlang::meta::reader::coded_index<xlang::meta::reader::TypeDefOrRef> const& type)
    {
        return get_typedef(get_type_semantics(type));
    };

    struct method_signature
    {
        using param_t = std::pair<xlang::meta::reader::Param, xlang::meta::reader::ParamSig const*>;

        explicit method_signature(xlang::meta::reader::MethodDef const& method) :
            m_method(method.Signature())
        {
            auto params = method.ParamList();

            if (m_method.ReturnType() && params.first != params.second && params.first.Sequence() == 0)
            {
                m_return = params.first;
                ++params.first;
            }

            for (uint32_t i{}; i != xlang::meta::reader::size(m_method.Params()); ++i)
            {
                m_params.emplace_back(params.first + i, &m_method.Params().first[i]);
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

        xlang::meta::reader::MethodDefSig m_method;
        std::vector<param_t> m_params;
        xlang::meta::reader::Param m_return;
    };
}