/*!
 * A class for scaling signals. Mostly used for linear scaling of signals, has an
 * autoscale feature. Could also be used to logarithmically scale a signal.
 *
 * Usage: See tests/testScale.cpp
 * e.g.:
 *   morph::Scale<float> s;
 *   s.do_autoscale = true;
 *   std::vector<float> vf = { 1, 2, 3 };
 *   std::vector<float> result(vf);
 *   s.transform (vf, result);
 *
 * Author: Seb James
 * Date: April 2020
 */

#pragma once

#include <stdexcept>
using std::runtime_error;
#include <cmath>
using std::sqrt;
#include "MathAlgo.h"
using morph::MathAlgo;
#include "number_type.h"

namespace morph {

    //! A label for what kind of scaling transformation to make
    enum class ScaleFn {
        Linear,
        Logarithmic
    };

    /*!
     * A base class for specialised implementations of ScaleImpl depending on whether
     * T is a scalar type or a (mathematical) vector type.
     *
     * Perhaps this is a base class for LinearScale, LogScale etc? Possibly in the
     * future when someone wants lots of different scaling function.
     *
     * I'm annoyed that I can't seem to put void transform (const vector<T>& data,
     * vector<T>& output) in this base class.
     */
    template <typename T>
    class ScaleImplBase {
    public:
        /*!
         * Transform the given datum using this Scale's parameters.
         *
         * I would have preferred to have named this function 'transform', matching
         * transform (const vector<T>&, vector<T>&). Had I done so then I would have
         * to have had two implementations of transform (const vector<T>&, vector<T>&)
         * in ScaleImpl<0,T> and ScaleImpl<1,T>, even though they are identical in
         * each derived class.
         */
        virtual T transform_one (const T& datum) const = 0;

        /*!
         * Transform a vector of data.
         *
         * FIXME: Make it possible to use any Container, not just vector<T>. See
         * MathAlgo for a way to achieve this.
         */
        void transform (const vector<T>& data, vector<T>& output) {
            size_t dsize = data.size();
            if (output.size() != dsize) {
                throw runtime_error ("ScaleImplBase::transform(): Ensure data.size()==output.size()");
            }
            if (this->do_autoscale == true && this->autoscaled == false) {
                this->autoscale_from (data); // not const
            }
            for (size_t i = 0; i < dsize; ++i) {
                output[i] = this->transform_one (data[i]);
            }
        }

        //! Compute the params for the scaling given the minimum and maximum inputs
        //! such that input_min gives this->min as output and input_max gives
        //! this->max as output.
        virtual void compute_autoscale (T input_min, T input_max) = 0;

        //! 'Autoscale from data'. Compute the params for the scaling given the vector
        //! of data such that min(data) gives this->min as output and max(data) gives
        //! this->max as output.
        void autoscale_from (const vector<T>& data) {
            pair<T, T> mm = MathAlgo::maxmin (data);
            this->compute_autoscale (mm.second, mm.first);
        }

        //! If true, then the params have been set by autoscaling
        bool autoscaled = false;

        //! Set to true to make the Scale object compute autoscaling when data is
        //! available.
        bool do_autoscale = false;

    protected:
        //! What type of scaling function is in use?
        ScaleFn type = ScaleFn::Linear;
    };

    //! Vector implementation
    template <int vtype = 0, typename T=float>
    class ScaleImpl : public ScaleImplBase<T>
    {
    public:
        using T_el=std::remove_reference_t<decltype(*std::begin(std::declval<T&>()))>;

        //! maximum and minimum for autoscaling. In vector implementation have to use
        //! the type of the elements of T
        //@{
        T_el range_min = 0.0;
        T_el range_max = 1.0;
        //@}

