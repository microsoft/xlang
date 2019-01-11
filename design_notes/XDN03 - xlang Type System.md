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
Int16      | a 16 bit signed integer
Int32      | a 32 bit signed integer
Int64      | a 64 bit signed integer
UInt8      | an 8 bit unsigned integer
UInt16     | a 16 bit unsigned integer
UInt32     | a 32 bit unsigned integer
UInt64     | a 64 bit unsigned integer
Single     | a 32-bit IEEE 754 floating point number
Double     | a 64-bit IEEE 754 floating point number
Char16     | a 16-bit non-numeric value representing a UTF-16 code unit
Boolean    | an 8-bit Boolean value
String     | an immutable sequence of Char16s used to represent text
Guid       | A 128-bit standard [universally unique identifier](https://en.wikipedia.org/wiki/Universally_unique_identifier)
Object     | An xlang object of unknown type

> Note: string encoding for xlang is currently under review. In particular, it has not been decided
if xlang will support UTF-8 strings in some capacity. A design note will be commissioned for this
decision.

## Enums

An enum type is a distinct value type with a set of named values. Each named value in an enum
corresponds to a constant integer value.

Each enum type has an underlying integral type. The only legal underlying integral types for enums
in xlang are Int32 and UInt32.

An enum type with an underlying integral type of UInt32 is considered a Flags enum. Flags enums
are intended to be treated as a bit field. Language projections can and should provide logical
operators such as and, or and not for Flags enum values.

Enums are additively versionable. Subsequent versions of a given enum may add new named values (and
associated constant integer values). Pre-existing values may not be removed or changed.

## Structs

Structs are a record type with one or more fields. Structs are always passed and returned by value.
Struct fields may only be enums, structs and fundamental types (including strings).

Structs must have at least one field. All of a struct's fields are public.

Structs cannot be generic or parameterized.

Structs are not versionable. Once they have been defined, they may never be changed.

## Object Members

### Methods

### Properties

### Events

## Interfaces

> Note, an xlang's type system interface is a different concept from
[COM Interfaces](XDN05%20-%20xlang%20Binary%20Interface.md#com-interfaces) in xlang's
[ABI](XDN05%20-%20xlang%20Binary%20Interface.md).
So while an xlang ABI interface must inherit from
[IXlangObject](XDN05%20-%20xlang%20Binary%20Interface.md#ixlangobject), interfaces in the xlang
type system do not. Inheriting from IXlangObject is an implementation detail that is not directly
surfaced in xlang's type system.

An interface is a type that acts as a contract that can be implemented by multiple classes. An
interface specifies the signatures for a group of object members (aka methods, properties and
events) but does not specify an implementation for those object members.

Interfaces can be implemented by multiple different classes. Each class implementation of a given
interface's object members is unrelated to any other class's implementation of the same interface.

Interfaces may optionally inherit from a single other interface. All of the type members of the
base interface are considered part of the derived interface as well. Any class implementing an
interface that uses inheritance must implement all of the type members specified by both the base
and derived interfaces.

> Note, WinRT interfaces support inheritance from multiple other interfaces. This approach leads to
inefficiencies at the ABI layer that xlang wishes to avoid by constraining interfaces to single
inheritance. This decision may be revisited as the xlang project progresses.

Interfaces are not versionable. Once they have been defined, they may never be changed.

### Parameterized Interfaces

xlang interfaces support type parameterization. Parameterized interfaces specify one or more type
parameters that can be used as parameter types or return values of type members.

Parameterized interfaces can inherit from both parameterized and non-parameterized interfaces.
When inheriting from a parameterized interface, the type parameters of the base interface must
be specified, though they may be specified in terms of the derived interface's type parameters.

Non-parameterized interfaces can only inherit from concrete interfaces. This includes
non-parameterized interfaces as  well as parameterized interfaces where all of the type parameters
have been fully specified.

Array types may not be used as type parameters.

#### Parameterized Interface Examples

For example, xlang's foundation types includes a set of parameterized interfaces to represent
collections.

- `IIterable<T>` represents a collection that can be enumerated
- `IVector<T>` represents a collection that can be randomly accessed
- `IMap<K,V>` represents a collection that maps from keys to values

Both `IVector<T>` and `IMap<K,V>` can be enumerated. `IVector<T>` derives from `IIterable<T>`.
In this case, the same type parameter is used for both `IVector` and `IIterable`. However,
`IMap<K,V>` derives from `IIterable<IKeyValuePair<K,V>>`. In this case, the `T` type parameter of
`IIterable` is instantiated as parameterized type `IKeyValuePair<K,V>`, using the type parameters
defined as part of of `IMap<K,V>`.

Using the same collection interfaces defined above, you could define an non parameterized interface
`IPropertySet` that inherits from `IMap<String, Object>`. Note the use of concrete types in the
declaration of IMap, where in the previous example we used type parameters.

## Delegates

## Classes