/*
 * A histogram class
 */
#pragma once

#include <morph/vec.h>
#include <morph/vvec.h>
#include <morph/range.h>
#include <morph/MathAlgo.h>
#include <memory>

namespace morph {

    /*!
     * A histogram class. Construct with data of type H and access bin locations
     *
     * \tparam H The type of the data from which to make the histogram. May be a floating point or
     * integer type.
     *
     * \tparam T The floating point type for proportions, etc. Must be a floating point type.
     */
    template <typename H=float, typename T=float>
    struct histo
    {
        /*!
         * Histogram constructor
         *
         * This constructor does all of the computation of the histogram (via a call to init). The
         * workflow is Construct -> access results in histo::bins histo::binwidth histo::proportions
         * and histo::counts.
         *
         * The histogram is computed based on the range of data values in data.
         *
         * \param data The histogram data
         *
         * \param n The number of bins to sort the data values into
         *
         * \tparam Container A container (std::vector, std::array, morph::vvec, etc) of data values
         * of type H
         *
         * \tparam Allocator The memory allocator for the container. You don't usually need to
         * specify this.
         */
        template < template <typename, typename> typename Container, typename Allocator=std::allocator<H> >
        histo (const Container<H, Allocator>& data, std::size_t n)
        {
            morph::range<H> r { std::numeric_limits<H>::max(), std::numeric_limits<H>::max() };
            this->init (data, n, r);
        }

        /*!
         * Histogram constructor for a manual data range
         *
         * This constructor does almost all of the computation of the histogram (via a call to
         * init). The workflow is Construct -> access results in histo::bins histo::binwidth
         * histo::proportions and histo::counts.
         *
         * The histogram is computed based on the range of data values provided by the user in
         * manual_datarange. manual_datarange should encompass the actual range of the data.
         *
         * \param data The histogram data
         *
         * \param n The number of bins to sort the data values into
         *
         * \param manual_datarange The user-provided data range.
         *
         * \tparam Container A container (std::vector, std::array, morph::vvec, etc) of data values
         * of type H
         *
         * \tparam Allocator The memory allocator for the container. You don't usually need to
         * specify this.
         */
        template < template <typename, typename> typename Container, typename Allocator=std::allocator<H> >
        histo (const Container<H, Allocator>& data, std::size_t n, const morph::range<H>& manual_datarange)
        {
            this->init (data, n, manual_datarange);
        }

        /*!
         * Histogram computation common to both constructors
         *
         * This function computes the histogram.
         *
         * \param data The histogram data
         *
         * \param n The number of bins to sort the data values into
         *
         * \param manual_datarange The user-provided data range or a dummy datarange containing the
         * value std::numeric_limits<H>::max() for both min and max.
         *
         * \tparam Container A container (std::vector, std::array, morph::vvec, etc) of data values
         * of type H
         *
         * \tparam Allocator The memory allocator for the container. You don't usually need to
         * specify this.
         */
        template < template <typename, typename> typename Container, typename Allocator=std::allocator<H> >
        void init (const Container<H, Allocator>& data, std::size_t n, const morph::range<H>& manual_datarange)
        {
            this->bins.resize (n, T{0});
            this->binedges.resize (n + 1U, T{0});
            this->counts.resize (n, 0u);
            this->proportions.resize (n, T{0});
            this->datacount = static_cast<T>(data.size());
            // Compute bin widths from range of data and n.
            if (manual_datarange.min == std::numeric_limits<H>::max()
                && manual_datarange.max == std::numeric_limits<H>::max()) {
                this->datarange = morph::MathAlgo::maxmin (data);
            } else {
                // Check manual_datarange
                morph::range<H> actual_datarange = morph::MathAlgo::maxmin (data);
                if (!manual_datarange.includes (actual_datarange)) {
                    throw std::runtime_error ("morph::histo: Make sure the manual_datarange is *larger* than the data's own datarange");
                }
                this->datarange = manual_datarange;
            }
            if (this->datarange.span() == H{0}) {
                throw std::runtime_error ("morph::histo: range span is 0, can't make a histogram");
            }
            T d_span = static_cast<T>(this->datarange.span());
            this->binwidth = d_span / static_cast<T>(n);
            for (std::size_t i = 0; i < n; ++i) {
                // bins[i] = min + i*bw + bw/2 but do the additions after the loop
                this->bins[i] = i * this->binwidth;
                this->binedges[i + 1U] = (i + 1U) * this->binwidth;
            }
            this->bins += (this->datarange.min + (this->binwidth/T{2}));
            this->binedges += this->datarange.min;

            // Compute counts
            for (auto datum : data) {
                T bin_proportion = static_cast<T>(datum - this->datarange.min) / d_span;
                if (std::abs(bin_proportion - T{1}) < std::numeric_limits<T>::epsilon()) {
                    // Edge case, right on t'limit. Place in last bin.
                    this->counts[n-1] += 1u;

                } else if (bin_proportion > T{1}) {
                    throw std::runtime_error ("morph::histo: shouldn't see proportion > 1");

                } else {
                    std::size_t idx = static_cast<std::size_t>(std::floor(bin_proportion * n));
                    this->counts[idx] += 1u;
                }
            }
            this->proportions = counts.as<T>()/this->datacount;
        }

        //! The max and min of the histogram data. Computed in constructor.
        morph::range<H> datarange;
        //! how many elements were there in data?
        std::size_t datacount = 0u;
        //! A computed width for each bin. Computed from the values that appear in the data
        //! (i.e. from the datarange)
        T binwidth = T{0};
        //! The location of the centres of the bins. Computed in terms of the binwidth. n elements
        //! (where n is a site_t passed to the constructor).
        morph::vvec<T> bins;
        //! The location of the edges of the bins. Computed in terms of the binwidth. n+1 elements.
        morph::vvec<T> binedges;
        //! The counts for each bin. n elements.
        morph::vvec<std::size_t> counts;
        //! The counts as proportions for each bin. n elements.
        morph::vvec<T> proportions;
    };
}
