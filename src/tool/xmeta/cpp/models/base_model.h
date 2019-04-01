#pragma once

#include <string>

namespace xlang
{
    namespace xmeta
    {
        class base_model
        {
        public:
            std::string id;
            size_t decl_line;

            base_model(const std::string &id, const size_t &decl_line) : id{ id }, decl_line{ decl_line } { }

        protected:
            base_model() = delete;

        };

        enum value_t
        {
            boolean_type,
            string_type,
            int16_type,
            int32_type,
            int64_type,
            uint8_type,
            uint16_type,
            uint32_type,
            uint64_type,
            char16_type,
            guid_type,
            single_type,
            double_type,
        };

        struct xmeta_type
        {
            bool is_void;
            bool is_array;
            bool is_class; // if (!is_class) { return type is value type (string, int16, int32, etc...) }
            std::string class_id;
            value_t value_type;
        };

        enum parameter_modifier_t
        {
            no_param_modifier,
            const_ref,
            out
        };

        struct formal_parameter_t
        {
            parameter_modifier_t parameter_modifier;
            xmeta_type parameter_type;
            std::string id;
        };
    }
}
