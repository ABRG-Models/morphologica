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
#include <algorithm>
#include <functional>
#include <morph/Random.h>

namespace morph {

    /*!
     * \brief N-D vector class deriving from std::vector
     *
     * An N dimensional vector class template which derives from std::vector. Vector
     * components are of scalar type S. It is anticipated that S will be set either to
     * floating point scalar types such as float or double, or to integer scalar types
     * such as int, long long int and so on. Thus, a typical (and in fact, the default)
     * signature would be:
     *
     *\code{.cpp}
     * vVector<float> v(3);
     *\endcode
     *
     * The class inherits std:vector's dynamnically-resizeable memory for storing the
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
        S x() const
        {
            return (*this)[0];
        }
        //! \return the second component of the vector
        S y() const
        {
            return (*this)[1];
        }
        //! \return the third component of the vector
        S z() const
        {
            return (*this)[2];
        }
        //! \return the fourth component of the vector
        S w() const
        {
            return (*this)[3];
        }

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

        /*!
         * Set data members from an std::vector (may not be necessary?)
         */
        template <typename _S=S>
        void set_from (const std::vector<_S>& vec)
        {
            std::copy (vec.begin(), vec.end(), this->begin());
        }

        /*!
         * Set data members from an std::array, matching the size of the array first.
         */
        template <typename _S=S, size_t N>
        void set_from (const std::array<_S, N>& ar)
        {
            this->resize(N);
            std::copy (ar.begin(), ar.end(), this->begin());
        }

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
         * A function to set the value of each element of the vector.
         */
        template <typename _S=S>
        void set (const _S& val)
        {
            std::fill (this->begin(), this->end(), val);
        }

        //! Stream the coordinates of the vector into \a ss as a comma separated list.
        void str_comma_separated (std::stringstream& ss) const
        {
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

        /*!
         * Renormalize the vector to length 1.0. Only for S types that are floating point.
         */
        template <typename F=S, std::enable_if_t<!std::is_integral<std::decay_t<F>>::value, int> = 0 >
        void renormalize()
        {
            auto add_squared = [](F a, F b) { return a + b * b; };
            const F denom = std::sqrt (std::accumulate (this->begin(), this->end(), F{0}, add_squared));
            if (denom != F{0}) {
                F oneovermag = F{1} / denom;
                auto x_oneovermag = [oneovermag](F f) { return f * oneovermag; };
                std::transform (this->begin(), this->end(), this->begin(), x_oneovermag);
            }
        }

        /*!
         * Zero the vector. Set all coordinates to 0
         */
        void zero()
        {
            std::fill (this->begin(), this->end(), S{0});
        }

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
            for (auto& i : *this) {
                i = ru.get();
            }
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
            for (auto& i : *this) {
                i = ru.get();
            }
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
            for (auto& i : *this) {
                i = rn.get();
            }
        }

