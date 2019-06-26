
namespace winrt::impl
{
    template <typename T>
    class has_bind_member
    {
        template <typename U, typename = decltype(std::declval<U>().bind(L""))> static constexpr bool get_value(int) { return true; }
        template <typename> static constexpr bool get_value(...) { return false; }

    public:

        static constexpr bool value = get_value<T>(0);
    };

    template <typename T>
    class has_bind_free
    {
        template <typename U, typename = decltype(winrt::bind(std::declval<U>(), L""))> static constexpr bool get_value(int) { return true; }
        template <typename> static constexpr bool get_value(...) { return false; }

    public:

        static constexpr bool value = get_value<T>(0);
    };

    template <typename T>
    class is_bindable
    {
        template <typename U, typename = decltype(bindable<U, int>::bind(std::declval<U>(), L""))> static constexpr bool get_value(int) { return true; }
        template <typename> static constexpr bool get_value(...) { return false; }

    public:

        static constexpr bool value = get_value<T>(0);
    };

    template <typename T>
    inline constexpr bool can_bind_v = has_bind_member<T>::value || has_bind_free<T>::value || is_bindable<T>::value;

    template <typename T>
    auto bind_member(T&& object, hstring const& name)
    {
        static_assert(can_bind_v<T>);

        if constexpr (has_bind_free<T>::value)
        {
            return winrt::bind(object, name);
        }
        else if constexpr (has_bind_member<T>::value)
        {
            return object.bind(name);
        }
        else
        {
            return bindable<T, int>::bind(object, name);
        }
    }

    struct binding_base
    {
        virtual Windows::Foundation::IInspectable get() = 0;
        virtual void set(Windows::Foundation::IInspectable const& value) = 0;
        virtual bool can_bind() = 0;
        virtual xaml_binding bind(hstring const& name) = 0;
        virtual Windows::UI::Xaml::Interop::TypeName type() = 0;
    };

    template <typename Get, typename Set>
    struct binding_implementation final : binding_base, Get, Set
    {
        using value_type = decltype(std::declval<Get>()());

        binding_implementation(Get&& get_self, Set&& set_self) :
            Get(std::forward<Get>(get_self)),
            Set(std::forward<Set>(set_self))
        {
        }

        Windows::Foundation::IInspectable get() final
        {
            return box_value((*this)());
        }

        void set(Windows::Foundation::IInspectable const& value) final
        {
            (*this)(unbox_value<value_type>(value));
        }

        bool can_bind() final
        {
            return impl::can_bind_v<value_type>;
        }

        xaml_binding bind([[maybe_unused]] hstring const& name) final
        {
            if constexpr (impl::can_bind_v<value_type>)
            {
                return impl::bind_member((*this)(), name);
            }

            return {};
        }

        Windows::UI::Xaml::Interop::TypeName type() final
        {
            return xaml_typename<value_type>();
        }
    };

    struct xaml_registry
    {
        template <typename T>
        static bool add()
        {
            registry().add_type(T::GetRuntimeClassName(), []
                {
                    static auto type = T::get_type();
                    return type;
                });

            return true;
        }

        static Windows::UI::Xaml::Markup::IXamlType get(hstring const& name)
        {
            return registry().get_type(name);
        }

    private:

        std::map<hstring, winrt::delegate<Windows::UI::Xaml::Markup::IXamlType()>> m_registry;

        xaml_registry() = default;

        inline static xaml_registry& registry() noexcept
        {
            // TODO: Try to avoid magic static
            static xaml_registry s_registry;
            return s_registry;
        }

        void add_type(hstring const& name, winrt::delegate<Windows::UI::Xaml::Markup::IXamlType()> const& get)
        {
            if (m_registry.find(name) == m_registry.end())
            {
                m_registry[name] = get;
            }
        }

        Windows::UI::Xaml::Markup::IXamlType get_type(hstring const& name) const
        {
            auto info = m_registry.find(name);

            if (info != m_registry.end())
            {
                return info->second();
            }

            return {};
        }
    };

