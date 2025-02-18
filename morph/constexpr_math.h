/*################################################################################
  ##
  ##   Copyright (C) 2016-2024 Keith O'Hara
  ##
  ##   This file is part of the GCE-Math C++ library.
  ##
  ##   But! Seb has cherry picked the functions he wanted in morphologica and
  ##   ruined/modified Keith's code to suit his needs.
  ##
  ##   If you need to understand it, then refer back to Keith's original, I
  ##   stripped many of the comments out of his code for brevity.
  ##
  ##   Many thanks to Keith for writing the code!
  ##   (copied from GCEM at commit 012ae73c6d0a2cb09ffe86475f5c6fba3926e200)
  ##
  ##   Licensed under the Apache License, Version 2.0 (the "License");
  ##   you may not use this file except in compliance with the License.
  ##   You may obtain a copy of the License at
  ##
  ##       http://www.apache.org/licenses/LICENSE-2.0
  ##
  ##   Unless required by applicable law or agreed to in writing, software
  ##   distributed under the License is distributed on an "AS IS" BASIS,
  ##   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  ##   See the License for the specific language governing permissions and
  ##   limitations under the License.
  ##
  ################################################################################*/

#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <morph/mathconst.h>

namespace morph
{
    namespace math
    {
        template<typename T>
        using return_t = typename std::conditional<std::is_integral<T>::value, double, T>::type;

        template<typename ...T>
        using common_t = typename std::common_type<T...>::type;

        template<typename ...T>
        using common_return_t = return_t<common_t<T...>>;

        static constexpr int log_max_iter_small = 25;
        static constexpr int exp_max_iter_small = 25;
        static constexpr int sqrt_max_iter = 100;

        template<class T, std::enable_if_t<std::is_arithmetic_v<T>>...>
        constexpr auto abs (const T& x) noexcept { return x < T{0} ? -x : x; }

        namespace internal // is_odd/even
        {
            constexpr bool is_odd (const int64_t x) noexcept { return (x & 1U) != 0; }
            constexpr bool is_even (const int64_t x) noexcept { return !is_odd (x); }
        }

        namespace internal // is_nan
        {
            template<typename T>
            constexpr bool is_nan (const T x) noexcept { return x != x; }
            template<typename T1, typename T2>
            constexpr bool any_nan (const T1 x, const T2 y) noexcept {  return (is_nan(x) || is_nan(y)); }
            template<typename T1, typename T2>
            constexpr bool all_nan (const T1 x, const T2 y) noexcept { return (is_nan(x) && is_nan(y)); }
            template<typename T1, typename T2, typename T3>
            constexpr bool any_nan (const T1 x, const T2 y, const T3 z) noexcept { return (is_nan(x) || is_nan(y) || is_nan(z)); }
            template<typename T1, typename T2, typename T3>
            constexpr bool all_nan (const T1 x, const T2 y, const T3 z) noexcept { return (is_nan(x) && is_nan(y) && is_nan(z)); }
        }

        namespace internal // is_inf
        {
            template<typename T>
            constexpr bool is_neginf (const T x) noexcept { return x == -std::numeric_limits<T>::infinity(); }
            template<typename T1, typename T2>
            constexpr bool any_neginf (const T1 x, const T2 y) noexcept { return (is_neginf(x) || is_neginf(y)); }
            template<typename T1, typename T2>
            constexpr bool all_neginf (const T1 x, const T2 y) noexcept { return (is_neginf(x) && is_neginf(y)); }
            template<typename T1, typename T2, typename T3>
            constexpr bool any_neginf (const T1 x, const T2 y, const T3 z) noexcept { return (is_neginf(x) || is_neginf(y) || is_neginf(z)); }
            template<typename T1, typename T2, typename T3>
            constexpr bool all_neginf (const T1 x, const T2 y, const T3 z) noexcept { return (is_neginf(x) && is_neginf(y) && is_neginf(z)); }

