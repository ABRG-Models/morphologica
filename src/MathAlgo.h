#ifndef _MATHALGO_H_
#define _MATHALGO_H_

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
#include <utility>
using std::pair;
using std::make_pair;
#include <iostream>
using std::endl;
using std::cout;

/*!
 * Mathematical algorithms in the morph namespace.
 */

namespace morph {

    /*!
     * A class containing some algorithms applied to numbers of type T. T may be floating point or
     * integer types, but some methods may not make much sense for integer types.
     */
    template <class T>
    class MathAlgo
    {
    public:
        //! Centroid of a set of 2D coordinates @points.
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
        //! Centroid of a set of 2D coordinates @points, assumed to be in order x1,y1,x2,y2,etc
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
        //! Centroid of a set of 3D coordinates @points, assumed to be in order x1,y1,z1, x2,y2,z2, etc
        static array<T,3> centroid3D (const vector<T> points) {
            array<T,3> centroid;
            centroid[0] = static_cast<T>(0);
            centroid[1] = static_cast<T>(0);
            centroid[2] = static_cast<T>(0);
            size_t psz = points.size();
            for (size_t i = 0; i < psz-2; i+=3) {
                centroid[0] += points[i];
                centroid[1] += points[i+1];
                centroid[1] += points[i+2];
            }
            centroid[0] /= (psz/3);
            centroid[1] /= (psz/3);
            centroid[3] /= (psz/3);
            return centroid;
        }

        //! Compute standard deviation of the T values in @values. Return SD.
        static T compute_sd (const vector<T>& values) {
            T mean = 0.0;
            return MathAlgo<T>::compute_mean_sd (values, mean);
        }

        //! Compute standard deviation of the T values in @values. Return SD, write mean into arg.
        static T compute_mean_sd (const vector<T>& values, T& mean) {
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
            return sqrt(variance);
        }

        //! The bubble sort algorithm, high to low. T could be floating point or integer types.
        static void bubble_sort_hi_to_lo (vector<T>& values) {
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

        //! The bubble sort algorithm, low to high. T could be floating point or integer types.
        static void bubble_sort_lo_to_hi (vector<T>& values) {
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

        //! Bubble sort, high to low, order is returned in indices, values are left unchanged
        static void bubble_sort_hi_to_lo (const vector<T>& values, vector<unsigned int>& indices) {

            vector<T> vcopy = values;

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

        //! Bubble sort, low to high, order is returned in indices, values are left unchanged
        static void bubble_sort_lo_to_hi (const vector<T>& values, vector<unsigned int>& indices) {

            vector<T> vcopy = values;

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

        // Fixme: Use traits to make it possible to generalise the container in these...
        //@{
        //! Return the max and min of the vector of values
        static pair<T, T> maxmin (const vector<T>& values) {
            T max = numeric_limits<T>::lowest();
            T min = numeric_limits<T>::max();
            for (auto v : values) {
                max = v > max ? v : max;
                min = v < min ? v : min;
            }
            return make_pair (max, min);
        }

        //! Return the max and min of the list of values
        static pair<T, T> maxmin (const list<T>& values) {
            T max = numeric_limits<T>::lowest();
            T min = numeric_limits<T>::max();
            for (auto v : values) {
                max = v > max ? v : max;
                min = v < min ? v : min;
            }
            return make_pair (max, min);
        }
        //@}
    };

}

#endif // _MATHALGO_H_
