---
id: LDN06
title: Application Binary Interface
author: ryansh@microsoft.com
status: draft
---

# Title: ​LDN06 - Application Binary Interface
* Author: Ryan Shepherd (ryansh@microsoft.com)
* Status: Draft

## Abstract

This design note documents the Application Binary Interface (or ABI) for Langworthy.

Overview
--------
Langworthy language projections depend on a stable binary interface for both interacting with the Platform Abstraction Layer (PAL) and other components.

The C programming language is the lingua franca for describing this interface. This document describes how the Langworthy type system maps to C types.

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
