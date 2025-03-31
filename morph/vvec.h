/*!
 * \file
 * \brief An N dimensional vector class template, morph::vvec which derives from std::vector.
 *
 * \author Seb James
 * \date May 2020
 */
#pragma once

#include <cmath>
#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <numeric>
#include <limits>
#include <iomanip>
#include <algorithm>
#include <functional>
#include <cstddef>
#include <morph/Random.h>
#include <morph/range.h>
#include <morph/trait_tests.h>

namespace morph {

    /*!
     * \brief N-D vector class deriving from std::vector
     *
     * An N dimensional vector class template which derives from std::vector. Its
     * components are of scalar type S. It is anticipated that S will be set either to
     * floating point scalar types such as float or double, or to integer scalar types
     * such as int, long long int and so on. Thus, a typical (and in fact, the default)
     * signature would be:
     *
     *\code{.cpp}
     * vvec<float> v;
     *\endcode
     *
     * The class inherits std::vector's dynamically-resizeable memory for storing the
     * components of the vector and std::vector's constructors.. It adds numerous
     * methods which allow objects of type vvec to have arithmetic operations applied
     * to them, either scalar (add a scalar to all elements; divide all elements by a
     * scalar, etc) or vector (including dot and cross products, normalization and so
     * on.
     *
     * This class is better for writing neural networks than morph::vec, whose size
     * has to be set at compile time.
     */
    template <typename S, typename Al> struct vvec;

    /*!
     * Template friendly mechanism to overload the stream operator.
     *
     * Note forward declaration of the vvec template class and this template for
     * stream operator overloading. Example adapted from
     * https://stackoverflow.com/questions/4660123
     */
    template <typename S, typename Al> std::ostream& operator<< (std::ostream&, const vvec<S, Al>&);

    template <typename S=float, typename Al=std::allocator<S>>
    struct vvec : public std::vector<S, Al>
    {
        //! We inherit std::vector's constructors like this:
        using std::vector<S, Al>::vector;

        //! Used in functions for which wrapping is important
        enum class wrapdata { none, wrap };
        //! Should a function resize the output?
        enum class resize_output { no, yes };
        //! Should a function treat a kernel as symmetric and centralize it?
        enum class centre_kernel { no, yes };

        //! \return the first component of the vector
        S x() const noexcept { return (*this)[0]; }
        //! \return the second component of the vector
        S y() const noexcept { return (*this)[1]; }
        //! \return the third component of the vector
        S z() const noexcept { return (*this)[2]; }
        //! \return the fourth component of the vector
        S w() const noexcept { return (*this)[3]; }

        /*!
         * An array access operator that accepts negative indices, using these to access from the
         * end of the array. This allows you to define an array of size 2n+1 and access elements
         * from .at_idx(-n) through to .at_idx(n), as is possible in some other languages.
         */
        template<typename I, typename std::enable_if< std::is_integral<I>{} && std::is_signed<I>{}, bool>::type = true >
        S& at_signed (const I idx)
        {
            if (idx > I{-1}) {
                // Note that this will allow positive indexing all the way up to the end of the
                // vector, returning elements that could also be returned by a negative idx.
                return this->at (idx);
            } else {
                // Check bounds of idx and throw std::out_of_range if necessary. Allow negative
                // indices all the way to the start of the vector.
                if (-idx > static_cast<I>(this->size())) { throw std::out_of_range ("vvec::at_signed: idx is too negative"); }
                return *(this->end() + idx);
            }
        }

        /*!
         * A const array access operator that accepts negative indices. The const version of
         * vvec::at_signed
         */
        template<typename I, typename std::enable_if< std::is_integral<I>{} && std::is_signed<I>{}, bool>::type = true >
        const S& c_at_signed (const I idx) const
        {
            if (idx > I{-1}) {
                return this->at (idx);
            } else {
                if (-idx > static_cast<I>(this->size())) { throw std::out_of_range ("vvec::c_at_signed: idx is too negative"); }
                return *(this->cend() + idx);
            }
        }

        /*!
         * Traits-based set_from that can work with sequential containers like std::array, deque, vector,
         * morph::vvec, morph::vec etc and even with std::set (though not with std::map).
         *
         * This uses a traits solution inspired by
         * https://stackoverflow.com/questions/7728478/c-template-class-function-with-arbitrary-container-type-how-to-define-it
         */
        template <typename C>
        std::enable_if_t<morph::is_copyable_container<C>::value && !std::is_same<std::decay_t<C>, S>::value, void>
        set_from (const C& c)
        {
            this->resize (c.size());
            std::copy (c.begin(), c.end(), this->begin());
        }

        //! What if we want to set all elements to something of type S, but S is itself a copyable
        //! container. In that case, enable this function.
        template <typename C>
        std::enable_if_t<morph::is_copyable_container<C>::value && std::is_same<std::decay_t<C>, S>::value, void>
        set_from (const C& v) noexcept { std::fill (this->begin(), this->end(), v); }

        //! Set all elements from the value type v.
        template <typename Sy=S>
        std::enable_if_t<!morph::is_copyable_container<Sy>::value, void>
        set_from (const Sy& v) noexcept { std::fill (this->begin(), this->end(), v); }

        /*!
         * Set the data members of this vvec from the passed in, larger dynamically resizable
         * vector, \a v, ignoring the last element of \a v. Used when working with 4D vectors in
         * graphics applications involving 4x4 transform matrices.
         *
         * \tparam C stands for Container and might be std::vector<T> or vvec<T> or std::array<T, N>
         * or morph::vec<T, N>
         */
        template <typename C>
        std::enable_if_t<morph::is_copyable_container<C>::value, void>
        set_from_onelonger (const C& v) noexcept
        {
            if (v.size() == (this->size() + 1)) {
                for (std::size_t i = 0; i < this->size(); ++i) {
                    (*this)[i] = v[i];
                }
            } // else do nothing
        }

        //! \return a vector with one less dimension - losing the last one.
        vvec<S> less_one_dim () const
        {
            std::size_t N = this->size();
            vvec<S> rtn(N-1);
            for (std::size_t i = 0; i < N-1; ++i) { rtn[i] = (*this)[i]; }
            return rtn;
        }

        //! \return a vector with one additional dimension - setting it to 0.
        vvec<S> plus_one_dim () const
        {
            std::size_t N = this->size();
            vvec<S> rtn(N+1);
            for (std::size_t i = 0; i < N; ++i) { rtn[i] = (*this)[i]; }
            rtn[N] = S{0};
            return rtn;
        }

        //! Return a vector with one additional dimension - setting it to val.
        vvec<S> plus_one_dim (const S val) const
        {
            std::size_t N = this->size();
            vvec<S> rtn(N+1);
            for (std::size_t i = 0; i < N; ++i) { rtn[i] = (*this)[i]; }
            rtn[N] = val;
            return rtn;
        }

        //! \return this vvec as a vvec<T>
        template<typename T>
        vvec<T> as() const
        {
            vvec<T> vv(this->size(), T{0});
            vv += *this;
            return vv;
        }

        //! \return this vvec in single precision, float format
        vvec<float> as_float() const { return this->as<float>(); }

        //! \return this vvec in double precision format
        vvec<double> as_double() const { return this->as<double>(); }

        //! \return this vvec in single precision, int format
        vvec<int> as_int() const { return this->as<int>(); }

        //! \return this vvec in single precision, unsigned int format
        vvec<unsigned int> as_uint() const { return this->as<unsigned int>(); }

        //! \return the first and last elements in the vvec as a two element vector. If *this is
        //! empty, return a 2 element vvec containing zeros.
        vvec<S> firstlast() const noexcept
        {
            vvec<S> rtn = { S{0}, S{0} }; // if we can't allocate 2 S sized things we are going to crash!
            if (!this->empty()) { rtn = { (*this)[0], (*this)[this->size() - 1] }; }
            return rtn;
        }

        /*!
         * Set a linear sequence into the vector from value start to value stop. If
         * num>0 then resize the vector first, otherwise use the vvec's current
         * size. You *can* use this with integer types, but be prepared to notice odd
         * rounding errors.
         */
        template <typename Sy=S, typename Sy2=S>
        void linspace (const Sy start, const Sy2 stop, const std::size_t num = 0u)
        {
            if (num > 0u) { this->resize (num); }
            S increment = (this->size() == 1u) ? S{0} : (static_cast<S>(stop) - static_cast<S>(start)) / (this->size() - 1u);
            for (std::size_t i = 0u; i < this->size(); ++i) { (*this)[i] = start + increment * i; }
        }

        /*!
         * Similar to numpy's arange. Set a linear sequence from start to stop with the given step
         * size. Again, odd results may be obtained with integer types for S.
         */
        template <typename Sy=S, typename Sy2=S>
        void arange (const Sy start, const Sy2 stop, const Sy2 increment)
        {
            this->clear();
            this->resize(0);
            // Figure out how many elements given the increment:
            S num = std::ceil((stop - start) / increment);
            if (num > S{0}) {
                this->resize (static_cast<std::size_t>(num));
                for (std::size_t i = 0; i < static_cast<std::size_t>(num); ++i) {
                    (*this)[i] = start + increment*static_cast<S>(i);
                }
            } // else vector should now be empty, just like Python does it
        }

        /*!
         * Stream the elements of the vector into \a ss as a comma separated list.
         *
         * num_in_line: How many elements to output before inserting a newline
         */
        void str_comma_separated (std::stringstream& ss,
                                  const unsigned int num_in_line=std::numeric_limits<unsigned int>::max(),
                                  const char sep = ',') const
        {
            if (this->empty()) { return; }
            ss << std::setprecision (std::numeric_limits<S>::max_digits10);
            bool first = true;
            unsigned int elementcount = 0;
            for (auto i : *this) {
                if (first) {
                    // Test for char/unsigned char and cast to int to avoid being interpreted as ascii
                    if constexpr (std::is_same<S, unsigned char>::value == true || std::is_same<S, char>::value == true) {
                        ss << static_cast<int>(i);
                    } else { ss << i; }
                    first = false;
                    if (elementcount == num_in_line - 1) { ss << "\n"; }
                } else {
                    ss << sep << (elementcount%num_in_line==0 ? "\n" : "");
                    if constexpr (std::is_same<S, unsigned char>::value == true || std::is_same<S, char>::value == true) {
                        ss << static_cast<int>(i);
                    } else { ss << i; }
                }
                ++elementcount;
            }
        }

        /*!
         * Create a string representation of the vector
         *
         * num_in_line: How many elements to output before inserting a newline
         *
         * \return A 'coordinate format' string such as "(1,1,2)", "(0.2,0.4)" or
         * "(5,4,5,5,40)".
         */
        std::string str (const unsigned int num_in_line=std::numeric_limits<unsigned int>::max()) const
        {
            std::stringstream ss;
            ss << "(";
            this->str_comma_separated (ss, num_in_line);
            ss << ")";
            return ss.str();
        }

