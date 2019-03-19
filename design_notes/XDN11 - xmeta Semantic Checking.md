---
id: XDN07
title: xmeta Semantic Checking
author: t-tiroma@microsoft.com
status: draft
---

# Title: XDN11 - xmeta Semantic Checking

- Author: Tim Romanski (t-tiroma@microsoft.com)
- Status: draft

## Abstract

This design note specifies the semantic checks performed when generating metadata files from xlang IDL files. The checks are based on the Class Declaration Language (CDL) specification and the xlang Type System. The categories to be checked are namespaces, types, and members of namespaces and of each type.

Types include classes, structs, interfaces, enums, and delegates.

### Namespaces

### Namespace members

##### Definitions:
A **declaration space** is defined by namespace declarations. Each namespace has its own unique declaration space, and outside of all namespaces is a special declaration space called the **global declaration space**. The scope of the global declaration space is limited to the IDL file it is contained in.

The scope defined by an IDL file is called a **compilation unit**.

**Namespace members** include nested namespace declarations and type declarations.

**Types** include classes, structs, interfaces, enums, and delegates.

Namespaces can include other namespaces with a **using namespace directive**. This has syntax `using <namespace-name>`. This allows members from the included namespace to be referenced.

You can also include namespaces or types through a **using alias directive**. This has syntax `using <identifier> = <namespace-or-type-name>;`.

A **namespace alias qualifier** refers to a namespace or a type. It guarantees that type name lookups are unaffected by the introduction of new types and members. This has syntax `identifier::identifier<A1,...,Ak>` where `<A1,...,Ak>` is optional.  

##### Checks:
Only types from the declaration space of included namespaces can be referenced. This means that types from nested namespaces are unavailable, since they belong to a separate declaration space.
For example:
```
namespace N1
{
    class A { }
    namespace N2
    {
        class B { }
    }
}

namespace N3
{
    using N1;
    class C: A { } // OK
    class D: B { } // Error, B is in N1.N2's declaration space, which is not included.
}
```
Class B was referenced, but its declaration space was not included. This is an error.

If two namespaces included define members with the same name, and the including namespace references that member name, it is ambiguous and is an error.

Namespace members in the same declaration space cannot have the same name.

The identifier of a using alias directive must be unique within its declaration space, and cannot be the same as any member defined in the declaration space.
For example:
```
namespace N3
{
    class B { }
}
namespace N3
{
    using E = N1.A;    // OK
    using E = N1.N2.B; // Error, E already exists in N3.
    using B = N1.N2.B; // Error, B already exists in N3.
}
```

Any referenced using alias directive must be defined in the namespace body or compilation unit in which it occurs.
For example:
```
using R = N1.N2;
namespace N3
{
    using S = N1;
    class C: R.B { } // OK, since R is defined in the compilation unit.
}
namespace N3
{
    class D: S.A; // Error, since S is not defined in the current namespace body or compilation unit.
}
```

Using aliases can name a closed constructed type like `using Y = N1.A<Int32>;`, however the type argument cannot be generic.

For namespace alias qualifiers, they have the form `N::I<A1,...,Ak>`.
* If N is the identifier 'global', then the global namespace is searched for I. One of the following must be true:
  * The global namespace contains a namespace named I and K is zero.
  * The global namespace contains a non-generic type named I and K is zero.
  * The global namespace contains a type named I that has K type parameters.
* Otherwise, search the immediate containing namespace then each enclosing namespace until the a matching entity is located. N must be associated with a namespace with an extern alias directive or using alias directive, and one of the following must be true:
  * The namespace associated with N contains a namespace named I and K is zero.
  * The namespace associated with N contains a type named I with K type arguments.



### Classes

### Class members

### Structs

### Struct members

### Interfaces

### Interface members

### Enums

### Enum members

### Delegates

### Delegate members
