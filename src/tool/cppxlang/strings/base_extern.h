
#ifdef _WIN32

extern "C"
{
    int32_t  WINRT_CALL WINRT_IIDFromString(wchar_t const* string, winrt::guid* iid) noexcept;
    void     WINRT_CALL WINRT_GetCurrentThreadStackLimits(uintptr_t* low_limit, uintptr_t* high_limit) noexcept;
}

WINRT_LINK(IIDFromString, 8)
WINRT_LINK(GetCurrentThreadStackLimits, 8)

#endif

extern "C"
{

    int32_t WINRT_CALL WINRT_RoGetActivationFactory(void* classId, winrt::guid const& iid, void** factory) noexcept;

    int32_t WINRT_CALL WINRT_WindowsCreateString(wchar_t const* sourceString, uint32_t length, void** string) noexcept;
    int32_t WINRT_CALL WINRT_WindowsCreateStringReference(wchar_t const* sourceString, uint32_t length, void* hstringHeader, void** string) noexcept;
    int32_t WINRT_CALL WINRT_WindowsDuplicateString(void* string, void** newString) noexcept;
    int32_t WINRT_CALL WINRT_WindowsDeleteString(void* string) noexcept;
    int32_t WINRT_CALL WINRT_WindowsStringHasEmbeddedNull(void* string, int* hasEmbedNull) noexcept;
    int32_t WINRT_CALL WINRT_WindowsPreallocateStringBuffer(uint32_t length, wchar_t** charBuffer, void** bufferHandle) noexcept;
    int32_t WINRT_CALL WINRT_WindowsDeleteStringBuffer(void* bufferHandle) noexcept;
    int32_t WINRT_CALL WINRT_WindowsPromoteStringBuffer(void* bufferHandle, void** string) noexcept;
    int32_t WINRT_CALL WINRT_WindowsConcatString(void* string1, void* string2, void** newString) noexcept;
    wchar_t const* WINRT_CALL WINRT_WindowsGetStringRawBuffer(void* string, uint32_t* length) noexcept;
    uint32_t WINRT_CALL WINRT_WindowsGetStringLen(void* string) noexcept;

    int32_t  WINRT_CALL WINRT_CoCreateInstance(winrt::guid const& clsid, void* outer, uint32_t context, winrt::guid const& iid, void** object) noexcept;
    int32_t  WINRT_CALL WINRT_CoGetCallContext(winrt::guid const& iid, void** object) noexcept;
    int32_t  WINRT_CALL WINRT_CoGetObjectContext(winrt::guid const& iid, void** object) noexcept;
    void*    WINRT_CALL WINRT_CoTaskMemAlloc(std::size_t size) noexcept;
    void     WINRT_CALL WINRT_CoTaskMemFree(void* ptr) noexcept;
    int32_t  WINRT_CALL WINRT_CloseHandle(void* hObject) noexcept;
    int32_t  WINRT_CALL WINRT_MultiByteToWideChar(uint32_t codepage, uint32_t flags, char const* in_string, int32_t in_size, wchar_t* out_string, int32_t out_size) noexcept;
    int32_t  WINRT_CALL WINRT_WideCharToMultiByte(uint32_t codepage, uint32_t flags, wchar_t const* int_string, int32_t in_size, char* out_string, int32_t out_size, char const* default_char, int32_t* default_used) noexcept;
    uint32_t WINRT_CALL WINRT_GetLastError() noexcept;
    void     WINRT_CALL WINRT_GetSystemTimePreciseAsFileTime(void* result) noexcept;

    uint32_t WINRT_CALL WINRT_WaitForSingleObject(void* handle, uint32_t milliseconds) noexcept;
    int32_t  WINRT_CALL WINRT_TrySubmitThreadpoolCallback(void(WINRT_CALL *callback)(void*, void* context), void* context, void*) noexcept;
    winrt::impl::ptp_timer WINRT_CALL WINRT_CreateThreadpoolTimer(void(WINRT_CALL *callback)(void*, void* context, void*), void* context, void*) noexcept;
    void     WINRT_CALL WINRT_SetThreadpoolTimer(winrt::impl::ptp_timer timer, void* time, uint32_t period, uint32_t window) noexcept;
    void     WINRT_CALL WINRT_CloseThreadpoolTimer(winrt::impl::ptp_timer timer) noexcept;
    winrt::impl::ptp_wait WINRT_CALL WINRT_CreateThreadpoolWait(void(WINRT_CALL *callback)(void*, void* context, void*, uint32_t result), void* context, void*) noexcept;
    void     WINRT_CALL WINRT_SetThreadpoolWait(winrt::impl::ptp_wait wait, void* handle, void* timeout) noexcept;
    void     WINRT_CALL WINRT_CloseThreadpoolWait(winrt::impl::ptp_wait wait) noexcept;
    winrt::impl::ptp_io WINRT_CALL WINRT_CreateThreadpoolIo(void* object, void(WINRT_CALL *callback)(void*, void* context, void* overlapped, uint32_t result, std::size_t bytes, void*) noexcept, void* context, void*) noexcept;
    void     WINRT_CALL WINRT_StartThreadpoolIo(winrt::impl::ptp_io io) noexcept;
    void     WINRT_CALL WINRT_CancelThreadpoolIo(winrt::impl::ptp_io io) noexcept;
    void     WINRT_CALL WINRT_CloseThreadpoolIo(winrt::impl::ptp_io io) noexcept;

    int32_t WINRT_CALL WINRT_CanUnloadNow() noexcept;
    int32_t WINRT_CALL WINRT_GetActivationFactory(void* classId, void** factory) noexcept;
}

