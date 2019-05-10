#pragma once

namespace xlang::meta::reader
{
    bool is_exclusive_to(TypeDef const& type)
    {
        return get_category(type) == category::interface_type && get_attribute(type, "Windows.Foundation.Metadata", "ExclusiveToAttribute");
    }

    bool is_flags_enum(TypeDef const& type)
    {
        return get_category(type) == category::enum_type && get_attribute(type, "System", "FlagsAttribute");
    }

    bool is_constructor(MethodDef const& method)
    {
        return method.Flags().RTSpecialName() && method.Name() == ".ctor";
    }

    bool is_special(MethodDef const& method)
    {
        return method.SpecialName() || method.Flags().RTSpecialName();
    }

    auto get_delegate_invoke(TypeDef const& type)
    {
        XLANG_ASSERT(get_category(type) == category::delegate_type);

        for (auto&& method : type.MethodList())
        {
            if (method.SpecialName() && (method.Name() == "Invoke"))
            {
                return method;
            }
        }

        throw_invalid("Invoke method not found");
    }

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

            for (uint32_t i{}; i != size(m_method.Params()); ++i)
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
                name = "_return_value";
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
}