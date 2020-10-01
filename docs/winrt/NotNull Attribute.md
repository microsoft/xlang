# Declaring Non-Nullable Parameters

## Background

Variables of certain WinRT types – classes, interfaces, and delegates – may be null to indicate the absence of an object. In such cases, the variable should only be used, or dereferenced, when the caller has previously checked that the variable is not null.

Languages like C# and Rust can benefit greatly from knowing that output or return parameters are not null by eliding the code needed to check that the resulting value is not null. This helps by improving the correctness of the code via static analysis, reducing code size, and eliminating run time checks. In the case of Rust, it also simplifies the syntax at call sites where these checks cannot easily be overlooked as is permitted in C# and C++.

## Metadata Representation

The `NotNull` attribute will be added to the `Windows.Foundation.Metadata` namespace alongside the existing `NoException` attribute that performs a complementary function. The name `NotNull` is borrowed from .NET that previously introduced a `NotNull` attribute in C# 8 to mean the same thing. While the `NoException` attribute may only be applied to methods and properties as a whole, the `NotNull` attribute may only be applied to parameters.

```idl
namespace Windows.Foundation.Metadata
{
    [attributeusage(target_parameter)]
    [attributename("not_null")]
    attribute NotNullAttribute
    {
    }
}
```

## Impact on MIDL3

API authors may apply the `NotNull` attribute, or its shortened form, to any parameter but will most often be used with return types.

```idl
runtimeclass Class
{
    Class();

    [not_null] Object Property;
    [not_null] Object Method([not_null] out Object value);
}
```

The resulting metadata, in the form of a winmd file, will include the `NotNull` attribute. As with the `NoException` attribute, language projections that are unaware of the new attribute, or choose not to apply it in any way, will simply ignore it. And as with the `NoException` attribute, `NotNull` is additive and may be retroactively applied to existing APIs. This allows existing code to be optimized without breaking compatibility with existing callers.

## Impact on C#

The `Windows.Foundation.Metadata.NotNullAttribute` attribute will directly map to the C# `System.Diagnostics.CodeAnalysis.NotNullAttribute` class. The C#/WinRT language projection will simply include the `NotNull` attribute on any WinRT parameters that include the attribute in metadata. The C# compiler can then use it as part of its static analysis processing to optimize call sites.

https://docs.microsoft.com/en-us/dotnet/csharp/nullable-references

https://docs.microsoft.com/en-us/dotnet/api/system.diagnostics.codeanalysis.notnullattribute?view=netcore-3.1

## Impact on Rust

Rust is explicit about error propagation and about nullability. What that means for WinRT in practice (at call sites) is that a method that returns a nullable type requires something like `method().unwrap().unwrap()` - the first `unwrap` deals with unpacking the `Result<T>` that might report failure and the second `unwrap` deals with resolving the `Option<T>` that may or may not be a null return value. This double `unwrap` is gross to say the least. The `NoException` WinRT attribute previously introduced deals with the first `unwrap` and the `NotNull` deals with the second. This significantly improves the quality of life for Rust developers. It is perhaps the only complaint we’ve heard about WinRT from the Rust community.

## Impact on Other Languages

Other languages will be unaffected but may in future choose to apply an appropriate use for the NotNull attribute.
