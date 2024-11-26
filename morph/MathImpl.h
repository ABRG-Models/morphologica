/*!
 * \file
 *
 * This file defines MathImpl, which implements some of the mathematical algorithms
 * from MathAlgo. This file contains default and specialized implementations for
 * different argument types where the arguments fall into two groups; scalar (float or
 * double) and vector (array<float, 3>, vector<float> etc).
 *
 * Client code should only call methods from morph::MathAlgo (see MathAlgo.h).
 *
 * \author Seb James
 * \date April 2020
 */

#pragma once

#include <vector>
#include <limits>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <type_traits>
#include <memory>
#include <morph/vec.h>
#include <morph/range.h>
#include <morph/mathconst.h>
#include <morph/trait_tests.h>

namespace morph {

    /*!
     * Vector MathAlgo algorithm specializations
     *
     * This default MathImpl template contains common vector implementations of
     * algorithms which are exposed by morph::MathAlgo (client code should always
     * select functions from MathAlgo).
     *
     * This is a templated class, with an integer template argument (vtype). That
     * integer allows specializations of this class with alternative implementations
     * of the functions.
     *
     * This default case is for 'vector' implementations; those for which the type T
     * is some sort of vector type such as std::array or std::vector.
     */
    template <int vtype = 0>
    struct MathImpl
    {
        //! Resizable and Fixed size vector maxmin implementations are common
        template <typename Container, std::enable_if_t<morph::is_copyable_container<Container>::value, int> = 0>
        static morph::range<typename Container::value_type> maxmin (const Container& values)
        {
            using T = typename Container::value_type;
            // Example to get the type of the container T.
            // See https://stackoverflow.com/questions/44521991/type-trait-to-get-element-type-of-stdarray-or-c-style-array
            using T_el = std::remove_reference_t<decltype(*std::begin(std::declval<T&>()))>;

            morph::range<T> r (std::numeric_limits<T>::max(), std::numeric_limits<T>::min());
            T_el maxlen = 0;
            T_el minlen = std::numeric_limits<T_el>::max();

            for (auto v : values) {

                // (Vector version compares sqrt (v[0]*v[0] + v[1]*v[1] +...))
                T_el vlen = 0;
                for (auto vi : v) { vlen += vi*vi; }
                vlen = std::sqrt(vlen);

                if (vlen > maxlen) {
                    maxlen = vlen;
                    r.max = v;
                }
                if (vlen < minlen) {
                    minlen = vlen;
                    r.min = v;
                }
            }

            return r;
        }

        //! common vector centroid implementation
        template < template <typename, typename> typename Container,
                   typename T,
                   typename Allocator=std::allocator<T> >
        static T centroid (const Container<T, Allocator>& coords)
        {
            T _centroid(*coords.begin());

            // If T has a .x and a .y as member attributes, assume it's a 2D point (e.g. cv::Point)
            if constexpr (has_xy_members<T>::value == true) {

                // using T_el = what? how to get the element type in a cv::Point_?
                using T_el = std::remove_reference_t<decltype(T::x)>;

                _centroid.x = T_el{0};
                _centroid.y = T_el{0};

                typename Container<T, Allocator>::const_iterator conti = coords.begin();
                while (conti != coords.end()) {
                    _centroid.x += conti->x;
                    _centroid.y += conti->y;
                    ++conti;
                }

                // Maybe T_el instead of double here?
                T_el csz = static_cast<T_el>(coords.size());
                _centroid.x /= csz;
                _centroid.y /= csz;

            } else { // This code is the default for iterable Ts (any number of dimensions)

                using T_el = std::remove_reference_t<decltype(*std::begin(std::declval<T&>()))>;

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
                typename Container<T, Allocator>::size_type csz = coords.size();
                ci = _centroid.begin();
                while (ci != _centroid.end()) {
                    *ci++ /= static_cast<T_el>(csz);
                }
            }

            return _centroid;
        }

