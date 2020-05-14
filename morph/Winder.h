/*!
 * Compute the winding number of a boundary with respect to a given coordinate.
 */

#pragma once

#include <memory>
#include <cmath>
#include "morph/MathConst.h"
#include <type_traits>

namespace morph {

    const bool debug_mode = true;

    // Expression SFINAE approach to testing for x() and y() methods
    template<typename T>
    struct has_xy_methods
    {
    private:
	template<typename U> static auto test(int) -> decltype(std::declval<U>().x() == 1
                                                               && std::declval<U>().y() == 1, std::true_type());
	template<typename> static std::false_type test(...);
    public:
	static constexpr bool value = std::is_same<decltype(test<T>(0)),std::true_type>::value;
    };

#if 0
    // Expression SFINAE approach to testing for a-b
    template<typename T>
    struct has_subtraction
    {
    private:
	template<typename U> static auto test(int) -> decltype(std::declval<U>() - std::declval<U>()==std::declval<U>(), std::true_type());
	template<typename> static std::false_type test(...);
    public:
	static constexpr bool value = std::is_same<decltype(test<T>(0)),std::true_type>::value;
    };
#endif

    // Expression SFINAE approach to testing for resize() method
    template<typename T>
    struct has_resize_method
    {
    private:
	template<typename U> static auto test(int) -> decltype(std::declval<U>().resize() == 1, std::true_type());
	template<typename> static std::false_type test(...);
    public:
	static constexpr bool value = std::is_same<decltype(test<T>(0)),std::true_type>::value;
    };

    // Expression SFINAE approach to testing for x and y member attributes
    template<typename T>
    struct has_xy_members
    {
    private:
	template<typename U> static auto test(int) -> decltype(std::declval<U>().x == 1
                                                               && std::declval<U>().y == 1, std::true_type());
	template<typename> static std::false_type test(...);
    public:
	static constexpr bool value = std::is_same<decltype(test<T>(0)),std::true_type>::value;
    };

    // Expression SFINAE approach to testing for first and second member attributes
    template<typename T>
    struct has_firstsecond_members
    {
    private:
	template<typename U> static auto test(int) -> decltype(std::declval<U>().first == 1
                                                               && std::declval<U>().second == 1, std::true_type());
	template<typename> static std::false_type test(...);
    public:
	static constexpr bool value = std::is_same<decltype(test<T>(0)), std::true_type>::value;
    };

    // Expression SFINAE approach to testing for ability to access like an array (i.e. std::array, morph::Vector)
    template<typename T>
    struct array_access_possible
    {
    private:
	template<typename U> static auto test(int) -> decltype(std::declval<U>()[0] == 1, std::true_type());
	template<typename> static std::false_type test(...);
    public:
	static constexpr bool value = std::is_same<decltype(test<T>(0)),std::true_type>::value;
    };

    /*!
     * A winding number class
     *
     * \tparam T the (2D) coordinate type (this might be cv::Point)
     *
     * \tparam Container Something like an std::vector, std::list or std::array,
     * containing a path of points.
     */
    template < typename T,
               template <typename, typename> typename Container,
               typename TT=T,
               typename Alloc=std::allocator<TT> >
    class Winder
    {
    public:
        //! Construct with the boundary reference.
        Winder (const Container<T, Alloc>& _boundary)
            : boundary(_boundary) {}


        //! Compute the winding number of the coordinate px with respect to the boundary.
        int wind (const T& px) {
            this->reset();
            for (auto bp : this->boundary) {
                this->wind (px, bp);
            }
            // Do first pixel again to complete the winding:
            T firstpoint = this->boundary.front();
            this->wind (px, firstpoint);
            double winding_no_d = std::round (this->angle_sum/morph::TWO_PI_D);
            std::cout << "winding_no: " << winding_no_d << std::endl;
            int winding_no = static_cast<int>(winding_no_d);
            return winding_no;
        }

