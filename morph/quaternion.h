/*!
 * A quaternion class for computing rotations in the visualization classes
 * (morph::Visual, morph::HexGridVisual, etc).
 *
 * This quaternion class adopts the Hamiltonian convention - w,x,y,z.
 */
#pragma once

#include <morph/mathconst.h>
#include <morph/constexpr_math.h>
#include <morph/vec.h>
#include <limits>
#include <cmath>
#include <array>
#include <iostream>
#include <sstream>

namespace morph {

    // Forward declare class and stream operator
    template <typename Flt> class quaternion;
    template <typename Flt> std::ostream& operator<< (std::ostream&, const quaternion<Flt>&);

    /*!
     * Quaternion computations
     */
    template <typename Flt>
    class quaternion
    {
    public:
        //! A default quaternion has magnitude 1.
        constexpr quaternion()
            : w(Flt{1})
            , x(Flt{0})
            , y(Flt{0})
            , z(Flt{0}) {}

        //! Initialize the quaternion with explicit values for the elements
        constexpr quaternion (Flt _w, Flt _x, Flt _y, Flt _z)
            : w(_w)
            , x(_x)
            , y(_y)
            , z(_z) {}

        /*!
         * A constructor that sets up a unit quaternion then applies a rotation (in
         * radians) about an axis
         */
        constexpr quaternion (const morph::vec<Flt, 3>& axis, const Flt angle)
            : w(Flt{1})
            , x(Flt{0})
            , y(Flt{0})
            , z(Flt{0}) { this->rotate (axis, angle); }

        //! User-declared copy constructor
        constexpr quaternion (const quaternion<Flt>& rhs)
            : w(rhs.w)
            , x(rhs.x)
            , y(rhs.y)
            , z(rhs.z) {}
        //! User-declared copy assignment constructor
        constexpr quaternion<Flt>& operator= (quaternion<Flt>& other)
        {
            w = other.w;
            x = other.x;
            y = other.y;
            z = other.z;
            return *this;
        }
        //! Explicitly defaulted  move constructor
        quaternion(quaternion<Flt>&& other) = default;
        //! Explicitly defaulted move assignment constructor
        quaternion<Flt>& operator=(quaternion<Flt>&& other) = default;

        alignas(Flt) Flt w;
        alignas(Flt) Flt x;
        alignas(Flt) Flt y;
        alignas(Flt) Flt z;

        /*!
         * Renormalize the quaternion, in case floating point precision errors have
         * caused it to have a magnitude significantly different from 1.
         */
        constexpr void renormalize()
        {
            Flt oneovermag = Flt{1} / morph::math::sqrt (w*w + x*x + y*y + z*z);
            this->w *= oneovermag;
            this->x *= oneovermag;
            this->y *= oneovermag;
            this->z *= oneovermag;
        }

        /*!
         * The threshold outside of which the quaternion is no longer considered to be a
         * unit quaternion.
         */
        static constexpr Flt unitThresh = Flt{0.001};

        //! Test to see if this quaternion is a unit quaternion.
        constexpr bool checkunit()
        {
            bool rtn = true;
            Flt metric = Flt{1} - (w*w + x*x + y*y + z*z);
            if (morph::math::abs(metric) > morph::quaternion<Flt>::unitThresh) {
                rtn = false;
            }
            return rtn;
        }

        //! Assignment operators
        void operator= (const quaternion<Flt>& q2)
        {
            this->w = q2.w;
            this->x = q2.x;
            this->y = q2.y;
            this->z = q2.z;
        }

        //! Equality operator. True if all elements match
        constexpr bool operator==(const quaternion<Flt>& rhs) const
        {
            return (morph::math::abs(this->w - rhs.w) < std::numeric_limits<Flt>::epsilon()
                    && morph::math::abs(this->x - rhs.x) < std::numeric_limits<Flt>::epsilon()
                    && morph::math::abs(this->y - rhs.y) < std::numeric_limits<Flt>::epsilon()
                    && morph::math::abs(this->z - rhs.z) < std::numeric_limits<Flt>::epsilon());
        }

