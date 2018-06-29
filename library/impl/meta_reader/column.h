
namespace xlang::meta::reader
{
    inline auto TypeDef::CustomAttribute() const
    {
        return equal_range(get_database().CustomAttribute, coded_index<HasCustomAttribute>());
    }

    inline auto TypeDef::GenericParam() const
    {
        return equal_range(get_database().GenericParam, coded_index<TypeOrMethodDef>());
    }

    inline auto TypeDef::InterfaceImpl() const
    {
        struct compare
        {
            bool operator()(uint32_t const left, reader::InterfaceImpl const& right) noexcept
            {
                return left < right.get_value<uint32_t>(0) - 1;
            }

            bool operator()(reader::InterfaceImpl const& left, uint32_t const right) noexcept
            {
                return left.get_value<uint32_t>(0) - 1 < right;
            }
        };

        return equal_range(get_database().InterfaceImpl, index(), compare{});
    }

    inline auto MethodDef::ParamList() const
    {
        auto first = get_database().Param.begin() + get_value<uint32_t>(5);
        auto last = get_database().Param.end();

        if (index() + 1 < get_database().MethodDef.size())
        {
            last = get_database().Param.begin() + get_database().MethodDef[index() + 1].get_value<uint32_t>(5);
        }

        return range{ std::pair{ first, last } };
    }

    inline auto InterfaceImpl::Class() const
    {
        return get_database().TypeDef[get_value<uint32_t>(0) - 1];
    }
}
