/*!
 * \file
 * \brief An N dimensional vector class template which derives from std::array.
 *
 * \author Seb James (with thanks to Konrad Rudolph and Miguel Avila for code review)
 * \date April 2020
 */
#pragma once

#include <cmath>
#include <array>
#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <numeric>
#include <limits>
#include <iomanip>
#include <algorithm>
#include <functional>
#include <morph/Random.h>

namespace morph {

    /*!
     * \brief N-D vector class
     *
     * An N dimensional vector class template which derives from std::array. Vector
     * components are of scalar type S. It is anticipated that S will be set either to
     * floating point scalar types such as float or double, or to integer scalar types
     * such as int, long long int and so on. Thus, a typical (and in fact, the default)
     * signature would be:
     *
     * Vector<float, 3> v;
     *
     * The class inherits std:array's fixed-size array of memory for storing the
     * components of the vector. It adds numerous methods which allow objects of type
     * Vector to have arithmetic operations applied to them, either scalar (add a scalar
     * to all elements; divide all elements by a scalar, etc) or vector (including dot
     * and cross products, normalization and so on.
     *
     * Because morph::Vector extends std::array, it works best when compiled with a
     * c++-17 compiler (although it can be compiled with a c++-11 compiler). This is
     * because std::array is an 'aggregate class' with no user-provided constructors,
     * and morph::Vector does not add any of its own constructors. Prior to c++-17,
     * aggregate classes were not permitted to have base classes. So, if you want to do:
     *
     * Vector<float, 3> v = { 1.0f , 1.0f, 1.0f };
     *
     * You need c++-17. Otherwise, restrict your client code to doing:
     *
     * Vector<float, 3> v;
     * v[0] = 1.0f; v[1] = 1.0f; v[2] = 1.0f;
     */
    template <typename S, size_t N> struct Vector;

    /*!
     * Template friendly mechanism to overload the stream operator.
     *
     * Note forward declaration of the Vector template class and this template for
     * stream operator overloading. Example adapted from
     * https://stackoverflow.com/questions/4660123
     */
    template <typename S, size_t N> std::ostream& operator<< (std::ostream&, const Vector<S, N>&);

    template <typename S=float, size_t N=3>
    struct Vector : public std::array<S, N>
    {
        //! \return the first component of the vector
        template <size_t _N = N, std::enable_if_t<(_N>0), int> = 0>
        S x() const { return (*this)[0]; }
        //! \return the second component of the vector
        template <size_t _N = N, std::enable_if_t<(_N>1), int> = 0>
        S y() const { return (*this)[1]; }
        //! \return the third component of the vector
        template <size_t _N = N, std::enable_if_t<(_N>2), int> = 0>
        S z() const { return (*this)[2]; }
        //! \return the fourth component of the vector
        template <size_t _N = N, std::enable_if_t<(_N>3), int> = 0>
        S w() const { return (*this)[3]; }

        //! Set data members from an std::vector
        template <typename _S=S>
        void set_from (const std::vector<_S>& vec)
        {
            if (vec.size() != N) {
                throw std::runtime_error ("Vector::set_from(): Ensure vector sizes match");
            }
            std::copy (vec.begin(), vec.end(), this->begin());
        }

        //! Set data members from an array the of same size and type.
        template <typename _S=S>
        void set_from (const std::array<_S, N>& ar)
        {
            std::copy (ar.begin(), ar.end(), this->begin());
        }

        /*!
         * Set the data members of this Vector from the passed in, larger array, \a ar,
         * ignoring the last element of \a ar. Used when working with 4D vectors in
         * graphics applications involving 4x4 transform matrices.
         */
        template <typename _S=S>
        void set_from (const std::array<_S, (N+1)>& ar)
        {
            // Don't use std::copy here, because ar has more elements than *this.
            for (size_t i = 0; i < N; ++i) { (*this)[i] = ar[i]; }
        }

        /*!
         * Set the data members of this Vector from the passed in, smaller array, \a ar,
         * ignoring the last element of this vector (which is set to 0). Used when
         * working with 2D vectors that you want to visualise in a 3D environment with
         * z set to 0.
         */
        template <typename _S=S>
        void set_from (const std::array<_S, (N-1)>& ar)
        {
            // Don't use std::copy here, because ar has more elements than *this.
            for (size_t i = 0; i < N-1; ++i) { (*this)[i] = ar[i]; }
            (*this)[N-1] = S{0};
        }

        /*!
         * Set an N D Vector from an N+1 D Vector. Intended to convert 4D vectors (that
         * have been operated on by 4x4 matrices) into 3D vectors.
         */
        template <typename _S=S>
        void set_from (const Vector<_S, (N+1)>& v)
        {
            for (size_t i = 0; i < N; ++i) { (*this)[i] = v[i]; }
        }

        //! Set an N D Vector from an (N-1) D Vector.
        template <typename _S=S>
        void set_from (const Vector<_S, (N-1)>& v)
        {
            for (size_t i = 0; i < N-1; ++i) { (*this)[i] = v[i]; }
            (*this)[N-1] = S{0};
        }