        //! Not equals
        constexpr bool operator!=(const quaternion<Flt>& rhs) const
        {
            return (morph::math::abs(this->w - rhs.w) >= std::numeric_limits<Flt>::epsilon()
                    || morph::math::abs(this->x - rhs.x) >= std::numeric_limits<Flt>::epsilon()
                    || morph::math::abs(this->y - rhs.y) >= std::numeric_limits<Flt>::epsilon()
                    || morph::math::abs(this->z - rhs.z) >= std::numeric_limits<Flt>::epsilon());
        }

        //! Multiply this quaternion by other as: this = this * q2, i.e. q1 is 'this->'
        constexpr void postmultiply (const quaternion<Flt>& q2)
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
        constexpr void premultiply (const quaternion<Flt>& q1)
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

        //! Overload * operator. q1 is 'this->'. Returns the quaternion that would be written into this by this->postmultiply(q2)
        template <typename F=Flt>
        constexpr quaternion<Flt> operator* (const quaternion<F>& q2) const
        {
            quaternion<Flt> q;
            q.w = this->w * q2.w - this->x * q2.x - this->y * q2.y - this->z * q2.z;
            q.x = this->w * q2.x + this->x * q2.w + this->y * q2.z - this->z * q2.y;
            q.y = this->w * q2.y - this->x * q2.z + this->y * q2.w + this->z * q2.x;
            q.z = this->w * q2.z + this->x * q2.y - this->y * q2.x + this->z * q2.w;
            return q;
        }

        //! Rotate a vector v_r by this quaternion, returning the resulting rotated vector
        template <typename F=Flt, std::size_t N = 3, std::enable_if_t<(N==3||N==4), int> = 0>
        constexpr morph::vec<Flt, N> operator* (const morph::vec<F, N>& v_r) const
        {
            // Do the rotation by extracting the rotation matrix and then rotating.
            std::array<Flt, 16> rotn_mat = { Flt{0} };

            this->rotationMatrix (rotn_mat);

            // Do matrix * vector
            morph::vec<Flt, 4> v = { Flt{0} };
            v[0] = rotn_mat[0] * v_r.x()
                + rotn_mat[4] * v_r.y()
                + rotn_mat[8] * v_r.z()
                + rotn_mat[12]; // * 1
            v[1] = rotn_mat[1] * v_r.x()
                + rotn_mat[5] * v_r.y()
                + rotn_mat[9] * v_r.z()
                + rotn_mat[13];
            v[2] = rotn_mat[2] * v_r.x()
                + rotn_mat[6] * v_r.y()
                + rotn_mat[10] * v_r.z()
                + rotn_mat[14];
            v[3] = rotn_mat[3] * v_r.x()
                + rotn_mat[7] * v_r.y()
                + rotn_mat[11] * v_r.z()
                + rotn_mat[15];

            if constexpr (N==3) {
                return v.less_one_dim();
            } else {
                return v;
            }
        }

        //! Overload / operator. q1 is 'this->', so this is q = q1 / q2
        constexpr quaternion<Flt> operator/ (const quaternion<Flt>& q2) const
        {
            quaternion<Flt> q;
            Flt denom = (w*w + x*x + y*y + z*z);
            q.w = (this->w * q2.w + this->x * q2.x + this->y * q2.y + this->z * q2.z) / denom;
            q.x = (this->w * q2.x - this->x * q2.w - this->y * q2.z + this->z * q2.y) / denom;
            q.y = (this->w * q2.y + this->x * q2.z - this->y * q2.w - this->z * q2.x) / denom;
            q.z = (this->w * q2.z - this->x * q2.y + this->y * q2.x - this->z * q2.w) / denom;
            return q;
        }

        //! Division by a scalar
        constexpr quaternion<Flt> operator/ (const Flt f) const
        {
            quaternion<Flt> q;
            q.w = this->w / f;
            q.x = this->x / f;
            q.y = this->y / f;
            q.z = this->z / f;
            return q;
        }

