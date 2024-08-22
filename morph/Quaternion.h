/*!
 * A Quaternion class for computing rotations in the visualization classes
 * (morph::Visual, morph::HexGridVisual, etc).
 *
 * This Quaternion class adopts the Hamiltonian convention - w,x,y,z.
 */
#pragma once

#include <morph/mathconst.h>
#include <morph/vec.h>
#include <limits>
#include <cmath>
#include <array>
#include <iostream>
#include <sstream>

namespace morph {

    // Forward declare class and stream operator
    template <typename Flt> class Quaternion;
    template <typename Flt> std::ostream& operator<< (std::ostream&, const Quaternion<Flt>&);

    /*!
     * Quaternion computations
     */
    template <typename Flt>
    class Quaternion
    {
    public:
        // Note, we need a Quaternion which has magnitude 1 as the default.
        constexpr Quaternion()
            : w(Flt{1})
            , x(Flt{0})
            , y(Flt{0})
            , z(Flt{0}) {}

        constexpr Quaternion (Flt _w, Flt _x, Flt _y, Flt _z)
            : w(_w)
            , x(_x)
            , y(_y)
            , z(_z) {}

        //! User-declared copy constructor
        constexpr Quaternion (const Quaternion<Flt>& rhs)
            : w(rhs.w)
            , x(rhs.x)
            , y(rhs.y)
            , z(rhs.z) {}
        //! User-declared copy assignment constructor
        constexpr Quaternion<Flt>& operator= (Quaternion<Flt>& other)
        {
            w = other.w;
            x = other.x;
            y = other.y;
            z = other.z;
            return *this;
        }
        //! Explicitly defaulted  move constructor
        Quaternion(Quaternion<Flt>&& other) = default;
        //! Explicitly defaulted move assignment constructor
        Quaternion<Flt>& operator=(Quaternion<Flt>&& other) = default;

        alignas(Flt) Flt w;
        alignas(Flt) Flt x;
        alignas(Flt) Flt y;
        alignas(Flt) Flt z;

        /*!
         * Renormalize the Quaternion, in case floating point precision errors have
         * caused it to have a magnitude significantly different from 1.
         */
        constexpr void renormalize()
        {
            Flt oneovermag = Flt{1} / std::sqrt (w*w + x*x + y*y + z*z);
            this->w *= oneovermag;
            this->x *= oneovermag;
            this->y *= oneovermag;
            this->z *= oneovermag;
        }

        /*!
         * The threshold outside of which the Quaternion is no longer considered to be a
         * unit Quaternion.
         */
        static constexpr Flt unitThresh = 0.001;

        //! Test to see if this Quaternion is a unit Quaternion.
        constexpr bool checkunit()
        {
            bool rtn = true;
            Flt metric = Flt{1} - (w*w + x*x + y*y + z*z);
            if (std::abs(metric) > morph::Quaternion<Flt>::unitThresh) {
                rtn = false;
            }
            return rtn;
        }

        //! Initialize the Quaternion from the given axis and angle *in degrees*
        constexpr void initFromAxisAngle (const vec<Flt>& axis, const Flt& angle)
        {
            Flt a = morph::mathconst<Flt>::pi_over_360 * angle; // angle/2 converted to rads
            Flt s = std::sin(a);
            Flt c = std::cos(a);
            vec<Flt> ax = axis;
            ax.renormalize();

            this->w = c;
            this->x = ax.x() * s;
            this->y = ax.y() * s;
            this->z = ax.z() * s;

            this->renormalize();
        }

        //! Assignment operators
        void operator= (const Quaternion<Flt>& q2)
        {
            this->w = q2.w;
            this->x = q2.x;
            this->y = q2.y;
            this->z = q2.z;
        }

        //! Equality operator. True if all elements match
        constexpr bool operator==(const Quaternion<Flt>& rhs) const
        {
            return (std::abs(this->w - rhs.w) < std::numeric_limits<Flt>::epsilon()
                    && std::abs(this->x - rhs.x) < std::numeric_limits<Flt>::epsilon()
                    && std::abs(this->y - rhs.y) < std::numeric_limits<Flt>::epsilon()
                    && std::abs(this->z - rhs.z) < std::numeric_limits<Flt>::epsilon());
        }

        //! Not equals
        constexpr bool operator!=(const Quaternion<Flt>& rhs) const
        {
            return (std::abs(this->w - rhs.w) >= std::numeric_limits<Flt>::epsilon()
                    || std::abs(this->x - rhs.x) >= std::numeric_limits<Flt>::epsilon()
                    || std::abs(this->y - rhs.y) >= std::numeric_limits<Flt>::epsilon()
                    || std::abs(this->z - rhs.z) >= std::numeric_limits<Flt>::epsilon());
        }

