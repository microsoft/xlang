---
id: XDN11
title: Error handling
author: mawign@microsoft.com
status: Draft
---

# XDN11 - Error handling

- Author: Manodasan Wignarajah (mawign@microsoft.com)
- Status: Draft

## Abstract

This document describes the error handling story for xlang and what it will support.

## Overview

As described in XDN01, xlang is a system that allows a component written in any supported language
to be callable from any other supported language. To achieve this, xlang needs to support a way to
translate errors from one supported language to another supported language.

These translated errors may end up being handled or end up as application failures which are
debugged by a developer, logged by the application, or captured in a dump. To help facilitate
this in a way that the developer has information available to them to diagnose these translated errors,
xlang needs to support a way to retrieve the error context captured by a language and store it with
the translated error for another language. Errors may also end up traveling through multiple components
written in different languages before reaching the failure point. Due to this, xlang needs to support
a way to store any additional error context from any language that the error propagates through.

## Translation of errors

xlang's goal is to allow a component to be written using natural and familiar code for the language
it is implemented in and for it to be consumed using natural and familiar code for the language which
the consumer is written in. To achieve this, xlang needs to translate errors from the natural form
of one language to a natural form of that error in another language.

But different languages have different errors which can occur and not all errors in one language
always directly map to errors from another language. In addition, maintaining a mapping of errors
between each supported language is not scalable. Lastly, there are a lot of errors which are domain
specific and may not make sense in other languages and thereby not have equivalent error codes natively
available. Given this, xlang will support a limited set of common errors which can be used by the
projection of a language to map to or from its error. If the error being mapped has an equivalent in
this set of common errors and the projection of the consumer's language provides an equivalent mapping
from that mapped error, then that error will have a natural mapping in the other language.

In the case there isn't an equivalent error to map to in xlang, then a generic error, xlang_fail,
will be available to be used. Note that in this case, the natural mapping of errors from one language
to the other will be lost in the sense that it will not be an equivalent mapping. In the case there
isn't an equivalent error to map to in a consumer's language, it is left to the projection of that
language to decide what error from its language to map to based on what makes sense for that language.
This can end up being a generic error.

## xlang errors

xlang will use an Int32, xlang_result, to represent its errors. The representation of this 32-bit
value will be based on the same model used in COM for HRESULTs to assist with sharing error codes,
but will not necessarily be defined in this design note on a bit by bit basis as is in COM.
This is because xlang will not inherit all the errors used in COM and will instead define a set of
common errors with their values defined. This will allow xlang to avoid defining all the errors
available in COM on each supported platform especially when most will not be used. This will also
allow for projections to avoid needing to maintain a large mapping of errors.

The following errors will be defined in xlang:

xlang error                | Value      | Description
-------------------------- | ---------- | ----------------------------------------------------------------------
xlang_fail                 | 0x80004005 | Generic error when no equivalent or better error mapping is available
xlang_invalid_arg          | 0x80000003 | Invalid argument in a call
xlang_illegal_state_change | 0x8000000d | Change in state that is illegal
xlang_out_of_memory        | 0x80000002 | Allocation issue resulting from not enough memory
xlang_pointer              | 0x80000005 | Null pointer related errors
xlang_not_impl             | 0x80000001 | Function / feature is not implemented
xlang_access_denied        | 0x80070005 | Requested operation / call is not allowed
xlang_no_interface         | 0x80000004 | Class or interface not found
xlang_handle               | 0x80000006 | Invalid handle
xlang_abort                | 0x80000007 | Operation / call has been canceled
xlang_file_not_found       | 0x80070002 | File was not found
xlang_path_not_found       | 0x80070003 | Directory was not found
xlang_arithmetic           | 0x80070216 | Error during arithmetic operation
xlang_io                   | 0x80131620 | IO error than does not have a better error mapping
xlang_bounds               | 0x8000000b | Invalid index used such as when indexing arrays
xlang_timeout              | 0x80131505 | Operation / call timed out
xlang_thread_state         | 0x80131520 | Operation can not be done due to the thread state
xlang_thread_interrupted   | 0x80131519 | Interrupt on thread
xlang_security             | 0x8013150A | Security related error
xlang_type_load            | 0x80131522 | Syntax / type error during runtime

