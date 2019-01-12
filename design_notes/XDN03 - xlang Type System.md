---
id: XDN03
title: xlang Type System
author: hpierson@microsoft.com
status: draft
---

# Title: XDN03 - xlang Type System

- Author: Harry Pierson (hpierson@microsoft.com)
- Status: draft

## Abstract

This design note describes the xlang's [type system](https://en.wikipedia.org/wiki/Type_system).
The type system is designed to interoperate with a variety of object-oriented programming
languages in a natural and familiar way.

## Namespaces

A namespace is a naming scope used to organize code and avoid naming collisions. All type
categories in the xlang type system (enums, structs, delegates, interfaces, and runtime classes)
except for fundamental types live in a namespace.

Namespaces may contain other namespaces.

xlang namespaces and type names are case preserving but insensitive. This means that you cannot
have namespaces or type names that vary only by case. For example: you cannot have `Foo.SomeType`
and `foo.SomeType` nor can you have `Foo.SomeType` and `Foo.someType`. However, you can have
`Foo.SomeType` and `foo.AnotherType`. In this case `SomeType` and `AnotherType` would be considered
as belonging to the same namespace.

## Fundamental Types

The xlang Type system includes a core set of built-in primitive types.

xlang Type | Type Description
---------- | ----------------------------------------------------------
Int8       | 8 bit signed integer
Int16      | 16 bit signed integer
Int32      | 32 bit signed integer
Int64      | 64 bit signed integer
UInt8      | 8 bit unsigned integer
UInt16     | 16 bit unsigned integer
UInt32     | 32 bit unsigned integer
UInt64     | 64 bit unsigned integer
Single     | 32-bit IEEE 754 floating point number
Double     | 64-bit IEEE 754 floating point number
Char16     | 16-bit non-numeric value representing a UTF-16 code unit
Boolean    | 8-bit Boolean value
String     | immutable sequence of Char16s used to represent text
Guid       | 128-bit standard [universally unique identifier](https://en.wikipedia.org/wiki/Universally_unique_identifier)
Object     | xlang object of unknown type

> Note: we need an XDN for string encoding. WinRT originally specified UTF-16 character type and
strings. However, UTF-8 has become the dominant encoding on the web since WinRT was originally
designed. This work is tracked by https://github.com/Microsoft/xlang/issues/53

## Enums

An enum type is a set of named values. Each named value in an enum corresponds to a constant integer
value.

Each enum type has an underlying integral type. The only legal underlying integral types for enums
in xlang are Int32 and UInt32.

> TODO: consider expanding the types usable for enums. 64bit bit flags, unsigned enums that aren't
flags (addresses) were both mentioned.

An enum type with an underlying integral type of UInt32 is considered a Flags enum. Flags enums
are intended to be treated as a bit field. Language projections can and should provide logical
operators such as and, or and not for Flags enum values.

Enums are additively versionable. Subsequent versions of a given enum may add new named values (and
associated constant integer values). Pre-existing values may not be removed or changed.

> Note, we need an XDN that focuses on how versioning works in xlang.

## Structs

Structs are a record type with one or more fields. Structs are always passed and returned by value.
Struct fields may only be enums, structs and fundamental types (including strings).

Structs must have at least one field. All of a struct's fields are public.

Structs cannot be generic or parameterized.

Structs are not versionable. Once they have been defined, they may never be changed.

## Object Members

xlang objects (i.e. interfaces and classes) support three types of members: methods, properties and
events.

### Member Errors

All xlang object members may fail to execute for a variety of reasons. Error information is returned
at the ABI layer and mapped to a language projection appropriate mechanism such as an exception for
further processing

### Methods

xlang objects support methods that take zero or more parameters and an optional return a value.
Methods must declare the names and types of their parameters. Methods must declare the type of their
return value, if there is one. 

Parameterized delegates and methods of parameterized interfaces may use type parameters for both
parameter and return value types.

xlang methods support four kinds of parameters: value parameters, const reference parameters, output
parameters, and array parameters.

A value parameter is used to pass an input parameter. A value parameter gets its initial value from
the argument that was passed for the parameter. Modifications to a value parameter do not affect the
argument that was passed for the parameter. So, a value parameter is effective a const input parameter.

A const reference parameter is used for efficient input parameter passing of large value types.
Methods receive such parameters as a reference to the actual value, rather than receiving a copy of
the actual value. For large value types, it's more efficient to pass (to a function) a pointer-sized
reference to a value than to pass a copy of the actual value itself.

An output parameter is used for output parameter passing. For an output parameter, the caller
provides  a pointer to the location in which the callee should assign the value.

An array parameter is a parameter that takes or returns a collection of variables of the same type,
randomly accessed via computed indices. Arrays can be passed into or out of a method via a parameter
in three ways:

- PassArray – this style is used when the caller is providing an array to the method.
- FillArray – this style is used when the caller is providing an array for the method to fill, up to
  a maximum array size.
- ReceiveArray – this style is used when the caller is receiving an array that was allocated by the
  method.

xlang supports method overloading on parameter types but favors overloading on the number of input
parameters – aka the method's arity. This is done in order to ensure better support for weakly typed
languages such as JavaScript. Note, output and receive array parameters are not included in the
count of input parameters. Optionally, overloaded methods should be given a separate, unique name
that can be used when projecting into languages that don't support overloading.

xlang objects do not support operator overloading.

### Properties

Properties are a paired get/set methods with matching name and type that appear in language
projections as fields rather than methods.

Properties on parameterized interfaces may use a type parameter as the property type.

Properties may not be array types.

Properties must have a get method. A property getter method has no parameters and returns a value of
the property type. Properties with only a get method are called read only properties.

Properties may optionally have a set method. A property setter method has a single parameter of the
property type and returns void. Properties with both a get and set method are called read/write.

### Events

Events are paired add/remove listener methods with matching name and delegate type. They provide a
mechanism for the type to notify interested parties when something of interest happens.

xlang events must specify the listener type via a delegate. An event add listener method takes a
single parameter of the event delegate type and returns an event registration token. And event
remove listener method has a single event registration token argument and returns void.

## Interfaces

> Note, an xlang's type system interface is a different concept from [COM Interfaces](XDN05%20-%20xlang%20Binary%20Interface.md#com-interfaces)
in xlang's [ABI](XDN05%20-%20xlang%20Binary%20Interface.md). So while all xlang ABI interfaces must
eventually inherit from [IUnknown](XDN05%20-%20xlang%20Binary%20Interface.md#iunknown), interfaces
in xlang's type system do not. Ultimately inheriting from IUnknown is an implementation detail that
is not directly surfaced in xlang's type system.

An interface is a type that acts as a contract that can be implemented by multiple classes. An
interface specifies the signatures for a group of object members (aka methods, properties and
events) but does not specify an implementation for those object members.

Interfaces can be implemented by multiple different classes. Each class implementation of a given
interface's object members is unrelated to any other class's implementation of the same interface.

Interfaces may optionally inherit from a single other interface. All of the type members of the
base interface are considered part of the derived interface as well. Any class implementing an
interface that uses inheritance must implement all of the type members specified in all of the
interfaces in the inheritance tree.

> Note, WinRT interfaces do not support true inheritance. Rather, WinRT interfaces may optionally
declare that they require one or more other interfaces to additionally be implemented on a class.
This approach leads to inefficiencies at the ABI layer - both runtime overhead that stems from
having to call QueryInterface as well as memory overhead that stems from types having multiple
interface virtual memory tables. By limiting interfaces at the type system level to single
inheritance, xlang can implement them at the ABI layer using a single composite virtual method
table, avoiding the runtime and memory inefficiencies mentioned above.

All xlang interfaces need an GUID identifier. This identifier has no relevance at the type system
level, it is only used at the ABI level. An interface's GUID can either be explicitly provided or
it can be implicitly generated from the interface's fully namespace qualified name using the
algorithm defined in [RFC4122 section 4.3](https://tools.ietf.org/html/rfc4122#section-4.3).

Interfaces are not versionable. Once they have been defined, they may never be changed.

### Parameterized Interfaces

xlang interfaces support type parameterization. Parameterized interfaces specify one or more type
parameters that can be used as parameter types or return values of type members. Parameterized
interface members may declare argument and/or return value types in terms of the type parameters
instead of as concrete types.

Like non-parameterized interfaces, parameterized interfaces require a GUID identifier. This GUID can
be explicitly specified or implicitly generated from the type name.

Parameterized interfaces can inherit from both parameterized and non-parameterized interfaces.
When inheriting from a parameterized interface, the type parameters of the base interface must
be specified, though they may be specified in terms of the derived interface's type parameters.

A concrete interface is either a non-parameterized interface or a parameterized interface where all
of the type parameters have been fully specified. Non-parameterized interfaces may inherit from
concrete interfaces, but they may not inherit from a parameterized interface.

#### Parameterized Interface Examples

For example, xlang's foundation types includes a set of parameterized interfaces to represent
collections.

- `IIterable<T>` represents a collection that can be enumerated
- `IVector<T>` represents a collection that can be randomly accessed
- `IMap<K,V>` represents a collection that maps from keys to values

`IIterable<T>` is a parameterized interface. `IIterable<String>` is a concrete interface, as all
of the type parameters have been specified. 

Both `IVector<T>` and `IMap<K,V>` can be enumerated. `IVector<T>` derives from `IIterable<T>`.
In this case, the same type parameter is used for both `IVector` and `IIterable`. However,
`IMap<K,V>` derives from `IIterable<IKeyValuePair<K,V>>`. In this case, the `T` type parameter of
`IIterable` is instantiated as parameterized type `IKeyValuePair<K,V>`, using the type parameters
defined as part of of `IMap<K,V>`.

Using the same collection interfaces defined above, you could define an non parameterized interface
`IPropertySet` that inherits from `IMap<String, Object>`. Note the use of concrete types in the
declaration of IMap, where in the previous example we used type parameters.

## Delegates

Delegates are xlang types that act as a type-safe function pointer. They are similar conceptually
to an [interface](#interfaces) that declares one and only one method. Defining delegates as a
separate type from interfaces enables better integration with free functions and/or [anonymous
functions](https://en.wikipedia.org/wiki/Anonymous_function) in language projections. Delegates are
often (but not exclusively) used to declare the signature for xlang event handlers.

xlang delegates are named types and define a method signature. Delegate method signatures follow the
same rules for parameters as interface methods do.

Like interfaces, delegates require a GUID identifier. THis identifier can be provided explicitly or
generated implicitly from the type's name.

Like interfaces, delegates can be parameterized. Parameterized delegates may declare arguments
and/or their return value type in terms of the type parameters instead of as concrete types.

Delegates are not versionable. Once they have been defined, they may never be changed.

## Classes

Classes are xlang types that expose behavior and encapsulate data. Often times, classes can be activated

Classes are additively versionable. Subsequent versions of a given enum may add new members. 
Pre-existing members may not be removed or modified in any way.