    private:
        //template <typename _T=T, std::enable_if_t<std::is_object<std::decay_t<typename _T::x>>::value, int> = 1 >
        void wind (const T& px, const T& bp) {

            // Get angle from px to bp.

            T pt;
            if constexpr (array_access_possible<T>::value == true) {
                if constexpr (has_resize<T>::value == true) {
                    pt.resize(2);
                }
                pt[0] = bp[0] - px[0];
                pt[1] = bp[1] - px[1];
                std::cout << "pt is " << pt[0]<< "," << pt[1]<< std::endl;
            } else if constexpr (has_firstsecond_members<T>::value == true) {
                pt.first = bp.first - px.first;
                pt.second = bp.second - px.second;
            } else {
                pt = bp - px;
            }
            double raw_angle = 0.0;

            if constexpr (has_xy_methods<T>::value == true) {
                std::cout << "It's a type with a x() and y() functions\n";
                raw_angle = std::atan2 (pt.y(), pt.x());
            } else if constexpr (has_xy_members<T>::value == true) {
                std::cout << "It's a type with x and y member attributes\n";
                raw_angle = std::atan2 (pt.y, pt.x);
            } else if constexpr (has_firstsecond_members<T>::value == true) {
                std::cout << "It's a type with first and second member attributes\n";
                raw_angle = std::atan2 (pt.second, pt.first);
            } else if constexpr (array_access_possible<T>::value == true) {
                std::cout << "It's a type with array access\n";
                raw_angle = std::atan2 (pt[1], pt[0]);
            } else {
                std::cout << "Maybe it's a type that behaves like an array\n";
                raw_angle = std::atan2 (pt[1], pt[0]);
            }
            std::cout << "3\n";
            this->wind (raw_angle);
        }

        //! Update this->angle, this->angle_last and this->angle_sum based on \a raw_angle
        void wind (const double& raw_angle) {

            // Convert -pi -> 0 -> +pi range of atan2 to 0->2pi:
            this->angle = raw_angle >= 0 ? raw_angle : (morph::TWO_PI_D + raw_angle);

            if constexpr (morph::debug_mode == true) {
                std::cout << "angle: " << this->angle << "\n";
            }

            // Set the initial angle.
            if (this->angle_last == double{-100.0}) {
                this->angle_last = angle;
            }

            double delta = double{0.0}; // delta is 'angle change'
            if (this->angle == double{0.0}) {
                // Special treatment
                if (this->angle_last > morph::PI_D) {
                    // Clockwise to 0
                    delta = (morph::TWO_PI_D - this->angle_last);
                } else if (this->angle_last < morph::PI_D) {
                    // Anti-clockwise to 0
                    delta = -this->angle_last;
                } else { //angle_last must have been 0.0
                    delta = double{0.0};
                }

            } else {

                // Special treatment required ALSO if we crossed the 0 line without being on it.
                if (this->angle_last > morph::PI_D && this->angle < morph::PI_D) {
                    // crossed from 2pi side to 0 side: Clockwise
                    delta = this->angle + (morph::TWO_PI_D - this->angle_last);
                } else if (this->angle_last < morph::PI_OVER_2_D && this->angle > morph::PI_x3_OVER_2_D) {
                    // crossed from 0 side to 2pi side: Anti-clockwise
                    delta = - this->angle_last - (morph::TWO_PI_D - this->angle);
                } else { // Both are > pi or both are < pi.
                    delta = (this->angle - this->angle_last);
                }
            }
            this->angle_last = this->angle;
            this->angle_sum += delta;
        }

        //! Reset the angle member variables
        void reset() {
            this->angle_last = double{-100.0};
            this->angle = double{0.0};
            this->angle_sum = double{0.0};
        }

        //! Member reference to the boundary
        const Container<T, Alloc>& boundary;
        //! Current angle around a point
        double angle;
        //! The sum of angles
        double angle_sum;
        //! The angle of the last boundary point
        double angle_last;
    };

} // namespace morph
