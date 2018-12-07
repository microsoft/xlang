/*******************************************************************************
 **                                                                           **
 ** windows.foundation.collections.h - Common collection-related types.       **
 **                                                                           **
 ** Copyright (c) Microsoft Corporation. All rights reserved.                 **
 **                                                                           **
 **      Contains definitions of C++ class templates used by                  **
 **      MIDL for generating parameter interface instances.                   **
 **                                                                           **
 **  Known Limitations                                                        **
 **      Midl does not import other IDL files that the IObservable*           **
 **      interfaces depend on.  If you instantiate either IObservableMap or   **
 **      IObservableVector in your IDL file, you will need to explicitly      **
 **      import "eventtoken.idl" at the top of your IDL file.                 **
 **                                                                           **
 **  Notes on design:                                                         **
 **      The bodies of each of these interface templates are each             **
 **      defined once, with the postfix "_impl", defining each of             **
 **      the members. MIDL will then hook up the "_impl" with                 **
 **      the method definitions to the real template, and assign it           **
 **      a guid.                                                              **
 **                                                                           **
 **      For instance, it might generate:                                     **
 **                                                                           **
 **          template <>                                                      **
 **          struct                                                           **
 **          __declspec(uuid("77e5b300-6a88-4f63-b490-e25b414bed4e"))         **
 **          IVectorView<int> : IVectorView_impl<int>                         **
 **          {                                                                **
 **          };                                                               **
 **                                                                           **
 **      In this fashion, IVectorView<int> is assigned an IID.                **
 **         As well, for each parameterized type, the original template is    **
 **      not just forward declared, but defined and derives from the "_impl"  **
 **      form. This is done as it was found to generate usable autocomplete   **
 **      in IDEs.                                                             **
 **         The original unspecialized template also is made to static_assert **
 **      if the user attempts to use it.  This is to avoid ODR violations,    **
 **      and the scenario that the user forgot to mention a 'declare'         **
 **      clause in the MIDL input.                                            **
 *******************************************************************************/

#ifndef WINDOWS_FOUNDATION_COLLECTIONS_H
#define WINDOWS_FOUNDATION_COLLECTIONS_H

#ifdef _MSC_VER
#pragma once
#endif  /* _MSC_VER */

#include <inspectable.h>
#include <rpcndr.h> // used for boolean
#include <eventtoken.h>  // used for EventRegistrationToken
#include <winstring.h> // needed for WindowsDeleteString

/* Use of templates and namespaces */
#ifdef __cplusplus

#include <asyncinfo.h>

// metafunctions for handling Interface Groups and Runtime Types in parameterized interfaces