        //! Output the vector in a form suitable to paste into MATLAB or Octave
        std::string str_mat (const unsigned int num_in_line=std::numeric_limits<unsigned int>::max()) const
        {
            std::stringstream ss;
            ss << "[";
            this->str_comma_separated (ss, num_in_line);
            ss << "]";
            return ss.str();
        }

        /*!
         * Output the vector in a form suitable to paste into Python, as a numpy vector,
         * assuming you imported numpy as np
         */
        std::string str_numpy(const unsigned int num_in_line=std::numeric_limits<unsigned int>::max()) const
        {
            std::stringstream ss;
            ss << "np.array((";
            this->str_comma_separated (ss, num_in_line);
            ss << "))";
            return ss.str();
        }

        //! Output in a form that can be used as an initializer list in C++
        std::string str_initializer(const unsigned int num_in_line=std::numeric_limits<unsigned int>::max()) const
        {
            std::stringstream ss;
            ss << "{";
            this->str_comma_separated (ss, num_in_line);
            ss << "}";
            return ss.str();
        }

        //! Renormalize the vector to length 1.0. Only for S types that are floating point.
        template <typename Sy=S, std::enable_if_t<!std::is_integral<std::decay_t<Sy>>::value, int> = 0 >
        void renormalize() noexcept
        {
            auto add_squared = [](Sy a, Sy b) { return a + b * b; };
            const Sy denom = std::sqrt (std::accumulate (this->begin(), this->end(), Sy{0}, add_squared));
            if (denom != Sy{0}) {
                Sy oneovermag = Sy{1} / denom;
                auto x_oneovermag = [oneovermag](Sy f) { return f * oneovermag; };
                std::transform (this->begin(), this->end(), this->begin(), x_oneovermag);
            }
        }

        //! Rescale the vector elements so that they all lie in the range 0-1. NOT the same as renormalize.
        template <typename Sy=S, std::enable_if_t<!std::is_integral<std::decay_t<Sy>>::value, int> = 0 >
        void rescale() noexcept
        {
            morph::range<Sy> r = this->minmax();
            Sy m = r.max - r.min;
            Sy g = r.min;
            auto rescale_op = [m, g](Sy f) { return (f - g)/m; };
            std::transform (this->begin(), this->end(), this->begin(), rescale_op);
        }

        //! Rescale the vector elements so that they all lie in the range -1 to 0.
        template <typename Sy=S, std::enable_if_t<!std::is_integral<std::decay_t<Sy>>::value, int> = 0 >
        void rescale_neg() noexcept
        {
            morph::range<Sy> r = this->minmax();
            Sy m = r.max - r.min;
            Sy g = r.max;
            auto rescale_op = [m, g](Sy f) { return (f - g)/m; };
            std::transform (this->begin(), this->end(), this->begin(), rescale_op);
        }

        //! Rescale the vector elements symetrically about 0 so that they all lie in the range -1 to 1.
        template <typename Sy=S, std::enable_if_t<!std::is_integral<std::decay_t<Sy>>::value, int> = 0 >
        void rescale_sym() noexcept
        {
            morph::range<Sy> r = this->minmax();
            Sy m = (r.max - r.min) / Sy{2};
            Sy g = (r.max + r.min) / Sy{2};
            auto rescale_op = [m, g](Sy f) { return (f - g)/m; };
            std::transform (this->begin(), this->end(), this->begin(), rescale_op);
        }

        //! Zero the vector. Set all elements to 0
        void zero() noexcept { std::fill (this->begin(), this->end(), S{0}); }
        //! Set all elements of the vector to the maximum possible value given type S
        void set_max() noexcept { std::fill (this->begin(), this->end(), std::numeric_limits<S>::max()); }
        //! Set all elements of the vector to the lowest (i.e. most negative) possible value given type S
        void set_lowest() noexcept { std::fill (this->begin(), this->end(), std::numeric_limits<S>::lowest()); }

        /*!
         * Randomize the vector
         *
         * Randomly set the elements of the vector. Elements are set to random
         * numbers drawn from a uniform distribution between 0 and 1 if S is a
         * floating point type or to integers between std::numeric_limits<S>::min()
         * and std::numeric_limits<S>::max() if S is an integral type (See
         * morph::RandUniform for details).
         */
        void randomize()
        {
            RandUniform<S> ru;
            for (auto& i : *this) { i = ru.get(); }
        }

        /*!
         * Randomize the vector with provided bounds
         *
         * Randomly set the elements of the vector. Elements are set to random
         * numbers drawn from a uniform distribution between \a min and \a
         * max. Strictly, the range is [min, max) (including min, not including max)
         */
        void randomize (S min, S max)
        {
            RandUniform<S> ru (min, max);
            for (auto& i : *this) { i = ru.get(); }
        }

        /*!
         * Randomize the vector from a Gaussian distribution
         *
         * Randomly set the elements of the vector. Elements are set to random
         * numbers drawn from a uniform distribution between \a min and \a
         * max. Strictly, the range is [min, max)
         */
        void randomizeN (S _mean, S _sd)
        {
            RandNormal<S> rn (_mean, _sd);
            for (auto& i : *this) { i = rn.get(); }
        }

        /*!
         * Re-order the elements in the vvec - shuffle it up. Don't duplicate any
         * entries, so that summary statistics such as mean() and variance() should
         * return the same value on the returned, jumbled vvec. This just randomizes the
         * order of the elements. Uses std::shuffle with a selected RNG.
         */
        void shuffle()
        {
            std::random_device rd; // we have access to these via #include <morph/Random.h>
            std::mt19937 generator(rd());
            std::shuffle (this->begin(), this->end(), generator);
        }

        /*!
         * As shuffle() but return the shuffled vvec
         */
        morph::vvec<S> shuffled()
        {
            morph::vvec<S> rtn (*this);
            std::random_device rd; // we have access to these via #include <morph/Random.h>
            std::mt19937 generator(rd());
            std::shuffle (rtn.begin(), rtn.end(), generator);
            return rtn;
        }

        /*!
         * Permute the elements one time in a rotation. This 'rotates left', i.e. in an
         * N element vvec: 0->N-1, 1->0, 2->1, etc. Useful for swapping x and y in a 2D
         * vector.
         */
        void rotate() noexcept
        {
            if (this->size() > 1) {
                S z_el = (*this)[0];
                for (std::size_t i = 1; i < this->size(); ++i) {
                    (*this)[i-1] = (*this)[i];
                }
                (*this)[this->size()-1] = z_el;
            } // else no op
        }

        /*!
         * Templated rotate for integral types T. Rotates 'n steps to the left' so, if
         * n=1, and the vvec length is N, then element 0->N-1, 1->0, 2->1, etc. To rotate to
         * the right, you can use -n.
         */
        template <typename T=int>
        void rotate (T n)
        {
            static_assert (std::numeric_limits<T>::is_integer);

            n %= static_cast<T>(this->size());

            auto _start = this->begin();
            if constexpr (std::numeric_limits<T>::is_signed) {
                std::size_t _n = n >= 0 ? n : this->size() + n;
                std::advance (_start, _n);
            } else {
                std::advance (_start, n);
            }
            std::rotate (this->begin(), _start, this->end());
        }

        //! If size is even, permute pairs of elements in a rotation. 0->1, 1->0, 2->3, 3->2, etc.
        void rotate_pairs()
        {
            std::size_t N = this->size();
            if (N%2!=0) {
                throw std::runtime_error ("vvec size must be even to call morph::vvec::rotate_pairs");
            }
            S tmp_el = S{0};
            for (std::size_t i = 0; i < N; i+=2) {
                tmp_el = (*this)[i];
                (*this)[i] = (*this)[i+1];
                (*this)[i+1] = tmp_el;
            }
        }

        /*!
         * constexpr function to return a type-suitable value for the 'unit threshold'. A perfect
         * unit vector has length==1. abs(1 - length(any vector)) gives an error value. If this
         * error value is smaller than the unit threshold, we call the vector a unit vector to
         * within the tolerances that we can compute.
         */
        static constexpr S unitThresh() noexcept
        {
            // Note: std::float16_t comes with C++23
            if constexpr (std::is_same<S, float>::value) {
                return S{1e-6};
            } else if constexpr (std::is_same<S, double>::value) {
                return S{1e-14};
            } else {
                return S{0};
            }
        }

        /*!
         * Test to see if this vector is a unit vector (it doesn't *have* to be).
         *
         * \return true if the length of the vector is 1.
         */
        bool checkunit() const noexcept
        {
            auto subtract_squared = [](S a, S b) { return a - b * b; };
            const S metric = std::accumulate (this->begin(), this->end(), S{1}, subtract_squared);
            if (std::abs(metric) > morph::vvec<S>::unitThresh()) {
                return false;
            }
            return true;
        }

        /*!
         * Find the length of the vector.
         *
         * \return the length
         */
        template <typename Sy=S>
        Sy length() const noexcept
        {
            auto add_squared = [](Sy a, S b) { return a + b * b; };
            // Add check on whether return type Sy is integral or float. If integral, then std::round then cast the result of std::sqrt()
            if constexpr (std::is_integral<std::decay_t<Sy>>::value == true) {
                return static_cast<Sy>(std::round(std::sqrt(std::accumulate(this->begin(), this->end(), Sy{0}, add_squared))));
            } else {
                return std::sqrt(std::accumulate(this->begin(), this->end(), Sy{0}, add_squared));
            }
        }

        /*!
         * Reduce the length of the vector by the amount dl, if possible. If dl makes the vector
         * have a negative length, then return a zeroed vector. Enable only for real valued vectors.
         */
        template <typename Sy=S, std::enable_if_t<!std::is_integral<std::decay_t<Sy>>::value, int> = 0 >
        vvec<S> shorten (const S dl) const
        {
            vvec<S> v = *this;
            S newlen = this->length() - dl;
            if (newlen <= S{0}) {
                v.zero();
            } else {
                v *= newlen/this->length();
            }
            return v;
        }

        /*!
         * Opposite of vvec::shorten. Increase the length of the vector by the amount dl, if
         * possible. If dl is negative, and makes the vector have a negative length, then return a
         * zeroed vector. Enable only for real valued vectors.
         */
        template <typename Sy=S, std::enable_if_t<!std::is_integral<std::decay_t<Sy>>::value, int> = 0 >
        vvec<S> lengthen (const S dl) const
        {
            vvec<S> v = *this;
            S newlen = this->length() + dl;
            if (newlen <= S{0}) { // dl could be negative
                v.zero();
            } else {
                v *= newlen/this->length();
            }
            return v;
        }

