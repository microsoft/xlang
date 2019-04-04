#include <iostream>

#include "antlr4-runtime.h"
#include "XlangParser.h"
#include "XlangLexer.h"

#include <windows.h>
#include <mscoree.h>
#include <cor.h>

#pragma execution_character_set("utf-8")

using namespace antlr4;
// A generic assembly metadata struct.
const ASSEMBLYMETADATA s_genericMetadata =
    {
        // usMajorVersion - Unspecified major version
        0xFF,
        // usMinorVersion - Unspecified minor version
        0xFF,
        // usRevisionNumber - Unspecified revision number
        0xFF,
        // usBuildNumber - Unspecified build number
        0xFF,
        // szLocale - locale indepedence
        nullptr,
        // cbLocale
        0,
        // rProcessor - Processor independence
        nullptr,
        // ulProcessor
        0,
        // rOS - OS independence
        nullptr,
        // ulOS
        0,
    };
    
int main(int argc, const char * argv[]) {

    std::ifstream stream;
    printf("Opening %s \n", argv[1]);
    stream.open(argv[1]);

    ANTLRInputStream input(stream);
    XlangLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    XlangParser parser(&tokens);
    parser.setBuildParseTree(true);

    tree::ParseTree *tree = parser.xlang();
    std::string s = tree->toStringTree(&parser);
    std::cout << tree->toStringTree(&parser) << std::endl;

    std::cout << parser.getNumberOfSyntaxErrors() << std::endl;

    // This gets the token stream and prints out all the tokens corresponding to the file
    TokenStream * ts = parser.getTokenStream();
    for (int i = 0; i < ts->size(); i++)
    {
        // std::cout << ts->get(i)->getType() << std::endl;
    }
    
    // Getting the meta data dispenser
    IMetaDataDispenserEx *pMetaDataDispenser;
    HRESULT hr;
    CoInitialize(NULL);

    hr = CoCreateInstance(CLSID_CorMetaDataDispenser, 
        0, 
        CLSCTX_INPROC_SERVER, 
        IID_IMetaDataDispenser, 
        (void **)&pMetaDataDispenser);
        
    if (FAILED(hr)) {
        CoUninitialize();
        return 1;
    }
    
    // ToDo: Parse through the models 

    // SetModuleProps: Should be the name of hte file in which this module is stored. 

    // Saving to File
    LPCWSTR filename = L"test4.xmd";

    // Creating the assembly emitter
    IMetaDataAssemblyEmit *pMetaDataAssemblyEmitter;
    hr = pMetaDataDispenser->DefineScope(
        CLSID_CorMetaDataRuntime,
        0,
        IID_IMetaDataAssemblyEmit,
        (IUnknown **)&pMetaDataAssemblyEmitter);

    if (FAILED(hr)) {
        std::cout << "failed 1" << std::endl;
        pMetaDataDispenser->Release();
        CoUninitialize();
        return 1;
    }

    mdAssembly tokenAssembly;
    hr = pMetaDataAssemblyEmitter->DefineAssembly(
        NULL,
        0,
        0x8004,
        L"testingassembly",
        &(s_genericMetadata),
        0,
        &tokenAssembly);

    if (FAILED(hr)) {
        std::cout << "failed 2" << std::endl;
        pMetaDataDispenser->Release();
        CoUninitialize();
        return 1;
    }

    // Creating the Emitter
    IMetaDataEmit2 *pMetaDataEmitter;
    pMetaDataAssemblyEmitter->QueryInterface(IID_IMetaDataEmit2, (void **)&pMetaDataEmitter);

    if (FAILED(hr)) {
        pMetaDataDispenser->Release();
        CoUninitialize();
        return 1;
    }
    pMetaDataEmitter->Save(filename, 0);
    std::cout << "succeeded" << std::endl;
    return 0;
}
