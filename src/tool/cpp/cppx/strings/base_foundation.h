
WINRT_EXPORT namespace xlang::System
{
    struct Point
    {
        float X;
        float Y;

        Point() noexcept = default;

        constexpr Point(float X, float Y) noexcept
            : X(X), Y(Y)
        {}

#ifdef WINRT_NUMERICS

        constexpr Point(Numerics::float2 const& value) noexcept
            : X(value.x), Y(value.y)
        {}

        operator Numerics::float2() const noexcept
        {
            return { X, Y };
        }

#endif
    };

    constexpr bool operator==(Point const& left, Point const& right) noexcept
    {
        return left.X == right.X && left.Y == right.Y;
    }

    constexpr bool operator!=(Point const& left, Point const& right) noexcept
    {
        return !(left == right);
    }

    struct Size
    {
        float Width;
        float Height;

        Size() noexcept = default;

        constexpr Size(float Width, float Height) noexcept
            : Width(Width), Height(Height)
        {}

#ifdef WINRT_NUMERICS

        constexpr Size(Numerics::float2 const& value) noexcept
            : Width(value.x), Height(value.y)
        {}

        operator Numerics::float2() const noexcept
        {
            return { Width, Height };
        }

#endif
    };

    constexpr bool operator==(Size const& left, Size const& right) noexcept
    {
        return left.Width == right.Width && left.Height == right.Height;
    }

    constexpr bool operator!=(Size const& left, Size const& right) noexcept
    {
        return !(left == right);
    }

    struct Rect
    {
        float X;
        float Y;
        float Width;
        float Height;

        Rect() noexcept = default;

        constexpr Rect(float X, float Y, float Width, float Height) noexcept :
            X(X), Y(Y), Width(Width), Height(Height)
        {}

        constexpr Rect(Point const& point, Size const& size)  noexcept :
            X(point.X), Y(point.Y), Width(size.Width), Height(size.Height)
        {}
    };

    constexpr bool operator==(Rect const& left, Rect const& right) noexcept
    {
        return left.X == right.X && left.Y == right.Y && left.Width == right.Width && left.Height == right.Height;
    }

    constexpr bool operator!=(Rect const& left, Rect const& right) noexcept
    {
        return !(left == right);
    }
}

namespace xlang::impl
{
    template <> struct name<System::Point>
    {
        static constexpr auto & value{ L"Windows.Foundation.Point" };
    };

    template <> struct category<System::Point>
    {
        using type = struct_category<float, float>;
    };

    template <> struct name<System::Size>
    {
        static constexpr auto & value{ L"Windows.Foundation.Size" };
    };

    template <> struct category<System::Size>
    {
        using type = struct_category<float, float>;
    };
    
    template <> struct name<System::Rect>
    {
        static constexpr auto & value{ L"Windows.Foundation.Rect" };
    };

    template <> struct category<System::Rect>
    {
        using type = struct_category<float, float, float, float>;
    };

#ifdef WINRT_NUMERICS

    template <> struct name<System::Numerics::float2>
    {
        static constexpr auto & value{ L"Windows.Foundation.Numerics.Vector2" };
    };

    template <> struct category<System::Numerics::float2>
    {
        using type = struct_category<float, float>;
    };

    template <> struct name<System::Numerics::float3>
    {
        static constexpr auto & value{ L"Windows.Foundation.Numerics.Vector3" };
    };

    template <> struct category<System::Numerics::float3>
    {
        using type = struct_category<float, float, float>;
    };

    template <> struct name<System::Numerics::float4>
    {
        static constexpr auto & value{ L"Windows.Foundation.Numerics.Vector4" };
    };

    template <> struct category<System::Numerics::float4>
    {
        using type = struct_category<float, float, float, float>;
    };

    template <> struct name<System::Numerics::float3x2>
    {
        static constexpr auto & value{ L"Windows.Foundation.Numerics.Matrix3x2" };
    };

    template <> struct category<System::Numerics::float3x2>
    {
        using type = struct_category<float, float, float, float, float, float>;
    };

    template <> struct name<System::Numerics::float4x4>
    {
        static constexpr auto & value{ L"Windows.Foundation.Numerics.Matrix4x4" };
    };

    template <> struct category<System::Numerics::float4x4>
    {
        using type = struct_category<
            float, float, float, float,
            float, float, float, float,
            float, float, float, float,
            float, float, float, float
        >;
    };

    template <> struct name<System::Numerics::quaternion>
    {
        static constexpr auto & value{ L"Windows.Foundation.Numerics.Quaternion" };
    };

    template <> struct category<System::Numerics::quaternion>
    {
        using type = struct_category<float, float, float, float>;
    };

    template <> struct name<System::Numerics::plane>
    {
        static constexpr auto & value{ L"Windows.Foundation.Numerics.Plane" };
    };

    template <> struct category<System::Numerics::plane>
    {
        using type = struct_category<System::Numerics::float3, float>;
    };

#endif
}
