/*!
 * \file
 * \brief An N dimensional vector class template, morph::vVector which derives from std::vector.
 *
 * \author Seb James
 * \date May 2020
 */
#pragma once

#include <cmath>
#include <vector>
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
     * \brief N-D vector class deriving from std::vector
     *
     * An N dimensional vector class template which derives from std::vector. Its
     * components are of scalar type S. It is anticipated that S will be set either to
     * floating point scalar types such as float or double, or to integer scalar types
     * such as int, long long int and so on. Thus, a typical (and in fact, the default)
     * signature would be:
     *
     *\code{.cpp}
     * vVector<float> v(3);
     *\endcode
     *
     * The class inherits std::vector's dynamically-resizeable memory for storing the
     * components of the vector and std::vector's constructors.. It adds numerous
     * methods which allow objects of type vVector to have arithmetic operations applied
     * to them, either scalar (add a scalar to all elements; divide all elements by a
     * scalar, etc) or vector (including dot and cross products, normalization and so
     * on.
     *
     * This class is better for writing neural networks than morph::Vector, whose size
     * has to be set at compile time.
     */
    template <typename S, typename Al> struct vVector;

    /*!
     * Template friendly mechanism to overload the stream operator.
     *
     * Note forward declaration of the vVector template class and this template for
     * stream operator overloading. Example adapted from
     * https://stackoverflow.com/questions/4660123
     */
    template <typename S, typename Al> std::ostream& operator<< (std::ostream&, const vVector<S, Al>&);

    template <typename S=float, typename Al=std::allocator<S>>
    struct vVector : public std::vector<S, Al>
    {
        //! We inherit std::vector's constructors like this:
        using std::vector<S, Al>::vector;

        //! \return the first component of the vector
        S x() const { return (*this)[0]; }
        //! \return the second component of the vector
        S y() const { return (*this)[1]; }
        //! \return the third component of the vector
        S z() const { return (*this)[2]; }
        //! \return the fourth component of the vector
        S w() const { return (*this)[3]; }

        //! Set data members from an std::vector (by copying)
        template <typename _S=S>
        void set_from (const std::vector<_S>& vec)
        {
            this->resize(vec.size());
            std::copy (vec.begin(), vec.end(), this->begin());
        }

        //! Set data members from an std::array, matching the size of the array first.
        template <typename _S=S, size_t N>
        void set_from (const std::array<_S, N>& ar)
        {
            this->resize(N);
            std::copy (ar.begin(), ar.end(), this->begin());
        }

        //! Set all elements from the value type v. Same as vVector::set
        template <typename _S=S>
        void set_from (const _S& v) { std::fill (this->begin(), this->end(), v); }

        /*!
         * Set the data members of this vVector from the passed in, larger array, \a ar,
         * ignoring the last element of \a ar. Used when working with 4D vectors in
         * graphics applications involving 4x4 transform matrices.
         */
        template <typename _S=S>
        void set_from_onelonger (const std::vector<_S>& v)
        {
            if (v.size() == (this->size()+1)) {
                for (size_t i = 0; i < this->size(); ++i) {
                    (*this)[i] = v[i];
                }
            } // else do nothing?
        }

        /*!
         * Set the data members of this vVector from the passed in, larger array, \a ar,
         * ignoring the last element of \a ar. Used when working with 4D vectors in
         * graphics applications involving 4x4 transform matrices.
         */
        template <typename _S=S, size_t N>
        void set_from_onelonger (const std::array<_S, N>& v)
        {
            if ((this->size()+1) == N) {
                for (size_t i = 0; i < this->size(); ++i) {
                    (*this)[i] = v[i];
                }
            } // else do nothing?
        }

        //! Return a vector with one less dimension - losing the last one.
        vVector<S> less_one_dim () const
        {
            size_t N = this->size();
            vVector<S> rtn(N-1);
            for (size_t i = 0; i < N-1; ++i) { rtn[i] = (*this)[i]; }
            return rtn;
        }

        //! Return a vector with one additional dimension - setting it to 0.
        vVector<S> plus_one_dim () const
        {
            size_t N = this->size();
            vVector<S> rtn(N+1);
            for (size_t i = 0; i < N; ++i) { rtn[i] = (*this)[i]; }
            rtn[N] = S{0};
            return rtn;
        }

        //! Return this vVector in single precision, float format
        vVector<float> as_float() const
        {
            vVector<float> vv(this->size(), 0.0f);
            vv += *this;
            return vv;
        }

        //! Return this vVector in double precision format
        vVector<double> as_double() const
        {
            vVector<double> vv(this->size(), 0.0);
            vv += *this;
            return vv;
        }

        /*!
         * Set an N-D vVector from an N+1 D vVector. Intended to convert 4D vectors (that
         * have been operated on by 4x4 matrices) into 3D vectors.
         */
        template <typename _S=S>
        void set_from_onelonger (const vVector<_S>& v)
        {
            if (v.size() == (this->size()+1)) {
                for (size_t i = 0; i < this->size(); ++i) {
                    (*this)[i] = v[i];
                }
            } // else do nothing?
        }

        /*!
         * Set a linear sequence into the vector from value start to value stop. If
         * num>0 then resize the vector first, otherwise use the vVector's current
         * size. You *can* use this with integer types, but be prepared to notice odd
         * rounding errors.
         */
        template <typename _S=S, typename _S2=S>
        void linspace (const _S start, const _S2 stop, const size_t num=0)
        {
            if (num > 0) { this->resize (num); }
            S increment = (static_cast<S>(stop) - static_cast<S>(start)) / (this->size()-1);
            for (size_t i = 0; i < this->size(); ++i) { (*this)[i] = start + increment * i; }
        }

        //! Stream the coordinates of the vector into \a ss as a comma separated list.
        void str_comma_separated (std::stringstream& ss) const
        {
            if (this->empty()) { return; }
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

        /*!
         * Output the vector in a form suitable to paste into Python, as a numpy vector,
         * assuming you imported numpy as np
         */
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
            for (auto& i : *this) { i = ru.get(); }
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
            for (auto& i : *this) { i = ru.get(); }
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
            for (auto& i : *this) { i = rn.get(); }
        }

        /*!
         * Permute the elements in a rotation. 0->N-1, 1->0, 2->1, etc. Useful for
         * swapping x and y in a 2D vector.
         */
        void rotate()
        {
            if (this->size() > 1) {
                S z_el = (*this)[0];
                for (size_t i = 1; i < this->size(); ++i) {
                    (*this)[i-1] = (*this)[i];
                }
                (*this)[this->size()-1] = z_el;
            } // else no op
        }

        //! If size is even, permute pairs of elements in a rotation. 0->1, 1->0, 2->3, 3->2, etc.
        void rotate_pairs()
        {
            size_t N = this->size();
            if (N%2!=0) {
                throw std::runtime_error ("vVector size must be even to call morph::vVector::rotate_pairs");
            }
            S tmp_el = S{0};
            for (size_t i = 0; i < N; i+=2) {
                tmp_el = (*this)[i];
                (*this)[i] = (*this)[i+1];
                (*this)[i+1] = tmp_el;
            }
        }

        /*!
         * Test to see if this vector is a unit vector (it doesn't *have* to be).
         *
         * \return true if the length of the vector is 1.
         */
        bool checkunit() const
        {
            /*!
             * \brief Unit vector threshold
             *
             * The threshold outside of which the vector is no longer considered to be a
             * unit vector. Note this is hard coded as a constexpr, to avoid messing with
             * the initialization of the vVector with curly brace initialization.
             *
             * Clearly, this will be the wrong threshold for some cases. Possibly, a
             * template parameter could set this; so size_t U could indicate the threshold;
             * 0.001 could be U=-3 (10^-3).
             *
             * Another idea would be to change unitThresh based on the type S. Or use
             * numeric_limits<S>::epsilon and find out what multiple of epsilon would make
             * sense.
             */
            static constexpr S unitThresh = 0.001;

            auto subtract_squared = [](S a, S b) { return a - b * b; };
            const S metric = std::accumulate (this->begin(), this->end(), S{1}, subtract_squared);
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
            const S len = std::sqrt (std::accumulate (this->begin(), this->end(), S{0}, add_squared));
            return len;
        }

        /*!
         * Find the squared length of the vector which is the same as the sum of squared
         * elements, if elements are scalar.
         *
         * \return the length squared
         */
        S length_sq() const
        {
            auto add_squared = [](S a, S b) { return a + b * b; };
            const S len_sq = std::accumulate (this->begin(), this->end(), S{0}, add_squared);
            return len_sq;
        }

        /*!
         * Return the sum of the squares of the elements. If S typed elements are
         * morph::Vectors or morph::vVectors, then return the sum of the squares of the
         * lengths of the elements in the zeroth element of the return S type.
         */
        S sos() const
        {
            S _sos = S{0};
            if constexpr (std::is_scalar<std::decay_t<S>>::value) {
                _sos = this->length_sq();
            } else {
                // S is a vector so i is a vector.
                for (auto& i : *this) { _sos[0] += i.length_sq(); }
            }
            return _sos;
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

        /*!
         * Return the index of the shortest component of the vector. If this is a vector
         * of vectors, then return the index of the shortest vector.
         */
        size_t argshortest() const
        {
            size_t idx = 0;
            // Check on the type S. If S is a vector thing, then abs_compare needs to be different.
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
            return themax == this->end() ? S{0} : *themax;
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
            return themin == this->end() ? S{0} : *themin;
        }

        //! Return the index of the minimum (smallest or most negative) component of the vector.
        size_t argmin() const
        {
            auto themin = std::min_element (this->begin(), this->end());
            size_t idx = (themin - this->begin());
            return idx;
        }

        //! Perform element-wise max. For each element, if val is the maximum, the element becomes val.
        template <typename _S=S>
        void max_elementwise_inplace (const _S& val) { for (auto& i : *this) { i = std::max (i, val); } }

        //! Perform element-wise min. For each element, if val is the minimum, the element becomes val.
        template <typename _S=S>
        void min_elementwise_inplace (const _S& val) { for (auto& i : *this) { i = std::min (i, val); } }

        //! Return true if any element is zero
        bool has_zero() const
        {
            return std::any_of (this->cbegin(), this->cend(), [](S i){ return i == S{0}; });
        }

        //! Return true if any element is NaN or infinity
        bool has_nan_or_inf() const
        {
            bool has_nan_or_inf = false;
            if constexpr (std::numeric_limits<S>::has_quiet_NaN) {
                has_nan_or_inf = std::any_of (this->cbegin(), this->cend(), [](S i){
                        return i == std::numeric_limits<S>::quiet_NaN();
                    });
            }
            if (has_nan_or_inf) { return has_nan_or_inf; }

            if constexpr (std::numeric_limits<S>::has_infinity) {
                has_nan_or_inf = std::any_of (this->cbegin(), this->cend(), [](S i){
                        return i == std::numeric_limits<S>::infinity();
                    });
            }
            if (has_nan_or_inf) { return has_nan_or_inf; }

            if constexpr (std::numeric_limits<S>::has_signaling_NaN) {
                has_nan_or_inf = std::any_of (this->cbegin(), this->cend(), [](S i){
                        return i == std::numeric_limits<S>::signaling_NaN();
                    });
            }

            return has_nan_or_inf;
        }

        //! Return true if any element is NaN or infinity
        bool has_inf() const
        {
            bool has_inf = false;
            if constexpr (std::numeric_limits<S>::has_infinity) {
                has_inf = std::any_of (this->cbegin(), this->cend(), [](S i){
                        return i == std::numeric_limits<S>::infinity();
                    });
            }
            return has_inf;
        }

        //! Return true if any element is NaN or infinity
        bool has_nan() const
        {
            bool has_nan = false;
            if constexpr (std::numeric_limits<S>::has_quiet_NaN) {
                has_nan = std::any_of (this->cbegin(), this->cend(), [](S i){
                        return i == std::numeric_limits<S>::quiet_NaN();
                    });
            }
            if (has_nan) { return has_nan; }

            if constexpr (std::numeric_limits<S>::has_signaling_NaN) {
                has_nan = std::any_of (this->cbegin(), this->cend(), [](S i){
                        return i == std::numeric_limits<S>::signaling_NaN();
                    });
            }

            return has_nan;
        }

        //! Return the arithmetic mean of the elements
        S mean() const
        {
            const S sum = std::accumulate (this->begin(), this->end(), S{0});
            return sum / this->size();
        }

        //! Return the variance of the elements
        S variance() const
        {
            if (this->empty()) { return S{0}; }
            S _mean = this->mean();
            S sos_deviations = S{0};
            for (S val : *this) {
                sos_deviations += ((val-_mean)*(val-_mean));
            }
            S variance = sos_deviations / (this->size()-1);
            return variance;
        }

        //! Return the standard deviation of the elements
        S std() const
        {
            if (this->empty()) { return S{0}; }
            return std::sqrt (this->variance());
        }

        //! Return the sum of the elements
        S sum() const
        {
            return std::accumulate (this->begin(), this->end(), S{0});
        }

        //! Return the product of the elements
        S product() const
        {
            auto _product = [](S a, S b) mutable { return a ? a * b : b; };
            return std::accumulate (this->begin(), this->end(), S{0}, _product);
        }

        /*!
         * Compute the element-wise pth power of the vector
         *
         * \return a vVector whose elements have been raised to the power p
         */
        vVector<S> pow (const S& p) const
        {
            vVector<S> rtn(this->size());
            auto raise_to_p = [p](S coord) { return std::pow(coord, p); };
            std::transform (this->begin(), this->end(), rtn.begin(), raise_to_p);
            return rtn;
        }
        //! Raise each element to the power p
        void pow_inplace (const S& p) { for (auto& i : *this) { i = std::pow (i, p); } }

        //! Element-wise power
        template<typename _S=S>
        vVector<S> pow (const vVector<_S>& p) const
        {
            if (p.size() != this->size()) {
                throw std::runtime_error ("element-wise power: p dims should equal vVector's dims");
            }
            auto pi = p.begin();
            vVector<S> rtn(this->size());
            auto raise_to_p = [pi](S coord) mutable { return std::pow(coord, static_cast<S>(*pi++)); };
            std::transform (this->begin(), this->end(), rtn.begin(), raise_to_p);
            return rtn;
        }
        //! Raise each element, i, to the power p[i]
        template<typename _S=S>
        void pow_inplace (const vVector<_S>& p)
        {
            if (p.size() != this->size()) {
                throw std::runtime_error ("element-wise power: p dims should equal vVector's dims");
            }
            auto pi = p.begin();
            for (auto& i : *this) { i = std::pow (i, static_cast<S>(*pi++)); }
        }

        //! Return the signum of the vVector, with signum(0)==0
        vVector<S> signum() const
        {
            vVector<S> rtn(this->size());
            auto _signum = [](S coord) { return (coord > S{0} ? S{1} : (coord == S{0} ? S{0} : S{-1})); };
            std::transform (this->begin(), this->end(), rtn.begin(), _signum);
            return rtn;
        }
        void signum_inplace() { for (auto& i : *this) { i = (i > S{0} ? S{1} : (i == S{0} ? S{0} : S{-1})); } }

        /*!
         * Compute the element-wise square root of the vector
         *
         * \return a vVector whose elements have been square-rooted
         */
        vVector<S> sqrt() const
        {
            vVector<S> rtn(this->size());
            auto sqrt_element = [](S coord) { return static_cast<S>(std::sqrt(coord)); };
            std::transform (this->begin(), this->end(), rtn.begin(), sqrt_element);
            return rtn;
        }
        //! Replace each element with its own square root
        void sqrt_inplace() { for (auto& i : *this) { i = static_cast<S>(std::sqrt (i)); } }

        /*!
         * Compute the element-wise square of the vector
         *
         * \return a vVector whose elements have been squared
         */
        vVector<S> sq() const
        {
            vVector<S> rtn(this->size());
            auto sq_element = [](S coord) { return std::pow(coord, 2); };
            std::transform (this->begin(), this->end(), rtn.begin(), sq_element);
            return rtn;
        }
        //! Replace each element with its own square
        void sq_inplace() { for (auto& i : *this) { i = (i*i); } }

        /*!
         * Compute the element-wise natural logarithm of the vector
         *
         * \return a vVector whose elements have been logged
         */
        vVector<S> log() const
        {
            vVector<S> rtn(this->size());
            auto log_element = [](S coord) { return std::log(coord); };
            std::transform (this->begin(), this->end(), rtn.begin(), log_element);
            return rtn;
        }
        //! Replace each element with its own log
        void log_inplace() { for (auto& i : *this) { i = std::log(i); } }

        /*!
         * Compute the element-wise logarithm-to-base-10 of the vector
         *
         * \return a vVector whose elements have been log10ed
         */
        vVector<S> log10() const
        {
            vVector<S> rtn(this->size());
            auto log_element = [](S coord) { return std::log10(coord); };
            std::transform (this->begin(), this->end(), rtn.begin(), log_element);
            return rtn;
        }
        //! Replace each element with its own log
        void log10_inplace() { for (auto& i : *this) { i = std::log10(i); } }

        //! Sine
        vVector<S> sin() const
        {
            vVector<S> rtn(this->size());
            auto sin_element = [](S coord) { return std::sin(coord); };
            std::transform (this->begin(), this->end(), rtn.begin(), sin_element);
            return rtn;
        }
        //! Replace each element with its own sine
        void sin_inplace() { for (auto& i : *this) { i = std::sin(i); } }

        //! Cosine
        vVector<S> cos() const
        {
            vVector<S> rtn(this->size());
            auto cos_element = [](S coord) { return std::cos(coord); };
            std::transform (this->begin(), this->end(), rtn.begin(), cos_element);
            return rtn;
        }
        //! Replace each element with its own cosine
        void cos_inplace() { for (auto& i : *this) { i = std::cos(i); } }

        /*!
         * Compute the element-wise natural exponential of the vector
         *
         * \return a vVector whose elements have been exponentiate
         */
        vVector<S> exp() const
        {
            vVector<S> rtn(this->size());
            auto exp_element = [](S coord) { return std::exp(coord); };
            std::transform (this->begin(), this->end(), rtn.begin(), exp_element);
            return rtn;
        }
        //! Replace each element with its own exp
        void exp_inplace() { for (auto& i : *this) { i = std::exp(i); } }

        /*!
         * Compute the element-wise absolute values of the vector
         *
         * \return a vVector whose elements have been 'absed'
         */
        vVector<S> abs() const
        {
            vVector<S> rtn(this->size());
            auto abs_element = [](S coord) { return std::abs(coord); };
            std::transform (this->begin(), this->end(), rtn.begin(), abs_element);
            return rtn;
        }
        //! Replace each element with its absolute value
        void abs_inplace() { for (auto& i : *this) { i = std::abs(i); } }

        //! Less than a scalar. Return true if every element is less than the scalar
        bool operator<(const S rhs) const
        {
            auto _element_fails = [rhs](S a, S b) { return a + (b < rhs ? S{0} : S{1}); };
            return std::accumulate (this->begin(), this->end(), S{0}, _element_fails) == S{0} ? true : false;
        }

        //! <= a scalar. Return true if every element is less than the scalar
        bool operator<=(const S rhs) const
        {
            auto _element_fails = [rhs](S a, S b) { return a + (b <= rhs ? S{0} : S{1}); };
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

        // vVector-to-vVector comparisons. Although there's an underlying std::vector
        // implementation of these operators, I prefer to reimplement with a requirement
        // that the vVectors should have the same size to be compared, as this is a
        // better defined comparison. These might be slow if your vVector is very big.
        //
        // More importantly, I *completely redefine the meaning of the comparison
        // operators between vVectors*. This messes up the use of containers that make
        // use of comparison operators like std::set.
        //
        // Use something like this as a compare function when storing morph::Vectors in
        // a std::set:
        //
        //    auto _cmp = [](Vector<float, 3> a, Vector<float, 3> b) { return a.lexical_lessthan(b); };
        //    std::set<Vector<float, 3>, decltype(_cmp)> aset(_cmp); // C++-11/C++-17
        //
        // If the std::set is a class member, then define a compare struct with an operator().
        //
        // The default comparison for std::set is the operator<. The definition here
        // applied to !comp(a,b) && !comp(b,a) will suggest that two different vVectors
        // are equal even when they're not and so your std::sets will fail to insert
        // unique vVectors

        //! Lexical less-than similar to the operator< implemented for std::vector
        template<typename _S=S>
        bool lexical_lessthan (const vVector<_S>& rhs) const
        {
            return std::lexicographical_compare (this->begin(), this->end(), rhs.begin(), rhs.end());
        }

        //! Another way to compare vectors would be by length.
        template<typename _S=S>
        bool length_lessthan (const vVector<_S>& rhs) const
        {
            if (rhs.size() != this->size()) {
                throw std::runtime_error ("length based comparison: rhs dims should equal vVector's dims");
            }
            return this->length() < rhs.length();
        }

        //! Return true if each element of *this is less than its counterpart in rhs.
        template<typename _S=S>
        bool operator< (const vVector<_S>& rhs) const
        {
            if (rhs.size() != this->size()) {
                throw std::runtime_error ("element-wise comparison: rhs dims should equal vVector's dims");
            }
            auto ri = rhs.begin();
            auto _element_fails = [ri](S a, S b) mutable { return a + (b < (*ri++) ? S{0} : S{1}); };
            return std::accumulate (this->begin(), this->end(), S{0}, _element_fails) == S{0} ? true : false;
        }

        //! Return true if each element of *this is <= its counterpart in rhs.
        template<typename _S=S>
        bool operator<= (const vVector<_S>& rhs) const
        {
            if (rhs.size() != this->size()) {
                throw std::runtime_error ("element-wise comparison: rhs dims should equal vVector's dims");
            }
            auto ri = rhs.begin();
            auto _element_fails = [ri](S a, S b) mutable { return a + (b <= (*ri++) ? S{0} : S{1}); };
            return std::accumulate (this->begin(), this->end(), S{0}, _element_fails) == S{0} ? true : false;
        }

        //! Return true if each element of *this is greater than its counterpart in rhs.
        template<typename _S=S>
        bool operator> (const vVector<_S>& rhs) const
        {
            if (rhs.size() != this->size()) {
                throw std::runtime_error ("element-wise comparison: rhs dims should equal vVector's dims");
            }
            auto ri = rhs.begin();
            auto _element_fails = [ri](S a, S b) mutable { return a + (b > (*ri++) ? S{0} : S{1}); };
            return std::accumulate (this->begin(), this->end(), S{0}, _element_fails) == S{0} ? true : false;
        }

        //! Return true if each element of *this is >= its counterpart in rhs.
        template<typename _S=S>
        bool operator>= (const vVector<_S>& rhs) const
        {
            if (rhs.size() != this->size()) {
                throw std::runtime_error ("element-wise comparison: rhs dims should equal vVector's dims");
            }
            auto ri = rhs.begin();
            auto _element_fails = [ri](S a, S b) mutable { return a + (b >= (*ri++) ? S{0} : S{1}); };
            return std::accumulate (this->begin(), this->end(), S{0}, _element_fails) == S{0} ? true : false;
        }

        /*!
         * Unary negate operator
         *
         * \return a vVector whose elements have been negated.
         */
        vVector<S> operator-() const
        {
            vVector<S> rtn(this->size());
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
         * \brief Scalar (dot) product of two vVectors
         *
         * Compute the scalar product of this vVector and the vVector, v.
         *
         * If \a v and *this have different sizes, then throw exception.
         *
         * \return scalar product
         */
        template<typename _S=S>
        S dot (const vVector<_S>& v) const
        {
            if (this->size() != v.size()) {
                throw std::runtime_error ("vVector::dot(): vectors must have equal size");
            }
            auto vi = v.begin();
            auto dot_product = [vi](S a, _S b) mutable -> S { return a + static_cast<S>(b) * static_cast<S>(*vi++); };
            const S rtn = std::accumulate (this->begin(), this->end(), S{0}, dot_product);
            return rtn;
        }

        /*!
         * Compute the vector cross product.
         *
         * Cross product of this with another vVector \a v (if N==3). In
         * higher dimensions, its more complicated to define what the cross product is,
         * and I'm unlikely to need anything other than the plain old 3D cross product.
         */
        template<typename _S=S>
        vVector<S, Al> cross (const vVector<_S>& v) const
        {
            vVector<S, Al> vrtn;
            if (this->size() == 3 && v.size() == 3) {
                vrtn.resize(3);
                vrtn[0] = (*this)[1] * v.z() - (*this)[2] * v.y();
                vrtn[1] = (*this)[2] * v.x() - (*this)[0] * v.z();
                vrtn[2] = (*this)[0] * v.y() - (*this)[1] * v.x();
            } else {
                throw std::runtime_error ("vVector::cross(): Cross product is defined here for 3 dimensions only");
            }
            return vrtn;
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
        vVector<S, Al> operator* (const vVector<_S>& v) const
        {
            if (v.size() != this->size()) {
                throw std::runtime_error ("vVector::operator*: Hadamard product is defined here for vectors of same dimensionality only");
            }
            vVector<S, Al> rtn(this->size(), S{0});
            auto vi = v.begin();
            // Visual Studio may complain about there being no static_cast<S> of (*vi++), here
            auto mult_by_s = [vi](S lhs) mutable -> S { return lhs * (*vi++); };
            std::transform (this->begin(), this->end(), rtn.begin(), mult_by_s);
            return rtn;
        }

        /*!
         * vVector multiply *= operator.
         *
         * Hadamard product. Multiply *this vector with \a v, elementwise. If \a v has a
         * different number of elements to *this, then an exception is thrown.
         */
        template <typename _S=S>
        void operator*= (const vVector<_S>& v) {
            if (v.size() == this->size()) {
                auto vi = v.begin();
                auto mult_by_s = [vi](S lhs) mutable -> S { return lhs * (*vi++); };
                std::transform (this->begin(), this->end(), this->begin(), mult_by_s);
            } else {
                throw std::runtime_error ("vVector::operator*=: Hadamard product is defined here for vectors of same dimensionality only");
            }
        }

        /*!
         * operator/ gives element by element division
         *
         * 'Hadamard' division - elementwise division. If the vectors are of
         * differing lengths, then an exception is thrown.
         *
         * \return Hadamard division of left hand size (*this) by right hand size (\a v)
         */
        template<typename _S=S>
        vVector<S, Al> operator/ (const vVector<_S>& v) const
        {
            if (v.size() != this->size()) {
                throw std::runtime_error ("vVector::operator*: Hadamard division is defined here for vectors of same dimensionality only");
            }
            vVector<S, Al> rtn(this->size(), S{0});
            auto vi = v.begin();
            auto div_by_s = [vi](S lhs) mutable -> S { return lhs / (*vi++); };
            std::transform (this->begin(), this->end(), rtn.begin(), div_by_s);
            return rtn;
        }

        /*!
         * vVector division /= operator.
         *
         * Hadamard division. Divide *this vector by \a v, elementwise. If \a v has a
         * different number of elements to *this, then an exception is thrown.
         */
        template <typename _S=S>
        void operator/= (const vVector<_S>& v) {
            if (v.size() == this->size()) {
                auto vi = v.begin();
                auto div_by_s = [vi](S lhs) mutable -> S { return lhs / (*vi++); };
                std::transform (this->begin(), this->end(), this->begin(), div_by_s);
            } else {
                throw std::runtime_error ("vVector::operator*=: Hadamard division is defined here for vectors of same dimensionality only");
            }
        }

        /*!
         * Scalar multiply * operator
         *
         * This function will only be defined if typename _S is a
         * scalar type. Multiplies this vVector<S> by s, element-wise.
         */
        template <typename _S=S, std::enable_if_t<std::is_scalar<std::decay_t<_S>>::value, int> = 0 >
        vVector<S> operator* (const _S& s) const
        {
            vVector<S> rtn(this->size());
            auto mult_by_s = [s](S coord) -> S { return coord * s; };
            std::transform (this->begin(), this->end(), rtn.begin(), mult_by_s);
            return rtn;
        }

        /*!
         * Scalar multiply *= operator
         *
         * This function will only be defined if typename _S is a
         * scalar type. Multiplies this vVector<S> by s, element-wise.
         */
        template <typename _S=S, std::enable_if_t<std::is_scalar<std::decay_t<_S>>::value, int> = 0 >
        void operator*= (const _S& s)
        {
            auto mult_by_s = [s](S coord) -> S { return coord * s; };
            std::transform (this->begin(), this->end(), this->begin(), mult_by_s);
        }

        //! Scalar divide by s
        template <typename _S=S, std::enable_if_t<std::is_scalar<std::decay_t<_S>>::value, int> = 0 >
        vVector<S> operator/ (const _S& s) const
        {
            vVector<S> rtn(this->size());
            auto div_by_s = [s](S coord) -> S { return coord / s; };
            std::transform (this->begin(), this->end(), rtn.begin(), div_by_s);
            return rtn;
        }

        //! Scalar divide by s
        template <typename _S=S, std::enable_if_t<std::is_scalar<std::decay_t<_S>>::value, int> = 0 >
        void operator/= (const _S& s)
        {
            auto div_by_s = [s](S coord) -> S { return coord / s; };
            std::transform (this->begin(), this->end(), this->begin(), div_by_s);
        }

        //! vVector addition operator
        template<typename _S=S>
        vVector<S> operator+ (const vVector<_S>& v) const
        {
            vVector<S> vrtn(this->size());
            auto vi = v.begin();
            // Static cast is encouraged by Visual Studio, but it prevents addition of vVector of Vectors and vVector of scalars
            auto add_v = [vi](S a) mutable -> S { return a + /* static_cast<S> */(*vi++); };
            std::transform (this->begin(), this->end(), vrtn.begin(), add_v);
            return vrtn;
        }

        //! vVector addition operator
        template<typename _S=S>
        void operator+= (const vVector<_S>& v)
        {
            auto vi = v.begin();
            auto add_v = [vi](S a) mutable -> S { return a + /* static_cast<S> */(*vi++); };
            std::transform (this->begin(), this->end(), this->begin(), add_v);
        }

        //! A vVector subtraction operator
        template<typename _S=S>
        vVector<S> operator- (const vVector<_S>& v) const
        {
            vVector<S> vrtn(this->size());
            auto vi = v.begin();
            auto subtract_v = [vi](S a) mutable -> S { return a - (*vi++); };
            std::transform (this->begin(), this->end(), vrtn.begin(), subtract_v);
            return vrtn;
        }

        //! A vVector subtraction operator
        template<typename _S=S>
        void operator-= (const vVector<_S>& v)
        {
            auto vi = v.begin();
            auto subtract_v = [vi](S a) mutable -> S { return a - (*vi++); };
            std::transform (this->begin(), this->end(), this->begin(), subtract_v);
        }

        //! Scalar addition
        template <typename _S=S, std::enable_if_t<std::is_scalar<std::decay_t<_S>>::value, int> = 0 >
        vVector<S> operator+ (const _S& s) const
        {
            vVector<S> rtn(this->size());
            auto add_s = [s](S coord) -> S { return coord + s; };
            std::transform (this->begin(), this->end(), rtn.begin(), add_s);
            return rtn;
        }

        //! Scalar addition
        template <typename _S=S, std::enable_if_t<std::is_scalar<std::decay_t<_S>>::value, int> = 0 >
        void operator+= (const _S& s)
        {
            auto add_s = [s](S coord) -> S { return coord + s; };
            std::transform (this->begin(), this->end(), this->begin(), add_s);
        }

        //! Scalar subtraction
        template <typename _S=S, std::enable_if_t<std::is_scalar<std::decay_t<_S>>::value, int> = 0 >
        vVector<S> operator- (const _S& s) const
        {
            vVector<S> rtn(this->size());
            auto subtract_s = [s](S coord) -> S { return coord - s; };
            std::transform (this->begin(), this->end(), rtn.begin(), subtract_s);
            return rtn;
        }

        //! Scalar subtraction
        template <typename _S=S, std::enable_if_t<std::is_scalar<std::decay_t<_S>>::value, int> = 0 >
        void operator-= (const _S& s)
        {
            auto subtract_s = [s](S coord) -> S { return coord - s; };
            std::transform (this->begin(), this->end(), this->begin(), subtract_s);
        }

        //! Addition which should work for any member type that implements the + operator
        vVector<S> operator+ (const S& s) const
        {
            vVector<S> rtn(this->size());
            auto add_s = [s](S coord) -> S { return coord + s; };
            std::transform (this->begin(), this->end(), rtn.begin(), add_s);
            return rtn;
        }

        //! Addition += operator for any time same as the enclosed type that implements + op
        void operator+= (const S& s) const
        {
            auto add_s = [s](S coord) -> S { return coord + s; };
            std::transform (this->begin(), this->end(), this->begin(), add_s);
        }

        //! Subtraction which should work for any member type that implements the - operator
        vVector<S> operator- (const S& s) const
        {
            vVector<S> rtn(this->size());
            auto subtract_s = [s](S coord) -> S { return coord - s; };
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
        friend std::ostream& operator<< <S> (std::ostream& os, const vVector<S>& v);
    };

    template <typename S=float, typename Al=std::allocator<S>>
    std::ostream& operator<< (std::ostream& os, const vVector<S, Al>& v)
    {
        os << v.str();
        return os;
    }

    // Operators that can do premultiply, predivide by scaler so you could do,
    // e.g. vVector<float> result = float(1) / vVector<float>({1,2,3});

    //! Scalar * vVector<> (commutative; lhs * rhs == rhs * lhs, so return rhs * lhs)
    template <typename S> vVector<S> operator* (S lhs, const vVector<S>& rhs) { return rhs * lhs; }

    //! Scalar / vVector<>
    template <typename S>
    vVector<S> operator/ (S lhs, const vVector<S>& rhs)
    {
        vVector<S> division(rhs.size(), S{0});
        auto lhs_div_by_vec = [lhs](S coord) { return lhs / coord; };
        std::transform (rhs.begin(), rhs.end(), division.begin(), lhs_div_by_vec);
        return division;
    }

    //! Scalar + vVector<> (commutative)
    template <typename S> vVector<S> operator+ (S lhs, const vVector<S>& rhs) { return rhs + lhs; }

    //! Scalar - vVector<>
    template <typename S>
    vVector<S> operator- (S lhs, const vVector<S>& rhs)
    {
        vVector<S> subtraction(rhs.size(), S{0});
        auto lhs_minus_vec = [lhs](S coord) { return lhs - coord; };
        std::transform (rhs.begin(), rhs.end(), subtraction.begin(), lhs_minus_vec);
        return subtraction;
    }

} // namespace morph