    template <typename D, bool Register = true>
    struct xaml_registration
    {
        static inline bool registered{ xaml_registry::add<D>() };
    };

    template <typename D>
    struct xaml_registration<D, false>
    {
    };
}

namespace winrt
{
    template <typename T, typename>
    xaml_binding::xaml_binding(T& reference) : xaml_binding
    {
        [&]
        {
            return reference;
        },
        [&](T const& value)
        {
            reference = value;
        }
    }
    {
    }

    template <typename T, typename>
    xaml_binding::xaml_binding(T const& object) : xaml_binding
    {
        [object]
        {
            return object;
        },
        [](T const&) {}
    }
    {
    }

    template <typename Get, typename>
    xaml_binding::xaml_binding(Get&& get_self) : xaml_binding
    {
        std::forward<Get>(get_self),
        [](auto&&) {}
    }
    {
    }

    template <typename Get, typename Set>
    xaml_binding::xaml_binding(Get&& get_self, Set&& set_self) : m_binding
    {
        std::make_unique<impl::binding_implementation<Get, Set>>(
            std::forward<Get>(get_self),
            std::forward<Set>(set_self))
    }
    {
    }

    inline auto xaml_binding::get() const
    {
        if (m_binding)
        {
            return m_binding->get();
        }

        return Windows::Foundation::IInspectable{};
    }

    inline auto xaml_binding::set(Windows::Foundation::IInspectable const& value) const
    {
        if (m_binding)
        {
            m_binding->set(value);
        }
    }

    inline auto xaml_binding::can_bind() const
    {
        if (m_binding)
        {
            return m_binding->can_bind();
        }

        return false;
    }

    inline auto xaml_binding::bind(hstring const& name) const
    {
        if (m_binding)
        {
            return m_binding->bind(name);
        }

        return xaml_binding{};
    }

    inline auto xaml_binding::type() const
    {
        if (m_binding)
        {
            return m_binding->type();
        }

        return Windows::UI::Xaml::Interop::TypeName{};
    }

    template <typename D, typename... I>
    struct xaml_app : Windows::UI::Xaml::ApplicationT<D, Windows::UI::Xaml::Markup::IXamlMetadataProvider, I...>
    {
        Windows::UI::Xaml::Markup::IXamlType GetXamlType(Windows::UI::Xaml::Interop::TypeName const& type) const
        {
            return GetXamlType(type.Name);
        }

        Windows::UI::Xaml::Markup::IXamlType GetXamlType(hstring const& name) const
        {
            return impl::xaml_registry::get(name);
        }

        com_array<Windows::UI::Xaml::Markup::XmlnsDefinition> GetXmlnsDefinitions() const
        {
            return {};
        }
    };

    template <typename D, bool R, template <typename...> typename B, typename... I>
    struct xaml_type : B<D, Windows::UI::Xaml::Data::INotifyPropertyChanged, I...>, impl::xaml_registration<D, R>
    {
        event_token PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
        {
            return m_changed.add(handler);
        }

        void PropertyChanged(winrt::event_token token)
        {
            m_changed.remove(token);
        }

        static Windows::UI::Xaml::Markup::IXamlType get_type()
        {
            return make<xaml_type_instance>();
        }

        static hstring GetRuntimeClassName()
        {
            return D::type_name();
        }

    protected:

        using base_type = xaml_type<D, R, B, I...>;

        static inline D* s_last_type;

        xaml_type(hstring const& uri)
        {
            Windows::UI::Xaml::Application::LoadComponent(*this, Windows::Foundation::Uri(uri));
            s_last_type = static_cast<D*>(this);
        }

        void property_changed(hstring const& name)
        {
            m_changed(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(name));
        }

    private:

        struct xaml_member;

        struct bind_property : implements<bind_property, Windows::UI::Xaml::Data::ICustomProperty>
        {
            bind_property(com_ptr<xaml_member> && object, xaml_binding&& xaml_binding, hstring const& name) :
                m_object(std::move(object)),
                m_binding(std::move(xaml_binding)),
                m_name(name)
            {
            }

