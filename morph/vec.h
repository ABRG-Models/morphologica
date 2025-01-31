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
#include <cstddef>
#include <morph/Random.h>
#include <morph/range.h>

namespace morph {

    /*!
     * \brief N-D vector class
     *
     * An N dimensional vector class template which derives from std::array. vec
     * components are of scalar type S. It is anticipated that S will be set either to
     * floating point scalar types such as float or double, or to integer scalar types
     * such as int, long long int and so on. Thus, a typical (and in fact, the default)
     * signature would be:
     *
     * vec<float, 3> v;
     *
     * The class inherits std:array's fixed-size array of memory for storing the
     * components of the vector. It adds numerous methods which allow objects of type
     * vec to have arithmetic operations applied to them, either scalar (add a scalar
     * to all elements; divide all elements by a scalar, etc) or vector (including dot
     * and cross products, normalization and so on.
     *
     * Because morph::vec extends std::array, it works best when compiled with a
     * c++-17 compiler (although it can be compiled with a c++-11 compiler). This is
     * because std::array is an 'aggregate class' with no user-provided constructors,
     * and morph::vec does not add any of its own constructors. Prior to c++-17,
     * aggregate classes were not permitted to have base classes. So, if you want to do:
     *
     * vec<float, 3> v = { 1.0f , 1.0f, 1.0f };
     *
     * You need c++-17. Otherwise, restrict your client code to doing:
     *
     * vec<float, 3> v;
     * v[0] = 1.0f; v[1] = 1.0f; v[2] = 1.0f;
     */
    template <typename S, std::size_t N> struct vec;

    /*!
     * Template friendly mechanism to overload the stream operator.
     *
     * Note forward declaration of the vec template class and this template for
     * stream operator overloading. Example adapted from
     * https://stackoverflow.com/questions/4660123
     */
    template <typename S, std::size_t N> std::ostream& operator<< (std::ostream&, const vec<S, N>&);

    template <typename S=float, std::size_t N=3>
    struct vec : public std::array<S, N>
    {
        //! \return the first component of the vector
        template <std::size_t _N = N, std::enable_if_t<(_N>0), int> = 0>
        constexpr S x() const { return (*this)[0]; }
        //! \return the second component of the vector
        template <std::size_t _N = N, std::enable_if_t<(_N>1), int> = 0>
        constexpr S y() const { return (*this)[1]; }
        //! \return the third component of the vector
        template <std::size_t _N = N, std::enable_if_t<(_N>2), int> = 0>
        constexpr S z() const { return (*this)[2]; }
        //! \return the fourth component of the vector
        template <std::size_t _N = N, std::enable_if_t<(_N>3), int> = 0>
        constexpr S w() const { return (*this)[3]; }

        //! Set data members from an std::vector
        template <typename _S=S>
        void set_from (const std::vector<_S>& vec)
        {
            if (vec.size() != N) {
                throw std::runtime_error ("vec::set_from(): Ensure vector sizes match");
            }
            std::copy (vec.begin(), vec.end(), this->begin());
        }

        //! Set data members from an array the of same size and type.
        template <typename _S=S>
        constexpr void set_from (const std::array<_S, N>& ar)
        {
            std::copy (ar.begin(), ar.end(), this->begin());
        }

        /*!
         * Set the data members of this vec from the passed in, larger array, \a ar,
         * ignoring the last element of \a ar. Used when working with 4D vectors in
         * graphics applications involving 4x4 transform matrices.
         */
        template <typename _S=S>
        constexpr void set_from (const std::array<_S, (N+1)>& ar)
        {
            // Don't use std::copy here, because ar has more elements than *this.
            for (std::size_t i = 0; i < N; ++i) { (*this)[i] = ar[i]; }
        }

        /*!
         * Set the data members of this vec from the passed in, smaller array, \a ar,
         * ignoring the last element of this vector (which is set to 0). Used when
         * working with 2D vectors that you want to visualise in a 3D environment with
         * z set to 0.
         */
        template <typename _S=S>
        constexpr void set_from (const std::array<_S, (N-1)>& ar)
        {
            // Don't use std::copy here, because ar has more elements than *this.
            for (std::size_t i = 0; i < N-1; ++i) { (*this)[i] = ar[i]; }
            (*this)[N-1] = S{0};
        }

        /*!
         * Set an N D vec from an N+1 D vec. Intended to convert 4D vectors (that
         * have been operated on by 4x4 matrices) into 3D vectors.
         */
        template <typename _S=S>
        constexpr void set_from (const vec<_S, (N+1)>& v)
        {
            for (std::size_t i = 0; i < N; ++i) { (*this)[i] = v[i]; }
        }

        //! Set an N D vec from an (N-1) D vec.
        template <typename _S=S>
        constexpr void set_from (const vec<_S, (N-1)>& v)
        {
            for (std::size_t i = 0; i < N-1; ++i) { (*this)[i] = v[i]; }
            (*this)[N-1] = S{0};
        }

        //! Set all elements from the value type v
        template <typename _S=S>
        constexpr void set_from (const _S& v) { std::fill (this->begin(), this->end(), v); }

        /*!
         * Set a linear sequence into the vector from value start to value stop. Uses
         * the vec's size to determine how many values to create. You *can* use this
         * with integer types, but be prepared to notice strange rounding errors.
         */
        template <typename _S=S, typename _S2=S>
        constexpr void linspace (const _S start, const _S2 stop)
        {
            S increment = (static_cast<S>(stop) - static_cast<S>(start)) / (N-1);
            for (std::size_t i = 0; i < this->size(); ++i) { (*this)[i] = start + increment * i; }
        }

        /*!
         * Similar to numpy's arange. Set a linear sequence from start to stop with the given step
         * size. If this leads to too many elements to fit in this vec, simply stop when it's
         * full. If too few, then the rest will be 0.
         */
        template <typename _S=S, typename _S2=S>
        constexpr void arange (const _S start, const _S2 stop, const _S2 increment)
        {
            this->zero();
            // Figure out how many elements given the increment:
            S num = std::ceil((stop - start) / increment);
            if (num > S{0}) {
                for (std::size_t i = 0; i < static_cast<std::size_t>(num) && i < N; ++i) {
                    (*this)[i] = start + increment*static_cast<S>(i);
                }
            } // else vector will now be full of zeros, not quite like Python does it (it returns an
              // empty array with no elements)
        }

