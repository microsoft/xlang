---
id: XDN07
title: Platform Support for Activation Factories
author: ryansh@microsoft.com
status: Draft
---

# Title: XDN07 - Platform Support for Activation factories
* Author: Ryan Shepherd (ryansh@microsoft.com)
* Status: Draft

## Abstract

This document describes how the Platform Abstraction Layer (PAL) supports and integrates with xlang activation factories. 

## Overview

As described in the Type System Specification, a class that is activatable, composable, and/or has static methods requires an activation factory.

When consuming code needs an instance of a factory, it should not require knowledge of which component implements that factory, nor should it be responsible for dynamically loading that module and searching for exports.

For that reason, consuming code will request factories through the PAL.
The PAL will consult its mapping of class names to component libraries, load (if neccessary) the library, and then call the exported method on the library, returning the result to the caller.

This implies three pieces of functionality that work together to support activation:
* A function exported by the PAL for client code to request factories.
* A function exported by components to retrieve instances of its factories.
* A scheme for the PAL to build a mapping of class names to component libraries.

## Exporting factories from components

For the PAL to retrieve factories, xlang components must implement and export a function, **xlang_lib_get_activation_factory**, which accepts a class name in string form and the GUID of the requested interface, and returns the newly created class factory.
Components providing factories are expected to correctly handle calls to this function from multiple threads concurrently.

For components providing class factories, it is risky to have factories with state that depends on other components.
Because factories are requested and initialized in response to requests to construct a runtime class, a dependency cycle between components' factories could easily lead to their initialization methods being called reentrantly.
This problem is easily avoided, and correct multithreaded behavior is simpler, if class factories are stateless.

## Retrieving factory interfaces

The PAL provides the function **xlang_get_activation_factory**, which accepts as input the class name in string form, and the GUID of the requested interface.
Upon success, the output is the requested interface.

This function may fail if:
* The PAL can not map the class name to a component.
* The PAL can not load the mapped component.
* The PAL loads the component, but can not locate the exported **xlang_lib_get_activation_factory** function to retrieve the component's factories.
* The component itself fails to return the requested factory.

## Mapping classes to components

The PAL provides a function **xlang_map_component_factories**, that maps the class factories defined in a metadata file (passed as a string), to a component dynamic/shared library file (passed as a string).
This function will iterate over every class factory defined in the metadata and associate each one with the provided component file name.

This function will not attempt to immediately load the component library - it will be loaded on demand when an appropriate class factory is requested. It will fail if it is requested to map a class name that is already associated with a different component.

This functionality is expected to evolve as xlang's application model matures, with new methods and abstractions being added as appropriate.

## Built in class names

All class names in the "xlang" namespace or one of its nested namespaces are reserved for the xlang runtime.
Attempting to map a reserved class name to a different component is a failure.
Projects wanting to override or "mock" built in classes for testing purposes should instead intercept calls to **xlang_get_activation_factory**.

PAL implementations running on platforms that support the Windows Runtime (i.e. Windows 8 and later) will forward requests for all class names under the "Windows" namespace  directly to the OS function, **RoGetActivationFactory**.
