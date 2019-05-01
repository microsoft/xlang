#pragma once
#include "xmeta_models.h"
#include "xlang_model_listener.h"

namespace xlang::xmeta
{
    class xlang_model_resolver : public xlang_model_listener
    {
    public:
        xlang_model_resolver() {};
        void listen_namespace_model(std::shared_ptr<namespace_model> const& model) final;
        void listen_class_model(std::shared_ptr<class_model> const& model) final;
        void listen_struct_model(std::shared_ptr<struct_model> const& model) final;
        void listen_interface_model(std::shared_ptr<interface_model> const& model) final;
        void listen_enum_model(std::shared_ptr<enum_model> const& model) final;
        void listen_delegate_model(std::shared_ptr<delegate_model> const& model) final;
    };
}