        //! Return a vector with one less dimension - losing the last one.
        constexpr vec<S, N-1> less_one_dim () const
        {
            vec<S, N-1> rtn;
            for (std::size_t i = 0; i < N-1; ++i) { rtn[i] = (*this)[i]; }
            return rtn;
        }

        //! Return a vector with one additional dimension - setting it to 0.
        constexpr vec<S, N+1> plus_one_dim () const
        {
            vec<S, N+1> rtn;
            for (std::size_t i = 0; i < N; ++i) { rtn[i] = (*this)[i]; }
            rtn[N] = S{0};
            return rtn;
        }

        //! Return a vector with one additional dimension - setting it to val.
        constexpr vec<S, N+1> plus_one_dim (const S val) const
        {
            vec<S, N+1> rtn;
            for (std::size_t i = 0; i < N; ++i) { rtn[i] = (*this)[i]; }
            rtn[N] = val;
            return rtn;
        }

        //! Return a vec that contains the elements of this vec in type \tparam T
        template<typename T>
        constexpr vec<T, N> as() const
        {
            vec<T, N> v = { T{0} };
            v += *this;
            return v;
        }

        //! Return this vec in single precision, float format
        constexpr vec<float, N> as_float() const { return this->as<float>(); }

        //! Return this vec in double precision, float format
        constexpr vec<double, N> as_double() const { return this->as<double>(); }

        //! Return this vec in single precision, int format
        constexpr vec<int, N> as_int() const { return this->as<int>(); }

        //! Return this vec in single precision, unsigned int format
        constexpr vec<unsigned int, N> as_uint() const { return this->as<unsigned int>(); }

        //! Stream the elements of the vector into \a ss as a comma separated list.
        void str_comma_separated (std::stringstream& ss, const char sep = ',') const
        {
            ss << std::setprecision (std::numeric_limits<S>::max_digits10);
            bool first = true;
            for (auto i : *this) {
                if (first) {
                    first = false;
                } else {
                    ss << sep;
                }
                if constexpr (std::is_same<S, unsigned char>::value == true || std::is_same<S, char>::value == true) {
                    ss << static_cast<int>(i);
                } else { ss << i; }
            }
        }

