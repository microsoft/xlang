#pragma once

#include <string>

#include "member_model.h"

namespace xlang::xmeta
{
    class method_model : member_model
    {
    public:

    protected:
        method_model() = delete;
        method_model(method_model const&) = delete;
        method_model& operator=(method_model const&) = delete;

        method_model(std::string name) noexcept : member_model(string) { }

    private:
        int arity;
    };
}