        //! Set all elements from the value type v
        template <typename _S=S>
        void set_from (const _S& v) { std::fill (this->begin(), this->end(), v); }

        /*!
         * Set a linear sequence into the vector from value start to value stop. Uses
         * the Vector's size to determine how many values to create. You *can* use this
         * with integer types, but be prepared to notice strange rounding errors.
         */
        template <typename _S=S, typename _S2=S>
        void linspace (const _S start, const _S2 stop)
        {
            S increment = (static_cast<S>(stop) - static_cast<S>(start)) / (N-1);
            for (size_t i = 0; i < this->size(); ++i) { (*this)[i] = start + increment * i; }
        }

        //! Return a vector with one less dimension - losing the last one.
        Vector<S, N-1> less_one_dim () const
        {
            Vector<S, N-1> rtn;
            for (size_t i = 0; i < N-1; ++i) { rtn[i] = (*this)[i]; }
            return rtn;
        }

        //! Return a vector with one additional dimension - setting it to 0.
        Vector<S, N+1> plus_one_dim () const
        {
            Vector<S, N+1> rtn;
            for (size_t i = 0; i < N; ++i) { rtn[i] = (*this)[i]; }
            rtn[N] = S{0};
            return rtn;
        }

        //! Return this Vector in single precision, float format
        Vector<float, N> as_float() const
        {
            Vector<float, N> v;
            v.zero();
            v += *this;
            return v;
        }

        //! Return this Vector in double precision, float format
        Vector<float, N> as_double() const
        {
            Vector<double, N> v;
            v.zero();
            v += *this;
            return v;
        }

        //! Stream the coordinates of the vector into \a ss as a comma separated list.
        void str_comma_separated (std::stringstream& ss) const
        {
            ss << std::setprecision (std::numeric_limits<S>::max_digits10);
            bool first = true;
            for (auto i : *this) {
                if (first) {
                    ss << i;
                    first = false;
                } else {
                    ss << "," << i;
                }
            }
        }

        /*!
         * Create a string representation of the vector
         *
         * \return A 'coordinate format' string such as "(1,1,2)", "(0.2,0.4)" or
         * "(5,4,5,5,40)".
         */
        std::string str() const
        {
            std::stringstream ss;
            ss << "(";
            this->str_comma_separated (ss);
            ss << ")";
            return ss.str();
        }

        //! Output the vector in a form suitable to paste into MATLAB or Octave
        std::string str_mat() const
        {
            std::stringstream ss;
            ss << "[";
            this->str_comma_separated (ss);
            ss << "]";
            return ss.str();
        }

        //! Output the vector in a form suitable to paste into Python, as a numpy
        //! vector, assuming you imported numpy as np
        std::string str_numpy() const
        {
            std::stringstream ss;
            ss << "np.array((";
            this->str_comma_separated (ss);
            ss << "))";
            return ss.str();
        }

        //! Renormalize the vector to length 1.0. Only for S types that are floating point.
        template <typename _S=S, std::enable_if_t<!std::is_integral<std::decay_t<_S>>::value, int> = 0 >
        void renormalize()
        {
            auto add_squared = [](_S a, _S b) { return a + b * b; };
            const _S denom = std::sqrt (std::accumulate (this->begin(), this->end(), _S{0}, add_squared));
            if (denom != _S{0}) {
                _S oneovermag = _S{1} / denom;
                auto x_oneovermag = [oneovermag](_S f) { return f * oneovermag; };
                std::transform (this->begin(), this->end(), this->begin(), x_oneovermag);
            }
        }

        /*!
         * Permute the elements in a rotation. 0->N-1, 1->0, 2->1, etc. Useful for
         * swapping x and y in a 2D Vector.
         */
        void rotate()
        {
            if constexpr (N>1) {
                S z_el = (*this)[0];
                for (size_t i = 1; i < N; ++i) {
                    (*this)[i-1] = (*this)[i];
                }
                (*this)[N-1] = z_el;
            } // else no op
        }

        //! If N is even, permute pairs of elements in a rotation. 0->1, 1->0, 2->3, 3->2, etc.
        void rotate_pairs()
        {
            static_assert ((N%2==0), "N must be even to call morph::Vector::rotate_pairs");
            S tmp_el = S{0};
            for (size_t i = 0; i < N; i+=2) {
                tmp_el = (*this)[i];
                (*this)[i] = (*this)[i+1];
                (*this)[i+1] = tmp_el;
            }
        }

        //! Zero the vector. Set all coordinates to 0
        void zero() { std::fill (this->begin(), this->end(), S{0}); }

        /*!
         * Randomize the vector
         *
         * Randomly set the elements of the vector. Coordinates are set to random
         * numbers drawn from a uniform distribution between 0 and 1 if S is a
         * floating point type or to integers between std::numeric_limits<S>::min()
         * and std::numeric_limits<S>::max() if S is an integral type (See
         * morph::RandUniform for details).
         */
        void randomize()
        {
            RandUniform<S> ru;
            ru.get (*this);
        }

