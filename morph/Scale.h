/*!
 * \file
 *
 * \brief A class for scaling signals
 *
 * Scale is a class for scaling (transforming) signals. It has been coded for linear
 * scaling of signals. It could also be used to logarithmically scale signals with
 * suitable extension, and this has been kept in mind (see ScaleFn). It has an
 * autoscaling feature which allows a signal which ranges between x and y to be rescaled
 * to range between 0 and 1 (or -1 and 1 or -w and z)
 *
 * Classes created from the template class morph::Scale will derive from one of the
 * morph::ScaleImpl <ntype, T, S> classes, where ntype is the 'number type' (0 means the
 * numbers are mathematical vectors like morph::Vector, std::array or std::vector; 1
 * means that the numbers are scalars like float, double or int)
 *
 * Usage: See \c tests/testScale.cpp \n
 * e.g.:\n
 * \code{.cpp}
 *   morph::Scale<float> s;
 *   s.do_autoscale = true;
 *   std::vector<float> vf = { 1, 2, 3 };
 *   std::vector<float> result(vf);
 *   s.transform (vf, result);
 *\endcode
 *
 * If the output type is different from the input type, then specify both as template parameters:
 * \code{.cpp}
 *   morph::Scale<int, float> s;
 *   s.do_autoscale = true;
 *   std::vector<int> vi = { -2, -1, 1, 3 };
 *   std::vector<float> result(vi.size());
 *   s.transform (vi, result);
 *\endcode
 *
 * \author Seb James
 * \date April 2020
 */

#pragma once

#include <stdexcept>
#include <cmath>
#include <morph/MathAlgo.h>
#include <morph/number_type.h>

namespace morph {

    //! \brief A label for what kind of scaling transformation to make
    enum class ScaleFn {
        Linear,
        Logarithmic
    };

    /*!
     * \brief Base class for morph::ScaleImpl
     *
     * A base class for specialised implementations of ScaleImpl depending on whether
     * \a T is a scalar type or a (mathematical) vector type.
     *
     * This class contains code common to all implementations of ScaleImpl.
     *
     * \tparam T The type of the number to be scaled. Should be some scalar type such as
     * int, double, etc or a type which can contain a vector such as std::array,
     * morph::Vector or std::vector.
     *
     * \tparam S Output number type. Having separate type allows for scaling of a range
     * of integers into a floating point value between 0 and 1 which can be advantageous
     * for graphing of integers.
     */
    template <typename T, typename S>
    class ScaleImplBase
    {
    public:
        /*!
         * \brief Transform the given datum using this Scale's parameters.
         *
         * I would have preferred to have named this function 'transform', matching
         * transform (const vector<T>&, vector<T>&). Had I done so then I would have
         * to have had two implementations of transform (const vector<T>&, vector<T>&)
         * in ScaleImpl<0,T> and ScaleImpl<1,T>, even though they are identical in
         * each derived class.
         *
         * \param datum The datum (scalar or vector) to be transformed by the current
         * scaling.
         *
         * \return The scaled datum.
         */
        virtual S transform_one (const T& datum) const = 0;

        /*!
         * Inverse transform
         *
         * \param datum The datum to be inverse-transformed
         *
         * \return the de-scaled datum
         */
        virtual T inverse_one (const S& datum) const = 0;

        /*!
         * \brief Transform a container of scalars or vectors.
         *
         * This uses the scaling parameters \b params (ScaleImpl::params) to scale the
         * input \a data. If #do_autoscale is true and #autoscaled is false, then the
         * parameters are computed from the input \a data.
         *
         * \param data The input data
         * \param output The scaled output
         *
         * \tparam Container A container of values (scalar or vector) for input
         * (to-be-transformed) data.
         *
         * \tparam TT Type for the contained input values which may be a scalar or
         * vector
         *
         * \tparam Allocator Part of the plumbing. Memory allocator for Container.
         *
         * \tparam OContainer A container of values (scalar or vector) for the output
         * transformed data.
         *
         * \tparam ST Type for the contained output values which may be a scalar or
         * vector
         *
         * \tparam OAllocator Memory allocator for OContainer.
         */
        template < template <typename, typename> typename Container,
                   typename TT=T,
                   typename Allocator=std::allocator<TT>,
                   template <typename, typename> typename OContainer=Container,
                   typename ST=S,
                   typename OAllocator=std::allocator<ST> >
        void transform (const Container<TT, Allocator>& data, OContainer<ST, OAllocator>& output)
        {
            size_t dsize = data.size();
            if (output.size() != dsize) {
                throw std::runtime_error ("ScaleImplBase::transform(): Ensure data.size()==output.size()");
            }
            if (this->do_autoscale == true && this->autoscaled == false) {
                this->autoscale_from<Container> (data); // not const
            }
            typename Container<TT, Allocator>::const_iterator di = data.begin();
            typename Container<ST, OAllocator>::iterator oi = output.begin();
            while (di != data.end()) {
                *oi++ = this->transform_one (*di++);
            }
        }

