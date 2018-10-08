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

Components implementing these runtime classes will provide these factories via an exported method.
The PAL will provide functionality to map class names to components (to route factory requests to the correct component), and to request instances of factories from those components.

## Exporting factories from components

For the PAL to retrieve factories, xlang components must implement and export a method, **xlang_lib_get_activation_factory**, which accepts a class name in string form, and returns the newly created class factory.
Components providing factories are expected to correctly handle calls to this method from multiple threads concurrently.

For components providing class factories, it is risky to have factories with state that depends on other components.
Because factories are requested and initialized in response to requests to construct a runtime class, a dependency cycle between components' factories could easily lead to their initialization methods being callsed reentrantly.
This problem is easily avoided, and correct multithreaded behavior is simpler, if class factories are stateless.

## Retrieving factory interfaces

The PAL provides the function **xlang_get_activation_factory**, which accepts as input the class name in string form, and the GUID of the requested interface.
Upon success, the output is the requested interface.

This function may fail if:
* The PAL can not map the class name to a component.
* The PAL can not load the mapped component.
* The PAL loads the component, but can not locate the exported **xlang_lib_get_activation_factory** method to retrieve the component's factories.
* The component itself fails to return the requested factory.

## Mapping classes to components

The PAL provides a method **xlang_load_component_metadata**, that maps the class factories defined in a metadata file (passed as a string), to a component dynamic/shared library file (passed as a string).
This method will iterate over every class factory defined in the metadata and associate each one with the provided component file name.

This function will not attempt to immediately load the component library - it will be loaded on demand when an appropriate class factory is requested. It will fail if it is requested to map a class name that is already associated with a different component.

This functionality is expected to evolve as xlang's application model matures, with new methods and abstractions being added as appropriate.

## Built in class names

All class names in the "xlang" namespace or one of its nested namespaces are reserved for the xlang runtime.
Attempting to map a reserved class name to a different component is a failure.
Projects wanting to override or "mock" built in classes for testing purposes should instead intercept calls to **xlang_get_activation_factory**.

PAL implementations running on platforms that support the Windows Runtime (i.e. Windows 8 and later) will forward requests for all class names under the "Windows" namespace  directly to the OS function, **RoGetActivationFactory**.
