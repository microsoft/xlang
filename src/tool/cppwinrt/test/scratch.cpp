#include "pch.h"

namespace winrt::impl
{
    template <typename T, typename Container>
    struct convertible_observable_vector :
        observable_vector_base<convertible_observable_vector<T, Container>, T>,
        implements<convertible_observable_vector<T, Container>,
            wfc::IObservableVector<T>, wfc::IVector<T>, wfc::IVectorView<T>, wfc::IIterable<T>,
            wfc::IObservableVector<Windows::Foundation::IInspectable>, wfc::IVector<Windows::Foundation::IInspectable>, wfc::IVectorView<Windows::Foundation::IInspectable>, wfc::IIterable<Windows::Foundation::IInspectable>>
    {
        static_assert(!std::is_same_v<T, Windows::Foundation::IInspectable>);
        static_assert(std::is_same_v<Container, std::remove_reference_t<Container>>, "Must be constructed with rvalue.");

        using container_type = convertible_observable_vector<T, Container>;
        using base_type = observable_vector_base<convertible_observable_vector<T, Container>, T>;

        explicit convertible_observable_vector(Container&& values) : m_values(std::forward<Container>(values))
        {
        }

        auto& get_container() noexcept
        {
            return m_values;
        }

        auto& get_container() const noexcept
        {
            return m_values;
        }

        auto First()
        {
            struct result
            {
                container_type* owner;

                operator wfc::IIterator<T>()
                {
                    return static_cast<base_type*>(owner)->First();
                }

                operator wfc::IIterator<Windows::Foundation::IInspectable>()
                {
                    return make<iterator>(owner);
                }
            };

            return result{ this };
        }

        auto GetAt(uint32_t const index) const
        {
            struct result
            {
                base_type const* owner;
                uint32_t const index;

                operator T() const
                {
                    return owner->GetAt(index);
                }

                operator Windows::Foundation::IInspectable() const
                {
                    return box_value(owner->GetAt(index));
                }
            };

            return result{ this, index };
        }

        using base_type::IndexOf;

        bool IndexOf(Windows::Foundation::IInspectable const& value, uint32_t& index) const
        {
            return IndexOf(unbox_value<T>(value), index);
        }

        using base_type::GetMany;

        uint32_t GetMany(uint32_t const startIndex, array_view<Windows::Foundation::IInspectable> values) const
        {
            if (startIndex >= m_values.size())
            {
                return 0;
            }

            uint32_t const actual = (std::min)(static_cast<uint32_t>(m_values.size() - startIndex), values.size());

            std::transform(m_values.begin() + startIndex, m_values.begin() + startIndex + actual, values.begin(), [&](auto&& value)
                {
                    return box_value(value);
                });

            return actual;
        }

        auto GetView() const noexcept
        {
            struct result
            {
                container_type const* owner;

                operator wfc::IVectorView<T>() const
                {
                    return *owner;
                }

                operator wfc::IVectorView<Windows::Foundation::IInspectable>() const
                {
                    return *owner;
                }
            };

            return result{ this };
        }

        using base_type::SetAt;

        void SetAt(uint32_t const index, Windows::Foundation::IInspectable const& value)
        {
            SetAt(index, unbox_value<T>(value));
        }

        using base_type::InsertAt;

        void InsertAt(uint32_t const index, Windows::Foundation::IInspectable const& value)
        {
            InsertAt(index, unbox_value<T>(value));
        }

        using base_type::Append;

        void Append(Windows::Foundation::IInspectable const& value)
        {
            Append(unbox_value<T>(value));
        }

        using base_type::ReplaceAll;

        void ReplaceAll(array_view<Windows::Foundation::IInspectable const> values)
        {
            this->increment_version();
            m_values.clear();
            m_values.reserve(values.size());

            std::transform(values.begin(), values.end(), std::back_inserter(m_values), [&](auto&& value)
                {
                    return unbox_value<T>(value);
                });

            this->call_changed(Windows::Foundation::Collections::CollectionChange::Reset, 0);
        }

        using base_type::VectorChanged;

        event_token VectorChanged(wfc::VectorChangedEventHandler<Windows::Foundation::IInspectable> const& handler)
        {
            return base_type::VectorChanged([handler](auto&& sender, auto&& args)
                {
                    handler(sender.try_as<wfc::IObservableVector<Windows::Foundation::IInspectable>>(), args);
                });
        }

    private:

        struct iterator : impl::collection_version::iterator_type, implements<iterator, Windows::Foundation::Collections::IIterator<Windows::Foundation::IInspectable>>
        {
            void abi_enter()
            {
                m_owner->abi_enter();
                check_version(*m_owner);
            }

            void abi_exit()
            {
                m_owner->abi_exit();
            }

            explicit iterator(container_type* const owner) noexcept :
                impl::collection_version::iterator_type(*owner),
                m_current(owner->get_container().begin()),
                m_end(owner->get_container().end())
            {
                m_owner.copy_from(owner);
            }

            Windows::Foundation::IInspectable Current() const
            {
                if (m_current == m_end)
                {
                    throw hresult_out_of_bounds();
                }

                return box_value(*m_current);
            }

            bool HasCurrent() const noexcept
            {
                return m_current != m_end;
            }

            bool MoveNext() noexcept
            {
                if (m_current != m_end)
                {
                    ++m_current;
                }

                return HasCurrent();
            }

            uint32_t GetMany(array_view<Windows::Foundation::IInspectable> values)
            {
                uint32_t const actual = (std::min)(static_cast<uint32_t>(std::distance(m_current, m_end)), values.size());

                std::transform(m_current, m_current + actual, values.begin(), [&](auto&& value)
                    {
                        return box_value(value);
                    });

                std::advance(m_current, actual);
                return actual;
            }

        private:

            com_ptr<container_type> m_owner;
            decltype(m_owner->get_container().begin()) m_current;
            decltype(m_owner->get_container().end()) const m_end;
        };

        Container m_values;
    };
}

namespace winrt
{
    template <typename T, typename Allocator = std::allocator<T>>
    Windows::Foundation::Collections::IObservableVector<T> single_threaded_convertible_observable_vector(std::vector<T, Allocator> && values = {})
    {
        return make<impl::convertible_observable_vector<T, std::vector<T, Allocator>>>(std::move(values));
    }
}

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;

TEST_CASE("scratch")
{
    IIterable<int> a = make<impl::convertible_observable_vector<int, std::vector<int>>>(std::vector<int>{});

    IIterable<IInspectable> b = a.as<IIterable<IInspectable>>();

    IIterator<int> ai = a.First();

    IIterator<IInspectable> bi = b.First();
}
