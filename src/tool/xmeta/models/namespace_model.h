#pragma once

#include <string>
#include <vector>

#include "types/type_model.h"

namespace xlang::xmeta
{
    class namespace_model
    {
    public:

    protected:
        namespace_model() = delete;
        namespace_model(namespace_model const&) = delete;
        namespace_model& operator=(namespace_model const&) = delete;

        namespace_model(std::string name) noexcept : name{ name } {}

    private:
        std::string name;
        std::vector<type_model> types;
    };
}