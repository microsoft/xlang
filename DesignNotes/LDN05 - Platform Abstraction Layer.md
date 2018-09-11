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

This design note documents the platform abstraction layer (or PAL) for Langworthy.

## Revisions
 * 2018-09-06, First Draft created by Ryan Shepherd


Overview
--------
Langworthy language projections depend on language-agnostic functionality that is provided by the underlying platform. Activating types, allocating shared cross-module memory and strings all require some built in functionality that's provided for app code and/or language projections.

The C language binding will be the lingua franca for the ABI underpinning all other language projections and interop.
In this document, therefore, function syntax will be declared in the C language.

Types and conventions
--------
The following fundamental Langworthy types map to C types as follows:

| Langworthy Type | C type |
|-----------------|------- |
| Int16           | int16_t |
| Int32           | int32_t |
| Int64           | int64_t |
| UInt8           | uint8_t |
| UInt16          | uint16_t |
| UInt32          | uint32_t |
| UInt64          | uint64_t |
| Single          | float |
| Double          | double |
| Char16          | char16_t |
| Boolean          | uint8_t (C99 does not guarantee _Bool to be 8 bits) |

* void* - A native pointer type to "unknown type".
This implies that the pointer size is sufficient to address arbitrary memory on the target architecture - 32 bits in 32-bit code, 64 bits in 64-bit code.
* size_t - An unsigned integer type, capable of storing the size of any object.
Generally, this implies it is the same size as a pointer, and will be uint32_t or uint64_t on 32-bit or 64-bit platforms, respectively.

### Calling conventions
For non-member (i.e. free) functions, the 32-bit x86 calling convention will be specified in the function syntax declaration.
This will be either *__stdcall* or *__cdecl*. These calling conventions shall have no effect on other architectures and can be safely ignored in those cases.

Shared memory
--------
Langworthy code will, at times, require some memory to be dynamically allocated and passed between in-process components. *ReceiveArray* method parameters are a common example of this pattern.

Because these components may use different different allocators, runtime heaps, or even different language projections, Langworthy provides a dedicated facility to guarantee these allocations and deallocations are performed in a consistent manner.

This also precludes the possibility of statically-linking the PAL, as each component carrying a statically-linked PAL would defeat the purpose of having a dedicated, single module managing this memory.

Note that at this time, no guarantees are being made regarding cross-process shared memory. This functionality exists solely for intra-process shared memory.

### XlangMemAlloc
Allocates a block of memory.
#### Syntax:
    void* __stdcall XlangMemAlloc(size_t count);
#### Parameters:
* count - The size of the memory block to be allocated, in bytes.
#### Return value:
If the function succeeds, it returns the allocated memory block. Otherwise, it returns **NULL**.

### XlangMemFree
Frees a block of memory.
#### Syntax:
    void __stdcall XlangMemFree(void* ptr);
#### Parameters:
* ptr - A pointer to the memory block to be freed. If this parameter is **NULL**, the function has no effect.
#### Return value:
This function does not return a value.