Even though xlang only defines these common errors, it will not prevent the use of other errors from
the HRESULT family. But, it is likely that those errors will not get a natural mapping from
projections and end up as generic errors.

## How to indicate the result of a function

Functions declared at the xlang ABI (i.e. language projections, PAL) need a way to indicate whether
an error occurred. This will be done through the return parameter.

```
xlang_result functionName(param, ...)
```

Returning one of the error codes will indicate that the function resulted in an error.
Returning xlang_ok which has a value of 0 will indicate the function completed without errors.

## Alternative to errors

Even though xlang does provide a set of errors to assist with the error translation from one language
to another in an equivalent form, it is limited. In addition, when errors are translated from one language
to another, there is a chance that the accuracy of the error is lost due to an exact error mapping is not
available in the other language. This means the caller of an API may not always have the accurate and
necessary information to decide what to do with a failure.

Due to this, xlang encourages the use of errors only for diagnosing failures and not for controlling
code flow. For controlling code flow, xlang encourages the use of the Try pattern in functions.
The Try pattern can be used to pass back custom domain specific errors as parameters if necessary.
The parameter type and its possible values are declared in the producing component and would be translated
across languages as is. This means the consumer can rely on using it to make decisions without being
concerned of the translation accuracy.

## Diagnosing application failures

Errors coming from components written in other languages can be one of the causes of application
failures. Just like with any other failure, developers will want to diagnose where and why the
failure occurred. To assist with this, xlang will provide the ability for a projection to associate
diagnostic information with the error. The following information can be associated:

- Language specific error (i.e. error code, exception type)
- Error message
- Stack trace
- Projection identifier
- Any additional diagnostic information

Some of this information can be represented by different languages in different ways. For instance,
in some languages, stack traces can be an array of addresses while in others it can be a list of
function names. Due to this, xlang will store the information generically as strings and it will
be the responsibility of the projections to convert the representation it has to a string representation
that can be logged and is readable by a developer or tooling.

Errors may also end up traveling through several different components written in different languages
before becoming a failure. To assist with diagnosing these failures, xlang will allow a projection to
associate additional diagnostic information to the existing information as an error travels through it.
This will help a developer get a complete picture of the failure which wouldn't be available with just
the information collected from the language that initially returned the error. An example of this is
the stack trace. A language will typically only be aware of the stack within its language boundary and
thereby the diagnostic information collected at the initial error would only have a partial stack.
The following additional information can be associated with the error as it travels through projections:

- Stack trace
- Projection identifier
- Any additional diagnostic information

The diagnostic information will be stored in a COM object (error info) implementing the
IXlangErrorInfo interface. As with any COM object, it will be reference counted and will implement
IUnknown. This will allow it to be easily accessed by projections written in different languages
while still being able to ensure the cleanup of it.

## Handling language errors

Producing projections may encounter a language error when projecting an API call to a component.
The error will either be from the translation of the call to the component's language or be from
the component itself as a result of the call. Either way, upon encountering an error, the projection
should determine whether the error is new or is a propagation of a previous xlang error. Strategies
on doing this will be discussed in a later section.

If the error is new, the projection should translate the error to an equivalent xlang error or to
xlang_fail if there is no equivalent mapping. After translating the error, the projection should
call xlang's error origination API, xlang_originate_error, to store the diagnostic information
collected by the language for the error. The API will create an error info with the provided
diagnostic information and store it on a thread level storage. The created error info will remain
on the thread level storage until it is retrieved by the consuming projection.

