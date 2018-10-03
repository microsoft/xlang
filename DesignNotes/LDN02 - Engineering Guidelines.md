---
title: A Strategy for Language Interoperability
author: "benkuhn@microsoft.com"
---

Title: ​LDN02 - Engineering Guidelines
=======================================================

-   Author: Ben Kuhn (benkuhn\@microsoft.com)

-   Status: Draft

Abstract
--------

The Xlang Project involves many components written in a variety of
programming languages. This document describes coding guidelines for
each language.


Coding Guidelines
-----------------

This project adopts industry standard guidelines where one exists. As new language support is added to the project, this list will be expanded to include applicable standards. 

The project references these as guidelines, not laws. Thoughtful deviation is allowed if it aids readability of the specific code in question or is necessary for a technical reason. However, guidelines should not be ignored because of a disagreement with the guideline itself. 

Due to the nature of language interoperability, types expressed using the xlang type system will adhere to naming guidelines in some languages and deviate from others. Naming conventions of the xlang type system take precedence over language-specific guidelines where applicable.

### References by language

* C++ code follows the [C++ core guidelines](https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md)

* C# code follows the [C# coding conventions](https://docs.microsoft.com/en-us/dotnet/csharp/programming-guide/inside-a-program/coding-conventions)

* .NET types adhere to the [.NET Framework Design Guidelines](https://docs.microsoft.com/en-us/dotnet/csharp/programming-guide/inside-a-program/coding-conventions).

* Python code is [pythonic](https://docs.python-guide.org/writing/style/).