WINRT_LINK(RoGetActivationFactory, 12)

WINRT_LINK(WindowsCreateString, 12)
WINRT_LINK(WindowsCreateStringReference, 16)
WINRT_LINK(WindowsDuplicateString, 8)
WINRT_LINK(WindowsDeleteString, 4)
WINRT_LINK(WindowsStringHasEmbeddedNull, 8)
WINRT_LINK(WindowsPreallocateStringBuffer, 12)
WINRT_LINK(WindowsDeleteStringBuffer, 4)
WINRT_LINK(WindowsPromoteStringBuffer, 8)
WINRT_LINK(WindowsConcatString, 12)
WINRT_LINK(WindowsGetStringRawBuffer, 8)
WINRT_LINK(WindowsGetStringLen, 4)

WINRT_LINK(CoCreateInstance, 20)
WINRT_LINK(CoGetCallContext, 8)
WINRT_LINK(CoGetObjectContext, 8)
WINRT_LINK(CoTaskMemAlloc, 4)
WINRT_LINK(CoTaskMemFree, 4)
WINRT_LINK(CloseHandle, 4)
WINRT_LINK(MultiByteToWideChar, 24)
WINRT_LINK(WideCharToMultiByte, 32)
WINRT_LINK(GetLastError, 0)
WINRT_LINK(GetSystemTimePreciseAsFileTime, 4)

WINRT_LINK(WaitForSingleObject, 8)
WINRT_LINK(TrySubmitThreadpoolCallback, 12)
WINRT_LINK(CreateThreadpoolTimer, 12)
WINRT_LINK(SetThreadpoolTimer, 16)
WINRT_LINK(CloseThreadpoolTimer, 4)
WINRT_LINK(CreateThreadpoolWait, 12)
WINRT_LINK(SetThreadpoolWait, 12)
WINRT_LINK(CloseThreadpoolWait, 4)
WINRT_LINK(CreateThreadpoolIo, 16)
WINRT_LINK(StartThreadpoolIo, 4)
WINRT_LINK(CancelThreadpoolIo, 4)
WINRT_LINK(CloseThreadpoolIo, 4)
