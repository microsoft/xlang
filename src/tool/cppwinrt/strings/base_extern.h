
extern "C"
{
    int32_t WINRT_CALL WINRT_GetRestrictedErrorInfo(void** info) noexcept;
    int32_t WINRT_CALL WINRT_RoGetActivationFactory(void* classId, winrt::guid const& iid, void** factory) noexcept;
    int32_t WINRT_CALL WINRT_RoInitialize(uint32_t type) noexcept;
    int32_t WINRT_CALL WINRT_RoOriginateLanguageException(int32_t error, void* message, void* exception) noexcept;
    void    WINRT_CALL WINRT_RoUninitialize() noexcept;
    int32_t WINRT_CALL WINRT_SetRestrictedErrorInfo(void* info) noexcept;
    int32_t WINRT_CALL WINRT_RoGetAgileReference(uint32_t options, winrt::guid const& iid, void* object, void** reference) noexcept;
    int32_t WINRT_CALL WINRT_CoIncrementMTAUsage(void** cookie) noexcept;

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

    int32_t  WINRT_CALL WINRT_CoCreateFreeThreadedMarshaler(void* outer, void** marshaler) noexcept;
    int32_t  WINRT_CALL WINRT_CoCreateInstance(winrt::guid const& clsid, void* outer, uint32_t context, winrt::guid const& iid, void** object) noexcept;
    int32_t  WINRT_CALL WINRT_CoGetCallContext(winrt::guid const& iid, void** object) noexcept;
    int32_t  WINRT_CALL WINRT_CoGetObjectContext(winrt::guid const& iid, void** object) noexcept;
    int32_t  WINRT_CALL WINRT_CoGetApartmentType(int32_t* type, int32_t* qualifier) noexcept;
    void*    WINRT_CALL WINRT_CoTaskMemAlloc(std::size_t size) noexcept;
    void     WINRT_CALL WINRT_CoTaskMemFree(void* ptr) noexcept;
    void     WINRT_CALL WINRT_SysFreeString(winrt::impl::bstr string) noexcept;
    uint32_t WINRT_CALL WINRT_SysStringLen(winrt::impl::bstr string) noexcept;
    int32_t  WINRT_CALL WINRT_IIDFromString(wchar_t const* string, winrt::guid* iid) noexcept;
    int32_t  WINRT_CALL WINRT_MultiByteToWideChar(uint32_t codepage, uint32_t flags, char const* in_string, int32_t in_size, wchar_t* out_string, int32_t out_size) noexcept;
    int32_t  WINRT_CALL WINRT_WideCharToMultiByte(uint32_t codepage, uint32_t flags, wchar_t const* int_string, int32_t in_size, char* out_string, int32_t out_size, char const* default_char, int32_t* default_used) noexcept;
    int32_t  WINRT_CALL WINRT_HeapFree(void* heap, uint32_t flags, void* value) noexcept;
    void*    WINRT_CALL WINRT_GetProcessHeap() noexcept;
    uint32_t WINRT_CALL WINRT_FormatMessageW(uint32_t flags, void const* source, uint32_t code, uint32_t language, wchar_t* buffer, uint32_t size, va_list* arguments) noexcept;
    uint32_t WINRT_CALL WINRT_GetLastError() noexcept;
    void     WINRT_CALL WINRT_GetSystemTimePreciseAsFileTime(void* result) noexcept;

    int32_t  WINRT_CALL WINRT_OpenProcessToken(void* process, uint32_t access, void** token) noexcept;
    void*    WINRT_CALL WINRT_GetCurrentProcess() noexcept;
    int32_t  WINRT_CALL WINRT_DuplicateToken(void* existing, uint32_t level, void** duplicate) noexcept;
    int32_t  WINRT_CALL WINRT_OpenThreadToken(void* thread, uint32_t access, int32_t self, void** token) noexcept;
    void*    WINRT_CALL WINRT_GetCurrentThread() noexcept;
    int32_t  WINRT_CALL WINRT_SetThreadToken(void** thread, void* token) noexcept;

    void    WINRT_CALL WINRT_AcquireSRWLockExclusive(winrt::impl::srwlock* lock) noexcept;
    void    WINRT_CALL WINRT_AcquireSRWLockShared(winrt::impl::srwlock* lock) noexcept;
    uint8_t WINRT_CALL WINRT_TryAcquireSRWLockExclusive(winrt::impl::srwlock* lock) noexcept;
    uint8_t WINRT_CALL WINRT_TryAcquireSRWLockShared(winrt::impl::srwlock* lock) noexcept;
    void    WINRT_CALL WINRT_ReleaseSRWLockExclusive(winrt::impl::srwlock* lock) noexcept;
    void    WINRT_CALL WINRT_ReleaseSRWLockShared(winrt::impl::srwlock* lock) noexcept;
    int32_t WINRT_CALL WINRT_SleepConditionVariableSRW(winrt::impl::condition_variable* cv, winrt::impl::srwlock* lock, uint32_t milliseconds, uint32_t flags) noexcept;
    void    WINRT_CALL WINRT_WakeConditionVariable(winrt::impl::condition_variable* cv) noexcept;
    void    WINRT_CALL WINRT_WakeAllConditionVariable(winrt::impl::condition_variable* cv) noexcept;
    void    WINRT_CALL WINRT_InitializeSListHead(void* head) noexcept;
    void*   WINRT_CALL WINRT_InterlockedPushEntrySList(void* head, void* entry) noexcept;
    void*   WINRT_CALL WINRT_InterlockedFlushSList(void* head) noexcept;

    void* WINRT_CALL WINRT_CreateEventW(void*, int32_t, int32_t, void*) noexcept;
    int32_t WINRT_CALL WINRT_SetEvent(void*) noexcept;
    int32_t  WINRT_CALL WINRT_CloseHandle(void* hObject) noexcept;
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
    winrt::impl::ptp_pool WINRT_CALL WINRT_CreateThreadpool(void* reserved) noexcept;
    void WINRT_CALL WINRT_SetThreadpoolThreadMaximum(winrt::impl::ptp_pool pool, uint32_t value) noexcept;
    int32_t WINRT_CALL WINRT_SetThreadpoolThreadMinimum(winrt::impl::ptp_pool pool, uint32_t value) noexcept;
    void     WINRT_CALL WINRT_CloseThreadpool(winrt::impl::ptp_pool pool) noexcept;

    int32_t WINRT_CALL WINRT_CanUnloadNow() noexcept;
    int32_t WINRT_CALL WINRT_GetActivationFactory(void* classId, void** factory) noexcept;
}

