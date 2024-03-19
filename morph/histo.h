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

    template <typename T=float>
    struct histo
    {
        template < template <typename, typename> typename Container,
                   typename Allocator=std::allocator<T> >
        histo (const Container<T, Allocator>& data, std::size_t n)
        {
            this->bins.resize (n, T{0});
            this->binedges.resize (n + 1U, T{0});
            this->counts.resize (n, T{0});
            this->proportions.resize (n, T{0});
            this->datacount = static_cast<T>(data.size());

            // Compute bin widths from range of data and n.
            morph::range<T> maximini = morph::MathAlgo::maxmin (data);
            this->max = maximini.max;
            this->min = maximini.min;
            this->range = max - min;
            this->binwidth = this->range / static_cast<T>(n);
            for (std::size_t i = 0; i < n; ++i) {
                // bins[i] = min + i*bw + bw/2 but do the additions after the loop
                this->bins[i] = i * this->binwidth;
                this->binedges[i + 1U] = (i + 1U) * this->binwidth;
            }
            this->bins += (this->min + (this->binwidth/T{2}));
            this->binedges += this->min;

            // Compute counts
            for (auto datum : data) {
                int idx = static_cast<int>(std::floor(((datum - this->min)/this->range)*n));
                if (idx > -1) {
                    counts[idx] += T{1};
                }
            }
            this->proportions = counts/this->datacount;
        }

        T min = T{0}; // min value in data
        T max = T{0}; // max value in data
        T range = T{0}; // range of data
        T datacount = T{0}; // how many elements were there in data?
        T binwidth = T{0}; // Width of each bin
        morph::vvec<T> bins; // centres of bins
        morph::vvec<T> binedges;
        morph::vvec<T> counts;
        morph::vvec<T> proportions;
    };
}
