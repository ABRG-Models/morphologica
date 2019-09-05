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

        //! Compute standard deviation of the T values in @values
        static T compute_sd (const vector<T>& values) {
            T mean = 0.0;
            for (T val : values) {
                mean += val;
            }
            mean /= values.size();
            T sos_deviations = 0.0;
            for (T val : values) {
                sos_deviations += ((val-mean)*(val-mean));
            }
            return sqrt(sos_deviations);
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
    };

}

#endif // _MATHALGO_H_
