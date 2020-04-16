#pragma once

#include <vector>
using std::vector;
#include <array>
using std::array;
#include <list>
using std::list;
#include <limits>
using std::numeric_limits;
#include <cmath>
using std::sqrt;
using std::min;
using std::max;
#include <utility>
using std::pair;
using std::make_pair;
#include <iostream>
using std::endl;
using std::cout;
#include <stdexcept>
using std::runtime_error;
#include <algorithm>
using std::min;
using std::max;
#include <type_traits>
#include <memory>
#include "MathConst.h"

/*!
 * Mathematical algorithms in the morph namespace. This file contains default and
 * specialized implementations for different argument types where the arguments fall
 * into two groups; scalar (float or double) and vector (array<float, 3>,
 * vector<float> etc).
 *
 * Client code should only call methods from morph::MathAlgo (see MathAlgo.h).
 */

namespace morph {

    /*!
     * Default MathImpl template contains common vector implementations
     *
     * This special case is for vector implementations which are identical whether
     * they are for resizable vector types or fixed-size vector types.
     */
    template <int vtype = 0>
    struct MathImpl
    {
        //! Resizable and Fixed size vector maxmin implementations are common
        template < template <typename, typename> typename Container,
                   typename T,
                   typename Allocator=std::allocator<T> >
        static pair<T, T> maxmin (const Container<T, Allocator>& values) {

            // Example to get the type of the container T.
            // See https://stackoverflow.com/questions/44521991/type-trait-to-get-element-type-of-stdarray-or-c-style-array
            using T_el = std::remove_reference_t<decltype(*std::begin(std::declval<T&>()))>;

            T vmax = numeric_limits<T>::min();
            T vmin = numeric_limits<T>::max();
            T_el maxlen = 0;
            T_el minlen = numeric_limits<T_el>::max();

            for (auto v : values) {

                // (Vector version compares sqrt (v[0]*v[0] + v[1]*v[1] +...))
                T_el vlen = 0;
                for (auto vi : v) {
                    vlen += vi*vi;
                }
                vlen = sqrt(vlen);

                if (vlen > maxlen) {
                    maxlen = vlen;
                    vmax = v;
                }
                if (vlen < minlen) {
                    minlen = vlen;
                    vmin = v;
                }
            }

            return make_pair (vmax, vmin);
        }

        //! common vector centroid implementation
        template < template <typename, typename> typename Container,
                   typename T,
                   typename Allocator=std::allocator<T> >
        static T centroid (const Container<T, Allocator>& coords) {

            using T_el = std::remove_reference_t<decltype(*std::begin(std::declval<T&>()))>;

            T _centroid(*coords.begin());

            // zero _centroid
            typename T::iterator ci = _centroid.begin();
            while (ci != _centroid.end()) {
                *ci++ = static_cast<T_el>(0);
            }

            typename Container<T, Allocator>::const_iterator conti = coords.begin();
            while (conti != coords.end()) {
                ci = _centroid.begin();
                typename T::const_iterator conti_eli = conti->begin(); // conti element iterator
                while (ci != _centroid.end()) {
                    *ci++ += *conti_eli++;
                }
                ++conti;
            }
            size_t csz = coords.size();
            ci = _centroid.begin();
            while (ci != _centroid.end()) {
                *ci++ /= static_cast<T_el>(csz);
            }

            return _centroid;
        }

        //! Common vector autoscale implementation. Autoscales from min to max.
        template < template <typename, typename> typename Container,
                   typename T,
                   typename Allocator=std::allocator<T>,
                   typename S > // S is a scalar
        static Container<T, Allocator> autoscale (const Container<T, Allocator>& values, S range_min, S range_max) {

            // S and T_el should be the same type. Could put this line as default for
            // typename S above, but what to do for the
            using T_el=std::remove_reference_t<decltype(*std::begin(std::declval<T&>()))>;

            // FIXME: Check S and T_el are same? What's the correct templating approach?

            // autoscale vectors (with fixed size vector aka std::array)
            pair<T,T> mm = MathImpl<0>::maxmin (values);

            // For simplicity with types, avoid using MathAlgo::distance here and
            // write the code out longhand.
            T_el sos = static_cast<T_el>(0);
            typename T::const_iterator vi = mm.first.begin();
            while (vi != mm.first.end()) {
                const T_el val = *vi;
                sos += (val * val); // induces a cryptic -Wmaybe-uninitialized warning.
                ++vi;
            }
            T_el max_v = sqrt (sos); // max length vector

            sos = static_cast<T_el>(0);
            vi = mm.second.begin();
            while (vi != mm.second.end()) {
                const T_el val = *vi;
                sos += (val * val);
                ++vi;
            }
            T_el min_v = sqrt (sos); // min length vector in the input

            // For this vector implementation, min and max are min and max LENGTH of
            // the vectors and are thus always positive and max >= min.
            if (range_min < static_cast<S>(0) || range_min >= range_max) {
                throw runtime_error ("Check your min and max. min >= 0 please and max > min");
            }

            // Static cast redundant here, as the type S should be same as the type T_el
            T_el scale_v = static_cast<T_el>(range_max - range_min) / (max_v - min_v);

            // Return object
            Container<T, Allocator> norm_v (values);

            // Create iterators into the input (values) and the output (norm_v)
            typename Container<T, Allocator>::const_iterator values_i = values.begin();
            typename Container<T, Allocator>::iterator n_i = norm_v.begin();

            while (values_i != values.end()) {
                typename T::const_iterator values_i_eli = values_i->begin(); // values_i element iterator
                typename T::iterator n_i_eli = n_i->begin(); // n_i element iterator

                // We have to measure the length of each vector in values:
                sos = static_cast<T_el>(0);
                while (values_i_eli != values_i->end()) {
                    sos += (*values_i_eli) * (*values_i_eli);
                    ++values_i_eli;
                }
                T_el vec_len = sqrt (sos);

                values_i_eli = values_i->begin();
                while (values_i_eli != values_i->end()) {
                    // length of vector element i
                    T_el el_len = *values_i_eli++;
                    // Here's the scaling of a component
                    *n_i_eli++ = (el_len - (el_len/vec_len)*min_v) * scale_v;
                }
                ++values_i;
                ++n_i;
            }

            return norm_v;
        }
    };