        /*!
         * Randomize the vector with provided bounds
         *
         * Randomly set the elements of the vector. Coordinates are set to random
         * numbers drawn from a uniform distribution between \a min and \a
         * max. Strictly, the range is [min, max)
         */
        void randomize (S min, S max)
        {
            RandUniform<S> ru (min, max);
            ru.get (*this);
        }

        /*!
         * Randomize the vector from a Gaussian distribution
         *
         * Randomly set the elements of the vector. Coordinates are set to random
         * numbers drawn from a uniform distribution between \a min and \a
         * max. Strictly, the range is [min, max)
         */
        void randomizeN (S _mean, S _sd)
        {
            RandNormal<S> rn (_mean, _sd);
            rn.get (*this);
        }

        /*!
         * Test to see if this vector is a unit vector (it doesn't *have* to be).
         *
         * \return true if the length of the vector is 1.
         */
        template <typename _S=S, std::enable_if_t<!std::is_integral<std::decay_t<_S>>::value, int> = 0 >
        bool checkunit() const
        {
            /*!
             * \brief Unit vector threshold
             *
             * The threshold outside of which the vector is no longer considered to be a
             * unit vector. Note this is hard coded as a constexpr, to avoid messing with
             * the initialization of the Vector with curly brace initialization.
             *
             * Clearly, this will be the wrong threshold for some cases. Possibly, a
             * template parameter could set this; so size_t U could indicate the threshold;
             * 0.001 could be U=-3 (10^-3).
             *
             * Another idea would be to change unitThresh based on the type S. Or use
             * numeric_limits<S>::epsilon and find out what multiple of epsilon would make
             * sense.
             */
            static constexpr _S unitThresh = _S{0.001};

            auto subtract_squared = [](_S a, _S b) { return static_cast<_S>(a - b * b); };
            const _S metric = std::accumulate (this->begin(), this->end(), _S{1}, subtract_squared);
            if (std::abs(metric) > unitThresh) {
                return false;
            }
            return true;
        }

        /*!
         * Find the length of the vector.
         *
         * \return the length
         */
        S length() const
        {
            auto add_squared = [](S a, S b) { return a + b * b; };
            // Add check on whether S is integral or float. If integral, then std::round then cast the result of std::sqrt()
            if constexpr (std::is_integral<std::decay_t<S>>::value == true) {
                const S len = static_cast<S>(std::round(std::sqrt(std::accumulate(this->begin(), this->end(), S{ 0 }, add_squared))));
                return len;
            } else {
                const S len = std::sqrt(std::accumulate(this->begin(), this->end(), S{ 0 }, add_squared));
                return len;
            }
        }

        /*!
         * Find the squared length of the vector.
         *
         * \return the square of the length
         */
        S length_sq() const
        {
            auto add_squared = [](S a, S b) { return a + b * b; };
            const S len_sq = std::accumulate (this->begin(), this->end(), S{0}, add_squared);
            return len_sq;
        }

        //! Return the value of the longest component of the vector.
        S longest() const
        {
            auto abs_compare = [](S a, S b) { return (std::abs(a) < std::abs(b)); };
            auto thelongest = std::max_element (this->begin(), this->end(), abs_compare);
            S rtn = *thelongest;
            return rtn;
        }

        //! Return the index of the longest component of the vector.
        size_t arglongest() const
        {
            size_t idx = 0;
            if constexpr (std::is_scalar<std::decay_t<S>>::value) {
                auto abs_compare = [](S a, S b) { return (std::abs(a) < std::abs(b)); };
                auto thelongest = std::max_element (this->begin(), this->end(), abs_compare);
                idx = (thelongest - this->begin());
            } else {
                auto vec_compare = [](S a, S b) { return (a.length() < b.length()); };
                auto thelongest = std::max_element (this->begin(), this->end(), vec_compare);
                idx = (thelongest - this->begin());
            }
            return idx;
        }

        //! Return the value of the shortest component of the vector.
        S shortest() const
        {
            auto abs_compare = [](S a, S b) { return (std::abs(a) > std::abs(b)); };
            auto theshortest = std::max_element (this->begin(), this->end(), abs_compare);
            S rtn = *theshortest;
            return rtn;
        }

        //! Return the index of the shortest component of the vector.
        size_t argshortest() const
        {
            size_t idx = 0;
            // Check on the type S. If S is a Vector thing, then abs_compare needs to be different.
            if constexpr (std::is_scalar<std::decay_t<S>>::value) {
                auto abs_compare = [](S a, S b) { return (std::abs(a) > std::abs(b)); };
                auto theshortest = std::max_element (this->begin(), this->end(), abs_compare);
                idx = (theshortest - this->begin());
            } else {
                auto vec_compare = [](S a, S b) { return (a.length() > b.length()); };
                auto theshortest = std::max_element (this->begin(), this->end(), vec_compare);
                idx = (theshortest - this->begin());
            }
            return idx;
        }

