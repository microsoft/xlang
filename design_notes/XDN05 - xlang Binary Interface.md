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

This design note describes xlang's
[application binary interface](https://en.wikipedia.org/wiki/Application_binary_interface)
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
software libraries that interact at run time. xlang leverages
[COM interfaces](https://docs.microsoft.com/en-us/windows/desktop/com/com-objects-and-interfaces)
as well as the definition of fundamental COM interface
[IUnknown](https://docs.microsoft.com/en-us/windows/desktop/com/using-and-implementing-iunknown)
as a foundation for stable contracts that both components and language projections can depend on.

A COM interface pointer is basically a pointer to an array of function pointers, similar to how
most C++ compilers implement
[virtual method tables](https://en.wikipedia.org/wiki/Virtual_method_table).
Each member of a COM interface is stored in an array slot as a function pointer. Each function in
the array takes the array pointer as it's first parameter. While this design is intrinsic to most
C++ compilers, these function pointer arrays can also be built up explicitly in other languages
where needed.

COM interfaces are identified by a 128-bit integer known as a globally unique identifier or
[GUID](https://en.wikipedia.org/wiki/Universally_unique_identifier).
This identifier enables calling code to query a given xlang component about its support for
specific interfaces.

Once a COM interface is published, it cannot be changed. As stated before, a COM interface is a
stable contract between caller and callee. Interoperability would be impossible if COM interfaces
were allowed to change over time.

COM Interfaces are allowed to inherit from other interfaces. This is often used to add capabilities
to later versions of a given component. By inheriting from another interface, the COM interface is
defining a single longer array of function pointers, where the first set of function pointers are
defined by the base interface while the next set of function pointers are defined by the derived
interface. Multiple levels of COM interface single inheritance is supported.

> Note, while xlang uses COM interfaces as a stable contract between components, xlang is
explicitly avoiding the rest of the COM standard, including concepts such as apartments, monikers
and marshalling. These constructs are tightly bound to the Windows platform and so have little use
in the xlang project. Likewise, xlang is explicitly avoiding Windows technologies that build on COM
such as DCOM, MTS, COM+, ActiveX and the Windows Runtime. Windows Runtime is an inspiration for
xlang, but remains a separate implementation.

## IUnknown

> This section is intended to be a short introduction to the IUnknown COM interface.
For a more thorough explanation of IUnknown, please review the official Microsoft documentation
[Using and Implementing IUnknown](https://docs.microsoft.com/en-us/windows/desktop/com/using-and-implementing-iunknown).

IUnknown is the root COM interface in any COM-based system. It provides functions for two core
capabilities - navigating an object's capabilities and managing the object's lifetime. All COM
interfaces derive from IUnknown and all objects in any COM based system must implement these
methods.

``` cpp
struct IUnknown
{
    int32_t  QueryInterface(guid const& id, void** object);
    uint32_t AddRef();
    uint32_t Release();
};
```

### QueryInterface

Given any COM interface pointer, you can query for any other COM interface via IUnknown's
QueryInterface method. If the referenced COM object supports the interface being queried, a pointer
to that COM interface is returned to the caller. Otherwise, an error code indicating that the
requested interface is not supported.

As stated above, interfaces are identified by a GUID. This GUID is passed by reference as the first
parameter of QueryInterface. The pointer to the requested interface - if available - is returned
via a void** out parameter. It is the responsibility of the calling code to cast this pointer to
the correct COM interface pointer type.

Methods on COM interfaces (including xlang ABI interfaces) never throw C++ exceptions, due to the
need for a stable contract across compilers. QueryInterface returns a 32-bit signed integer as an
error code known. On Windows, this code is known as an
[HRESULT](https://docs.microsoft.com/en-us/windows/desktop/com/error-handling-in-com).
Generally with HRESULTs, negative integers indicate errors, positive integers and zero indicate
success. QueryInterface in particular should always return 0 on success.

QueryInterface may return the following error codes:

- 0x80004002 (E_NOINTERFACE) indicates the requested interface is not supported by this object.

### AddRef and Release

COM objects manage their own lifetime via an internal reference count. When this count goes to
zero, the COM object cleans itself up. IUnknown provides methods to increment and decrement the
object's reference count. If a language projection makes a copy of a COM interface pointer, it must
call AddRef. When client code is finished using a given pointer to a COM interface, the language
projection must call Release.

The return value of these function is specified to be the new reference count of the object.
However the COM standard itself warns that this value is intended to be used only for test purposes.
In practice on Windows, the value returned from these two functions is often inaccurate. The one
scenario where you can trust the return value is when Release returns zero - in this case, you can
reliably assume the object has cleaned itself up.

## IXlangObject

In addition to the core functionality described by IUnknown, ABI interfaces for xlang types derive
from IXlangObject. This interface adds additional core functionality needed for xlang.

``` cpp
enum ObjectInfoCategory
{
    StringRepresentation,
    TypeName,
    MemoryUsage
};

struct IXlangObject
{
    int32_t GetInfo(ObjectInfoCategory info_category, void** info);
};
```

### GetInfo

In WinRT, the IInspectable interface provides property get methods that retrieved information about
the object needed in specific runtime scenarios. Over time, the list of information categories that
might be useful at runtime grew. For example, knowing the memory footprint of a given WinRT object
would allow for garbage collected languages like C# make better scheduling decisions. But since
IInspectable is the base COM interface for every interface in WinRT, it's impossible to extend.

For xlang, instead of individual property get methods, IXlangObject exposes a single GetObjectInfo
method. The method takes an enum value indicating the object info being requested and a void** to
hold the returned value. xlang objects are not required to support all object information
categories. Similar to QueryInterface, an object can simply return an error code indicating that a
requested information category is not supported by the object.

Having a single GetInfo method has two primary benefits over the IInspectable approach. Because
enums can be additively versioned over time, it is possible to add new ones over time. Since
supporting a given information category is optional, adding new ones would not break existing
objects. Furthermore, it reduces the size of virtual function table that every xlang object needs
to support.

Over time, we expect the list of object information categories to grow. Unlike COM interfaces, we
expect the list of object information categories to be managed centrally by the xlang project.

#### String Representation

Most mainstream languages including
[JavaScript](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Object/toString),
[Python](https://docs.python.org/3/library/functions.html#func-str),
[.NET](https://docs.microsoft.com/en-us/dotnet/api/system.object.tostring?view=netframework-4.7.2),
[Java](https://docs.oracle.com/javase/7/docs/api/java/lang/Object.html#toString()) and
[Objective-C](https://developer.apple.com/documentation/objectivec/1418956-nsobject/1418746-description)
have a mechanism to retrieve the string representation of an object. 

The StringRepresentation information category retrieves the string representation of the object in
question. The property type for StringRepresentation is an xlang_string.

While all information categories are optional, it is highly recommended that all objects support
StringRepresentation. By default, this method can return the type's name, however it can be
implemented in whatever manner makes most sense for the underlying object.

#### Type Information

Dynamic languages such as JavaScript and Python often need to be able to discover information
about an object at runtime. In order to do this, xlang object must have a mechanism to find the
metadata for arbitrary xlang objects.

The TypeName information category retrieves the fully namespace qualified name of the type in
question. The property type for TypeName is an xlang_string.

While all information categories are optional, it is highly recommended that all objects support
TypeName in order to be usable from dynamic languages.

#### Memory Usage

In languages C# and Java, garbage collection happens on background threads on a non-deterministic
schedule. These garbage collectors typically depend on an omniscient understanding of memory usage.
The memory used by xlang components is not typically visible to these garbage collectors, leading
to uninformed decision making regarding collection scheduling. If these systems could retrieve
the memory usage of arbitrary xlang objects, their scheduling heuristics would be better and the
system would run more smoothly overall.

The MemoryUsage information category retrieves the memory usage in bytes of the object in question.
The property type for MemoryUsage is an unsigned 32-bit integer.

## Weak References

One issue faced in COM based systems is circular references - i.e. two objects who each hold a
pointer to the other. In this case, neither reference count will ever go to zero and neither
object will ever be cleaned up. To avoid this issue, xlang supports weak references.

A weak reference to an xlang object does not keep it alive. In other words, when all non-weak
reference count of an object goes to zero, it cleans itself up even if there are outstanding weak
references to it. As such, weak reference holders must check to see if a given reference is still
valid via the IWeakReference::Resolve method.

Weak references are retrieved for an arbitrary xlang object by querying for the
IWeakReferenceSource interface and calling GetWeakReference. If an object does not support weak
references, it simply returns the no interface error code from query interface.

Note, because IWeakReferenceSource and IWeakReference are ABI interfaces that represents a proxy to
an xlang object that may no longer exist, none of the IXlangObject methods are relevant. As such,
this is one of the few places where an xlang ABI interface inherits directly from IUnknown.

``` cpp
struct IWeakReference : IUnknown
{
    int32_t Resolve(guid const& iid, void** object_reference);
};

struct IWeakReferenceSource : IUnknown
{
    int32_t GetWeakReference(IWeakReference** weak_reference);
};
```