        /*!
         * Find the squared length of the vector which is the same as the sum of squared
         * elements, if elements are scalar.
         *
         * If S typed elements are morph::vecs or morph::vvecs, then return the sum of the squares
         * of the lengths of the elements in the return Sy type, which you will have to ensure to be
         * a scalar.
         */
        template <typename Sy=S>
        Sy length_sq() const noexcept
        {
            Sy _sos = Sy{0};
            if constexpr (std::is_scalar<std::decay_t<S>>::value) {
                if constexpr (std::is_scalar<std::decay_t<Sy>>::value) {
                    // Return type is also scalar
                    _sos = this->sos<Sy>();
                } else {
                    // Return type is a vector. Too weird.
                    // C++-20:
                    []<bool flag = false>() { static_assert(flag, "Won't compute sum of squared scalar elements into a vector type"); }();
                }
            } else {
                // S is a vector so i is a vector.
                if constexpr (std::is_scalar<std::decay_t<Sy>>::value) {
                    // Return type Sy is a scalar
                    for (auto& i : *this) { _sos += i.template sos<Sy>(); }
                } else {
                    // Return type Sy is also a vector, place result in 0th element? No, can now use vvec<vec<float>>::sos<float>()
                    []<bool flag = false>() { static_assert(flag, "Won't compute sum of squared vector length elements into a vector type"); }();
                }
            }
            return _sos;
        }

        /*!
         * \return the sum of the squares of the elements.
         *
         * If S is a vector type, then the result will be a vector type containing the
         * sos of all elements i - that is, element 0 of the returned vector will contain
         *  the sum of squares of element 0 of all members of *this.
         *
         * \return the length squared
         */
        template <bool test_for_nans = false, typename Sy=S>
        Sy sos() const noexcept
        {
            if constexpr (test_for_nans) {
                auto add_squared = [](Sy a, S b) { return std::isnan(b) ? a : a + b * b; };
                return std::accumulate (this->begin(), this->end(), Sy{0}, add_squared);
            } else {
                auto add_squared = [](Sy a, S b) { return a + b * b; };
                return std::accumulate (this->begin(), this->end(), Sy{0}, add_squared);
            }
        }

        //! \return the value of the longest component of the vector.
        template <typename Sy=S, std::enable_if_t<std::is_scalar<std::decay_t<Sy>>::value, int> = 0 >
        S longest() const noexcept
        {
            auto abs_compare = [](S a, S b) { return (std::abs(a) < std::abs(b)); };
            auto thelongest = std::max_element (this->begin(), this->end(), abs_compare);
            S rtn = *thelongest;
            return rtn;
        }

        // For a vvec of vecs, longest() should return the same as max()
        template <typename Sy=S, std::enable_if_t<morph::is_copyable_fixedsize<std::decay_t<Sy>>::value, int> = 0 >
        S longest() const noexcept { return this->max(); }

        //! \return the index of the longest component of the vector.
        template <typename Sy=S, std::enable_if_t<std::is_scalar<std::decay_t<Sy>>::value, int> = 0 >
        std::size_t arglongest() const noexcept
        {
            std::size_t idx = 0;
            if constexpr (std::is_scalar<std::decay_t<S>>::value) {
                auto abs_compare = [](S a, S b) { return (std::abs(a) < std::abs(b)); };
                auto thelongest = std::max_element (this->begin(), this->end(), abs_compare);
                idx = (thelongest - this->begin());
            } else {
                auto vec_compare = [](S a, S b) { return (a.length() < b.length()); };
                auto thelongest = std::max_element (this->begin(), this->end(), vec_compare);
                idx = (thelongest - this->begin());
            }
            return idx;
        }

        // For a vvec of vecs, arglongest() should return then same as argmax()
        template <typename Sy=S, std::enable_if_t<morph::is_copyable_fixedsize<std::decay_t<Sy>>::value, int> = 0 >
        std::size_t arglongest() const noexcept { return this->argmax(); }

        //! \return the value of the shortest component of the vector.
        template <typename Sy=S, std::enable_if_t<std::is_scalar<std::decay_t<Sy>>::value, int> = 0 >
        S shortest() const noexcept
        {
            auto abs_compare = [](S a, S b) { return (std::abs(a) > std::abs(b)); };
            auto theshortest = std::max_element (this->begin(), this->end(), abs_compare);
            S rtn = *theshortest;
            return rtn;
        }

        //! A version of shortest for vvec of vecs
        template <typename Sy=S, std::enable_if_t<morph::is_copyable_fixedsize<std::decay_t<Sy>>::value, int> = 0 >
        S shortest() const noexcept
        {
            auto theshortest = std::max_element (this->begin(), this->end(), [](S a, S b){return a.length_gtrthan(b);});
            return theshortest == this->end() ? S{0} : *theshortest;
        }

        /*!
         * Find the shortest non-zero type S element in this vvec.
         *
         * \return The shortest non-zero element, or if there are NO non-zero elements in this vvec,
         * return S{0}
         */
        template <typename Sy=S, std::enable_if_t<std::is_scalar<std::decay_t<Sy>>::value, int> = 0 >
        S shortest_nonzero() const noexcept
        {
            // We need to find the first non-zero element in *this, else max_element will wrongly
            // return 0 with our abs_compare_nonz function
            auto nonzero_start = this->begin();
            while (nonzero_start != this->end() && *nonzero_start == S{0}) {
                ++nonzero_start;
            }
            // Return value of 0 means there were no non-zero elements in the vvec
            if (nonzero_start == this->end()) { return S{0}; }

            auto abs_compare_nonz = [](S a, S b) { return (std::abs(a) > std::abs(b) && b != S{0}); };
            auto theshortest = std::max_element (nonzero_start, this->end(), abs_compare_nonz);
            S rtn = *theshortest;
            return rtn;
        }

        /*!
         * Find the shortest non-zero length type S vector in this vvec.
         *
         * \return The shortest non-zero length vector, or if there are NO non-zero length vectors
         * in this vvec, return S{0}
         */
        template <typename Sy=S, std::enable_if_t<morph::is_copyable_fixedsize<std::decay_t<Sy>>::value, int> = 0 >
        S shortest_nonzero() const noexcept
        {
            // Move to first non-zero length vector in *this
            auto nonzero_start = this->begin();
            while (nonzero_start != this->end() && nonzero_start->length() == typename S::value_type{0}) {
                ++nonzero_start;
            }
            // If NO non-zero length vectors in *this, return a zero length vector
            if (nonzero_start == this->end()) { return S{0}; }

            auto shortest_nonz = [](S a, S b) { return a.length_gtrthan(b) && b.length() > typename S::value_type{0}; };
            auto theshortest = std::max_element (nonzero_start, this->end(), shortest_nonz);
            return theshortest == this->end() ? S{0} : *theshortest;
        }

        /*!
         * \return the index of the shortest component of the vector. If this is a vector
         * of vectors, then return the index of the shortest vector.
         */
        template <typename Sy=S, std::enable_if_t<std::is_scalar<std::decay_t<Sy>>::value, int> = 0 >
        std::size_t argshortest() const noexcept
        {
            std::size_t idx = 0;
            // Check on the type S. If S is a vector thing, then abs_compare needs to be different.
            if constexpr (std::is_scalar<std::decay_t<S>>::value) {
                auto abs_compare = [](S a, S b) { return (std::abs(a) > std::abs(b)); };
                auto theshortest = std::max_element (this->begin(), this->end(), abs_compare);
                idx = (theshortest - this->begin());
            } else {
                auto vec_compare = [](S a, S b) { return (a.length() > b.length()); };
                auto theshortest = std::max_element (this->begin(), this->end(), vec_compare);
                idx = (theshortest - this->begin());
            }
            return idx;
        }

        //! vvec of vecs version of argshortest
        template <typename Sy=S, std::enable_if_t<morph::is_copyable_fixedsize<std::decay_t<Sy>>::value, int> = 0 >
        std::size_t argshortest() const noexcept
        {
            auto theshortest = std::max_element (this->begin(), this->end(), [](S a, S b){return a.length_gtrthan(b);});
            std::size_t idx = (theshortest - this->begin());
            return idx;
        }

        //! \return the value of the maximum (most positive) component of the vector.
        template <typename Sy=S, std::enable_if_t<std::is_scalar<std::decay_t<Sy>>::value, int> = 0 >
        S max() const noexcept
        {
            auto themax = std::max_element (this->begin(), this->end());
            return themax == this->end() ? S{0} : *themax;
        }

        //! \return the max lengthed element of the vvec. Intended for use with a vvec of vecs
        //! (morph::vvec<morph::vec<T, N>>). Note that the enclosed non-scalar thing must have
        //! function length_lessthan (as morph::vec does).
        template <typename Sy=S, std::enable_if_t<morph::is_copyable_fixedsize<std::decay_t<Sy>>::value, int> = 0 >
        S max() const noexcept
        {
            auto themax = std::max_element (this->begin(), this->end(), [](S a, S b){return a.length_lessthan(b);});
            return themax == this->end() ? S{0} : *themax;
        }

        //! \return the index of the maximum (most positive) component of the vector.
        template <typename Sy=S, std::enable_if_t<std::is_scalar<std::decay_t<Sy>>::value, int> = 0 >
        std::size_t argmax() const noexcept
        {
            auto themax = std::max_element (this->begin(), this->end());
            std::size_t idx = (themax - this->begin());
            return idx;
        }

        //! vvec of vecs version of argmax returns the index of the maximum length morph::vec
        template <typename Sy=S, std::enable_if_t<morph::is_copyable_fixedsize<std::decay_t<Sy>>::value, int> = 0 >
        std::size_t argmax() const noexcept
        {
            auto themax = std::max_element (this->begin(), this->end(), [](S a, S b){return a.length_lessthan(b);});
            std::size_t idx = (themax - this->begin());
            return idx;
        }

        //! \return the value of the minimum (smallest or most negative) component of the vector.
        template <typename Sy=S, std::enable_if_t<std::is_scalar<std::decay_t<Sy>>::value, int> = 0 >
        S min() const noexcept
        {
            auto themin = std::min_element (this->begin(), this->end());
            return themin == this->end() ? S{0} : *themin;
        }

        //! For a vvec of vecs, min() is shortest()
        template <typename Sy=S, std::enable_if_t<morph::is_copyable_fixedsize<std::decay_t<Sy>>::value, int> = 0 >
        S min() const noexcept { return this->shortest(); }

        //! \return the index of the minimum (smallest or most negative) component of the vector.
        template <typename Sy=S, std::enable_if_t<std::is_scalar<std::decay_t<Sy>>::value, int> = 0 >
        std::size_t argmin() const noexcept
        {
            auto themin = std::min_element (this->begin(), this->end());
            std::size_t idx = (themin - this->begin());
            return idx;
        }

        //! For a vvec of vecs, argmin() is argshortest()
        template <typename Sy=S, std::enable_if_t<morph::is_copyable_fixedsize<std::decay_t<Sy>>::value, int> = 0 >
        std::size_t argmin() const noexcept { return this->argshortest(); }

        //! \return the min and max values of the vvec, ignoring any not-a-number elements. If you
        //! pass 'true' as the template arg, then you can test for nans, and return the min/max of
        //! the rest of the numbers
        template<bool test_for_nans = false>
        morph::range<S> minmax() const { return this->range<test_for_nans>(); }

