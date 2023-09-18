/*
 * A histogram class
 */
#pragma once

#include <morph/vec.h>
#include <morph/vvec.h>
#include <morph/MathAlgo.h>
#include <memory>

namespace morph {

    template <typename T=float>
    struct histo
    {
        template < template <typename, typename> typename Container,
                   typename Allocator=std::allocator<T> >
        histo (const Container<T, Allocator>& data, size_t n)
        {
            this->bins.resize(n, T{0});
            this->binedges.resize(n+1, T{0});
            this->counts.resize(n, 0);
            this->proportions.resize(n, T{0});
            this->datacount = static_cast<T>(data.size());

            // Compute bin widths from range of data and n.
            morph::vec<T, 2> maximini = morph::MathAlgo::maxmin (data);
            this->max = maximini[0];
            this->min = maximini[1];
            this->range = max - min;
            this->binwidth = this->range / n;
            for (size_t i = 0; i < n; ++i) {
                // bins[i] = min + i*bw + bw/2 but do the additions after the loop
                this->bins[i] = i * this->binwidth;
                this->binedges[i+1] = (i+1) * this->binwidth;
            }
            this->bins += (this->min + (this->binwidth/T{2}));
            this->binedges += this->min;

            // Compute counts
            for (auto datum : data) {
                size_t idx = std::floor(((datum - this->min)/this->range)*n);
                counts[idx] += T{1};
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
