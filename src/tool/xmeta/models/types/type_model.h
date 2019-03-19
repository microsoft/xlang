#pragma once

#include <string>

namespace xlang::xmeta
{
    class type_model
    {
    public:

    protected:
        type_model() = delete;
        type_model(type_model const&) = delete;
        type_model& operator=(type_model const&) = delete;

        type_model(std::string name) noexcept : name{ name } {}

    private:
        std::string name;
    };
}