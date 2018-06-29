#include "winrt/base.h"

enum class hstring_type : uint32_t
{
    ref_counted,
    string_reference,
    prealloc_string_buffer,
};

struct hstring_header
{
    hstring_header() 
        : hstring_header(hstring_type::string_reference, nullptr, 0)
    {
    }

    hstring_header(hstring_type type, char16_t const* buffer, uint32_t size)
        : _type(type), _buffer(buffer), _size(size)
    {
    }

    hstring_type type() const noexcept
    {
        return _type;
    }
    uint32_t size() const noexcept
    {
        return _size;
    }

    char16_t const* data() const noexcept
    {
        return _buffer; 
    }

    bool has_embedded_null() const noexcept
    {
        for (auto c : std::u16string_view{_buffer, _size})
        {
            if (c == u'\0')
            {
                return true;
            }
        }

        return false;
    }

protected:
    void type(hstring_type type) noexcept
    {
        _type = type;
    }

private:
    hstring_type _type;
    uint32_t _size;
    char16_t const* _buffer;
};

// TODO: renable static_assert
// static_assert(sizeof(winrt::param::hstring::header) == sizeof(hstring_header));

struct refcounted_hstring : public hstring_header
{
    uint32_t add_ref() noexcept
    {
        return 1 + m_references.fetch_add(1, std::memory_order_relaxed);
    }

    uint32_t release_ref() noexcept
    {
        uint32_t const remaining = m_references.fetch_sub(1, std::memory_order_release) - 1;

        if (remaining == 0)
        {
            this->dealloc();
        }

        return remaining;
    }

    static refcounted_hstring * alloc(hstring_type type, uint32_t size)
    {
        if (size == 0)
        {
            return nullptr;
        }

        void * raw = ::operator new(sizeof(refcounted_hstring) + (sizeof(char16_t )* size));

        if (raw == nullptr)
        {
            throw std::bad_alloc();
        }

        return new(raw) refcounted_hstring(type, size);
    } 

    static void copy(refcounted_hstring * str, std::u16string_view view, uint32_t pos = 0)
    {
        std::memcpy(str->_data() + pos, view.data(), sizeof(char16_t) * view.size());
    }

    char16_t* _data() noexcept
    {
        return reinterpret_cast<char16_t*>(this + 1);
    }

    void promote()
    {
        if (type() != hstring_type::prealloc_string_buffer)
        {
            throw std::invalid_argument("this");
        }

        type(hstring_type::ref_counted);
    }

    void dealloc()
    {
        std::atomic_thread_fence(std::memory_order_acquire);
        this->~refcounted_hstring();
        ::operator delete(static_cast<void*>(this));
    }

private:
    refcounted_hstring(hstring_type type, uint32_t size) 
        : hstring_header(type, _data(), size)
    {
        _data()[size] = u'\0';
    }   

    std::atomic<uint32_t> m_references{ 1 };
};

refcounted_hstring * make_string(char16_t const * source, uint32_t size)
{
    if (!source && size != 0)
    {
        throw std::invalid_argument("source");
    }

    if (size == 0)
    {
        return nullptr;
    }

    auto str = refcounted_hstring::alloc(hstring_type::ref_counted, size);
    refcounted_hstring::copy(str, {source, size});
    return str;
}

void make_string_reference(char16_t const * source, uint32_t size, hstring_header * header)
{
    if (!header)
    {
        throw std::invalid_argument("header");
    }

    if (!source && size > 0)
    {
        throw std::invalid_argument("source");
    }

    if (source && source[size] != u'\0')
    {
        throw winrt::hresult_error(0x80000017 /*E_STRING_NOT_NULL_TERMINATED*/);
    }

    *header = {hstring_type::string_reference, source, size};
}

refcounted_hstring * make_prealloc_buffer(uint32_t size)
{
    if (size == 0)
    {
        return nullptr;
    }
    
    return refcounted_hstring::alloc(hstring_type::prealloc_string_buffer, size);
}

void delete_string(hstring_header * string)
{
    if (string)
    {
        if (string->type() == hstring_type::prealloc_string_buffer)
        {
            throw std::invalid_argument("string");
        }
        
        if (string->type() == hstring_type::ref_counted)
        {
            auto rc_string = static_cast<refcounted_hstring*>(string);
            rc_string->release_ref();
        }
    }
}

void delete_prealloc_buffer(refcounted_hstring * string)
{
    if (string)
    {
        if (string->type() != hstring_type::prealloc_string_buffer)
        {
            throw std::invalid_argument("string");
        }

        string->dealloc();
    }
}

char16_t const* get_string_buffer(hstring_header const* string)
{
    if (string)
    {
        return string->data(); 
    }

    return u"";
}

uint32_t get_string_size(hstring_header const* string)
{
    if (string)
    {
        return string->size();
    }

    return 0;
}

refcounted_hstring * duplicate_string(hstring_header * string)
{
    if (string && string->type() == hstring_type::ref_counted)
    {
        auto rc_string = static_cast<refcounted_hstring*>(string);
        rc_string->add_ref();
        return rc_string;
    }

    return make_string(string->data(), string->size());
}

bool string_has_embedded_null(hstring_header * string)
{
    if (!string)
    {
        return false;
    }

    return string->has_embedded_null();
}

