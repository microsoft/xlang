#pragma once

#include <cctype>

namespace xlang
{
    using namespace std::experimental::filesystem;
    using namespace text;
    using namespace meta::reader;

    struct type_name
    {
        std::string_view name;
        std::string_view name_space;

        explicit type_name(std::string_view const& name, std::string_view const& name_space) :
            name(name),
            name_space(name_space)
        {
        }

        explicit type_name(TypeDef const& type) :
            name(type.TypeName()),
            name_space(type.TypeNamespace())
        {
        }

        explicit type_name(TypeRef const& type) :
            name(type.TypeName()),
            name_space(type.TypeNamespace())
        {
        }
    };

    enum type_system
    {
        java_descriptor,
        java_suffix,
        java_type,
        jni_type,
    };

    enum class convention
    {
        lower,
        upper,
        camel,
        mixed,
    };

    struct name_with_convention
    {
        std::string_view value;
        convention convention;
    };

    struct java_type_name : type_name
    {
        using type_name::type_name;
    };

    struct java_export 
    {
        std::string_view name_space;
    };

    bool operator==(type_name const& left, std::string_view const& right)
    {
        if (left.name.size() + 1 + left.name_space.size() != right.size())
        {
            return false;
        }

        if (right[left.name_space.size()] != '.')
        {
            return false;
        }

        if (0 != right.compare(left.name_space.size() + 1, left.name.size(), left.name))
        {
            return false;
        }

        return 0 == right.compare(0, left.name_space.size(), left.name_space);
    }

    static auto remove_tick(std::string_view const& name)
    {
        return name.substr(0, name.rfind('`'));
    }

    struct generic_iterator
    {
        std::string name;
        std::string cpp_type;
        std::string java_element;
    };

    inline bool operator<(generic_iterator const& left, generic_iterator const& right) noexcept
    {
        return left.name < right.name;
    }

    static std::optional<TypeDef> get_exclusive_to(TypeDef const& type)
    {
        if (auto exclusive_attr = get_attribute(type, "Windows.Foundation.Metadata", "ExclusiveToAttribute"))
        {
            auto sig = exclusive_attr.Value();
            auto const& fixed_args = sig.FixedArgs();
            XLANG_ASSERT(fixed_args.size() == 1);
            auto sys_type = std::get<ElemSig::SystemType>(std::get<ElemSig>(fixed_args[0].value).value);
            return type.get_cache().find_required(sys_type.name);
        }
        // Note: Some interfaces lack the ExclusiveTo attribute (e.g., IWwwFormUrlDecoderEntry).  We'll use a
        // naming convention to assume ExclusiveTo in these cases, and avoid unnecessary projection and casting.
        // E.g., Some::Namespace::IRuntimeClass is assumed to be ExclusiveTo Some::Namespace::RuntimeClass.
        if (auto probe = type.get_database().get_cache().find(type.TypeNamespace(), type.TypeName().substr(1)))
        {
            return probe;
        }
        return {};
    }

    struct writer : writer_base<writer>
    {
        using writer_base<writer>::write;

        std::string_view current_namespace;
        std::string_view current_type;
        type_system current_type_system{};
        bool param_names{};
        bool consume_types{};
        bool async_types{};
        std::set<generic_iterator> iterators;
        std::vector<std::string_view> unregisters;
        std::vector<std::vector<std::string>> generic_param_stack;

        struct type_system_guard
        {
            writer& owner;
            type_system last_type_system;
            type_system_guard(writer& owner, type_system type_system) noexcept : 
                owner(owner), last_type_system(owner.current_type_system)
            {
                owner.current_type_system = type_system;
            }
            ~type_system_guard() noexcept
            {
                owner.current_type_system = last_type_system;
            }
        };

        void write_as(type_system system, TypeSig const& sig)
        {
            type_system_guard guard(*this, system);
            write(sig.Type());
        }

        template <typename... Args>
        void write_as(type_system system, std::string_view const& value, Args const&... args)
        {
            type_system_guard guard(*this, system);
            write(value, args...);
        }

        struct generic_param_guard
        {
            explicit generic_param_guard(writer* arg = nullptr)
                : owner(arg)
            {}

            ~generic_param_guard()
            {
                if (owner)
                {
                    owner->generic_param_stack.pop_back();
                }
            }

            generic_param_guard(generic_param_guard && other)
                : owner(other.owner)
            {
                owner = nullptr;
            }

