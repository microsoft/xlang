# Side-by-Side Versioned Components

## Status

Proposed: This document describes a proposed design that has not been implemented.

## Background

The Windows Runtime type system was largely designed with the needs of operating system components in mind. Although applications can define their own API types using the Windows Runtime type system, the feature set for doing so is somewhat basic. Versioning strategies are only vaguely defined, and no provisions are made for side-by-side support of a component. As the community has developed more sophisticated libraries over time, these deficiencies are becoming more challenging, and further support is needed.

Today, the Windows Runtime only supports binary-compatible versioning. That is, if a an application or component is developed with a dependency on version N of another component, version N+1 *can* be developed in such a way that code designed to use version N can leverage version N+1 without changes or recompilation of the client component. Alternately, a component author can ignore binary compatibility (which comes with trade-offs) and require that applications use a single, specific version of the component. 

Up to this point, this has been largely sufficient. The single largest "component" is the Windows operating system, which has processes in place to ensure binary compatibility. Other components have typically been distributed as part of an MSIX packaged application. The expectation is that the application developer can manage the dependency graph. This has largely been true up to this point. However, authors of new components are recognizing that they will be consumed in more complex dependency graphs, where the application may have dependencies on multiple versions of the component, and the component author does not want to be locked into a binary-stable model. This implies the need for some form of side by side support.

## Terminology

* Cross-talk - any ambiguity or contention caused by the presence of two highly similar pieces of code that are not intended to interact, such as two versions of the same component.

## Requirements

Side-by-side support implies, in it's simplest form, that an application can consume two versions of the same component with overlapping type names and functionality, and within reasonable bounds, operate normally.

### Goals

* Enable specific activation of a version of a component without the potential for inadvertently activating an alternate version.
* Enable developer experiences in projections that maintain a natural and familiar feel
* To the extent possible, avoid cluttering the developer experience with version information
* Ensure compatibility with existing tools and projections. Specifically, JavaScript and C++ /CX support, although not under active development, remain important aspects of the Windows Runtime ecosystem. These projections should not be prevented from consuming components designed to support side-by-side versioning.
* Ensure that actively maintained projections and tooling provide a simple, clean experience.

### Non Goals

* Automatically support side-by-side for all components. Since side-by-side support requires component authors to implement components in such a manner as to avoid cross-talk, component authors should opt-in only when components guarantee the ability to safely share process space with other instances of the same components. This includes avoiding or versioning use of global resources such as named objects, etc.

## Design

The design proposed in this specification is a refinement on the model proposed here: https://github.com/microsoft/ProjectReunion/issues/153.

In simplest terms, the design presented here is built around two key aspects:
1) To ensure compatibility with existing tooling, side-by-side components are marked as such using a Windows Runtime custom attribute (TODO: As annotated on a contract? Type? Where?).
2) The metadata that describes types intended for side-by-side use contains an extra layer of namespace that expresses the version of the component.
3) Projections are responsible for masking this versioning information if they understand the version data.
4) If a projection does not have versioning support, the version appears in the projected namespace used for the component as if it had been explicitly stated. 

### Authoring in MIDL3

The simulated namespace must follow normal identifier grammar for backward compatibility. Grammar for Windows Runtime types is defined here:
https://docs.microsoft.com/en-us/uwp/winrt-cref/winrt-type-system

MIDL 3 grammar for a versioned component will follow the form:

```
[component_version(1)]
component Widgets {
    namespace Contoso.Widgets {
        runtime class Doodad;
    }
}
```

The version is specified as a single integral value. The value is approximately equivalent to the major version number in a semver version number. That is, version 1 is not compatible with version 2, etc. For compatible versions, prefer contract versioning. Contract versions, by contrast, are like a semver minor version. Contract version 2 is compatible with contract version 1.

In this example, this is an update to Widgets which introduces non-breaking changes. This would be equivalent to semantic vesion 1.2

```
[component_version(1)]
component Widgets {

    [contract(Contoso.Widgets, 2)]
    namespace Contoso.Widgets {

        [contractversion(2.0)]
        apicontract WidgetContract{};

        [contract(Contoso.Widgets.WidgetContract, 1)]
        runtime_class Doodad;

        [contract(Contoso.Widgets.WidgetContract, 2)]
        runtime_class Thingamabob;
    }
}
```

