#pragma once

#include <string>
#include <map>
#include <memory>
#include <vector>

#include "base_model.h"
#include "class_model.h"
#include "struct_model.h"


namespace xlang
{
    namespace xmeta
    {
        struct namespace_body_model;
        class namespace_model;

        typedef std::vector<std::shared_ptr<namespace_model>> namespace_vector;
        typedef std::map<std::string, std::shared_ptr<namespace_model>> child_ns_map;

        // The scope of a using directive extends over the namespace member declarations of
        // its immediately containing namespace body. The namespace_body_model struct
        // will be used to differentiate the separate bodies.
        struct namespace_body_model
        {
            // Using directives
            std::map<std::string, std::string> using_alias_directives;
            std::vector<std::string> using_namespace_directives;

            // Members
            std::map<std::string, std::shared_ptr<class_model>> classes;
            std::map<std::string, std::shared_ptr<struct_model>> structs;
            std::map<std::string, std::shared_ptr<interface_model>> interfaces;

            std::shared_ptr<namespace_model> containing_namespace;

            // Methods
            bool member_id_exists(const std::string &member_name);
            std::string get_full_namespace_name();
        };

        class namespace_model final : public base_model
        {
        public:
            namespace_model(const std::string &id, const size_t &decl_line, const std::shared_ptr<namespace_model> &pn);

            // Used for semantic check #3 for namespace members
            bool member_id_exists(const std::string &member_id);
            std::string get_full_namespace_name();

            std::shared_ptr<namespace_model> parent_namespace;

            // Members
            child_ns_map child_namespaces;

            // Vector of different namespace bodies defined.
            std::vector<std::shared_ptr<namespace_body_model>> ns_bodies;


        protected:
            namespace_model() = delete;

        private:
            //std::vector<class_model> classes;
            //std::vector<namespace_name_type> using_namespace_directives;
        };
    }
}
