#pragma once

/*
 * A tiny container class to hold the min and max of a range. I wanted a common type in
 * which to return minmax values for use in morph::vec and morph::vvec. An option would
 * have been an std::array, but I prefer this, as vvec doesn't otherwise need to include
 * <array>.
 *
 * Author: Seb James
 * Date: Sept. 2023.
 */

#include <string>
#include <sstream>
#include <ostream>

namespace morph {

    // Forward declare class and stream operator
    template <typename T> class range;
    template <typename T> std::ostream& operator<< (std::ostream&, const range<T>&);

    template <typename T>
    struct range
    {
        range() {}
        range (const T& _min, const T& _max) : min(_min), max(_max) {}

        // The minimum
        T min = T{0};
        // The maximum
        T max = T{0};

        // Does the range include v?
        bool includes (const T& v) { return (v <= this->max && v >= this->min); }

        // Output a string with notation "[min, max]" to indicate a closed interval
        std::string str() const
        {
            std::stringstream ss;
            ss << "[" << this->min << ", " << this->max << "]";
            return ss.str();
        }

        // Overload the stream output operator
        friend std::ostream& operator<< <T> (std::ostream& os, const range<T>& r);
    };

    template <typename T>
    std::ostream& operator<< (std::ostream& os, const range<T>& r)
    {
        os << r.str();
        return os;
    }

} // namespace morph