        //! \return the range of values in the vvec (the min and max values). If you pass 'true' as
        //! the template arg, then you can test for nans, and return the min/max of the rest of the
        //! numbers
        template<bool test_for_nans = false, typename Sy = S, std::enable_if_t<std::is_scalar<std::decay_t<Sy>>::value, int> = 0 >
        morph::range<S> range() const
        {
            morph::range<S> r;
            if constexpr (test_for_nans) {
                if (this->has_nan()) {
                    // Deal with non-numbers by removing them
                    morph::vvec<S> sans_nans = this->prune_nan();
                    auto mme = std::minmax_element (sans_nans.begin(), sans_nans.end());
                    r.min = mme.first == sans_nans.end() ? S{0} : *mme.first;
                    r.max = mme.second == sans_nans.end() ? S{0} : *mme.second;
                } else {
                    auto mme = std::minmax_element (this->begin(), this->end());
                    r.min = mme.first == this->end() ? S{0} : *mme.first;
                    r.max = mme.second == this->end() ? S{0} : *mme.second;
                }
            } else { // no testing for nans
                // minmax_element returns pair<vvec<S>::iterator, vvec<S>::iterator>
                auto mme = std::minmax_element (this->begin(), this->end());
                r.min = mme.first == this->end() ? S{0} : *mme.first;
                r.max = mme.second == this->end() ? S{0} : *mme.second;
            }
            return r;
        }

        /*!
         * vvec of vecs version of range(). Define this as the shortest vector to the
         * longest vector.
         */
        template<typename Sy = S, std::enable_if_t<morph::is_copyable_fixedsize<std::decay_t<Sy>>::value, int> = 0 >
        morph::range<S> range() const
        {
            morph::range<S> r;
            r.min = this->min(); // the shortest vec
            r.max = this->max(); // the longest vec
            return r;
        }

        // The extent if S is scalar is just the same as range; a morph::range<S> is returned.
        template <typename Sy=S, std::enable_if_t<std::is_scalar<std::decay_t<Sy>>::value, int> = 0 >
        morph::range<S> extent() const { return this->range(); }

        /*!
         * For a vvec of vecs/arrays extent (that is, S is a morph::vec or std::array),
         * we return a range with min containing the minimum value for each dimension
         * and max containing the max value for each dimension. This gives the
         * N-dimensional volume that contains all the coordinates in the vvec.
         *
         * This function is enabled for S types that are fixed size containers
         * (morph::is_copyable_fixedsize).
         */
        template <typename Sy=S, std::enable_if_t<morph::is_copyable_fixedsize<std::decay_t<Sy>>::value, int> = 0 >
        morph::range<S> extent() const noexcept
        {
            constexpr S s = {};                  // A dummy variable whose size is stored as sz
            constexpr std::size_t sz = s.size(); // Should work for S=std::array or morph::vec
            // Obtain the element type of S:
            using S_el = std::remove_reference_t<decltype(*std::begin(std::declval<S&>()))>;
            // min and max vectors for the extents
            S min = {};
            S max = {};
            for (std::size_t i = 0; i < sz; ++i) {
                min[i] = std::numeric_limits<S_el>::max();
                max[i] = std::numeric_limits<S_el>::lowest();
            }
            // Now go through the vvec, element by element, to find the extents
            for (std::size_t j = 0; j < this->size(); ++j) {
                for (std::size_t i = 0; i < sz; ++i) {
                    min[i] = (*this)[j][i] < min[i] ? (*this)[j][i] : min[i];
                    max[i] = (*this)[j][i] > max[i] ? (*this)[j][i] : max[i];
                }
            }
            return morph::range<S>{min, max};
        }

        /*!
         * \brief Finds the 'crossing points' of a function
         *
         * Returned as a float so that it can specify intermediate values and also indicate the
         * direction of the crossing. For example, if the function crosses from a <val at index 2 to
         * >val at index 3, then the entry in the return object would be 2.5f. If it also crosses
         * val from >val at index 6 to <val at index 7, then the other element in the return object
         * would be -6.5f. If the function evaluates as val *at* index 12, then the last entry would
         * be 12.0f.
         *
         * \param val the data value that the crossing points cross over.
         *
         * \param wrap Does the data wrap? If so, start and end elements may cross the value
         *
         * \return the locations where the function crosses the value.
         */
        vvec<float> crossing_points (const S& val, const wrapdata wrap = wrapdata::none) const
        {
            int n = this->size();
            vvec<float> crossings;
            if (wrap == wrapdata::none) {
                S lastval = (*this)[0];
                // What about the very first one? By definition, it can't cross.
                for (int i = 1; i < n; ++i) {
                    if ((*this)[i] == val) {
                        // Positive or negative crossing?
                        if (i == n-1) {
                            // 0 right at end. In no-wrap mode, can't cross by definition.
                        } else {
                            if (lastval < val && ((*this)[i+1] > val)) {
                                crossings.push_back (static_cast<float>(i));
                            } else if (lastval > val && ((*this)[i+1] < val)) {
                                crossings.push_back (-static_cast<float>(i));
                            }
                            //else if (lastval > val && ((*this)[i+1] > val)) { dirn = -1; } // fn touches 0 from above, DOESN'T cross
                            //else if (lastval < val && ((*this)[i+1] < val)) { dirn = 1; } // fn touches 0 from below, DOESN'T cross
                        }

                    } else if (lastval > val && (*this)[i] < val) {
                        crossings.push_back (-static_cast<float>(i) + 0.5f);
                    } else if (lastval < val && (*this)[i] > val) {
                        crossings.push_back (static_cast<float>(i) - 0.5f);
                    }
                    lastval = (*this)[i];
                }
            } else { // wrapdata::wrap
                S lastval = (*this)[n-1];
                for (int i = 0; i < n; ++i) {
                    if ((*this)[i] == val) {
                        // Positive or negative crossing?
                        if (i == n-1) {
                            // 0 right at end.
                            if (lastval < val && ((*this)[0] > val)) {
                                crossings.push_back (static_cast<float>(i));
                            } else if (lastval > val && ((*this)[0] < val)) {
                                crossings.push_back (-static_cast<float>(i));
                            }
                        } else {
                            if (lastval < val && ((*this)[i+1] > val)) {
                                crossings.push_back (static_cast<float>(i));
                            } else if (lastval > val && ((*this)[i+1] < val)) {
                                crossings.push_back (-static_cast<float>(i));
                            }
                        }

                    } else if (lastval > val && (*this)[i] < val) {
                        crossings.push_back (i > 0 ? -static_cast<float>(i)+0.5f : -static_cast<float>(n-1)-0.5f);
                    } else if (lastval < val && (*this)[i] > val) {
                        crossings.push_back (i > 0 ? static_cast<float>(i)-0.5f :  static_cast<float>(n-1)-0.5f);
                    }
                    lastval = (*this)[i];
                }
                // Crossing at the start should show up at the end
                if (!crossings.empty() && std::abs(crossings[0]) > 1.0f) { crossings.rotate(); }
            }
            return crossings;
        }

        /*!
         * \brief Finds the 'zero-crossings' of a function
         *
         * Returned as a float so that it can take intermediate values and also indicate
         * the direction of the crossing. For example, if the function crosses from a
         * negative value at index 2 to a positive value at index 3, then the entry in
         * the return object would be 2.5f. If it also crosses zero from +ve at index 6
         * to -ve at index 7, then the other element in the return object would be
         * -6.5f. If the function evaluates as zero *at* index 12, then the last entry
         * would be 12.0f.
         *
         * \param wrap Does the data wrap? If so, start and end elements may cross zero
         *
         * \return the locations where the function crosses zero.
         */
        vvec<float> zerocross (const wrapdata wrap = wrapdata::none) const { return this->crossing_points (S{0}, wrap); }

        //! Perform element-wise max. For each element, if val is the maximum, the element becomes val.
        template <typename Sy=S>
        void max_elementwise_inplace (const Sy& val) noexcept { for (auto& i : *this) { i = std::max (i, val); } }

        //! Perform element-wise min. For each element, if val is the minimum, the element becomes val.
        template <typename Sy=S>
        void min_elementwise_inplace (const Sy& val) noexcept { for (auto& i : *this) { i = std::min (i, val); } }

        //! Find first element matching argument
        template <typename Sy=S>
        std::size_t find_first_of (const Sy& val) const noexcept
        {
            std::size_t i = 0;
            for (i = 0; i < this->size(); i++) {
                if ((*this)[i] == val) { break; }
            }
            return i;
        }

        //! Find last element matching argument
        template <typename Sy=S>
        std::size_t find_last_of (const Sy& val) const noexcept
        {
            std::size_t i = 0;
            for (i = this->size()-1; i >= 0; i--) {
                if ((*this)[i] == val) { break; }
            }
            return i;
        }

        //! Find all elements matching argument, returning a vvec containing the indices.
        template <typename Sy=S>
        morph::vvec<std::size_t> find (const Sy& val) const
        {
            morph::vvec<std::size_t> indices;
            std::size_t i = 0;
            for (i = 0; i < this->size(); i++) {
                if ((*this)[i] == val) { indices.push_back(i); }
            }
            return indices;
        }

        //! \return true if any element is zero
        bool has_zero() const noexcept
        {
            return std::any_of (this->cbegin(), this->cend(), [](S i){ return i == S{0}; });
        }

        //! \return true if any element is infinity
        bool has_inf() const noexcept
        {
            if constexpr (std::numeric_limits<S>::has_infinity) {
                return std::any_of (this->cbegin(), this->cend(), [](S i){return std::isinf(i);});
            } else {
                return false;
            }
        }

        //! \return true if any element is NaN
        bool has_nan() const noexcept
        {
            if constexpr (std::numeric_limits<S>::has_quiet_NaN || std::numeric_limits<S>::has_signaling_NaN) {
                return std::any_of (this->cbegin(), this->cend(), [](S i){return std::isnan(i);});
            } else {
                return false;
            }
        }

        //! \return true if any element is NaN or infinity
        bool has_nan_or_inf() const noexcept
        {
            bool has_nan_or_inf = false;
            has_nan_or_inf = this->has_nan();
            return has_nan_or_inf ? has_nan_or_inf : this->has_inf();
        }

        //! \return the arithmetic mean of the elements
        template<bool test_for_nans = false, typename Sy=S>
        Sy mean() const noexcept
        {
            if constexpr (test_for_nans) {
                if (this->has_nan()) {
                    // Deal with non-numbers with a special accumulate function
                    std::size_t n_nans = 0u;
                    auto _ignoring_nans = [&n_nans](Sy a, S b) mutable { return std::isnan(b) ? ++n_nans, a : a + b; };
                    Sy sum =  std::accumulate (this->begin(), this->end(), Sy{0}, _ignoring_nans);
                    return sum / (this->size() - n_nans);
                } else {
                    const Sy sum = std::accumulate (this->begin(), this->end(), Sy{0});
                    return sum / this->size();
                }
            } else {
                const Sy sum = std::accumulate (this->begin(), this->end(), Sy{0});
                return sum / this->size();
            }
        }

