---
id: XDN07
title: xmeta Semantic Checking
authors: t-tiroma@microsoft.com, Kyaw.Thant@microsoft.com
status: draft
---

# Title: XDN12 - xmeta Semantic Checking

- Authors: Tim Romanski (t-tiroma@microsoft.com) and Will Thant (Kyaw.Thant@microsoft.com)
- Status: draft

## Abstract

This design note specifies the semantic checks performed when generating metadata files from xlang IDL files. The checks are based on the Class Declaration Language (CDL) specification and the [xlang Type System](https://github.com/Microsoft/xlang/blob/master/design_notes/XDN03%20-%20xlang%20Type%20System.md). The categories to be checked are namespaces, types, and members of namespaces and of each type.

Types include classes, structs, interfaces, enums, and delegates.

Note that these are purely semantic checks, and not syntax.

### Namespaces
#### Definitions:
Namespaces can include other namespaces with a **using namespace directive**. This has syntax `using <namespace-name>`. This allows members from the included namespace to be referenced.

You can also include namespaces or types through a **using alias directive**. This has syntax `using <identifier> = <namespace-or-type-name>;`.

A **namespace alias qualifier** refers to a namespace or a type. It guarantees that type name lookups are unaffected by the introduction of new types and members. This has syntax `identifier::identifier<A1,...,Ak>` where `<A1,...,Ak>` is optional.

#### Semantic checks:
1) The identifier of a using alias directive must be unique within its containing namespace body. This does not include other namespace bodies from the same namespace.

2) The identifier of a using alias directive cannot be the same as any member defined in its declaration space. This includes all other namespace bodies from the same namespace.
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

3) The identifier of a using alias directive cannot be `global`.

4) Any referenced using alias directive must be defined in the namespace body or compilation unit in which it occurs.
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

5) Using aliases can name a closed constructed type like `using Y = N1.A<Int32>;`, however the type argument cannot be generic.

6) For namespace alias qualifiers, they have the form `N::I<A1,...,Ak>`.
* If N is the identifier 'global', then the global namespace is searched for I. One of the following must be true:
  * The global namespace contains a namespace named I and K is zero.
  * The global namespace contains a non-generic type named I and K is zero.
  * The global namespace contains a type named I that has K type parameters.
* Otherwise, search the immediate containing namespace then each enclosing namespace until the a matching entity is located. N must be associated with a namespace with an extern alias directive or using alias directive, and one of the following must be true:
  * The namespace associated with N contains a namespace named I and K is zero.
  * The namespace associated with N contains a type named I with K type arguments.

### Namespace members
#### Definitions:
A **declaration space** is defined by namespace declarations. Each namespace has its own unique declaration space, and outside of all namespaces is a special declaration space called the **global declaration space**. The scope of the global declaration space is limited to the file it is contained in.

The scope defined by a file is called a **compilation unit**.

**Namespace members** include nested namespace declarations and type declarations.

**Types** include classes, structs, interfaces, enums, and delegates.

#### Semantic checks:
1) Only types from the declaration space of included namespaces can be referenced. This means that types from nested namespaces are unavailable, since they belong to a separate declaration space.
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

2) If two namespaces included define members with the same name, and the including namespace references that member name, it is ambiguous and is an error.

3) Namespace members in the same declaration space cannot have the same name.

4) Two namespace names cannot differ only by case.


### Classes
#### Definitions:
A **class modifier** can be added to a class declaration. A class can be modified as either `unsealed` or `static`, but not both. This is guaranteed by the xmeta parser. Classes are sealed by default, and must be declared as `unsealed` if you wish to inherit from it.

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

#### Semantic checks:
1) Classes must override all type members specified in all interfaces in its inheritance tree.

2) You can only derive from an `unsealed` class.

3) Rules for static classes:
* It cannot be used as a type, and it can only contain static members. This means all members must explicitly include a `static` modifier.
* You cannot define an instance constructor in a static class.
* You cannot derive from a static class.
* A static class can only inherit from static interfaces.
* You may only reference members of static classes with the format `T.I` where T is the static class and I is a member of T.

4) Rules for type parameters:
* Type parameters cannot contain duplicates.
* Type parameters cannot have the same name as a member declared in the class. 
* Type parameters cannot have the same name as the type itself.

5) Rules for base classes:
* Base classes cannot have the same identifier as a type parameter.
* A class cannot depend on itself.

6) Rules for partial classes:
* Same as regular classes.

7) You can only define the same class modifier once in a class declaration.

