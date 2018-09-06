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

This design note documents the platform abstraction layer for Langworthy.

## Revisions
 * 2018-09-06, First Draft created by Ryan Shepherd


Overview
--------
Langworthy language projections depend on language-agnostic functionality that is provided by the underlying platform. Activating types, allocating shared cross-module memory and strings all require some built in functionality that's provided for app code and/or language projections.

Types and conventions
--------
Whenever possible, this document will refer to Langworthy fundamental types.
Otherwise, standard C types will be used.
* void* - A native pointer type to "unknown type".
This implies that the pointer size is sufficient to address arbitrary memory on the target architecture - 32 bits in 32-bit code, 64 bits in 64-bit code.
* size_t - An unsigned integer type, capable of storing the size of any object.
Generally, this implies it is the same size as a pointer.

Shared memory
--------
Langworthy code will, at times, require some memory to be dynamically allocated and passed between methods (e.g. *ReceiveArray* parameters).
Furthermore, the code allocating the memory may reside in a separate module and language projection than the code responsible for freeing it.
In order to satisfy platform constraints around dynamic memory allocation (matching process heaps, allocators, etc), Langworthy provides the following methods for allocating and freeing memory:

### XlangMemAlloc
Allocates a block of memory.
#### Syntax:
    void* XlangMemAlloc(size_t count);
#### Parameters:
* count - The size of the memory block to be allocated, in bytes.
#### Return value:
If the function succeeds, it returns the allocated memory block. Otherwise, it returns **NULL**.

### XlangMemFree
Frees a block of memory.
#### Syntax:
    void XlangMemFree(void* ptr);
#### Parameters:
* ptr - A pointer to the memory block to be freed. If this parameter is **NULL**, the function has no effect.
#### Return value:
This function does not return a value.