        //! \return the variance of the elements
        template<bool test_for_nans = false, typename Sy=S>
        Sy variance() const noexcept
        {
            if (this->empty()) { return S{0}; }
            Sy _mean = this->mean<test_for_nans, Sy>();
            Sy sos_deviations = Sy{0};
            std::size_t n_nans = 0u;
            for (S val : *this) {
                if constexpr (test_for_nans) {
                    if (std::isnan (val)) {
                        ++n_nans; continue;
                    }
                }
                sos_deviations += ((val - _mean) * (val - _mean));
            }
            Sy variance = sos_deviations / (this->size() - 1 - n_nans);
            return variance;
        }

        //! \return the standard deviation of the elements
        template<bool test_for_nans = false, typename Sy=S>
        Sy std() const noexcept
        {
            if (this->empty()) { return Sy{0}; }
            return std::sqrt (this->variance<test_for_nans, Sy>());
        }

        //! \return the sum of the elements. If elements are of a constrained type, you can call this something like:
        //! vvec<uint8_t> uv (256, 10);
        //! unsigned int thesum = uv.sum<false, unsigned int>();
        //! If test_for_nans is true, then sum is performed ignoring any nan values
        template<bool test_for_nans = false, typename Sy=S>
        Sy sum() const noexcept
        {
            if constexpr (test_for_nans) {
                auto _ignoring_nans = [](Sy a, S b) mutable { return std::isnan(b) ? a : a + b; };
                return std::accumulate (this->begin(), this->end(), Sy{0}, _ignoring_nans);
            } else {
                return std::accumulate (this->begin(), this->end(), Sy{0});
            }
        }

        //! \return the product of the elements. If elements are of a constrained type, you can call
        //! this something like:
        //! vvec<uint8_t> uv (256, 10);
        //! unsigned int theproduct = uv.product<unsigned int>();
        template<bool test_for_nans = false, typename Sy=S>
        Sy product() const noexcept
        {
            if constexpr (test_for_nans) {
                auto _product_ign_nans = [](Sy a, S b) mutable { return (a ? (std::isnan(b) ? a : a * b) : b); };
                return std::accumulate (this->begin(), this->end(), Sy{0}, _product_ign_nans);
            } else {
                auto _product = [](Sy a, S b) mutable { return a ? a * b : b; };
                return std::accumulate (this->begin(), this->end(), Sy{0}, _product);
            }
        }

        /*!
         * Compute the element-wise pth power of the vector
         *
         * \return a vvec whose elements have been raised to the power p
         */
        template<typename Sy=S>
        vvec<Sy> pow (const S& p) const noexcept
        {
            vvec<Sy> rtn(this->size());
            auto raise_to_p = [p](S elmnt) { return std::pow(elmnt, p); };
            std::transform (this->begin(), this->end(), rtn.begin(), raise_to_p);
            return rtn;
        }
        //! Raise each element to the power p
        void pow_inplace (const S& p) noexcept { for (auto& i : *this) { i = std::pow (i, p); } }

        //! Element-wise power
        template<typename Sy=S>
        vvec<S> pow (const vvec<Sy>& p) const
        {
            if (p.size() != this->size()) {
                throw std::runtime_error ("element-wise power: p dims should equal vvec's dims");
            }
            auto pi = p.begin();
            vvec<S> rtn(this->size());
            auto raise_to_p = [pi](S elmnt) mutable { return std::pow(elmnt, static_cast<S>(*pi++)); };
            std::transform (this->begin(), this->end(), rtn.begin(), raise_to_p);
            return rtn;
        }
        //! Raise each element, i, to the power p[i]
        template<typename Sy=S>
        void pow_inplace (const vvec<Sy>& p)
        {
            if (p.size() != this->size()) {
                throw std::runtime_error ("element-wise power: p dims should equal vvec's dims");
            }
            auto pi = p.begin();
            for (auto& i : *this) { i = std::pow (i, static_cast<S>(*pi++)); }
        }

        //! \return the signum of the vvec, with signum(0)==0
        vvec<S> signum() const noexcept
        {
            vvec<S> rtn(this->size());
            auto _signum = [](S elmnt) { return (elmnt > S{0} ? S{1} : (elmnt == S{0} ? S{0} : S{-1})); };
            std::transform (this->begin(), this->end(), rtn.begin(), _signum);
            return rtn;
        }
        void signum_inplace() noexcept { for (auto& i : *this) { i = (i > S{0} ? S{1} : (i == S{0} ? S{0} : S{-1})); } }

        //! \return a vvec which is a copy of *this for which positive, non-zero elements have been removed
        vvec<S> prune_positive() const
        {
            vvec<S> rtn;
            for (auto& i : *this) { if (i <= S{0}) { rtn.push_back(i); } }
            return rtn;
        }
        void prune_positive_inplace()
        {
            vvec<S> pruned;
            // We *could* reduce *this down, but that would involve a lot of std::vector deletions.
            for (auto& i : *this) { if (i <= S{0}) { pruned.push_back(i); } }
            this->swap (pruned);
        }

        //! \return a vvec which is a copy of *this for which negative, non-zero elements have been removed
        vvec<S> prune_negative() const
        {
            vvec<S> rtn;
            for (auto& i : *this) { if (i >= S{0}) { rtn.push_back(i); } }
            return rtn;
        }
        void prune_negative_inplace()
        {
            vvec<S> pruned;
            for (auto& i : *this) { if (i >= S{0}) { pruned.push_back(i); } }
            this->swap (pruned);
        }

        //! \return a vvec which is a copy of *this for which zero-valued elements have been removed
        vvec<S> prune_zero() const
        {
            vvec<S> rtn;
            for (auto& i : *this) { if (i != S{0}) { rtn.push_back(i); } }
            return rtn;
        }
        void prune_zero_inplace()
        {
            vvec<S> pruned;
            for (auto& i : *this) { if (i != S{0}) { pruned.push_back(i); } }
            this->swap (pruned);
        }

        //! \return a vvec which is a copy of *this for which NaN elements have been removed
        vvec<S> prune_nan() const
        {
            static_assert (std::numeric_limits<S>::has_quiet_NaN, "S does not have quiet_NaNs");
            vvec<S> rtn;
            for (auto& i : *this) { if (!std::isnan(i)) { rtn.push_back(i); } }
            return rtn;
        }
        void prune_nan_inplace()
        {
            static_assert (std::numeric_limits<S>::has_quiet_NaN, "S does not have quiet_NaNs");
            vvec<S> pruned;
            for (auto& i : *this) { if (!std::isnan(i)) { pruned.push_back(i); } }
            this->swap (pruned);
        }

        void replace_nan_with (const S replacement) noexcept
        {
            static_assert (std::numeric_limits<S>::has_quiet_NaN, "S does not have quiet_NaNs");
            for (auto& i : *this) { if (std::isnan(i)) { i = replacement; } }
        }

        void replace_nan_or_inf_with (const S replacement) noexcept
        {
            static_assert (std::numeric_limits<S>::has_quiet_NaN, "S does not have quiet_NaNs");
            static_assert (std::numeric_limits<S>::has_infinity, "S does not have infinities");
            for (auto& i : *this) { if (std::isnan(i) || std::isinf(i)) { i = replacement; } }
        }

        void search_replace (const S searchee, const S replacement) noexcept
        {
            for (auto& i : *this) { if (i == searchee) { i = replacement; } }
        }

        // \return a vec in which we replace any value that's above upper with upper and any below lower with lower
        vvec<S> threshold (const S lower, const S upper) const
        {
            vvec<S> rtn(this->size());
            auto _threshold = [lower, upper](S elmnt) {
                return (elmnt <= lower ? lower : (elmnt >= upper ? upper : elmnt));
            };
            std::transform (this->begin(), this->end(), rtn.begin(), _threshold);
            return rtn;
        }

        // Replace any value that's above upper with upper and any below lower with lower
        void threshold_inplace (const S lower, const S upper) noexcept
        {
            for (auto& i : *this) { i = (i <= lower ? lower : (i >= upper ? upper : i)); }
        }

        /*!
         * Compute the element-wise square root of the vector
         *
         * \return a vvec whose elements have been square-rooted
         */
        vvec<S> sqrt() const
        {
            vvec<S> rtn(this->size());
            auto sqrt_element = [](S elmnt) { return static_cast<S>(std::sqrt(elmnt)); };
            std::transform (this->begin(), this->end(), rtn.begin(), sqrt_element);
            return rtn;
        }
        //! Replace each element with its own square root
        void sqrt_inplace() noexcept { for (auto& i : *this) { i = static_cast<S>(std::sqrt (i)); } }

        /*!
         * Compute the element-wise square of the vector
         *
         * \return a vvec whose elements have been squared
         */
        vvec<S> sq() const
        {
            vvec<S> rtn(this->size());
            auto sq_element = [](S elmnt) { return std::pow(elmnt, 2); };
            std::transform (this->begin(), this->end(), rtn.begin(), sq_element);
            return rtn;
        }
        //! Replace each element with its own square
        void sq_inplace() noexcept { for (auto& i : *this) { i = (i*i); } }

        /*!
         * Compute the element-wise natural logarithm of the vector
         *
         * \return a vvec whose elements have been logged
         */
        vvec<S> log() const
        {
            vvec<S> rtn(this->size());
            auto log_element = [](S elmnt) { return std::log(elmnt); };
            std::transform (this->begin(), this->end(), rtn.begin(), log_element);
            return rtn;
        }
        //! Replace each element with its own log
        void log_inplace() { for (auto& i : *this) { i = std::log(i); } }

        /*!
         * Compute the element-wise logarithm-to-base-10 of the vector
         *
         * \return a vvec whose elements have been log10ed
         */
        vvec<S> log10() const
        {
            vvec<S> rtn(this->size());
            auto log_element = [](S elmnt) { return std::log10(elmnt); };
            std::transform (this->begin(), this->end(), rtn.begin(), log_element);
            return rtn;
        }
        //! Replace each element with its own log
        void log10_inplace() { for (auto& i : *this) { i = std::log10(i); } }

        //! Sine
        vvec<S> sin() const
        {
            vvec<S> rtn(this->size());
            auto sin_element = [](S elmnt) { return std::sin(elmnt); };
            std::transform (this->begin(), this->end(), rtn.begin(), sin_element);
            return rtn;
        }
        //! Replace each element with its own sine
        void sin_inplace() { for (auto& i : *this) { i = std::sin(i); } }

        //! Cosine
        vvec<S> cos() const
        {
            vvec<S> rtn(this->size());
            auto cos_element = [](S elmnt) { return std::cos(elmnt); };
            std::transform (this->begin(), this->end(), rtn.begin(), cos_element);
            return rtn;
        }
        //! Replace each element with its own cosine
        void cos_inplace() { for (auto& i : *this) { i = std::cos(i); } }

        /*!
         * Compute the element-wise natural exponential of the vector
         *
         * \return a vvec whose elements have been exponentiate
         */
        vvec<S> exp() const
        {
            vvec<S> rtn(this->size());
            auto exp_element = [](S elmnt) { return std::exp(elmnt); };
            std::transform (this->begin(), this->end(), rtn.begin(), exp_element);
            return rtn;
        }
        //! Replace each element with its own exp
        void exp_inplace() { for (auto& i : *this) { i = std::exp(i); } }