### Class members
#### Definitions:

Class members include **methods**, **properties**, **events**, and **instance constructors**.

Class members are either **staic members** or **instance members**. Static members belong to the class itself, and instance membes belong to object instances of the class.

A **constituent type** is a type used in the declaration of a member. These includes the type of a property, an event, the return type of a method, and the parameter types of a method or instance constructor.

Each property and event creates two **reserved member names**. For example, a property `P` of type `T` reserves `T get_P();` and `void set_P(T value);`. These are required by the Windows Runtime. The format will always remain the same, i.e. prepend `set_` and `get_` to the name, the return type of `get_` will be `T` and will have no parameters, and `set_` will have return type `void` and one parameter of type `T`. These get propagated to any inheriting class.

The **formal parameters** of a method are the parameters enclosed by brackets `(` and `)`.

**Type parameters** define a generic type used for constructed types. For a declaration of example method `m`: `void m<S,T>();`, `S` and `T` are the type parameters.

An **overridden base method** is a method in a parent class that is being overridden. If class `A` overrides method `m`, the next direct parent class is checked for `m`, then the parent of the parent class if one exists, and so on. Only `public` and `protected` methods can be matched for an override.

#### Semantic checks:

Note: inherited members are not considered to be within the class declaration space, so the names of these inherited members do not influence the following rules.

1) No members other than instance constructors may have the same name as the immediately enclosing class.

2) The constituent types of a member must be at least as accessible as that member itself.

3) Rules for methods:
* The name of each method must differ from all other non-method members declared in the same class.
* The signature of each method must differ from the signatures of all other methods declared in the same class. It must also differ from methods declared in inherited classes, unless the override modifier is used. Signatures cannot differ solely by ref and out.
* Methods may not match the signature of a reserved member name. Even if class B derives from class A, B's methods cannot match the signature of reserved member names created by A.
* The return type of each parameter type must be at least as accessible as the method itself.
* All formal parameters and type parameters of a method declaration must have different names.
* For overridden methods:
  * An overridden base method must be found.
  * There is exactly one overridden base method that matches.
  * The overridden base method is not static.
  * The method cannot change the accessibility of the overriden base method.

4) Rules for properties:
* The name of each property must differ from all other member names declared in the same class.
* The type of a property must be at least as accessible as the property itself.
* `get` and `set` accessors of properties can only be declared once.
* A property cannot be passed as a parameter of type `ref` or `out`.

5) Rules for events:
* The name of each event must differ from all other member names declared in the same class.
* The type of the event must be at least as accessible as the event itself.
* You can only define the same event modifier once in an event declaration.

6) Rules for instance constructors:
* Must have the same name as the immediately enclosing class.
* The signatures of each instance constructor must be unique, and they cannot differ solely by ref and out.
* All formal parameters of an instance constructor must have different names.

### Structs

### Struct members
#### Definitions:
Struct members include: **fields**, **enums**, and **structs**.

#### Semantic checks:
1) The semantic checks of class members apply to struct members as well.

### Interfaces
#### Definitions:
The **required interfaces** of an interface are the explicit required interfaces and their required interfaces.

#### Semantic checks:
1) An interface cannot directly or indirectly inherit from itself.

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
#### Definitions:
The process of matching a declaration of a member in a class to the corresponding interface member is known as **interface mapping**.

#### Semantic checks:
1) The name of a method must differ from the names of all properties and events declared in the same interface. 

2) The signature of a method must differ from the signatures of all other methods declared in the same interface, and two methods declared in the same interface may not have signatures that differ solely by ref and out.

3) The name of a property or event must differ from the names of all other members declared in the same interface.

4) Interface mapping
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
#### Definitions:
An **enum type** is a distinct value type that declares a set of named constants.

#### Checks:
1) An enum type can only be Int32 or UInt32.

<!--
Enums with an underlying type of UInt32 must carry the FlagsAttribute. Enums with an underlying type of Int32 must not carry the FlagsAttribute.
Versioning: Enums are additively versionable. Subsequent versions of a given enum may add values (aka named constants). Pre-existing values may not be removed or changed. Enum values optionally carry the VersionAttribute to distinguish when specific values were added to the enum type. Enum values without a VersionAttribute are considered to have the same version value as the enclosing enum type.
-->

### Enum members
#### Definitions:
Each enum member has its own constant value. This can be assigned **explicitly** or **implicitly**. Explicitly the syntax is `<member-name> = <constant-expression>` followed by an optional comma. Implicitly you just leave the `<member-name>`. Implicit assignment works with the following rules:
* If it is the first member, assign it zero.
* Otherwise, assign it the value of the immediately preceding enum member plus one.

