#pragma once
#include "xmeta_models.h"
#include <map>

namespace xlang::xmeta
{
    struct xlang_model_listener
    {
        virtual void listen_namespace_model(std::shared_ptr<namespace_model> const&) {};
        virtual void listen_class_model(std::shared_ptr<class_model> const&) {};
        virtual void listen_struct_model(std::shared_ptr<struct_model> const&) {};
        virtual void listen_interface_model(std::shared_ptr<interface_model> const&) {};
        virtual void listen_enum_model(std::shared_ptr<enum_model> const&) {};
        virtual void listen_delegate_model(std::shared_ptr<delegate_model> const&) {};
    };
}