        std::string str_comma_separated (const char sep = ',') const
        {
            std::stringstream ss;
            this->str_comma_separated (ss, sep);
            return ss.str();
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

        //! Output in a form that can be used as an initializer list in C++
        std::string str_initializer() const
        {
            std::stringstream ss;
            ss << "{";
            this->str_comma_separated (ss);
            ss << "}";
            return ss.str();
        }

        //! Renormalize the vector to length 1.0. Only for S types that are floating point.
        template <typename _S=S, std::enable_if_t<!std::is_integral<std::decay_t<_S>>::value, int> = 0 >
        constexpr void renormalize()
        {
            auto add_squared = [](_S a, _S b) { return a + b * b; };
            const _S denom = std::sqrt (std::accumulate (this->begin(), this->end(), _S{0}, add_squared));
            if (denom != _S{0}) {
                _S oneovermag = _S{1} / denom;
                auto x_oneovermag = [oneovermag](_S f) { return f * oneovermag; };
                std::transform (this->begin(), this->end(), this->begin(), x_oneovermag);
            }
        }

        //! Rescale the vector elements so that they all lie in the range 0-1. NOT the same as renormalize.
        template <typename _S=S, std::enable_if_t<!std::is_integral<std::decay_t<_S>>::value, int> = 0 >
        constexpr void rescale()
        {
            morph::range<_S> r = this->range();
            _S m = r.max - r.min;
            _S g = r.min;
            auto rescale_op = [m, g](_S f) { return (f - g)/m; };
            std::transform (this->begin(), this->end(), this->begin(), rescale_op);
        }

        //! Rescale the vector elements so that they all lie in the range -1 to 0.
        template <typename _S=S, std::enable_if_t<!std::is_integral<std::decay_t<_S>>::value, int> = 0 >
        constexpr void rescale_neg()
        {
            morph::range<_S> r = this->range();
            _S m = r.max - r.min;
            _S g = r.max;
            auto rescale_op = [m, g](_S f) { return (f - g)/m; };
            std::transform (this->begin(), this->end(), this->begin(), rescale_op);
        }

        //! Rescale the vector elements symetrically about 0 so that they all lie in the range -1 to 1.
        template <typename _S=S, std::enable_if_t<!std::is_integral<std::decay_t<_S>>::value, int> = 0 >
        constexpr void rescale_sym()
        {
            morph::range<_S> r = this->range();
            _S m = (r.max - r.min) / _S{2};
            _S g = (r.max + r.min) / _S{2};
            auto rescale_op = [m, g](_S f) { return (f - g)/m; };
            std::transform (this->begin(), this->end(), this->begin(), rescale_op);
        }

        /*!
         * Permute the elements in a rotation. 0->N-1, 1->0, 2->1, etc. Useful for
         * swapping x and y in a 2D vec.
         */
        constexpr void rotate()
        {
            if constexpr (N>1) {
                S z_el = (*this)[0];
                for (std::size_t i = 1; i < N; ++i) {
                    (*this)[i-1] = (*this)[i];
                }
                (*this)[N-1] = z_el;
            } // else no op
        }

        //! Templated rotate for integral types T
        template <typename T=int>
        constexpr void rotate (T n)
        {
            static_assert (std::numeric_limits<T>::is_integer);

            n %= static_cast<T>(N);

            auto _start = this->begin();
            if constexpr (std::numeric_limits<T>::is_signed) {
                std::size_t _n = n >= 0 ? n : N + n;
                std::advance (_start, _n);
            } else {
                std::advance (_start, n);
            }
            std::rotate (this->begin(), _start, this->end());
        }

        //! If N is even, permute pairs of elements in a rotation. 0->1, 1->0, 2->3, 3->2, etc.
        constexpr void rotate_pairs()
        {
            static_assert ((N%2==0), "N must be even to call morph::vec::rotate_pairs");
            S tmp_el = S{0};
            for (std::size_t i = 0; i < N; i+=2) {
                tmp_el = (*this)[i];
                (*this)[i] = (*this)[i+1];
                (*this)[i+1] = tmp_el;
            }
        }

        //! Zero the vector. Set all elements to 0
        constexpr void zero() { std::fill (this->begin(), this->end(), S{0}); }
        //! Set all elements of the vector to the maximum possible value given type S
        constexpr void set_max() { std::fill (this->begin(), this->end(), std::numeric_limits<S>::max()); }
        //! Set all elements of the vector to the lowest (i.e. most negative) possible value given type S
        constexpr void set_lowest() { std::fill (this->begin(), this->end(), std::numeric_limits<S>::lowest()); }

        /*!
         * Randomize the vector
         *
         * Randomly set the elements of the vector. Elements are set to random
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
         * Randomly set the elements of the vector. Elements are set to random
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
         * Randomly set the elements of the vector. Elements are set to random
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
        constexpr bool checkunit() const
        {
            /*!
             * \brief Unit vector threshold
             *
             * The threshold outside of which the vector is no longer considered to be a
             * unit vector. Note this is hard coded as a constexpr, to avoid messing with
             * the initialization of the vec with curly brace initialization.
             *
             * Clearly, this will be the wrong threshold for some cases. Possibly, a
             * template parameter could set this; so std::size_t U could indicate the threshold;
             * 0.001 could be U=-3 (10^-3).
             *
             * Another idea would be to change unitThresh based on the type S. Or use
             * numeric_limits<S>::epsilon and find out what multiple of epsilon would make
             * sense.
             */
            constexpr _S unitThresh = _S{0.001};

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
         * \return a value which is as close as possible to the length
         */
        template <typename _S=S>
        constexpr _S length() const
        {
            auto add_squared = [](_S a, S b) { return a + b * b; };
            // Add check on whether return type _S is integral or float. If integral, then std::round then cast the result of std::sqrt()
            if constexpr (std::is_integral<std::decay_t<_S>>::value == true) {
                return static_cast<_S>(std::round(std::sqrt(std::accumulate(this->begin(), this->end(), _S{0}, add_squared))));
            } else {
                return std::sqrt(std::accumulate(this->begin(), this->end(), _S{0}, add_squared));
            }
        }

        //! Reduce the length of the vector by the amount dl, if possible. If dl makes the vector
        //! have a negative length, then return a null vector
        template <typename _S=S, std::enable_if_t<!std::is_integral<std::decay_t<_S>>::value, int> = 0 >
        constexpr vec<S, N> shorten (const S dl) const
        {
            vec<S, N> v = *this;
            S newlen = this->length() - dl;
            if (newlen <= S{0}) {
                v.zero();
            } else {
                v *= newlen/this->length();
            }
            return v;
        }

        //! Increase the length of the vector by the amount dl, if possible. If dl makes the vector
        //! have a negative length, then return a null vector
        template <typename _S=S, std::enable_if_t<!std::is_integral<std::decay_t<_S>>::value, int> = 0 >
        constexpr vec<S, N> lengthen (const S dl) const
        {
            vec<S, N> v = *this;
            S newlen = this->length() + dl;
            if (newlen <= S{0}) { // dl could be negative, so still need to test new length of vector
                v.zero();
            } else {
                v *= newlen/this->length();
            }
            return v;
        }

        /*!
         * Find the squared length of the vector which is the same as the sum of squared
         * elements, if elements are scalar.
         *
         * If S typed elements are morph::vecs or morph::vvecs, then return the sum of the squares
         * of the lengths of the elements in the return _S type, which you will have to ensure to be
         * a scalar.
         */
        template <typename _S=S>
        constexpr _S length_sq() const
        {
            _S _sos = _S{0};
            if constexpr (std::is_scalar<std::decay_t<S>>::value) {
                if constexpr (std::is_scalar<std::decay_t<_S>>::value) {
                    // Return type is also scalar
                    _sos = this->sos<_S>();
                } else {
                    // Return type is a vector. Too weird.
                    //[]<bool flag = false>() { static_assert(flag, "Won't compute sum of squared scalar elements into a vector type"); }();
                    throw std::runtime_error ("Won't compute sum of squared scalar elements into a vector type");
                }
            } else {
                // S is a vector so i is a vector.
                if constexpr (std::is_scalar<std::decay_t<_S>>::value) {
                    // Return type _S is a scalar
                    for (auto& i : *this) { _sos += i.template sos<_S>(); }
                } else {
                    // Return type _S is also a vector, place result in 0th element? No, can now use vvec<vec<float>>::sos<float>()
                    throw std::runtime_error ("Won't compute sum of squared vector lengths into a vector type");
                    //[]<bool flag = false>() { static_assert(flag, "Won't compute sum of squared vector length elements into a vector type"); }();
                }
            }
            return _sos;
        }

        /*!
         * Return the sum of the squares of the elements.
         *
         * If S is a vector type, then the result will be a vector type containing the
         * sos of all elements i - that is, element 0 of the returned vector will contain
         *  the sum of squares of element 0 of all members of *this.
         *
         * \return the length squared
         */
        template <typename _S=S>
        constexpr _S sos() const
        {
            auto add_squared = [](_S a, S b) { return a + b * b; };
            return std::accumulate (this->begin(), this->end(), _S{0}, add_squared);
        }

        //! Return the value of the longest component of the vector.
        constexpr S longest() const
        {
            auto abs_compare = [](S a, S b) { return (std::abs(a) < std::abs(b)); };
            auto thelongest = std::max_element (this->begin(), this->end(), abs_compare);
            S rtn = *thelongest;
            return rtn;
        }

        //! Return the index of the longest component of the vector.
        constexpr std::size_t arglongest() const
        {
            std::size_t idx = 0;
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
        constexpr S shortest() const
        {
            auto abs_compare = [](S a, S b) { return (std::abs(a) > std::abs(b)); };
            auto theshortest = std::max_element (this->begin(), this->end(), abs_compare);
            S rtn = *theshortest;
            return rtn;
        }

        //! Return the index of the shortest component of the vector.
        constexpr std::size_t argshortest() const
        {
            std::size_t idx = 0;
            // Check on the type S. If S is a vec thing, then abs_compare needs to be different.
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
        constexpr S max() const
        {
            auto themax = std::max_element (this->begin(), this->end());
            S rtn = *themax;
            return rtn;
        }

        //! Return the index of the maximum (most positive) component of the vector.
        constexpr std::size_t argmax() const
        {
            auto themax = std::max_element (this->begin(), this->end());
            std::size_t idx = (themax - this->begin());
            return idx;
        }

        //! Return the value of the minimum (smallest or most negative) component of the vector.
        constexpr S min() const
        {
            auto themin = std::min_element (this->begin(), this->end());
            S rtn = *themin;
            return rtn;
        }

        //! Return the index of the minimum (smallest or most negative) component of the vector.
        constexpr std::size_t argmin() const
        {
            auto themin = std::min_element (this->begin(), this->end());
            std::size_t idx = (themin - this->begin());
            return idx;
        }

        //! Return the range of the vec (the min and max values of the vec)
        constexpr morph::range<S> range() const
        {
            auto mm = std::minmax_element (this->begin(), this->end());
            return morph::range<S>(*mm.first, *mm.second);
        }

        //! Return true if any element is zero
        constexpr bool has_zero() const
        {
            return std::any_of (this->cbegin(), this->cend(), [](S i){ return i == S{0}; });
        }

        //! Return true if any element is infinity
        constexpr bool has_inf() const
        {
            if constexpr (std::numeric_limits<S>::has_infinity) {
                return std::any_of (this->cbegin(), this->cend(), [](S i){return std::isinf(i);});
            } else {
                return false;
            }
        }

        //! Return true if any element is NaN
        constexpr bool has_nan() const
        {
            if constexpr (std::numeric_limits<S>::has_quiet_NaN
                          || std::numeric_limits<S>::has_signaling_NaN) {
                return std::any_of (this->cbegin(), this->cend(), [](S i){return std::isnan(i);});
            } else {
                return false;
            }
        }

        //! Return true if any element is NaN or infinity
        constexpr bool has_nan_or_inf() const
        {
            bool has_nan_or_inf = false;
            has_nan_or_inf = this->has_nan();
            return has_nan_or_inf ? has_nan_or_inf : this->has_inf();
        }

        constexpr void replace_nan_with (const S replacement)
        {
            static_assert (std::numeric_limits<S>::has_quiet_NaN, "S does not have quiet_NaNs");
            for (auto& i : *this) { if (std::isnan(i)) { i = replacement; } }
        }

        constexpr void replace_nan_or_inf_with (const S replacement)
        {
            static_assert (std::numeric_limits<S>::has_quiet_NaN, "S does not have quiet_NaNs");
            static_assert (std::numeric_limits<S>::has_infinity, "S does not have infinities");
            for (auto& i : *this) { if (std::isnan(i) || std::isinf(i)) { i = replacement; } }
        }

        /*!
         * Considering the 3 element vec as RGB pixels, convert to greyscale using the
         * technique described in
         * https://docs.opencv.org/3.4/de/d25/imgproc_color_conversions.html
         *
         * Only enabled for arrays of size 3 (for now, RGBA vecs could also be considered)
         *
         * Only really makes sense for real types S/_S.
         */
        template <typename _S=S, std::size_t _N = N, std::enable_if_t<(_N==3), int> = 0>
        constexpr _S rgb_to_grey() const
        {
            const _S grey = _S{0.299} * (*this)[0] + _S{0.587} * (*this)[1] + _S{0.114} * (*this)[2];
            return grey;
        }

        //! Return the arithmetic mean of the elements
        template<typename _S=S>
        constexpr _S mean() const
        {
            const _S sum = std::accumulate (this->begin(), this->end(), _S{0});
            return sum / this->size();
        }

        //! Return the variance of the elements
        template<typename _S=S>
        constexpr _S variance() const
        {
            if (this->empty()) { return S{0}; }
            _S _mean = this->mean<_S>();
            _S sos_deviations = _S{0};
            for (S val : *this) {
                sos_deviations += ((val-_mean)*(val-_mean));
            }
            _S variance = sos_deviations / (this->size()-1);
            return variance;
        }

        //! Return the standard deviation of the elements
        template<typename _S=S>
        constexpr _S std() const
        {
            if (this->empty()) { return _S{0}; }
            return std::sqrt (this->variance<_S>());
        }

        //! Return the sum of the elements
        template<typename _S=S>
        constexpr _S sum() const
        {
            return std::accumulate (this->begin(), this->end(), _S{0});
        }

        //! Return the product of the elements
        template<typename _S=S>
        constexpr _S product() const
        {
            auto _product = [](_S a, S b) mutable { return a ? a * b : b; };
            return std::accumulate (this->begin(), this->end(), _S{0}, _product);
        }

        /*!
         * Compute the element-wise pth power of the vector
         *
         * \return a vec whose elements have been raised to the power p
         */
        constexpr vec<S, N> pow (const S& p) const
        {
            vec<S, N> rtn{};
            auto raise_to_p = [p](S elmnt) -> S { return static_cast<S>(std::pow(elmnt, p)); };
            std::transform (this->begin(), this->end(), rtn.begin(), raise_to_p);
            return rtn;
        }
        //! Raise each element to the power p
        constexpr void pow_inplace (const S& p) { for (auto& i : *this) { i = static_cast<S>(std::pow (i, p)); } }

        //! Element-wise power
        template<typename _S=S>
        constexpr vec<S, N> pow (const vec<_S, N>& p) const
        {
            auto pi = p.begin();
            vec<S, N> rtn{};
            auto raise_to_p = [pi](S elmnt) mutable -> S { return static_cast<S>(std::pow(elmnt, (*pi++))); };
            std::transform (this->begin(), this->end(), rtn.begin(), raise_to_p);
            return rtn;
        }
        template<typename _S=S>
        void pow_inplace (const vec<_S, N>& p)
        {
            auto pi = p.begin();
            for (auto& i : *this) { i = static_cast<S>(std::pow (i, (*pi++))); }
        }

        //! Return the signum of the vec, with signum(0)==0
        constexpr vec<S, N> signum() const
        {
            vec<S, N> rtn{};
            auto _signum = [](S elmnt) -> S { return (elmnt > S{0} ? S{1} : (elmnt == S{0} ? S{0} : S{-1})); };
            std::transform (this->begin(), this->end(), rtn.begin(), _signum);
            return rtn;
        }
        constexpr void signum_inplace() { for (auto& i : *this) { i = (i > S{0} ? S{1} : (i == S{0} ? S{0} : S{-1})); } }

        //! Return the floor of the vec
        constexpr vec<S, N> floor() const
        {
            vec<S, N> rtn{};
            auto _floor = [](S elmnt) -> S { return (std::floor(elmnt)); };
            std::transform (this->begin(), this->end(), rtn.begin(), _floor);
            return rtn;
        }
        constexpr void floor_inplace() { for (auto& i : *this) { i = std::floor(i); } }

        //! Return the floor-or-ceiling of the vector's elements - i.e. apply std::trunc
        constexpr vec<S, N> trunc() const
        {
            vec<S, N> rtn{};
            auto _trunc = [](S elmnt) -> S { return (std::trunc(elmnt)); };
            std::transform (this->begin(), this->end(), rtn.begin(), _trunc);
            return rtn;
        }
        constexpr void trunc_inplace() { for (auto& i : *this) { i = std::trunc(i); } }

        //! Return the ceiling of the vec
        constexpr vec<S, N> ceil() const
        {
            vec<S, N> rtn{};
            auto _ceil = [](S elmnt) -> S { return (std::ceil(elmnt)); };
            std::transform (this->begin(), this->end(), rtn.begin(), _ceil);
            return rtn;
        }
        constexpr void ceil_inplace() { for (auto& i : *this) { i = std::ceil(i); } }

        /*!
         * Compute the element-wise square root of the vector
         *
         * \return a vec whose elements have been square-rooted
         */
        constexpr vec<S, N> sqrt() const
        {
            vec<S, N> rtn{};
            auto sqrt_element = [](S elmnt) -> S { return static_cast<S>(std::sqrt(elmnt)); };
            std::transform (this->begin(), this->end(), rtn.begin(), sqrt_element);
            return rtn;
        }
        //! Replace each element with its own square root
        constexpr void sqrt_inplace() { for (auto& i : *this) { i = static_cast<S>(std::sqrt (i)); } }

        /*!
         * Compute the element-wise square of the vector
         *
         * \return a vec whose elements have been squared
         */
        constexpr vec<S, N> sq() const
        {
            vec<S, N> rtn{};
            auto sq_element = [](S elmnt) -> S { return (elmnt * elmnt); };
            std::transform (this->begin(), this->end(), rtn.begin(), sq_element);
            return rtn;
        }
        //! Replace each element with its own square
        constexpr void sq_inplace() { for (auto& i : *this) { i = (i*i); } }

        /*!
         * Compute the element-wise natural log of the vector
         *
         * \return a vec whose elements have been logged
         */
        constexpr vec<S, N> log() const
        {
            vec<S, N> rtn{};
            auto log_element = [](S elmnt) -> S { return static_cast<S>(std::log(elmnt)); };
            std::transform (this->begin(), this->end(), rtn.begin(), log_element);
            return rtn;
        }
        //! Replace each element with its own natural log
        constexpr void log_inplace() { for (auto& i : *this) { i = static_cast<S>(std::log(i)); } }

        /*!
         * Compute the element-wise log to base 10 of the vector
         *
         * \return a vec whose elements have been logged
         */
        constexpr vec<S, N> log10() const
        {
            vec<S, N> rtn{};
            auto log_element = [](S elmnt) -> S { return static_cast<S>(std::log10(elmnt)); };
            std::transform (this->begin(), this->end(), rtn.begin(), log_element);
            return rtn;
        }
        //! Replace each element with its own log to base 10
        constexpr void log10_inplace() { for (auto& i : *this) { i = static_cast<S>(std::log10(i)); } }

        /*!
         * Compute the element-wise natural exponential of the vector
         *
         * \return a vec whose elements have been exponentiated
         */
        constexpr vec<S, N> exp() const
        {
            vec<S, N> rtn{};
            auto exp_element = [](S elmnt) -> S { return static_cast<S>(std::exp(elmnt)); };
            std::transform (this->begin(), this->end(), rtn.begin(), exp_element);
            return rtn;
        }
        //! Replace each element with its own natural exponential
        constexpr void exp_inplace() { for (auto& i : *this) { i = static_cast<S>(std::exp(i)); } }

        /*!
         * Compute the element-wise absolute values of the vector
         *
         * \return a vec of the absolute values of *this
         */
        constexpr vec<S, N> abs() const
        {
            vec<S, N> rtn{};
            auto abs_element = [](S elmnt) -> S { return static_cast<S>(std::abs(elmnt)); };
            std::transform (this->begin(), this->end(), rtn.begin(), abs_element);
            return rtn;
        }
        //! Replace each element with its own absolute value
        constexpr void abs_inplace() { for (auto& i : *this) { i = static_cast<S>(std::abs(i)); } }

        //! Less than a scalar. Return true if every element is less than the scalar
        constexpr bool operator<(const S rhs) const
        {
            auto _element_fails = [rhs](S a, S b) -> S { return a + (b < rhs ? S{0} : S{1}); };
            return std::accumulate (this->begin(), this->end(), S{0}, _element_fails) == S{0} ? true : false;
        }

        //! <= a scalar. Return true if every element is less than the scalar
        constexpr bool operator<=(const S rhs) const
        {
            auto _element_fails = [rhs](S a, S b) -> S { return a + (b <= rhs ? S{0} : S{1}); };
            return std::accumulate (this->begin(), this->end(), S{0}, _element_fails) == S{0} ? true : false;
        }

        //! Greater than a scalar. Return true if every element is gtr than the scalar
        constexpr bool operator>(const S rhs) const
        {
            auto _element_fails = [rhs](S a, S b) { return a + (b > rhs ? S{0} : S{1}); };
            return std::accumulate (this->begin(), this->end(), S{0}, _element_fails) == S{0} ? true : false;
        }

        //! >= a scalar. Return true if every element is gtr than the scalar
        constexpr bool operator>=(const S rhs) const
        {
            auto _element_fails = [rhs](S a, S b) { return a + (b >= rhs ? S{0} : S{1}); };
            return std::accumulate (this->begin(), this->end(), S{0}, _element_fails) == S{0} ? true : false;
        }

        /*!
         * Use something like this as a compare function when storing morph::vecs in
         * a std::set:
         *
         *    auto _cmp = [](vec<float, 3> a, vec<float, 3> b) { return a.lexical_lessthan(b); };
         *    std::set<vec<float, 3>, decltype(_cmp)> aset(_cmp);
         *
         * The default comparison for std::set is the operator<. The definition here
         * applied to !comp(a,b) && !comp(b,a) will suggest that two different vecs
         * are equal even when they're not and so your std::sets will fail to insert
         * unique vecs.
         *
         * This can also be an issue with using a morph::vec as a key to an std::map. So similarly
         * do something like this to create a map with a key type morph::vec<int, 2> and a value
         * type std::string:
         *
         *    auto _cmp = [](morph::vec<int,2> a, morph::vec<int,2> b){return a.lexical_lessthan(b);};
         *    std::map<morph::vec<int, 2>, std::string, decltype(_cmp)> themap(_cmp);
         */
        template<typename _S=S>
        bool lexical_lessthan (const vec<_S, N>& rhs) const
        {
            return std::lexicographical_compare (this->begin(), this->end(), rhs.begin(), rhs.end());
        }

        /*!
         * Like lexical_lessthan, but elements of vec must be less than by at least n_eps *
         * numeric_limits<_S>::epsilon() to be different. If *this is less than rhs on that basis,
         * return true.
         */
        template<typename _S=S>
        bool lexical_lessthan_beyond_epsilon (const vec<_S, N>& rhs, const int n_eps = 1) const
        {
            for (std::size_t i = 0; i < N; ++i) {
                const _S _this = (*this)[i];
                const _S _rhs = rhs[i];
                const _S eps = std::numeric_limits<_S>::epsilon() * n_eps;
                if ((_rhs - _this) > eps) {
                    // _rhs is properly less than _this, so _this is gtr than _rhs
                    return false;
                } else if ((_this - _rhs) > eps) {
                    // _rhs is properly greater than _this, so _this is less than _rhs
                    return true;
                } // else next element
            }
            return false;
        }

        //! Another way to compare vectors would be by length.
        template<typename _S=S>
        bool length_lessthan (const vec<_S, N>& rhs) const { return this->length() < rhs.length(); }

        //! Length less-than-or-equal
        template<typename _S=S>
        bool length_lte (const vec<_S, N>& rhs) const { return this->length() <= rhs.length(); }

        //! Length greater-than
        template<typename _S=S>
        bool length_gtrthan (const vec<_S, N>& rhs) const { return this->length() > rhs.length(); }

        //! Length greater-than-or-equal
        template<typename _S=S>
        bool length_gte (const vec<_S, N>& rhs) const { return this->length() >= rhs.length(); }

        //! Return true if each element of *this is less than its counterpart in rhs.
        template<typename _S=S>
        constexpr bool operator< (const vec<_S, N>& rhs) const
        {
            auto ri = rhs.begin();
            auto _element_fails = [ri](S a, S b) mutable { return a + (b < (*ri++) ? S{0} : S{1}); };
            return std::accumulate (this->begin(), this->end(), S{0}, _element_fails) == S{0} ? true : false;
        }

        //! Return true if each element of *this is <= its counterpart in rhs.
        template<typename _S=S>
        constexpr bool operator<= (const vec<_S, N>& rhs) const
        {
            auto ri = rhs.begin();
            auto _element_fails = [ri](S a, S b) mutable { return a + (b <= (*ri++) ? S{0} : S{1}); };
            return std::accumulate (this->begin(), this->end(), S{0}, _element_fails) == S{0} ? true : false;
        }

        //! Return true if each element of *this is greater than its counterpart in rhs.
        template<typename _S=S>
        constexpr bool operator> (const vec<_S, N>& rhs) const
        {
            auto ri = rhs.begin();
            auto _element_fails = [ri](S a, S b) mutable { return a + (b > (*ri++) ? S{0} : S{1}); };
            return std::accumulate (this->begin(), this->end(), S{0}, _element_fails) == S{0} ? true : false;
        }

        //! Return true if each element of *this is >= its counterpart in rhs.
        template<typename _S=S>
        constexpr bool operator>= (const vec<_S, N>& rhs) const
        {
            auto ri = rhs.begin();
            auto _element_fails = [ri](S a, S b) mutable { return a + (b >= (*ri++) ? S{0} : S{1}); };
            return std::accumulate (this->begin(), this->end(), S{0}, _element_fails) == S{0} ? true : false;
        }

        /*!
         * Unary negate operator
         *
         * \return a vec whose elements have been negated.
         */
        constexpr vec<S, N> operator-() const
        {
            vec<S, N> rtn{};
            std::transform (this->begin(), this->end(), rtn.begin(), std::negate<S>());
            return rtn;
        }

        /*!
         * Unary not operator.
         *
         * \return true if the vector length is 0, otherwise it returns false.
         */
        constexpr bool operator!() const { return (this->length() == S{0}) ? true : false; }

        /*!
         * \brief Scalar (dot) product
         *
         * Compute the scalar product of this vec and the vec, v.
         *
         * \return scalar product
         */
        template<typename _S=S>
        constexpr S dot (const vec<_S, N>& v) const
        {
            auto vi = v.begin();
            auto dot_product = [vi](S a, _S b) mutable { return a + b * (*vi++); };
            const S rtn = std::accumulate (this->begin(), this->end(), S{0}, dot_product);
            return rtn;
        }

        /*!
         * vec cross product.
         *
         * Cross product of this with another vector \a v (if N==3). In
         * higher dimensions, its more complicated to define what the cross product is,
         * and I'm unlikely to need anything other than the plain old 3D cross product.
         */
        template <typename _S=S, std::size_t _N = N, std::enable_if_t<(_N==3), int> = 0>
        constexpr vec<S, _N> cross (const vec<_S, _N>& v) const
        {
            vec<S, _N> vrtn{};
            vrtn[0] = (*this)[1] * v.z() - (*this)[2] * v.y();
            vrtn[1] = (*this)[2] * v.x() - (*this)[0] * v.z();
            vrtn[2] = (*this)[0] * v.y() - (*this)[1] * v.x();
            return vrtn;
        }

        //! Define a 2D cross product, v x w to be v_x w_y - v_y w_x.
        template <typename _S=S, std::size_t _N = N, std::enable_if_t<(_N==2), int> = 0>
        constexpr S cross (const vec<_S, _N>& w) const
        {
            S rtn = (*this)[0] * w.y() - (*this)[1] * w.x();
            return rtn;
        }

        // Convert 3D Cartesian (x,y,z) to spherical coordinates (rho, theta, phi) where theta is
        // the angle about the z axis (range [0, 2pi]) and phi is the azimuthal angle (range [0,
        // pi]).  This is the naming convention in mathematical texts. YOU MAY NEED TO SWITCH theta
        // AND phi because some functions, including boost::math for its spherical harmonics uses a
        // different convention, swapping theta and phi!
        template <std::size_t _N = N, std::enable_if_t<(_N==3), int> = 0>
        constexpr vec<S, _N> cartesian_to_spherical() const
        {
            vec<S, _N> spherical = {S{0}}; // { rho, theta, phi }
            // Assuming *this is cartesian, convert to spherical coordinates.
            S rho = this->length();
            spherical[0] = rho;                                  // rho
            spherical[1] = std::atan2 ((*this)[1], (*this)[0]);  // theta
            spherical[2] = std::acos ((*this)[2] / rho);         // phi
            return spherical;
        }

        /*!
         * Return the magnitude of the angle between this vector and the other. Works
         * for any N.
         */
        constexpr S angle (const vec<S, N>& other) const
        {
            S cos_theta = this->dot(other) / (this->length() * other.length());
            return std::acos (cos_theta);
        }

        /*!
         * Return this angle between this vector and the other. Works for any N.
         *
         * axis is the axis of rotation, so this angle IS signed, positive if 'other' is
         * at a positive right-handed angle wrt *this. axis does not need to be
         * *exactly* the axis of rotation, though it could be. The exact direction of
         * the axis of rotation can be obtained from this->cross (other), but this
         */
        constexpr S angle (const vec<S, N>& other, const vec<S, N>& axis) const
        {
            S angle_magn = this->angle (other);
            return (this->cross (other).dot (axis) > S{0}) ? angle_magn : -angle_magn;
        }

        /*!
         * Two dimensional angle in radians (only for N=2) wrt to the axes
         */
        template <typename _S=S, std::size_t _N = N, std::enable_if_t<(_N==2), int> = 0>
        constexpr S angle() const
        {
            S _angle = std::atan2 ((*this)[1], (*this)[0]);
            return _angle;
        }

        /*!
         * Set a two dimensional angle in radians (only for N=2). Preserve length, unless vector
         * length is 0, in which case set as unit vector.
         */
        template <typename _S=S, std::size_t _N = N, std::enable_if_t<(_N==2), int> = 0>
        constexpr void set_angle (const _S _ang)
        {
            S l = this->length();
            (*this)[0] = std::cos (_ang);
            (*this)[1] = std::sin (_ang);
            (*this) *= l > S{0} ? l : S{1};
        }

        /*!
         * Scalar multiply * operator
         *
         * This function will only be defined if typename _S is a
         * scalar type. Multiplies this vec<S, N> by s, element-wise.
         */
        template <typename _S=S, std::enable_if_t<std::is_scalar<std::decay_t<_S>>::value, int> = 0 >
        constexpr vec<S, N> operator* (const _S& s) const
        {
            vec<S, N> rtn{};
            auto mult_by_s = [s](S elmnt) { return elmnt * s; };
            std::transform (this->begin(), this->end(), rtn.begin(), mult_by_s);
            return rtn;
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
        constexpr vec<S, N> operator* (const vec<_S, N>& v) const
        {
            vec<S, N> rtn = {};
            auto vi = v.begin();
            auto mult_by_s = [vi](S lhs) mutable -> S { return lhs * static_cast<S>(*vi++); };
            std::transform (this->begin(), this->end(), rtn.begin(), mult_by_s);
            return rtn;
        }

        /*!
         * Scalar multiply *= operator
         *
         * This function will only be defined if typename _S is a
         * scalar type. Multiplies this vec<S, N> by s, element-wise.
         */
        template <typename _S=S, std::enable_if_t<std::is_scalar<std::decay_t<_S>>::value, int> = 0 >
        constexpr void operator*= (const _S& s)
        {
            auto mult_by_s = [s](S elmnt) { return elmnt * s; };
            std::transform (this->begin(), this->end(), this->begin(), mult_by_s);
        }

        /*!
         * vec multiply *= operator.
         *
         * Hadamard product. Multiply *this vector with \a v, elementwise.
         */
        template <typename _S=S>
        constexpr void operator*= (const vec<_S, N>& v)
        {
            auto vi = v.begin();
            auto mult_by_s = [vi](S lhs) mutable -> S { return lhs * static_cast<S>(*vi++); };
            std::transform (this->begin(), this->end(), this->begin(), mult_by_s);
        }

        //! Scalar divide by s
        template <typename _S=S, std::enable_if_t<std::is_scalar<std::decay_t<_S>>::value, int> = 0 >
        constexpr vec<S, N> operator/ (const _S& s) const
        {
            vec<S, N> rtn;
            for (std::size_t i = 0; i < N; ++i) { rtn[i] = S{0}; } // init rtn
            auto div_by_s = [s](S elmnt) { return elmnt / s; };
            std::transform (this->begin(), this->end(), rtn.begin(), div_by_s);
            return rtn;
        }

        /*!
         * operator/ gives a 'Hadamard' division - elementwise division.
         *
         * If the vectors are of differing lengths, then an exception is thrown.
         *
         * \return elementwise division of left hand size (*this) by right hand size (\a v)
         */
        template<typename _S=S>
        constexpr vec<S, N> operator/ (const vec<_S, N>& v) const
        {
            vec<S, N> rtn{};
            std::transform (this->begin(), this->end(), v.begin(), rtn.begin(), std::divides<S>());
            return rtn;
        }

        //! Scalar divide by s
        template <typename _S=S, std::enable_if_t<std::is_scalar<std::decay_t<_S>>::value, int> = 0 >
        constexpr void operator/= (const _S& s)
        {
            auto div_by_s = [s](S elmnt) { return elmnt / s; };
            std::transform (this->begin(), this->end(), this->begin(), div_by_s);
        }

        /*!
         * vec division /= operator.
         *
         * Element by element division. Divide *this vector by \a v, elementwise.
         */
        template <typename _S=S>
        constexpr void operator/= (const vec<_S, N>& v)
        {
            std::transform (this->begin(), this->end(), v.begin(), this->begin(), std::divides<S>());
        }

        //! Scalar addition
        template <typename _S=S, std::enable_if_t<std::is_scalar<std::decay_t<_S>>::value, int> = 0 >
        constexpr vec<S, N> operator+ (const _S& s) const
        {
            vec<S, N> rtn{};
            auto add_s = [s](S elmnt) { return elmnt + s; };
            std::transform (this->begin(), this->end(), rtn.begin(), add_s);
            return rtn;
        }

        //! Addition which should work for any member type that implements the + operator
        constexpr vec<S, N> operator+ (const S& s) const
        {
            vec<S, N> rtn{};
            auto add_s = [s](S elmnt) { return elmnt + s; };
            std::transform (this->begin(), this->end(), rtn.begin(), add_s);
            return rtn;
        }

        //! vec addition operator
        template<typename _S=S>
        constexpr vec<S, N> operator+ (const vec<_S, N>& v) const
        {
            vec<S, N> vrtn{};
            auto vi = v.begin();
            auto add_v = [vi](S a) mutable { return a + (*vi++); };
            std::transform (this->begin(), this->end(), vrtn.begin(), add_v);
            return vrtn;
        }

        //! Scalar addition
        template <typename _S=S, std::enable_if_t<std::is_scalar<std::decay_t<_S>>::value, int> = 0 >
        constexpr void operator+= (const _S& s)
        {
            auto add_s = [s](S elmnt) { return elmnt + s; };
            std::transform (this->begin(), this->end(), this->begin(), add_s);
        }

        //! Addition += operator for any type same as the enclosed type that implements + op
        constexpr void operator+= (const S& s) const
        {
            auto add_s = [s](S elmnt) { return elmnt + s; };
            std::transform (this->begin(), this->end(), this->begin(), add_s);
        }

        //! vec addition operator
        template<typename _S=S>
        constexpr void operator+= (const vec<_S, N>& v)
        {
            auto vi = v.begin();
            auto add_v = [vi](S a) mutable { return a + (*vi++); };
            std::transform (this->begin(), this->end(), this->begin(), add_v);
        }

        //! Scalar subtraction
        template <typename _S=S, std::enable_if_t<std::is_scalar<std::decay_t<_S>>::value, int> = 0 >
        constexpr vec<S, N> operator- (const _S& s) const
        {
            vec<S, N> rtn{};
            auto subtract_s = [s](S elmnt) { return elmnt - s; };
            std::transform (this->begin(), this->end(), rtn.begin(), subtract_s);
            return rtn;
        }

        //! Subtraction which should work for any member type that implements the - operator
        constexpr vec<S, N> operator- (const S& s) const
        {
            vec<S, N> rtn{};
            auto subtract_s = [s](S elmnt) { return elmnt - s; };
            std::transform (this->begin(), this->end(), rtn.begin(), subtract_s);
            return rtn;
        }

        //! vec subtraction operator
        template<typename _S=S>
        constexpr vec<S, N> operator- (const vec<_S, N>& v) const
        {
            vec<S, N> vrtn{};
            auto vi = v.begin();
            auto subtract_v = [vi](S a) mutable { return a - (*vi++); };
            std::transform (this->begin(), this->end(), vrtn.begin(), subtract_v);
            return vrtn;
        }

        //! Scalar subtraction
        template <typename _S=S, std::enable_if_t<std::is_scalar<std::decay_t<_S>>::value, int> = 0 >
        constexpr void operator-= (const _S& s)
        {
            auto subtract_s = [s](S elmnt) { return elmnt - s; };
            std::transform (this->begin(), this->end(), this->begin(), subtract_s);
        }

        //! Subtraction -= operator for any time same as the enclosed type that implements - op
        constexpr void operator-= (const S& s) const
        {
            auto subtract_s = [s](S elmnt) { return elmnt - s; };
            std::transform (this->begin(), this->end(), this->begin(), subtract_s);
        }

        //! vec subtraction operator
        template<typename _S=S>
        constexpr void operator-= (const vec<_S, N>& v)
        {
            auto vi = v.begin();
            auto subtract_v = [vi](S a) mutable { return a - (*vi++); };
            std::transform (this->begin(), this->end(), this->begin(), subtract_v);
        }

        //! Overload the stream output operator
        friend std::ostream& operator<< <S, N> (std::ostream& os, const vec<S, N>& v);
    };

    template <typename S=float, std::size_t N=3>
    std::ostream& operator<< (std::ostream& os, const vec<S, N>& v)
    {
        os << v.str();
        return os;
    }

    // Operators that can do premultiply, predivide by scaler so you could do,
    // e.g. vec<float> denom = {1,2,3}; vec<float> result = float(1) / denom;

    //! Scalar * vec<> (commutative; lhs * rhs == rhs * lhs, so return rhs * lhs)
    template <typename S, std::size_t N>
    constexpr vec<S, N> operator* (S lhs, const vec<S, N>& rhs) { return rhs * lhs; }

    //! Scalar / vec<>
    template <typename S, std::size_t N>
    constexpr vec<S, N> operator/ (S lhs, const vec<S, N>& rhs)
    {
        vec<S, N> division;
        auto lhs_div_by_vec = [lhs](S elmnt) { return lhs / elmnt; };
        std::transform (rhs.begin(), rhs.end(), division.begin(), lhs_div_by_vec);
        return division;
    }

    //! Scalar + vec<> (commutative)
    template <typename S, std::size_t N>
    constexpr vec<S, N> operator+ (S lhs, const vec<S, N>& rhs) { return rhs + lhs; }

    //! Scalar - vec<>
    template <typename S, std::size_t N>
    constexpr vec<S, N> operator- (S lhs, const vec<S, N>& rhs)
    {
        vec<S, N> subtraction;
        auto lhs_minus_vec = [lhs](S elmnt) { return lhs - elmnt; };
        std::transform (rhs.begin(), rhs.end(), subtraction.begin(), lhs_minus_vec);
        return subtraction;
    }

} // namespace morph