        /*!
         * \brief Inverse transform a container of scalars or vectors.
         */
        template < template <typename, typename> typename OContainer,
                   typename ST=S,
                   typename OAllocator=std::allocator<ST>,
                   template <typename, typename> typename Container=OContainer,
                   typename TT=T,
                   typename Allocator=std::allocator<TT> >
        void inverse (const Container<ST, OAllocator>& data, OContainer<T, Allocator>& output)
        {
            size_t dsize = data.size();
            if (output.size() != dsize) {
                throw std::runtime_error ("ScaleImplBase::inverse(): Ensure data.size()==output.size()");
            }
            if (this->do_autoscale == true && this->autoscaled == false) {
                throw std::runtime_error ("ScaleImplBase::inverse(): Can't inverse transform; set params of this Scale, first");
            }
            typename Container<ST, OAllocator>::const_iterator di = data.begin();
            typename Container<TT, Allocator>::iterator oi = output.begin();
            while (di != data.end()) {
                *oi++ = this->inverse_one (*di++);
            }
        }

        /*!
         * \brief Compute scaling parameters
         *
         * Compute the parameters for the scaling given the minimum and maximum inputs
         * such that \a input_min gives \b range_min as output and \a input_max gives \b
         * range_max as output.
         *
         * \param input_min The minimum value of the input data
         * \param input_max The maximum value of the input data
         */
        virtual void compute_autoscale (T input_min, T input_max) = 0;

        /*!
         * \brief Autoscale from data
         *
         * 'Autoscale from data'. Compute the parameters for the scaling given the
         * container of data such that min(\a data) gives \b range_min as output and
         * max(\a data) gives \b range_max as output.
         *
         * This method sub-calls #compute_autoscale, when then modifies
         * ScaleImpl::params.
         *
         * \tparam Container The STL container holding the data. Restricted to those
         * containers which take two arguments for construction. This includes
         * std::vector, std::list but excludes std::map.
         *
         * \tparam TT The type of the contained data (takes a copy of \a T).
         *
         * \tparam Allocator The STL container allocator determined from \a TT.
         *
         * \param data The data from which to determine the scaling parameters. In
         * practice, this will be something like \c std::vector<float> or
         * \c std::list<morph::Vector<double,2>>
         */
        template < template <typename, typename> typename Container,
                   typename TT=T,
                   typename Allocator=std::allocator<TT> >
        void autoscale_from (const Container<TT, Allocator>& data)
        {
            std::pair<TT, TT> mm = MathAlgo::maxmin (data);
            this->compute_autoscale (mm.second, mm.first);
        }

        //! If true, then the parameters (ScaleImpl::params) have been set by
        //! autoscaling
        bool autoscaled = false;

        //! Set to true to make the Scale object compute autoscaling when data is
        //! available, i.e. on the first call to #transform.
        bool do_autoscale = false;

        // Set type for transformations/autoscaling
        void setType (ScaleFn t)
        {
            // Reset autoscaled, because any autoscaling will need to be re-computed
            this->autoscaled = false;
            this->type = t;
        }

        void setlog (void)
        {
            this->autoscaled = false;
            this->type = ScaleFn::Logarithmic;
        }

        void setlinear (void)
        {
            this->autoscaled = false;
            this->type = ScaleFn::Linear;
        }

        virtual bool ready() const = 0;

    protected:
        /*!
         * What type of scaling function is in use? Intended for future implementations
         * when Scale could carry out logarithmic (or other) scalings, in addition to
         * linear transforms.
         */
        ScaleFn type = ScaleFn::Linear;
    };

    /*!
     * \brief ScaleImpl for vector \a T
     *
     * A default implementation base class for Scale which is used when the number type
     * of \a T is a vector such as std::array or morph::Vector.
     *
     * \tparam ntype The 'number type' as contained in number_type::value. 1 for
     * vectors, 0 for scalars. Default is 0. This class is active if ntype is 0. There
     * is a specialization of this class which is active if ntype is 1. Other values of
     * ntype would activate this default implementation.
     *
     * \tparam T The type of the number to be scaled. Should be some scalar type such as
     * int, double, etc or a type which can contain a vector such as std::array,
     * morph::Vector or std::vector. It is from the type \a T that ntype is determined.
     *
     * \sa ScaleImplBase
     */
    template <int ntype = 0, typename T=float, typename S=float>
    class ScaleImpl : public ScaleImplBase<T, S>
    {
    public:
        //! In a vector implementation we have to get the type of the components of the
        //! vector type \a T. The component (or element) type is named \a T_el.
        using T_el=std::remove_reference_t<decltype(*std::begin(std::declval<T&>()))>;
        //! Element type of S
        using S_el=std::remove_reference_t<decltype(*std::begin(std::declval<S&>()))>;