        /*!
         * Compute the element-wise absolute values of the vector
         *
         * \return a vvec whose elements have been 'absed'
         */
        vvec<S> abs() const
        {
            vvec<S> rtn(this->size());
            auto abs_element = [](S elmnt) { return std::abs(elmnt); };
            std::transform (this->begin(), this->end(), rtn.begin(), abs_element);
            return rtn;
        }
        //! Replace each element with its absolute value
        void abs_inplace() noexcept { for (auto& i : *this) { i = std::abs(i); } }

        //! Compute the symmetric Gaussian function
        vvec<S> gauss (const S sigma) const
        {
            vvec<S> rtn(this->size());
            auto _element = [sigma](S i) { return std::exp (i*i/(S{-2}*sigma*sigma)); };
            std::transform (this->begin(), this->end(), rtn.begin(), _element);
            return rtn;
        }
        void gauss_inplace (const S sigma)
        {
            for (auto& i : *this) { i = std::exp (i*i/(S{-2}*sigma*sigma)); }
        }

        //! \return a vvec containing the generalised logistic function of this vvec:
        //! f(x) = 1 / [ 1 + exp(-k*(x - x0)) ]
        vvec<S> logistic (const S k = S{1}, const S x0 = S{0}) const
        {
            vvec<S> rtn(this->size());
            auto _logisticfn = [k, x0](S _x) { return S{1} / (S{1} + std::exp (k*(x0 - _x))); };
            std::transform (this->begin(), this->end(), rtn.begin(), _logisticfn);
            return rtn;
        }
        //! Replace each element x with the generalised logistic function of the element:
        //! f(x) = 1 / [ 1 + exp(-k*(x - x0)) ]
        void logistic_inplace (const S k = S{1}, const S x0 = S{0})
        {
            for (auto& _x : *this) { _x = S{1} / (S{1} + std::exp (k*(x0 - _x))); }
        }

        //! Smooth the vector by convolving with a gaussian filter with Gaussian width
        //! sigma and overall width 2*sigma*n_sigma
        template<wrapdata wrap = wrapdata::none>
        vvec<S> smooth_gauss (const S sigma, const unsigned int n_sigma) const
        {
            morph::vvec<S> filter;
            S hw = std::round(sigma*n_sigma);
            std::size_t elements = static_cast<std::size_t>(2*hw) + 1;
            filter.linspace (-hw, hw, elements);
            filter.gauss_inplace (sigma);
            filter /= filter.sum();
            return this->convolve<wrap, centre_kernel::yes, resize_output::no> (filter);
        }
        //! Gaussian smoothing in place
        template<wrapdata wrap = wrapdata::none>
        void smooth_gauss_inplace (const S sigma, const unsigned int n_sigma)
        {
            morph::vvec<S> filter;
            S hw = std::round(sigma*n_sigma);
            std::size_t elements = static_cast<std::size_t>(2*hw) + 1;
            filter.linspace (-hw, hw, elements);
            filter.gauss_inplace (sigma);
            filter /= filter.sum();
            this->convolve_inplace<wrap, centre_kernel::yes, resize_output::no> (filter);
        }

        //! Do 1-D convolution of *this with the presented kernel and return the result
        //! \tparam wrap whether or not we wrap around the ends of the vvec.
        //! \tparam resize_output If true, execute the pure maths version of convolve, in
        //! which the vvec returned is be larger than the input by (kernel_width-1).
        template<wrapdata wrap = wrapdata::none,
                 centre_kernel centre = centre_kernel::yes,
                 resize_output resize_out = resize_output::no>
        vvec<S> convolve (const vvec<S>& kernel) const
        {
            int sz = this->size();
            int osz = sz;           // osz is size of output vvec
            int kw = kernel.size(); // kernel width
            int zki = 0;            // zero of the kernel index (or how many steps 'right'
                                    // to shift the reversed kernel before starting)
            if constexpr (centre == centre_kernel::yes) { zki = kw / 2; }
            if constexpr (resize_out == resize_output::yes) { osz += (kw - 1); }
            if constexpr (wrap == wrapdata::wrap) {
                if (kw > sz) { throw std::runtime_error ("if wrapping, kernel width must be <= data size"); }
            }
            vvec<S> rtn(osz);
            for (int i = 0; i < osz; ++i) {
                // For each element, i, compute the convolution sum
                S sum = S{0};
                for (int j = 0; j < kw; ++j) {
                    // ii is the index into the data by which kernel[j] should be multiplied
                    int ii = i - j + zki; // -j effectively 'flips' the kernel, as is required by
                                          // the definition of convolution
                    //std::cout << "i=" << i << " j=" << j << " zki=" << zki << ", data index i-j+zki=" << ii << std::endl;
                    if constexpr (wrap == wrapdata::wrap) {
                        // Handle wrapping around the data
                        ii += ii < 0 ? sz : 0;
                        ii -= ii >= sz ? sz : 0;
                        //std::cout << "wrap: ii becomes " << ii << std::endl;
                    } // else nothing to do
                    if (ii < 0 || ii >= sz) { continue; }
                    //std::cout << "rtn[" << i << "] += " << (*this)[ii] << " * " << kernel[j] << " = " << (*this)[ii] * kernel[j] << std::endl;
                    sum += (*this)[ii] * kernel[j];
                }
                rtn[i] = sum;
            }
            return rtn;
        }
        template<wrapdata wrap = wrapdata::none,
                 centre_kernel centre = centre_kernel::yes,
                 resize_output resize_out = resize_output::no>
        void convolve_inplace (const vvec<S>& kernel)
        {
            int sz = this->size();
            vvec<S> d(*this);        // We make a copy of *this;
            int osz = sz;            // osz is size of output vvec
            int kw = kernel.size();  // kernel width
            int zki = 0;             // zero of the kernel index
            if constexpr (centre == centre_kernel::yes) { zki = kw / 2; }
            if constexpr (resize_out == resize_output::yes) {
                osz += (kw - 1);
                this->resize (osz); // resize self
            }
            if constexpr (wrap == wrapdata::wrap) {
                if (kw > sz) { throw std::runtime_error ("if wrapping, kernel width must be <= data size"); }
            }
            for (int i = 0; i < osz; ++i) {
                // For each element, i, compute the convolution sum
                S sum = S{0};
                for (int j = 0; j < kw; ++j) {
                    // ii is the index into the data by which kernel[j] should be multiplied
                    int ii = i - j + zki;
                    if constexpr (wrap == wrapdata::wrap) {
                        // Handle wrapping around the data
                        ii += ii < 0 ? sz : 0;
                        ii -= ii >= sz ? sz : 0;
                    } // else nothing to do
                    if (ii < 0 || ii >= sz) { continue; }
                    sum += d[ii] * kernel[j];
                }
                (*this)[i] = sum;
            }
        }

        //! \return the discrete differential, computed as the mean difference between a
        //! datum and its adjacent neighbours.
        vvec<S> diff (const wrapdata wrap = wrapdata::none)
        {
            int n = this->size();
            vvec<S> rtn (n);
            if (wrap == wrapdata::none) {
                S last = (*this)[0];
                rtn[0] = (*this)[1] - last;
                for (int i = 1; i < n-1; ++i) {
                    S difn = S{0.5} * (((*this)[i] - last) + ((*this)[i+1] - (*this)[i]));
                    last = (*this)[i];
                    rtn[i] = difn;
                }
                rtn[n-1] = (*this)[n-1] - last;
            } else {
                S last = (*this)[n-1];
                for (int i = 0; i < n; ++i) {
                    S next = (i == n-1) ? (*this)[0] : (*this)[i+1];
                    S difn = S{0.5} * (((*this)[i] - last) + (next         - (*this)[i]));
                    last = (*this)[i];
                    rtn[i] = difn;
                }
            }
            return rtn;
        }
        //! Compute the discrete differential of the data in *this
        void diff_inplace (const wrapdata wrap = wrapdata::none)
        {
            int n = this->size();
            if (wrap == wrapdata::none) {
                S last = (*this)[0];
                (*this)[0] = (*this)[1] - last; // first step precedes loop
                for (int i = 1; i < n-1; ++i) {
                    S difn = S{0.5} * (((*this)[i] - last) + ((*this)[i+1] - (*this)[i]));
                    last = (*this)[i];
                    (*this)[i] = difn;
                }
                (*this)[n-1] = (*this)[n-1] - last; // last step follows the loop
            } else { // DO wrap
                S first = (*this)[0];
                S last = (*this)[n-1];
                for (int i = 0; i < n; ++i) {
                    S next = (i == n-1) ? first : (*this)[i+1];
                    S difn = S{0.5} * (((*this)[i] - last) + (next         - (*this)[i]));
                    last = (*this)[i];
                    (*this)[i] = difn;
                }
            }
        }

        // Element-wise greater-than-or-equal comparison. Put 1 in each element of a
        // return vvec for which *this is >= val, else 0.
        vvec<S> element_compare_gteq (const S val) const
        {
            vvec<S> comparison(this->size(), S{0});
            for (unsigned int i = 0; i < this->size(); ++i) {
                comparison[i] = (*this)[i] >= val ? S{1} : S{0};
            }
            return comparison;
        }
        vvec<S> element_compare_gt (const S val) const
        {
            vvec<S> comparison(this->size(), S{0});
            for (unsigned int i = 0; i < this->size(); ++i) {
                comparison[i] = (*this)[i] > val ? S{1} : S{0};
            }
            return comparison;
        }
        vvec<S> element_compare_lt (const S val) const
        {
            vvec<S> comparison(this->size(), S{0});
            for (unsigned int i = 0; i < this->size(); ++i) {
                comparison[i] = (*this)[i] < val ? S{1} : S{0};
            }
            return comparison;
        }
        vvec<S> element_compare_lte (const S val) const
        {
            vvec<S> comparison(this->size(), S{0});
            for (unsigned int i = 0; i < this->size(); ++i) {
                comparison[i] = (*this)[i] <= val ? S{1} : S{0};
            }
            return comparison;
        }
        vvec<S> element_compare_eq (const S val) const
        {
            vvec<S> comparison(this->size(), S{0});
            for (unsigned int i = 0; i < this->size(); ++i) {
                comparison[i] = (*this)[i] == val ? S{1} : S{0};
            }
            return comparison;
        }
        vvec<S> element_compare_neq (const S val) const
        {
            vvec<S> comparison(this->size(), S{0});
            for (unsigned int i = 0; i < this->size(); ++i) {
                comparison[i] = (*this)[i] != val ? S{1} : S{0};
            }
            return comparison;
        }

        //! Less than a scalar. \return true if every element is less than the scalar
        bool operator<(const S rhs) const noexcept
        {
            auto _element_fails = [rhs](S a, S b) { return a + (b < rhs ? S{0} : S{1}); };
            return std::accumulate (this->begin(), this->end(), S{0}, _element_fails) == S{0} ? true : false;
        }

