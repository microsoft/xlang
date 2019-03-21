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

This design note specifies the semantic checks performed when generating metadata files from xlang IDL files. The checks are based on the Class Declaration Language (CDL) specification and the [xlang Type System](https://github.com/Microsoft/xlang/blob/master/design_notes/XDN03%20-%20xlang%20Type%20System.md). The categories to be checked are namespaces, types, and members of namespaces and of each type.

Types include classes, structs, interfaces, enums, and delegates.

### Namespaces

### Namespace members

##### Definitions:
A **declaration space** is defined by namespace declarations. Each namespace has its own unique declaration space, and outside of all namespaces is a special declaration space called the **global declaration space**. The scope of the global declaration space is limited to the file it is contained in. 
The scope defined by a file is called a **compilation unit**.
**Namespace members** include nested namespace declarations and type declarations.
**Types** include classes, structs, interfaces, enums, and delegates.
Namespaces can include other namespaces with a **using namespace directive**. This has syntax `using <namespace-name>`. This allows members from the included namespace to be referenced.
You can also include namespaces or types through a **using alias directive**. This has syntax `using <identifier> = <namespace-or-type-name>;`.
A **namespace alias qualifier** refers to a namespace or a type. It guarantees that type name lookups are unaffected by the introduction of new types and members. This has syntax `identifier::identifier<A1,...,Ak>` where `<A1,...,Ak>` is optional.

##### Semantic checks:
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

##### Definitions:

A **class modifier** can be added to a class declaration. These include `sealed` and `static`.

You can define a class as **partial** with syntax `partial <class-name>`.

All classes are **public** by default without any keywords added, and you cannot modify that.

A **type parameter** allows classes to have generic types. These have the form `class S<T1, ...,Tk>` where T1 through Tk are the type parameters.

A class **depends** on classes it inherits from, all classes inherited from the classes it inherits from, and so on.
For example:
```
class A { }
class B { }
class C: A, B { }
class D: C { }
```
Here, class D depends on classes A, B, and C.

##### Semantic checks:

Rules for sealed classes:
* You cannot derive from a sealed class.

Rules for static classes:
* You cannot instantiate a static class.
* It cannot be used as a type, and it can only contain static members. This means all members must explicitly include a `static` modifier.
* You cannot define an instance constructor in a static class.
* You cannot derive from a static class.
* A static class can only inherit from static interfaces.
* You may only reference static classes with the format `T.I` where T is the static class and I is a member of T.

Rules for type parameters:
* Type parameters cannot contain duplicates.
* Type parameters cannot have the same name as a member declared in the class. 
* Type parameters cannot have the same name as the type itself.

Rules for base classes:
* Base classes cannot be the same as a type parameter.
* A class cannot depend on itself.

Rules for partial classes:
* As in regular cl

### Class members

##### Definitions:

Class members include **methods**, **properties**, **events**, and **instance constructors**.

Class members are either **staic members** or **instance members**. Static members belong to the class itself, and instance membes belong to object instances of the class.

A **constituent type** is a type used in the declaration of a member. These includes the type of a property, an event, the return type of a method, and the parameter types of a method or instance constructor.

Each property and event creates two **reserved member names**. For example, a property `P` of type `T` reserves `T get_P();` and `void set_P(T value);`. These are required by the Windows Runtime. The format will always remain the same, i.e. prepend `set_` and `get_` to the name, the return type of `get_` will be `T` and will have no parameters, and `set_` will have return type `void` and one parameter of type `T`. These get propagated to any inheriting class.

The **formal parameters** of a method are the parameters enclosed by brackets `(` and `)`.

**Type parameters** define a generic type used for constructed types. For a declaration of example method `m`: `void m<S,T>();`, `S` and `T` are the type parameters.

An **overridden base method** is a method in a parent class that is being overridden. If class `A` overrides method `m`, the next direct parent class is checked for `m`, then the parent of the parent class if one exists, and so on. Only `public` and `protected` methods can be matched for an override.

##### Semantic checks:

Note: inherited members are not considered to be within the class declaration space, so the names of these inherited members do not influence the following rules.

No members other than instance constructors may have the same name as the immediately enclosing class.

A class member declaration can have at most one accessibility modifier, which can be one of `public` or `protected`.

The constituent types of a member must be at least as accessible as that member itself.

Rules for methods:
* The name of each method must differ from all other non-method members declared in the same class.
* The signature of each method must differ from the signatures of all other methods declared in the same class. It must also differ from methods declared in inherited classes, unless the override modifier is used. Signatures cannot differ solely by ref and out.
* Methods may not match the signature of a reserved member name. Even if class B derives from class A, B's methods cannot match the signature of reserved member names created by A.
* If a method is declared as `sealed`, it must also have the `override` modifier.
* If a method is `partial`, it does not include the modifiers: `public`, `protcted`, `sealed`, or `override`.
* The return type of each parameter type must be at least as accessible as the method itself.
* All formal parameters and type parameters of a method declaration must have different names.
* For overridden methods:
  * An overridden base method must be found.
  * There is exactly one overridden base method that matches.
  * The overridden base method is not static.
  * The method cannot change the accessibility of the overriden base method.
  * The overridden base method cannot be sealed.

Rules for properties:
* The name of each property must differ from all other member names declared in the same class.
* If a property is declared as `sealed`, it must also have the `override` modifier.
* If a property is `partial`, it does not include the modifiers: `public`, `protected`, `sealed`, or `override`.
* The type of a property must be at least as accessible as the property itself.
* `get` and `set` accessors of properties can only be declared once.
* A property cannot be passed as a parameter of type `ref` or `out`.

Rules for events:
* The name of each event must differ from all other member names declared in the same class.
* The type of the event must be at least as accessible as the event itself.

Rules for instance constructors:
* Must have the same name as the immediately enclosing class.
* The signatures of each instance constructor must be unique, and they cannot differ solely by ref and out.
* All formal parameters of an instance constructor must have different names.

### Structs

### Struct members

### Interfaces
##### Definitions:
The **required interfaces** of an interface are the explicit required interfaces and their required interfaces.

##### Checks:
1) It is a compile-time error for an interface to directly or indirectly inherit from itself.