            template<typename T>
            constexpr bool is_posinf (const T x) noexcept { return x == std::numeric_limits<T>::infinity(); }
            template<typename T1, typename T2>
            constexpr bool any_posinf (const T1 x, const T2 y) noexcept { return (is_posinf(x) || is_posinf(y)); }
            template<typename T1, typename T2>
            constexpr bool all_posinf (const T1 x, const T2 y) noexcept { return (is_posinf(x) && is_posinf(y)); }
            template<typename T1, typename T2, typename T3>
            constexpr bool any_posinf (const T1 x, const T2 y, const T3 z) noexcept { return (is_posinf(x) || is_posinf(y) || is_posinf(z)); }
            template<typename T1, typename T2, typename T3>
            constexpr bool all_posinf (const T1 x, const T2 y, const T3 z) noexcept { return (is_posinf(x) && is_posinf(y) && is_posinf(z)); }

            template<typename T>
            constexpr bool is_inf (const T x) noexcept { return (is_neginf(x) || is_posinf(x)); }
            template<typename T1, typename T2>
            constexpr bool any_inf (const T1 x, const T2 y) noexcept { return (is_inf(x) || is_inf(y)); }
            template<typename T1, typename T2>
            constexpr bool all_inf (const T1 x, const T2 y) noexcept { return (is_inf(x) && is_inf(y)); }
            template<typename T1, typename T2, typename T3>
            constexpr bool any_inf (const T1 x, const T2 y, const T3 z) noexcept { return (is_inf(x) || is_inf(y) || is_inf(z)); }
            template<typename T1, typename T2, typename T3>
            constexpr bool all_inf (const T1 x, const T2 y, const T3 z) noexcept { return (is_inf(x) && is_inf(y) && is_inf(z)); }
        }

        namespace internal // is_finite
        {
            template<typename T>
            constexpr bool is_finite (const T x) noexcept { return (!is_nan(x)) && (!is_inf(x)); }
            template<typename T1, typename T2>
            constexpr bool any_finite (const T1 x, const T2 y) noexcept { return (is_finite(x) || is_finite(y)); }
            template<typename T1, typename T2>
            constexpr bool all_finite (const T1 x, const T2 y) noexcept { return (is_finite(x) && is_finite(y)); }
            template<typename T1, typename T2, typename T3>
            constexpr bool any_finite (const T1 x, const T2 y, const T3 z) noexcept
            { return (is_finite(x) || is_finite(y) || is_finite(z)); }
            template<typename T1, typename T2, typename T3>
            constexpr bool all_finite (const T1 x, const T2 y, const T3 z) noexcept
            { return (is_finite(x) && is_finite(y) && is_finite(z)); }
        }

        // signum
        template<typename T> constexpr int sgn (const T x) noexcept { return (x > T{0} ? 1 : (x < T{0} ? -1 : 0)); }

        // signbit
        template <typename T>
        constexpr bool signbit (const T x) noexcept {
#ifdef _MSC_VER
            return _signbit(x); // or: ((x == T{0}) ? (_fpclass(x) == _FPCLASS_NZ) : (x < T{0}));
#else
            return __builtin_signbit(x);
#endif
        }

        // copysign
        template <typename T1, typename T2>
        constexpr T1 copysign (const T1 x, const T2 y) noexcept { return (signbit(x) != signbit(y) ? -x : x ); }

        // neg_zero
        template<typename T> constexpr bool neg_zero (const T x) noexcept { return ((x == T{0}) && (copysign(T{1}, x) == T{-1})); }

        namespace internal // mantissa
        {
            template<typename T>
            constexpr T mantissa(const T x) noexcept { return (x < T{1} ? mantissa (x * T{10}) : x > T{10} ? mantissa (x / T{10}) : x ); }
        }

        namespace internal // find_exponent
        {
            template<typename T>
            constexpr int64_t find_exponent (const T x, const int64_t exponent) noexcept
            {
                return (x < T{1e-03}  ? find_exponent(x * T{1e+04}, exponent - int64_t{4}) :
                        x < T{1e-01}  ? find_exponent(x * T{1e+02}, exponent - int64_t{2}) :
                        x < T{1} ? find_exponent(x * T{10}, exponent - int64_t{1}) :
                        x > T{10} ? find_exponent(x / T{10}, exponent + int64_t{1}) :
                        x > T{1e+02} ? find_exponent(x / T{1e+02}, exponent + int64_t{2}) :
                        x > T{1e+04} ? find_exponent(x / T{1e+04}, exponent + int64_t{4}) : exponent);
            }
        }

