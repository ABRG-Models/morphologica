/*!
 * An N dimensional vector class template which derives from std::array.
 *
 * Author: Seb James
 * Date: April 2020
 */
#pragma once

#include <cmath>
using std::abs;
using std::sqrt;

#include <array>
using std::array;

#include <iostream>
using std::cout;
using std::endl;
using std::ostream;

#include <string>
using std::string;
#include <sstream>
using std::stringstream;

#include <type_traits>
using std::enable_if;
using std::enable_if_t;
using std::is_integral;
using std::is_scalar;
using std::decay_t;

#include "Random.h"
using morph::RandUniformReal;
using morph::RandUniformInt;

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
    template <typename S, size_t N> ostream& operator<< (ostream&, const Vector<S, N>&);

    template <typename S=float, size_t N=3>
    struct Vector : public array<S, N>
    {
        //! \return the first component of the vector
        template <size_t _N = N, enable_if_t<(_N>0), int> = 0>
        S x (void) const {
            return (*this)[0];
        }
        //! \return the second component of the vector
        template <size_t _N = N, enable_if_t<(_N>1), int> = 0>
        S y (void) const {
            return (*this)[1];
        }
        //! \return the third component of the vector
        template <size_t _N = N, enable_if_t<(_N>2), int> = 0>
        S z (void) const {
            return (*this)[2];
        }
        //! \return the fourth component of the vector
        template <size_t _N = N, enable_if_t<(_N>3), int> = 0>
        S w (void) const {
            return (*this)[3];
        }

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
         */
        static constexpr S unitThresh = 0.001;

        /*!
         * Set data members from an array the of same size and type.
         */
        void setFrom (const array<S, N> v) {
            for (size_t i = 0; i < N; ++i) {
                (*this)[i] = v[i];
            }
        }

        /*!
         * Set the data members of this Vector from the passed in, larger vector, v,
         * ignoring the last element of v. Used when working with 4D vectors in graphics
         * applications involving 4x4 transform matrices.
         */
        void setFrom (const array<S, (N+1)> v) {
            for (size_t i = 0; i < N; ++i) {
                (*this)[i] = v[i];
            }
        }

        /*!
         * Output the vector to stdout
         */
        void output (void) const {
            cout << "Vector" << this->asString();
        }

        /*!
         * Create a string representation of the vector
         *
         * \return A 'coordinate format' string such as "(1,1,2)", "(0.2,0.4)" or
         * "(5,4,5,5,40)".
         */
        string asString (void) const {
            stringstream ss;
            auto i = this->begin();
            ss << "(";
            bool first = true;
            while (i != this->end()) {
                if (first) {
                    ss << *i++;
                    first = false;
                } else {
                    ss << "," << *i++;
                }
            }
            ss << ")";
            return ss.str();
        }

        /*!
         * Renormalize the vector to length 1.
         */
        void renormalize (void) {
            S denom = static_cast<S>(0);
            auto i = this->begin();
            while (i != this->end()) {
                denom += ((*i) * (*i));
                ++i;
            }
            denom = sqrt(denom);
            if (denom != static_cast<S>(0.0)) {
                S oneovermag = static_cast<S>(1.0) / denom;
                i = this->begin();
                while (i != this->end()) {
                    *i++ *= oneovermag;
                }
            }
        }

        /*!
         * Randomize the vector
         *
         * Randomly set the elements of the vector consisting of floating point
         * coordinates. Coordinates are set to random numbers drawn from a uniform
         * distribution between 0 and 1 (See morph::RandUniformReal for details).
         *
         * Note that I need a real or int implementation here, depending on the type of
         * S. This allows me to use the correct type of randomizer.
         *
         * Note, if you omit the second template arg from enable_if_t (or enable_if)
         * then the type defaults to void.
         */
        template <typename F=S, enable_if_t<!is_integral<decay_t<F>>::value, int> = 0 >
        void randomize (void) {
            RandUniformReal<F> ruf (static_cast<F>(0), static_cast<F>(1));
            auto i = this->begin();
            while (i != this->end()) {
                *i++ = ruf.get();
            }
        }

        /*!
         * Randomize the vector
         *
         * Randomly set the elements of the vector consisting of integer
         * coordinates. Coordinates are set to random numbers drawn from a uniform
         * distribution between 0 and 255 (See morph::RandUniformInt for details).
         *
         * Note on the template syntax: Here, if F is integral, then enable_if_t's type
         * is '0' and the function is defined (I think).
        */
        template <typename F=S, enable_if_t<is_integral<decay_t<F>>::value, int> = 0 >
        void randomize (void) {
            RandUniformInt<F> rui (static_cast<F>(0), static_cast<F>(255));
            auto i = this->begin();
            while (i != this->end()) {
                *i++ = rui.get();
            }
        }

        /*!
         * Test to see if this vector is a unit vector (it doesn't *have* to be).
         *
         * \return true if the length of the vector is 1.
         */
        bool checkunit (void) {
            bool rtn = true;
            S metric = 1.0;
            auto i = this->begin();
            while (i != this->end()) {
                metric -= ((*i) * (*i));
                ++i;
            }
            if (abs(metric) > morph::Vector<S, N>::unitThresh) {
                rtn = false;
            }
            return rtn;
        }

        /*!
         * Find the length of the vector.
         *
         * \return the length
         */
        S length (void) const {
            S sos = static_cast<S>(0);
            auto i = this->begin();
            while (i != this->end()) {
                sos += ((*i) * (*i));
                ++i;
            }
            return sqrt(sos);
        }

        /*!
         * Unary negate operator
         *
         * \return a Vector whose elements have been negated.
         */
        Vector<S, N> operator- (void) const {
            Vector<S, N> rtn;
            auto i = this->begin();
            auto j = rtn.begin();
            while (i != this->end()) {
                *j++ = -(*i++);
            }
            return rtn;
        }

        /*!
         * Unary not operator.
         *
         * \return true if the vector length is 0, otherwise it returns false.
         */
        bool operator! (void) const {
            return (this->length() == static_cast<S>(0.0)) ? true : false;
        }

        /*!
         * Vector multiply * operator.
         *
         * Cross product of this with another vector v2 (if N==3). In
         * higher dimensions, its more complicated to define what the cross product is,
         * and I'm unlikely to need anything other than the plain old 3D cross product.
         */
        template <size_t _N = N, enable_if_t<(_N==3), int> = 0>
        Vector<S, N> operator* (const Vector<S, _N>& v2) const {
            Vector<S, _N> v;
            v[0] = (*this)[1] * v2.z() - (*this)[2] * v2.y();
            v[1] = (*this)[2] * v2.x() - (*this)[0] * v2.z();
            v[2] = (*this)[0] * v2.y() - (*this)[1] * v2.x();
            return v;
        }

        /*!
         * Vector multiply *= operator.
         *
         * Cross product of this with another vector v2 (if N==3). Result written into
         * this.
         */
        template <size_t _N = N, enable_if_t<(_N==3), int> = 0>
        void operator*= (const Vector<S, _N>& v2) {
            Vector<S, _N> v;
            v[0] = (*this)[1] * v2.z() - (*this)[2] * v2.y();
            v[1] = (*this)[2] * v2.x() - (*this)[0] * v2.z();
            v[2] = (*this)[0] * v2.y() - (*this)[1] * v2.x();
            (*this)[0] = v[0];
            (*this)[1] = v[1];
            (*this)[2] = v[2];
        }

        /*!
         * \brief Scalar (dot) product
         *
         * Compute the scalar product of this Vector and the Vector, v2.
         *
         * \return scalar product
         */
        S dot (const Vector<S, N>& v2) const {
            S rtn = static_cast<S>(0);
            auto i = this->begin();
            auto j = v2.begin();
            while (i != this->end()) {
                rtn += ((*i++) * (*j++));
            }
            return rtn;
        }

        /*!
         * Scalar multiply * operator
         *
         * This function will only be defined if typename _S is a
         * scalar type. Multiplies this Vector<S, N> by s, element-wise.
         */
        template <typename _S=S, enable_if_t<is_scalar<decay_t<_S>>::value, int> = 0 >
        Vector<S, N> operator* (const _S& s) const {
            Vector<S, N> rtn;
            auto val = this->begin();
            auto rval = rtn.begin();
            // Here's a way to iterate through which the compiler should be able to
            // autovectorise; it knows what i is on each loop:
            for (size_t i = 0; i < N; ++i) {
                *(rval+i) = *(val+i) * static_cast<S>(s);
            }
            return rtn;
        }

        /*!
         * Scalar multiply *= operator
         *
         * This function will only be defined if typename _S is a
         * scalar type. Multiplies this Vector<S, N> by s, element-wise.
         */
        template <typename _S=S, enable_if_t<is_scalar<decay_t<_S>>::value, int> = 0 >
        void operator*= (const _S& s) {
            auto val = this->begin();
            for (size_t i = 0; i < N; ++i) {
                *(val+i) *= static_cast<S>(s);
            }
        }

        /*!
         * Scalar division * operator
         */
        template <typename _S=S, enable_if_t<is_scalar<decay_t<_S>>::value, int> = 0 >
        Vector<S, N> operator/ (const _S& s) const {
            Vector<S, N> rtn;
            auto val = this->begin();
            auto rval = rtn.begin();
            for (size_t i = 0; i < N; ++i) {
                *(rval+i) = *(val+i) / static_cast<S>(s);
            }
            return rtn;
        }

        /*!
         * Scalar division *= operator
         */
        template <typename _S=S, enable_if_t<is_scalar<decay_t<_S>>::value, int> = 0 >
        void operator/= (const _S& s) {
            auto val = this->begin();
            for (size_t i = 0; i < N; ++i) {
                *(val+i) /= static_cast<S>(s);
            }
        }

        /*!
         * Vector addition operator
         */
        Vector<S, N> operator+ (const Vector<S, N>& v2) const {
            Vector<S, N> v;
            auto val = this->begin();
            auto val2 = v2.begin();
            for (size_t i = 0; i < N; ++i) {
                v[i] = *(val+i) + *(val2+i);
            }
            return v;
        }

        /*!
         * Vector addition operator
         */
        void operator+= (const Vector<S, N>& v2) {
            auto val = this->begin();
            auto val2 = v2.begin();
            for (size_t i = 0; i < N; ++i) {
                *(val+i) += *(val2+i);
            }
        }

        /*!
         * Vector subtraction
         */
        Vector<S, N> operator- (const Vector<S, N>& v2) const {
            Vector<S, N> v;
            auto val = this->begin();
            auto val2 = v2.begin();
            for (size_t i = 0; i < N; ++i) {
                v[i] = *(val+i) - *(val2+i);
            }
            return v;
        }

        /*!
         * Vector subtraction
         */
        void operator-= (const Vector<S, N>& v2) {
            auto val = this->begin();
            auto val2 = v2.begin();
            for (size_t i = 0; i < N; ++i) {
                *(val+i) -= *(val2+i);
            }
        }

        /*!
         * Scalar addition
         */
        template <typename _S=S, enable_if_t<is_scalar<decay_t<_S>>::value, int> = 0 >
        Vector<S, N> operator+ (const _S& s) const {
            Vector<S, N> rtn;
            auto val = this->begin();
            auto rval = rtn.begin();
            for (size_t i = 0; i < N; ++i) {
                *(rval+i) = *(val+i) + static_cast<S>(s);
            }
            return rtn;
        }

        /*!
         * Scalar addition
         */
        template <typename _S=S, enable_if_t<is_scalar<decay_t<_S>>::value, int> = 0 >
        void operator+= (const _S& s) {
            auto val = this->begin();
            for (size_t i = 0; i < N; ++i) {
                *(val+i) += static_cast<S>(s);
            }
        }

        /*!
         * Scalar subtraction
         */
        template <typename _S=S, enable_if_t<is_scalar<decay_t<_S>>::value, int> = 0 >
        Vector<S, N> operator- (const _S& s) const {
            Vector<S, N> rtn;
            auto val = this->begin();
            auto rval = rtn.begin();
            for (size_t i = 0; i < N; ++i) {
                *(rval+i) = *(val+i) - static_cast<S>(s);
            }
            return rtn;
        }

        /*!
         * Scalar subtraction
         */
        template <typename _S=S, enable_if_t<is_scalar<decay_t<_S>>::value, int> = 0 >
        void operator-= (const _S& s) {
            auto val = this->begin();
            for (size_t i = 0; i < N; ++i) {
                *(val+i) -= static_cast<S>(s);
            }
        }

        /*!
         * Overload the stream output operator
         */
        friend ostream& operator<< <S, N> (ostream& os, const Vector<S, N>& v);
    };

    template <typename S=float, size_t N=3>
    ostream& operator<< (ostream& os, const Vector<S, N>& v)
    {
        os << v.asString();
        return os;
    }

} // namespace morph
