
namespace Windows { namespace Foundation { namespace Internal
{
    // LogicalType - the Windows Runtime type (eg, runtime class, inteface group, etc)
    //               being provided as an argument to an _impl template, when that type
    //               cannot be represented at the ABI.
    // AbiType     - the type used for marshalling, ie "at the ABI", for the logical type.
    template <class LogicalType, class AbiType>
    struct AggregateType
    {
    };

    // Gets the ABI type.  See AggregateType for description.
    template <class T>
    struct GetAbiType
    {
        typedef T type;
    };

    template <class L, class A>
    struct GetAbiType<AggregateType<L, A> >
    {
        typedef A type;
    };

    // Gets the LogicalType.  See AggregateType for description.
    template <class T>
    struct GetLogicalType
    {
        typedef T type;
    };

    template <class L, class A>
    struct GetLogicalType<AggregateType<L, A> >
    {
        typedef L type;
    };

}}} // namespace Windows::Foundation::Internal
