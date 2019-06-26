
namespace winrt::impl
{
    struct binding_base;
}

namespace winrt
{
    struct binding
    {
        binding() noexcept = default;

        // TODO: add overloads that take leading strong/weak reference
        template <typename T, typename = std::enable_if_t<!std::is_base_of_v<Windows::Foundation::IUnknown, T>>>
        binding(T& reference);

        template <typename T, typename = std::enable_if_t<std::is_base_of_v<Windows::Foundation::IUnknown, T>>>
        binding(T const& object);

        template <typename Get, typename = std::enable_if_t<std::is_invocable_v<Get>>>
        binding(Get && get_self);

        template <typename Get, typename Set>
        binding(Get&& get_self, Set&& set_self);

        auto get() const;
        auto set(Windows::Foundation::IInspectable const& value) const;
        auto can_bind() const;
        auto bind(hstring const& name) const;
        auto type() const;

    private:

        std::unique_ptr<impl::binding_base> m_binding;
    };
}