        //! minimum for autoscaling. After autoscaling, the minimum value of the scaled
        //! values should be have this value.
        S_el range_min = 0.0;
        //! maximum for autoscaling. After autoscaling, the maxmum value of the scaled
        //! values should be have this value.
        S_el range_max = 1.0;

        //! Transform a single (math) vector T into a (math) vector S
        virtual S transform_one (const T& datum) const
        {
            if (this->type != ScaleFn::Linear) {
                throw std::runtime_error ("This transform function is for Linear scaling only");
            }
            if (params.size() != 2) {
                throw std::runtime_error ("For linear scaling of ND vector lengths, need 2 params (set do_autoscale or call setParams())");
            }
            S rtn(datum);
            T_el vec_len = this->vec_length (datum);
            for (size_t i = 0; i < datum.size(); ++i) {
                rtn[i] = static_cast<S_el>(datum[i]) * this->params[0] + this->params[1];
                S_el el_len = static_cast<S_el>(datum[i]);
                // Here's the scaling of a component
                rtn[i] = (el_len - (el_len/static_cast<S_el>(vec_len))*this->params[1]) * this->params[0];
            }
            return rtn;
        }

        virtual T inverse_one (const S& datum) const
        {
            throw std::runtime_error ("Inverse transform not yet implemented for vectors");
        }

        virtual void compute_autoscale (T input_min, T input_max)
        {
            if (this->type != ScaleFn::Linear) {
                throw std::runtime_error ("This autoscale function is for Linear scaling only");
            }
            this->params.resize (2, T_el{0});
            // Vector version: get lengths of input_min/max
            T_el imax_len = vec_length (input_max);
            T_el imin_len = vec_length (input_min);
            // Handle imax_len == imin_len
            if (imax_len == imin_len) {
                // params[0] is already 0
                this->params[1] = (this->range_max-this->range_min)/S_el{2};
            } else {
                // m = rise/run
                this->params[0] = (this->range_max - this->range_min) / static_cast<S_el>(imax_len - imin_len);
                // c = y - mx => min = m * input_min + c => c = min - (m * input_min)
                this->params[1] = static_cast<S_el>(imin_len); // this->range_min - (this->params[0] * imin_len);
            }

            this->autoscaled = true;
        }

        //! Set params for a two parameter scaling
        //! \param p0 The zeroth parameter
        //! \param p1 The first parameter
        void setParams (S_el p0, S_el p1)
        {
            this->do_autoscale = false;
            this->params.resize (2, S_el{0});
            this->params[0] = p0;
            this->params[1] = p1;
        }

        //! Getter for params
        //! \param idx The index into #params
        //! \return The specified element of #params
        S_el getParams (size_t idx) { return this->params[idx]; }

        //! The Scale object is ready if params has size 2.
        bool ready() const { return (this->params.size() > 1); }

    private:
        //! Compute vector length
        //! \param vec the vector of type \a T
        //! \return The vector's length
        T_el vec_length (const T& vec) const
        {
            T_el sos = T_el{0};
            typename T::const_iterator vi = vec.begin();
            while (vi != vec.end()) {
                const T_el val = *vi;
                sos += (val * val);
                ++vi;
            }
            return std::sqrt (sos);
        }

        //! The parameters for the scaling. For linear scaling, this will contain two
        //! scalar values. Note the type is the output element type
        std::vector<S_el> params;
    };

    /*!
     * \brief ScaleImpl for scalar \a T
     *
     * A specialized implementation base class for the template class Scale, which is used
     * when the number type of \a T is a scalar such as int, double, float or long
     * double.
     *
     * \tparam ntype The 'number type' as contained in number_type::value. 0 for
     * vectors, 1 for scalars. This class is active only for ntype==1 (scalar).
     *
     * \tparam T The type of the number to be scaled. Should be some scalar type such as
     * int, double. It is from the type \a T that ntype is determined.
     *
     * \tparam S The output type for the scaled number. For integer T, this might well
     * be a floating point type.
     *
     * \sa ScaleImplBase
     */
    template<typename T, typename S>
    class ScaleImpl<1, T, S> : public ScaleImplBase<T, S>
    {
    public:
        //! minimum for autoscaling. After autoscaling, the minimum value of the scaled
        //! values should be have this value.
        S range_min = 0.0;
        //! maximum for autoscaling. After autoscaling, the maxmum value of the scaled
        //! values should be have this value.
        S range_max = 1.0;

