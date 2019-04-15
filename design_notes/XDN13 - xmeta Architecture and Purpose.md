---
id: XDN13
title: xmeta Architecture and Purpose
author: t-tiroma@microsoft.com
status: draft
---

# Title: XDN13 - xmeta Architecture and Purpose

- Author: Tim Romanski (t-tiroma@microsoft.com)
- Status: Draft

## Abstract

This document will outline the purpose of xmeta and the design and architecture decisions made for the tool.

## Purpose

The xmeta tool will be a cross platform version of MIDLRT. As a brief sidenote, MIDLRT was a fork of MIDL with a very verbose syntax. A new syntax called MIDL3 was created, and it was added to MIDLRT rather than having its own tool built from scratch. Unfortunately, there is now a lot of hardcoded WinRT knowledge in MIDLRT that would be difficult to adapt for xlang.

Functionally, xmeta will take in a MIDL3 [IDL](https://docs.microsoft.com/en-us/windows/desktop/com/idl-files) file and previously compiled `.winmd` files, and will compile them into new `.winmd` metadata files.

## Architecture

For parsing the IDL file, we use the [ANTLR](https://www.antlr.org/) parser generator. This allows us to define the IDL grammar which ANTLR uses to parse an IDL file and generate an abstract syntax tree (AST). We transfer this AST into a custom symbol table. To represent the AST in a symbol table we have C++ classes for namespaces, types that consist of classes, structs, interfaces, enums, and delegates, and members which consist of methods, properties, events, and fields.

Before the AST is created we extract the `.winmd` input files into the symbol tables. Then the IDL file is parsed and the AST is generated. This is then extracted into the same symbol tables the `.winmd` input populated. 

Once the symbol tables are fully populated we perform two passes through them for semantic checking.

If there are no errors we proceed to generating the output `.winmd` metadata file.

(currently uncertain which API will be used for metadata reading/writing).