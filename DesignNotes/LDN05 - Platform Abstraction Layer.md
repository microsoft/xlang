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

Overview
--------
Langworthy language projections depend on language-agnostic functionality that is provided by the underlying platform. Activating types, allocating shared cross-module memory and strings all require some built in functionality that's provided for app code and/or language projections.

The C language binding will be the lingua franca for the ABI underpinning all other language projections and interop.
In this document, therefore, function syntax will be declared in the C language.

Types and conventions
--------
See the [ABI design notes](LDN06%20-%20Application%20Binary%20Interface.md)

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