            generic_param_guard& operator=(generic_param_guard&& other)
            {
                owner = std::exchange(other.owner, nullptr);
                return *this;
            }

            generic_param_guard& operator=(generic_param_guard const&) = delete;
            writer* owner;
        };

        void add_iterator(generic_iterator&& it)
        {
            iterators.insert(std::move(it));
        }

        void add_unregister(std::string_view const& unregister)
        {
            unregisters.push_back(unregister);
        }

        [[nodiscard]] auto push_generic_params(std::pair<GenericParam, GenericParam> const& params)
        {
            if (empty(params))
            {
                return generic_param_guard{ nullptr };
            }

            std::vector<std::string> names;

            for (auto&& param : params)
            {
                names.push_back(std::string{ param.Name() });
            }

            generic_param_stack.push_back(std::move(names));
            return generic_param_guard{ this };
        }

        [[nodiscard]] auto push_generic_params(GenericTypeInstSig const& signature)
        {
            std::vector<std::string> names;

            for (auto&& arg : signature.GenericArgs())
            {
                names.push_back(write_temp("%", arg));
            }

            generic_param_stack.push_back(std::move(names));
            return generic_param_guard{ this };
        }

        void write_value(int32_t value)
        {
            write_printf("%d", value);
        }

        void write_value(uint32_t value)
        {
            write_printf("%#0x", value);
        }

        void write_escaped_namespace(std::string_view const& name_space, std::string_view const& separator)
        {
            for (auto&& c : name_space)
            {
                if (c == '.')
                {
                    write(separator);
                }
                else if (c == '`')
                {
                    return;
                }
                else
                {
                    write(c);
                }
            }
        }

        void write_code(std::string_view const& value)
        {
            write_escaped_namespace(value, "::");
        }

        void write(name_with_convention const& name)
        {
            auto write_chars = [&](auto&& transform)
            {
                for (auto c : name.value)
                {
                    if( auto x = static_cast<char>(transform(static_cast<unsigned char>(c))); x != '_')
                    {
                        write(x);
                    }
                }
            };

            auto index = 0;
            switch (name.convention)
            {
            case convention::lower:
                return write_chars([](unsigned char c)
                    {
                        return std::tolower(c);
                    });
            case convention::upper:
                return write_chars([](unsigned char c)
                    {
                        return std::toupper(c);
                    });
            case convention::camel:
                return write_chars([&](unsigned char c)
                    {
                        return index++ == 0 ? std::tolower(c) : c;
                    });
            case convention::mixed:
                return write_chars([&](unsigned char c)
                    {
                        return index++ == 0 ? std::toupper(c) : c;
                    });
            }
        }

        void write(java_type_name const& type_name)
        {
            write_escaped_namespace(type_name.name_space, "/");
            write("/");
            write(type_name.name);
        }
        
        void write(java_export const& java_export)
        {
            write("Java_");
            write_escaped_namespace(java_export.name_space, "_");
        }

        void write(Constant const& value)
        {
            switch (value.Type())
            {
            case ConstantType::Int32:
                write_value(value.ValueInt32());
                break;
            case ConstantType::UInt32:
                write_value(value.ValueUInt32());
                break;
            default:
                throw_invalid("Unexpected constant type");
            }
        }

        void write(TypeDef const& type)
        {
            if(auto exclusive_to = get_exclusive_to(type); exclusive_to.has_value())
            {
                write(exclusive_to.value());
                return;
            }

            auto name_space = std::string(type.TypeNamespace());
            if (name_space == current_namespace)
            {
                name_space = "";
            }
            else
            {
                name_space += ".";
            }
            auto name = type.TypeName();
            auto generics = type.GenericParam();

            if (!empty(generics))
            {
                //write("@::%<%>", name_space, remove_tick(name), bind_list(", ", generics));
                write("%%<%>", name_space, remove_tick(name), bind_list(", ", generics));
                return;
            }

            //write("@::%", name_space, name);
            write("%%", name_space, name);
        }

        void write(TypeRef const& type)
        {
            if (type_name(type) == "System.Guid")
            {
                write("winrt::guid");
            }
            else
            {
                write(find_required(type));
            }
        }

        void write(GenericParam const& param)
        {
            write(param.Name());
        }