        namespace internal // log
        {
            template<typename T>
            constexpr T log_cf_main (const T xx, const int depth_end) noexcept
            {
                int depth = log_max_iter_small - 1;
                T res = T(2 * (depth + 1) - 1);
                while (depth > depth_end - 1) {
                    res = T(2 * depth - 1) - T(depth * depth) * xx / res;
                    --depth;
                }
                return res;
            }
            template<typename T>
            constexpr T log_cf_begin (const T x) noexcept { return (T{2} * x / log_cf_main (x * x, 1)); }
            template<typename T>
            constexpr T log_main (const T x) noexcept { return (log_cf_begin ((x - T{1}) / (x + T{1}))); }

            constexpr long double log_mantissa_integer (const int x) noexcept
            {
                return (x == 2  ? 0.6931471805599453094172321214581765680755L :
                        x == 3  ? 1.0986122886681096913952452369225257046475L :
                        x == 4  ? 1.3862943611198906188344642429163531361510L :
                        x == 5  ? 1.6094379124341003746007593332261876395256L :
                        x == 6  ? 1.7917594692280550008124773583807022727230L :
                        x == 7  ? 1.9459101490553133051053527434431797296371L :
                        x == 8  ? 2.0794415416798359282516963643745297042265L :
                        x == 9  ? 2.1972245773362193827904904738450514092950L :
                        x == 10 ? 2.3025850929940456840179914546843642076011L : 0.0L );
            }

            template<typename T>
            constexpr T log_mantissa (const T x) noexcept
            {
                return (log_main(x / T(static_cast<int>(x))) + T(log_mantissa_integer(static_cast<int>(x))));
            }

            template<typename T>
            constexpr T log_breakup (const T x) noexcept
            {
                return (log_mantissa (mantissa(x)) + morph::mathconst<T>::ln_10 * T(find_exponent (x, 0)));
            }

            template<typename T>
            constexpr T log_check (const T x) noexcept
            {
                return (is_nan(x) ? std::numeric_limits<T>::quiet_NaN() :
                        x < T(0) ? std::numeric_limits<T>::quiet_NaN() :
                        std::numeric_limits<T>::min() > x ? -std::numeric_limits<T>::infinity() :
                        std::numeric_limits<T>::min() > abs(x - T{1}) ? T{0} :
                        x == std::numeric_limits<T>::infinity() ? std::numeric_limits<T>::infinity() :
                        (x < T{0.5} || x > T{1.5}) ? log_breakup(x) : log_main(x));
            }

            template<typename T>
            constexpr return_t<T> log_integral_check (const T x) noexcept
            {
                return (std::is_integral<T>::value ? x == T(0) ? -std::numeric_limits<return_t<T>>::infinity() :
                        x > T{1} ? log_check (static_cast<return_t<T>>(x)) :
                        static_cast<return_t<T>>(0) : log_check (static_cast<return_t<T>>(x)));
            }
        }
        template<typename T>
        constexpr return_t<T> log (const T x) noexcept { return internal::log_integral_check (x); }

        namespace internal // pow_integral
        {
            template<typename T1, typename T2>
            constexpr T1 pow_integral_compute (const T1 base, const T2 exp_term) noexcept;
            template<typename T1, typename T2>
            constexpr T1 pow_integral_compute_recur (const T1 base, const T1 val, const T2 exp_term) noexcept
            {
                return (exp_term > T2{1} ? (is_odd (exp_term) ?
                                            pow_integral_compute_recur (base * base, val * base, exp_term / 2) :
                                            pow_integral_compute_recur (base * base, val, exp_term / 2)) :
                        (exp_term == T2{1} ? val * base : val));
            }
            template<typename T1, typename T2, typename std::enable_if<std::is_signed<T2>::value>::type* = nullptr>
            constexpr T1 pow_integral_sgn_check (const T1 base, const T2 exp_term) noexcept
            {
                return (exp_term < T2{0} ? T1{1} / pow_integral_compute (base, -exp_term) : pow_integral_compute_recur (base, T1{1}, exp_term));
            }
            template<typename T1, typename T2, typename std::enable_if<!std::is_signed<T2>::value>::type* = nullptr>
            constexpr T1 pow_integral_sgn_check (const T1 base, const T2 exp_term) noexcept { return (pow_integral_compute_recur (base, T1{1}, exp_term)); }

