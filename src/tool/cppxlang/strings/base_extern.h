
#ifdef _WIN32

extern "C"
{
    int32_t  WINRT_CALL WINRT_IIDFromString(wchar_t const* string, winrt::guid* iid) noexcept;
    void     WINRT_CALL WINRT_GetCurrentThreadStackLimits(uintptr_t* low_limit, uintptr_t* high_limit) noexcept;
	uint32_t WINRT_CALL WINRT_GetLastError() noexcept;
	int32_t  WINRT_CALL WINRT_CoGetObjectContext(winrt::guid const& iid, void** object) noexcept;

	uint32_t WINRT_CALL WINRT_WaitForSingleObject(void* handle, uint32_t milliseconds) noexcept;
	int32_t  WINRT_CALL WINRT_TrySubmitThreadpoolCallback(void(WINRT_CALL *callback)(void*, void* context), void* context, void*) noexcept;
	winrt::impl::ptp_timer WINRT_CALL WINRT_CreateThreadpoolTimer(void(WINRT_CALL *callback)(void*, void* context, void*), void* context, void*) noexcept;
	void     WINRT_CALL WINRT_SetThreadpoolTimer(winrt::impl::ptp_timer timer, void* time, uint32_t period, uint32_t window) noexcept;
	void     WINRT_CALL WINRT_CloseThreadpoolTimer(winrt::impl::ptp_timer timer) noexcept;
	winrt::impl::ptp_wait WINRT_CALL WINRT_CreateThreadpoolWait(void(WINRT_CALL *callback)(void*, void* context, void*, uint32_t result), void* context, void*) noexcept;
	void     WINRT_CALL WINRT_SetThreadpoolWait(winrt::impl::ptp_wait wait, void* handle, void* timeout) noexcept;
	void     WINRT_CALL WINRT_CloseThreadpoolWait(winrt::impl::ptp_wait wait) noexcept;
}

WINRT_LINK(IIDFromString, 8)
WINRT_LINK(GetCurrentThreadStackLimits, 8)
WINRT_LINK(GetLastError, 0)
WINRT_LINK(CoGetObjectContext, 8)

WINRT_LINK(WaitForSingleObject, 8)
WINRT_LINK(TrySubmitThreadpoolCallback, 12)
WINRT_LINK(CreateThreadpoolTimer, 12)
WINRT_LINK(SetThreadpoolTimer, 16)
WINRT_LINK(CloseThreadpoolTimer, 4)
WINRT_LINK(CreateThreadpoolWait, 12)
WINRT_LINK(SetThreadpoolWait, 12)
WINRT_LINK(CloseThreadpoolWait, 4)

#endif

extern "C"
{
    int32_t WINRT_CALL WINRT_GetActivationFactory(void* classId, void** factory) noexcept;
}