For example:
```
enum Color
{
       Red,
       Green = 10,
       Blue
}
```
The associated values are: Red = 0, Green = 10, and Blue = 11 for the following reasons:
* Red is 0 since it is the first member and is not assigned a constant expression.
* Green is explicitly given the value 10;
* Blue is automatically assigned the value one greater than the member that precedes it.

An enum member can **explicitly depend** or **implicitly depend** on other members. It explicitly depends on another member if its assigned constant expression contains the member. A member implicitly depends on another member if it is not assigned a constant expression and it is not the first member. In this case, it implicitly depends on the directly preceding member.

For example:
```
enum Color
{
    Red = 11,
    Blue = Red,
    Green
}
```
Here, Blue explicitly depends on Red. Green implicitly depends on Blue.

#### Semantic checks:
1) The constant value for each enum member must be in the range of the underlying type for the enum. 

For example:
```
enum Color: UInt32
{
       Red = -1,
       Green = -2,
       Blue = -3
}
```
results in a compile-time error because the constant values -1, -2, and –3 are not in the range of the underlying integral type UInt32.

2) Member names must be unique to the enum.

3) The dependency tree of an enum cannot contain itself.

For example:
```
enum Circular
{
       A = B,
       B
}
```
B implicitly depends on A which explicitly depends on B. Thus, B is within its own dependency tree, which is an error.

4) The following operators can be used on values of enum types: binary + (§5.4.4), binary   (§5.4.5), ^, &, | (§5.6.2), and ~ (§5.3.3).

### Delegates
#### Definitions:
**Delegates** enable scenarios that other languages—such as C++, Pascal, and Modula—have addressed with function pointers.

#### Semantic checks:
1) Delegate names must be unique in their declaration space.

2) The delegate parameter list and type parameters follow the same rules as normal methods.


### Attributes
#### Definitions:
The attributes in the xlang grammar are generalized and do not specify any keywords except for the attributes_target that are listed.
As such there is much work needed to semantically check the meaning of each attribute in the grammar. Particularly, there is no difference in the grammar between global and regular attributes. We will have the check the attribute targets semantically to tell the difference and apply the right rules. 

**Positional parameters** are a sequence of parameters without a identifier
**Named parameters** are parameters with a specified name

##### AttributeUsage
**AttributeUsage** is used to describe how an attribute class can be used. It is not defined as a keyword in the grammar and it is the semantic checker's job to make sure it is used correctly on attribute declaration.
**AttributeUsage** has positional parameters used to specify the types of declarations on which it can be used: AttributeTargets.Class | AttributeTarget.Interface
It also has named parameters such as **AllowMultiple** and **Inherited**. 
**AllowMultiple** allows the attribute to be specified more than once for a given entity. If unspecified, the attribute is always single use.
**Inherited** indicates whether or not the attribute is inherited by classes that derive from the class the attribute was originally defined on. If unspecified, its default value is true. 


#### Semantic checks:
1) The types of the input parameters passed into the attribute constructor must match the types specified in the constructor's declaration.

2) References to attributes must refer to defined attributes within scope.