            auto GetValue(Windows::Foundation::IInspectable const&) const
            {
                return m_binding.get();
            }

            void SetValue(Windows::Foundation::IInspectable const&, Windows::Foundation::IInspectable const& value)
            {
                m_binding.set(value);
                m_object->property_changed(m_name);
            }

            bool CanWrite() const noexcept
            {
                return true;
            }

            bool CanRead() const noexcept
            {
                return true;
            }

            Windows::UI::Xaml::Interop::TypeName Type() const noexcept
            {
                return m_binding.type();
            }

            hstring Name() const noexcept
            {
                return {};
            }

            Windows::Foundation::IInspectable GetIndexedValue(Windows::Foundation::IInspectable const&, Windows::Foundation::IInspectable const&) const noexcept
            {
                return {};
            }

            void SetIndexedValue(Windows::Foundation::IInspectable const&, Windows::Foundation::IInspectable const&, Windows::Foundation::IInspectable const&) const noexcept
            {
            }

        private:
            com_ptr<xaml_member> m_object;
            xaml_binding m_binding;
            hstring m_name;
        };

        struct xaml_member : implements<xaml_member, Windows::UI::Xaml::Markup::IXamlMember, Windows::UI::Xaml::Data::ICustomPropertyProvider, Windows::UI::Xaml::Data::INotifyPropertyChanged>
        {
            xaml_member(com_ptr<D>&& object, hstring const& name) :
                m_object(std::move(object)),
                m_name(name),
                m_binding{ bind_member(*m_object, m_name) }
            {
            }

            Windows::Foundation::IInspectable GetValue(Windows::Foundation::IInspectable const&)
            {
                if (m_binding.can_bind())
                {
                    return *this;
                }
                else
                {
                    return m_binding.get();
                }
            }

            void SetValue(Windows::Foundation::IInspectable const&, Windows::Foundation::IInspectable const& value)
            {
                m_binding.set(value);
                m_object->property_changed(m_name);
            }

            bool IsReadOnly() const
            {
                return false;
            }

            bool IsAttachable() const noexcept
            {
                return {};
            }

            bool IsDependencyProperty() const noexcept
            {
                return {};
            }

            hstring Name() const noexcept
            {
                return {};
            }

            auto Type() const noexcept
            {
                struct result
                {
                    operator Windows::UI::Xaml::Markup::IXamlType()
                    {
                        return {};
                    }
                    operator Windows::UI::Xaml::Interop::TypeName()
                    {
                        return {};
                    }
                };

                return result{};
            }

            Windows::UI::Xaml::Markup::IXamlType TargetType() const noexcept
            {
                return {};
            }

            Windows::UI::Xaml::Data::ICustomProperty GetCustomProperty(hstring const& member)
            {
                return make<bind_property>(get_strong(), m_binding.bind(member), member);
            }

            Windows::UI::Xaml::Data::ICustomProperty GetIndexedProperty(hstring const&, Windows::UI::Xaml::Interop::TypeName const&) const noexcept
            {
                return {};
            }

            hstring GetStringRepresentation() const noexcept
            {
                return {};
            }

            void property_changed(hstring const& name)
            {
                m_changed(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(name));
            }

            event_token PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
            {
                return m_changed.add(handler);
            }

            void PropertyChanged(winrt::event_token const& token)
            {
                m_changed.remove(token);
            }

        private:

            com_ptr<D> m_object;
            hstring const m_name;
            xaml_binding m_binding;
            event<Windows::UI::Xaml::Data::PropertyChangedEventHandler> m_changed;
        };

        struct xaml_type_instance : implements<xaml_type_instance, Windows::UI::Xaml::Markup::IXamlType>
        {
            hstring FullName() const
            {
                return D::GetRuntimeClassName();
            }

            Windows::Foundation::IInspectable ActivateInstance() const
            {
                return make<D>();
            }

            Windows::UI::Xaml::Markup::IXamlType BaseType() const
            {
                return nullptr;
            }

            bool IsConstructible() const
            {
                return true;
            }