        //! <= a scalar. \return true if every element is less than the scalar
        bool operator<=(const S rhs) const noexcept
        {
            auto _element_fails = [rhs](S a, S b) { return a + (b <= rhs ? S{0} : S{1}); };
            return std::accumulate (this->begin(), this->end(), S{0}, _element_fails) == S{0} ? true : false;
        }

        //! Greater than a scalar. \return true if every element is gtr than the scalar
        bool operator>(const S rhs) const noexcept
        {
            auto _element_fails = [rhs](S a, S b) { return a + (b > rhs ? S{0} : S{1}); };
            return std::accumulate (this->begin(), this->end(), S{0}, _element_fails) == S{0} ? true : false;
        }

        //! >= a scalar. \return true if every element is gtr than the scalar
        bool operator>=(const S rhs) const noexcept
        {
            auto _element_fails = [rhs](S a, S b) { return a + (b >= rhs ? S{0} : S{1}); };
            return std::accumulate (this->begin(), this->end(), S{0}, _element_fails) == S{0} ? true : false;
        }

        // vvec-to-vvec comparisons. Although there's an underlying std::vector
        // implementation of these operators, I prefer to reimplement with a requirement
        // that the vvecs should have the same size to be compared, as this is a
        // better defined comparison. These might be slow if your vvec is very big.
        //
        // More importantly, I *completely redefine the meaning of the comparison
        // operators between vvecs*. This messes up the use of containers that make
        // use of comparison operators like std::set.
        //
        // Use something like this as a compare function when storing morph::vvec in
        // a std::set:
        //
        //    auto _cmp = [](vvec<float, 3> a, vvec<float, 3> b) { return a.lexical_lessthan(b); };
        //    std::set<vvec<float, 3>, decltype(_cmp)> aset(_cmp); // C++-11/C++-17
        //
        // If the std::set is a class member, then define a compare struct with an operator().
        //
        // The default comparison for std::set is the operator<. The definition here
        // applied to !comp(a,b) && !comp(b,a) will suggest that two different vvecs
        // are equal even when they're not and so your std::sets will fail to insert
        // unique vvecs

        //! Lexical less-than similar to the operator< implemented for std::vector
        template<typename Sy=S>
        bool lexical_lessthan (const vvec<Sy>& rhs) const
        {
            return std::lexicographical_compare (this->begin(), this->end(), rhs.begin(), rhs.end());
        }

        /*!
         * Like lexical_lessthan, but elements of the vvec must be less than by at least n_eps *
         * numeric_limits<Sy>::epsilon() to be different. If *this is less than rhs on that basis,
         * return true.
         */
        template<typename Sy=S>
        bool lexical_lessthan_beyond_epsilon (const vvec<Sy>& rhs, const int n_eps = 1) const
        {
            if (rhs.size() != this->size()) {
                throw std::runtime_error ("lexical_lessthan_beyond_epsilon: rhs dims should equal vvec's dims");
            }

            for (std::size_t i = 0; i < this->size(); ++i) {
                const Sy _this = (*this)[i];
                const Sy _rhs = rhs[i];
                const Sy eps = std::numeric_limits<Sy>::epsilon() * n_eps;
                if ((_rhs - _this) > eps) {
                    // _rhs is properly less than _this, so _this is gtr than _rhs
                    return false;
                } else if ((_this - _rhs) > eps) {
                    // _rhs is properly greater than _this, so _this is less than _rhs
                    return true;
                } // else next element
            }
            return false;
        }

        //! Another way to compare vectors would be by length.
        template<typename Sy=S>
        bool length_lessthan (const vvec<Sy>& rhs) const
        {
            if (rhs.size() != this->size()) {
                throw std::runtime_error ("length based comparison: rhs dims should equal vvec's dims");
            }
            return this->length() < rhs.length();
        }

        template<typename Sy=S>
        bool length_lte (const vvec<Sy>& rhs) const
        {
            if (rhs.size() != this->size()) {
                throw std::runtime_error ("length based comparison: rhs dims should equal vvec's dims");
            }
            return this->length() <= rhs.length();
        }

        template<typename Sy=S>
        bool length_gtrthan (const vvec<Sy>& rhs) const
        {
            if (rhs.size() != this->size()) {
                throw std::runtime_error ("length based comparison: rhs dims should equal vvec's dims");
            }
            return this->length() > rhs.length();
        }

        template<typename Sy=S>
        bool length_gte (const vvec<Sy>& rhs) const
        {
            if (rhs.size() != this->size()) {
                throw std::runtime_error ("length based comparison: rhs dims should equal vvec's dims");
            }
            return this->length() >= rhs.length();
        }

        //! \return true if each element of *this is less than its counterpart in rhs.
        template<typename Sy=S>
        bool operator< (const vvec<Sy>& rhs) const
        {
            if (rhs.size() != this->size()) {
                throw std::runtime_error ("element-wise comparison: rhs dims should equal vvec's dims");
            }
            auto ri = rhs.begin();
            auto _element_fails = [ri](S a, S b) mutable { return a + (b < (*ri++) ? S{0} : S{1}); };
            return std::accumulate (this->begin(), this->end(), S{0}, _element_fails) == S{0} ? true : false;
        }

        //! \return true if each element of *this is <= its counterpart in rhs.
        template<typename Sy=S>
        bool operator<= (const vvec<Sy>& rhs) const
        {
            if (rhs.size() != this->size()) {
                throw std::runtime_error ("element-wise comparison: rhs dims should equal vvec's dims");
            }
            auto ri = rhs.begin();
            auto _element_fails = [ri](S a, S b) mutable { return a + (b <= (*ri++) ? S{0} : S{1}); };
            return std::accumulate (this->begin(), this->end(), S{0}, _element_fails) == S{0} ? true : false;
        }

        //! \return true if each element of *this is greater than its counterpart in rhs.
        template<typename Sy=S>
        bool operator> (const vvec<Sy>& rhs) const
        {
            if (rhs.size() != this->size()) {
                throw std::runtime_error ("element-wise comparison: rhs dims should equal vvec's dims");
            }
            auto ri = rhs.begin();
            auto _element_fails = [ri](S a, S b) mutable { return a + (b > (*ri++) ? S{0} : S{1}); };
            return std::accumulate (this->begin(), this->end(), S{0}, _element_fails) == S{0} ? true : false;
        }

        //! \return true if each element of *this is >= its counterpart in rhs.
        template<typename Sy=S>
        bool operator>= (const vvec<Sy>& rhs) const
        {
            if (rhs.size() != this->size()) {
                throw std::runtime_error ("element-wise comparison: rhs dims should equal vvec's dims");
            }
            auto ri = rhs.begin();
            auto _element_fails = [ri](S a, S b) mutable { return a + (b >= (*ri++) ? S{0} : S{1}); };
            return std::accumulate (this->begin(), this->end(), S{0}, _element_fails) == S{0} ? true : false;
        }

        /*!
         * Unary negate operator
         *
         * \return a vvec whose elements have been negated.
         */
        vvec<S> operator-() const
        {
            vvec<S> rtn(this->size());
            std::transform (this->begin(), this->end(), rtn.begin(), std::negate<S>());
            return rtn;
        }

        /*!
         * Unary not operator.
         *
         * \return true if the vector length is 0, otherwise it returns false.
         */
        bool operator!() const noexcept { return (this->length() == S{0}) ? true : false; }

        /*!
         * \brief Scalar (dot) product of two vvecs
         *
         * Compute the scalar product of this vvec and the vvec, v.
         *
         * If \a v and *this have different sizes, then throw exception.
         *
         * \return scalar product
         */
        template<typename Sy=S>
        S dot (const vvec<Sy>& v) const
        {
            if (this->size() != v.size()) {
                throw std::runtime_error ("vvec::dot(): vectors must have equal size");
            }
            auto vi = v.begin();
            auto dot_product = [vi](S a, Sy b) mutable -> S { return a + static_cast<S>(b) * static_cast<S>(*vi++); };
            const S rtn = std::accumulate (this->begin(), this->end(), S{0}, dot_product);
            return rtn;
        }

        /*!
         * Compute the vector cross product.
         *
         * Cross product of this with another vvec \a v (if N==3). In
         * higher dimensions, its more complicated to define what the cross product is,
         * and I'm unlikely to need anything other than the plain old 3D cross product.
         */
        template<typename Sy=S>
        vvec<S, Al> cross (const vvec<Sy>& v) const
        {
            vvec<S, Al> vrtn;
            if (this->size() == 3 && v.size() == 3) {
                vrtn.resize(3);
                vrtn[0] = (*this)[1] * v.z() - (*this)[2] * v.y();
                vrtn[1] = (*this)[2] * v.x() - (*this)[0] * v.z();
                vrtn[2] = (*this)[0] * v.y() - (*this)[1] * v.x();
            } else {
                throw std::runtime_error ("vvec::cross(): Cross product is defined here for 3 dimensions only");
            }
            return vrtn;
        }

        /*!
         * Scalar/fixed-size vec multiply * operator
         *
         * This function will only be defined if typename Sy is a
         * scalar type or a fixed size vector. Multiplies this vvec<S> by s, element-wise.
         */
        template <typename Sy=S, std::enable_if_t<std::is_scalar<std::decay_t<Sy>>::value || morph::is_copyable_fixedsize<std::decay_t<Sy>>::value, int> = 0 >
        vvec<S> operator* (const Sy& s) const
        {
            vvec<S> rtn(this->size());
            auto mult_by_s = [s](S elmnt) -> S { return elmnt * s; };
            std::transform (this->begin(), this->end(), rtn.begin(), mult_by_s);
            return rtn;
        }

        /*!
         * operator* gives the Hadamard product.
         *
         * Hadamard product - elementwise multiplication. If the vectors are of
         * differing lengths, then an exception is thrown.
         *
         * \return Hadamard product of left hand size (*this) and right hand size (\a v)
         */
        template<typename Sy=S>
        vvec<S, Al> operator* (const vvec<Sy>& v) const
        {
            if (v.size() != this->size()) {
                throw std::runtime_error ("vvec::operator*: Hadamard product is defined here for vectors of same dimensionality only");
            }
            vvec<S, Al> rtn(this->size(), S{0});
            auto vi = v.begin();
            // Visual Studio may complain about there being no static_cast<S> of (*vi++), here
            auto mult_by_s = [vi](S lhs) mutable -> S { return lhs * (*vi++); };
            std::transform (this->begin(), this->end(), rtn.begin(), mult_by_s);
            return rtn;
        }

        /*!
         * Scalar/fixed-size vec multiply *= operator
         *
         * This function will only be defined if typename Sy is a
         * scalar type or a fixed size vec. Multiplies this vvec<S> by s, element-wise.
         */
        template <typename Sy=S, std::enable_if_t<std::is_scalar<std::decay_t<Sy>>::value || morph::is_copyable_fixedsize<std::decay_t<Sy>>::value, int> = 0 >
        void operator*= (const Sy& s) noexcept
        {
            auto mult_by_s = [s](S elmnt) -> S { return elmnt * s; };
            std::transform (this->begin(), this->end(), this->begin(), mult_by_s);
        }

