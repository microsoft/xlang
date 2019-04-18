#include <iostream>

#include "antlr4-runtime.h"
#include "XlangParser.h"
#include "XlangLexer.h"

#include "xlang_model_walker.h"
#include "xmeta_emit.h"
#include "xmeta_models.h"
#include <windows.h>
#include <mscoree.h>
#include <cor.h>
#include <winrt/base.h>

#pragma execution_character_set("utf-8")

using namespace antlr4;
using namespace winrt;

int main(int argc, const char * argv[]) {

    //std::ifstream stream;
    //printf("Opening %s \n", argv[1]);
    //stream.open(argv[1]);

    //ANTLRInputStream input(stream);
    //XlangLexer lexer(&input);
    //CommonTokenStream tokens(&lexer);
    //XlangParser parser(&tokens);
    //parser.setBuildParseTree(true);

    //tree::ParseTree *tree = parser.xlang();
    //std::string s = tree->toStringTree(&parser);
    //std::cout << tree->toStringTree(&parser) << std::endl;

    //std::cout << parser.getNumberOfSyntaxErrors() << std::endl;

    std::vector<std::shared_ptr<xlang::xmeta::namespace_model>> v;

    xlang::xmeta::xlang_model_walker walker(v);
    std::shared_ptr<xlang::xmeta::xmeta_emit> emitter = std::make_shared<xlang::xmeta::xmeta_emit>(std::string("firsttest"));
    emitter->initialize();
    walker.register_listener(emitter);
    walker.walk();
    emitter->saveToFile();
    emitter->uninitialize();
    return 0;
}

//// Getting the meta data dispenser
//IMetaDataDispenserEx *pMetaDataDispenser;
//HRESULT hr;
//CoInitialize(NULL);
//
//hr = CoCreateInstance(CLSID_CorMetaDataDispenser,
//    0,
//    CLSCTX_INPROC_SERVER,
//    IID_IMetaDataDispenser,
//    (void **)&pMetaDataDispenser);
//
//if (FAILED(hr)) {
//    CoUninitialize();
//    return 1;
//}
//
//// ToDo: Parse through the models 
//
//// SetModuleProps: Should be the name of hte file in which this module is stored. 
//
//// Saving to File
//LPCWSTR filename = L"test4.xmd";
//
//// Creating the assembly emitter
//IMetaDataAssemblyEmit *pMetaDataAssemblyEmitter;
//hr = pMetaDataDispenser->DefineScope(
//    CLSID_CorMetaDataRuntime,
//    0,
//    IID_IMetaDataAssemblyEmit,
//    (IUnknown **)&pMetaDataAssemblyEmitter);
//
//if (FAILED(hr)) {
//    std::cout << "failed 1" << std::endl;
//    pMetaDataDispenser->Release();
//    CoUninitialize();
//    return 1;
//}
//
//mdAssembly tokenAssembly;
//hr = pMetaDataAssemblyEmitter->DefineAssembly(
//    NULL,
//    0,
//    0x8004,
//    L"testingassembly",
//    &(s_genericMetadata),
//    0,
//    &tokenAssembly);
//
//if (FAILED(hr)) {
//    std::cout << "failed 2" << std::endl;
//    pMetaDataDispenser->Release();
//    CoUninitialize();
//    return 1;
//}
//
//// Creating the Emitter
//IMetaDataEmit2 *pMetaDataEmitter;
//pMetaDataAssemblyEmitter->QueryInterface(IID_IMetaDataEmit2, (void **)&pMetaDataEmitter);
//
//if (FAILED(hr)) {
//    pMetaDataDispenser->Release();
//    CoUninitialize();
//    return 1;
//}
//pMetaDataEmitter->Save(filename, 0);
//std::cout << "succeeded" << std::endl;
//return 0;