        //! Overload * operator. q1 is 'this->'
        template <typename F=Flt>
        constexpr Quaternion<Flt> operator* (const Quaternion<F>& q2) const
        {
            Quaternion<Flt> q;
            q.w = this->w * q2.w - this->x * q2.x - this->y * q2.y - this->z * q2.z;
            q.x = this->w * q2.x + this->x * q2.w + this->y * q2.z - this->z * q2.y;
            q.y = this->w * q2.y - this->x * q2.z + this->y * q2.w + this->z * q2.x;
            q.z = this->w * q2.z + this->x * q2.y - this->y * q2.x + this->z * q2.w;
            return q;
        }

        //! Overload / operator. q1 is 'this->', so this is q = q1 / q2
        constexpr Quaternion<Flt> operator/ (const Quaternion<Flt>& q2) const
        {
            Quaternion<Flt> q;
            Flt denom = (w*w + x*x + y*y + z*z);
            q.w = (this->w * q2.w + this->x * q2.x + this->y * q2.y + this->z * q2.z) / denom;
            q.x = (this->w * q2.x - this->x * q2.w - this->y * q2.z + this->z * q2.y) / denom;
            q.y = (this->w * q2.y + this->x * q2.z - this->y * q2.w - this->z * q2.x) / denom;
            q.z = (this->w * q2.z - this->x * q2.y + this->y * q2.x - this->z * q2.w) / denom;
            return q;
        }

        //! Division by a scalar
        constexpr Quaternion<Flt> operator/ (const Flt f) const
        {
            Quaternion<Flt> q;
            q.w = this->w / f;
            q.x = this->x / f;
            q.y = this->y / f;
            q.z = this->z / f;
            return q;
        }

        //! Invert the rotation represented by this Quaternion and return the result.
        constexpr Quaternion<Flt> invert() const
        {
            Quaternion<Flt> qi = *this;
            qi.w = -this->w;
            return qi;
        }

        //! Conjugate of the Quaternion. This happens to give a quaternion representing the same
        //! rotation as that returned by invert() because -q represents an quivalent rotation to q.
        constexpr Quaternion<Flt> conjugate() const
        {
            Quaternion<Flt> qconj (this->w, -this->x, -this->y, -this->z);
            return qconj;
        }

        //! Compute the inverse, q^-1. Also known as the reciprocal, q^-1 * q = I.
        constexpr Quaternion<Flt> inverse() const
        {
            return (this->conjugate() / (w*w + x*x + y*y + z*z));
        }

        //! Return the magnitude of the Quaternion
        constexpr Flt magnitude() const { return std::sqrt (w*w + x*x + y*y + z*z); }

        //! Reset to a zero rotation
        constexpr void reset()
        {
            this->w = Flt{1};
            this->x = Flt{0};
            this->y = Flt{0};
            this->z = Flt{0};
        }

        //! Multiply this quaternion by other as: this = this * q2, i.e. q1 is 'this->'
        constexpr void postmultiply (const Quaternion<Flt>& q2)
        {
            // First make copies of w, x, y, z
            Flt q1_w = this->w;
            Flt q1_x = this->x;
            Flt q1_y = this->y;
            Flt q1_z = this->z;
            // Now compute
            this->w = q1_w * q2.w - q1_x * q2.x - q1_y * q2.y - q1_z * q2.z;
            this->x = q1_w * q2.x + q1_x * q2.w + q1_y * q2.z - q1_z * q2.y;
            this->y = q1_w * q2.y - q1_x * q2.z + q1_y * q2.w + q1_z * q2.x;
            this->z = q1_w * q2.z + q1_x * q2.y - q1_y * q2.x + q1_z * q2.w;
        }

        //! Multiply this quaternion by other as: this = q1 * this
        constexpr void premultiply (const Quaternion<Flt>& q1)
        {
            // First make copies of w, x, y, z
            Flt q2_w = this->w;
            Flt q2_x = this->x;
            Flt q2_y = this->y;
            Flt q2_z = this->z;
            // Now compute
            this->w = q1.w * q2_w - q1.x * q2_x - q1.y * q2_y - q1.z * q2_z;
            this->x = q1.w * q2_x + q1.x * q2_w + q1.y * q2_z - q1.z * q2_y;
            this->y = q1.w * q2_y - q1.x * q2_z + q1.y * q2_w + q1.z * q2_x;
            this->z = q1.w * q2_z + q1.x * q2_y - q1.y * q2_x + q1.z * q2_w;
        }

        /*!
         * Change this Quaternion to represent a new rotation by rotating it \a angle
         * (radians) around the axis given by \a axis_x, \a axis_y, \a axis_z.
         */
        constexpr void rotate (const Flt& axis_x, const Flt& axis_y, const Flt& axis_z, const Flt& angle)
        {
            Flt halfangle = angle * Flt{0.5};
            Flt cosHalf = std::cos (halfangle);
            Flt sinHalf = std::sin (halfangle);
            Quaternion<Flt> local(cosHalf, axis_x * sinHalf, axis_y * sinHalf, axis_z * sinHalf);
            this->premultiply (local);
        }