        /*!
         * vvec multiply *= operator.
         *
         * Hadamard product. Multiply *this vector with \a v, elementwise. If \a v has a
         * different number of elements to *this, then an exception is thrown.
         */
        template <typename Sy=S>
        void operator*= (const vvec<Sy>& v)
        {
            if (v.size() == this->size()) {
                auto vi = v.begin();
                auto mult_by_s = [vi](S lhs) mutable -> S { return lhs * (*vi++); };
                std::transform (this->begin(), this->end(), this->begin(), mult_by_s);
            } else {
                throw std::runtime_error ("vvec::operator*=: Hadamard product is defined here for vectors of same dimensionality only");
            }
        }

        //! Scalar/fixed size vec divide by s
        template <typename Sy=S, std::enable_if_t<std::is_scalar<std::decay_t<Sy>>::value || morph::is_copyable_fixedsize<std::decay_t<Sy>>::value, int> = 0 >
        vvec<S> operator/ (const Sy& s) const noexcept
        {
            vvec<S> rtn(this->size());
            auto div_by_s = [s](S elmnt) -> S { return elmnt / s; };
            std::transform (this->begin(), this->end(), rtn.begin(), div_by_s);
            return rtn;
        }

        /*!
         * operator/ gives element by element division
         *
         * 'Hadamard' division - elementwise division. If the vectors are of
         * differing lengths, then an exception is thrown.
         *
         * \return Hadamard division of left hand size (*this) by right hand size (\a v)
         */
        template<typename Sy=S>
        vvec<S, Al> operator/ (const vvec<Sy>& v) const
        {
            if (v.size() != this->size()) {
                throw std::runtime_error ("vvec::operator/: Hadamard division is defined here for vectors of same dimensionality only");
            }
            vvec<S, Al> rtn(this->size(), S{0});
            auto vi = v.begin();
            auto div_by_s = [vi](S lhs) mutable -> S { return lhs / (*vi++); };
            std::transform (this->begin(), this->end(), rtn.begin(), div_by_s);
            return rtn;
        }

        //! Scalar divide/fixed size vec by s
        template <typename Sy=S, std::enable_if_t<std::is_scalar<std::decay_t<Sy>>::value || morph::is_copyable_fixedsize<std::decay_t<Sy>>::value, int> = 0 >
        void operator/= (const Sy& s)
        {
            auto div_by_s = [s](S elmnt) -> S { return elmnt / s; };
            std::transform (this->begin(), this->end(), this->begin(), div_by_s);
        }

        /*!
         * vvec division /= operator.
         *
         * Hadamard division. Divide *this vector by \a v, elementwise. If \a v has a
         * different number of elements to *this, then an exception is thrown.
         */
        template <typename Sy=S>
        void operator/= (const vvec<Sy>& v)
        {
            if (v.size() == this->size()) {
                auto vi = v.begin();
                auto div_by_s = [vi](S lhs) mutable -> S { return lhs / (*vi++); };
                std::transform (this->begin(), this->end(), this->begin(), div_by_s);
            } else {
                throw std::runtime_error ("vvec::operator/=: Hadamard division is defined here for vectors of same dimensionality only");
            }
        }

        //! Scalar addition with a thing that is of a different type to S (but must be scalar or fixed size vec/array)
        template <typename Sy=S, std::enable_if_t<std::is_scalar<std::decay_t<Sy>>::value || morph::is_copyable_fixedsize<std::decay_t<Sy>>::value, int> = 0 >
        vvec<S> operator+ (const Sy& s) const
        {
            vvec<S> rtn(this->size());
            auto add_s = [s](S elmnt) -> S { return elmnt + s; };
            std::transform (this->begin(), this->end(), rtn.begin(), add_s);
            return rtn;
        }

        //! Addition strictly of something of type S which should work for any type S that implements the + operator
        vvec<S> operator+ (const S& s) const
        {
            vvec<S> rtn(this->size());
            auto add_s = [s](S elmnt) -> S { return elmnt + s; };
            std::transform (this->begin(), this->end(), rtn.begin(), add_s);
            return rtn;
        }

        //! vvec addition operator
        template<typename Sy=S>
        vvec<S> operator+ (const vvec<Sy>& v) const
        {
            if (v.size() != this->size()) {
                throw std::runtime_error ("vvec::operator+: adding vvecs of different dimensionality is suppressed");
            }
            vvec<S> vrtn(this->size());
            auto vi = v.begin();
            // Static cast is encouraged by Visual Studio, but it prevents addition of vvec of vecs and vvec of scalars
            auto add_v = [vi](S a) mutable -> S { return a + /* static_cast<S> */(*vi++); };
            std::transform (this->begin(), this->end(), vrtn.begin(), add_v);
            return vrtn;
        }

        //! Scalar addition
        template <typename Sy=S, std::enable_if_t<std::is_scalar<std::decay_t<Sy>>::value || morph::is_copyable_fixedsize<std::decay_t<Sy>>::value, int> = 0 >
        void operator+= (const Sy& s) noexcept
        {
            auto add_s = [s](S elmnt) -> S { return elmnt + s; };
            std::transform (this->begin(), this->end(), this->begin(), add_s);
        }

        //! vvec addition operator
        template<typename Sy=S>
        void operator+= (const vvec<Sy>& v)
        {
            if (v.size() == this->size()) {
                auto vi = v.begin();
                auto add_v = [vi](S a) mutable -> S { return a + /* static_cast<S> */(*vi++); };
                std::transform (this->begin(), this->end(), this->begin(), add_v);
            } else {
                throw std::runtime_error ("vvec::operator+=: adding vvecs of different dimensionality is suppressed");
            }
        }

        //! Addition += operator for any type same as the enclosed type that implements + op
        void operator+= (const S& s) const noexcept
        {
            auto add_s = [s](S elmnt) -> S { return elmnt + s; };
            std::transform (this->begin(), this->end(), this->begin(), add_s);
        }

        //! Scalar subtraction with a thing that is of a different type to S (but must be scalar or fixed size vec/array)
        template <typename Sy=S, std::enable_if_t<std::is_scalar<std::decay_t<Sy>>::value || morph::is_copyable_fixedsize<std::decay_t<Sy>>::value, int> = 0 >
        vvec<S> operator- (const Sy& s) const
        {
            vvec<S> rtn(this->size());
            auto subtract_s = [s](S elmnt) -> S { return elmnt - s; };
            std::transform (this->begin(), this->end(), rtn.begin(), subtract_s);
            return rtn;
        }

        //! Subtraction which should work for any strictly member type (i.e. S) that implements the - operator
        vvec<S> operator- (const S& s) const
        {
            vvec<S> rtn(this->size());
            auto subtract_s = [s](S elmnt) -> S { return elmnt - s; };
            std::transform (this->begin(), this->end(), rtn.begin(), subtract_s);
            return rtn;
        }

        //! A vvec subtraction operator
        template<typename Sy=S>
        vvec<S> operator- (const vvec<Sy>& v) const
        {
            if (v.size() != this->size()) {
                throw std::runtime_error ("vvec::operator-: subtracting vvecs of different dimensionality is suppressed");
            }
            vvec<S> vrtn(this->size());
            auto vi = v.begin();
            auto subtract_v = [vi](S a) mutable -> S { return a - (*vi++); };
            std::transform (this->begin(), this->end(), vrtn.begin(), subtract_v);
            return vrtn;
        }

        //! Scalar subtraction
        template <typename Sy=S, std::enable_if_t<std::is_scalar<std::decay_t<Sy>>::value || morph::is_copyable_fixedsize<std::decay_t<Sy>>::value, int> = 0 >
        void operator-= (const Sy& s) noexcept
        {
            auto subtract_s = [s](S elmnt) -> S { return elmnt - s; };
            std::transform (this->begin(), this->end(), this->begin(), subtract_s);
        }

        //! A vvec subtraction operator
        template<typename Sy=S>
        void operator-= (const vvec<Sy>& v)
        {
            if (v.size() == this->size()) {
                auto vi = v.begin();
                auto subtract_v = [vi](S a) mutable -> S { return a - (*vi++); };
                std::transform (this->begin(), this->end(), this->begin(), subtract_v);
            } else {
                throw std::runtime_error ("vvec::operator-=: subtracting vvecs of different dimensionality is suppressed");
            }
        }

        //! Subtraction -= operator for any time same as the enclosed type that implements - op
        void operator-= (const S& s) const noexcept
        {
            auto subtract_s = [s](S elmnt) { return elmnt - s; };
            std::transform (this->begin(), this->end(), this->begin(), subtract_s);
        }

        //! Concatentate the vvec<S>& a to the end of *this.
        void concat (const vvec<S>& a)
        {
            std::size_t sz = this->size();
            this->resize (sz + a.size());
            auto iter = this->begin();
            std::advance (iter, sz);
            std::copy (a.begin(), a.end(), iter);
        }

        //! Overload the stream output operator
        friend std::ostream& operator<< <S> (std::ostream& os, const vvec<S>& v);
    };

    template <typename S=float, typename Al=std::allocator<S>>
    std::ostream& operator<< (std::ostream& os, const vvec<S, Al>& v)
    {
        os << v.str();
        return os;
    }

    // Operators that can do premultiply, predivide by scaler so you could do,
    // e.g. vvec<float> result = float(1) / vvec<float>({1,2,3});

    //! Scalar * vvec<> (commutative; lhs * rhs == rhs * lhs, so return rhs * lhs)
    template <typename T, typename S> requires std::is_arithmetic_v<T>
    vvec<S> operator* (T lhs, const vvec<S>& rhs) { return rhs * lhs; }

    //! Scalar / vvec<>
    template <typename T, typename S> requires std::is_arithmetic_v<T>
    vvec<S> operator/ (T lhs, const vvec<S>& rhs)
    {
        vvec<S> division(rhs.size(), S{0});
        auto lhs_div_by_vec = [lhs](S elmnt) { return static_cast<S>(lhs / elmnt); };
        std::transform (rhs.begin(), rhs.end(), division.begin(), lhs_div_by_vec);
        return division;
    }

    //! Scalar + vvec<> (commutative)
    template <typename T, typename S> requires std::is_arithmetic_v<T>
    vvec<S> operator+ (T lhs, const vvec<S>& rhs) { return rhs + lhs; }

    //! Scalar - vvec<>
    template <typename T, typename S> requires std::is_arithmetic_v<T>
    vvec<S> operator- (T lhs, const vvec<S>& rhs)
    {
        vvec<S> subtraction(rhs.size(), S{0});
        auto lhs_minus_vec = [lhs](S elmnt) { return static_cast<S>(lhs - elmnt); };
        std::transform (rhs.begin(), rhs.end(), subtraction.begin(), lhs_minus_vec);
        return subtraction;
    }

} // namespace morph
