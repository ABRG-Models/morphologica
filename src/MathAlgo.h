#ifndef _MATHALGO_H_
#define _MATHALGO_H_

#include <vector>
using std::vector;
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
