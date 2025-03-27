/*!
 * \file
 *
 * \brief A class for scaling signals
 *
 * scale is a class for scaling (transforming) signals. It has been coded for linear scaling of
 * signals. It could also be used to logarithmically scale signals with suitable extension, and this
 * has been kept in mind (see ScaleFn). It has an autoscaling feature which allows a signal which
 * ranges between x and y to be rescaled to range between 0 and 1 (or -1 and 1 or -w and z)
 *
 * Classes created from the template class morph::scale will derive from one of the morph::ScaleImpl
 * <ntype, T, S> classes, where ntype is the 'number type' (0 means the numbers are mathematical
 * vectors like morph::vec, std::array or std::vector; 1 means that the numbers are scalars like
 * float, double or int)
 *
 * Usage: See \c tests/testScale.cpp \n
 * e.g.:\n
 * \code{.cpp}
 *   morph::scale<float> s;
 *   s.do_autoscale = true;
 *   std::vector<float> vf = { 1, 2, 3 };
 *   std::vector<float> result(vf);
 *   s.transform (vf, result);
 *\endcode
 *
 * If the output type is different from the input type, then specify both as template parameters:
 * \code{.cpp}
 *   morph::scale<int, float> s;
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
#include <cstddef>
#include <string>
#include <sstream>
#include <morph/MathAlgo.h>
#include <morph/trait_tests.h>
#include <morph/vvec.h>
#include <morph/range.h>

namespace morph {

    //! \brief A label for what kind of scaling transformation to make
    enum class scaling_function {
        Linear,
        Logarithmic
    };

    // For stream operator
    template <typename T, typename S> struct scale_impl_base;
    template <typename T, typename S> std::ostream& operator<< (std::ostream&, const scale_impl_base<T, S>&);

    /*!
     * \brief Base class for morph::scale_impl
     *
     * A base class for specialised implementations of scale_impl depending on whether \a T is a
     * scalar type or a (mathematical) vector type.
     *
     * This class contains code common to all implementations of scale_impl.
     *
     * \tparam T The type of the number to be scaled. Should be some scalar type such as int,
     * double, etc or a type which can contain a vector such as std::array, morph::vec or
     * std::vector.
     *
     * \tparam S Output number type. Having separate type allows for scaling of a range of integers
     * into a floating point value between 0 and 1 which can be advantageous for graphing of
     * integers.
     */
    template <typename T, typename S>
    struct scale_impl_base
    {
    public:
        /*!
         * \brief Transform the given datum using this scale's parameters.
         *
         * I would have preferred to have named this function 'transform', matching transform (const
         * vector<T>&, vector<T>&). Had I done so then I would have to have had two implementations
         * of transform (const vector<T>&, vector<T>&) in scale_impl<0,T> and scale_impl<1,T>, even
         * though they are identical in each derived class.
         *
         * \param datum The datum (scalar or vector) to be transformed by the current scaling.
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
         * Output a short description of the scaling
         */
        std::string str() const
        {
            std::string _type = this->type == scaling_function::Linear ? "Linear" : "Logarithmic";
            std::stringstream ss;
            ss << _type << " scaling " << typeid(T).name() << " to " << typeid(S).name()
               << " as: " << this->transform_str()
               << ". ready()=" << (this->ready() ? "true" : "false")
               << ", do_autoscale=" << (this->do_autoscale ? "true" : "false")
               << ", params=" << this->params_str() << ", output range: " << this->output_range_str();
            return ss.str();
        }

        //! Output the params vvec as a string
        virtual std::string params_str() const = 0;
        //! Describe the transformation in a text string
        virtual std::string transform_str() const = 0;
        //! Describe the output range in a text string
        virtual std::string output_range_str() const = 0;

        /*!
         * \brief Transform a container of scalars or vectors.
         *
         * This uses the scaling parameters \b params (scale_impl::params) to scale the input \a
         * data. If #do_autoscale is true and this->ready() is false, then the parameters are computed
         * from the input \a data.
         *
         * \param data The input data
         * \param output The scaled output
         *
         * \tparam Container A container of values (scalar or vector) for input (to-be-transformed)
         * data.
         *
         * \tparam TT Type for the contained input values which may be a scalar or vector
         *
         * \tparam Allocator Part of the plumbing. Memory allocator for Container.
         *
         * \tparam OContainer A container of values (scalar or vector) for the output transformed
         * data.
         *
         * \tparam ST Type for the contained output values which may be a scalar or vector
         *
         * \tparam OAllocator Memory allocator for OContainer.
         */
        template <typename Container, typename OContainer=Container>
        std::enable_if_t<morph::is_copyable_container<Container>::value
                         && morph::is_copyable_container<OContainer>::value, void>
        transform (const Container& data, OContainer& output)
        {
            std::size_t dsize = data.size();
            if (output.size() != dsize) {
                throw std::runtime_error ("scale_impl_base::transform(): Ensure data.size()==output.size()");
            }
            if (this->do_autoscale == true && !this->ready()) {
                this->compute_scaling_from_data<Container> (data); // not const
            } else if (this->do_autoscale == false && !this->ready()) {
                throw std::runtime_error ("scale_impl_base::transform(): Params are not set and do_autoscale is set false. Can't transform.");
            }
            typename Container::const_iterator di = data.begin();
            typename OContainer::iterator oi = output.begin();
            while (di != data.end()) { *oi++ = this->transform_one (*di++); }
        }

        /*!
         * \brief Inverse transform a container of scalars or vectors.
         */
        template <typename OContainer, typename Container=OContainer>
        std::enable_if_t<morph::is_copyable_container<Container>::value
                         && morph::is_copyable_container<OContainer>::value, void>
        inverse (const Container& data, OContainer& output)
        {
            std::size_t dsize = data.size();
            if (output.size() != dsize) {
                throw std::runtime_error ("scale_impl_base::inverse(): Ensure data.size()==output.size()");
            }
            if (!this->ready()) {
                throw std::runtime_error ("scale_impl_base::inverse(): Can't inverse transform; set params of this scale, first");
            }
            typename Container::const_iterator di = data.begin();
            typename OContainer::iterator oi = output.begin();
            while (di != data.end()) { *oi++ = this->inverse_one (*di++); }
        }

        //! Transform a range
        morph::range<S> transform (const morph::range<T>& data)
        {
            return morph::range<S>{ this->transform_one (data.min), this->transform_one (data.max) };
        }

        //! Inverse transform a range
        morph::range<T> inverse (const morph::range<S>& data)
        {
            return morph::range<T>{ this->inverse_one (data.min), this->inverse_one (data.max) };
        }

        /*!
         * \brief Compute scaling parameters
         *
         * Compute the parameters for the scaling given the minimum and maximum inputs such that \a
         * input_min gives \b range_min as output and \a input_max gives \b output_range.max as output.
         *
         * \param input_min The minimum value of the input data
         * \param input_max The maximum value of the input data
         */
        virtual void compute_autoscale (T input_min, T input_max) = 0; // deprecated name, left to avoid breaking client code
        virtual void compute_scaling (const T input_min, const T input_max) = 0;
        virtual void compute_scaling (const morph::range<T>& input_range) = 0;

        //! Set the identity scaling (a linear scaling with 0 mapping to 0 and 1 mapping to 1)
        virtual void identity_scaling() = 0;

        //! Set null scaling. This scales any value to S{0}
        virtual void null_scaling() = 0;

        /*!
         * \brief Compute scaling function from data
         *
         * 'Compute the scaling function from the data'. This function computes the
         * parameters for the scaling given the container of data such that min(\a data)
         * gives \b output_range.min as output and max(\a data) gives \b
         * output_range.max as output.
         *
         * This method sub-calls #compute_scaling(T, T), which then modifies scale_impl::params.
         *
         * \tparam Container The STL container holding the data. Restricted to those containers
         * which take two arguments for construction. This includes std::vector, std::list but
         * excludes std::map.
         *
         * \tparam TT The type of the contained data (takes a copy of \a T).
         *
         * \tparam Allocator The STL container allocator determined from \a TT.
         *
         * \param data The data from which to determine the scaling parameters. In practice, this
         * will be something like \c std::vector<float> or \c std::list<morph::vec<double,2>>
         */
        template <typename Container>
        std::enable_if_t<morph::is_copyable_container<Container>::value, void>
        compute_scaling_from_data (const Container& data)
        {
            morph::range<typename Container::value_type> mm = MathAlgo::maxmin (data);
            this->compute_scaling (mm.min, mm.max);
        }

        //! Set to true to make the scale object compute autoscaling when data is available, i.e. on
        //! the first call to #transform.
        bool do_autoscale = false;

        // Set type for transformations/autoscaling
        void setType (scaling_function t)
        {
            // Reset, because any autoscaling will need to be re-computed
            this->reset();
            this->type = t;
        }

        scaling_function getType() { return this->type; }

        void setlog()
        {
            this->reset();
            this->type = scaling_function::Logarithmic;
        }

        void setlinear()
        {
            this->reset();
            this->type = scaling_function::Linear;
        }

        virtual bool ready() const = 0;

        virtual void reset() = 0;

    protected:
        /*!
         * What type of scaling function is in use? Intended for future implementations when scale
         * could carry out logarithmic (or other) scalings, in addition to linear transforms.
         */
        scaling_function type = scaling_function::Linear;

    public:
        //! Overload the stream output operator
        friend std::ostream& operator<< <T> (std::ostream& os, const scale_impl_base<T, S>& scl);
    };

    template <typename T, typename S>
    std::ostream& operator<< (std::ostream& os, const scale_impl_base<T, S>& scl)
    {
        os << scl.str();
        return os;
    }

    /*!
     * \brief scale_impl for vector \a T
     *
     * A default implementation base class for scale which is used when the number type of \a T is a
     * vector such as std::array or morph::vec.
     *
     * FIXME: Rename ntype, because T is 'input number type' and S is 'output number
     * type' with type meaning machine number type. ntype would be better as vector_scalar or similar.
     *
     * \tparam ntype The 'number type' as contained in morph::number_type::value. 1 for vectors, 0
     * for scalars, 2 for complex, 3 for vector of complex (not supported by scale). Default is
     * 0. This class is active if ntype is 0 (vector). There are other specializations of this class
     * which are active if ntype is 1 or 2.
     *
     * \tparam T The type of the number to be scaled. Should be some scalar type such as
     * int, double, etc or a type which can contain a vector such as std::array,
     * morph::vec or std::vector. It is from the type \a T that ntype is
     * determined. From T, the element type, T_el is derived.
     *
     * \tparam S The output number type. From S is derived the type of the output
     * elements, S_el.
     *
     * \sa scale_impl_base
     */
    template <int ntype = 0, typename T=float, typename S=float>
    class scale_impl : public scale_impl_base<T, S>
    {
    public:
        //! In a vector implementation we have to get the type of the components of the vector type
        //! \a T. The component (or element) type is named \a T_el.
        using T_el=std::remove_reference_t<decltype(*std::begin(std::declval<T&>()))>;
        //! Element type of S
        using S_el=std::remove_reference_t<decltype(*std::begin(std::declval<S&>()))>;

        //! The output range required. Change if you want to scale to something other than [0, 1]
        morph::range<S_el> output_range = morph::range<S_el>(S_el{0}, S_el{1});

        //! Transform a single (math) vector T into a (math) vector S
        S transform_one (const T& datum) const
        {
            if (this->type != scaling_function::Linear) {
                throw std::runtime_error ("scale_impl<0=vector>::transform_one(): This transform function is for Linear scaling only");
            }
            if (params.size() != 2) {
                throw std::runtime_error ("scale_impl<0=vector>::transform_one(): For linear scaling of ND vector lengths, need 2 params (set do_autoscale or call setParams())");
            }
            S rtn(datum);
            T_el vec_len = this->vec_length (datum);
            for (std::size_t i = 0; i < datum.size(); ++i) {
                S_el el_len = static_cast<S_el>(datum[i]);
                // Here's the scaling of a component
                rtn[i] = (el_len - (el_len/static_cast<S_el>(vec_len))*this->params[1]) * this->params[0];
            }
            return rtn;
        }

        //! For clarity, here's a description of the transform function
        std::string transform_str() const
        {
            std::stringstream ss;
            if (this->type == scaling_function::Logarithmic) {
                ss << "log scaling of vectors is unimplemented";
            } else if (this->type == scaling_function::Linear && this->params.size() > 1) {
                ss << "(elementwise) y[i] = (x[i] - (x[i]/|x|) * " << this->params[1] << ") * " << this->params[0];
            } else {
                ss << "unknown scaling type";
            }
            return ss.str();
        }

        std::string output_range_str() const
        {
            std::stringstream ss;
            ss << this->output_range;
            return ss.str();
        }

        T inverse_one (const S& datum) const
        {
            T rtn = T{};
            if (this->type == scaling_function::Logarithmic) {
                rtn = this->inverse_one_log (datum);
            } else if (this->type == scaling_function::Linear) {
                rtn = this->inverse_one_linear (datum);
            } else {
                throw std::runtime_error ("scale_impl<0=vector>::inverse_one(): Unknown scaling");
            }
            return rtn;
        }

        // deprecated name. use compute_scaling()
        void compute_autoscale (T input_min, T input_max)
        {
            std::cerr << "Note: The function scale::compute_autoscale has been renamed to compute_scaling. Please update your code.\n";
            this->compute_scaling (input_min, input_max);
        }

        void compute_scaling (const morph::range<T>& input_range) { this->compute_scaling (input_range.min, input_range.max); }
        void compute_scaling (const T input_min, const T input_max)
        {
            if (this->type != scaling_function::Linear) {
                throw std::runtime_error ("scale_impl<0=vector>::compute_scaling)(): This scaling function is for Linear scaling only");
            }
            this->params.resize (2, T_el{0});
            // Vector version: get lengths of input_min/max
            T_el imax_len = vec_length (input_max);
            T_el imin_len = vec_length (input_min);
            // Handle imax_len == imin_len
            if (imax_len == imin_len) {
                // params[0] is already 0
                this->params[1] = (this->output_range.max - this->output_range.min)/S_el{2};
            } else {
                // m = rise/run
                this->params[0] = (this->output_range.max - this->output_range.min) / static_cast<S_el>(imax_len - imin_len);
                // c = y - mx => min = m * input_min + c => c = min - (m * input_min)
                this->params[1] = static_cast<S_el>(imin_len); // this->output_range.min - (this->params[0] * imin_len);
            }
        }

        virtual void identity_scaling()
        {
            this->do_autoscale = false;
            // Make 1D vectors of min and max lengths
            T minvec = {this->output_range.min};
            T maxvec = {this->output_range.max};
            // Compute a scaling with these vectors
            this->compute_scaling (minvec, maxvec);
        }

        virtual void null_scaling()
        {
            this->do_autoscale = false;
            this->setParams (S_el{0}, S_el{0});
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
        S_el getParams (unsigned int idx) { return this->params[idx]; }

        //! Get all the params
        morph::vvec<S_el> getParams() { return this->params; }

        //! return string representation of params
        std::string params_str() const { return this->params.str(); }

        //! The scale object is ready if params has size 2.
        bool ready() const { return (this->params.size() > 1); }

        //! Reset the Scaling by emptying params
        void reset() { this->params.clear(); }

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

        //! The inverse linear transform; x = (y-c)/m
        T inverse_one_linear (const T& datum) const
        {
            if (this->params.size() < 2) { throw std::runtime_error ("scale_impl<0=vector>::inverse_one_linear(): scaling params not set"); }
            T rtn (datum);
            std::size_t i = 0;
            for (auto el : datum) {
                rtn[i++] = el - this->params[1] / this->params[0];
            }
            return rtn;
        }

        //! The inverse of the log transform is exp.
        T inverse_one_log (const T& datum) const
        {
            T rtn = inverse_one_linear (datum);
            for (auto& el : rtn) { el = std::exp (el); }
            return rtn;
        }

        //! The parameters for the scaling. For linear scaling, this will contain two scalar
        //! values. Note the type is the output element type
        morph::vvec<S_el> params;
    };

    /*!
     * \brief scale_impl for scalar \a T
     *
     * A specialized implementation base class for the template class scale, which is used when the
     * number type of \a T is a scalar such as int, double, float or long double.
     *
     * \tparam ntype The 'number type' as contained in number_type::value. 0 for vectors, 1 for
     * scalars. This class is active only for ntype==1 (scalar).
     *
     * \tparam T The type of the number to be scaled. Should be some scalar type such as int,
     * double. It is from the type \a T that ntype is determined.
     *
     * \tparam S The output type for the scaled number. For integer T, this might well be a floating
     * point type.
     *
     * \sa scale_impl_base
     */
    template<typename T, typename S>
    class scale_impl<1, T, S> : public scale_impl_base<T, S>
    {
    public:
        //! The output range required. Change if you want to scale to something other than [0, 1]
        morph::range<S> output_range = morph::range<S>(S{0}, S{1});

        S transform_one (const T& datum) const
        {
            S rtn = S{0};
            if (this->type == scaling_function::Logarithmic) {
                rtn = this->transform_one_log (datum);
            } else if (this->type == scaling_function::Linear) {
                rtn = this->transform_one_linear (datum);
            } else {
                throw std::runtime_error ("scale_impl<1=scalar>::transform_one(): Unknown scaling");
            }
            return rtn;
        }

        //! A description of the transform function
        std::string transform_str() const
        {
            std::stringstream ss;
            if (this->type == scaling_function::Logarithmic && this->params.size() > 1) {
                ss << "y = " << this->params[0] << " * log(x) + " << this->params[1];
            } else if (this->type == scaling_function::Linear && this->params.size() > 1) {
                ss << "y = " << this->params[0] << " * x + " << this->params[1];
            } else {
                ss << "unknown scaling type";
            }
            return ss.str();
        }

        std::string output_range_str() const
        {
            std::stringstream ss;
            ss << this->output_range;
            return ss.str();
        }

        T inverse_one (const S& datum) const
        {
            T rtn = T{0};
            if (this->type == scaling_function::Logarithmic) {
                rtn = this->inverse_one_log (datum);
            } else if (this->type == scaling_function::Linear) {
                rtn = this->inverse_one_linear (datum);
            } else {
                throw std::runtime_error ("scale_impl<1=scalar>::inverse_one(): Unknown scaling");
            }
            return rtn;
        }

        // deprecated name
        void compute_autoscale (T input_min, T input_max)
        {
            std::cerr << "Note: The function scale::compute_autoscale has been renamed to compute_scaling. Please update your code.\n";
            this->compute_scaling (input_min, input_max);
        }

        void compute_scaling (const morph::range<T>& input_range) { this->compute_scaling (input_range.min, input_range.max); }
        void compute_scaling (const T input_min, const T input_max)
        {
            if (this->type == scaling_function::Logarithmic) {
                this->compute_scaling_log (input_min, input_max);
            } else if (this->type == scaling_function::Linear) {
                this->compute_scaling_linear (input_min, input_max);
            } else {
                throw std::runtime_error ("scale_impl<1=scalar>::compute_scaling(): Unknown scaling");
            }
        }

        virtual void identity_scaling()
        {
            this->do_autoscale = false;
            this->compute_scaling (this->output_range.min, this->output_range.max);
        }

        virtual void null_scaling()
        {
            this->do_autoscale = false;
            this->setParams (S{0}, S{0});
        }

        //! Set params for a two parameter scaling
        //! \param p0 The zeroth parameter
        //! \param p1 The first parameter
        void setParams (S p0, S p1)
        {
            this->do_autoscale = false;
            this->params.resize (2, S{0});
            this->params[0] = p0;
            this->params[1] = p1;
        }

        //! Getter for params
        //! \param idx The index into #params
        //! \return The specified element of #params
        S getParams (unsigned int idx) { return this->params[idx]; }

        //! Get all the params
        morph::vvec<S> getParams() { return this->params; }

        //! return string representation of params
        std::string params_str() const { return this->params.str(); }

        //! The scale object is ready if params has size 2.
        bool ready() const { return (this->params.size() > 1); }

        //! Reset the Scaling by emptying params
        void reset() { this->params.clear(); }

    protected:
        //! Linear transform for scalar type; y = mx + c
        S transform_one_linear (const T& datum) const
        {
            if (this->params.size() < 2) { throw std::runtime_error ("scale_impl<1=scalar>::transform_one_linear(): (scalar) scaling params not set"); }
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
            if (this->params.size() < 2) { throw std::runtime_error ("scale_impl<1=scalar>::inverse_one_linear(): (scalar) scaling params not set"); }
            return ((datum-this->params[1])/this->params[0]);
        }

        //! The inverse of the log transform is exp.
        T inverse_one_log (const S& datum) const
        {
            T res = inverse_one_linear(datum);
            return (std::exp (res));
        }

        virtual void compute_scaling_linear (T input_min, T input_max)
        {
            // Here, we need to use the output type for the computations. Does that mean
            // params is stored in the output type? I think it does.
            this->params.resize (2, S{0});
            if (input_min == input_max) {
                this->params[0] = T{0};
                this->params[1] = (this->output_range.max - this->output_range.min) / S{2};
            } else {
                // m = rise/run
                this->params[0] = (this->output_range.max - this->output_range.min) / static_cast<S>(input_max - input_min);
                // c = y - mx => min = m * input_min + c => c = min - (m * input_min)
                this->params[1] = this->output_range.min - (this->params[0] * static_cast<S>(input_min));
            }
        }

        void compute_scaling_log (T input_min, T input_max)
        {
            // Have to check here as scale_impl<2, T, S> is built from scale_impl<1, T, S> but <= operator makes no sense
            if constexpr (morph::number_type<T>::value == 1) {
                if (input_min <= T{0} || input_max <= T{0}) {
                    throw std::runtime_error ("scale_impl<1=scalar>::compute_scaling_log(): Can't logarithmically autoscale a range which includes zeros or negatives");
                }
            }
            T ln_imin = std::log(input_min);
            T ln_imax = std::log(input_max);
            // Now just scale linearly between ln_imin and ln_imax
            this->compute_scaling_linear (ln_imin, ln_imax);
        }

        //! The parameters for the scaling. If linear, this will contain two scalar values.
        morph::vvec<S> params;
    };

    /*!
     * \brief Experimental scale_impl for complex scalars\a T
     *
     * A specialized implementation base class for the template class scale, which is used when the
     * number type of \a T is std::complex<>
     *
     * \tparam ntype The 'number type' as contained in number_type::value. 0 for vectors, 1 for
     * scalars. This class is active only for ntype==2 (complex scalar).
     *
     * \tparam T The type of the number to be scaled. Should be some complex scalar type such as
     * std::complex<float> or std::complex<double>.
     *
     * \tparam S The output type for the scaled number. For integer T, this might well be a floating
     * point type. Does it make sense to scale from complex to non complex? Maybe, if you're scaling
     * to magnitude.
     *
     * \sa scale_impl_base
     */
    template<typename T, typename S>
    class scale_impl<2, T, S> : public scale_impl<1, T, S>
    {
    public:
        // Any public overrides go here
    protected:
        // Scaling a set of complex numbers means stretching/squashing the plane out so that the
        // complex values have magnitudes between output_min and output_max. This is a little
        // different from scaling of regular scalars.
        void compute_scaling_linear (T input_min, T input_max)
        {
            // We enforce output_range.min == {0, 0} for complex number scaling. We simply shrink or
            // stretch the complex plane so that the input numbers with the largest magnitude are
            // given the magniude of output_range.max.
            if (this->output_range.min != T{0}) {
                throw std::runtime_error ("scale_impl<2, T, S>::compute_scaling_linear: "
                                          "output_range minimum must be (0 + 0i) for complex scaling");
            }

            // Here, we need to use the output type for the computations. That means params is
            // stored in the output type, which is (should be) complex. Only the magnitude of each
            // param matters.
            //
            // Note that we ONLY scale values on the complex plane by multiplication. The origin
            // remains at the origin. So we leave params[1] equal to S{0}
            this->params.resize (2, S{0});

            if (input_min != input_max) {
                // m = rise/run but note that we ignore input_min
                this->params[0] = (std::abs(this->output_range.max) - std::abs(this->output_range.min)) / static_cast<S>(std::abs(input_max) /*- std::abs(input_min)*/);
            }
        }

        void compute_scaling_log (T input_min, T input_max)
        {
            // Log scaling complex range?
            if (std::abs(input_min) == T{0} || std::abs(input_max) == T{0}) {
                throw std::runtime_error ("scale_impl<2, T, S>::compute_scaling_log(): Can't logarithmically autoscale a complex range which includes zeros");
            }
            T ln_imin = std::log(input_min);
            T ln_imax = std::log(input_max);
            // Now just scale linearly between ln_imin and ln_imax
            this->compute_scaling_linear (ln_imin, ln_imax);
        }
    };

    // Scaling a vector of complex numbers is unsupported
    template<typename T, typename S>
    class scale_impl<3, T, S> : public scale_impl_base<T, S> {
        void compute_scaling_linear ([[maybe_unused]] T input_min, [[maybe_unused]] T input_max) {
            []<bool flag = false>() { static_assert(flag, "morph::scale does not support scaling vectors of complex numbers"); }();
        }
    };

    // Scaling is unsupported if the input number_type is not a number type.
    template<typename T, typename S>
    class scale_impl<-1, T, S> : public scale_impl_base<T, S> {
        void compute_scaling_linear ([[maybe_unused]] T input_min, [[maybe_unused]] T input_max) {
            []<bool flag = false>() { static_assert(flag, "morph::scale does not support scaling non-number types"); }();
        }
    };

    /*!
     * A class for scaling and normalizing signals.
     *
     * Mostly used for linear scaling of signals, has an autoscale feature. Could also be used to
     * logarithmically scale a signal with suitable extension.
     *
     * morph::scale derives from scale_impl<N> depending on the type of T
     *
     * Usage: See tests/testScale.cpp
     * e.g.:
     *   morph::scale<float> s;
     *   s.do_autoscale = true;
     *   std::vector<float> vf = { 1, 2, 3 };
     *   std::vector<float> result(vf);
     *   s.transform (vf, result);
     *
     * \tparam T Input number type. The type of the numbers (or vectors) that will be scaled.
     *
     * \tparam S The type of the output numbers (or for vectors, their elements). Defaults to be the
     * same type as T, but when scaling integers, may well be a different type such as float or
     * double
     */
    template <typename T, typename S=T>
    struct scale : public scale_impl<morph::number_type<T>::value, T, S> {};

} // namespace morph
