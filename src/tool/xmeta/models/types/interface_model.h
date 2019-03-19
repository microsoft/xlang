#pragma once

#include <string>
#include <vector>

#include "type_model.h"
#include "interface_model.h"

namespace xlang::xmeta
{
    class interface_model : type_model
    {
    public:

    protected:
        interface_model() = delete;
        interface_model(interface_model const&) = delete;
        interface_model& operator=(interface_model const&) = delete;

        interface_model(std::string name) noexcept : type_model(string) { }

    private:
    };
}
