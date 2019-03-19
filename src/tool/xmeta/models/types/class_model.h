#pragma once

#include <string>
#include <vector>

#include "type_model.h"
#include "interface_model.h"

namespace xlang::xmeta
{
    class class_model : type_model
    {
    public:

    protected:
        class_model() = delete;
        class_model(class_model const&) = delete;
        class_model& operator=(class_model const&) = delete;

        ~class_model() = default;

        class_model(std::string name) noexcept : type_model(string) { }

    private:
        std::unique_ptr<class_model> base_class;
        std::vector<std::shared_ptr<interface_model>> implemented_interfaces;

        // This will contain all methods, including constructors.
        std::vector<std::shared_ptr<method_model>> methods;
        std::vector<std::shared_ptr<property_model>> properties;
        std::vector<std::shared_ptr<event_model>> events;
        std::vector<std::shared_ptr<field_model>> fields;
    };
}
