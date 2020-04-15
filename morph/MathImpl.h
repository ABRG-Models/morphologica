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
#include <type_traits>

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
     * Implementations of algorithms where 'T' is a dynamically allocated vector-like
     * object, such as vector<>, list<> or queue<>. Each algorithm is a templated
     * static function.
     *
     * The default implementations in this class template are for vectors/lists etc.
     *
     * This is the implementation for "number_type::value is 0". Resizable vector.
     */
    template <int vtype>
    struct MathImpl
    {
        //! Find the max and min length vector values and return in a pair.
        template<typename T>
        static pair<T, T> maxmin (const vector<T>& values) {
            cout << "T is resizable 'vector'" << endl;

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

        // First implementation is for resizable vectors
        template <typename T>
        static T centroid (const vector<T>& coords) {

            using element_type_t = std::remove_reference_t<decltype(*std::begin(std::declval<T&>()))>;

            cout << "T is resizable 'vector'" << endl;
            T _centroid;

            // Return if vector<T> is empty
            if (coords.empty()) {
                return _centroid;
            }

            // zero _centroid
            size_t vsz = coords.begin()->size();

            // This is the resizable version
            _centroid.resize (vsz); // But array has no .resize(), so this fails.

            for (size_t j = 0; j < vsz; ++j) {
                _centroid[j] = static_cast<element_type_t>(0);
            }

            size_t csz = coords.size();
            for (size_t i = 0; i < csz; ++i) {
                for (size_t j = 0; j < vsz; ++j) {
                    _centroid[j] += coords[i][j];
                }
            }
            for (size_t j = 0; j < vsz; ++j) {
                _centroid[j] /= static_cast<element_type_t>(csz);
            }

            return _centroid;
        }
    };

    /*!
     * A 'type 1' type is a fixed size array type, such as std::array or morph::Vector3
     *
     * This is the implementation for "number_type::value is 1" - fixed size.
     */
    template<>
    struct MathImpl<1>
    {
        //! Find the max and min length vector values and return in a pair.
        template<typename T>
        static pair<T, T> maxmin (const vector<T>& values) {
            cout << "T is fixed size 'vector'" << endl;

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

        template <typename T>
        static T centroid (const vector<T>& coords) {
            cout << "T is fixed size 'vector'" << endl;
            using element_type_t = std::remove_reference_t<decltype(*std::begin(std::declval<T&>()))>;

            T _centroid;

            // Return if vector<T> is empty
            if (coords.empty()) {
                return _centroid;
            }

            cout << "Zero _centroid" << endl;
            // zero _centroid
            size_t vsz = coords.begin()->size();

            // No resize here, as this is the version for non-resizable vectors
            // (array<float,3> Vector3, etc)

            for (size_t j = 0; j < vsz; ++j) {
                _centroid[j] = static_cast<element_type_t>(0);
            }

            cout << "Compute _centroid" << endl;
            size_t csz = coords.size();
            for (size_t i = 0; i < csz; ++i) {
                for (size_t j = 0; j < vsz; ++j) {
                    _centroid[j] += coords[i][j];
                }
            }
            for (size_t j = 0; j < vsz; ++j) {
                _centroid[j] /= static_cast<element_type_t>(csz);
            }

            cout << "Return _centroid" << endl;
            return _centroid;
        }
    };

    /*!
     * Implementations of algorithms taking T as a scalar-like object, such as float
     * or double.
     *
     * number_type::value is 2 - scalar
     */
    template<>
    struct MathImpl<2>
    {
        //! Find the max and min scalar values and return in a pair.
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

        //! Centroid of a set of 3D coordinates coords, T is scalar and cordinates
        //! are assumed 3D and to be in order x1,y1,z1, x2,y2,z2, etc
        template <typename T>
        static T centroid (const vector<T>& coords) {
            cout << "T is scalar" << endl;
            throw runtime_error ("Call this with vector<T>& coords where T is a vector/array type");
        }
    };

} // namespace morph