            template<typename T1, typename T2>
            constexpr T1 pow_integral_compute (const T1 base, const T2 exp_term) noexcept
            {
                return (exp_term == T2{3} ? base * base * base :
                        exp_term == T2{2} ? base * base :
                        exp_term == T2{1} ? base :
                        exp_term == T2{0} ? T1{1} :
                        exp_term == std::numeric_limits<T2>::min() ? T1{0} :
                        exp_term == std::numeric_limits<T2>::max() ? std::numeric_limits<T1>::infinity() :
                        pow_integral_sgn_check (base, exp_term));
            }

            template<typename T1, typename T2, typename std::enable_if<std::is_integral<T2>::value>::type* = nullptr>
            constexpr T1 pow_integral_type_check (const T1 base, const T2 exp_term) noexcept { return pow_integral_compute (base,exp_term); }

            template<typename T1, typename T2, typename std::enable_if<!std::is_integral<T2>::value>::type* = nullptr>
            constexpr T1 pow_integral_type_check (const T1 base, const T2 exp_term) noexcept
            {
                return pow_integral_compute (base, static_cast<int64_t>(exp_term));
            }

            template<typename T1, typename T2>
            constexpr T1 pow_integral (const T1 base, const T2 exp_term) noexcept { return internal::pow_integral_type_check (base, exp_term); }
        }

        namespace internal // ceil
        {
            template<typename T>
            constexpr int ceil_resid (const T x, const T x_whole) noexcept { return ((x > T{0}) && (x > x_whole)); }
            template<typename T>
            constexpr T ceil_int (const T x, const T x_whole) noexcept { return (x_whole + static_cast<T>(ceil_resid(x,x_whole))); }
            template<typename T>
            constexpr T ceil_check_internal (const T x) noexcept { return x; }
            template<>
            constexpr float ceil_check_internal<float> (const float x) noexcept
            {
                return (abs(x) >= 8388608.f ? x : ceil_int(x, float(static_cast<int>(x))));
            }
            template<>
            constexpr double ceil_check_internal<double> (const double x) noexcept
            { return (abs(x) >= 4503599627370496. ? x : ceil_int(x, double(static_cast<int64_t>(x)))); }
            template<>
            constexpr long double ceil_check_internal<long double> (const long double x) noexcept
            { return (abs(x) >= 9223372036854775808.l ?  x : ceil_int(x, ((long double)static_cast<uint64_t>(abs(x))) * sgn(x))); }
            template<typename T>
            constexpr T ceil_check (const T x) noexcept
            {
                return (is_nan(x) ? std::numeric_limits<T>::quiet_NaN() :
                        !is_finite(x) ? x :
                        std::numeric_limits<T>::min() > abs(x) ? x : ceil_check_internal(x));
            }
        }
        template<typename T>
        constexpr return_t<T> ceil (const T x) noexcept { return internal::ceil_check (static_cast<return_t<T>>(x)); }

        namespace internal // floor
        {
            template<typename T>
            constexpr int floor_resid (const T x, const T x_whole) noexcept { return ((x < T{0}) && (x < x_whole)); }
            template<typename T>
            constexpr T floor_int (const T x, const T x_whole) noexcept { return (x_whole - static_cast<T>(floor_resid (x,x_whole))); }
            template<typename T>
            constexpr T floor_check_internal (const T x) noexcept { return x; }
            template<>
            constexpr float floor_check_internal<float> (const float x) noexcept
            {
                return (abs(x) >= 8388608.f ? x : floor_int (x, float(static_cast<int>(x))));
            }
            template<>
            constexpr double floor_check_internal<double> (const double x) noexcept
            {
                return (abs(x) >= 4503599627370496. ? x : floor_int (x, double(static_cast<int64_t>(x))));
            }
            template<>
            constexpr long double floor_check_internal<long double> (const long double x) noexcept
            {
                return (abs(x) >= 9223372036854775808.l ? x : floor_int (x, ((long double)static_cast<uint64_t>(abs(x))) * sgn(x)));
            }
            template<typename T>
            constexpr T floor_check (const T x) noexcept
            {
                return (is_nan(x) ? std::numeric_limits<T>::quiet_NaN() : !is_finite(x) ? x :
                        std::numeric_limits<T>::min() > abs(x) ? x : floor_check_internal(x));
            }
        }
        template<typename T>
        constexpr return_t<T> floor (const T x) noexcept { return internal::floor_check (static_cast<return_t<T>>(x)); }

