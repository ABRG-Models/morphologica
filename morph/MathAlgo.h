/*!
 * Mathematical algorithms in the morph namespace.
 */

#pragma once

#include <vector>
#include <array>
#include <iostream>
#include <algorithm>
#include <bitset>
#include <memory>
#include <cuchar>
#include <morph/vec.h>
#include <morph/vvec.h>
#include <morph/range.h>
#include <morph/mathconst.h>
#include <morph/number_type.h>
#include <morph/MathImpl.h>

namespace morph {

    enum class rotation_sense
    {
        colinear,
        clockwise,
        anticlockwise
    };

    /*!
     * The new MathAlgo class with its methods.
     */
    struct MathAlgo
    {
        /*
         * Functions which can take objects where T is EITHER a scalar, such as float
         * or double (or int, etc, if it makes sense) OR a mathematical vector
         * encapsulated in a std::list, std::vector or std::array (or any other STL
         * container which needs just a type and an allocator to be initialised).
         */
        /*******************************************************************/

        /*!
         * Functions whose implementations are in MathImpl, and which differ depending on whether
         * the container's value_type is a scalar type or a vector-like object (such as std::vector
         * or std::list).
         *
         * Don't confuse this with C++11's std::minmax, which does something similar,
         * but won't do a max/min length of vector search like this does.
         */
        template <typename Container, std::enable_if_t<morph::is_copyable_container<Container>::value, int> = 0>
        static morph::range<typename Container::value_type> maxmin (const Container& vec) {
            return MathImpl<number_type<typename Container::value_type>::value>::maxmin (vec);
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

        /*
         * Functions without specific scalar/vector implementations
         */

        //! Return mean and sum of squared deviations from the mean
        template < template <typename, typename> typename Container,
                   typename T,
                   typename Allocator=std::allocator<T> >
        static morph::vec<T, 2> meansos (const Container<T, Allocator>& values)
        {
            morph::vec<T, 2> meansos = {T{0},T{0}};
            if (values.empty()) { return meansos; }
            for (T val : values) { meansos[0] += val; }
            meansos[0] /= values.size();

            for (T val : values) {
                // Add up sum of squared deviations
                meansos[1] += ((val-meansos[0])*(val-meansos[0]));
            }

            return meansos;
        }

        //! Covariance of two sets of numbers
        template < template <typename, typename> typename Container,
                   typename T,
                   typename Allocator=std::allocator<T> >
        static T covariance (const Container<T, Allocator>& x,
                             const Container<T, Allocator>& y)
        {
            if (x.empty() || y.empty()) { throw std::runtime_error ("x or y is empty."); }
            if (x.size() != y.size()) {
                throw std::runtime_error ("covariance: both number arrays to be same size.");
            }
            morph::vec<T, 2> ms_x = MathAlgo::meansos<Container, T, Allocator> (x);
            morph::vec<T, 2> ms_y = MathAlgo::meansos<Container, T, Allocator> (y);
            T cov = T{0};
            for (typename Container<T, Allocator>::size_type i = 0; i < x.size(); ++i) {
                cov += ((x[i] - ms_x[0]) * (y[i] - ms_y[0]));
            }
            return cov;
        }

        //! Covariance of two sets of numbers, where means of x and y have already been computed
        template < template <typename, typename> typename Container,
                   typename T,
                   typename Allocator=std::allocator<T> >
        static T covariance (const Container<T, Allocator>& x, const T mean_x,
                             const Container<T, Allocator>& y, const T mean_y)
        {
            if (x.empty() || y.empty()) { throw std::runtime_error ("x or y is empty."); }
            if (x.size() != y.size()) {
                throw std::runtime_error ("covariance: both number arrays to be same size.");
            }
            T cov = T{0};
            for (typename Container<T, Allocator>::size_type i = 0; i < x.size(); ++i) {
                cov += ((x[i] - mean_x) * (y[i] - mean_y));
            }
            return cov;
        }

        //! Linear regression. Return slope (first) and offset (second) (m and c from 'y
        //! = mx + c') in an vec<T, 2>
        template < template <typename, typename> typename Container,
                   typename T,
                   typename Allocator=std::allocator<T> >
        static morph::vec<T, 2> linregr (const Container<T, Allocator>& x,
                                       const Container<T, Allocator>& y)
        {
            morph::vec<T, 2> ms_x = MathAlgo::meansos<Container, T, Allocator> (x);
            morph::vec<T, 2> ms_y = MathAlgo::meansos<Container, T, Allocator> (y);
            T cov_xy = MathAlgo::covariance<Container, T, Allocator> (x, ms_x[0], y, ms_y[0]);
            T m = cov_xy / ms_x[1];
            T c = ms_y[0] - (m * ms_x[0]);
            return morph::vec<T, 2> ({m, c});
        }

        //! Compute distance from p1 to p2 (ND)
        template<typename T, std::size_t N>
        static T distance (const std::array<T, N> p1, const std::array<T, N> p2) {
            T sos = T{0};
            for (std::size_t i = 0; i < N; ++i) {
                T pdiff = p2[i]-p1[i];
                sos += pdiff * pdiff;
            }
            T dist = std::sqrt (sos);
            return dist;
        }

        //! Compute distance^2 from p1 to p2 (ND)
        template<typename T, std::size_t N>
        static T distance_sq (const std::array<T, N> p1, const std::array<T, N> p2) {
            T sos = T{0};
            for (std::size_t i = 0; i < N; ++i) {
                T pdiff = p2[i]-p1[i];
                sos += pdiff * pdiff;
            }
            return sos;
        }

        //! Compute distance from p1 to p2 (2D, see BezCurve.h for use)
        template<typename T>
        static T distance (const morph::vec<T, 2> p1, const morph::vec<T, 2> p2) {
            T xdiff = p2[0]-p1[0];
            T ydiff = p2[1]-p1[1];
            T dist = std::sqrt (xdiff*xdiff + ydiff*ydiff);
            return dist;
        }

        //! Compute squared distance from p1 to p2 (2D, see BezCurve.h for use)
        template<typename T>
        static T distance_sq (const morph::vec<T, 2> p1, const morph::vec<T, 2> p2) {
            T xdiff = p2[0]-p1[0];
            T ydiff = p2[1]-p1[1];
            T dist_sq = xdiff*xdiff + ydiff*ydiff;
            return dist_sq;
        }

        //! Compute orientation of three points which form a triangle pqr.
        //! \return 0 if co-linear, 1 if clockwise; 2 if anticlockwise
        //! Algorithm, which uses slopes, taken from
        //! https://www.geeksforgeeks.org/orientation-3-ordered-points/
        template<typename T>
        static rotation_sense orientation (const morph::vec<T, 2>& p,
                                           const morph::vec<T, 2>& q,
                                           const morph::vec<T, 2>& r)
        {
            constexpr T thresh = T{10} * std::numeric_limits<T>::epsilon();
            T val = (q[1] - p[1]) * (r[0] - q[0])  -  (q[0] - p[0]) * (r[1] - q[1]);

            // Mathematically, this would be if (val == T{0}) {...} but we have to consider
            // numerical precision, hence the comparison with thresh (3 epsilons)
            if (std::abs(val) < thresh) { return rotation_sense::colinear; }
            return (val > T{0}) ? rotation_sense::clockwise : rotation_sense::anticlockwise;
        }

        // Given three colinear points p, q, r, the function checks if
        // point q lies on line segment 'pr'. Copied from:
        // https://www.geeksforgeeks.org/check-if-two-given-line-segments-intersect/
        template<typename T>
        static bool onsegment (const morph::vec<T, 2>& p,
                               const morph::vec<T, 2>& q,
                               const morph::vec<T, 2>& r)
        {
            if (q[0] <= std::max(p[0], r[0]) && q[0] >= std::min(p[0], r[0]) &&
                q[1] <= std::max(p[1], r[1]) && q[1] >= std::min(p[1], r[1])) {
                return true;
            }
            return false;
        }

        /*!
         * Do the line segments p1-q1 and p2-q2 intersect? Are they instead colinear? Return these
         * booleans in a bitset (bit0: intersection, bit1: colinear)
         *
         * \param p1 Start of line segment 1
         * \param q1 End of line segment 1
         * \param p2 Start of line segment 2
         * \param q2 End of line segment 2
         *
         * \return A bitset whose bit 0 indicates if the lines intersect and whose bit 1 indicates
         * if the lines are colinear
         */
        template<typename T>
        static std::bitset<2> segments_intersect (const morph::vec<T, 2>& p1, const morph::vec<T, 2> q1,
                                                  const morph::vec<T, 2>& p2, const morph::vec<T, 2> q2)
        {
            constexpr bool debug_this = false;
            if constexpr (debug_this) {
                std::cout << "Testing intersection for " << p1 << "->" << q1
                          << " and " << p2 << "->" << q2 << std::endl;
            }
            std::bitset<2> rtn;
            morph::rotation_sense p1q1p2 = morph::MathAlgo::orientation (p1, q1, p2);
            morph::rotation_sense p1q1q2 = morph::MathAlgo::orientation (p1, q1, q2);
            morph::rotation_sense p2q2p1 = morph::MathAlgo::orientation (p2, q2, p1);
            morph::rotation_sense p2q2q1 = morph::MathAlgo::orientation (p2, q2, q1);
            if (p1q1p2 != p1q1q2 && p2q2p1 != p2q2q1) { // They intersect
                rtn.set(0, true);
            } else { // Are they colinear?
                if constexpr (debug_this) {
                    std::cout << "Test colinearity... epsilon is " << std::numeric_limits<T>::epsilon() << "\n";
                    if (p1q1p2 == morph::rotation_sense::colinear) {
                        std::cout << "p1q1p2 rotn sense is colinear. On segment? "
                                  << (morph::MathAlgo::onsegment (p1, p2, q1) ? "T" : "F") << std::endl;
                    } else if (p1q1q2 == morph::rotation_sense::colinear) {
                        std::cout << "p1q1q2 rotn sense is colinear On segment? "
                                  << (morph::MathAlgo::onsegment (p1, q2, q1) ? "T" : "F") << std::endl;
                    } else if (p2q2p1 == morph::rotation_sense::colinear) {
                        std::cout << "p2q2p1 rotn sense is colinear On segment? "
                                  << (morph::MathAlgo::onsegment (p2, p1, q2) ? "T" : "F") << std::endl;
                    } else if (p2q2q1 == morph::rotation_sense::colinear) {
                        std::cout << "p2q2q1 rotn sense is colinear On segment? "
                                  << (morph::MathAlgo::onsegment (p2, q1, q2) ? "T" : "F") << std::endl;
                    } else {
                        std::cout << "NO rotn sense is colinear\n";
                    }
                }
                if (p1q1p2 == morph::rotation_sense::colinear && morph::MathAlgo::onsegment (p1, p2, q1)) { rtn.set(1, true); }
                else if (p1q1q2 == morph::rotation_sense::colinear && morph::MathAlgo::onsegment (p1, q2, q1)) { rtn.set(1, true); }
                else if (p2q2p1 == morph::rotation_sense::colinear && morph::MathAlgo::onsegment (p2, p1, q2)) { rtn.set(1, true); }
                else if (p2q2q1 == morph::rotation_sense::colinear && morph::MathAlgo::onsegment (p2, q1, q2)) { rtn.set(1, true); }
            }
            if constexpr (debug_this) { std::cout << "return " << rtn << std::endl; }
            return rtn;
        }

        /*!
         * Find the coordinate of the crossing point of the two line segments p1-q1 and p2-q2,
         * *assuming* the segments intersect. Call this *after* you have used
         * MathAlgo::segments_intesect!
         *
         * \param p1 Start of line segment 1
         * \param q1 End of line segment 1
         * \param p2 Start of line segment 2
         * \param q2 End of line segment 2
         *
         * \return Coordinate of the crossing point
         */
        template<typename T>
        static morph::vec<T, 2> crossing_point (const morph::vec<T, 2>& p1, const morph::vec<T, 2>& q1,
                                                const morph::vec<T, 2>& p2, const morph::vec<T, 2>& q2)
        {
            morph::vec<T, 2> p = p1;
            morph::vec<T, 2> r = p1 - q1;
            morph::vec<T, 2> q = p2;
            morph::vec<T, 2> s = p2 - q2;
            auto t = (q - p).cross(s / r.cross(s));
            morph::vec<T, 2> cross_point = p + t * r;
            return cross_point;
        }

#ifdef USE_Q_INVSQRT
        //! Quake fast 1/sqrt(x) approximation. Error ~1%
        //! See https://www.youtube.com/watch?v=p8u_k2LIZyo for explanation
        static float Q_invsqrt (float number)
        {
            long i;
            float x2, y;
            const float threehalfs = 1.5f;

            x2 = number * 0.5f;
            y = number;
            i = *(long*)&y;          // evil floating point bit hack. i is like log(y)
                                     // (the bits of y are the log of y)
            i = 0x5f3759df - (i>>1); // what the fuck? -(i>>1) is -log(y)/2
            y = y * (threehalfs - (x2 * y * y));  // 1st iteration
            //y = y * (threehalfs - (x2 * y * y));  // 2nd iteration can be removed
            return y;
        }
#endif

        //! Centroid of a set of 2D coordinates @points.
        template<typename T>
        static morph::vec<T, 2> centroid2D (const std::vector<morph::vec<T, 2>> points) {
            morph::vec<T, 2> centroid;
            centroid[0] = T{0};
            centroid[1] = T{0};
            if (points.size() == 0) { return centroid; }
            for (auto p : points) {
                centroid[0] += p[0];
                centroid[1] += p[1];
            }
            centroid[0] /= points.size();
            centroid[1] /= points.size();
            return centroid;
        }

        //! Centroid of a set of 2D coordinates @points, assumed to be in order
        //! x1,y1,x2,y2,etc
        template<typename T>
        static morph::vec<T, 2> centroid2D (const std::vector<T> points) {
            morph::vec<T, 2> centroid;
            centroid[0] = T{0};
            centroid[1] = T{0};
            typename std::vector<T>::size_type psz = points.size();
            if (psz == 0U) { return centroid; }
            for (typename std::vector<T>::size_type i = 0; i < psz-1; i+=2) {
                centroid[0] += points[i];
                centroid[1] += points[i+1];
            }
            centroid[0] /= (psz/2);
            centroid[1] /= (psz/2);
            return centroid;
        }

        //! Centroid of a set of 3D coordinates @points, assumed to be in order
        //! x1,y1,z1, x2,y2,z2, etc
        // *Used in Stalefish only (I think)
        template<typename T>
        static std::array<T,3> centroid3D (const std::vector<T> points) {
            std::array<T,3> centroid;
            centroid[0] = T{0};
            centroid[1] = T{0};
            centroid[2] = T{0};
            typename std::vector<T>::size_type psz = points.size();
            if (psz == 0U) { return centroid; }
            for (typename std::vector<T>::size_type i = 0; i < psz-2; i+=3) {
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
            centroid[0] = T{0};
            centroid[1] = T{0};
            centroid[2] = T{0};
            typename std::array<T, 12>::size_type psz = 12;
            for (typename std::array<T, 12>::size_type i = 0; i < psz-2; i+=3) {
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
            T mean = T{0};
            return MathAlgo::compute_mean_sd<T> (values, mean);
        }

        //! Compute standard deviation of the T values in @values. Return SD, write
        //! mean into arg.
        template<typename T>
        static T compute_mean_sd (const std::vector<T>& values, T& mean) {
            mean = T{0};
            if (values.empty()) {
                return T{0};
            }
            for (T val : values) {
                mean += val;
            }
            mean /= values.size();

            T sos_deviations = T{0};
            for (T val : values) {
                sos_deviations += ((val-mean)*(val-mean));
            }
            if (values.size() > 1) {
                T variance = sos_deviations / (values.size()-1);
                return std::sqrt(variance);
            } else {
                return T{0};
            }
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
         * Boxfilter implementation 1
         *
         * Apply a 2d, horizontally wrapping box filter. Test to see if boxside is odd and disallow
         * even (which was not not tested). Assume the data in the vvec relates to a rectangle of width w.
         *
         * \param data The input data. Should be a multiple of \tparam w in size. Must be
         * pre-allocated and of the same size as result.
         * \param result The output container. Must not be the same memory as input data.
         *
         * \tparam T The type of the input data
         * \tparam boxside The length of the boxfilter square
         * \tparam w The width of the input data (and of the result)
         * \tparam onlysum If true, only sum up the contributions from the box. If false (the
         * default), sum contributions and divide by box area.
         * \tparam T_o The type of the output data
         */
        template<typename T, int boxside, int w, bool onlysum = false, typename T_o = T>
        static void boxfilter_2d (const morph::vvec<T>& data, morph::vvec<T_o>& result)
        {
            if constexpr (boxside % 2 == 0) {
                throw std::runtime_error ("boxfilter_2d was not designed for even box filter squares (set boxside template param. to an odd value)");
            }
            if (result.size() != data.size()) {
                throw std::runtime_error ("The input data vector is not the same size as the result vector.");
            }

            // Divide by boxarea without accounting for edges (wrapping will sort horz edges)
            static constexpr T_o oneover_boxa = T_o{1} / (static_cast<T_o>(boxside) * static_cast<T_o>(boxside));
            static constexpr int halfbox = boxside / 2;
            static constexpr int halfbox_p1 = halfbox + 1;

            morph::vec<T_o, w> colsum;
            colsum.zero();
            T_o rowsum = T{0};

            int h = data.size() / w;

            // process data row by row
            for (int y = -halfbox; y < h; ++y) {

                // 1. Accumulate column sums; pull out last row.
                if (y + halfbox < h) {
                    if (y >= halfbox_p1) {
                        for (int x = 0; x < w; ++x) {
                            // Add to the next row from the data array and subtract the last (bottom) row of the colsum
                            colsum[x] += data[(y + halfbox) * w + x]  -  data[(y - halfbox_p1) * w + x];
                            // T_o       T                         T
                        }
                    } else {
                        for (int x = 0; x < w; ++x) {
                            // Just add to the next row from the data array
                            colsum[x] += data[(y + halfbox) * w + x];
                        }
                    }
                } else {
                    if (y >= halfbox_p1) {
                        // Just subtract
                        for (int x = 0; x < w; ++x) {
                            colsum[x] -= data[(y - halfbox_p1) * w + x];
                        }
                    } // else no op on colsum[x]
                }

                rowsum = T{0};
                if (y >= 0) {
                    // 2. Initialise rowsum. This happens after we have accumulated colsums. Init rowsum as the sum of the end col
                    for (int i = -halfbox_p1; i < 0; ++i) { rowsum += colsum[i + w]; }
                    for (int i = 0; i < halfbox; ++i) { rowsum += colsum[i]; }

                    // 3. Compute the sum along the row, and write this into result
                    int l = -halfbox_p1;
                    int r = halfbox;
                    for (int x = 0; x < w; ++x) {
                        // A modulus where -x modulus w gives always a positive index: (w + (a % w)) % w
                        rowsum += colsum[(w + (r++ % w)) % w] - colsum[(w + (l++ % w)) % w];

                        if constexpr (onlysum == true) {
                            result[y * w + x] = rowsum;
                        } else {
                            result[y * w + x] = rowsum * oneover_boxa;
                        }
                    }
                }
            }
        }

        /*!
         * Boxfilter implementation 2
         *
         * A 'fixed-size containers boxfilter'. Implemented to see if it is any faster than one in
         * which input/output data are morph::vvec. Turns out that it runs at the same speed.
         *
         * Apply a 2d, horizontally wrapping box filter. Test to see if boxside is odd and disallow
         * even (which was not not tested). Assume the data in the vvec relates to a rectangle of width w.
         *
         * \param data The input data. Should be a multiple of \tparam w in size.
         * \param result The output container. Must not be the same memory as input data.
         *
         * \tparam T The type of the input data
         * \tparam T_o The type of the output data
         * \tparam boxside The length of the boxfilter square
         * \tparam w The width of the input data rectangle (and of the result)
         * \tparam h The height of the input data (and of the result)
         * \tparam onlysum If true, only sum up the contributions from the box. If false, sum
         * contributions and divide by box area.
         */
        template<typename T, int boxside, int w, int h, bool onlysum = false, typename T_o = T>
        static void boxfilter_2d (const std::array<T, w * h>& data, std::array<T_o, w * h>& result)
        {
            static_assert ((boxside > 0 && (boxside % 2) > 0),
                           "boxfilter_2d was not designed for even box filter squares (set boxside template param. to an odd value)");

            // Divide by boxarea without accounting for edges (wrapping will sort horz edges)
            static constexpr T_o oneover_boxa = T_o{1} / (static_cast<T_o>(boxside) * static_cast<T_o>(boxside));
            static constexpr int halfbox = boxside / 2;
            static constexpr int halfbox_p1 = halfbox + 1;

            std::array<T_o, w> colsum;
            for (auto& el : colsum) { el = T_o{0}; }
            T_o rowsum = T_o{0};

            // process data row by row
            for (int y = -halfbox; y < h; ++y) {

                // 1. Accumulate column sums; pull out last row.
                if (y+halfbox < h) {
                    if (y >= halfbox_p1) {
                        for (int x = 0; x < w; ++x) {
                            // Add to the next row from the data array and subtract the last (bottom) row of the colsum
                            colsum[x] += data[(y + halfbox) * w + x]  -  data[(y - halfbox_p1) * w + x];
                            // T_o       T                         T
                        }
                    } else {
                        for (int x = 0; x < w; ++x) {
                            // Just add to the next row from the data array
                            colsum[x] += data[(y + halfbox) * w + x];
                        }
                    }
                } else {
                    if (y >= halfbox_p1) {
                        // Just subtract
                        for (int x = 0; x < w; ++x) {
                            colsum[x] -= data[(y - halfbox_p1) * w + x];
                        }
                    } // else no op on colsum[x]
                }

                rowsum = T{0};
                if (y >= 0) {
                    // 2. Initialise rowsum. This happens after we have accumulated colsums. Init rowsum as the sum of the end col
                    for (int i = -halfbox_p1; i < 0; ++i) { rowsum += colsum[i + w]; }
                    for (int i = 0; i < halfbox; ++i) { rowsum += colsum[i]; }

                    // 3. Compute the sum along the row, and write this into result
                    int l = -halfbox_p1;
                    int r = halfbox;
                    for (int x = 0; x < w; ++x) {
                        // A modulus where -x modulus w gives always a positive index: (w + (a % w)) % w
                        rowsum += colsum[(w + (r++ % w)) % w] - colsum[(w + (l++ % w)) % w];

                        if constexpr (onlysum == true) {
                            result[y * w + x] = rowsum;
                        } else {
                            result[y * w + x] = rowsum * oneover_boxa;
                        }
                    }
                }
            }
        }

        /*!
         * Boxfilter implementation 3
         *
         * This is a Boxfilter that works with a runtime-configured width, w, and not a
         * template-specified w, as is the case in implementations 1 and 2.
         *
         * \param data The input data. Should be a multiple of \tparam w in size. Must be
         * pre-allocated and of the same size as result.
         * \param result The output container. Must not be the same memory as input data.
         * \param w The width of rectangular data presented in the input.
         *
         * \tparam T The type of the input and output data
         * \tparam boxside The length of the boxfilter square
         * \tparam onlysum If true, only sum up the contributions from the box. If false, sum
         * contributions and divide by box area.
         */
        template<typename T, int boxside, bool onlysum = false>
        static void boxfilter_2d (const morph::vvec<T>& data, morph::vvec<T>& result, const int w)
        {
            if constexpr (boxside % 2 == 0) {
                throw std::runtime_error ("boxfilter_2d was not designed for even box filter squares (set boxside template param. to an odd value)");
            }
            if (result.size() != data.size()) {
                throw std::runtime_error ("The input data vector is not the same size as the result vector.");
            }
            if (&data == &result) {
                throw std::runtime_error ("Pass in separate memory for the result.");
            }

            // Divide by boxarea without accounting for edges (wrapping will sort horz edges)
            static constexpr T oneover_boxa = T{1} / (static_cast<T>(boxside) * static_cast<T>(boxside));
            static constexpr int halfbox = boxside / 2;
            static constexpr int halfbox_p1 = halfbox + 1;

            morph::vvec<T> colsum (w, T{0});
            T rowsum = T{0};

            int h = data.size() / w;

            // process data row by row
            for (int y = -halfbox; y < h; ++y) {

                // 1. Accumulate column sums; pull out last row.
                if (y + halfbox < h) {
                    if (y >= halfbox_p1) {
                        for (int x = 0; x < w; ++x) {
                            // Add to the next row from the data array and subtract the last (bottom) row of the colsum
                            colsum[x] += data[(y + halfbox) * w + x]  -  data[(y - halfbox_p1) * w + x];
                        }
                    } else {
                        for (int x = 0; x < w; ++x) {
                            // Just add to the next row from the data array
                            colsum[x] += data[(y + halfbox) * w + x];
                        }
                    }
                } else {
                    if (y >= halfbox_p1) {
                        // Just subtract
                        for (int x = 0; x < w; ++x) {
                            colsum[x] -= data[(y - halfbox_p1) * w + x];
                        }
                    } // else no op on colsum[x]
                }

                rowsum = T{0};
                if (y >= 0) {
                    // 2. Initialise rowsum. This happens after we have accumulated colsums. Init rowsum as the sum of the end col
                    for (int i = -halfbox_p1; i < 0; ++i) { rowsum += colsum[i + w]; }
                    for (int i = 0; i < halfbox; ++i) { rowsum += colsum[i]; }

                    // 3. Compute the sum along the row, and write this into result
                    int l = -halfbox_p1;
                    int r = halfbox;
                    for (int x = 0; x < w; ++x) {
                        // A modulus where -x modulus w gives always a positive index: (w + (a % w)) % w
                        rowsum += colsum[(w + (r++ % w)) % w] - colsum[(w + (l++ % w)) % w];

                        if constexpr (onlysum == true) {
                            result[y * w + x] = rowsum;
                        } else {
                            result[y * w + x] = rowsum * oneover_boxa;
                        }
                    }
                }
            }
        }

        // Carry out a simple, 2 pixel kernel edge convolution for both vertical and horizontal
        // edges. The one-d array data is assumed to be rectangular with width w. I have chosen to
        // place the edge between element i and element i+1 (or i+w) in edges[i] (it would be
        // equally sensible to place it in i+1/i+w). I've assumed that the 1D array fills a
        // rectangle from the bottom left (so it's a right-handed coordinate system).  I have
        // provided an option to invert edges for either axis, which allows for other assumptions
        // about the way the 1D array fills a rectangle.
        template<typename T, int w, bool invert_vert_edges=false, bool invert_horz_edges=false>
        static void edgeconv_2d (const morph::vvec<T>& data, morph::vvec<T>& v_edges, morph::vvec<T>& h_edges)
        {
            if (v_edges.size() != data.size() || h_edges.size() != data.size()) {
                throw std::runtime_error ("The input data vector is not the same size as the result vectors.");
            }
            if (&data == &v_edges || &data == &h_edges) {
                throw std::runtime_error ("Pass in separate memory for the results.");
            }

            int lastrow_index = data.size() - w;

            for (int i = 0; i < static_cast<int>(data.size()); ++i) {
                if ((i+1)%w == 0) { // on last column; do horizontal wrapping
                    if constexpr (invert_vert_edges == true) {
                        v_edges[i] =  data[i] - data[i-w+1];
                    } else {
                        v_edges[i] = -data[i] + data[i-w+1];
                    }
                } else {
                    if constexpr (invert_vert_edges == true) {
                        v_edges[i] =  data[i] - data[i+1];
                    } else {
                        v_edges[i] = -data[i] + data[i+1];
                    }
                }
                if (i >= lastrow_index) { // Then we're on the last row
                    h_edges[i] = T{0};
                } else {
                    if constexpr (invert_horz_edges == true) {
                        h_edges[i] =  data[i] - data[i+w];
                    } else {
                        h_edges[i] = -data[i] + data[i+w];
                    }
                }
            }
        }

        // fixed sized arrays version.
        template<typename T, int w, int h, bool invert_vert_edges=false, bool invert_horz_edges=false>
        static void edgeconv_2d (const morph::vec<T, w*h>& data, morph::vec<T, w*h>& v_edges, morph::vec<T, w*h>& h_edges)
        {
            int lastrow_index = (w*h) - w;

            for (int i = 0; i < w*h; ++i) {
                if ((i+1)%w == 0) { // on last column; do horizontal wrapping
                    if constexpr (invert_vert_edges == true) {
                        v_edges[i] =  data[i] - data[i-w+1];
                    } else {
                        v_edges[i] = -data[i] + data[i-w+1];
                    }
                } else {
                    if constexpr (invert_vert_edges == true) {
                        v_edges[i] =  data[i] - data[i+1];
                    } else {
                        v_edges[i] = -data[i] + data[i+1];
                    }
                }
                if (i >= lastrow_index) {
                    h_edges[i] = T{0};
                } else {
                    if constexpr (invert_horz_edges == true) {
                        h_edges[i] =  data[i] - data[i+w];
                    } else {
                        h_edges[i] = -data[i] + data[i+w];
                    }
                }
            }
        }

        // Do an on-centre, off-surround filtering for a pixel in data and its 8 neighbours
        template<typename T, int w, bool horz_wrap=true>
        static void oncentre_offsurround (const morph::vvec<T>& data, morph::vvec<T>& result)
        {
            if (result.size() != data.size()) {
                throw std::runtime_error ("The data vector is not the same size as the result vector.");
            }
            if (&data == &result) {
                throw std::runtime_error ("Pass in separate memory for the result.");
            }

            // First, copy data into result - this is the 'on-centre'
            std::copy (data.begin(), data.end(), result.begin());

            int lastrow_index = static_cast<int>(data.size()) - w;

            // Now go through and subtract the neighbouring values - this is 'off-surround'.
            int i = 0;
             // BL pixel
            if constexpr (horz_wrap) {
                result[i] -= (data[i+1] + data[i+w-1] + data[w+i+w-1] + data[w+i] + data[w+i+1]) / T{5};
            } else {
                result[i] -= (data[i+1] + data[i+w] + data[i+w+1]) / T{3};
            }
            ++i;
            // First row
            for (; i < w-1; ++i) {
                result[i] -= (data[i-1]  + data[i+1] + data[i+w-1] + data[i+w] + data[i+w+1]) / T{5};
            }
            // BR pixel
            if constexpr (horz_wrap) {
                result[i] -= (data[0] + data[i-1] + data[i+w-1] + data[i+w] + data[w]) / T{5};
            } else {
                result[i] -= (data[i-1] + data[i+w-1] + data[i+w]) / T{3};
            }
            ++i;

            // Intermediate rows
            for (; i < lastrow_index; ++i) {
                if (i%w == 0) {             // first column
                    if constexpr (horz_wrap) {
                        //                R           'L'            'UL'           U             UR          D             DR        'DL'
                        result[i] -= (data[i+1] + data[i+w-1] + data[w+i+w-1] + data[w+i] + data[w+i+1] + data[i-w] + data[i-w+1] + data[i-1]) / T{8};
                    } else {
                        result[i] -= (data[i+1] + data[i+w] + data[i+w+1] + data[i-w] + data[i-w+1]) / T{5};
                    }
                } else if ((i+1)%w == 0)  { // on last column
                    if constexpr (horz_wrap) {
                        //                L       'R'            U             'UR'           UL           D            DL         'DR'
                        result[i] -= (data[i-1] + data[i-w+1] + data[i+w] + data[i+1] + data[i+w-1] + data[i-w] + data[i-w-1] + data[i-w-w+1]) / T{5};
                    } else {
                        result[i] -= (data[i-1] + data[i+w] + data[i+w-1] + data[i-w] + data[i-w-1]) / T{5};
                    }
                } else {                    // All the rest have 8 neighbours
                    result[i] -= (data[i-1] + data[i+1] + data[i+w-1] + data[i+w] + data[i+w+1] + data[i-w-1] + data[i-w] + data[i-w+1]) / T{8};
                }
            }
            // TL pixel
            if constexpr (horz_wrap) {
                result[i] -= (data[i+1] + data[i-w+1] + data[i-w] + data[i+w-1] + data[i-1]) / T{5};
            } else {
                result[i] -= (data[i+1] + data[i-w+1] + data[i-w]) / T{3};
            }
            ++i;

            // Top row
            for (; i < static_cast<int>(data.size())-1; ++i) { // last row
                result[i] -= (data[i-1] + data[i+1] + data[i-w+1] + data[i-w] + data[i-w-1]) / T{5};
            }
            // TR pixel
            if constexpr (horz_wrap) {
                result[i] -= (data[i-1] + data[i-w-1] + data[i-w] + data[i-w+1] + data[i-w-w+1]) / T{5};
            } else {
                result[i] -= (data[i-1] + data[i-w-1] + data[i-w]) / T{3};
            }
        }

        /*
         * Functions which help you to arrange dots on circular rings
         */

        //! How many items (dots, for example) could you arrange on a circle of
        //! radius=@radius with @d between each item's centre?
        template<typename T>
        static int numOnCircle (T radius, T d) {
            if (radius == T{0}) { return 1; }
            T circum = morph::mathconst<T>::two_pi * radius;
            return static_cast<int>(floor (circum / d));
        }

        //! How many items on a circular arc of angle @a?
        template<typename T>
        static int numOnCircleArc (T radius, T d, T a) {
            //std::cout << "Called for radius == " << radius << ", d=" << d <<  std::endl;
            if (radius == T{0}) { return 1; }
            T circum = morph::mathconst<T>::two_pi * radius;
            //std::cout << "circum = " << circum << std::endl;
            T rtn = 0;
#if 1
            // longhand, with a test for a circular arc
            if (a >= morph::mathconst<T>::two_pi) {
                rtn = floor (circum / d);
            } else {
                T proportion = a / morph::mathconst<T>::two_pi;
                //std::cout << "prop = " << proportion << std::endl;
                T arclen = circum * proportion;
                //std::cout << "arclen = " << arclen << std::endl;
                rtn = floor (arclen / d);
            }
#else
            T proportion = a / morph::mathconst<T>::two_pi;
            rtn = floor (circum * proportion / d);
#endif
            //std::cout << "rtn " << rtn << std::endl;
            return rtn;
        }

        //! How many dots spaced by d can be placed on circular arc rings with d between them?
        template<typename T>
        static int numDotsOnRings (T minRadius, T maxRadius, T d,
                                   T a = morph::mathconst<T>::two_pi) {

            // Computation of nrings differs depending on whether we have a dot and nrings, or nrings
            // from minRadius to maxRadius. Herein lies the problem!
            int n_dots = 0;
            if (minRadius == T{0}) {
                int nrings = static_cast<int>(std::floor ((maxRadius-minRadius)/d));
                if (minRadius == T{0}) { // Should this not be `if (nrings ==0) {` ?
                    nrings++; // cos of centre dot.
                }

                for (int r=0; r<nrings; ++r) {
                    n_dots += MathAlgo::numOnCircleArc<T> (minRadius+r*d, d, a);
                }
            } else {
                // Annulus
                int nrings = 1 + static_cast<int>(std::floor ((maxRadius-minRadius)/d));
                //std::cout << nrings << " rings" << std::endl;
                for (int r=0; r<nrings; ++r) {
                    n_dots += MathAlgo::numOnCircleArc<T> (minRadius+r*d, d, a);
                }
            }
            //std::cout << "n_dots for d=" << d << " is " << n_dots << std::endl;
            return n_dots;
        }

        template<typename T>
        static T scale_0_to_almost_2pi (const T angle_rad)
        {
            T _angle = angle_rad;
            // Place _angle in bounds. If <0, then add 2pi. If above 2pi, subtract 2pi.
            if (_angle < T{0}) {
                int multiples = static_cast<int>(std::floor(-_angle / morph::mathconst<T>::two_pi));
                _angle += morph::mathconst<T>::two_pi + multiples * morph::mathconst<T>::two_pi;
            } else {
                int multiples = static_cast<int>(std::floor(_angle / morph::mathconst<T>::two_pi));
                _angle -= multiples * morph::mathconst<T>::two_pi;
            }
            // If angle is indistinguishably close to 2pi, then set it to 0
            _angle = std::abs(_angle - morph::mathconst<T>::two_pi) < std::numeric_limits<T>::epsilon() ? T{0} : _angle;
            return _angle;
        }

    }; // struct MathAlgo

} // namespace morph
