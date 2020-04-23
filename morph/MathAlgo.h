/*!
 * Mathematical algorithms in the morph namespace.
 */

#pragma once

#include <vector>
#include <array>
#include <utility>
#include <iostream>
//#include <type_traits>
#include <memory>
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
         * Functions which can take objects where T is EITHER a scalar, such as float
         * or double (or int, etc, if it makes sense) OR a mathematical vector
         * encapsulated in a std::list, std::vector or std::array (or any other STL
         * container which needs just a type and an allocator to be initialised).
         */
        //@{
        /*******************************************************************/

        /*!
         * Functions whose implementations are in MathImpl, and which differ depending
         * on whether T is a scalar type or a vector-like object (such as std::vector
         * or std::list).
         *
         * Don't confuse this with C++11's std::minmax, which does something similar,
         * but won't do a max/min length of vector search like this does.
         */
        template < template <typename, typename> typename Container,
                   typename T,
                   typename Allocator=std::allocator<T> >
        static std::pair<T,T> maxmin (const Container<T, Allocator>& vec) {
            return MathImpl<number_type<T>::value>::maxmin (vec);
        }

        /*!
         * Find the centroid of a set of coordinates. If T is e.g. array<float, N> or
         * vector<double> or list<float> then the N-D centroid of the coordinates
         * defined in Container<T> is computed.
         */
        template < template <typename, typename> typename Container,
                   typename T,
                   typename Allocator=std::allocator<T> >
        static T centroid (const Container<T, Allocator>& coords) {
            return MathImpl<number_type<T>::value>::centroid (coords);
        }

        /*!
         * Autoscale a vector of numbers (or vectors) so that the range min to max is
         * scaled from 0.0 to 1.0.
         */
        template < template <typename, typename> typename Container,
                   typename T,
                   typename Allocator=std::allocator<T>,
                   typename S >
        static Container<T, Allocator> autoscale (const Container<T, Allocator>& values, S range_min, S range_max) {
            return MathImpl<number_type<T>::value>::autoscale (values, range_min, range_max);
        }

        /*******************************************************************/
        //@}

        /*!
         * Functions without specific scalar/vector implementations
         */
        //@{
        //! Compute distance from p1 to p2 (ND)
        template<typename T, std::size_t N>
        static T distance (const std::array<T, N> p1, const std::array<T, N> p2) {
            T sos = static_cast<T>(0);
            for (size_t i = 0; i < N; ++i) {
                T pdiff = p2[i]-p1[i];
                sos += pdiff * pdiff;
            }
            T dist = std::sqrt (sos);
            return dist;
        }

        //! Compute distance^2 from p1 to p2 (ND)
        template<typename T, std::size_t N>
        static T distance_sq (const std::array<T, N> p1, const std::array<T, N> p2) {
            T sos = static_cast<T>(0);
            for (size_t i = 0; i < N; ++i) {
                T pdiff = p2[i]-p1[i];
                sos += pdiff * pdiff;
            }
            return sos;
        }

        //! Compute distance from p1 to p2 (2D, see BezCurve.h for use)
        template<typename T>
        static T distance (const std::pair<T, T> p1, const std::pair<T, T> p2) {
            T xdiff = p2.first-p1.first;
            T ydiff = p2.second-p1.second;
            T dist = std::sqrt (xdiff*xdiff + ydiff*ydiff);
            return dist;
        }

        //! Compute squared distance from p1 to p2 (2D, see BezCurve.h for use)
        template<typename T>
        static T distance_sq (const std::pair<T, T> p1, const std::pair<T, T> p2) {
            T xdiff = p2.first-p1.first;
            T ydiff = p2.second-p1.second;
            T dist_sq = xdiff*xdiff + ydiff*ydiff;
            return dist_sq;
        }

        //! Centroid of a set of 2D coordinates @points.
        template<typename T>
        static std::pair<T,T> centroid2D (const std::vector<std::pair<T,T>> points) {
            std::pair<T,T> centroid;
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
        static std::pair<T,T> centroid2D (const std::vector<T> points) {
            std::pair<T,T> centroid;
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
        static std::array<T,3> centroid3D (const std::vector<T> points) {
            std::array<T,3> centroid;
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
        static std::array<T,3> centroid3D (const std::array<T, 12> points) {
            std::array<T,3> centroid;
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

        //! Compute standard deviation of the T values in @values. Return SD.
        template<typename T>
        static T compute_sd (const std::vector<T>& values) {
            T mean = 0.0;
            return MathAlgo::compute_mean_sd<T> (values, mean);
        }

        //! Compute standard deviation of the T values in @values. Return SD, write
        //! mean into arg.
        template<typename T>
        static T compute_mean_sd (const std::vector<T>& values, T& mean) {
            mean = 0.0;
            for (T val : values) {
                mean += val;
            }
            mean /= values.size();

            T sos_deviations = 0.0;
            for (T val : values) {
                sos_deviations += ((val-mean)*(val-mean));
            }
            T variance = sos_deviations / (values.size()-1);
            return std::sqrt(variance);
        }

        //! The bubble sort algorithm, high to low. T could be floating point or
        //! integer types.
        template<typename T>
        static void bubble_sort_hi_to_lo (std::vector<T>& values) {
            T value;
            unsigned int jplus;
            for (unsigned int i = 0; i < values.size(); ++i) {
                for (unsigned int j = 0; j < values.size()-1; ++j) {
                    jplus = j+1;
                    if (values[j] < values[jplus]) {
                        value = values[j];
                        values[j] = values[jplus];
                        values[jplus] = value;
                    }
                }
            }
        }

        //! The bubble sort algorithm, low to high. T could be floating point or
        //! integer types.
        template<typename T>
        static void bubble_sort_lo_to_hi (std::vector<T>& values) {
            T value;
            unsigned int jplus;
            for (unsigned int i = 0; i < values.size(); ++i) {
                for (unsigned int j = 0; j < values.size()-1; ++j) {
                    jplus = j+1;
                    if (values[j] > values[jplus]) {
                        value = values[j];
                        values[j] = values[jplus];
                        values[jplus] = value;
                    }
                }
            }
        }

        //! Bubble sort, high to low, order is returned in indices, values are left
        //! unchanged
        template<typename T>
        static void bubble_sort_hi_to_lo (const std::vector<T>& values, std::vector<unsigned int>& indices) {

            std::vector<T> vcopy = values;

            // Init indices to be a sequence
            for (unsigned int i = 0; i < indices.size(); ++i) {
                indices[i] = i;
            }

            T value;
            unsigned int index;
            unsigned int jplus;
            for (unsigned int i = 0; i < vcopy.size(); ++i) {
                for (unsigned int j = 0; j < vcopy.size()-1; ++j) {
                    jplus = j+1;
                    if (vcopy[j] < vcopy[jplus]) {
                        // Swap value in the copy
                        value = vcopy[j];
                        vcopy[j] = vcopy[jplus];
                        vcopy[jplus] = value;
                        // Swap index too
                        index = indices[j];
                        indices[j] = indices[jplus];
                        indices[jplus] = index;
                    }
                }
            }
        }

        //! Bubble sort, low to high, order is returned in indices, values are left
        //! unchanged
        template<typename T>
        static void bubble_sort_lo_to_hi (const std::vector<T>& values, std::vector<unsigned int>& indices) {

            std::vector<T> vcopy = values;

            // Init indices to be a sequence
            for (unsigned int i = 0; i < indices.size(); ++i) {
                indices[i] = i;
            }

            T value;
            unsigned int index;
            unsigned int jplus;
            for (unsigned int i = 0; i < vcopy.size(); ++i) {
                for (unsigned int j = 0; j < vcopy.size()-1; ++j) {
                    jplus = j+1;
                    if (vcopy[j] > vcopy[jplus]) {
                        // Swap value in the copy
                        value = vcopy[j];
                        vcopy[j] = vcopy[jplus];
                        vcopy[jplus] = value;
                        // Swap index too
                        index = indices[j];
                        indices[j] = indices[jplus];
                        indices[jplus] = index;
                    }
                }
            }
        }

        /*!
         * Functions which help you to arrange dots on circular rings
         */
        //@{

        //! How many items (dots, for example) could you arrange on a circle of
        //! radius=@radius with @d between each item's centre?
        template<typename T>
        static int numOnCircle (T radius, T d) {
            if (radius == static_cast<T>(0.0)) {
                return 1;
            }
            T circum = (T)morph::TWO_PI_D * radius;
            return static_cast<int>(floor (circum / d));
        }

        //! How many items on a circular arc of angle @a?
        template<typename T>
        static int numOnCircleArc (T radius, T d, T a) {
            //std::cout << "Called for radius == " << radius << ", d=" << d <<  std::endl;
            if (radius == static_cast<T>(0.0)) {
                return 1;
            }
            T circum = static_cast<T>(morph::TWO_PI_D) * radius;
            //std::cout << "circum = " << circum << std::endl;
            T rtn = 0;
#if 1
            // longhand, with a test for a circular arc
            if (a >= static_cast<T>(morph::TWO_PI_D)) {
                rtn = floor (circum / d);
            } else {
                T proportion = a / static_cast<T>(morph::TWO_PI_D);
                //std::cout << "prop = " << proportion << std::endl;
                T arclen = circum * proportion;
                //std::cout << "arclen = " << arclen << std::endl;
                rtn = floor (arclen / d);
            }
#else
            T proportion = a / static_cast<T>(morph::TWO_PI_D);
            rtn = floor (circum * proportion / d);
#endif
            //std::cout << "rtn " << rtn << std::endl;
            return rtn;
        }

        //! How many dots spaced by d can be placed on circular arc rings with d between them?
        template<typename T>
        static int numDotsOnRings (T minRadius, T maxRadius, T d,
                                   T a = static_cast<T>(morph::TWO_PI_D)) {

            // Computation of nrings differs depending on whether we have a dot and nrings, or nrings
            // from minRadius to maxRadius. Herein lies the problem!
            int n_dots = 0;
            if (minRadius == static_cast<T>(0.0)) {
                int nrings = (int) floor ((maxRadius-minRadius)/d);
                if (minRadius == 0.0) {
                    nrings++; // cos of centre dot.
                }

                for (int r=0; r<nrings; ++r) {
                    n_dots += MathAlgo::numOnCircleArc<T> (minRadius+r*d, d, a);
                }
            } else {
                // Annulus
                int nrings = 1 + (int) floor ((maxRadius-minRadius)/d);
                //std::cout << nrings << " rings" << std::endl;
                for (int r=0; r<nrings; ++r) {
                    n_dots += MathAlgo::numOnCircleArc<T> (minRadius+r*d, d, a);
                }
            }
            //std::cout << "n_dots for d=" << d << " is " << n_dots << std::endl;
            return n_dots;
        }

        //@}
        //@}

    }; // struct MathAlgo

} // namespace morph
