#pragma once

namespace xlang
{
    inline auto get_start_time()
    {
        return std::chrono::high_resolution_clock::now();
    }

    inline auto get_elapsed_time(std::chrono::time_point<std::chrono::high_resolution_clock> const& start)
    {
        return std::chrono::duration_cast<std::chrono::duration<int64_t, std::milli>>(std::chrono::high_resolution_clock::now() - start).count();
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

    bool is_ptype(TypeDef const& type)
    {
        return distance(type.GenericParam()) > 0;
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
                return std::move(segments);
            }

            segments.push_back(ns.substr(pos, new_pos - pos));
            pos = new_pos + 1;
        } while (true);
    };

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

    bool has_dealloc(TypeDef const& type)
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
}