        void write_java_descriptor(ElementType type)
        {
            if (type == ElementType::Void) { write("V"); }
            else if (type == ElementType::Boolean) { write("Z"); }
            else if (type == ElementType::Char) { write("C"); }
            else if (type == ElementType::I1) { write("B"); }
            else if (type == ElementType::U1) { write("B"); }
            else if (type == ElementType::I2) { write("S"); }
            else if (type == ElementType::U2) { write("S"); }
            else if (type == ElementType::I4) { write("I"); }
            else if (type == ElementType::U4) { write("I"); }
            else if (type == ElementType::I8) { write("J"); }
            else if (type == ElementType::U8) { write("J"); }
            else if (type == ElementType::R4) { write("F"); }
            else if (type == ElementType::R8) { write("D"); }
            else if (type == ElementType::String) { write("Ljava/lang/String;"); }
            else
            {
                write("LTODO-desc-ElementType;");
                //XLANG_ASSERT(false); 
            }
        }

        void write_java_suffix(ElementType type)
        {
            if (type == ElementType::Void) { write("V"); }
            else if (type == ElementType::Boolean) { write("Z"); }
            else if (type == ElementType::Char) { write("C"); }
            else if (type == ElementType::I1) { write("B"); }
            else if (type == ElementType::U1) { write("B"); }
            else if (type == ElementType::I2) { write("S"); }
            else if (type == ElementType::U2) { write("S"); }
            else if (type == ElementType::I4) { write("I"); }
            else if (type == ElementType::U4) { write("I"); }
            else if (type == ElementType::I8) { write("J"); }
            else if (type == ElementType::U8) { write("J"); }
            else if (type == ElementType::R4) { write("F"); }
            else if (type == ElementType::R8) { write("D"); }
            else if (type == ElementType::String) { write("$"); }
            else
            {
                write("LTODO-desc-brief-ElementType;");
                //XLANG_ASSERT(false); 
            }
        }

        void write_java_type(ElementType type)
        {
            if (type == ElementType::Void) { write("void"); }
            else if (type == ElementType::Boolean) { write("boolean"); }
            else if (type == ElementType::Char) { write("char"); }
            else if (type == ElementType::I1) { write("byte"); }
            else if (type == ElementType::U1) { write("byte"); }
            else if (type == ElementType::I2) { write("short"); }
            else if (type == ElementType::U2) { write("short"); }
            else if (type == ElementType::I4) { write("int"); }
            else if (type == ElementType::U4) { write("int"); }
            else if (type == ElementType::I8) { write("long"); }
            else if (type == ElementType::U8) { write("long"); }
            else if (type == ElementType::R4) { write("float"); }
            else if (type == ElementType::R8) { write("double"); }
            else if (type == ElementType::String) { write("String"); }
            else if (type == ElementType::Object) { write("Class"); }
            else
            {
                write("LTODO-java-ElementType;");
                //XLANG_ASSERT(false); 
            }
        }

        void write_jni_type(ElementType type)
        {
            if (type == ElementType::Void) { write("void"); }
            else if (type == ElementType::Boolean) { write("jboolean"); }
            else if (type == ElementType::Char) { write("jchar"); }
            else if (type == ElementType::I1) { write("jbyte"); }
            else if (type == ElementType::U1) { write("jbyte"); }
            else if (type == ElementType::I2) { write("jshort"); }
            else if (type == ElementType::U2) { write("jshort"); }
            else if (type == ElementType::I4) { write("jint"); }
            else if (type == ElementType::U4) { write("jint"); }
            else if (type == ElementType::I8) { write("jlong"); }
            else if (type == ElementType::U8) { write("jlong"); }
            else if (type == ElementType::R4) { write("jfloat"); }
            else if (type == ElementType::R8) { write("jdouble"); }
            else if (type == ElementType::String) { write("jstring"); }
            else if (type == ElementType::Object) { write("jobject"); }
            else
            {
                write("jlong");
                //XLANG_ASSERT(false); 
            }
        }

        void write(ElementType type)
        {
            switch (current_type_system)
            {
            case type_system::java_descriptor:
                write_java_descriptor(type);
                break;
            case type_system::java_suffix:
                write_java_suffix(type);
                break;
            case type_system::java_type:
                write_java_type(type);
                break;
            case type_system::jni_type:
                write_jni_type(type);
                break;
            }
        }

