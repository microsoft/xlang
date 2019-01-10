---
id: XDN05
title: xlang Binary Interface
author: hpierson@microsoft.com
status: draft
---

# Title: XDN05 - xlang Binary Interface

- Author: Harry Pierson (hpierson@microsoft.com)
- Status: draft

## Abstract

This design note describes xlang's [application binary interface](https://en.wikipedia.org/wiki/Application_binary_interface)
(or ABI). The ABI defines a stable contract between xlang language projections and xlang components
that is necessary in order to interoperate.

## Overview

xlang components are intended to be authored and built independently from their clients - both
applications as well as other xlang components. Because an xlang component may be built at a
different time and potentially using a different compiler from the calling code, xlang needs a
stable contract - known as an application binary interface or ABI - to enable interoperability.

xlang uses COM interfaces as the stable contract for xlang object types. Flat C functions are also
used in xlang component binary loading as well as in xlang's
[Platform Abstraction Layer](XDN06%20-%20xlang%20Platform%20Abstraction%20Layer.md).

## COM Interfaces

> This section is intended to be a short introduction to COM interfaces.
For a more thorough definition of COM interfaces, please review the official Microsoft
[COM Technical Overview](https://docs.microsoft.com/en-us/windows/desktop/com/com-technical-overview).

[COM](https://docs.microsoft.com/en-us/windows/desktop/com/component-object-model--com--portal)
is a Microsoft technology that defines a binary interoperability standard for creating reusable
software libraries that interact at run time. xlang leverages the core concept of
[COM interfaces](https://docs.microsoft.com/en-us/windows/desktop/com/com-objects-and-interfaces)
as well as the definition of the fundamental COM interface
[IUnknown](https://docs.microsoft.com/en-us/windows/desktop/com/using-and-implementing-iunknown)
to provide a stable contract that both components and language projections can depend on.

COM interfaces are basically a pointer to an array of function pointers, similar to how most C++
compilers implement [virtual method tables](https://en.wikipedia.org/wiki/Virtual_method_table).
Each member of a COM interface is the array as a function pointer. Each function in the array takes
the array pointer as it's first parameter. While this design is intrinsic to most C++ compilers,
these arrays can be built up explicitly in other languages where needed.

COM interfaces are identified by a globally unique identifier or
[GUID](https://en.wikipedia.org/wiki/Universally_unique_identifier).
This identifier enables calling code to query a specific xlang component about its support for
specific interfaces.

COM Interfaces are allowed to inherit from other interfaces. This is often used to add capabilities
to later versions of a given component. By inheriting from another interface, the COM interface is
defining a single long array of function pointers, where the first set of function pointers are
defined by the base interface while the next set of function pointers are defined by the derived
interface. Multiple levels of COM interface single inheritance is supported.

> Note, while xlang uses COM interfaces as a stable contract between components, xlang is
explicitly avoiding the rest of the COM standard, including concepts such as apartments, monikers
and marshalling. These constructs are tightly bound to the Windows platform and so have little use
in the xlang project. Likewise, xlang is explicitly avoiding Windows technologies that build on COM
such as DCOM, MTS, COM+, ActiveX and the Windows Runtime.

## IUnknown

> This section is intended to be a short introduction to the IUnknown COM interface.
For a more thorough explanation of IUnknown, please review the official Microsoft documentation 
[Using and Implementing IUnknown](https://docs.microsoft.com/en-us/windows/desktop/com/using-and-implementing-iunknown).

IUnknown is the root interface in any COM-based system. It provides functions for two core
capabilities - navigating an object's capabilities and managing the object's lifetime. All COM
interfaces derive from IUnknown and all objects in any COM based system must implement these
methods.

> Note, code snippets 

### Capability Navigation

Given any COM interface pointer, you can query for any other COM interface via IUnknown's
QueryInterface method. As stated above, interfaces are identified by a GUID which is passed by
reference as the first parameter of QueryInterface. The pointer to the requested interface - if
available - is returned via a void** out parameter. It is the responsibility of the calling code to
cast this pointer to the correct COM interface pointer type.

Methods on COM interfaces (including ones used in xlang) never throw C++ exceptions, due to the
need for a stable contract across compilers. QueryInterface returns a 32-bit signed integer as an
error code known. On Windows, this code is known as an
[HRESULT](https://docs.microsoft.com/en-us/windows/desktop/com/error-handling-in-com). 

``` cpp
int32_t QueryInterface(guid const& interface_id, void** object)
```

### Lifetime Management

COM objects manage their own lifetime via an internal reference count. When this count goes to
zero, the COM object cleans itself up. IUnknown provides methods to increment and decrement the
object's reference count. If a language projection makes a copy of a COM interface pointer, it must
call AddRef. When client code is finished using a given pointer to a COM interface, the language
projection must call Release.

The return value of these function is the new reference count of the system. As per the COM
standard, this value is intended to be used only for test purposes.

``` cpp
uint32_t AddRef();
uint32_t Release();
```

### IXlangUnknown

> Note, WinRT calls this interface IInspectable.

In addition to the core functionality described by IUnknown, all ABI interfaces in xlang derive from
IXlangUnknown. This interface adds additional functionality.

#### String Representation

Many languages have a standard way to retrieve a string representation of a given object. For
example, C# has
[Object.ToString](https://docs.microsoft.com/en-us/dotnet/api/system.object.tostring?view=netframework-4.7.2)
and Python has the [str function](https://docs.python.org/3/library/functions.html#func-str).
IXlangUnknown::ToString returns a string representation of a given object. By default, this method
should return the type's name, however it can be implemented in whatever manner makes most sense
for the underlying object.

``` cpp
int32_t ToString(xlang_string* object_string)
```

#### Type Information

> Note, WinRT calls this method GetRuntimeClassName.

The intention of GetTypeName is to provide a mechanism to discover a given object's type at
runtime. This is of significant importance for use in dynamic languages such as JavaScript and
Python. The expectation is that the caller has access to the xlang metadata for this component and
can use the type name to find the associated type.

> Note, we need a design note to cover GetTypeName.

``` cpp
int32_t GetTypeName(xlang_string* object_string);
```

#### Weak References

One issue faced in COM based systems is circular references - i.e. two objects who each hold a
pointer to the other. In this case, neither reference count will ever go to zero and neither
object will ever be cleaned up. To avoid this issue, xlang supports weak references.

A weak reference to an xlang object does not keep it alive. In other words, when all non-weak
reference count of an object goes to zero, it cleans itself up even if there are outstanding weak
references to it. As such, weak reference holders must check to see if a given reference is still
valid via the IWeakReference::Resolve method.

Note, because IWeakReference represents a proxy to an xlang object that may no longer exist, none
of the IXlangUnknown methods are relevant. As such, this is one of the few places in xlang where
an ABI interface inherits directly from IUnknown.

``` cpp
int32_t GetWeakReference(IWeakReference** weak_reference);

struct IWeakReference : IUnknown
{
    virtual int32_t Resolve(guid const& iid, void** objectReference);
};
```

#### Reference Count Retrieval

``` cpp
int32_t GetReferenceCount(uint32_t** reference_count) noexcept;
```