        //! Return the value of the maximum (most positive) component of the vector.
        S max() const
        {
            auto themax = std::max_element (this->begin(), this->end());
            S rtn = *themax;
            return rtn;
        }

        //! Return the index of the maximum (most positive) component of the vector.
        size_t argmax() const
        {
            auto themax = std::max_element (this->begin(), this->end());
            size_t idx = (themax - this->begin());
            return idx;
        }

        //! Return the value of the minimum (smallest or most negative) component of the vector.
        S min() const
        {
            auto themin = std::min_element (this->begin(), this->end());
            S rtn = *themin;
            return rtn;
        }

        //! Return the index of the minimum (smallest or most negative) component of the vector.
        size_t argmin() const
        {
            auto themin = std::min_element (this->begin(), this->end());
            size_t idx = (themin - this->begin());
            return idx;
        }

        //! Return true if any element is zero
        bool has_zero() const
        {
            return std::any_of (this->cbegin(), this->cend(), [](S i){ return i == S{0}; });
        }

        //! Return true if any element is infinity
        bool has_inf() const
        {
            if constexpr (std::numeric_limits<S>::has_infinity) {
                return std::any_of (this->cbegin(), this->cend(), [](S i){return std::isinf(i);});
            } else {
                return false;
            }
        }

        //! Return true if any element is NaN
        bool has_nan() const
        {
            if constexpr (std::numeric_limits<S>::has_quiet_NaN
                          || std::numeric_limits<S>::has_signaling_NaN) {
                return std::any_of (this->cbegin(), this->cend(), [](S i){return std::isnan(i);});
            } else {
                return false;
            }
        }

        //! Return true if any element is NaN or infinity
        bool has_nan_or_inf() const
        {
            bool has_nan_or_inf = false;
            has_nan_or_inf = this->has_nan();
            return has_nan_or_inf ? has_nan_or_inf : this->has_inf();
        }

        //! Return the arithmetic mean of the elements
        S mean() const
        {
            const S sum = std::accumulate (this->begin(), this->end(), S{0});
            return sum / this->size();
        }

        //! Return the sum of the elements
        S sum() const { return std::accumulate (this->begin(), this->end(), S{0}); }

        //! Return the product of the elements
        S product() const
        {
            auto _product = [](S a, S b) mutable -> S { return a ? a * b : b; };
            return std::accumulate (this->begin(), this->end(), S{0}, _product);
        }

        /*!
         * Compute the element-wise pth power of the vector
         *
         * \return a Vector whose elements have been raised to the power p
         */
        Vector<S, N> pow (const S& p) const
        {
            Vector<S, N> rtn;
            auto raise_to_p = [p](S coord) -> S { return static_cast<S>(std::pow(coord, p)); };
            std::transform (this->begin(), this->end(), rtn.begin(), raise_to_p);
            return rtn;
        }
        //! Raise each element to the power p
        void pow_inplace (const S& p) { for (auto& i : *this) { i = static_cast<S>(std::pow (i, p)); } }

        //! Element-wise power
        template<typename _S=S>
        Vector<S, N> pow (const Vector<_S, N>& p) const
        {
            auto pi = p.begin();
            Vector<S, N> rtn;
            auto raise_to_p = [pi](S coord) mutable -> S { return static_cast<S>(std::pow(coord, (*pi++))); };
            std::transform (this->begin(), this->end(), rtn.begin(), raise_to_p);
            return rtn;
        }
        template<typename _S=S>
        void pow_inplace (const Vector<_S, N>& p)
        {
            auto pi = p.begin();
            for (auto& i : *this) { i = static_cast<S>(std::pow (i, (*pi++))); }
        }

        //! Return the signum of the Vector, with signum(0)==0
        Vector<S, N> signum() const
        {
            Vector<S, N> rtn;
            auto _signum = [](S coord) -> S { return (coord > S{0} ? S{1} : (coord == S{0} ? S{0} : S{-1})); };
            std::transform (this->begin(), this->end(), rtn.begin(), _signum);
            return rtn;
        }
        void signum_inplace() { for (auto& i : *this) { i = (i > S{0} ? S{1} : (i == S{0} ? S{0} : S{-1})); } }

        //! Return the floor of the Vector
        Vector<S, N> floor() const
        {
            Vector<S, N> rtn;
            auto _floor = [](S coord) -> S { return (std::floor(coord)); };
            std::transform (this->begin(), this->end(), rtn.begin(), _floor);
            return rtn;
        }
        void floor_inplace() { for (auto& i : *this) { i = std::floor(i); } }

        //! Return the floor-or-ceiling of the vector's elements - i.e. apply std::trunc
        Vector<S, N> trunc() const
        {
            Vector<S, N> rtn;
            auto _trunc = [](S coord) -> S { return (std::trunc(coord)); };
            std::transform (this->begin(), this->end(), rtn.begin(), _trunc);
            return rtn;
        }
        void trunc_inplace() { for (auto& i : *this) { i = std::trunc(i); } }

