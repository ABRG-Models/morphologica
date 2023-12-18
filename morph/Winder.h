/*!
 * \file
 *
 * Provides morph::Winder, a class to compute the winding number of a boundary with
 * respect to a given coordinate.
 *
 * \author Seb James
 * \date May 2020
 */

#pragma once

#include <memory>
#include <cmath>
#include <stdexcept>
#include <morph/mathconst.h>
#include <type_traits>
#include <morph/trait_tests.h>

namespace morph {

    const bool debug_mode = false;

    /*!
     * A winding number class
     *
     * This class contains an algorithm to integrate the angle transected/traversed
     * (what's the right word?) by a vector (think 'clock hand') drawn from a single
     * coordinate to, in turn, each coordinate on a boundary path. If the single
     * coordinate was inside the boundary, the integrated angle sum will be some
     * multiple of +/-2pi. This gives the 'winding number'.
     *
     * To use, instantiate an object of this class passing the boundary of coordinates
     * that is your path. Then call Winder::wind(const T& coordinate) for some
     * coordinate to find out its winding number (and hence whether it was inside or
     * outside the boundary, which is my motivation for writing this class).
     *
     * This class is specialised so that the container which contains the path
     * coordinates can be any of the straightforward STL containers such as std::vector
     * or std::list (but not std::map). The coordinate should be some type which has one
     * of the following: .first and .second attributes (such as std::pair), .x and .y
     * attributes (such as OpenCV's cv::Point), .x() and .y() methods or the ability to
     * access members in an array-like fashion (std::vector or morph::vec). For example:
     *
     *\code{c++}
     *  std::list<morph::vec<float, 2>> path;
     *  // Code which populates path goes here
     *  morph::Winder w(path);
     *  morph::vec<float, 2> pixel = {0.7, 0.6};
     *  int winding_number = w.wind (pixel);
     *\endcode
     *
     * \tparam T the (2D) coordinate type (this might be cv::Point, morph::BezCoord,
     * morph::vvec, morph::vec, std::array or std::vector)
     *
     * \tparam Container Something like an std::vector, std::list or std::array,
     * containing a path of points. The template-template is not flexible enough for
     * Container to be std::map.
     */
    template<typename C>
    class Winder
    {
        // C must be copyable. This should ensure it has a vlue_type, too.
        static_assert (morph::is_copyable_container<C>::value == true);
        using T = typename C::value_type;

    public:
        //! Construct with the boundary reference.
        Winder (const C& _boundary) : boundary(_boundary) {}

        //! Compute the winding number of the coordinate px with respect to the boundary.
        int wind (const T& px) {
            this->reset();
            for (auto bp : this->boundary) {
                this->wind (px, bp);
            }
            // Do first pixel again to complete the winding:
            T firstpoint = this->boundary.front();
            this->wind (px, firstpoint);
            double winding_no_d = std::round (this->angle_sum/morph::mathconst<double>::two_pi);
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
            this->angle = raw_angle >= 0 ? raw_angle : (morph::mathconst<double>::two_pi + raw_angle);
            if constexpr (morph::debug_mode == true) { std::cout << "angle: " << this->angle << "\n"; }

            // Set the initial angle.
            if (this->angle_last == double{-100.0}) {
                this->angle_last = angle;
            }

            // Compute the change in angle, delta
            double delta = double{0.0}; // delta is 'angle change'
            if (this->angle == double{0.0}) {
                // Special treatment
                if (this->angle_last > morph::mathconst<double>::pi) {
                    // Clockwise to 0
                    delta = (morph::mathconst<double>::two_pi - this->angle_last);
                } else if (this->angle_last < morph::mathconst<double>::pi) {
                    // Anti-clockwise to 0
                    delta = -this->angle_last;
                } else { //angle_last must have been 0.0
                    delta = double{0.0};
                }

            } else {

                // Special treatment required ALSO if we crossed the 0 line without being on it.
                if (this->angle_last > morph::mathconst<double>::pi && this->angle < morph::mathconst<double>::pi) {
                    // crossed from 2pi side to 0 side: Clockwise
                    delta = this->angle + (morph::mathconst<double>::two_pi - this->angle_last);
                } else if (this->angle_last < morph::mathconst<double>::pi_over_2 && this->angle > morph::mathconst<double>::three_pi_over_2) {
                    // crossed from 0 side to 2pi side: Anti-clockwise
                    delta = - this->angle_last - (morph::mathconst<double>::two_pi - this->angle);
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
        const C& boundary;
        //! Current angle around a point
        double angle;
        //! The sum of angles
        double angle_sum;
        //! The angle of the last boundary point
        double angle_last;
    };

} // namespace morph