2) All Xlang interfaces must inherit directly from IInspectable, which in turn inherits from IUnknown.

3) Interfaces Requires:
Interfaces may specify that they require one or more other interfaces that must be implemented on any object that implements the current interface. For example, if IButton required IControl then any class implementing IButton would also need to implement IControl.
Adding new functionality by implementing new interfaces that inherit from existing interfaces (i.e. IFoo2 inherits from IFoo) is not allowed. (How is this different from require interfaces???)
The required interfaces of an interface are the explicit required interfaces and their required interfaces. In other words, the set of required interfaces is the complete transitive closure of the explicit required interfaces, their explicit required interfaces, and so on. In the example
```
interface IControl
{
	void Paint();
}
interface ITextBox: IControl
{
	void SetText(String text);
}
interface IListBox: IControl
{
	void SetItems(String[] items);
}
interface IComboBox: ITextBox, IListBox {}
```
the required interfaces of IComboBox are IControl, ITextBox, and IListBox.

A class that implements an interface also implicitly implements all of the interface’s required interfaces. (What does implicitly means in this case?)

The members of an interface are the members declared by the interface itself, but do not include the members from the required interface

4) Parameterized Interface
A required interface of a parameterized interface may share the same type argument list, such that a single type argument is used to specify the parameterized instance of both the interface and the interface it requires (eq IVector<T> requires IIterable<T>). The signature of any member (aka method, property or event) of a parameterized interface may reference a type from the parameterized interface’s type arguments list. (eq. IVector<T>.SetAt([in] UInt32 index, [in] T value)).

The inherited members of an interface are specifically not part of the declaration space of the interface. Thus, an interface is allowed to declare a member with the same name or signature as an inherited member. 

5) The interfaces referenced by a generic type declaration must remain unique for all possible constructed types. Without this rule, it would be impossible to determine the correct method to call for certain constructed types. For example, suppose a generic class declaration were permitted to be written as follows:
```
interface I<T>
{
	   void F();
}
class X<U,V>: I<U>, I<V>					// Error: I<U> and I<V> conflict
```

To determine if the interface list of a generic type declaration is valid, the following steps are performed:

•	Let L be the list of interfaces directly specified in a generic class, struct, or interface declaration C.

•	Add to L any required interface of the interfaces already in L.

•	Remove any duplicates from L.

•	If any possible constructed type created from C would, after type arguments are substituted into L, cause two interfaces in L to be identical, then the declaration of C is invalid

### Interface members
##### Definitions:
The process of matching a declaration of a member in a class to the corresponding interface member is known as **interface mapping**.