        //! Return the ceiling of the Vector
        Vector<S, N> ceil() const
        {
            Vector<S, N> rtn;
            auto _ceil = [](S coord) -> S { return (std::ceil(coord)); };
            std::transform (this->begin(), this->end(), rtn.begin(), _ceil);
            return rtn;
        }
        void ceil_inplace() { for (auto& i : *this) { i = std::ceil(i); } }

        /*!
         * Compute the element-wise square root of the vector
         *
         * \return a Vector whose elements have been square-rooted
         */
        Vector<S, N> sqrt() const
        {
            Vector<S, N> rtn;
            auto sqrt_element = [](S coord) -> S { return static_cast<S>(std::sqrt(coord)); };
            std::transform (this->begin(), this->end(), rtn.begin(), sqrt_element);
            return rtn;
        }
        //! Replace each element with its own square root
        void sqrt_inplace() { for (auto& i : *this) { i = static_cast<S>(std::sqrt (i)); } }

        /*!
         * Compute the element-wise square of the vector
         *
         * \return a Vector whose elements have been squared
         */
        Vector<S, N> sq() const
        {
            Vector<S, N> rtn;
            auto sq_element = [](S coord) -> S { return (coord * coord); };
            std::transform (this->begin(), this->end(), rtn.begin(), sq_element);
            return rtn;
        }
        //! Replace each element with its own square
        void sq_inplace() { for (auto& i : *this) { i = (i*i); } }

        /*!
         * Compute the element-wise natural log of the vector
         *
         * \return a Vector whose elements have been logged
         */
        Vector<S, N> log() const
        {
            Vector<S, N> rtn;
            auto log_element = [](S coord) -> S { return static_cast<S>(std::log(coord)); };
            std::transform (this->begin(), this->end(), rtn.begin(), log_element);
            return rtn;
        }
        //! Replace each element with its own natural log
        void log_inplace() { for (auto& i : *this) { i = static_cast<S>(std::log(i)); } }

        /*!
         * Compute the element-wise log to base 10 of the vector
         *
         * \return a Vector whose elements have been logged
         */
        Vector<S, N> log10() const
        {
            Vector<S, N> rtn;
            auto log_element = [](S coord) -> S { return static_cast<S>(std::log10(coord)); };
            std::transform (this->begin(), this->end(), rtn.begin(), log_element);
            return rtn;
        }
        //! Replace each element with its own log to base 10
        void log10_inplace() { for (auto& i : *this) { i = static_cast<S>(std::log10(i)); } }

        /*!
         * Compute the element-wise natural exponential of the vector
         *
         * \return a Vector whose elements have been exponentiated
         */
        Vector<S, N> exp() const
        {
            Vector<S, N> rtn;
            auto exp_element = [](S coord) -> S { return static_cast<S>(std::exp(coord)); };
            std::transform (this->begin(), this->end(), rtn.begin(), exp_element);
            return rtn;
        }
        //! Replace each element with its own natural exponential
        void exp_inplace() { for (auto& i : *this) { i = static_cast<S>(std::exp(i)); } }

        /*!
         * Compute the element-wise absolute values of the vector
         *
         * \return a Vector of the absolute values of *this
         */
        Vector<S, N> abs() const
        {
            Vector<S, N> rtn;
            auto abs_element = [](S coord) -> S { return static_cast<S>(std::abs(coord)); };
            std::transform (this->begin(), this->end(), rtn.begin(), abs_element);
            return rtn;
        }
        //! Replace each element with its own absolute value
        void abs_inplace() { for (auto& i : *this) { i = static_cast<S>(std::abs(i)); } }

        //! Less than a scalar. Return true if every element is less than the scalar
        bool operator<(const S rhs) const
        {
            auto _element_fails = [rhs](S a, S b) -> S { return a  (b < rhs ? S{0} : S{1}); };
            return std::accumulate (this->begin(), this->end(), S{0}, _element_fails) == S{0} ? true : false;
        }

        //! <= a scalar. Return true if every element is less than the scalar
        bool operator<=(const S rhs) const
        {
            auto _element_fails = [rhs](S a, S b) -> S { return a + (b <= rhs ? S{0} : S{1}); };
            return std::accumulate (this->begin(), this->end(), S{0}, _element_fails) == S{0} ? true : false;
        }

        //! Greater than a scalar. Return true if every element is gtr than the scalar
        bool operator>(const S rhs) const
        {
            auto _element_fails = [rhs](S a, S b) { return a + (b > rhs ? S{0} : S{1}); };
            return std::accumulate (this->begin(), this->end(), S{0}, _element_fails) == S{0} ? true : false;
        }