        //! Common vector autoscale implementation. Autoscales from min to max.
        template < template <typename, typename> typename Container,
                   typename T,
                   typename Allocator=std::allocator<T>,
                   typename S > // S is a scalar
        static Container<T, Allocator> autoscale (const Container<T, Allocator>& values, S range_min, S range_max)
        {
            // S and T_el should be the same type. Could put this line as default for
            // typename S above, but what to do for the
            using T_el=std::remove_reference_t<decltype(*std::begin(std::declval<T&>()))>;

            // FIXME: Check S and T_el are same? What's the correct templating approach?

            // autoscale vectors (with fixed size vector aka std::array)
            morph::range<T> r = MathImpl<0>::maxmin (values);

            // For simplicity with types, avoid using MathAlgo::distance here and
            // write the code out longhand.
            T_el sos = static_cast<T_el>(0);
            typename T::const_iterator vi = r.max.begin();
            while (vi != r.max.end()) {
                const T_el val = *vi;
                sos += (val * val);
                ++vi;
            }
            T_el max_v = std::sqrt (sos); // max length vector

            sos = static_cast<T_el>(0);
            vi = r.min.begin();
            while (vi != r.min.end()) {
                const T_el val = *vi;
                sos += (val * val);
                ++vi;
            }
            T_el min_v = std::sqrt (sos); // min length vector in the input

            // For this vector implementation, min and max are min and max LENGTH of
            // the vectors and are thus always positive and max >= min.
            if (range_min < static_cast<S>(0) || range_min >= range_max) {
                throw std::runtime_error ("Check your min and max. min >= 0 please and max > min");
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
                T_el vec_len = std::sqrt (sos);

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
     * Scalar MathAlgo algorithm specializations
     *
     * This is a specialization of MathImpl with vtype set to 1. The templates are
     * applied if the type T is a scalar such as float or double.
     *
     * This specialization contains scalar implementations of algorithms which are
     * exposed by morph::MathAlgo (client code should always select functions from
     * MathAlgo).
     *
     * number_type::value will have been 1 - scalar (see morph::number_type in trait_tests.h).
     */
    template<>
    struct MathImpl<1>
    {
        //! Scalar maxmin implementation
        template <typename Container, std::enable_if_t<morph::is_copyable_container<Container>::value, int> = 0>
        static morph::range<typename Container::value_type> maxmin (const Container& values)
        {
            using T = typename Container::value_type;
            morph::range<T> r (std::numeric_limits<T>::max(), std::numeric_limits<T>::lowest());
            for (auto v : values) {
                r.max = v > r.max ? v : r.max;
                r.min = v < r.min ? v : r.min;
            }
            return r;
        }

        //! Scalar autoscale implementation
        template < template <typename, typename> typename Container,
                   typename T,
                   typename Allocator=std::allocator<T>,
                   typename S > // FIXME: Check T==S?
        static Container<T, Allocator> autoscale (const Container<T, Allocator>& values, S range_min, S range_max)
        {
            morph::range<T> r = MathImpl<1>::maxmin (values);
            T scale_v = (range_max - range_min) / (r.max - r.min);
            std::vector<T> norm_v(values.size());
            for (unsigned int i = 0; i<values.size(); ++i) {
                norm_v[i] = std::min (std::max (((values[i]) - r.min) * scale_v, T{0}), T{1});
            }
            return norm_v;
        }

    };

    /*!
     * Complex scalar MathAlgo algorithm specializations
     *
     * This is a specialization of MathImpl with vtype set to 2. The templates are applied if the
     * type T is a complex scalar such as std::complex<float> or std::complex<double>.
     *
     * This specialization contains complex scalar implementations of algorithms which are exposed
     * by morph::MathAlgo (client code should always select functions from MathAlgo).
     *
     * number_type::value will have been 2 - complex scalar (see morph::number_type in trait_tests.h).
     */
    template<>
    struct MathImpl<2>
    {
        //! Complex scalar maxmin implementation
        template <typename Container, std::enable_if_t<morph::is_copyable_container<Container>::value, int> = 0>
        static morph::range<typename Container::value_type> maxmin (const Container& values)
        {
            using T = typename Container::value_type;
            using T_el = typename T::value_type; // If T is std::complex<float>, T_el will be float
            // Note that there's no specialization of numeric_limits for std::complex, so set it up manually
            morph::range<T> r ({std::numeric_limits<T_el>::max(), std::numeric_limits<T_el>::max() }, T{0, 0});
            for (auto v : values) {
                // comparison operations on complex numbers commonly consider their modulus - how
                // far the number is from the origin.
                r.max = std::abs(v) > std::abs(r.max) ? v : r.max;
                r.min = std::abs(v) < std::abs(r.min) ? v : r.min;
            }
            return r;
        }
    };

} // namespace morph