##### Checks:
1) The name of a method must differ from the names of all properties and events declared in the same interface. 

The signature of a method must differ from the signatures of all other methods declared in the same interface, and two methods declared in the same interface may not have signatures that differ solely by ref and out.

The name of a property or event must differ from the names of all other members declared in the same interface.

2) Interface mapping
For purposes of interface mapping, a class member A matches an interface member B when:
•	A and B are methods, and the name, type, and formal parameter lists of A and B are identical.
•	A and B are properties, the name and type of A and B are identical, and A has the same accessors as B.
•	A and B are events, and the name and type of A and B are identical.

NOTE: There is abit more to interface mapping then I thought. Apparently in the CDL methods can implement methods from the interface using this syntax. 
```
interface IArtist
{
	void Draw();
}
interface ICowboy
{
	void Draw();
}
class CowboyArtist : ICowboy, IArtist
{
	void DrawWeapon() implements ICowboy.Draw;
	void DrawPainting() implements IArtist.Draw;
}
```
This was not specified in the grammar and I am unsure whether we will support this in Xlang. 

### Enums
##### Definitions:
An **enum type** is a distinct value type that declares a set of named constants.

##### Checks:
1) Enums with an underlying type of UInt32 must carry the FlagsAttribute. Enums with an underlying type of Int32 must not carry the FlagsAttribute.

2) Enums must have public visibility.

3) Versioning:
Enums are additively versionable. Subsequent versions of a given enum may add values (aka named constants). Pre-existing values may not be removed or changed. Enum values optionally carry the VersionAttribute to distinguish when specific values were added to the enum type. Enum values without a VersionAttribute are considered to have the same version value as the enclosing enum type.

### Enum members
1) The constant value for each enum member must be in the range of the underlying type for the enum. The example
```
enum Color: UInt32
{
	   Red = -1,
	   Green = -2,
	   Blue = -3
}
```
results in a compile-time error because the constant values -1, -2, and –3 are not in the range of the underlying integral type UInt32.

2) Multiple enum members may share the same associated value. The example
```
enum Color 
{
	   Red,
	   Green,
	   Blue,
	Max = Blue
}
```
shows an enum in which two enum members—Blue and Max—have the same associated value.

3) If the declaration of the enum member has a constant-expression initializer, the value of that constant expression, implicitly converted to the underlying type of the enum. 

If the declaration has no initializer, it is set implicity as follows: 
If the enum member is the first enum member declared in the enum type, its associated value is zero.

Otherwise, the associated value of the enum member is obtained by increasing the associated value of the textually preceding enum member by one. This increased value must be within the range of values that can be represented by the underlying type, otherwise a compile-time error occurs.
The example
```
using System;
enum Color
{
	   Red,
	   Green = 10,
	   Blue
}
```
The associated values are:
Red = 0
Green = 10
Blue = 11
for the following reasons:
•	the enum member Red is automatically assigned the value zero (since it has no initializer and is the first enum member);
•	the enum member Green is explicitly given the value 10;
•	and the enum member Blue is automatically assigned the value one greater than the member that textually precedes it.
 

The associated value of an enum member may not, directly or indirectly, use the value of its own associated enum member. Other than this circularity restriction, enum member initializers may freely refer to other enum member initializers, regardless of their textual position. Within an enum member initializer, values of other enum members are always treated as having the type of their underlying type, so that casts are not necessary when referring to other enum members. 
The example
```
enum Circular
{
	   A = B,
	   B
}
```
results in a compile-time error because the declarations of A and B are circular. A depends on B explicitly, and B depends on A implicitly.

4) The following operators can be used on values of enum types: binary + (§5.4.4), binary   (§5.4.5), ^, &, | (§5.6.2), and ~ (§5.3.3).

### Delegates
##### Definitions:
**Delegates** enable scenarios that other languages—such as C++, Pascal, and Modula—have addressed with function pointers. 

##### Checks:
1) Delegates require a GUID identifier. This identifier can be provided explicitly or generated implicitly from the type's name.

2) Delegate types in CDL are name equivalent, not structurally equivalent. Specifically, two different delegate types that have the same parameter lists and return type are considered different delegate types.

3) Type parameter Example:
```
delegate Boolean Predicate<T>(T value);
class X
{
	static Boolean F(Int32 i);
	static Boolean G(String s);
}
``` 
The method X.F is compatible with the delegate type Predicate<Int32> and the method X.G is compatible with the delegate type Predicate<String> .