        void write(coded_index<TypeDefOrRef> const& type)
        {
            switch (type.type())
            {
            case TypeDefOrRef::TypeDef:
                switch (current_type_system)
                {
                case type_system::java_descriptor:
                case type_system::java_suffix:
                case type_system::java_type:
                    write("L%;", java_type_name{ type.TypeDef() });
                    break;
                case type_system::jni_type:
                    write("jlong");
                    break;
                }
                break;
            case TypeDefOrRef::TypeRef:
                switch (current_type_system)
                {
                case type_system::java_descriptor:
                case type_system::java_suffix:
                case type_system::java_type:
                    write(type.TypeRef());
                    break;
                case type_system::jni_type:
                    write("jlong");
                    break;
                }
                break;
            case TypeDefOrRef::TypeSpec:
                switch (current_type_system)
                {
                case type_system::java_descriptor:
                case type_system::java_suffix:
                case type_system::java_type:
                    write(type.TypeSpec().Signature().GenericTypeInst());
                    break;
                case type_system::jni_type:
                    write("jlong");
                    break;
                }
                break;
            }
        }

        void write(GenericTypeIndex var)
        {
            write(generic_param_stack.back()[var.index]);
        };

        void write(GenericTypeInstSig const& type)
        {
            switch (current_type_system)
            {
            case type_system::java_descriptor:
                break;
            case type_system::java_suffix:
                break;
            case type_system::java_type:
                break;
            case type_system::jni_type:
                break;
            }

            auto generic_type = type.GenericType().TypeRef();
            auto name_space = generic_type.TypeNamespace();
            auto name = generic_type.TypeName();
            name.remove_suffix(name.size() - name.rfind('`'));

            if (consume_types)
            {
                static constexpr std::string_view iterable("Windows::Foundation::Collections::IIterable<"sv);
                static constexpr std::string_view vector_view("Windows::Foundation::Collections::IVectorView<"sv);
                static constexpr std::string_view map_view("Windows::Foundation::Collections::IMapView<"sv);
                static constexpr std::string_view vector("Windows::Foundation::Collections::IVector<"sv);
                static constexpr std::string_view map("Windows::Foundation::Collections::IMap<"sv);

                consume_types = false;
                // auto full_name = write_temp("@::%<%>", name_space, name, bind_list(", ", type.GenericArgs()));
                auto full_name = write_temp("%.%<%>", name_space, name, bind_list(", ", type.GenericArgs()));
                consume_types = true;

                if (starts_with(full_name, iterable))
                {
                    if (async_types)
                    {
                        write("param::async_iterable%", full_name.substr(iterable.size() - 1));
                    }
                    else
                    {
                        write("param::iterable%", full_name.substr(iterable.size() - 1));
                    }
                }
                else if (starts_with(full_name, vector_view))
                {
                    if (async_types)
                    {
                        write("param::async_vector_view%", full_name.substr(vector_view.size() - 1));
                    }
                    else
                    {
                        write("param::vector_view%", full_name.substr(vector_view.size() - 1));
                    }
                }

                else if (starts_with(full_name, map_view))
                {
                    if (async_types)
                    {
                        write("param::async_map_view%", full_name.substr(map_view.size() - 1));
                    }
                    else
                    {
                        write("param::map_view%", full_name.substr(map_view.size() - 1));
                    }
                }
                else if (starts_with(full_name, vector))
                {
                    write("param::vector%", full_name.substr(vector.size() - 1));
                }
                else if (starts_with(full_name, map))
                {
                    write("param::map%", full_name.substr(map.size() - 1));
                }
                else
                {
                    write(full_name);
                }
            }
            else
            {
                //write("@::%<%>", name_space, name, bind_list(", ", type.GenericArgs()));
                write("%.%<%>", name_space, name, bind_list(", ", type.GenericArgs()));
            }
        }

        void write(TypeSig::value_type const& type)
        {
            call(type,[&](auto&& type)
            {
                write(type);
            });
        }

        void write(TypeSig const& signature)
        {
            //if (!abi_types && signature.is_szarray())
            //{
            //    write("com_array<%>", signature.Type());
            //}
            //else
            {
                write(signature.Type());
            }
        }

        void write(RetTypeSig const& value)
        {
            if (value)
            {
                write(value.Type());
            }
            else
            {
                write("void");
            }
        }

        void write(Field const& value)
        {
            write(value.Signature().Type());
        }
    };
}
