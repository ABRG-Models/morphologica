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
     * \tparam H The type of the data from which to make the histogram
     * \tparam T The floating point type for proportions, etc
     */
    template <typename H=float, typename T=float>
    struct histo
    {
        template < template <typename, typename> typename Container,
                   typename Allocator=std::allocator<H> >
        histo (const Container<H, Allocator>& data, std::size_t n)
        {
            this->bins.resize (n, T{0});
            this->binedges.resize (n + 1U, T{0});
            this->counts.resize (n, 0u);
            this->proportions.resize (n, T{0});
            this->datacount = static_cast<T>(data.size());
            // Compute bin widths from range of data and n.
            morph::range<H> maximini = morph::MathAlgo::maxmin (data);
            this->max = maximini.max;
            this->min = maximini.min;
            this->range = max - min;
            if (this->range == H{0}) { throw std::runtime_error ("morph::histo: range == 0, can't make a histogram"); }
            this->binwidth = static_cast<T>(this->range) / static_cast<T>(n);
            for (std::size_t i = 0; i < n; ++i) {
                // bins[i] = min + i*bw + bw/2 but do the additions after the loop
                this->bins[i] = i * this->binwidth;
                this->binedges[i + 1U] = (i + 1U) * this->binwidth;
            }
            this->bins += (this->min + (this->binwidth/T{2}));
            this->binedges += this->min;

            // Compute counts
            for (auto datum : data) {
                T bin_proportion = static_cast<T>(datum - this->min) / static_cast<T>(this->range);
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

        H min = H{0}; // min value in data
        H max = H{0}; // max value in data
        H range = H{0}; // range of data

        std::size_t datacount = 0u; // how many elements were there in data?
        T binwidth = T{0}; // Width of each bin
        morph::vvec<T> bins; // centres of bins
        morph::vvec<T> binedges;
        morph::vvec<std::size_t> counts;
        morph::vvec<T> proportions;
    };
}
