---
title: A Strategy for Language Interoperability
author: "hpierson@microsoft.com"
---

Title: ​LDN01 - A Strategy for Language Interoperability
=======================================================

-   Author: Harry Pierson (hpierson\@microsoft.com)

-   Status: Draft

Abstract
--------

This design note describes the goals and guiding principles of this Langworthy.

Revisions
---------

-   2018-07-05, First Draft by Harry Pierson

Overview
--------

Langworthy is a system to enable code written in one language to be callable
from another. Interoperability is achieved via a combination of language
independent metadata, binary interface standards and cross platform tooling.

Langworthy is based on technology originally developed for the Windows Runtime
during the Windows 8 product cycle. However, it is an explicit goal for
Langworthy to support multiple platforms beyond just Windows. One of
Langworthy’s core scenarios is to enable a component written in C++ to be
callable directly by platform’s native language. For example, enabling a
component written in C++ to be called from Objective C on iOS, from Java on
Android and from C\# on Windows.

At the time of this writing (July 2018), Langworthy is an unannounced internal
Microsoft project. However, it is an explicit goal for Langworthy to be run as
an open source project, including taking design feedback and code updates from
the community at large. When Langworthy is announced, all documentation and code
assets will be moved to a publicly available location (aka GitHub) so that
interested parties from across the industry can participate in the project.
Langworthy will be managed similarly to other Microsoft developer focused open
source projects such as [.NET Core](https://dotnet.github.io/) and
[Roslyn](https://github.com/dotnet/roslyn).

As an open source project, Langworthy will be made available to the community at
large under a permissive open source license. The specific license Langworthy
will use is to be determined.

Conceptual Model
----------------

Delivering on this vision requires an ecosystem of software built around a set
of foundational concepts.

### Primordial Type System

Langworthy’s type system is explicitly designed \*not\* to be accessed directly.
Langworthy components are written using the type system of the implementation
language and accessed using the type system of the consuming language. As such,
the type system is designed to interoperate with modern object-oriented
programming languages. This includes both static (such as C\#) and dynamic (such
as Python) OO languages as well as strongly typed (such as C++) and weakly typed
(such as JavaScript) languages.

LDN03 describes Langworthy’s type system in detail.

### API Metadata

All Langworthy APIs are described in machine readable metadata stored in an
industry standard format. This metadata is used by both the consuming &
producing projections. Sometimes the data is consumed at runtime, and sometimes
at compile time, depending on the nature of the projection. The metadata is the
definitive description of the available APIs and how to call them. In the
Windows Runtime, metadata provides rich information about dependencies and
versioning. Langworthy would inherit these capabilities.

LDN04 described Langworthy’s metadata representation in detail.

### Abstract Binary Interface

In most object-oriented programming languages, method calls on object instances
are implemented as C function calls with an implicit “this” parameter added by
the compiler. Furthermore, many languages that support dynamic dispatch based on
concrete type (aka virtual functions) via a virtual function dispatch table
attached to an object’s type.

COM leveraged this consistency to create language interoperability by
translating language-specific concepts into C-style calls to object virtual
methods. The Windows Runtime refined this model by introducing a stricter type
system that could be bound to a wide variety of languages, as well as new
patterns for handling generic interfaces and other modern programming
constructs.

Langworthy borrows the Windows Runtime ABI and extends its applicability across
a wider range of machine architectures and operating systems.

LDN05 describes Langworthy’s abstract binary interface.

### Language Projection

A projection converts calls made in using familiar concepts in one programming
language into a calls that, on the machine’s call stack and in machine
registers, conforms to the abstract binary interface. On the other side of the
call, another projection converts those calls into a call tailored to the
language that the library was implemented in. As a simple example, this means
that you could call an API in C\# using a System.String, and the C++
implementation would receive a std::string_view, typically without ever copying
the string data.

In some compiled languages, this translation may involve a few simple type
conversions, or potentially no work at all. In interpreted languages, this can
sometimes involve more complex transformations. E.g. JavaScript has a very
different notion of numbers than most other languages. However, the ABI is
specifically tailored to minimize the cost and data loss in these conversions.

### Runtime

Much of the ABI is defined in terms of specific machine layout of call data,
including the binary formats of data types. However, a few specific operations
and types are intentionally opaque and implemented in the runtime. This enables
contextual optimizations of the behavior of those types, hides details that
might be system-dependent, and provides canonical implementations of behavior
for which there is exactly one defined behavior. Specifically, the runtime
handles ownership and primitive operations on strings, object lookup, GUID
calculations, and handful of other primitives. The runtime itself is exposed as
flat C APIs.

To deliver on this model, the ABI becomes a platform independent standard
calling convention. In a very real sense, it replaces Java Native interfaces,
pInvoke, and any other language-specific binding to C / external code. In its
most successful form, the project would deliver documentation, libraries and
tests that enable a motivated developer to create a new language binding, and
the set of bindings would grow well beyond the scope of what Microsoft builds
in-house. Of course, Microsoft will do more than simply lay out a concept. As
we’ve done with the Windows Runtime, our investments in Langworthy would include
a solid foundation of tools, libraries, and projections for others to use and
build upon.