        /*!
         * Change this Quaternion to represent a new rotation by rotating it \a angle
         * (radians) around the axis given by \a axis.
         */
        constexpr void rotate (const std::array<Flt, 3>& axis, const Flt& angle)
        {
            Flt halfangle = angle * Flt{0.5};
            Flt cosHalf = std::cos (halfangle);
            Flt sinHalf = std::sin (halfangle);
            Quaternion<Flt> local(cosHalf, axis[0] * sinHalf, axis[1] * sinHalf, axis[2] * sinHalf);
            this->premultiply (local);
        }

        /*!
         * Change this Quaternion to represent a new rotation by rotating it \a angle
         * (radians) around the axis given by \a axis.
         */
        constexpr void rotate (const vec<Flt, 3>& axis, const Flt& angle)
        {
            Flt halfangle = angle * Flt{0.5};
            Flt cosHalf = std::cos (halfangle);
            Flt sinHalf = std::sin (halfangle);
            Quaternion<Flt> local(cosHalf, axis[0] * sinHalf, axis[1] * sinHalf, axis[2] * sinHalf);
            this->premultiply (local);
        }

        /*!
         * Obtain the rotation matrix (without assumption that this is a unit
         * Quaternion)
         *
         * std::array represents a matrix with indices like this (i.e. column major
         * format, which is OpenGL friendly)
         *
         *  0  4  8 12
         *  1  5  9 13
         *  2  6 10 14
         *  3  7 11 15
         */
        constexpr std::array<Flt, 16> rotationMatrix() const
        {
            std::array<Flt, 16> mat;
            this->rotationMatrix (mat);
            return mat;
        }

        //! Rotate the matrix \a mat by this Quaternion witout assuming it's a unit Quaternion
        constexpr void rotationMatrix (std::array<Flt, 16>& mat) const
        {
            mat[0] = w*w + x*x - y*y - z*z;
            mat[1] = Flt{2}*x*y + Flt{2}*w*z;
            mat[2] = Flt{2}*x*z - Flt{2}*w*y;
            mat[3] = Flt{0};

            mat[4] = Flt{2}*x*y - Flt{2}*w*z;
            mat[5] = w*w - x*x + y*y - z*z;
            mat[6] = Flt{2}*y*z - Flt{2}*w*x;
            mat[7] = Flt{0};

            mat[8] = Flt{2}*x*z + Flt{2}*w*y;
            mat[9] = Flt{2}*y*z + Flt{2}*w*x;
            mat[10] = w*w - x*x - y*y + z*z;
            mat[11] = Flt{0};

            mat[12] = Flt{0};
            mat[13] = Flt{0};
            mat[14] = Flt{0};
            mat[15] = Flt{1};
        }

        //! Obtain rotation matrix assuming this IS a unit Quaternion
        constexpr std::array<Flt, 16> unitRotationMatrix() const
        {
            std::array<Flt, 16> mat;
            this->unitRotationMatrix (mat);
            return mat;
        }

        //! Rotate the matrix \a mat by this Quaternion, assuming it's a unit Quaternion
        constexpr void unitRotationMatrix (std::array<Flt, 16>& mat) const
        {
            mat[0] = Flt{1} - Flt{2}*y*y - Flt{2}*z*z;
            mat[1] = Flt{2}*x*y + Flt{2}*w*z;
            mat[2] = Flt{2}*x*z - Flt{2}*w*y;
            mat[3] = Flt{0};

            mat[4] = Flt{2}*x*y - Flt{2}*w*z;
            mat[5] = 1.0 - Flt{2}*x*x - Flt{2}*z*z;
            mat[6] = Flt{2}*y*z - Flt{2}*w*x;
            mat[7] = Flt{0};

            mat[8] = Flt{2}*x*z + Flt{2}*w*y;
            mat[9] = Flt{2}*y*z + Flt{2}*w*x;
            mat[10] = Flt{1} - Flt{2}*x*x - Flt{2}*y*y;
            mat[11] = Flt{0};

            mat[12] = Flt{0};
            mat[13] = Flt{0};
            mat[14] = Flt{0};
            mat[15] = Flt{1};
        }

        //! Overload the stream output operator
        friend std::ostream& operator<< <Flt> (std::ostream& os, const Quaternion<Flt>& q);
    };

    template <typename Flt>
    std::ostream& operator<< (std::ostream& os, const Quaternion<Flt>& q)
    {
        os << "Quaternion[wxyz]=(" << q.w << "," << q.x << "," << q.y << "," << q.z << ")";
        return os;
    }

} // namespace morph