    /*!
     * Implementations of algorithms where 'T' is a dynamically allocated vector-like
     * object, such as vector<>, list<> or queue<>. Each algorithm is a templated
     * static function.
     *
     * The default implementations in this class template are for vectors/lists etc.
     *
     * This is the implementation for "number_type::value is 1". Resizable vector.
     */
    template<>
    struct MathImpl<1>
    {
        //! Resizable vector maxmin implementation
        template < template <typename, typename> typename Container,
                   typename T,
                   typename Allocator=std::allocator<T> >
        static pair<T, T> maxmin (const Container<T, Allocator>& values) {
            // Use the common implementation:
            return MathImpl<0>::maxmin (values);
        }

        //! Resizable vector centroid implementation
        template < template <typename, typename> typename Container,
                   typename T,
                   typename Allocator=std::allocator<T> >
        static T centroid (const Container<T, Allocator>& coords) {
            return MathImpl<0>::centroid (coords);
        }

        //! Resizable vector autoscale implementation
        template < template <typename, typename> typename Container,
                   typename T,
                   typename Allocator=std::allocator<T>,
                   typename S >
        static Container<T, Allocator> autoscale (const Container<T, Allocator>& values, S range_min, S range_max) {
            // autoscale vectors (with variable size vector)
            return MathImpl<0>::autoscale (values, range_min, range_max);
        }
    };

    /*!
     * A 'type 2' type is a fixed size array type, such as std::array or morph::Vector3
     *
     * This is the implementation for "number_type::value is 2" - fixed size.
     */
    template<>
    struct MathImpl<2>
    {
        //! Fixed size vector maxmin implementation
        template < template <typename, typename> typename Container,
                   typename T,
                   typename Allocator=std::allocator<T> >
        static pair<T, T> maxmin (const Container<T, Allocator>& values) {
            // Use the common implementation:
            return MathImpl<0>::maxmin (values);
        }

        //! Fixed size vector centroid implementation
        template < template <typename, typename> typename Container,
                   typename T,
                   typename Allocator=std::allocator<T> >
        static T centroid (const Container<T, Allocator>& coords) {
            return MathImpl<0>::centroid (coords);
        }

        //! Fixed size vector autoscale implementation. Autoscales from min to max.
        template < template <typename, typename> typename Container,
                   typename T,
                   typename Allocator=std::allocator<T>,
                   typename S > // S is a scalar
        static Container<T, Allocator> autoscale (const Container<T, Allocator>& values, S range_min, S range_max) {
            return MathImpl<0>::autoscale (values, range_min, range_max);
        }
    };

    /*!
     * Implementations of algorithms taking T as a scalar-like object, such as float
     * or double.
     *
     * number_type::value is 3 - scalar
     */
    template<>
    struct MathImpl<3>
    {
        //! Scalar maxmin implementation
        template < template <typename, typename> typename Container,
                   typename T,
                   typename Allocator=std::allocator<T> >
        static pair<T, T> maxmin (const Container<T, Allocator>& values) {
            T vmax = numeric_limits<T>::lowest();
            T vmin = numeric_limits<T>::max();
            for (auto v : values) {
                vmax = v > vmax ? v : vmax;
                vmin = v < vmin ? v : vmin;
            }
            return make_pair (vmax, vmin);
        }

#if 0
        //! Scalar centroid implementation. This throws runtime exception. Really it'd
        //! be better to omit it to give a compiler error.
        template < template <typename, typename> typename Container,
                   typename T,
                   typename Allocator=std::allocator<T> >
        static T centroid (const Container<T, Allocator>& coords) {
            throw runtime_error ("Call this with Container<T>& coords where T is a vector/array type");
        }
#endif
        //! Scalar autoscale implementation
        template < template <typename, typename> typename Container,
                   typename T,
                   typename Allocator=std::allocator<T>,
                   typename S > // FIXME: Check T==S?
        static Container<T, Allocator> autoscale (const Container<T, Allocator>& values, S range_min, S range_max) {
            pair<T,T> mm = MathImpl<3>::maxmin (values);
            T min_v = mm.second;
            T max_v = mm.first;
            T scale_v = (range_max - range_min) / (max_v - min_v);
            vector<T> norm_v(values.size());
            for (unsigned int i = 0; i<values.size(); ++i) {
                norm_v[i] = std::min (std::max (((values[i]) - min_v) * scale_v, static_cast<T>(0.0)), static_cast<T>(1.0));
            }
            return norm_v;
        }

    };

} // namespace morph