        namespace internal // find_whole, find_fraction
        {
            template<typename T>
            constexpr int64_t find_whole (const T x) noexcept
            {
                return (abs(x - internal::floor_check(x)) >= T{0.5} ? static_cast<int64_t>(internal::floor_check(x) + sgn(x)) :
                        static_cast<int64_t>(internal::floor_check(x)));
            }

            template<typename T>
            constexpr T find_fraction (const T x) noexcept
            {
                return (abs(x - internal::floor_check(x)) >= T{0.5} ? x - internal::floor_check(x) - sgn(x) :
                        x - internal::floor_check(x));
            }
        }

        namespace internal // exp
        {
            template<typename T>
            constexpr T exp_cf_recur (const T x, const int depth_end) noexcept
            {
                int depth = exp_max_iter_small - 1;
                T res = T{1};
                while (depth > depth_end - 1) {
                    res = T{1} + x / T(depth - 1) - x / depth / res;
                    --depth;
                }
                return res;
            }

            template<typename T>
            constexpr T exp_cf (const T x) noexcept { return (T{1} / (T{1} - x / exp_cf_recur(x, 2))); }

            template<typename T>
            constexpr T exp_split(const T x) noexcept
            {
                return (static_cast<T>(pow_integral (morph::mathconst<T>::e, find_whole(x))) * exp_cf (find_fraction(x)));
            }

            template<typename T>
            constexpr T exp_check(const T x) noexcept
            {
                return (is_nan(x) ? std::numeric_limits<T>::quiet_NaN() : is_neginf(x) ? T{0} :
                        std::numeric_limits<T>::min() > abs(x) ? T{1} : is_posinf(x) ? std::numeric_limits<T>::infinity() :
                        abs(x) < T{2} ? exp_cf(x) : exp_split(x));
            }

        }
        template<typename T>
        constexpr return_t<T> exp (const T x) noexcept { return internal::exp_check (static_cast<return_t<T>>(x)); }

        namespace internal // pow
        {
            template<typename T>
            constexpr T pow_dbl (const T base, const T exp_term) noexcept { return exp (exp_term * log(base)); }

            template<typename T1, typename T2, typename TC = common_t<T1, T2>,
                     typename std::enable_if<!std::is_integral<T2>::value>::type* = nullptr>
            constexpr TC pow_check (const T1 base, const T2 exp_term) noexcept
            {
                return (base < T1{0} ? std::numeric_limits<TC>::quiet_NaN() : pow_dbl (static_cast<TC>(base), static_cast<TC>(exp_term)));
            }

            template<typename T1, typename T2, typename TC = common_t<T1, T2>,
                     typename std::enable_if<std::is_integral<T2>::value>::type* = nullptr>
            constexpr TC pow_check (const T1 base, const T2 exp_term) noexcept { return pow_integral (base, exp_term); }

        }
        template<typename T1, typename T2>
        constexpr common_t<T1, T2> pow (const T1 base, const T2 exp_term) noexcept { return internal::pow_check (base, exp_term); }

