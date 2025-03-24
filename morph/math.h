// Namespaced mathematical algorithms

#pragma once

/*
 * Maths in a morph::math namespace
 *
 * Prefer this location for maths functions over the class in morph/MathAlgo.
 */

#include <type_traits>
#include <cmath>
#include <limits>
#include <morph/mathconst.h>
#include <morph/constexpr_math.h> // constexpr math functions from Keith O'Hara
#include <morph/range.h>
#include <iostream> // debug

namespace morph::math {

    // Find the significant base10 columns in a floating point number of type F Return a
    // range whose max is the order of magnitude of the largest base10 column and whose
    // min is the order of magnitude of the smallest significant base10 column.
    template <typename F> requires std::is_floating_point_v<F>
    constexpr morph::range<int> significant_cols (const F& f)
    {
        morph::range<int> sigcols = { 0, 0 };

        // If NaN or infinity, return 0, 0. Or something else?
        if (morph::math::isnan(f) || morph::math::isinf(f)) { return sigcols; }

        // How many sig figures does the type (float, double...) support?
        constexpr F epsilon = std::numeric_limits<F>::epsilon();
        constexpr int type_sf = static_cast<int>(morph::math::floor (morph::math::log10 (F{1} / epsilon)));

        // copy absolute value of f
        F fcpy = f > F{0} ? f : -f;

        // Find the biggest column
        sigcols.max = static_cast<int>(morph::math::floor (morph::math::log10 (fcpy)));

        // Initialize smallest column
        sigcols.min = sigcols.max;

        // We will need to use 10^sigcols.max
        F tentothe = morph::math::pow (F{10}, sigcols.max);

        // Loop down sigcols.min until we hit the limit for type F
        for (; sigcols.min > (sigcols.max - type_sf); --sigcols.min) {

            // What's the value in this column?
            F colval = morph::math::floor(fcpy / tentothe);

            // Is colval very small? Break if so.
            if (std::abs(colval) < epsilon) {
                ++sigcols.min;
                break;
            }

            // Is col val very close to 10? Break if so. This should relate to epsilon and current position
            if (colval > F{8}) {
                // additional cols depends on max precision of type and current col.
                int cols_remaining = type_sf - (sigcols.max - sigcols.min); // How many significant column have we already used?
                if (cols_remaining > 0) {
                    F additional_cols = morph::math::pow (F{10}, cols_remaining);
                    if (morph::math::abs((fcpy - colval * tentothe) - tentothe) < (tentothe / additional_cols)) {
                        ++sigcols.min;
                        break;
                    }
                } else { // no cols left
                    ++sigcols.min;
                    break;
                }
            }

            // Update fcpy by subtracting the column value, then shifting-right with multiply by 10
            fcpy = (fcpy - colval * tentothe) * F{10};
        }

        return sigcols;
    }

    // Return the amount of precision (significant figures in base 10) required for the
    // floating point number f
    template <typename F> requires std::is_floating_point_v<F>
    constexpr int significant_figs (const F& f)
    {
        morph::range<int> sc = morph::math::significant_cols<F> (f);
        return sc.span();
    }

    static constexpr bool rc_debug = true;

    // Round the number f to the base10 col mincol. In f, our convention is that column
    // 1 is 10s, 0 is 1s, -1 is tenths, -2 hundreds, etc. So round_to_col (1.2345f, -2)
    // should return 1.23f.
    template <typename F> requires std::is_floating_point_v<F>
    constexpr F round_to_col (const F& f, const int mincol)
    {
        if (morph::math::isnan(f) || morph::math::isinf(f)) { return f; }
        // How many sig figures does the type (float, double...) support?
        constexpr F epsilon = std::numeric_limits<F>::epsilon();
        constexpr int type_sf = static_cast<int>(morph::math::floor (morph::math::log10 (F{1} / epsilon)));
        if constexpr (rc_debug) { std::cout << "rtc: round_to_col(" << f << ", " << mincol << ") called\n"; }
        // Save sign and a positive version of the input
        int sign = f > F{0} ? 1 : -1;
        F fcpy = f > F{0} ? f : -f;
        // Find the biggest column
        int maxcol = static_cast<int>(morph::math::floor (morph::math::log10 (fcpy)));
        // We will need to use 10^maxcol
        F tentothe = morph::math::pow (F{10}, maxcol);
        // If user passed a mincol that's too big, return number rounded to maxcol
        if (mincol > maxcol) { return (sign > 0 ? tentothe : -tentothe); }
        F rounded = F{0};
        // Loop down curcol until we hit the limit for type F or mincol
        for (int curcol = maxcol; curcol >= mincol && curcol > (maxcol - type_sf); --curcol) {
            // What's the value in this column?
            F colval = morph::math::floor(fcpy / tentothe);
            if constexpr (rc_debug) { std::cout << "rtc: curcol = " << curcol << ". colval = " << colval << std::endl; }
            // Add to rounded
            rounded += morph::math::pow (F{10}, curcol) * colval;
            if constexpr (rc_debug) { std::cout << "rtc: rounded now = " << rounded << std::endl; }

            if (curcol == mincol) {
                if constexpr (rc_debug) { std::cout << "Got curcol == mincol\n"; }
                // Last col. do we round?
                F final_diff = std::abs(f) - rounded;
                if constexpr (rc_debug) { std::cout << "rtc: Final_diff: " << final_diff << std::endl; }
                if (final_diff > F{0}) {
                    int fdcol = static_cast<int>(morph::math::floor (morph::math::log10 (final_diff)));
                    if constexpr (rc_debug) { std::cout << "rtc: Final col: " << fdcol << std::endl; }
                    F final_diff_raised = final_diff * morph::math::pow (F{10}, -fdcol);
                    if constexpr (rc_debug) { std::cout << "rtc: Final diff raised: " << final_diff_raised << std::endl; }
                    if (final_diff_raised > F{8}) { // HERE. Is it 8 or 0.8?
                        if constexpr (rc_debug) {
                            std::cout << "rtc: Add " << morph::math::pow (F{10}, curcol) << " to rounded\n";
                        }
                        rounded += morph::math::pow (F{10}, curcol);
                    }
                }
            }

            // Update fcpy by subtracting the column value, then shifting-right with multiply by 10
            fcpy = (fcpy - colval * tentothe) * F{10};
        }
        if constexpr (rc_debug) { std::cout << "rtc: return " << (sign > 0 ? rounded : -rounded) << std::endl; }
        return (sign > 0 ? rounded : -rounded);
    }

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

#ifndef __APPLE__ // There is no std::assoc_legendre on Mac, apparently
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
#endif // __APPLE__

} // morph::math