WINRT_LINK(GetRestrictedErrorInfo, 4)
WINRT_LINK(RoGetActivationFactory, 12)
WINRT_LINK(RoInitialize, 4)
WINRT_LINK(RoOriginateLanguageException, 12)
WINRT_LINK(RoUninitialize, 0)
WINRT_LINK(SetRestrictedErrorInfo, 4)
WINRT_LINK(RoGetAgileReference, 16)
WINRT_LINK(CoIncrementMTAUsage, 4)

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

WINRT_LINK(CoCreateFreeThreadedMarshaler, 8)
WINRT_LINK(CoCreateInstance, 20)
WINRT_LINK(CoGetCallContext, 8)
WINRT_LINK(CoGetObjectContext, 8)
WINRT_LINK(CoGetApartmentType, 8)
WINRT_LINK(CoTaskMemAlloc, 4)
WINRT_LINK(CoTaskMemFree, 4)
WINRT_LINK(SysFreeString, 4)
WINRT_LINK(SysStringLen, 4)
WINRT_LINK(IIDFromString, 8)
WINRT_LINK(MultiByteToWideChar, 24)
WINRT_LINK(WideCharToMultiByte, 32)
WINRT_LINK(HeapFree, 12)
WINRT_LINK(GetProcessHeap, 0)
WINRT_LINK(FormatMessageW, 28)
WINRT_LINK(GetLastError, 0)
WINRT_LINK(GetSystemTimePreciseAsFileTime, 4)

WINRT_LINK(OpenProcessToken, 12)
WINRT_LINK(GetCurrentProcess, 0)
WINRT_LINK(DuplicateToken, 12)
WINRT_LINK(OpenThreadToken, 16)
WINRT_LINK(GetCurrentThread, 0)
WINRT_LINK(SetThreadToken, 8)

WINRT_LINK(AcquireSRWLockExclusive, 4)
WINRT_LINK(AcquireSRWLockShared, 4)
WINRT_LINK(TryAcquireSRWLockExclusive, 4)
WINRT_LINK(TryAcquireSRWLockShared, 4)
WINRT_LINK(ReleaseSRWLockExclusive, 4)
WINRT_LINK(ReleaseSRWLockShared, 4)
WINRT_LINK(SleepConditionVariableSRW, 16)
WINRT_LINK(WakeConditionVariable, 4)
WINRT_LINK(WakeAllConditionVariable, 4)
WINRT_LINK(InitializeSListHead, 4)
WINRT_LINK(InterlockedPushEntrySList, 8)
WINRT_LINK(InterlockedFlushSList, 4)

WINRT_LINK(CreateEventW, 16)
WINRT_LINK(SetEvent, 4)
WINRT_LINK(CloseHandle, 4)
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
WINRT_LINK(CreateThreadpool, 4)
WINRT_LINK(SetThreadpoolThreadMaximum, 8)
WINRT_LINK(SetThreadpoolThreadMinimum, 8)
WINRT_LINK(CloseThreadpool, 4)
