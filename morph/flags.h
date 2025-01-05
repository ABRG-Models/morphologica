// C++20 Flag enum class converted to C++17. This code came from an idea in the Vulkan
// code base via https://gist.github.com/fschoenberger/54c5342f220af510e1f78308a8994a45
// Have left CXX20 stuff so that this can be neatened when morphologica goes C++20.

#include <compare>
#include <type_traits>

namespace morph {
#ifdef _HAVE_CXX20_ // Neat C++20 concept code
    template <typename E> // You pass in your own enum class type
    requires std::is_enum_v<E>
#else // C++17 compatible enable_if version
    template <typename E, typename std::enable_if< std::is_enum<E>{}, bool>::type = true>
#endif
    class flags {
    public:
        using I = std::underlying_type_t<E>;

        // constructors
        constexpr flags() noexcept : bits(0) {}

#ifdef _HAVE_CXX20_
        explicit(false) // explicit(false) in C++20 means "not explicit"
#endif
        constexpr flags(E bit) noexcept : bits(static_cast<I>(bit)) {}

#ifdef _HAVE_CXX20_
        constexpr flags(flags<E> const& rhs) noexcept = default; // require C++20 to default a constexpr constructor
#else
        constexpr flags(flags<E> const& rhs) noexcept : bits(rhs.bits) {}
#endif
        constexpr explicit flags(I _bits) noexcept : bits(_bits) {}

        // relational operators
#ifdef _HAVE_CXX20_
        constexpr auto operator<=>(flags<E> const&) const = default;
#endif
        // logical operator
        constexpr bool operator!() const noexcept { return !this->bits; }

        // bitwise operators
        constexpr flags<E> operator&(flags<E> const& rhs) const noexcept { return flags<E>(this->bits & rhs.bits); }
        constexpr flags<E> operator|(flags<E> const& rhs) const noexcept { return flags<E>(this->bits | rhs.bits); }
        constexpr flags<E> operator^(flags<E> const& rhs) const noexcept { return flags<E>(this->bits ^ rhs.bits); }
        constexpr flags<E> operator~() const noexcept { return flags<E>(bits ^ static_cast<I>(-1)); }

        // assignment operators
#ifdef _HAVE_CXX20_
        constexpr flags<E>& operator=(flags<E> const& rhs) noexcept = default;
#else
        constexpr flags<E>& operator=(flags<E> const& rhs) noexcept { this->bits = rhs.bits; };
#endif
        constexpr flags<E>& operator|=(flags<E> const& rhs) noexcept
        {
            this->bits |= rhs.bits;
            return *this;
        }
        constexpr flags<E>& operator&=(flags<E> const& rhs) noexcept
        {
            this->bits &= rhs.bits;
            return *this;
        }
        constexpr flags<E>& operator^=(flags<E> const& rhs) noexcept
        {
            this->bits ^= rhs.bits;
            return *this;
        }

        // Set from a bit
        constexpr flags<E>& operator|=(const E& rhs) noexcept
        {
            this->bits |= (I{1} << static_cast<I>(rhs));
            return *this;
        }
        constexpr void set (const E& flag) noexcept
        {
            this->bits |= (I{1} << static_cast<I>(flag));
        }

        // Unset from a bit (in enum form)
        constexpr flags<E>& operator^=(const E& rhs) noexcept
        {
            this->bits ^= (I{1} << static_cast<I>(rhs));
            return *this;
        }
        constexpr void reset (const E& flag) noexcept
        {
            this->bits ^= (I{1} << static_cast<I>(flag));
        }

        // Test a flag
        constexpr bool test (const E& flag) noexcept
        {
            return (this->bits & (I{1} << static_cast<I>(flag))) > I{0} ? true : false;
        }

        // Get the underlying bits
        constexpr I get() const noexcept { return this->bits; }

        // cast operators
        explicit constexpr operator bool() const noexcept { return !!this->bits; }
        explicit constexpr operator I() const noexcept { return this->bits; }

    private:
        I bits;
    };
#if 0
    // bitwise operators
    template <typename E>
    requires std::is_enum_v<E>
    constexpr flags<E> operator&(E bit, flags<E> const& flags) noexcept
    {
        return flags.operator&(bit);
    }

    template <typename E>
    requires std::is_enum_v<E>
    constexpr flags<E> operator|(E bit, flags<E> const& flags) noexcept
    {
        return flags.operator|(bit);
    }

    template <typename E>
    requires std::is_enum_v<E>
    constexpr flags<E> operator^(E bit, flags<E> const& flags) noexcept
    {
        return flags.operator^(bit);
    }

    // bitwise operators on E
    template <typename E>
    requires std::is_enum_v<E>
    inline constexpr flags<E> operator&(E lhs, E rhs) noexcept
    {
        return flags<E>(lhs) & rhs;
    }

    template <typename E>
    requires std::is_enum_v<E>
    inline constexpr flags<E> operator|(E lhs, E rhs) noexcept
    {
        return flags<E>(lhs) | rhs;
    }

    template <typename E>
    requires std::is_enum_v<E>
    inline constexpr flags<E> operator^(E lhs, E rhs) noexcept
    {
        return flags<E>(lhs) ^ rhs;
    }

    template <typename E>
    requires std::is_enum_v<E>
    inline constexpr flags<E> operator~(E bit) noexcept
    {
        return ~(flags<E>(bit));
    }
#endif
} // namespace util
