/*!
 * A Boolean flags class for morphologica
 *
 * You can use morph::flags<E> rather like std::bitset<10>. The difference is that E may
 * be (and must be) an enum class, which does not work with a bitset without ugly static
 * casting.
 *
 * Usage:
 *
 * First create your enum class
 *
 * enum class myflags : uint64_t { // up to 64 flags are possible with this underlying type
 *     flag_one, // Name the class and flags in some way that is right for your application
 *     flag_two  // It is not necessary to be explicit about the values of each flag
 * };
 *
 * (It is up to you to ensure that your enum class does not contain more flags than there are bits
 * in the underlying type!)
 *
 * morph::flags<myflags> fl;
 *
 * Set a flag:
 *
 * fl.set (myflags::flag_one);
 *
 * Test a flag:
 *
 * bool flag_one_is_set = fl.test (myflags::flag_one);
 *
 * Clear/reset a flag:
 *
 * fl.reset (myflags::flag_one);
 *
 * Flip a flag:
 *
 * fl.flip (myflags::flag_one);
 *
 * This code was adapted from an idea in the Vulkan code base via
 * https://gist.github.com/fschoenberger/54c5342f220af510e1f78308a8994a45
 *
 * Author: Seb James
 * Date: Jan 2025
 */

#pragma once

#include <compare>
#include <type_traits>

namespace morph {

    template <typename E> requires std::is_enum_v<E>
    struct flags
    {
        using I = std::underlying_type_t<E>;

        // constructors
        constexpr flags() noexcept : bits(I{0}) {}
        constexpr flags(E flag) noexcept : bits( I{1} << static_cast<I>(flag) ) {}
        constexpr flags(flags<E> const& rhs) noexcept = default;
        constexpr explicit flags(I _bits) noexcept : bits(_bits) {}

        // logical operator
        constexpr bool operator!() const noexcept { return !this->bits; }

        // bitwise operators
        constexpr flags<E> operator&(flags<E> const& rhs) const noexcept { return flags<E>(this->bits & rhs.bits); }
        constexpr flags<E> operator|(flags<E> const& rhs) const noexcept { return flags<E>(this->bits | rhs.bits); }
        constexpr flags<E> operator^(flags<E> const& rhs) const noexcept { return flags<E>(this->bits ^ rhs.bits); }
        constexpr flags<E> operator~() const noexcept { return flags<E>(bits ^ I{-1}); }

        // assignment operators
        constexpr flags<E>& operator=(flags<E> const& rhs) noexcept = default;

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
        //
        // NB: If static_cast<I>(flag) is greater than the number of bits in the type I, then no
        // flag will be set. That is, it is up to the programmer to ensure that the enum E does not
        // have more flags than bits in its underlying type I.
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

        // Return true if any flags are set
        constexpr bool any() const noexcept { return this->count() > I{0}; }

        // Return true if no flags are set
        constexpr bool none() const noexcept { return this->count() == I{0}; }

        // Note: there is no flags::all() to match std::bitset::all() because it is not currently
        // possible in C++ to know how many enumerated values there are in the enum class E.

        // cast operators
        explicit constexpr operator bool() const noexcept { return !!this->bits; }
        explicit constexpr operator I() const noexcept { return this->bits; }

    private:
        I bits = I{0};
    };

    // bitwise operators
    template <typename E> requires std::is_enum_v<E>
    constexpr flags<E> operator&(E flag, flags<E> const& flags) noexcept
    {
        return flags.operator&(flag);
    }

    template <typename E> requires std::is_enum_v<E>
    constexpr flags<E> operator|(E flag, flags<E> const& flags) noexcept
    {
        return flags.operator|(flag);
    }

    template <typename E> requires std::is_enum_v<E>
    constexpr flags<E> operator^(E flag, flags<E> const& flags) noexcept
    {
        return flags.operator^(flag);
    }

    // bitwise operators on E
    template <typename E> requires std::is_enum_v<E>
    inline constexpr flags<E> operator&(E lhs, E rhs) noexcept
    {
        return flags<E>(lhs) & rhs;
    }

    template <typename E> requires std::is_enum_v<E>
    inline constexpr flags<E> operator|(E lhs, E rhs) noexcept
    {
        return flags<E>(lhs) | rhs;
    }

    template <typename E> requires std::is_enum_v<E>
    inline constexpr flags<E> operator^(E lhs, E rhs) noexcept
    {
        return flags<E>(lhs) ^ rhs;
    }

    template <typename E> requires std::is_enum_v<E>
    inline constexpr flags<E> operator~(E flag) noexcept
    {
        return ~(flags<E>(flag));
    }

} // namespace morph