xlang_originate_error will be defined in the PAL and will take the following parameters:
- a xlang_result with the translated xlang error for the language specific error that occurred
- a xlang_string with the language specific error that occurred
- a xlang_string with the error message
- a xlang_string with the error stack if available
- a xlang_string with an identifier for the projection
- an IUnknown pointer with any additional language specific information (can be null)

```
void xlang_originate_error(
	xlang_result error,
	xlang_string languageError,
	xlang_string message,
	xlang_string stack,
	xlang_string projectionIdentifier,
	IUnknown* languageInformation)
```

If the error is a propagation of a previous xlang error (i.e. the component called another xlang
projected component which returned an error), then the associated error info should be retrieved
along with the associated xlang error. The retrieved xlang error should be used as the error instead
of translating the language error to a new xlang error. This will allow the identity of the error to
be independent of the translations it may go through as it propagates through various languages and
for it to be defined as what the initial projection which returned the error determined it was
equivalent to. Diagnostic information collected by the language should still be added to the error info
as it can provide useful information in the case the error becomes a failure. This can be done by a call
to xlang_propagate_error which will add the provided information to the given error info and store it on
the thread level storage.

xlang_propagate_error will be defined in the PAL and will take the following parameters:
- an IXlangErrorInfo* for the error which is being propagated
- a xlang_string with the language specific error
- a xlang_string indicating the error stack if available
- a xlang_string indicating an identifier for the language
- an IUnknown pointer with any additional language specific information (can be null)

```
void xlang_propagate_error(
	IXlangErrorInfo* errorInfo,
	xlang_string languageError,
	xlang_string stack,
	xlang_string projectionIdentifier,
	IUnknown* languageInformation)
```

After calling xlang's origination / propagation API, the projection should return the error as the
result so that the consuming projection can receive the error and propagate it to the language it projects.
In some cases, the projection may want to call other functions (i.e. cleanup) before returning the error.
If any of these functions error, the projection may treat them as best effort and ignore the error and
still propagate the previous error after calling them. But, those functions may call the
error origination API upon error and override the error info stored on the thread local storage.
To avoid this, the projection should retrieve the error info from the thread local storage before doing
the best effort work. Then after doing that work, it should set the retrieved error info back on the
thread local store and then return the error. To help with this, xlang will provide the
xlang_get_error_info and xlang_set_error_info API.

```
void xlang_get_error_info(IXLangErrorInfo** errorInfo)
void xlang_set_error_info(IXLangErrorInfo* errorInfo)
```

## Providing additional language specific information

Most of the parameters for xlang_originate_error and xlang_propagate_error are self-explanatory from
the descriptions. But one parameter to call out is the IUnknown parameter used to provide additional
language specific information.

A projection can use this parameter to provide additional diagnostic information about the error.
It can do this by passing a COM object implementing the IXlangErrorValue interface. This interface
has a GetValue function which, when implemented, will allow to provide the information as a string.
This design allows any projection that the error propagates through to make this information available
without it needing to know the specifics of what additional information each projection provides.

```
interface IXlangErrorValue : IUnknown
{
	void GetValue(xlang_string * errorValue)
}
```

A projection can also use this parameter to store other information that it may want to retrieve later
or expose to tooling. An example use of this is to store a language exception or a custom error info.
Lets say an exception thrown in a component propagates through several different components. One of
these components may be written in the same language as the component which threw the exception.
In that case, a projection may want to re-throw the same exception initially thrown. It can achieve
this by storing the information necessary to re-construct the exception in an COM object and passing
it to this parameter. The object passed to this parameter will be stored along with the other error
information and will propagate with the error. It can later be retrieved when the error propagates back
to the same language and used to re-construct the exception. Similarly, a projection which allows to call
WinRT APIs may want tooling to be able to access the custom error info (IRestrictedErrorInfo)
it has for the error. It can achieve this by passing the IRestrictedErrorInfo to this parameter and
then let tooling query for it from xlang's error info upon failure.

## Projecting the error and associating error info