        /*!
         * Test to see if this vector is a unit vector (it doesn't *have* to be).
         *
         * \return true if the length of the vector is 1.
         */
        bool checkunit() const
        {
            auto subtract_squared = [](S a, S b) { return a - b * b; };
            const S metric = std::accumulate (this->begin(), this->end(), S{1}, subtract_squared);
            if (std::abs(metric) > vVector<S>::unitThresh) {
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
         * Return the value of the longest component of the vector.
         */
        S longest() const
        {
            auto abs_compare = [](S a, S b) { return (std::abs(a) < std::abs(b)); };
            auto thelongest = std::max_element (this->begin(), this->end(), abs_compare);
            S rtn = *thelongest;
            return rtn;
        }

        /*!
         * Return the index of the longest component of the vector.
         */
        size_t arglongest() const
        {
            auto abs_compare = [](S a, S b) { return (std::abs(a) < std::abs(b)); };
            auto thelongest = std::max_element (this->begin(), this->end(), abs_compare);
            size_t idx = (thelongest - this->begin());
            return idx;
        }

        /*!
         * Return the value of the shortest component of the vector.
         */
        S shortest() const
        {
            auto abs_compare = [](S a, S b) { return (std::abs(a) > std::abs(b)); };
            auto theshortest = std::max_element (this->begin(), this->end(), abs_compare);
            S rtn = *theshortest;
            return rtn;
        }

        /*!
         * Return the index of the shortest component of the vector.
         */
        size_t argshortest() const
        {
            auto abs_compare = [](S a, S b) { return (std::abs(a) > std::abs(b)); };
            auto theshortest = std::max_element (this->begin(), this->end(), abs_compare);
            size_t idx = (theshortest - this->begin());
            return idx;
        }

        /*!
         * Return the value of the maximum (most positive) component of the vector.
         */
        S max() const
        {
            auto themax = std::max_element (this->begin(), this->end());
            S rtn = *themax;
            return rtn;
        }

        /*!
         * Return the index of the maximum (most positive) component of the vector.
         */
        size_t argmax() const
        {
            auto themax = std::max_element (this->begin(), this->end());
            size_t idx = (themax - this->begin());
            return idx;
        }

        /*!
         * Return the value of the minimum (smallest or most negative) component of the vector.
         */
        S min() const
        {
            auto themin = std::min_element (this->begin(), this->end());
            S rtn = *themin;
            return rtn;
        }

        /*!
         * Return the index of the minimum (smallest or most negative) component of the vector.
         */
        size_t argmin() const
        {
            auto themin = std::min_element (this->begin(), this->end());
            size_t idx = (themin - this->begin());
            return idx;
        }

        /*!
         * Compute the element-wise pth power of the vector
         *
         * \return a vVector whose elements have been raised to the power p
         */
        vVector<S> pow (const S& p) const
        {
            // To get power in-place:
            //for (auto& i : *this) { i = std::pow (i, p); }
            vVector<S> rtn(this->size());
            auto raise_to_p = [p](S coord) { return std::pow(coord, p); };
            std::transform (this->begin(), this->end(), rtn.begin(), raise_to_p);
            return rtn;
        }
        //! Raise each element to the power p
        void pow_inplace (const S& p) { for (auto& i : *this) { i = std::pow (i, p); } }

        /*!
         * Compute the element-wise square root of the vector
         *
         * \return a vVector whose elements have been square-rooted
         */
        vVector<S> sqrt() const
        {
            vVector<S> rtn(this->size());
            auto sqrt_element = [](S coord) { return std::sqrt(coord); };
            std::transform (this->begin(), this->end(), rtn.begin(), sqrt_element);
            return rtn;
        }
        //! Replace each element with its own square root
        void sqrt_inplace() { for (auto& i : *this) { i = std::sqrt (i); } }

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
        bool operator!() const
        {
            return (this->length() == S{0}) ? true : false;
        }

#if 0 // Haven't figured this out yet
        //! Assignment from std::vector
        template <typename _S=S, typename _Al=Al>
        vVector<S, Al>& operator= (const std::vector<_S, _Al>& v)
        {
            return std::vector<_S, _Al>::operator=(v);
            // or:
            //this->resize (v.size());
            //std::copy (v.begin(), v.end(), this->begin());
            //return *this;
        }
#endif

#if 0 // Haven't figured this out yet
        //! Assignment from std::array
        template <typename _S=S, size_t N>
        vVector<S>& operator= (const std::array<_S, N>& ar)
        {
            this->resize (N);
            std::copy (ar.begin(), ar.end(), this->begin());
            return *this;
        }
#endif

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
            std::transform (v.begin(), v.end(), this->begin(), rtn.begin(), std::multiplies<S>());
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
                std::transform (v.begin(), v.end(), this->begin(), this->begin(), std::multiplies<S>());
            } else {
                throw std::runtime_error ("vVector::operator*=: Hadamard product is defined here for vectors of same dimensionality only");
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
            auto mult_by_s = [s](S coord) { return coord * s; };
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
            auto mult_by_s = [s](S coord) { return coord * s; };
            std::transform (this->begin(), this->end(), this->begin(), mult_by_s);
        }

        /*!
         * Scalar divide by s
         */
        template <typename _S=S, std::enable_if_t<std::is_scalar<std::decay_t<_S>>::value, int> = 0 >
        vVector<S> operator/ (const _S& s) const
        {
            vVector<S> rtn(this->size());
            auto div_by_s = [s](S coord) { return coord / s; };
            std::transform (this->begin(), this->end(), rtn.begin(), div_by_s);
            return rtn;
        }

        /*!
         * Scalar divide by s
         */
        template <typename _S=S, std::enable_if_t<std::is_scalar<std::decay_t<_S>>::value, int> = 0 >
        void operator/= (const _S& s)
        {
            auto div_by_s = [s](S coord) { return coord / s; };
            std::transform (this->begin(), this->end(), this->begin(), div_by_s);
        }

        /*!
         * vVector addition operator
         */
        template<typename _S=S>
        vVector<S> operator+ (const vVector<_S>& v) const
        {
            vVector<S> vrtn(this->size());
            auto vi = v.begin();
            auto add_v = [vi](S a) mutable { return a + (*vi++); };
            std::transform (this->begin(), this->end(), vrtn.begin(), add_v);
            return vrtn;
        }

        /*!
         * vVector addition operator
         */
        template<typename _S=S>
        void operator+= (const vVector<_S>& v)
        {
            auto vi = v.begin();
            auto add_v = [vi](S a) mutable { return a + (*vi++); };
            std::transform (this->begin(), this->end(), this->begin(), add_v);
        }

        /*!
         * Vector subtraction operator
         */
        template<typename _S=S>
        vVector<S> operator- (const vVector<_S>& v) const
        {
            vVector<S> vrtn(this->size());
            auto vi = v.begin();
            auto subtract_v = [vi](S a) mutable { return a - (*vi++); };
            std::transform (this->begin(), this->end(), vrtn.begin(), subtract_v);
            return vrtn;
        }

        /*!
         * Vector subtraction operator
         */
        template<typename _S=S>
        void operator-= (const vVector<_S>& v)
        {
            auto vi = v.begin();
            auto subtract_v = [vi](S a) mutable { return a - (*vi++); };
            std::transform (this->begin(), this->end(), this->begin(), subtract_v);
        }

        /*!
         * Scalar addition
         */
        template <typename _S=S, std::enable_if_t<std::is_scalar<std::decay_t<_S>>::value, int> = 0 >
        vVector<S> operator+ (const _S& s) const
        {
            vVector<S> rtn(this->size());
            auto add_s = [s](S coord) { return coord + s; };
            std::transform (this->begin(), this->end(), rtn.begin(), add_s);
            return rtn;
        }

        /*!
         * Scalar addition
         */
        template <typename _S=S, std::enable_if_t<std::is_scalar<std::decay_t<_S>>::value, int> = 0 >
        void operator+= (const _S& s)
        {
            auto add_s = [s](S coord) { return coord + s; };
            std::transform (this->begin(), this->end(), this->begin(), add_s);
        }

        /*!
         * Scalar subtraction
         */
        template <typename _S=S, std::enable_if_t<std::is_scalar<std::decay_t<_S>>::value, int> = 0 >
        vVector<S> operator- (const _S& s) const
        {
            vVector<S> rtn;
            auto subtract_s = [s](S coord) { return coord - s; };
            std::transform (this->begin(), this->end(), rtn.begin(), subtract_s);
            return rtn;
        }

        /*!
         * Scalar subtraction
         */
        template <typename _S=S, std::enable_if_t<std::is_scalar<std::decay_t<_S>>::value, int> = 0 >
        void operator-= (const _S& s)
        {
            auto subtract_s = [s](S coord) { return coord - s; };
            std::transform (this->begin(), this->end(), this->begin(), subtract_s);
        }

        /*!
         * Overload the stream output operator
         */
        friend std::ostream& operator<< <S> (std::ostream& os, const vVector<S>& v);
    };

    template <typename S=float, typename Al=std::allocator<S>>
    std::ostream& operator<< (std::ostream& os, const vVector<S, Al>& v)
    {
        os << v.str();
        return os;
    }

} // namespace morph
