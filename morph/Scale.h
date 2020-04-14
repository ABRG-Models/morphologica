/*!
 * A class for scaling signals. Mostly used for linear scaling of signals, has an
 * autoscale feature. Could also be used to logarithmically scale a signal.
 *
 * Author: Seb James
 * Date: April 2020
 */

#pragma once

#include <stdexcept>
using std::runtime_error;
#include "MathAlgo.h"
using morph::MathAlgo;
#include <type_traits>
using std::is_array;
using std::is_vector;

namespace morph {

    //! What kind of scaling transformation to make
    enum class ScaleFn {
        Linear,
        Logarithmic
    };

    //! Perhaps this is a base class for LinearScale, LogScale etc? Possibly in the
    //! future when someone wants lots of different scaling function.
    template <typename T>
    class Scale {
    public:
        Scale(){}
        ~Scale(){}

        //! Transform the given datum using this Scale's parameters. How to deal with
        //! regular types and arrays of types? T might be scalar, or vector.
        virtual inline T transform (const T& datum) const {
            if (this->type != ScaleFn::Linear) {
                throw runtime_error ("This transform function is for Linear scaling only");
            }
            if (is_array<T>::value == true) {
                // It's an array type; a (mathematical) vector. Scale the vector by m
                // times. params should contain enough values for c to be a vector
                // itself.
                cout << "T has array type." << endl;
                if (params.size() <= datum.size()) {
                    throw runtime_error ("For linear scaling of ND vector, need N+1 params");
                }
                T rtn;
                for (size_t i = 0; i < datum.size(); ++i) {
                    rtn[i] = datum[i] * this->params[0] + this->params[1+i];
                }
                return rtn;
            } else {
                // Assume scalar type; y = mx + c
                return (datum * this->params[0] + this->params[1]);
            }
        }

        //! Transform a vector of data
        virtual void transform (const vector<T>& data, vector<T>& output) const {
            size_t dsize = data.size();
            if (output.size() != dsize) {
                output.resize (dsize, static_cast<T>(0));
            }
            for (size_t i = 0; i < dsize; ++i) {
                output[i] = this->transform (data[i]);
            }
        }

        //! Compute the params for the scaling given the minimum and maximum inputs
        //! such that input_min gives this->min as output and input_max gives
        //! this->max as output.
        virtual void autoscale (T input_min, T input_max) {
            if (this->type != ScaleFn::Linear) {
                throw runtime_error ("This autoscale function is for Linear scaling only");
            }
            // m = rise/run
            this->params[0] = (this->max - this->min) / (input_max - input_min);
            // c = y - mx => min = m * input_min + c => c = min - (m * input_min)
            this->params[1] = this->min - (this->params[0] * input_min);
            this->autoscaled = true;
        }
        //! Compute the params for the scaling given the vector of data such that
        //! min(data) gives this->min as output and max(data) gives this->max as
        //! output
        virtual void autoscale (const vector<T>& data) {
            pair<T, T> maxmin = MathAlgo<T>::maxmin (data);
            this->autoscale (maxmin.first, maxmin.second);
        }

        //! Set params for a two parameter scaling
        void setParams (T p0, T p1) {
            this->params.resize (2, static_cast<T>(0.0));
            this->params[0] = p0;
            this->params[1] = p1;
        }

        //! If true, then the params have been set by autoscaling
        bool autoscaled = false;

        bool do_autoscale = false;

        //! maximum and minimum for autoscaling
        //@{
        T min; // 0.0; or {0,0,0}; or whatever
        T max; // 1.0; or {1/sqrt(3), 1/sqrt(3), 1/sqrt(3)}; (length 1)
        //@}

    private:
        //! A vector of parameters for this scaling function.
        vector<T> params;

        //! What type of scaling function is in use?
        ScaleFn type = ScaleFn::Linear;
    };

} // namespace morph