Upon receiving an error for a call, consuming projections should map the xlang error to the equivalent
native error for the language they project. This will allow for consumers to be written using natural
and familiar code for the language they are written in. But in some cases, there might not be an
equivalent native error to map to. In these cases, it is left to the projection to decide which native
error to map to because a projection implementer will be able to make a better decision on what a
consumer of the language they project would expect.

Consuming projections should also retrieve the error info associated with the xlang error and associate
it in some way with the native error. It should be associated in a way that allows the logging of the
native error to also log the information in the error info. For instance, in certain languages,
exceptions have a ToString function which consumers of that language call upon failure to log the
error for later analysis. This ToString function should also return the information in xlang's error info
to allow the developer to get a better picture of the failure. In addition, it should also be associated
in a way that allows a producing projection for the same language to determine if there is an error info
associated with the native error and to be able to retrieve it if there is. This will allow the
producing projection to propagate the error with the previously collected diagnostic information if
the error ends up being propagated by the component instead of being handled.

It is left to the projections to decide how to achieve this because it will be different for each language.
But this design note will suggest some approaches to consider. One approach is using inheritance in the
case of exception-based projections if the language isn't affected by object slicing. For each of the
errors from xlang that the projection maps to, a new type inheriting the native exception type for that
error can be created and be used to store the error info. This new type would just be an implementation
detail of the projection and would not be caught directly by components written in that language.
Instead components would use the native exception types which these types inherit from to catch exceptions.
Projections would use these custom types to determine whether it is a translated error and to get the
associated error info. For error code based projections, an approach would be to use a thread level
storage to store the error info. The consuming projection would store the error info there, while the
producing projection would retrieve it from there when the same error propagates to the producing side.
If the error is handled in the consumer, the consumer should have an API provided by the projection to
clear the stored error info to ensure proper cleanup of it. The consumer should also have an API
available to get/set the stored error info if the error gets passed across threads or to log it if a
failure occurs.

## Getting the information stored in the error info

As mentioned earlier, projections should expose the details stored in the xlang error info as part of
the function used in the language they project to log errors / exceptions (i.e. ToString on an exception).
This will allow consumers to get a full picture of the failure when they log an error using the natural and
familiar code they would typically use. To achieve this, xlang provides the IXlangErrorInfo interface.

```
interface IXLangErrorInfo
{
	void GetError(xlang_result* error)
	void GetMessage(xlang_string* message)
	void GetLanguageError(xlang_string* error)
	void GetStack(xlang_string* stack)
	void GetLanguageIdentifier(xlang_string* identifier)
	void GetLanguageInformation(IUnknown* languageInformation)
	void GetLanguageInformationAsString(xlang_string* languageInformation)
	void GetPropagatedError(IXlangErrorInfo** propagatedError)
}
```

Most of the functions provided in this interface are self-explanatory. But there are a few which
should be called out. There are 2 variants of the function used to get the additional language
specific information (GetLanguageInformation and GetLanguageInformationAsString).
The first function is expected to be used in scenarios where the projection itself wants to retrieve
an object they stored. The second function is expected to be used in scenarios where a projection is
exposing the details in the error info object as part of the language function used to log errors.
This function will provide the information as a string if the COM object stored implements the
IXlangErrorValue interface. If it doesn't or no COM object was stored, it would just provide an
empty string. Another function to call out is the GetPropagatedError function. This function will
allow to traverse the diagnostic information from each language which the error propagated through.
For all the propagations, the error and message functions will provide the same values. The remaining
functions will provide the respective information the language captured.

## Error free functions

Some languages have a concept of error free functions (i.e. noexcept in C++). These functions
will not be projected as error free because errors can occur while translating the call from the
other language and while translating the result back (i.e. during activation of the class for the
function). But if there was any error during the execution of the function itself, the language
itself may take actions based on what it does for errors in error free functions (i.e. fail fast for
noexcept).

Even though none of the projected functions will be error free, the PAL can consist of error free
functions. These functions can be distinguished by the return parameter being void instead of
xlang_result. 