        //! >= a scalar. Return true if every element is gtr than the scalar
        bool operator>=(const S rhs) const
        {
            auto _element_fails = [rhs](S a, S b) { return a + (b >= rhs ? S{0} : S{1}); };
            return std::accumulate (this->begin(), this->end(), S{0}, _element_fails) == S{0} ? true : false;
        }

        /*!
         * Use something like this as a compare function when storing morph::Vectors in
         * a std::set:
         *
         *    auto _cmp = [](Vector<float, 3> a, Vector<float, 3> b) { return a.lexical_lessthan(b); };
         *    std::set<Vector<float, 3>, decltype(_cmp)> aset(_cmp);
         *
         * The default comparison for std::set is the operator<. The definition here
         * applied to !comp(a,b) && !comp(b,a) will suggest that two different Vectors
         * are equal even when they're not and so your std::sets will fail to insert
         * unique Vectors
         */
        template<typename _S=S>
        bool lexical_lessthan (const Vector<_S, N>& rhs) const
        {
            return std::lexicographical_compare (this->begin(), this->end(), rhs.begin(), rhs.end());
        }

        //! Another way to compare vectors would be by length.
        template<typename _S=S>
        bool length_lessthan (const Vector<_S, N>& rhs) const { return this->length() < rhs.length(); }

        //! Return true if each element of *this is less than its counterpart in rhs.
        template<typename _S=S>
        bool operator< (const Vector<_S, N>& rhs) const
        {
            auto ri = rhs.begin();
            auto _element_fails = [ri](S a, S b) mutable { return a + (b < (*ri++) ? S{0} : S{1}); };
            return std::accumulate (this->begin(), this->end(), S{0}, _element_fails) == S{0} ? true : false;
        }

        //! Return true if each element of *this is <= its counterpart in rhs.
        template<typename _S=S>
        bool operator<= (const Vector<_S, N>& rhs) const
        {
            auto ri = rhs.begin();
            auto _element_fails = [ri](S a, S b) mutable { return a + (b <= (*ri++) ? S{0} : S{1}); };
            return std::accumulate (this->begin(), this->end(), S{0}, _element_fails) == S{0} ? true : false;
        }

        //! Return true if each element of *this is greater than its counterpart in rhs.
        template<typename _S=S>
        bool operator> (const Vector<_S, N>& rhs) const
        {
            auto ri = rhs.begin();
            auto _element_fails = [ri](S a, S b) mutable { return a + (b > (*ri++) ? S{0} : S{1}); };
            return std::accumulate (this->begin(), this->end(), S{0}, _element_fails) == S{0} ? true : false;
        }

        //! Return true if each element of *this is >= its counterpart in rhs.
        template<typename _S=S>
        bool operator>= (const Vector<_S, N>& rhs) const
        {
            auto ri = rhs.begin();
            auto _element_fails = [ri](S a, S b) mutable { return a + (b >= (*ri++) ? S{0} : S{1}); };
            return std::accumulate (this->begin(), this->end(), S{0}, _element_fails) == S{0} ? true : false;
        }

        /*!
         * Unary negate operator
         *
         * \return a Vector whose elements have been negated.
         */
        Vector<S, N> operator-() const
        {
            Vector<S, N> rtn;
            std::transform (this->begin(), this->end(), rtn.begin(), std::negate<S>());
            return rtn;
        }

        /*!
         * Unary not operator.
         *
         * \return true if the vector length is 0, otherwise it returns false.
         */
        bool operator!() const { return (this->length() == S{0}) ? true : false; }

        /*!
         * \brief Scalar (dot) product
         *
         * Compute the scalar product of this Vector and the Vector, v.
         *
         * \return scalar product
         */
        template<typename _S=S>
        S dot (const Vector<_S, N>& v) const
        {
            auto vi = v.begin();
            auto dot_product = [vi](S a, _S b) mutable { return a + b * (*vi++); };
            const S rtn = std::accumulate (this->begin(), this->end(), S{0}, dot_product);
            return rtn;
        }

        /*!
         * Vector cross product.
         *
         * Cross product of this with another vector \a v (if N==3). In
         * higher dimensions, its more complicated to define what the cross product is,
         * and I'm unlikely to need anything other than the plain old 3D cross product.
         */
        template <typename _S=S, size_t _N = N, std::enable_if_t<(_N==3), int> = 0>
        Vector<S, _N> cross (const Vector<_S, _N>& v) const
        {
            Vector<S, _N> vrtn;
            vrtn[0] = (*this)[1] * v.z() - (*this)[2] * v.y();
            vrtn[1] = (*this)[2] * v.x() - (*this)[0] * v.z();
            vrtn[2] = (*this)[0] * v.y() - (*this)[1] * v.x();
            return vrtn;
        }

        //! Define a 2D cross product, v x w to be v_x w_y - v_y w_x.
        template <typename _S=S, size_t _N = N, std::enable_if_t<(_N==2), int> = 0>
        S cross (const Vector<_S, _N>& w) const
        {
            S rtn = (*this)[0] * w.y() - (*this)[1] * w.x();
            return rtn;
        }