There is one non-obvious aspect to the example above. Notice that the contract is defined within the Widgets component. This means that the metadata name of the contract is actually Widgets_1.Contoso.Widgets.WidgetContract. MIDLRT will be updated to allow you to omit the contract "namespace" when referencing a contract defined within the same component. Although the names are similar under the covers, and look the same in MIDL3 text, Widgets_1.Contosos.Widgets.WidgetContract has no version relationship with Widgets_2.Contosos.Widgets.WidgetContract. They are two distinct and unrelated contracts.

### Metadata Representation

In the metadata, the MIDL above translates into a custom attribute component_vesion, as well as a namespace path element. To an unenlightened projection the metadata will appear as if the type Doodad contained an additional path element at the root. The MIDL 3 equivalent would be:

```
namespace Widgets_1.Contoso.Widgets {
        runtime_class Doodad;
}
```

Version 2 of the same component will end up with a different implicit namespace in metadata:

```
namespace Widgets_2.Contoso.Widgets {
        runtime_class Doodad;
}
```

### ABI & Platform support

At the ABI, types will self-identify using the component prefix namespace. For example, the IInspectable::GetRuntimeClassName method for Doodad will return the string "Widgets_1.Contoso.Widgets.Doodad". This ensures that metadata resolution algorithms and metadata-based marshaling support in all versions of Windows that support it will be able to uniquely correlate runtime classes with metadata definitions, without risk of type confusion.

The component-qualified type name will also be used in calculations for pinterfaces GUID calculations. This ensures that marshaling and dynamic casting via QueryInterface will not result in type confusion.

Component authors that use explicit GUIDs for interfaces are encouraged, but not required to generate new GUIDs if the binary layout of an interface changes across major versions. 

Type registrations will include the component namespace element in the registered type name. This ensures that activation of different versions is unambiguous, and that an application can include more than one version of the component. Call to RoGetActivationFactory or RoActivateInstance should use the component-qualified name.

### The C++ Developer Experience

Developers using older versions of C++ /WinRT will see the qualified namespace name. Developers using enlightened versions of C++ /WinRT will use the components as if the component namespace does not exist. This ensures that only minimal work is required to migrate from version 1 to version 2 of a component. Migration work can focus on breaking changes across versions, rather than mechanical changes required to update references to version-specific names.

Open Design Issues: Should it be a goal to allow a single C++ /WinRT consumer to work with two versions of a component within the same compilation unit? If the answer is no, consider including "pragma detect mismatch" support to prevent accidental ODR violations.

### The C# Developer Experience

Clients using .NET Core 3.1 and earlier, as well as .NET Framework 4 will be able to use components, but will refer to types using component-qualified names.

Clients using the .NET 5 projection will not use component-qualified names. Instead, the projections will be differentiated by assembly name.

#### Consuming

Open Design Issue: How do the projection tools and build system handle uniquely named assemblies for different versions of the same component?

#### Authoring

Open Design Issue: How do the projection tools and build system handle uniquely named assemblies for different versions of the same component? Can the single source file authoring experience provide a high-quality experience for implementing components?

### ABITool & MIDLRT header creation support

and implicitly calculated interface names.

### Design Patterns

#### Cross-version interoperability

In some cases, interactions between objects in different versions of a component may be necessary. In these cases, binary-stable interfaces may still be used. To avoid duplicate definition errors in some projections or languages, these interfaces should still be defined within the component. It's critical that these interfaces retain binary-compatible (ideally identical) definitions, transitively across all types referenced from the interface. Changes to the interface could result in stack or heap corruption.

```
// original version
[component_version(2)]
component Widgets {
    namespace Contoso.Widgets {

        [uuid("94569FB3-D3BB-4D01-BF7C-B8E1D8F8B30D")]
        interface IDoodadInterop;
    }
}

// updated version
[component_version(1)]
component Widgets {
    namespace Contoso.Widgets {

        [uuid("94569FB3-D3BB-4D01-BF7C-B8E1D8F8B30D")]
        interface IDoodadInterop;
    }
}

```
