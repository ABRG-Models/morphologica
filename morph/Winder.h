/*!
 * Compute the winding number of a boundary with respect to a given coordinate.
 */

#pragma once

#include <memory>
#include <cmath>
#include <stdexcept>
#include "morph/MathConst.h"
#include "morph/expression_sfinae.h"

namespace morph {

    const bool debug_mode = false;

    /*!
     * A winding number class
     *
     * \tparam T the (2D) coordinate type (this might be cv::Point, morph::BezCoord,
     * morph::vVector, morph::Vector, std::array or std::vector)
     *
     * \tparam Container Something like an std::vector, std::list or std::array,
     * containing a path of points. The template-template is not flexible enough for
     * Container to be std::map.
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
            if constexpr (debug_mode == true) { std::cout << "winding_no: " << winding_no_d << std::endl; }
            int winding_no = static_cast<int>(winding_no_d);
            return winding_no;
        }

    private:

        //! Convert two coordinate objects (whatever they may be) which define one
        //! vector (pt) into an angle (using std::atan2) of type double so that the
        //! method morph::Winder::wind (const double&) may be called.
        void wind (const T& px, const T& bp) {
            // Get angle from px to bp. First create pt, the vector pointing from px to
            // bp whose angle we will calculate.
            T pt;
            if constexpr (array_access_possible<T>::value == true) {
                // In this case, T is a vector or array like thing
                if constexpr (has_resize_method<T>::value == true) {
                    // If T has resize(), then we need to resize pt to have size 2
                    if constexpr (debug_mode == true) { std::cout << "RESIZE\n"; }
                    pt.resize(2);
                }
                if constexpr (has_subtraction<T>::value == true) {
                    if constexpr (debug_mode == true) { std::cout << "subtraction operator\n"; }
                    pt = bp - px;
                } else {
                    if constexpr (debug_mode == true) { std::cout << "subtracting array elements\n"; }
                    pt[0] = bp[0] - px[0];
                    pt[1] = bp[1] - px[1];
                }

            } else if constexpr (has_firstsecond_members<T>::value == true) {
                // In this case, T is presumably a std::pair
                pt.first = bp.first - px.first;
                pt.second = bp.second - px.second;

            } else {
                // cv::Point, morph::BezCoord should fall through here.
                if constexpr (has_subtraction<T>::value == true) {
                    if constexpr (debug_mode == true) { std::cout << "has subtraction\n"; }
                    pt = bp - px;
                } else if constexpr (has_xy_members<T>::value == true) {
                    pt.x = bp.x - px.x;
                    pt.y = bp.y - px.y;
                } else {
                    throw std::runtime_error ("Template code failed to find out how to subtract one coordinate from another.");
                }
            }

            // Now compute the angle of the vector pt using std::atan2. Multiple specializations here, too.
            double raw_angle = 0.0;
            if constexpr (has_xy_methods<T>::value == true) {
                if constexpr (debug_mode == true) { std::cout << "It's a type with x() and y() functions\n"; }
                raw_angle = std::atan2 (pt.y(), pt.x());
            } else if constexpr (has_xy_members<T>::value == true) {
                if constexpr (debug_mode == true) { std::cout << "It's a type with x and y member attributes\n"; }
                raw_angle = std::atan2 (pt.y, pt.x);
            } else if constexpr (has_firstsecond_members<T>::value == true) {
                if constexpr (debug_mode == true) { std::cout << "It's a type with first and second member attributes\n"; }
                raw_angle = std::atan2 (pt.second, pt.first);
            } else if constexpr (array_access_possible<T>::value == true) {
                if constexpr (debug_mode == true) { std::cout << "It's a type with array access\n"; }
                raw_angle = std::atan2 (pt[1], pt[0]);
            } else {
                if constexpr (debug_mode == true) { std::cout << "Maybe it's a type that behaves like an array\n"; }
                raw_angle = std::atan2 (pt[1], pt[0]);
            }

            this->wind (raw_angle);
        }

        //! Update this->angle, this->angle_last and this->angle_sum based on \a
        //! raw_angle. Note that there is no template specialization in this method;
        //! all the specialization is in morph::Winder::wind (const T&, const T&).
        void wind (const double& raw_angle) {

            // Convert the raw angle which has range -pi -> 0 -> +pi into a tansformed angle with range 0->2pi:
            this->angle = raw_angle >= 0 ? raw_angle : (morph::TWO_PI_D + raw_angle);
            if constexpr (morph::debug_mode == true) { std::cout << "angle: " << this->angle << "\n"; }

            // Set the initial angle.
            if (this->angle_last == double{-100.0}) {
                this->angle_last = angle;
            }

            // Compute the change in angle, delta
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

            // Update angle_last and angle_sum
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
