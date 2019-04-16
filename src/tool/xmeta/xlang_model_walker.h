#pragma once
#include "xmeta_models.h"
#include "xlang_model_listener.h"

namespace xlang::xmeta
{
    class xlang_model_walker
    {
    public:

        xlang_model_walker(std::vector<std::shared_ptr<namespace_model>> namespaces)
            : m_namespaces(namespaces)
        { };

        void register_listener(std::shared_ptr<xlang_model_listener> const& listener);
        void walk();
        
        xlang_model_walker() = delete;

    private:
        std::shared_ptr<xlang_model_listener> m_listener;
        std::vector<std::shared_ptr<namespace_model>> m_namespaces;

        void enter_namespace_model(std::shared_ptr<namespace_model> const& model);
        void enter_class_model(std::shared_ptr<class_model> const& model);
        void enter_struct_model(std::shared_ptr<struct_model> const& model);
        void enter_interface_model(std::shared_ptr<interface_model> const& model);
        void enter_enum_model(std::shared_ptr<enum_model> const& model);
        void enter_delegate_model(delegate_model const& model);

        void enter_method_model(std::shared_ptr<method_model> const& model);
        void enter_property_model(std::shared_ptr<property_model> const& model);
        void enter_event_model(std::shared_ptr<event_model> const& model);
    };
}