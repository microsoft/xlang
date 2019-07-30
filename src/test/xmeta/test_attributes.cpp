//#include "pch.h"
//#include "test_model_infrastructure.h"
//
//using namespace antlr4;
//using namespace xlang::xmeta;
//
//// 1 will print error, 0 will not
//#define PRINT_ERROR_FLAG 1
//
//TEST_CASE("Attribute test")
//{
//    std::istringstream test_idl{ R"(
//        namespace N
//        {
//            attribute HelpAttribute
//            {
//                HelpAttribute(String url);
//
//                String Topic;
//                String url;
//            }
//        }
//    )" };
//
//    xmeta_idl_reader reader{ "" };
//    reader.read(test_idl, PRINT_ERROR_FLAG);
//    REQUIRE(reader.get_num_syntax_errors() == 0);
//    REQUIRE(reader.get_num_semantic_errors() == 0);
//}
