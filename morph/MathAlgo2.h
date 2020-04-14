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
#include <type_traits>
using std::is_scalar;

#include "MathConst.h"

/*!
 * Mathematical algorithms in the morph namespace.
 */

namespace morph {

    /*!
     * Implementations of algorithms where 'T' is a vector-like object, such as
     * array<> or vector<>.
     */
    template <bool b>
    struct MathImpl // default (is_scalar will have been false)
    {
        template<typename T>
        static pair<T, T> maxmin (const vector<T>& values) {
            cout << "T is vector" << endl;

            // Example to get the type of the container T.
            // See https://stackoverflow.com/questions/44521991/type-trait-to-get-element-type-of-stdarray-or-c-style-array
            using element_type_t = std::remove_reference_t<decltype(*std::begin(std::declval<T&>()))>;

            T max;
            T min;
            element_type_t maxlen = 0;
            element_type_t minlen = numeric_limits<element_type_t>::max();

            for (auto v : values) {

                // (Vector version compares sqrt (v[0]*v[0] + v[1]*v[1] +...))
                element_type_t vlen = 0;
                for (auto vi : v) {
                    vlen += vi*vi;
                }
                vlen = sqrt(vlen);

                if (vlen > maxlen) {
                    maxlen = vlen;
                    max = v;
                }
                if (vlen < minlen) {
                    minlen = vlen;
                    min = v;
                }
            }

            return make_pair (max, min);
        }
    };

    /*!
     * Implementations of algorithms taking T as a scalar-like object, such as float
     * or double.
     */
    template<>
    struct MathImpl<true> // is_scalar was true
    {
        template<typename T>
        static pair<T, T> maxmin (const vector<T>& values) {
            cout << "T is scalar" << endl;
            T max = numeric_limits<T>::lowest();
            T min = numeric_limits<T>::max();
            for (auto v : values) {
                max = v > max ? v : max;
                min = v < min ? v : min;
            }
            return make_pair (max, min);
        }
    };

    /*!
     * The new MathAlgo class with its methods.
     */
    struct MathAlgo
    {
        template <typename T>
        static pair<T,T> maxmin (vector<T>& vec) {
            return MathImpl<is_scalar<T>::value>::maxmin (vec);
        }
    };
}