            Windows::UI::Xaml::Interop::TypeName UnderlyingType() const
            {
                return {};
            }

            bool IsBindable() const
            {
                return true;
            }

            Windows::UI::Xaml::Markup::IXamlMember GetMember(hstring const& name) const
            {
                static std::map<hstring, Windows::UI::Xaml::Markup::IXamlMember> members;

                auto found = members.find(name);

                if (found != members.end())
                {
                    return found->second;
                }

                return members[name] = make<xaml_member>(s_last_type->get_strong(), name);
            }

            Windows::UI::Xaml::Markup::IXamlMember ContentProperty() const noexcept { return {}; }
            bool IsArray() const noexcept { return {}; }
            bool IsCollection() const noexcept { return {}; }
            bool IsDictionary() const noexcept { return {}; }
            bool IsMarkupExtension() const noexcept { return {}; }
            Windows::UI::Xaml::Markup::IXamlType ItemType() const noexcept { return {}; }
            Windows::UI::Xaml::Markup::IXamlType KeyType() const noexcept { return {}; }

            Windows::Foundation::IInspectable CreateFromString(hstring const& value) const noexcept
            {
                value;
                return {};
            }

            void AddToVector(Windows::Foundation::IInspectable const&, Windows::Foundation::IInspectable const&) const noexcept { }
            void AddToMap(Windows::Foundation::IInspectable const&, Windows::Foundation::IInspectable const&, Windows::Foundation::IInspectable const&) const noexcept { }
            void RunInitializer() const noexcept { }
        };

        static hstring type_name()
        {
            return {};
        }

        event<Windows::UI::Xaml::Data::PropertyChangedEventHandler> m_changed;
    };

    template <typename D, typename... I>
    struct registered_implements : implements<D, I...>, impl::xaml_registration<D, true>
    {
        static Windows::UI::Xaml::Markup::IXamlType get_type()
        {
            return make<xaml_type_instance>();
        }

        static hstring GetRuntimeClassName()
        {
            return D::type_name();
        }

    private:

        struct xaml_type_instance : implements<xaml_type_instance, Windows::UI::Xaml::Markup::IXamlType>
        {
            hstring FullName() const
            {
                return D::GetRuntimeClassName();
            }

            Windows::Foundation::IInspectable ActivateInstance() const
            {
                return make<D>();
            }

            Windows::UI::Xaml::Markup::IXamlType BaseType() const
            {
                return nullptr;
            }

            bool IsConstructible() const
            {
                return true;
            }

            Windows::UI::Xaml::Interop::TypeName UnderlyingType() const
            {
                return {};
            }

            bool IsBindable() const
            {
                return true;
            }

            Windows::UI::Xaml::Markup::IXamlMember GetMember(hstring const& name) const
            {
                name;
                return nullptr;
            }

            Windows::UI::Xaml::Markup::IXamlMember ContentProperty() const noexcept { return {}; }
            bool IsArray() const noexcept { return {}; }
            bool IsCollection() const noexcept { return {}; }
            bool IsDictionary() const noexcept { return {}; }
            bool IsMarkupExtension() const noexcept { return {}; }
            Windows::UI::Xaml::Markup::IXamlType ItemType() const noexcept { return {}; }
            Windows::UI::Xaml::Markup::IXamlType KeyType() const noexcept { return {}; }

            Windows::Foundation::IInspectable CreateFromString(hstring const& value) const noexcept
            {
                value;
                return {};
            }

            void AddToVector(Windows::Foundation::IInspectable const&, Windows::Foundation::IInspectable const&) const noexcept { }
            void AddToMap(Windows::Foundation::IInspectable const&, Windows::Foundation::IInspectable const&, Windows::Foundation::IInspectable const&) const noexcept { }
            void RunInitializer() const noexcept { }
        };
    };

    template <typename D, typename... I>
    using xaml_page = xaml_type<D, false, Windows::UI::Xaml::Controls::PageT, I...>;

    template <typename D, typename... I>
    using xaml_user_control = xaml_type<D, true, Windows::UI::Xaml::Controls::UserControlT, I...>;
}