For example:
```
class Class1 {}
\[Class1\] class Class2 {}	// Error
results in a compile-time error because it attempts to use Class1 as an attribute class when Class1 is not an attribute class. If a class wants to be an attribute, it must use the **AttributeUsage**. 

3) Attribute targets
Certain contexts permit the specification of an attribute on more than one target. A program can explicitly specify the target by including an attribute-target-specifier. When an attribute is placed at the global level, a global-attribute-target-specifier is required. In all other locations, a reasonable default is applied, but an attribute-target-specifier can be used to affirm or override the default in certain ambiguous cases (or to just affirm the default in non-ambiguous cases). Thus, typically, attribute-target-specifiers can be omitted except at the global level. 

The potentially ambiguous contexts are resolved as follows:

•	An attribute specified at global scope can apply either to the target assembly or the target module. No default exists for this context, so an attribute-target-specifier is always required in this context. The presence of the assembly attribute-target-specifier indicates that the attribute applies to the target assembly; the presence of the module attribute-target-specifier indicates that the attribute applies to the target module. 

•	An attribute specified on a delegate declaration can apply either to the delegate being declared or to its return value. In the absence of an attribute-target-specifier, the attribute applies to the delegate. The presence of the type attribute-target-specifier indicates that the attribute applies to the delegate; the presence of the return attribute-target-specifier indicates that the attribute applies to the return value.

•	An attribute specified on a method declaration can apply either to the method being declared or to its return value. In the absence of an attribute-target-specifier, the attribute applies to the method. The presence of the method attribute-target-specifier indicates that the attribute applies to the method; the presence of the return attribute-target-specifier indicates that the attribute applies to the return value.

•	An attribute specified on an operator declaration can apply either to the operator being declared or to its return value. In the absence of an attribute-target-specifier, the attribute applies to the operator. The presence of the method attribute-target-specifier indicates that the attribute applies to the operator; the presence of the return attribute-target-specifier indicates that the attribute applies to the return value.

•	An attribute specified on an event declaration that omits event accessors can apply to the event being declared, to the associated field (if the event is not abstract), or to the associated add and remove methods. In the absence of an attribute-target-specifier, the attribute applies to the event. The presence of the event attribute-target-specifier indicates that the attribute applies to the event; the presence of the field attribute-target-specifier indicates that the attribute applies to the field; and the presence of the method attribute-target-specifier indicates that the attribute applies to the methods.

•	An attribute specified on a get accessor declaration for a property or indexer declaration can apply either to the associated method or to its return value. In the absence of an attribute-target-specifier, the attribute applies to the method. The presence of the method attribute-target-specifier indicates that the attribute applies to the method; the presence of the return attribute-target-specifier indicates that the attribute applies to the return value.

•	An attribute specified on a set accessor for a property or indexer declaration can apply either to the associated method or to its lone implicit parameter. In the absence of an attribute-target-specifier, the attribute applies to the method. The presence of the method attribute-target-specifier indicates that the attribute applies to the method; the presence of the param attribute-target-specifier indicates that the attribute applies to the parameter; the presence of the return attribute-target-specifier indicates that the attribute applies to the return value.

•	An attribute specified on an add or remove accessor declaration for an event declaration can apply either to the associated method or to its lone parameter. In the absence of an attribute-target-specifier, the attribute applies to the method. The presence of the method attribute-target-specifier indicates that the attribute applies to the method; the presence of the param attribute-target-specifier indicates that the attribute applies to the parameter; the presence of the return attribute-target-specifier indicates that the attribute applies to the return value.

a) There are some invalid target attribute that can be applied to declarations
Example: param cannot be used on a class declaration 
```
[param: Author("Brian Kernighan")]		// Error
class Class1 {}
```

4) Attribute usages cannot be ambiguous. Ambiguities arise because by convention, attribute classes are named with a suffix of Attribute, and you can reference the attribute without this suffix. However, there may be another attribute with the same name without the Attribute suffix, which causes the ambiguity.

For example:
```
[AttributeUsage(AttributeTargets.All)]
attribute X
{}
[AttributeUsage(AttributeTargets.All)]
attribute XAttribute
{}
[X]                 // Error: ambiguity
class Class1 {}
[XAttribute]        // Refers to XAttribute
class Class2 {}
[@X]                // Refers to X
class Class3 {}
[@XAttribute]       // Refers to XAttribute
class Class4 {}
```

5) Single use attributes can only be used once on the same entity.

6) TODO: More on expressions later

7) Attributes cannot have the name of a reserved attribute. These include: AttributeUsageAttribute

8) A class that is decorated with the AttributeUsage attribute must derive from System.Attribute, either directly or indirectly.

The AttributeUseageAttribute is defined as follows:
```
namespace System
{
    [AttributeUsage(AttributeTargets.Class)]
    class AttributeUsageAttribute: Attribute
    {
        AttributeUsageAttribute(AttributeTargets validOn) {...}
        Boolean AllowMultiple { get {...} set {...} }
        Boolean Inherited { get {...} set {...} }
        AttributeTargets ValidOn { get {...} }
    }
    enum AttributeTargets
    {
        Assembly = 0x0001,
        Module = 0x0002,
        Class = 0x0004,
        Struct = 0x0008,
        Enum = 0x0010,
        Constructor = 0x0020,
        Method = 0x0040,
        Property = 0x0080,
        Field = 0x0100,
        Event = 0x0200,
        Interface = 0x0400,
        Parameter = 0x0800,
        Delegate = 0x1000,
        ReturnValue = 0x2000,
        All = Assembly | Module | Class | Struct | Enum | Constructor | 
                Method | Property | Field | Event | Interface | Parameter | 
                Delegate | ReturnValue
    }
}
```

