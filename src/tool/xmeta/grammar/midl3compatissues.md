What is still up for debate
1) Conflicts with keywords: Internal, Object, Single, GUID, String
Conflicts with keywords for attributes target: Type, Method, Param, Property

The general issue here is enums can specify types as identifiers.

```

    [webhosthidden]
    [contract(Windows.Foundation.UniversalApiContract, 6)]
    enum TreeViewSelectionMode
    {
        None,
        Single,
        Multiple,
    }
    
    // From microsoft.hyperv.datastore.idl
    [contract(Microsoft.HyperV.DataStore.DataStoreContract, 1)]
    enum KeyValueStoreKeyType
    {
        Free = 1,
        Int = 3,
        UInt,
        Double,
        String,
        Array,
        Bool,
        Node,
        Last,
    };
    
    // From windows.ai.machinelearning.idl
    enum TensorKind
    {
        Undefined,
        Float,
        UInt8,
        Int8,
        UInt16,
        Int16,
        Int32,
        Int64,
        String,
        Boolean,
        Float16,
        Double,
        UInt32,
        UInt64,
        Complex64,
        Complex128,
        Object,
    };
    
    // From windows.devices.bluetooth.idl
    runtimeclass GattPresentationFormatTypes
    {
        static UInt8 Boolean{ get; };
        static UInt8 Bit2{ get; };
        static UInt8 UInt12{ get; };
        static UInt8 UInt16{ get; };
        static UInt8 UInt24{ get; };
        static UInt8 UInt32{ get; };
        static UInt8 UInt48{ get; };
        static UInt8 UInt64{ get; };
    }
```
Example uses of Single: filepickerextension.idl, depcontrols.idl


2) GUID, String as a keyword being used as an Identifier
```
    runtimeclass LockScreenSettingsTileData
    {
        Guid Guid{ get; };
    }
    
    See also tablid.idl
    namespace Windows.Internal.ComposableShell.Tabs
    {
        [contractversion(1)]
        [internal]
        apicontract OnecoreUapInternalContract{};

        [contract(Windows.Internal.ComposableShell.Tabs.OnecoreUapInternalContract, 1)]
        struct ShellTabCommandId
        {
            Guid Guid;
        };

        [contract(Windows.Internal.ComposableShell.Tabs.OnecoreUapInternalContract, 1)]
        struct ShellTabId
        {
            Guid Guid;
        };

        [contract(Windows.Internal.ComposableShell.Tabs.OnecoreUapInternalContract, 1)]
        struct ShellTabGroupId
        {
            Guid Guid;
        };
    }
    runtimeclass TargetedContentValue
    {
        String String{ get; };
    }
```

3) There are attribute declaration
```
    [attributename("templatepart")]
    [attributeusage(target_runtimeclass)]
    [contract(Windows.Foundation.UniversalApiContract, 1)]
    [webhosthidden]
    [allowmultiple]
    attribute TemplatePartAttribute
    {
        String Name;
        type Type;
    };
```

What was Fixed
1) apicontract type
```namespace Windows.Internal.UI.ApplicationSettings
{
    [contractversion(1)]
    [internal]
    apicontract InternalContract{};
}
```


2*) Cannot do negatives in expressions
```
[contract(PhoneInternal.Experiences.Sync.InternalContract, 1)]
enum AccountSyncContentFreshness
{
    KeepAll = -1,
    DefaultFreshness = 7,
};
```

3) Method declarations can have attributes


4) Parameter modifier has REF
```
void ReadBytes(ref UInt8[] value);
```

5) Classes are SEALED by default, need UNSEALED Modifier.

6) OVERRIDABLE is a method modifier, not OVERRIDE?

7) OVERRIDABLE can be a property modifier
Note: depcontrols.idl has examples for (6), (7) and (8)

8) Constructors have modifiers: protected
```
    unsealed runtimeclass RevealBrush
        : Windows.UI.Xaml.Media.XamlCompositionBrushBase
    {
        [method_name( "CreateInstance" )]protected RevealBrush();
    }
```

9) REF CONST and CONST REF, they can go both ways

10) In delegates, formal_parameter_list is optional. 