        /*!
         * Two dimensional angle in radians (only for N=2)
         */
        template <typename _S=S, size_t _N = N, std::enable_if_t<(_N==2), int> = 0>
        S angle (const Vector<_S, _N>& v) const
        {
            S _angle = std::atan2 ((*this)[1], (*this)[0]);
            return _angle;
        }

        /*!
         * operator* gives the Hadamard product.
         *
         * Hadamard product - elementwise multiplication. If the vectors are of
         * differing lengths, then an exception is thrown.
         *
         * \return Hadamard product of left hand size (*this) and right hand size (\a v)
         */
        template<typename _S=S>
        Vector<S, N> operator* (const Vector<_S, N>& v) const
        {
            Vector<S, N> rtn;
            auto vi = v.begin();
            auto mult_by_s = [vi](S lhs) mutable -> S { return lhs * static_cast<S>(*vi++); };
            std::transform (this->begin(), this->end(), rtn.begin(), mult_by_s);
            return rtn;
        }

        /*!
         * Vector multiply *= operator.
         *
         * Hadamard product. Multiply *this vector with \a v, elementwise.
         */
        template <typename _S=S>
        void operator*= (const Vector<_S, N>& v)
        {
            auto vi = v.begin();
            auto mult_by_s = [vi](S lhs) mutable -> S { return lhs * static_cast<S>(*vi++); };
            std::transform (this->begin(), this->end(), this->begin(), mult_by_s);
        }

        /*!
         * Scalar multiply * operator
         *
         * This function will only be defined if typename _S is a
         * scalar type. Multiplies this Vector<S, N> by s, element-wise.
         */
        template <typename _S=S, std::enable_if_t<std::is_scalar<std::decay_t<_S>>::value, int> = 0 >
        Vector<S, N> operator* (const _S& s) const
        {
            Vector<S, N> rtn;
            auto mult_by_s = [s](S coord) { return coord * s; };
            std::transform (this->begin(), this->end(), rtn.begin(), mult_by_s);
            return rtn;
        }

        /*!
         * Scalar multiply *= operator
         *
         * This function will only be defined if typename _S is a
         * scalar type. Multiplies this Vector<S, N> by s, element-wise.
         */
        template <typename _S=S, std::enable_if_t<std::is_scalar<std::decay_t<_S>>::value, int> = 0 >
        void operator*= (const _S& s)
        {
            auto mult_by_s = [s](S coord) { return coord * s; };
            std::transform (this->begin(), this->end(), this->begin(), mult_by_s);
        }

        /*!
         * operator/ gives a 'Hadamard' division - elementwise division.
         *
         * If the vectors are of differing lengths, then an exception is thrown.
         *
         * \return elementwise division of left hand size (*this) by right hand size (\a v)
         */
        template<typename _S=S>
        Vector<S, N> operator/ (const Vector<_S, N>& v) const
        {
            Vector<S, N> rtn;
            std::transform (this->begin(), this->end(), v.begin(), rtn.begin(), std::divides<S>());
            return rtn;
        }

        /*!
         * Vector division /= operator.
         *
         * Element by element division. Divide *this vector by \a v, elementwise.
         */
        template <typename _S=S>
        void operator/= (const Vector<_S, N>& v)
        {
            std::transform (this->begin(), this->end(), v.begin(), this->begin(), std::divides<S>());
        }

        //! Scalar divide by s
        template <typename _S=S, std::enable_if_t<std::is_scalar<std::decay_t<_S>>::value, int> = 0 >
        Vector<S, N> operator/ (const _S& s) const
        {
            Vector<S, N> rtn;
            auto div_by_s = [s](S coord) { return coord / s; };
            std::transform (this->begin(), this->end(), rtn.begin(), div_by_s);
            return rtn;
        }

        //! Scalar divide by s
        template <typename _S=S, std::enable_if_t<std::is_scalar<std::decay_t<_S>>::value, int> = 0 >
        void operator/= (const _S& s)
        {
            auto div_by_s = [s](S coord) { return coord / s; };
            std::transform (this->begin(), this->end(), this->begin(), div_by_s);
        }

        //! Vector addition operator
        template<typename _S=S>
        Vector<S, N> operator+ (const Vector<_S, N>& v) const
        {
            Vector<S, N> vrtn;
            auto vi = v.begin();
            auto add_v = [vi](S a) mutable { return a + (*vi++); };
            std::transform (this->begin(), this->end(), vrtn.begin(), add_v);
            return vrtn;
        }

        //! Vector addition operator
        template<typename _S=S>
        void operator+= (const Vector<_S, N>& v)
        {
            auto vi = v.begin();
            auto add_v = [vi](S a) mutable { return a + (*vi++); };
            std::transform (this->begin(), this->end(), this->begin(), add_v);
        }