        virtual S transform_one (const T& datum) const
        {
            S rtn = S{0};
            if (this->type == ScaleFn::Logarithmic) {
                rtn = this->transform_one_log (datum);
            } else if (this->type == ScaleFn::Linear) {
                rtn = this->transform_one_linear (datum);
            } else {
                throw std::runtime_error ("Unknown scaling");
            }
            return rtn;
        }

        virtual T inverse_one (const S& datum) const
        {
            T rtn = T{0};
            if (this->type == ScaleFn::Logarithmic) {
                rtn = this->inverse_one_log (datum);
            } else if (this->type == ScaleFn::Linear) {
                rtn = this->inverse_one_linear (datum);
            } else {
                throw std::runtime_error ("Unknown scaling");
            }
            return rtn;
        }

        virtual void compute_autoscale (T input_min, T input_max)
        {
            if (this->type == ScaleFn::Logarithmic) {
                this->compute_autoscale_log (input_min, input_max);
            } else if (this->type == ScaleFn::Linear) {
                this->compute_autoscale_linear (input_min, input_max);
            } else {
                throw std::runtime_error ("Unknown scaling");
            }
            this->autoscaled = true;
        }

        //! Set params for a two parameter scaling
        //! \param p0 The zeroth parameter
        //! \param p1 The first parameter
        void setParams (S p0, S p1)
        {
            this->params.resize (2, S{0});
            this->params[0] = p0;
            this->params[1] = p1;
        }

        //! Getter for params
        //! \param idx The index into #params
        //! \return The specified element of #params
        T getParams (size_t idx) { return this->params[idx]; }

        //! The Scale object is ready if params has size 2.
        bool ready() const { return (this->params.size() > 1); }

    private:
        //! Linear transform for scalar type; y = mx + c
        S transform_one_linear (const T& datum) const
        {
            return (datum * this->params[0] + this->params[1]);
        }

        //! Log transform for scalar type
        S transform_one_log (const T& datum) const
        {
            return (transform_one_linear (std::log(datum)));
        }

        //! The inverse linear transform; x = (y-c)/m
        T inverse_one_linear (const S& datum) const
        {
            return ((datum-this->params[1])/this->params[0]);
        }

        //! The inverse of the log transform is exp.
        T inverse_one_log (const S& datum) const
        {
            T res = inverse_one_linear(datum);
            return (std::exp (res));
        }

        void compute_autoscale_linear (T input_min, T input_max)
        {
            // Here, we need to use the output type for the computations. Does that mean
            // params is stored in the output type? I think it does.
            this->params.resize (2, S{0});
            if (input_min == input_max) {
                this->params[0] = T{0};
                this->params[1] = (this->range_max - this->range_min) / S{2.0};
            } else {
                // m = rise/run
                this->params[0] = (this->range_max - this->range_min) / static_cast<S>(input_max - input_min);
                // c = y - mx => min = m * input_min + c => c = min - (m * input_min)
                this->params[1] = this->range_min - (this->params[0] * static_cast<S>(input_min));
            }
        }

        void compute_autoscale_log (T input_min, T input_max)
        {
            T ln_imin = std::log(input_min);
            T ln_imax = std::log(input_max);
            // Now just scale linearly between ln_imin and ln_imax
            this->compute_autoscale_linear (ln_imin, ln_imax);
        }

        //! The parameters for the scaling. If linear, this will contain two scalar
        //! values.
        std::vector<S> params;
    };

    /*!
     * A class for scaling and normalizing signals.
     *
     * Mostly used for linear scaling of signals, has an autoscale feature. Could also
     * be used to logarithmically scale a signal with suitable extension.
     *
     * morph::Scale derives from ScaleImpl<N> depending on the type of T
     *
     * Usage: See tests/testScale.cpp
     * e.g.:
     *   morph::Scale<float> s;
     *   s.do_autoscale = true;
     *   std::vector<float> vf = { 1, 2, 3 };
     *   std::vector<float> result(vf);
     *   s.transform (vf, result);
     *
     * \tparam T The type of the numbers (or vectors) that will be scaled.
     *
     * \tparam S The type of the output numbers (or for vectors, their
     * elements). Defaults to be the same type as T, but when scaling integers, may well
     * be a different type such as float or double
     */
    template <typename T, typename S=T>
    struct Scale : public ScaleImpl<number_type<T>::value, T, S> {};

} // namespace morph
