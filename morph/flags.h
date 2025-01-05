// C++20 Flag enum class converted to C++17. This code came from an idea in the Vulkan
// code base via https://gist.github.com/fschoenberger/54c5342f220af510e1f78308a8994a45
//
// The (more readable) C++20 equivalent to the C++17 compatible template declaration:
// template <typename E, typename std::enable_if<std::is_enum<E>{}, bool>::type = true>
// is:
// template <typename E>
// requires std::is_enum_v<E>

#pragma once

#include <compare>
#include <type_traits>

namespace morph {

    template <typename E, typename std::enable_if<std::is_enum<E>{}, bool>::type = true>
    class flags {
    public:
        using I = std::underlying_type_t<E>;

        // constructors
        constexpr flags() noexcept : bits(I{0}) {}
        constexpr flags(E flag) noexcept : bits( I{1} << static_cast<I>(flag) ) {}
        constexpr flags(flags<E> const& rhs) noexcept : bits(rhs.bits) {} // can default in C++20
        constexpr explicit flags(I _bits) noexcept : bits(_bits) {}

        // logical operator
        constexpr bool operator!() const noexcept { return !this->bits; }

        // bitwise operators
        constexpr flags<E> operator&(flags<E> const& rhs) const noexcept { return flags<E>(this->bits & rhs.bits); }
        constexpr flags<E> operator|(flags<E> const& rhs) const noexcept { return flags<E>(this->bits | rhs.bits); }
        constexpr flags<E> operator^(flags<E> const& rhs) const noexcept { return flags<E>(this->bits ^ rhs.bits); }
        constexpr flags<E> operator~() const noexcept { return flags<E>(bits ^ I{-1}); }

        // assignment operators
        constexpr flags<E>& operator=(flags<E> const& rhs) noexcept
        {
            this->bits = rhs.bits;
            return *this;
        }; // can default this function in C++20

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
        // Set from a bit (std::bitset-like function name)
        constexpr void set (const E& flag, bool value = true) noexcept
        {
            if (value == true) {
                this->bits |= (I{1} << static_cast<I>(flag));
            } else {
                this->bits &= ~(I{1} << static_cast<I>(flag));
            }
        }

        // Unset from a bit (in enum form)
        constexpr flags<E>& operator^=(const E& rhs) noexcept
        {
            this->bits ^= (I{1} << static_cast<I>(rhs));
            return *this;
        }
        // Unset from a big (std::bitset-like function name)
        constexpr void reset (const E& flag) noexcept
        {
            this->bits &= ~(I{1} << static_cast<I>(flag));
        }

        // Unset all (std::bitset-like function name)
        constexpr void reset() noexcept { this->bits = I{0}; }

        // Flip a flag (std::bitset-like function name)
        constexpr void flip (const E& flag) noexcept
        {
            this->bits ^= (I{1} << static_cast<I>(flag));
        }

        // Test a flag  (std::bitset-like function name)
        constexpr bool test (const E& flag) const noexcept
        {
            return (this->bits & (I{1} << static_cast<I>(flag))) > I{0} ? true : false;
        }

        // Get the underlying bits
        constexpr I get() const noexcept { return this->bits; }

        // Return the number of flags set true
        constexpr I count() const noexcept
        {
            I n = this->bits;
            I c = I{0};
            while (n) { n &= n--, ++c; } // Kernighan's algorithm
            return c;
        }

        // cast operators
        explicit constexpr operator bool() const noexcept { return !!this->bits; }
        explicit constexpr operator I() const noexcept { return this->bits; }

    private:
        I bits = I{0};
    };

    // bitwise operators
    template <typename E, typename std::enable_if<std::is_enum<E>{}, bool>::type = true>
    constexpr flags<E> operator&(E flag, flags<E> const& flags) noexcept
    {
        return flags.operator&(flag);
    }

    template <typename E, typename std::enable_if<std::is_enum<E>{}, bool>::type = true>
    constexpr flags<E> operator|(E flag, flags<E> const& flags) noexcept
    {
        return flags.operator|(flag);
    }

    template <typename E, typename std::enable_if<std::is_enum<E>{}, bool>::type = true>
    constexpr flags<E> operator^(E flag, flags<E> const& flags) noexcept
    {
        return flags.operator^(flag);
    }

    // bitwise operators on E
    template <typename E, typename std::enable_if<std::is_enum<E>{}, bool>::type = true>
    inline constexpr flags<E> operator&(E lhs, E rhs) noexcept
    {
        return flags<E>(lhs) & rhs;
    }

    template <typename E, typename std::enable_if<std::is_enum<E>{}, bool>::type = true>
    inline constexpr flags<E> operator|(E lhs, E rhs) noexcept
    {
        return flags<E>(lhs) | rhs;
    }

    template <typename E, typename std::enable_if<std::is_enum<E>{}, bool>::type = true>
    inline constexpr flags<E> operator^(E lhs, E rhs) noexcept
    {
        return flags<E>(lhs) ^ rhs;
    }

    template <typename E, typename std::enable_if<std::is_enum<E>{}, bool>::type = true>
    inline constexpr flags<E> operator~(E flag) noexcept
    {
        return ~(flags<E>(flag));
    }

} // namespace morph
