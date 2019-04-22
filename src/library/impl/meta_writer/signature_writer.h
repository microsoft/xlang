#pragma once

#include "../meta_reader/signature.h"

namespace xlang::meta::writer
{
    template <typename Iter>
    Iter compress_unsigned(uint32_t value, Iter iter)
    {
        if (value <= 0x7f)
        {
            *iter++ = static_cast<uint8_t>(value);
        }
        else if (value <= 0x3fff)
        {
            *iter++ = static_cast<uint8_t>((value >> 8) | 0x80);
            *iter++ = static_cast<uint8_t>(value & 0xff);
        }
        else if (value <= 0x1fffffff)
        {
            *iter++ = static_cast<uint8_t>((value >> 24) | 0xc0);
            *iter++ = static_cast<uint8_t>((value >> 16) & 0xff);
            *iter++ = static_cast<uint8_t>((value >> 8) & 0xff);
            *iter++ = static_cast<uint8_t>(value & 0xff);
        }
        else
        {
            throw_invalid("Attempted to compress an out of range integer");
        }
        return iter;
    }

    struct signature_blob
    {
        uint8_t const* data() const noexcept
        {
            return m_data.data();
        }

        uint32_t size() const noexcept
        {
            return static_cast<uint32_t>(m_data.size());
        }

        void add_compressed_unsigned(uint32_t value)
        {
            compress_unsigned(value, std::back_inserter(m_data));
        }

        template <typename T>
        void add_compressed_enum(T value)
        {
            static_assert(std::is_enum_v<T> && std::is_unsigned_v<std::underlying_type_t<T>>);
            add_compressed_unsigned(static_cast<std::underlying_type_t<T>>(value));
        }

        void add_null_string()
        {
            m_data.push_back(0xff);
        }

        void add_string(std::string_view value)
        {
            add_compressed_unsigned(static_cast<uint32_t>(value.size()));
            m_data.insert(m_data.end(), value.begin(), value.end());
        }

        template <typename T>
        void add_uncompressed(T value)
        {
            static_assert(std::is_arithmetic_v<T>);
            if constexpr (std::is_same_v<bool, std::decay_t<T>>)
            {
                m_data.push_back(value ? 1 : 0);
            }
            else
            {
                static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8);
                auto ptr = reinterpret_cast<uint8_t const*>(&value);
                m_data.insert(m_data.end(), ptr, ptr + sizeof(T));
            }
        }

        void add_type_ref(reader::coded_index<reader::TypeDefOrRef> const& value)
        {
            add_compressed_unsigned(value.raw_value());
        }

        void add_signature(reader::CustomModSig const& sig)
        {
            add_compressed_enum(sig.CustomMod());
            add_type_ref(sig.Type());
        }

        void add_signature(reader::GenericTypeInstSig const& sig)
        {
            add_compressed_enum(reader::ElementType::GenericInst);
            add_compressed_enum(sig.ClassOrValueType());
            add_type_ref(sig.GenericType());
            add_compressed_unsigned(sig.GenericArgCount());
            for (auto const& arg : sig.GenericArgs())
            {
                add_signature(arg);
            }
        }

        void add_signature(reader::TypeSig const& sig)
        {
            if (sig.is_szarray())
            {
                add_compressed_enum(reader::ElementType::SZArray);
            }
            call(sig.Type(),
                [&](reader::ElementType arg)
                {
                    add_compressed_enum(arg);
                },
                [&](reader::coded_index<reader::TypeDefOrRef> const& arg)
                {
                    add_type_ref(arg);
                },
                [&](reader::GenericTypeIndex const& arg)
                {
                    add_compressed_enum(reader::ElementType::Var);
                    add_compressed_unsigned(arg.index);
                },
                [&](reader::GenericTypeInstSig const& arg)
                {
                    add_signature(arg);
                },
                [&](reader::GenericMethodTypeIndex const& arg)
                {
                    add_compressed_enum(reader::ElementType::MVar);
                    add_compressed_unsigned(arg.index);
                }
            );
        }

        void add_signature(reader::FieldSig const& sig)
        {
            add_compressed_enum(sig.get_CallingConvention());
            add_signature(sig.Type());
        }

        void add_signature(reader::ParamSig const& sig)
        {
            for (auto const& cmod : sig.CustomMod())
            {
                add_signature(cmod);
                if (sig.ByRef())
                {
                    add_compressed_enum(reader::ElementType::ByRef);
                }
                add_signature(sig.Type());
            }
        }

        void add_signature(reader::RetTypeSig const& sig)
        {
            for (auto const& cmod : sig.CustomMod())
            {
                add_signature(cmod);
            }
            if (sig)
            {
                if (sig.ByRef())
                {
                    add_compressed_enum(reader::ElementType::ByRef);
                }
                add_signature(sig.Type());
            }
            else
            {
                add_compressed_enum(reader::ElementType::Void);
            }
        }

        void add_signature(reader::MethodDefSig const& sig)
        {
            add_compressed_enum(sig.CallConvention());
            if (enum_mask(sig.CallConvention(), reader::CallingConvention::Generic) == reader::CallingConvention::Generic)
            {
                add_compressed_unsigned(sig.GenericParamCount());
            }
            add_compressed_unsigned(static_cast<uint32_t>(reader::size(sig.Params())));
            add_signature(sig.ReturnType());
            for (auto const& param : sig.Params())
            {
                add_signature(param);
            }
        }

        void add_signature(reader::PropertySig const& sig)
        {
            add_compressed_enum(sig.CallConvention());
            add_compressed_unsigned(sig.ParamCount());
            for (auto const& cmod : sig.CustomMod())
            {
                add_signature(cmod);
            }
            add_signature(sig.Type());
            for (auto const& param : sig.Params())
            {
                add_signature(param);
            }
        }

        void add_signature(reader::TypeSpecSig const& sig)
        {
            add_signature(sig.GenericTypeInst());
        }

    private:
        std::vector<uint8_t> m_data;
    };
}
