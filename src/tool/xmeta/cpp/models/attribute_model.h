#pragma once

#include <string_view>

#include "model_types.h" 

namespace xlang::xmeta
{
    enum class AttributeTargets
    {
        Assembly = 0x0001,
        Module = 0x0002,
        Class = 0x0004,
        Struct = 0x0008,
        Enum = 0x0010,
        Constructor = 0x0020,
        Method = 0x0040,
        Property = 0x0080,
        Field = 0x0100,
        Event = 0x0200,
        Interface = 0x0400,
        Parameter = 0x0800,
        Delegate = 0x1000,
        ReturnValue = 0x2000,
        GenericParameter = 0x4000,


        All = Assembly | Module | Class | Struct | Enum | Constructor |
        Method | Property | Field | Event | Interface | Parameter |
        Delegate | ReturnValue | GenericParameter,
    };

    using attribute_parameter_value_semantics = std::variant<bool, int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t, std::string>;

    struct attribute_member
    {
        attribute_member() = delete;
        attribute_member(std::string_view const& str_val) :
            m_value{ str_val }
        {
        }

        std::errc resolve_value(type_ref type)
        {
            return std::errc();
        }

    private:
        model_ref<attribute_parameter_value_semantics> m_value;

    };


    struct attribute_type_model : base_model
    {
        attribute_type_model() = delete;

        attribute_type_model(std::string_view const& name,
            size_t decl_line,
            std::string_view const& assembly_name) :
            base_model{ name, decl_line, assembly_name }
        { }

        auto const& get_named_parameters() const noexcept
        {
            return m_named_parameter;
        }

        auto const& get_positonal_parameters() const noexcept
        {
            return m_positonal_parameter;
        }

        void add_named_parameter(std::string name, type_ref&& type)
        {
            m_named_parameter.emplace(name, type);
        }

        void add_positonal_parameter(type_ref&& field)
        {
            m_positonal_parameter.emplace_back(std::move(field));
        }

    private:
        std::map<std::string, type_ref> m_named_parameter;
        std::vector<type_ref> m_positonal_parameter;

        bool allow_multiple = false;
        bool inherited = false;
    };

    struct attribute_model
    {
        static std::shared_ptr<attribute_model> create_attribute(
            std::shared_ptr<attribute_type_model> const& attribute_class, 
            std::vector<attribute_member> & positoned_parameter, 
            std::map<std::string, attribute_member> & named_parameter)
        {
            // check that sizing is the same
            if (attribute_class->get_positonal_parameters().size() != positoned_parameter.size()
                || attribute_class->get_named_parameters().size() != named_parameter.size())
            {
                return nullptr;
            }

            for (size_t i = 0; i < attribute_class->get_positonal_parameters().size(); i++)
            {
                type_ref const& attr_class_param = attribute_class->get_positonal_parameters().at(i);
                if (positoned_parameter.at(i).resolve_value(attr_class_param) != std::errc())
                {
                    return nullptr;
                }
            }
            size_t i = 0;
            for (auto const& attribute_class_named_param_entry : attribute_class->get_named_parameters())
            {
                auto const& search = named_parameter.find(attribute_class_named_param_entry.first);
                type_ref const& attr_named_class_param = attribute_class_named_param_entry.second;
                if (search != named_parameter.end())
                {
                    if (search->second.resolve_value(attr_named_class_param) != std::errc())
                    {
                        return nullptr;
                    }
                }
                else
                {
                    return nullptr;
                }
            }
            //return nullptr;
            return std::make_shared<attribute_model>(attribute_class, positoned_parameter, named_parameter);
        }

        attribute_model(std::shared_ptr<attribute_type_model> const& attribute_class,
            std::vector<attribute_member> const& positoned_parameter,
            std::map<std::string, attribute_member> const& named_parameter) :
            m_attribute_class{ attribute_class }, m_positoned_parameter{ positoned_parameter }, m_named_parameter{ named_parameter }
        { }

        attribute_model() = delete;

    private:
        std::shared_ptr<attribute_type_model> m_attribute_class;
        std::map<std::string, attribute_member> m_named_parameter;
        std::vector<attribute_member> m_positoned_parameter;
    };
}
