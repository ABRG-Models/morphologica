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
#include <morph/constexpr_math.h>

namespace morph {

    // Different values to use to initialize a range object with
    enum class range_init { zeros, for_search };

    // Forward declare the class and stream operator
    template <typename T> struct range;
    template <typename T> std::ostream& operator<< (std::ostream&, const range<T>&);

    // range is a constexpr-friendly literal type defining a closed interval [min, max]
    template <typename T>
    struct range
    {
        // The minimum value in the closed interval
        T min = T{0};
        // The maximum value
        T max = T{0};

        // In the default constructor, min == max == T{0}
        constexpr range() noexcept {}
        // Range constructor in which you can specify that the range should be initialized for search
        constexpr range (const morph::range_init _range_init) noexcept
        {
            if (_range_init == morph::range_init::for_search) { this->search_init(); }
        }
        // Range constructor taking the min and max for a ready-to-go range
        constexpr range (const T& _min, const T& _max) noexcept : min(_min), max(_max) {}

        // Set the range to _min, _max
        constexpr void set (const T& _min, const T& _max) noexcept
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

        template<typename Ty=T>
        requires std::is_floating_point_v<Ty>
        constexpr bool operator== (const range<Ty>& rhs) const noexcept
        {
            return (morph::math::abs(this->min - static_cast<T>(rhs.min)) < std::numeric_limits<T>::epsilon()
                    && morph::math::abs(this->max - static_cast<T>(rhs.max)) < std::numeric_limits<T>::epsilon());
        }

        template<typename Ty=T>
        requires std::is_floating_point_v<Ty>
        constexpr bool operator!= (const range<Ty>& rhs) const noexcept
        {
            return (morph::math::abs(this->min - static_cast<T>(rhs.min)) > std::numeric_limits<T>::epsilon()
                    || morph::math::abs(this->max - static_cast<T>(rhs.max)) > std::numeric_limits<T>::epsilon());
        }

        template<typename Ty=T>
        requires std::is_integral_v<Ty>
        constexpr bool operator== (const range<Ty>& rhs) const noexcept
        {
            return (this->min == static_cast<T>(rhs.min) && this->max == static_cast<T>(rhs.max));
        }

        template<typename Ty=T>
        requires std::is_integral_v<Ty>
        constexpr bool operator!= (const range<Ty>& rhs) const noexcept
        {
            return (this->min != static_cast<T>(rhs.min) || this->max != static_cast<T>(rhs.max));
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
        constexpr void search_init() noexcept
        {
            if constexpr (morph::number_type<T>::value == 2) { // range is complex
                this->min = { std::numeric_limits<typename T::value_type>::max(), std::numeric_limits<typename T::value_type>::max() };
                this->max = { std::numeric_limits<typename T::value_type>::lowest(), std::numeric_limits<typename T::value_type>::lowest() };
            } else if constexpr (morph::number_type<T>::value == 1) { // range is scalar
                this->min = std::numeric_limits<T>::max();
                this->max = std::numeric_limits<T>::lowest();
            } else {
                []<bool flag = false>() { static_assert(flag, "morph::range::search_init does not support this type"); }();
            }
        }

        // Extend the range to include the given datum. Return true if the range changed.
        constexpr bool update (const T& d) noexcept
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
                []<bool flag = false>() { static_assert(flag, "morph::range::update does not support this type"); }();
            }
            return changed;
        }

        // Does the range include the value v?
        constexpr bool includes (const T& v) const noexcept
        {
            if constexpr (morph::number_type<T>::value == 2) { // range is complex
                // Is v inside the rectangle in the complex plane made by min and max?
                return (std::real(v) <= std::real(this->max) && std::real(v) >= std::real(this->min)
                        && std::imag(v) <= std::imag(this->max) && std::imag(v) >= std::imag(this->min));
            } else if constexpr (morph::number_type<T>::value == 1) { // range is scalar
                return (v <= this->max && v >= this->min);
            } else {
                []<bool flag = false>() { static_assert(flag, "morph::range::includes does not support this type"); }();
            }
        }

        // If the range other 'fits inside' this range, then this range contains (or encompasses) the range other.
        constexpr bool contains (const morph::range<T>& other) const noexcept
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
                []<bool flag = false>() { static_assert(flag, "morph::range::contains does not support this type"); }();
            }
        }

        // What's the 'span of the range'? Whether scalar or complex (or vector), it's max - min
        constexpr T span() const noexcept { return this->max - this->min; }

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