        //! Invert the rotation represented by this quaternion and return the result.
        constexpr quaternion<Flt> invert() const
        {
            quaternion<Flt> qi = *this;
            qi.w = -this->w;
            return qi;
        }

        //! Conjugate of the quaternion. This happens to give a quaternion representing the same
        //! rotation as that returned by invert() because -q represents an quivalent rotation to q.
        constexpr quaternion<Flt> conjugate() const
        {
            quaternion<Flt> qconj (this->w, -this->x, -this->y, -this->z);
            return qconj;
        }

        //! Compute the inverse, q^-1. Also known as the reciprocal, q^-1 * q = I.
        constexpr quaternion<Flt> inverse() const
        {
            return (this->conjugate() / this->norm_squared());
        }

        //! Return the magnitude of the quaternion (aka the norm)
        constexpr Flt magnitude() const { return morph::math::sqrt (w*w + x*x + y*y + z*z); }
        //! Return the norm of the quaternion (aka the magnitude)
        constexpr Flt norm() const { return morph::math::sqrt (w*w + x*x + y*y + z*z); }
        //! Sometimes you'll want the norm squared. Save the morph::math::sqrt and a multiplication.
        constexpr Flt norm_squared() const { return (w*w + x*x + y*y + z*z); }

        //! Reset to a zero rotation
        constexpr void reset()
        {
            this->w = Flt{1};
            this->x = Flt{0};
            this->y = Flt{0};
            this->z = Flt{0};
        }

        //! Reset the quaternion and set the rotation about the given axis and angle in
        //! radians. This function was previously called initFromAxisAngle
        constexpr void set_rotation (const vec<Flt>& axis, const Flt& angle)
        {
            Flt halfangle = angle * Flt{0.5};
            Flt cosHalf = std::cos(halfangle);
            Flt sinHalf = std::sin(halfangle);
            vec<Flt> ax = axis;
            ax.renormalize();

            this->w = cosHalf;
            this->x = ax.x() * sinHalf;
            this->y = ax.y() * sinHalf;
            this->z = ax.z() * sinHalf;

            this->renormalize();
        }

        /*!
         * Change this quaternion to represent a new rotation by rotating it \a angle
         * (radians) around the axis given by \a axis_x, \a axis_y, \a axis_z. Renormalize to finish.
         */
        constexpr void rotate (const Flt& axis_x, const Flt& axis_y, const Flt& axis_z, const Flt& angle)
        {
            Flt halfangle = angle * Flt{0.5};
            Flt cosHalf = std::cos (halfangle);
            Flt sinHalf = std::sin (halfangle);
            quaternion<Flt> local(cosHalf, axis_x * sinHalf, axis_y * sinHalf, axis_z * sinHalf);
            this->premultiply (local);
            this->renormalize();
        }

        /*!
         * Change this quaternion to represent a new rotation by rotating it \a angle
         * (radians) around the axis given by \a axis. Renormalize to finish.
         */
        constexpr void rotate (const std::array<Flt, 3>& axis, const Flt& angle)
        {
            Flt halfangle = angle * Flt{0.5};
            Flt cosHalf = std::cos (halfangle);
            Flt sinHalf = std::sin (halfangle);
            quaternion<Flt> local(cosHalf, axis[0] * sinHalf, axis[1] * sinHalf, axis[2] * sinHalf);
            this->premultiply (local);
            this->renormalize();
        }

        /*!
         * Change this quaternion to represent a new rotation by rotating it \a angle
         * (radians) around the axis given by \a axis. Renormalize to finish.
         */
        constexpr void rotate (const vec<Flt, 3>& axis, const Flt& angle)
        {
            Flt halfangle = angle * Flt{0.5};
            Flt cosHalf = std::cos (halfangle);
            Flt sinHalf = std::sin (halfangle);
            quaternion<Flt> local(cosHalf, axis[0] * sinHalf, axis[1] * sinHalf, axis[2] * sinHalf);
            this->premultiply (local);
            this->renormalize();
        }

