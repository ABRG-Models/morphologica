/*!
 * Mathematical algorithms in the morph namespace.
 */

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
#include "number_type.h"
#include "MathImpl.h"

namespace morph {

    /*!
     * The new MathAlgo class with its methods.
     */
    struct MathAlgo
    {
        /*!
         * Functions whose implementations are in MathImpl, and which differ depending
         * on whether T is a scalar type or a vector.
         */
        template <typename T>
        static pair<T,T> maxmin (vector<T>& vec) {
            //return MathImpl<is_scalar<T>::value>::maxmin (vec);
            return MathImpl<number_type<T>::value>::maxmin (vec);
        }

        //! Centroid coordinates. If T is array<Flt, N> then the N-D centroid of the
        //! points vector<T> is computed.  Try and make it possible to use my own
        //! Vector3 class for the coords. Find a better way of identifying
        //! array/vector types?
        template <typename T>
        static T centroid (const vector<T>& coords) {
            return MathImpl<number_type<T>::value>::centroid (coords);
        }

#if 1
        /*!
         * Functions without specific scalar/vector implementations
         */
        //@{

        //! Centroid of a set of 2D coordinates @points.
        template<typename T>
        static pair<T,T> centroid2D (const vector<pair<T,T>> points) {
            pair<T,T> centroid;
            centroid.first = static_cast<T>(0);
            centroid.second = static_cast<T>(0);
            for (auto p : points) {
                centroid.first += p.first;
                centroid.second += p.second;
            }
            centroid.first /= points.size();
            centroid.second /= points.size();
            return centroid;
        }

        //! Centroid of a set of 2D coordinates @points, assumed to be in order
        //! x1,y1,x2,y2,etc
        template<typename T>
        static pair<T,T> centroid2D (const vector<T> points) {
            pair<T,T> centroid;
            centroid.first = static_cast<T>(0);
            centroid.second = static_cast<T>(0);
            size_t psz = points.size();
            for (size_t i = 0; i < psz-1; i+=2) {
                centroid.first += points[i];
                centroid.second += points[i+1];
            }
            centroid.first /= (psz/2);
            centroid.second /= (psz/2);
            return centroid;
        }

        //! Centroid of a set of 3D coordinates @points, assumed to be in order
        //! x1,y1,z1, x2,y2,z2, etc
        // *Used in Stalefish only (I think)
        template<typename T>
        static array<T,3> centroid3D (const vector<T> points) {
            array<T,3> centroid;
            centroid[0] = static_cast<T>(0);
            centroid[1] = static_cast<T>(0);
            centroid[2] = static_cast<T>(0);
            size_t psz = points.size();
            for (size_t i = 0; i < psz-2; i+=3) {
                centroid[0] += points[i];
                centroid[1] += points[i+1];
                centroid[2] += points[i+2];
            }
            centroid[0] /= (psz/3);
            centroid[1] /= (psz/3);
            centroid[2] /= (psz/3);
            return centroid;
        }

        //! Centroid 4 3D coordinates
        template<typename T>
        static array<T,3> centroid3D (const array<T, 12> points) {
            array<T,3> centroid;
            centroid[0] = static_cast<T>(0);
            centroid[1] = static_cast<T>(0);
            centroid[2] = static_cast<T>(0);
            size_t psz = 12;
            for (size_t i = 0; i < psz-2; i+=3) {
                centroid[0] += points[i];
                centroid[1] += points[i+1];
                centroid[2] += points[i+2];
            }
            centroid[0] /= 4;
            centroid[1] /= 4;
            centroid[2] /= 4;
            return centroid;
        }
#endif
    }; // struct MathAlgo

} // namespace morph
