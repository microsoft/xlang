
namespace xlang::impl
{
    using ptp_io = struct tp_io*;
    using ptp_timer = struct tp_timer*;
    using ptp_wait = struct tp_wait*;
    using srwlock = struct srwlock_*;
    using condition_variable = struct condition_variable_*;
    using bstr = wchar_t*;

    inline bool is_guid_equal(uint32_t const* const left, uint32_t const* const right) noexcept
    {
        return left[0] == right[0] && left[1] == right[1] && left[2] == right[2] && left[3] == right[3];
    }
}

WINRT_EXPORT namespace xlang
{
    struct guid
    {
        uint32_t Data1;
        uint16_t Data2;
        uint16_t Data3;
        uint8_t  Data4[8];

        guid() noexcept = default;

        constexpr guid(uint32_t const Data1, uint16_t const Data2, uint16_t const Data3, std::array<uint8_t, 8> const& Data4) noexcept :
            Data1(Data1),
            Data2(Data2),
            Data3(Data3),
            Data4{ Data4[0], Data4[1], Data4[2], Data4[3], Data4[4], Data4[5], Data4[6], Data4[7] }
        {
        }

#ifdef WINRT_WINDOWS_ABI

        constexpr guid(GUID const& value) noexcept :
            Data1(value.Data1),
            Data2(value.Data2),
            Data3(value.Data3),
            Data4{ value.Data4[0], value.Data4[1], value.Data4[2], value.Data4[3], value.Data4[4], value.Data4[5], value.Data4[6], value.Data4[7] }
        {

        }

        operator GUID const&() const noexcept
        {
            return reinterpret_cast<GUID const&>(*this);
        }

#endif
    };

    inline bool operator==(guid const& left, guid const& right) noexcept
    {
        return impl::is_guid_equal(reinterpret_cast<uint32_t const*>(&left), reinterpret_cast<uint32_t const*>(&right));
    }

    inline bool operator!=(guid const& left, guid const& right) noexcept
    {
        return !(left == right);
    }
}

extern "C"
{
    int32_t WINRT_CALL WINRT_GetRestrictedErrorInfo(void** info) noexcept;
    int32_t WINRT_CALL WINRT_RoGetActivationFactory(void* classId, xlang::guid const& iid, void** factory) noexcept;
    int32_t WINRT_CALL WINRT_RoInitialize(uint32_t type) noexcept;
    int32_t WINRT_CALL WINRT_RoOriginateLanguageException(int32_t error, void* message, void* exception) noexcept;
    void    WINRT_CALL WINRT_RoUninitialize() noexcept;
    int32_t WINRT_CALL WINRT_SetRestrictedErrorInfo(void* info) noexcept;
    int32_t WINRT_CALL WINRT_RoGetAgileReference(uint32_t options, xlang::guid const& iid, void* object, void** reference) noexcept;
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
    int32_t  WINRT_CALL WINRT_CoCreateInstance(xlang::guid const& clsid, void* outer, uint32_t context, xlang::guid const& iid, void** object) noexcept;
    int32_t  WINRT_CALL WINRT_CoGetCallContext(xlang::guid const& iid, void** object) noexcept;
    int32_t  WINRT_CALL WINRT_CoGetObjectContext(xlang::guid const& iid, void** object) noexcept;
    int32_t  WINRT_CALL WINRT_CoGetApartmentType(int32_t* type, int32_t* qualifier) noexcept;
    void*    WINRT_CALL WINRT_CoTaskMemAlloc(std::size_t size) noexcept;
    void     WINRT_CALL WINRT_CoTaskMemFree(void* ptr) noexcept;
    void     WINRT_CALL WINRT_SysFreeString(xlang::impl::bstr string) noexcept;
    uint32_t WINRT_CALL WINRT_SysStringLen(xlang::impl::bstr string) noexcept;
    int32_t  WINRT_CALL WINRT_IIDFromString(wchar_t const* string, xlang::guid* iid) noexcept;
    int32_t  WINRT_CALL WINRT_CloseHandle(void* hObject) noexcept;
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

    void    WINRT_CALL WINRT_AcquireSRWLockExclusive(xlang::impl::srwlock* lock) noexcept;
    void    WINRT_CALL WINRT_AcquireSRWLockShared(xlang::impl::srwlock* lock) noexcept;
    uint8_t WINRT_CALL WINRT_TryAcquireSRWLockExclusive(xlang::impl::srwlock* lock) noexcept;
    uint8_t WINRT_CALL WINRT_TryAcquireSRWLockShared(xlang::impl::srwlock* lock) noexcept;
    void    WINRT_CALL WINRT_ReleaseSRWLockExclusive(xlang::impl::srwlock* lock) noexcept;
    void    WINRT_CALL WINRT_ReleaseSRWLockShared(xlang::impl::srwlock* lock) noexcept;
    int32_t WINRT_CALL WINRT_SleepConditionVariableSRW(xlang::impl::condition_variable* cv, xlang::impl::srwlock* lock, uint32_t milliseconds, uint32_t flags) noexcept;
    void    WINRT_CALL WINRT_WakeConditionVariable(xlang::impl::condition_variable* cv) noexcept;
    void    WINRT_CALL WINRT_WakeAllConditionVariable(xlang::impl::condition_variable* cv) noexcept;
    void    WINRT_CALL WINRT_InitializeSListHead(void* head) noexcept;
    void*   WINRT_CALL WINRT_InterlockedPushEntrySList(void* head, void* entry) noexcept;
    void*   WINRT_CALL WINRT_InterlockedFlushSList(void* head) noexcept;

    uint32_t WINRT_CALL WINRT_WaitForSingleObject(void* handle, uint32_t milliseconds) noexcept;
    int32_t  WINRT_CALL WINRT_TrySubmitThreadpoolCallback(void(WINRT_CALL *callback)(void*, void* context), void* context, void*) noexcept;
    xlang::impl::ptp_timer WINRT_CALL WINRT_CreateThreadpoolTimer(void(WINRT_CALL *callback)(void*, void* context, void*), void* context, void*) noexcept;
    void     WINRT_CALL WINRT_SetThreadpoolTimer(xlang::impl::ptp_timer timer, void* time, uint32_t period, uint32_t window) noexcept;
    void     WINRT_CALL WINRT_CloseThreadpoolTimer(xlang::impl::ptp_timer timer) noexcept;
    xlang::impl::ptp_wait WINRT_CALL WINRT_CreateThreadpoolWait(void(WINRT_CALL *callback)(void*, void* context, void*, uint32_t result), void* context, void*) noexcept;
    void     WINRT_CALL WINRT_SetThreadpoolWait(xlang::impl::ptp_wait wait, void* handle, void* timeout) noexcept;
    void     WINRT_CALL WINRT_CloseThreadpoolWait(xlang::impl::ptp_wait wait) noexcept;
    xlang::impl::ptp_io WINRT_CALL WINRT_CreateThreadpoolIo(void* object, void(WINRT_CALL *callback)(void*, void* context, void* overlapped, uint32_t result, std::size_t bytes, void*) noexcept, void* context, void*) noexcept;
    void     WINRT_CALL WINRT_StartThreadpoolIo(xlang::impl::ptp_io io) noexcept;
    void     WINRT_CALL WINRT_CancelThreadpoolIo(xlang::impl::ptp_io io) noexcept;
    void     WINRT_CALL WINRT_CloseThreadpoolIo(xlang::impl::ptp_io io) noexcept;

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
WINRT_LINK(CloseHandle, 4)
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
