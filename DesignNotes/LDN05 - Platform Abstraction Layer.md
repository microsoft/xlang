---
id: LDN05
title: Platform Abstraction Layer
author: ryansh@microsoft.com
status: draft
---

# Title: ​LDN05 - Platform Abstraction Layer
* Author: Ryan Shepherd (ryansh@microsoft.com)
* Status: Draft

## Abstract

This design note documents the platform abstraction layer (or PAL) for Xlang.

Overview
--------
Xlang language projections depend on language-agnostic functionality that is provided by the underlying platform. Activating types, allocating shared cross-module memory and strings all require some built in functionality that's provided for app code and/or language projections.

The C language binding will be the lingua franca for the ABI underpinning all other language projections and interop.
In this document, therefore, function syntax will be declared in the C language.

Types and conventions
--------
For low-level type definitions, see the [ABI design notes](LDN06%20-%20Application%20Binary%20Interface.md).

The ABI design notes also have XlangResult values used by the PAL, and ABI definitions of XlangString. Some types specific to the PAL are described below.
#### XlangStringHeader
This is a structure used to track the lifetime of *fast-pass* XlangStrings. See [XlangCreateStringReference](#xlangcreatestringreference) for more details.

This structure has the same alignment as the native pointer type, and is 24 or 20 bytes in size, on 64-bit or 32-bit platforms, respectively.
In C language terms, it behaves as if defined as:
```C
typedef struct XlangStringHeader
{
  union
  {
    void* Reserved1;
#if 64_BIT_PLATFORM
    char Reserved2[24];
#else
    char Reserved2[20];
#endif
  } Reserved;
};
```

#### XStringBuffer
This type holds a preallocated string buffer for subsequent promotion into a **XlangString**.
This type is the size of a pointer, but distinct from other pointer types, as if defined as:
```C
typedef struct XlangStringBuffer__
{
    int unused;
} XlangStringBuffer__;
typedef XlangStringBuffer__* XlangStringBuffer;
```
--------
Shared memory
--------
Xlang code will, at times, require some memory to be dynamically allocated and passed between in-process components. *ReceiveArray* method parameters are a common example of this pattern.

Because these components may use different different allocators, runtime heaps, or even different language projections, Xlang provides a dedicated facility to guarantee these allocations and deallocations are performed in a consistent manner.

This also precludes the possibility of statically-linking the PAL, as each component carrying a statically-linked PAL would defeat the purpose of having a dedicated, single module managing this memory.

Note that at this time, no guarantees are being made regarding cross-process shared memory. This functionality exists solely for intra-process shared memory.

### XlangMemAlloc
Allocates a block of memory.
#### Syntax
```C
void* __stdcall XlangMemAlloc(size_t count);
```
#### Parameters
* count - The size of the memory block to be allocated, in bytes. If count is 0, XlangMemAlloc attempts to allocates a zero-length item and return a valid pointer to that item.
#### Return value
If the function succeeds, it returns the allocated memory block. Otherwise, it returns **NULL**.
#### Remarks
To free this memory, call [XLangMemFree](#xlangmemfree).

This function is thread-safe: it behaves as though only accessing the memory locations visible through its argument, and not any static storage. In other words, the same thread safety guarantees as malloc in C11 and C++11.

### XlangMemFree
Frees a block of memory that was allocated by [XlangMemAlloc](#xlangmemalloc)
#### Syntax
```C
void __stdcall XlangMemFree(void* ptr);
```
#### Parameters
* ptr - A pointer to the memory block to be freed. If this parameter is **NULL**, the function has no effect.
#### Return value
This function does not return a value.

#### Remarks
This function is thread-safe: it behaves as though only accessing the memory locations visible through its argument, and not any static storage. In other words, the same thread safety guarantees as free in C11 and C++11.

--------
String functions
--------
Passing and sharing strings across components presents some of same challenges as dynamically allocated memory, with respect to matching allocation and deallocation.
Therefore, Xlang has support for allocating strings for cross-component interop.

Xlang strings can be encoded in either UTF-8 or UTF-16, using C's char or char16_t, respectively.
Functions that accept and/or return character data will \*U8 or \*U16 suffixes to disambiguate the two "flavors" of function call.

Xlang strings are immutable and hidden behind an opaque handle: **XlangString**. The underlying string data can only be accessed through Xlang string APIs via this handle.

Call [XlangCreateString](#xlangcreatestring) to create a new **XlangString**, and call [XlangDeleteString](#xlangdeletestring) to release the reference to the backing memory.

Copy an **XlangString** by calling [XlangDuplicateString](#xlangduplicatestring).

The underlying character data can be accessed by calling [XlangGetStringRawBuffer](#xlanggetstringrawbuffer).

Call [XlangPreallocateStringBuffer](#xlangpreallocatestringbuffer) to allocate a mutable string buffer that you can then promote into an immutable **XlangString**.
When you have finished populating the buffer, you can call [XlangPromoteStringBuffer](#xlangpromotestringbuffer) to convert that buffer into an immutable **XlangString**, or call [XlangDeleteStringBuffer](#xlangdeletestringbuffer) to discard it prior to promotion.
This two-phase construction has similar functionality to a "string builer" found in other libraries.

Xlang also supports creating "fast pass" strings by calling [XlangCreateStringReference](#xlangcreatestringreference).
In this case, the memory containing the backing string data is owned by the caller, and not allocated on the heap.
Therefore, Xlang relies upon the caller to maintain the backing string data, unchanged, for the duration of the string's lifetime.

Semantically, a **XlangString** containing the value **NULL** represents the empty string, which consists of zero content characters and a terminating null character.
Calling [XlangCreateString](#xlangcreatestring) with zero characters will produce the value **NULL**.
Calling [XlangGetStringRawBuffer](#xlanggetstringrawbuffer) with **NULL** will return an empty string followed only by the null terminating character.

### XlangCreateString
Creates a new XlangString.
#### Syntax
```C
XlangResult XlangCreateStringU8(
    char const* sourceString,
    uint32_t length,
    XlangString* string
);

XlangResult XlangCreateStringU16(
    char16_t const* sourceString,
    uint32_t length,
    XlangString* string
);
```
#### Parameters
* sourceString - A string to use as the source for a new **XlangString**. 
To create a new empty, or **NULL** string, pass **NULL** for *sourceString* and 0 for *length*.
* length - The length of the string, in code units. In other words, the number of elements pointed to by *sourceString*, not counting the null-terminator.
Must be 0 if *sourceString* is **NULL**.
* string - A pointer to the newly created **XlangString**, or **NULL** if an error occurs.
#### Return value
| Return code              | Description                       |
|--------------------------|-----------------------------------|
| XLANG_OK                 | The XlangString was created succesfully. |
| XLANG_INVALID_ARG        | *string* was **NULL**. |
| XLANG_OUTOFMEMORY        | Failed to allocate memory for the new XlangString. |
| XLANG_POINTER            | *sourceString* was **NULL** and *length* was non-zero. |
| XLANG_MEM_INVALID_SIZE   | The requested allocation size was too large. |
#### Remarks
Xlang copies *length* elements from *sourceString* to the backing buffer of the new **XlangString**, plus a null-terminator.

Call [XlangDeleteString](#xlangdeletestring) to deallocate the string.
Each call to **XlangCreateString** must be matched by a call to **XlangDeleteString**.

To create a *fast pass* string without a heap allocation or copy, call [XlangCreateStringReference](#xlangcreatestringreference).

To create a new empty or **NULL** string, pass **NULL** for *sourceString* and 0 for *length*.

The backing buffer of this string will be managed by a thread-safe reference count.

### XlangCreateStringReference
Create a *fast-pass* string based on the supplied string data.
#### Syntax
```C
XlangResult XlangCreateStringReferenceU8(
    char const* sourceString,
    uint32_t length,
    XlangStringHeader* header,
    XlangString* string
);
XlangResult XlangCreateStringReferenceU16(
    char16_t const* sourceString,
    uint32_t length,
    XlangStringHeader* header,
    XlangString* string
);
```
#### Parameters
* sourceString - A null-terminated string to use as the source. **NULL** represents the empty string if *length* is 0.
* length - The length of the string in code units. Must be 0 if *sourceString* is **NULL**. Otherwise, *sourceString* must have a terminating null character.
* header - A pointer to a [XlangStringHeader](#xlangstringheader) structure that Xlang uses to identify *string* as a *fast-pass* string.
* string - A pointer to the newly created string, or **NULL** if an error occurrs. This string will be a *fast-pass* string.
#### Return value
| Return code              | Description                       |
|--------------------------|-----------------------------------|
| XLANG_OK                 | The *fast-pass* **XlangString** was created succesfully. |
| XLANG_INVALID_ARG        | Either *string* or *header* was **NULL**. |
| XLANG_STRING_NOT_NULL_TERMINATED        | *string* was not null-terminated. |
| XLANG_POINTER            | *sourceString* was **NULL** and *length* was non-zero. |
#### Remarks
Use this function to create a *fast-pass* string.
Unlike a string created by [XlangCreateString](#xlangcreatestring), the lifetime of the backing buffer is not managed by Xlang.
The caller allocates *sourceString* on the stack frame, together with an uninitialized [XlangStringHeader](#xlangstringheader), to avoid a heap allocation.
The caller must ensure that *sourceString* and the contents of *header* remain unchanged during the lifetime of the attached **XlangString**.

Strings created with this function do not need to be deallocated with [XlangDeleteString](#xlangdeletestring).

### XlangDeleteString
Deletes a XlangString.
#### Syntax
```C
void XlangDeleteString(XlangString string);
```
#### Parameters
* string - The **XlangString** to delete.
* If *string* is a fast-pass string created by [XlangCreateStringReference](#xlangcreatestringreference) or **NULL**, no action is taken.
#### Return value
This function does not return a value.
#### Remarks
This function decrements the reference count of the backing buffer.
If the reference count reaches 0, the buffer will be deallocated.

### XlangDeleteStringBuffer
Discards a preallocated string buffer if it was not promoted to a **XlangString**.
#### Syntax
```C
XlangResult XlangDeleteStringBuffer(
    XlangStringBuffer bufferHandle
);
```
#### Parameters
* bufferHandle - The buffer to discard. 
#### Return value
| Return code              | Description                       |
|--------------------------|-----------------------------------|
| XLANG_OK                 | The buffer was discarded succesfully. |
| XLANG_POINTER            | *bufferHandle* is **NULL**.
| XLANG_INVALID_ARG        | *bufferHandle* was not created by [XlangPreallocateStringBuffer](#xlangpreallocatestringbuffer).
#### Remarks
Call this function to discard a string buffer that was created by [XlangPreallocateStringBuffer](#xlangpreallocatestringbuffer), but has not been promoted to an **XlangString** by the [XlangPromoteStringBuffer](#xlangpromotestringbuffer) function.

Calling **XlangPromoteStringBuffer** after calling **XlangDeleteStringBuffer** is undefined.

### XlangDuplicateString
Creates a copy of the specified string.
#### Syntax
```C
XlangResult XlangDuplicateString(
    XlangString string,
    XlangString* newString
);
```
#### Parameters
* string - The source string to be copied.
* newString - The copy of *string*.
#### Return value
| Return code              | Description                       |
|--------------------------|-----------------------------------|
| XLANG_OK                 | The **XlangString** was copied succesfully. |
| XLANG_INVALID_ARG        | *newString* was **NULL**. |
| XLANG_OUTOFMEMORY        | Failed to allocate memory for the new **XlangString**. |
#### Remarks
Use this function to copy a **XlangString**.

If *string* was created by calling [XlangCreateString](#xlangcreatestring), the reference count of the backing buffer is incremented, and *newString* uses the same backing buffer.

If *string* was created by calling [XlangCreateStringReference](#xlangcreatestringreference), (implying it is a *fast-pass* string), Xlang copies the source string to a new buffer with a new reference count.
The resulting copy has its own backing buffer and is not a *fast-pass* string.

Each call to **XlangDuplicateString** must be matched with a corresponding call to [XlangDeleteString](#xlangdeletestring).

### XlangGetStringRawBuffer
Get the backing buffer for the specified string.
#### Syntax
```C
XlangResult XlangGetStringRawBufferU8(
    XlangString string,
    char const* * buffer,
    uint32_t* length
);
XlangResult XlangGetStringRawBufferU16(
    XlangString string,
    char16_t const* * buffer,
    uint32_t* length
);
```
#### Parameters
* string - The string for which the backing buffer is to be received.
* buffer - Receives a pointer to the backing store. Receives **NULL** if *string* is **NULL** or the empty string.
* length - Receives the number of code units in the string, excluding the null terminator, or 0 if *string* is **NULL** or the empty string.
#### Return value
| Return code              | Description                       |
|--------------------------|-----------------------------------|
| XLANG_OK                 | Success.             |
| XLANG_OUTOFMEMORY        | Failed to allocate memory for the new buffer. |
#### Remarks
Do not change the contents of the buffer.

### XlangPreallocateStringBuffer
Allocates a mutable character buffer for use in string creation.
#### Syntax
```C
XlangResult XlangPreallocateStringBufferU8(
    uint32_t length,
    char** charBuffer,
    XlangStringBuffer* bufferHandle
);
XlangResult XlangPreallocateStringBufferU16(
    uint32_t length,
    char16_t** charBuffer,
    XlangStringBuffer* bufferHandle
);
```
#### Parameters
* length - The size of the buffer to allocate, in elements. A value of zero corresponds to the empty string.
* charBuffer - Receives the mutable buffer that holds the characters. The buffer already contains a terminating null character.
* bufferHandle - Receives the preallocated string buffer.
#### Return value
| Return code              | Description                       |
|--------------------------|-----------------------------------|
| XLANG_OK                 | Success.             |
| XLANG_OUTOFMEMORY        | Failed to allocate memory for the new buffer. |
| XLANG_POINTER            | *charBuffer* or *bufferHandle* was **NULL**. |
| XLANG_MEM_INVALID_SIZE   | The requested allocation size was too large. |
#### Remarks
Use this function to create a mutable character buffer that you can manipulate prior to promoting it into an immutable **XlangString**.
When you have finished populating the character buffer, call [XlangPromoteStringBuffer](#xlangpromotestringbuffer) to create the **XlangString**.

Call [XlangDeleteStringBuffer](#xlangdeletestringbuffer) to discard the buffer prior to promotion.
If the buffer has already been promoted by a call to **XlangPromoteStringBuffer**, call [XlangDeleteString](#xlangdeletestring) to discard the string.
If **XlangPromoteStringBuffer** fails, you can call **XlangDeleteStringBuffer** to discard the buffer.

### XlangPromoteStringBuffer
Creates a **XlangString** from the specified **XlangStringBuffer**.
#### Syntax
```C
XlangResult XlangPromoteStringBuffer(
    XlangStringBuffer bufferHandle,
    XlangString* string,
    uint32_t length
);
```
#### Parameters
* bufferHandle - The buffer to use for the new string.
You must call [XlangPreallocateStringBuffer](#xlangpreallocatestringbuffer) to create this.
* string - The newly created XlangString that contains the contents of *bufferHandle*.
* length - The length of the string in *bufferHandle*, not counting the null terminator.
This value must be less than or equal to the length passed to [XlangPreallocateStringBuffer](#xlangpreallocatestringbuffer). 
#### Return value
#### Return value
| Return code              | Description                       |
|--------------------------|-----------------------------------|
| XLANG_OK                 | Success.             |
| XLANG_POINTER            | *string* was **NULL**. |
| XLANG_INVALID_ARG        | *bufferHandle* was not created by calling [XlangPreallocateStringBuffer](#xlangpreallocatestringbuffer), or the caller has overwritten the terminating null character in *bufferHandle*, or *length* is greater than the *length* the buffer was created with.
#### Remarks
Calling this function converts the mutable buffer to an immutable **XlangString**.

If this function fails, you can use [XlangDeleteStringBuffer](#xlangstringbuffer) to discard the mutable buffer.

Each call to **XlangPromoteStringBuffer** must be matched with a corresponding call to [XlangDeleteString](#xlangdeletestring).
