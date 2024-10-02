#pragma once

/*
 * A tiny container class to hold the min and max of a range. I wanted a common type in
 * which to return minmax values for use in morph::vec and morph::vvec. An option would
 * have been an std::array, but I prefer this, as vvec doesn't otherwise need to include
 * <array>. range can be used in constexpr functions.
 *
 * Author: Seb James
 * Date: Sept. 2023.
 */

#include <ostream>
#include <limits>

namespace morph {

    // Forward declare the class and stream operator
    template <typename T> struct range;
    template <typename T> std::ostream& operator<< (std::ostream&, const range<T>&);

    // range is a constexpr-friendly literal type
    template <typename T>
    struct range
    {
        constexpr range() {}
        constexpr range (const T& _min, const T& _max) : min(_min), max(_max) {}

        // The minimum
        T min = T{0};
        // The maximum
        T max = T{0};

        // Set the range to _min, _max
        constexpr void set (const T& _min, const T& _max)
        {
            this->min = _min;
            this->max = _max;
        }

        // Initialise the range to participate in a search for the max and min through a range of data.
        //
        // Range can then be part of a loop through data with code like:
        //
        // morph::vvec<T> data;
        // data.randomize();
        // range<T> r;
        // r.search_init();
        // for (auto d : data) { r.update (d); }
        // std::cout << "The range of values in data was: " << r << std::endl;
        constexpr void search_init()
        {
            this->min = std::numeric_limits<T>::max();
            this->max = std::numeric_limits<T>::lowest();
        }

        // Extend the range to include the given datum
        constexpr void update (const T& d)
        {
            this->min = d < this->min ? d : this->min;
            this->max = d > this->max ? d : this->max;
        }

        // Does the range include v?
        constexpr bool includes (const T& v) { return (v <= this->max && v >= this->min); }

        // What's the 'span of the range'?
        constexpr T span() const { return this->max - this->min; }

        // Overload the stream output operator
        friend std::ostream& operator<< <T> (std::ostream& os, const range<T>& r);
    };

    // Output a string with notation "[min, max]" to indicate a closed interval
    template <typename T>
    std::ostream& operator<< (std::ostream& os, const range<T>& r)
    {
        os << "[" << r.min << ", " << r.max << "]";
        return os;
    }

} // namespace morph