        namespace internal // atan
        {
            template<typename T>
            constexpr T atan_series_order_calc (const T xx, const T x_pow, const uint32_t order) noexcept
            { return (T{1}/( T((order-1)*4 - 1) * x_pow ) - T{1}/( T((order-1)*4 + 1) * x_pow * xx)); }
            template<typename T>
            constexpr T atan_series_order (const T x, const T x_pow, const uint32_t order_begin, const uint32_t max_order) noexcept
            {
                if (max_order == 1) { return morph::mathconst<T>::pi_over_2 - T{1} / x_pow; }

                T xx = x * x;
                T res = atan_series_order_calc (xx, pow (x, 4 * max_order - 5), max_order);
                uint32_t depth = max_order - 1;
                while (depth > order_begin) {
                    res += atan_series_order_calc (xx, pow (x, 4 * depth - 5), depth);
                    --depth;
                }
                res += morph::mathconst<T>::pi_over_2 - T{1} / x;
                return res;
            }
            template<typename T>
            constexpr T atan_series_main (const T x) noexcept
            {
                return (x < T{3}    ? atan_series_order (x, x, 1U, 10U) :  // O(1/x^39)
                        x < T{4}    ? atan_series_order (x, x, 1U, 9U)  :  // O(1/x^35)
                        x < T{5}    ? atan_series_order (x, x, 1U, 8U)  :  // O(1/x^31)
                        x < T{7}    ? atan_series_order (x, x, 1U, 7U)  :  // O(1/x^27)
                        x < T{11}   ? atan_series_order (x, x, 1U, 6U)  :  // O(1/x^23)
                        x < T{25}   ? atan_series_order (x, x, 1U, 5U)  :  // O(1/x^19)
                        x < T{100}  ? atan_series_order (x, x, 1U, 4U)  :  // O(1/x^15)
                        x < T{1000} ? atan_series_order (x, x, 1U, 3U)  :  // O(1/x^11)
                        atan_series_order (x, x, 1U, 2U));  // O(1/x^7)
            }
            template<typename T>
            constexpr T atan_cf_recur (const T xx, const uint32_t depth_begin, const uint32_t max_depth) noexcept
            {
                uint32_t depth = max_depth - 1;
                T res = T(2 * (depth + 1) - 1);
                while (depth > depth_begin - 1) {
                    res = T(2 * depth - 1) + T(depth * depth) * xx / res;
                    --depth;
                }
                return res;
            }
            template<typename T>
            constexpr T atan_cf_main (const T x) noexcept
            {
                return (x < T{0.5} ? x/atan_cf_recur (x*x, 1U, 15U) : x < T{1}   ? x/atan_cf_recur (x*x, 1U, 25U) :
                        x < T{1.5} ? x/atan_cf_recur (x*x, 1U, 35U) : x < T{2}   ? x/atan_cf_recur (x*x, 1U, 45U) :
                        x / atan_cf_recur (x*x, 1U, 52U));
            }
            template<typename T>
            constexpr T atan_begin (const T x) noexcept { return (x > T(2.5) ? atan_series_main(x) : atan_cf_main(x)); }
            template<typename T>
            constexpr T atan_check (const T x) noexcept
            {
                return (is_nan(x) ? std::numeric_limits<T>::quiet_NaN() :
                        std::numeric_limits<T>::min() > abs(x) ? T{0} : x < T(0) ? -atan_begin(-x) : atan_begin(x));
            }
        }
        template<typename T>
        constexpr return_t<T> atan (const T x) noexcept { return internal::atan_check (static_cast<return_t<T>>(x)); }

        namespace internal // atan2
        {
            template<typename T>
            constexpr T atan2_compute (const T y, const T x) noexcept
            {
                return (any_nan(y,x) ? std::numeric_limits<T>::quiet_NaN() :
                        std::numeric_limits<T>::min() > abs(x) ? std::numeric_limits<T>::min() > abs(y) ? neg_zero(y) ? neg_zero(x) ? -morph::mathconst<T>::pi :
                        - T{0} : neg_zero(x) ? morph::mathconst<T>::pi : T{0} :
                        y > T{0} ? morph::mathconst<T>::pi_over_2 : -morph::mathconst<T>::pi_over_2 :
                        x < T{0} ? y < T{0} ? atan(y/x) - morph::mathconst<T>::pi :
                        atan(y/x) + morph::mathconst<T>::pi :
                        atan(y/x));
            }
            template<typename T1, typename T2, typename TC = common_return_t<T1, T2>>
            constexpr TC atan2_type_check (const T1 y, const T2 x) noexcept { return atan2_compute (static_cast<TC>(x), static_cast<TC>(y)); }
        }
        template<typename T1, typename T2>
        constexpr common_return_t<T1, T2> atan2 (const T1 y, const T2 x) noexcept { return internal::atan2_type_check (x,y); }

        namespace internal
        {
            template<typename T>
            constexpr T sqrt_recur (const T x, const T xn, const int count) noexcept
            {
                return (abs(xn - x/xn) / (T{1} + xn) < std::numeric_limits<T>::min() ? xn :
                        count < sqrt_max_iter ? sqrt_recur (x, T{0.5}*(xn + x/xn), count+1) : xn);
            }
            template<typename T>
            constexpr T sqrt_simplify (const T x, const T m_val) noexcept
            {
                return (x > T{1e+08} ? sqrt_simplify(x / T{1e+08}, T{1e+04} * m_val) :
                        x > T{1e+06} ? sqrt_simplify(x / T{1e+06}, T{1e+03} * m_val) :
                        x > T{1e+04} ? sqrt_simplify(x / T{1e+04}, T{1e+02} * m_val) :
                        x > T{100} ? sqrt_simplify(x / T{100}, T{10} * m_val) :
                        x > T{4} ? sqrt_simplify(x / T{4}, T{2} * m_val) : m_val * sqrt_recur(x, x / T{2}, 0));
            }
            template<typename T>
            constexpr T sqrt_check (const T x) noexcept
            {
                return (is_nan (x) ? std::numeric_limits<T>::quiet_NaN() :
                        x < T{0} ?  std::numeric_limits<T>::quiet_NaN() :
                        is_posinf(x) ? x :
                        std::numeric_limits<T>::min() > abs(x) ? T{0} :
                        std::numeric_limits<T>::min() > abs(T{1} - x) ? x : sqrt_simplify (x, T{1}));
            }
        }
        template<typename T>
        constexpr return_t<T> sqrt (const T x) noexcept { return internal::sqrt_check (static_cast<return_t<T>>(x)); }

