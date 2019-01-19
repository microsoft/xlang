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

        xlang::meta::reader::MethodDefSig m_method;
        std::vector<param_t> m_params;
        xlang::meta::reader::Param m_return;
    };

    template <typename T>
    struct signature_handler_base
    {
        void handle_class(xlang::meta::reader::TypeDef const& /*type*/) { xlang::throw_invalid("handle_class not implemented"); }
        void handle_delegate(xlang::meta::reader::TypeDef const& /*type*/) { xlang::throw_invalid("handle_delegate not implemented"); }
        void handle_guid(xlang::meta::reader::TypeRef const& /*type*/) { xlang::throw_invalid("handle_guid not implemented"); }
        void handle_interface(xlang::meta::reader::TypeDef const& /*type*/) { xlang::throw_invalid("handle_interface not implemented"); }
        void handle_struct(xlang::meta::reader::TypeDef const& /*type*/) { xlang::throw_invalid("handle_struct not implemented"); }

        void handle_enum(xlang::meta::reader::TypeDef const& type)
        {
            if (is_flags_enum(type))
            {
                static_cast<T*>(this)->handle(xlang::meta::reader::ElementType::U4);
            }
            else
            {
                static_cast<T*>(this)->handle(xlang::meta::reader::ElementType::I4);
            }
        }

        void handle(xlang::meta::reader::TypeRef const& type)
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

        void handle(xlang::meta::reader::TypeDef const& type)
        {
            switch (xlang::meta::reader::get_category(type))
            {
            case xlang::meta::reader::category::class_type:
                static_cast<T*>(this)->handle_class(type);
                break;
            case xlang::meta::reader::category::delegate_type:
                static_cast<T*>(this)->handle_delegate(type);
                break;
            case xlang::meta::reader::category::interface_type:
                static_cast<T*>(this)->handle_interface(type);
                break;
            case xlang::meta::reader::category::enum_type:
                static_cast<T*>(this)->handle_enum(type);
                break;
            case xlang::meta::reader::category::struct_type:
                static_cast<T*>(this)->handle_struct(type);
                break;
            }
        }

        void handle(xlang::meta::reader::coded_index<xlang::meta::reader::TypeDefOrRef> const& type)
        {
            switch (type.type())
            {
            case xlang::meta::reader::TypeDefOrRef::TypeDef:
                static_cast<T*>(this)->handle(type.TypeDef());
                break;

            case xlang::meta::reader::TypeDefOrRef::TypeRef:
                static_cast<T*>(this)->handle(type.TypeRef());
                break;

            case xlang::meta::reader::TypeDefOrRef::TypeSpec:
                static_cast<T*>(this)->handle(type.TypeSpec().Signature().GenericTypeInst());
                break;
            }
        }

        void handle(xlang::meta::reader::GenericTypeInstSig const& type)
        {
            static_cast<T*>(this)->handle(type.GenericType());

            for (auto&& arg : type.GenericArgs())
            {
                static_cast<T*>(this)->handle(arg);
            }
        }

        void handle(xlang::meta::reader::ElementType /*type*/) { xlang::throw_invalid("handle(ElementType) not implemented"); }

        void handle(xlang::meta::reader::GenericTypeIndex /*var*/) { xlang::throw_invalid("handle(GenericTypeIndex) not implemented"); }

        void handle(xlang::meta::reader::TypeSig const& signature)
        {
            xlang::call(signature.Type(), [this](auto&& type) { static_cast<T*>(this)->handle(type); });
        }
    };
}