        //! Vector subtraction operator
        template<typename _S=S>
        Vector<S, N> operator- (const Vector<_S, N>& v) const
        {
            Vector<S, N> vrtn;
            auto vi = v.begin();
            auto subtract_v = [vi](S a) mutable { return a - (*vi++); };
            std::transform (this->begin(), this->end(), vrtn.begin(), subtract_v);
            return vrtn;
        }

        //! Vector subtraction operator
        template<typename _S=S>
        void operator-= (const Vector<_S, N>& v)
        {
            auto vi = v.begin();
            auto subtract_v = [vi](S a) mutable { return a - (*vi++); };
            std::transform (this->begin(), this->end(), this->begin(), subtract_v);
        }

        //! Scalar addition
        template <typename _S=S, std::enable_if_t<std::is_scalar<std::decay_t<_S>>::value, int> = 0 >
        Vector<S, N> operator+ (const _S& s) const
        {
            Vector<S, N> rtn;
            auto add_s = [s](S coord) { return coord + s; };
            std::transform (this->begin(), this->end(), rtn.begin(), add_s);
            return rtn;
        }

        //! Scalar addition
        template <typename _S=S, std::enable_if_t<std::is_scalar<std::decay_t<_S>>::value, int> = 0 >
        void operator+= (const _S& s)
        {
            auto add_s = [s](S coord) { return coord + s; };
            std::transform (this->begin(), this->end(), this->begin(), add_s);
        }

        //! Scalar subtraction
        template <typename _S=S, std::enable_if_t<std::is_scalar<std::decay_t<_S>>::value, int> = 0 >
        Vector<S, N> operator- (const _S& s) const
        {
            Vector<S, N> rtn;
            auto subtract_s = [s](S coord) { return coord - s; };
            std::transform (this->begin(), this->end(), rtn.begin(), subtract_s);
            return rtn;
        }

        //! Scalar subtraction
        template <typename _S=S, std::enable_if_t<std::is_scalar<std::decay_t<_S>>::value, int> = 0 >
        void operator-= (const _S& s)
        {
            auto subtract_s = [s](S coord) { return coord - s; };
            std::transform (this->begin(), this->end(), this->begin(), subtract_s);
        }

        //! Addition which should work for any member type that implements the + operator
        Vector<S, N> operator+ (const S& s) const
        {
            Vector<S, N> rtn;
            auto add_s = [s](S coord) { return coord + s; };
            std::transform (this->begin(), this->end(), rtn.begin(), add_s);
            return rtn;
        }

        //! Addition += operator for any type same as the enclosed type that implements + op
        void operator+= (const S& s) const
        {
            auto add_s = [s](S coord) { return coord + s; };
            std::transform (this->begin(), this->end(), this->begin(), add_s);
        }

        //! Subtraction which should work for any member type that implements the - operator
        Vector<S, N> operator- (const S& s) const
        {
            Vector<S, N> rtn;
            auto subtract_s = [s](S coord) { return coord - s; };
            std::transform (this->begin(), this->end(), rtn.begin(), subtract_s);
            return rtn;
        }

        //! Subtraction -= operator for any time same as the enclosed type that implements - op
        void operator-= (const S& s) const
        {
            auto subtract_s = [s](S coord) { return coord - s; };
            std::transform (this->begin(), this->end(), this->begin(), subtract_s);
        }

        //! Overload the stream output operator
        friend std::ostream& operator<< <S, N> (std::ostream& os, const Vector<S, N>& v);
    };

    template <typename S=float, size_t N=3>
    std::ostream& operator<< (std::ostream& os, const Vector<S, N>& v)
    {
        os << v.str();
        return os;
    }

    // Operators that can do premultiply, predivide by scaler so you could do,
    // e.g. Vector<float> denom = {1,2,3}; Vector<float> result = float(1) / denom;

    //! Scalar * Vector<> (commutative; lhs * rhs == rhs * lhs, so return rhs * lhs)
    template <typename S, size_t N> Vector<S, N> operator* (S lhs, const Vector<S, N>& rhs) { return rhs * lhs; }

    //! Scalar / Vector<>
    template <typename S, size_t N>
    Vector<S, N> operator/ (S lhs, const Vector<S, N>& rhs)
    {
        Vector<S, N> division;
        auto lhs_div_by_vec = [lhs](S coord) { return lhs / coord; };
        std::transform (rhs.begin(), rhs.end(), division.begin(), lhs_div_by_vec);
        return division;
    }

    //! Scalar + Vector<> (commutative)
    template <typename S, size_t N> Vector<S, N> operator+ (S lhs, const Vector<S, N>& rhs) { return rhs + lhs; }

    //! Scalar - Vector<>
    template <typename S, size_t N>
    Vector<S, N> operator- (S lhs, const Vector<S, N>& rhs)
    {
        Vector<S, N> subtraction;
        auto lhs_minus_vec = [lhs](S coord) { return lhs - coord; };
        std::transform (rhs.begin(), rhs.end(), subtraction.begin(), lhs_minus_vec);
        return subtraction;
    }

} // namespace morph
