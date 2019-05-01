#include "xlang_model_resolver.h"

namespace xlang::xmeta
{
    void xlang_model_resolver::listen_namespace_model(std::shared_ptr<namespace_model> const& model) {}
    void xlang_model_resolver::listen_class_model(std::shared_ptr<class_model> const& model) {}
    void xlang_model_resolver::listen_struct_model(std::shared_ptr<struct_model> const& model) {}
    void xlang_model_resolver::listen_interface_model(std::shared_ptr<interface_model> const& model) {}
    void xlang_model_resolver::listen_enum_model(std::shared_ptr<enum_model> const& model) {}
    void xlang_model_resolver::listen_delegate_model(std::shared_ptr<delegate_model> const& model) {}
}