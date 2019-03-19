#pragma once

#include <string>

namespace xlang::xmeta
{
    class member_model
    {
    public:

    protected:
        member_model() = delete;
        member_model(member_model const&) = delete;
        member_model& operator=(member_model const&) = delete;

        member_model(std::string name) noexcept : name{ name } {}

    private:
        std::string name;
    };
}