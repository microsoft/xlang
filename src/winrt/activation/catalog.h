#define RETURN_IF_FAILED( exp ) { HRESULT _hr_ = (exp); if( FAILED( _hr_ ) ) return _hr_; }

HRESULT WinRTLoadComponent(PCWSTR componentPath);

HRESULT WinRTGetThreadingModel(
	HSTRING activatableClassId,
	DWORD* threading_model);

HRESULT WinRTGetActivationFactory(
    HSTRING activatableClassId,
    REFIID  iid,
    void** factory);

HRESULT WinRTGetMetadataFile(
    const HSTRING        name,
    IMetaDataDispenserEx* metaDataDispenser,
    HSTRING* metaDataFilePath,
    IMetaDataImport2** metaDataImport,
    mdTypeDef* typeDefToken);

HRESULT WinRTResolveNamespaceDetour(
    const HSTRING name,
    const HSTRING windowsMetaDataDir,
    const DWORD   packageGraphDirsCount,
    const HSTRING* packageGraphDirs,
    DWORD* metaDataFilePathsCount,
    HSTRING** metaDataFilePaths,
    DWORD* subNamespacesCount,
    HSTRING** subNamespaces);