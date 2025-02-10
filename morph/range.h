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
#include <sstream>
#include <limits>
#include <complex>
#include <morph/trait_tests.h>

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

        // Output a string representation of the min and max. Rewrite with <format> at some point.
        std::string str() const
        {
            std::stringstream ss;
            ss << "[" << this->min << ", " << this->max << "]";
            return ss.str();
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
            if constexpr (morph::number_type<T>::value == 2) { // range is complex
                this->min = { std::numeric_limits<typename T::value_type>::max(), std::numeric_limits<typename T::value_type>::max() };
                this->max = { std::numeric_limits<typename T::value_type>::lowest(), std::numeric_limits<typename T::value_type>::lowest() };
            } else if constexpr (morph::number_type<T>::value == 1) { // range is scalar
                this->min = std::numeric_limits<T>::max();
                this->max = std::numeric_limits<T>::lowest();
            } else {
#if __cplusplus >= 202002L
                []<bool flag = false>() { static_assert(flag, "morph::range::search_init does not support this type"); }();
#else
                throw std::runtime_error ("morph::range::search_init does not support this type");
#endif
            }
        }

        // Extend the range to include the given datum. Return true if the range changed.
        constexpr bool update (const T& d)
        {
            bool changed = false;
            if constexpr (morph::number_type<T>::value == 2) { // range is complex
                // Does d 'extend the rectangle in the complex plane that defines the complex range'?
                this->min = std::real(d) < std::real(this->min) || std::imag(d) < std::imag(this->min) ? changed = true, d : this->min;
                this->max = std::real(d) > std::real(this->max) || std::imag(d) > std::imag(this->max) ? changed = true, d : this->max;
            } else if constexpr (morph::number_type<T>::value == 1) { // range is scalar
                this->min = d < this->min ? changed = true, d : this->min;
                this->max = d > this->max ? changed = true, d : this->max;
            } else {
#if __cplusplus >= 202002L
                []<bool flag = false>() { static_assert(flag, "morph::range::update does not support this type"); }();
#else
                throw std::runtime_error ("morph::range::update does not support this type");
#endif
            }
            return changed;
        }

        // Does the range include v?
        constexpr bool includes (const T& v) const
        {
            if constexpr (morph::number_type<T>::value == 2) { // range is complex
                // Is v inside the rectangle in the complex plane made by min and max?
                return (std::real(v) <= std::real(this->max) && std::real(v) >= std::real(this->min)
                        && std::imag(v) <= std::imag(this->max) && std::imag(v) >= std::imag(this->min));
            } else if constexpr (morph::number_type<T>::value == 1) { // range is scalar
                return (v <= this->max && v >= this->min);
            } else {
#if __cplusplus >= 202002L
                []<bool flag = false>() { static_assert(flag, "morph::range::includes does not support this type"); }();
#else
                throw std::runtime_error ("morph::range::includes does not support this type");
#endif
            }
        }

        // If the range other 'fits inside' this range, then return true.
        constexpr bool contains (const morph::range<T>& other) const
        {
            if constexpr (morph::number_type<T>::value == 2) { // range is complex
                // Does other define a rectangle in the complex plane that fits inside the one made by this->min and max?
                bool othermin_inside = std::real(this->min) <= std::real(other.min) && std::imag(this->min) <= std::imag(other.min)
                && std::real(this->max) >= std::real(other.min) && std::imag(this->max) >= std::imag(other.min);
                bool othermax_inside = std::real(this->min) <= std::real(other.max) && std::imag(this->min) <= std::imag(other.max)
                && std::real(this->max) >= std::real(other.max) && std::imag(this->max) >= std::imag(other.max);
                return othermin_inside && othermax_inside;
            } else if constexpr (morph::number_type<T>::value == 1) { // range is scalar
                return (this->min <= other.min && this->max >= other.max);
            } else {
#if __cplusplus >= 202002L
                []<bool flag = false>() { static_assert(flag, "morph::range::contains does not support this type"); }();
#else
                throw std::runtime_error ("morph::range::contains does not support this type");
#endif
            }
        }

        // What's the 'span of the range'? Whether scalar or complex (or vector), it's max - min
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