        /*!
         * Return a 4 element vec containing the axis (elements 0, 1 and 2) about which
         * to rotate an angle (element 3) in radians to make a rotation equivalent to
         * this quaternion
         */
        constexpr morph::vec<Flt, 4> axis_angle() const
        {
            morph::vec<Flt, 4> aa{Flt{0}};
            aa[3] =  2 * std::acos (this->w);
            Flt s = morph::math::sqrt(Flt{1} - this->w * this->w);
            aa[0] = this->x / s;
            aa[1] = this->y / s;
            aa[2] = this->z / s;
            return aa;
        }

        /*!
         * Obtain the rotation matrix (without assumption that this is a unit
         * quaternion)
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

        /*!
         * Fill the matrix \a mat with the values to represent the rotation that is
         * represented by this quaternion. This function *does not assume that the
         * quaternion representing the rotation is a unit quaternion*.
         */
        constexpr void rotationMatrix (std::array<Flt, 16>& mat) const
        {
            mat[0] = w*w + x*x - y*y - z*z;
            mat[1] = Flt{2}*x*y + Flt{2}*w*z;
            mat[2] = Flt{2}*x*z - Flt{2}*w*y;
            mat[3] = Flt{0};

            mat[4] = Flt{2}*x*y - Flt{2}*w*z;
            mat[5] = w*w - x*x + y*y - z*z;
            mat[6] = Flt{2}*y*z + Flt{2}*w*x;
            mat[7] = Flt{0};

            mat[8] = Flt{2}*x*z + Flt{2}*w*y;
            mat[9] = Flt{2}*y*z - Flt{2}*w*x;
            mat[10] = w*w - x*x - y*y + z*z;
            mat[11] = Flt{0};

            mat[12] = Flt{0};
            mat[13] = Flt{0};
            mat[14] = Flt{0};
            mat[15] = Flt{1};

            // Without this renormalization, the quaternion *would* have to be unit.
            Flt one_over_norm_squared = Flt{1} / this->norm_squared();
            for (auto& e : mat) { e *= one_over_norm_squared; }
        }

        //! Obtain rotation matrix assuming this IS a unit quaternion
        constexpr std::array<Flt, 16> unitRotationMatrix() const
        {
            std::array<Flt, 16> mat;
            this->unitRotationMatrix (mat);
            return mat;
        }

        //! Rotate the matrix \a mat by this quaternion, *assuming it's a unit
        //! quaternion*.
        constexpr void unitRotationMatrix (std::array<Flt, 16>& mat) const
        {
            mat[0] = Flt{1} - Flt{2}*y*y - Flt{2}*z*z;
            mat[1] = Flt{2}*x*y + Flt{2}*w*z;
            mat[2] = Flt{2}*x*z - Flt{2}*w*y;
            mat[3] = Flt{0};

            mat[4] = Flt{2}*x*y - Flt{2}*w*z;
            mat[5] = Flt{1} - Flt{2}*x*x - Flt{2}*z*z;
            mat[6] = Flt{2}*y*z + Flt{2}*w*x;
            mat[7] = Flt{0};

            mat[8] = Flt{2}*x*z + Flt{2}*w*y;
            mat[9] = Flt{2}*y*z - Flt{2}*w*x;
            mat[10] = Flt{1} - Flt{2}*x*x - Flt{2}*y*y;
            mat[11] = Flt{0};

            mat[12] = Flt{0};
            mat[13] = Flt{0};
            mat[14] = Flt{0};
            mat[15] = Flt{1};
        }

        //! Overload the stream output operator
        friend std::ostream& operator<< <Flt> (std::ostream& os, const quaternion<Flt>& q);
    };

    template <typename Flt>
    std::ostream& operator<< (std::ostream& os, const quaternion<Flt>& q)
    {
        os << "quaternion[wxyz]=(" << q.w << "," << q.x << "," << q.y << "," << q.z << ")";
        return os;
    }

} // namespace morph