refcounted_hstring * concat_strings(hstring_header * string1, hstring_header * string2)
{
    // if one of the strings is null, make sure it's string1    
    if (!string2)
    {
        std::swap(string1, string2);
    }

    if (!string1)
    {
        if (!string2)
        {
            // both strings are null
            return nullptr; 
        }

        // string1 is null but string2 is not, return a dupe of string2
        return duplicate_string(string2);
    }

    auto str1_buf = get_string_buffer(string1);
    uint32_t str1_size = get_string_size(string1);
    auto str2_buf = get_string_buffer(string2);
    uint32_t str2_size = get_string_size(string2);

    auto str = refcounted_hstring::alloc(hstring_type::ref_counted, str1_size + str2_size);
    refcounted_hstring::copy(str, {str1_buf, str1_size});
    refcounted_hstring::copy(str, {str2_buf, str2_size}, str1_size);
    return str;
}

void promote_prealloc_buffer(refcounted_hstring * str)
{
    if (str)
    {
        str->promote();
    }
}

extern "C"
{
int32_t WINRT_WindowsCreateString(const char16_t * sourceString, uint32_t length, void ** string) noexcept
{
    try
    {
        if (!string)
        {
            throw std::invalid_argument("string");
        }

        *string = nullptr;

        auto str = make_string(sourceString, length);
        *string = str; // reinterpret_cast<HSTRING>(str);
        return winrt::impl::error_ok;
    }
    catch (...) { return winrt::to_hresult(); }
}

int32_t WINRT_WindowsCreateStringReference(char16_t const* sourceString, uint32_t length,
    void * hstringHeader, void ** string) noexcept
{
    try
    {
        if (!string)
        {
            throw std::invalid_argument("string");
        }

        *string = nullptr;

        make_string_reference(
            sourceString, 
            length,
            reinterpret_cast<hstring_header*>(hstringHeader));
        *string = hstringHeader;
        return winrt::impl::error_ok;
    }
    catch (...) { return winrt::to_hresult(); }
}

const char16_t* WINRT_WindowsGetStringRawBuffer(void* string, uint32_t * length) noexcept
{
    if (length)
    {
        *length = get_string_size(reinterpret_cast<hstring_header*>(string));
    }

    return get_string_buffer(reinterpret_cast<hstring_header*>(string));    
}

uint32_t WINRT_WindowsGetStringLen(void* string) noexcept
{
    return get_string_size(reinterpret_cast<hstring_header*>(string));       
}

int32_t WINRT_WindowsDeleteString(void* string) noexcept
{
    try
    {
        delete_string(reinterpret_cast<hstring_header*>(string));
        return winrt::impl::error_ok;    
    }
    catch (...) { return winrt::to_hresult(); }
}

int32_t WINRT_WindowsDuplicateString(void* string, void ** newString) noexcept
{
    try
    {
        if (newString == nullptr)
        {
            throw std::invalid_argument("newString");
        }

        *newString = nullptr;

        auto str = duplicate_string(reinterpret_cast<hstring_header *>(string));
        *newString = str;
        return winrt::impl::error_ok;    
    }
    catch (...) { return winrt::to_hresult(); }
}

int32_t WINRT_WindowsStringHasEmbeddedNull(void* string, int32_t* hasEmbedNull) noexcept
{
    try
    {
        if (!hasEmbedNull)
        {
            throw std::invalid_argument("hasEmbedNull");
        }

        auto has_null = string_has_embedded_null(reinterpret_cast<hstring_header*>(string));
        *hasEmbedNull = has_null ? 1 : 0;
        return winrt::impl::error_ok;
    }
    catch (...) { return winrt::to_hresult(); }
}

int32_t WINRT_WindowsConcatString(void* string1, void* string2, void** newString) noexcept
{
    try
    {
        if (newString == nullptr)
        {
            throw std::invalid_argument("newString");
        }

        *newString = nullptr;

        auto str = concat_strings(
            reinterpret_cast<hstring_header*>(string1),
            reinterpret_cast<hstring_header*>(string2));
        *newString = str;
        return winrt::impl::error_ok;
    }
    catch (...) { return winrt::to_hresult(); }
}

int32_t WINRT_WindowsPreallocateStringBuffer(uint32_t length, char16_t ** charBuffer, void** bufferHandle) noexcept
{
    try
    {
        if (charBuffer == nullptr)
        {
            throw std::invalid_argument("charBuffer");
        }

        if (bufferHandle == nullptr)
        {
            throw std::invalid_argument("bufferHandle");
        }

        *charBuffer = nullptr;
        *bufferHandle = nullptr;

        auto str = make_prealloc_buffer(length);
        *charBuffer = str->_data();
        *bufferHandle = str;
        
        return winrt::impl::error_ok;    
    }
    catch (...) { return winrt::to_hresult(); }
}

int32_t WINRT_WindowsPromoteStringBuffer(void* bufferHandle, void** string) noexcept
{
    try
    {
        if (string == nullptr)
        {
            throw std::invalid_argument("string");
        }

        *string = nullptr;

        promote_prealloc_buffer(reinterpret_cast<refcounted_hstring *>(bufferHandle));
        *string = bufferHandle;
        return winrt::impl::error_ok;    
    }
    catch (...) { return winrt::to_hresult(); }
}

int32_t WINRT_WindowsDeleteStringBuffer(void* bufferHandle) noexcept
{
    try
    {
        delete_prealloc_buffer(reinterpret_cast<refcounted_hstring *>(bufferHandle));
        return winrt::impl::error_ok;    
    }
    catch (...) { return winrt::to_hresult(); }
}
} // extern "C"
