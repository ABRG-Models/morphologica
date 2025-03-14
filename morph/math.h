// Namespaced mathematical algorithms

#pragma once

/*
 * Maths in a morph::math namespace
 *
 * Prefer this location for maths functions over the class in morph/MathAlgo.
 */

#include <cmath>
#include <morph/mathconst.h>
#include <morph/constexpr_math.h> // constexpr math functions from Keith O'Hara

namespace morph {

    namespace math {

        // Return n!
        template <typename T, typename I>
        constexpr T factorial (const I n)
        {
            T fac = T{1};
            for (I i = I{1}; i <= n; ++i) { fac *= i; }
            return fac;
        }

        // Compute the normalization function N^l_m. If invalid l<0 is passed, return 0.
        template <typename T, typename UI, typename I, typename std::enable_if< std::is_integral<UI>{} && !std::is_signed<UI>{} && std::is_integral<I>{} && std::is_signed<I>{}, bool>::type = true >
        T Nlm (const UI l, const I m)
        {
            using mc = morph::mathconst<T>;
            I absm = m < I{0} ? -m : m;
            return std::sqrt ( mc::one_over_four_pi * (T{2} * static_cast<T>(l) + T{1}) * (morph::math::factorial<T, I>(l - absm) / morph::math::factorial<T, I>(l + absm)));
        }

#ifndef __OSX__ // There is no std::assoc_legendre on Mac, apparently
        //! Wraps std::assoc_legendre, allowing signed m (abs(m) always passed to std::assoc_legendre)
        template <typename T, typename UI, typename I, typename std::enable_if< std::is_integral<UI>{} && !std::is_signed<UI>{} && std::is_integral<I>{} && std::is_signed<I>{}, bool>::type = true >
        T Plm (const UI l, const I m, const T x)
        {
            unsigned int absm = m >= 0 ? static_cast<unsigned int>(m) : static_cast<unsigned int>(-m);
            return std::assoc_legendre (static_cast<unsigned int>(l), absm, x);
        }

        //! Compute the real spherical harmonic function with pre-computed normalization term Nlm.
        template <typename T, typename UI, typename I, typename std::enable_if< std::is_integral<UI>{} && !std::is_signed<UI>{} && std::is_integral<I>{} && std::is_signed<I>{}, bool>::type = true >
        T real_spherical_harmonic (const UI l, const I m, const T nlm, const T phi, const T theta)
        {
            using mc = morph::mathconst<T>;
            T ylm = T{0};
            if (m > I{0}) {
                ylm = mc::root_2 * nlm * std::cos (m * phi) * morph::math::Plm<T, UI, I> (l, m, std::cos (theta));
            } else if (m < I{0}) {
                ylm = mc::root_2 * nlm * std::sin (-m * phi) * morph::math::Plm<T, UI, I> (l, -m, std::cos (theta));
            } else { // m == 0
                ylm = nlm * morph::math::Plm<T, UI, I> (l, I{0}, std::cos (theta));
            }
            return ylm;
        }

        //! Compute the real spherical harmonic function without a pre-computed normalization term
        template <typename T, typename UI, typename I>
        T real_spherical_harmonic (const UI l, const I m, const T phi, const T theta)
        {
            return morph::math::real_spherical_harmonic<T, UI, I>(l, m, morph::math::Nlm<T, UI, I>(l, m), phi, theta);
        }
#endif // __OSX__
    } // math
} // morph