        namespace internal // round
        {
            template<typename T>
            constexpr T round_int (const T x) noexcept
            {
                return (abs(x - internal::floor_check(x)) >= T{0.5} ? internal::floor_check(x) + sgn(x) : internal::floor_check(x));
            }
            template<typename T>
            constexpr T round_check_internal (const T x) noexcept { return x; }
            template<>
            constexpr float round_check_internal<float> (const float x) noexcept { return (abs(x) >= 8388608.f ? x : round_int (x)); }
            template<>
            constexpr double round_check_internal<double> (const double x) noexcept { return (abs(x) >= 4503599627370496. ? x : round_int (x)); }
            template<>
            constexpr long double round_check_internal<long double> (const long double x) noexcept { return (abs(x) >= 9223372036854775808.l ? x : round_int (x)); }
            template<typename T>
            constexpr T round_check (const T x) noexcept
            {
                return (is_nan(x) ? std::numeric_limits<T>::quiet_NaN() : !is_finite(x) ? x :
                        std::numeric_limits<T>::min() > abs(x) ? x : sgn(x) * round_check_internal (abs(x)));
            }
        }
        template<typename T>
        constexpr return_t<T> round (const T x) noexcept { return internal::round_check (static_cast<return_t<T>>(x)); }

        namespace internal // cos
        {
            template<typename T>
            constexpr T cos_compute (const T x) noexcept { return (T{1} - x * x) / (T{1} + x * x); }
            template<typename T>
            constexpr T cos_check (const T x) noexcept
            {
                return (is_nan(x) ? std::numeric_limits<T>::quiet_NaN() :
                        std::numeric_limits<T>::min() > abs(x) ? T{1} :
                        std::numeric_limits<T>::min() > abs(x - morph::mathconst<T>::pi_over_2) ? T{0} : // special cases: pi/2 and pi
                        std::numeric_limits<T>::min() > abs(x + morph::mathconst<T>::pi_over_2) ? T{0} :
                        std::numeric_limits<T>::min() > abs(x - morph::mathconst<T>::pi) ? T{-1} :
                        std::numeric_limits<T>::min() > abs(x + morph::mathconst<T>::pi) ? T{-1} : cos_compute (tan (x/T{2})));
            }
        }
        template<typename T>
        constexpr return_t<T> cos (const T x) noexcept { return internal::cos_check (static_cast<return_t<T>>(x)); }

        namespace internal // sin
        {
            template<typename T>
            constexpr T sin_compute (const T x) noexcept { return T{2} * x / (T{1} + x * x); }
            template<typename T>
            constexpr T sin_check (const T x) noexcept
            {
                return (is_nan(x) ? std::numeric_limits<T>::quiet_NaN() :
                        std::numeric_limits<T>::min() > abs(x) ? T{0} :
                        std::numeric_limits<T>::min() > abs(x - morph::mathconst<T>::pi_over_2) ? T{1} :
                        std::numeric_limits<T>::min() > abs(x + morph::mathconst<T>::pi_over_2) ? T{-1} :
                        std::numeric_limits<T>::min() > abs(x - morph::mathconst<T>::pi) ? T{0} :
                        std::numeric_limits<T>::min() > abs(x + morph::mathconst<T>::pi) ? T{-0} : sin_compute (tan (x / T{2})));
            }
        }
        template<typename T>
        constexpr return_t<T> sin (const T x) noexcept { return internal::sin_check (static_cast<return_t<T>>(x)); }

    } // math
} // morph