        //! Transform the given datum using this Scale's parameters. This is a linear
        //! scaling of the vector lengths only, so just need two parameters.
        virtual T transform_one (const T& datum) const {
            if (this->type != ScaleFn::Linear) {
                throw runtime_error ("This transform function is for Linear scaling only");
            }
#if 0
            // It's an array type; a (mathematical) vector. Scale the vector by m
            // times. params should contain enough values for c to be a vector
            // itself.
            if (params.size() <= datum.size()) {
                throw runtime_error ("For linear scaling of ND vector (with vector offset), need N+1 params");
            }
            T rtn;
            for (size_t i = 0; i < datum.size(); ++i) {
                rtn[i] = datum[i] * this->params[0] + this->params[1+i];
            }
#else
            // Simple approach; scale ONLY vector lengths
            if (params.size() != 2) {
                throw runtime_error ("For linear scaling of ND vector lengths, need 2 params (set do_autoscale or call setParams())");
            }
            T rtn(datum);
            T_el vec_len = this->vec_length (datum);
            for (size_t i = 0; i < datum.size(); ++i) {
                rtn[i] = datum[i] * this->params[0] + this->params[1];
                T_el el_len = datum[i];
                // Here's the scaling of a component
                rtn[i] = (el_len - (el_len/vec_len)*this->params[1]) * this->params[0];
            }
#endif
            return rtn;
        }

        //! Compute the params for the scaling given the minimum and maximum inputs
        //! such that input_min gives this->min as output and input_max gives
        //! this->max as output.
        virtual void compute_autoscale (T input_min, T input_max) {
            if (this->type != ScaleFn::Linear) {
                throw runtime_error ("This autoscale function is for Linear scaling only");
            }
            this->params.resize (2, static_cast<T_el>(0.0));
            // Vector version: get lengths of input_min/max
            T_el imax_len = vec_length (input_max);
            T_el imin_len = vec_length (input_min);
            // m = rise/run
            this->params[0] = (this->range_max - this->range_min) / (imax_len - imin_len);
            // c = y - mx => min = m * input_min + c => c = min - (m * input_min)
            this->params[1] = imin_len; // this->range_min - (this->params[0] * imin_len);

            this->autoscaled = true;
        }

        //! Set params for a two parameter scaling
        void setParams (T_el p0, T_el p1) {
            this->params.resize (2, static_cast<T_el>(0.0));
            this->params[0] = p0;
            this->params[1] = p1;
        }

    private:
        //! Compute vector length
        T_el vec_length (const T& vec) const {
            T_el sos = static_cast<T_el>(0);
            typename T::const_iterator vi = vec.begin();
            while (vi != vec.end()) {
                const T_el val = *vi;
                sos += (val * val);
                ++vi;
            }
            return sqrt (sos);
        }

    private:
        //! A vector of parameters for this scaling function.
        vector<T_el> params;
    };

    //! Specialised class for Scalar only stuff. First inherit the vector/common
    //! stuff, then override some of the methods
    template<typename T>
    class ScaleImpl<1, T> : public ScaleImplBase<T>
    {
    public:
        //! maximum and minimum for autoscaling
        //@{
        T range_min = 0.0;
        T range_max = 1.0;
        //@}

        //! Transform the given datum using this Scale's parameters. How to deal with
        //! regular types and arrays of types? T might be scalar, or vector.
        virtual T transform_one (const T& datum) const {
            if (this->type != ScaleFn::Linear) {
                throw runtime_error ("This transform function is for Linear scaling only");
            }
            // Scalar type; y = mx + c
            return (datum * this->params[0] + this->params[1]);
        }

        //! Compute the params for the scaling given the minimum and maximum inputs
        //! such that input_min gives this->min as output and input_max gives
        //! this->max as output.
        virtual void compute_autoscale (T input_min, T input_max) {
            if (this->type != ScaleFn::Linear) {
                throw runtime_error ("This autoscale function is for Linear scaling only");
            }
            this->params.resize (2, static_cast<T>(0.0));
            // m = rise/run
            this->params[0] = (this->range_max - this->range_min) / (input_max - input_min);
            // c = y - mx => min = m * input_min + c => c = min - (m * input_min)
            this->params[1] = this->range_min - (this->params[0] * input_min);
            this->autoscaled = true;
        }

        //! Set params for a two parameter scaling
        void setParams (T p0, T p1) {
            this->params.resize (2, static_cast<T>(0.0));
            this->params[0] = p0;
            this->params[1] = p1;
        }

    private:
        vector<T> params;
    };

    //! morph::Scale derives from ScaleImpl<N> depending on the type of T
    template <typename T>
    struct Scale : public ScaleImpl<number_type<T>::value, T> {};

} // namespace morph
