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
#include <type_traits>

namespace morph {

    // Forward declare class and stream operator
    template <typename F>  requires std::is_floating_point_v<F> class quaternion;
    template <typename F> std::ostream& operator<< (std::ostream&, const quaternion<F>&);

    /*!
     * Quaternion computations
     */
    template <typename F> requires std::is_floating_point_v<F>
    class quaternion
    {
    public:
        //! A default quaternion has magnitude 1.
        constexpr quaternion() noexcept : w(F{1}), x(F{0}), y(F{0}), z(F{0}) {}

        //! Initialize the quaternion with explicit values for the elements
        constexpr quaternion (F _w, F _x, F _y, F _z) noexcept : w(_w), x(_x), y(_y), z(_z) {}

        /*!
         * A constructor that sets up a unit quaternion then applies a rotation (in
         * radians) about an axis
         */
        constexpr quaternion (const morph::vec<F, 3>& axis, const F angle) noexcept : w(F{1}), x(F{0}), y(F{0}), z(F{0}) { this->rotate (axis, angle); }

        //! User-declared copy constructor
        constexpr quaternion (const quaternion<F>& rhs) noexcept : w(rhs.w), x(rhs.x), y(rhs.y), z(rhs.z) {}

        //! User-declared copy assignment constructor
        constexpr quaternion<F>& operator= (quaternion<F>& other) noexcept
        {
            w = other.w;
            x = other.x;
            y = other.y;
            z = other.z;
            return *this;
        }
        //! Explicitly defaulted  move constructor
        quaternion(quaternion<F>&& other) noexcept = default;
        //! Explicitly defaulted move assignment constructor
        quaternion<F>& operator=(quaternion<F>&& other) noexcept = default;

        alignas(F) F w;
        alignas(F) F x;
        alignas(F) F y;
        alignas(F) F z;

        /*!
         * Renormalize the quaternion, in case floating point precision errors have
         * caused it to have a magnitude significantly different from 1.
         */
        constexpr void renormalize() noexcept
        {
            F oneovermag = F{1} / morph::math::sqrt (w*w + x*x + y*y + z*z);
            this->w *= oneovermag;
            this->x *= oneovermag;
            this->y *= oneovermag;
            this->z *= oneovermag;
        }

        /*!
         * The threshold outside of which the quaternion is no longer considered to be a
         * unit quaternion.
         */
        static constexpr F unitThresh() noexcept
        {
            // Note: std::float16_t comes with C++23
            if constexpr (std::is_same<F, float>::value) {
                return F{1e-6};
            } else { // double
                return F{1e-14};
            }
        }

        //! Test to see if this quaternion is a unit quaternion.
        constexpr bool checkunit() noexcept
        {
            bool rtn = true;
            F metric = F{1} - (w*w + x*x + y*y + z*z);

            if (morph::math::abs(metric) > morph::quaternion<F>::unitThresh()) {
                rtn = false;
            }
            return rtn;
        }

        //! Assignment operators
        void operator= (const quaternion<F>& q2) noexcept
        {
            this->w = q2.w;
            this->x = q2.x;
            this->y = q2.y;
            this->z = q2.z;
        }

        //! Equality operator. True if all elements match
        constexpr bool operator== (const quaternion<F>& rhs) const noexcept
        {
            return (morph::math::abs(this->w - rhs.w) < std::numeric_limits<F>::epsilon()
                    && morph::math::abs(this->x - rhs.x) < std::numeric_limits<F>::epsilon()
                    && morph::math::abs(this->y - rhs.y) < std::numeric_limits<F>::epsilon()
                    && morph::math::abs(this->z - rhs.z) < std::numeric_limits<F>::epsilon());
        }

        //! Not equals
        constexpr bool operator!= (const quaternion<F>& rhs) const noexcept
        {
            return (morph::math::abs(this->w - rhs.w) >= std::numeric_limits<F>::epsilon()
                    || morph::math::abs(this->x - rhs.x) >= std::numeric_limits<F>::epsilon()
                    || morph::math::abs(this->y - rhs.y) >= std::numeric_limits<F>::epsilon()
                    || morph::math::abs(this->z - rhs.z) >= std::numeric_limits<F>::epsilon());
        }

        //! Multiply this quaternion by other as: this = this * q2, i.e. q1 is 'this->'
        constexpr void postmultiply (const quaternion<F>& q2) noexcept
        {
            // First make copies of w, x, y, z
            F q1_w = this->w;
            F q1_x = this->x;
            F q1_y = this->y;
            F q1_z = this->z;
            // Now compute
            this->w = q1_w * q2.w - q1_x * q2.x - q1_y * q2.y - q1_z * q2.z;
            this->x = q1_w * q2.x + q1_x * q2.w + q1_y * q2.z - q1_z * q2.y;
            this->y = q1_w * q2.y - q1_x * q2.z + q1_y * q2.w + q1_z * q2.x;
            this->z = q1_w * q2.z + q1_x * q2.y - q1_y * q2.x + q1_z * q2.w;
        }

        //! Multiply this quaternion by other as: this = q1 * this
        constexpr void premultiply (const quaternion<F>& q1) noexcept
        {
            // First make copies of w, x, y, z
            F q2_w = this->w;
            F q2_x = this->x;
            F q2_y = this->y;
            F q2_z = this->z;
            // Now compute
            this->w = q1.w * q2_w - q1.x * q2_x - q1.y * q2_y - q1.z * q2_z;
            this->x = q1.w * q2_x + q1.x * q2_w + q1.y * q2_z - q1.z * q2_y;
            this->y = q1.w * q2_y - q1.x * q2_z + q1.y * q2_w + q1.z * q2_x;
            this->z = q1.w * q2_z + q1.x * q2_y - q1.y * q2_x + q1.z * q2_w;
        }

        //! Overload * operator. q1 is 'this->'. Returns the quaternion that would be written into this by this->postmultiply(q2)
        template <typename Fy=F>
        constexpr quaternion<F> operator* (const quaternion<Fy>& q2) const noexcept
        {
            quaternion<F> q;
            q.w = this->w * q2.w - this->x * q2.x - this->y * q2.y - this->z * q2.z;
            q.x = this->w * q2.x + this->x * q2.w + this->y * q2.z - this->z * q2.y;
            q.y = this->w * q2.y - this->x * q2.z + this->y * q2.w + this->z * q2.x;
            q.z = this->w * q2.z + this->x * q2.y - this->y * q2.x + this->z * q2.w;
            return q;
        }

        //! Fast algorithm for performing the rotation of a 3 element vector using the quaternion. Returns the resulting 3 element rotated vector
        template <typename Fy=F>
        constexpr morph::vec<F, 3> rotate_vec (const morph::vec<Fy, 3>& v_r) const noexcept
        {
            morph::vec<F, 3> vr = v_r.template as<F>(); // Make copy of v_r using type F
            morph::vec<F, 3> q_im = { this->x, this->y, this->z };
            morph::vec<F, 3> t = F{2} * q_im.cross (vr);
            vr += (this->w * t + q_im.cross (t));
            return vr;
        }

        //! Rotate a 3D vector that has been padded at the end with a single zero to create a 4 element vector. Returns the 3 element rotated vector padded with a single trailing zero
        template <typename Fy=F>
        constexpr morph::vec<F, 4> rotate_vec (const morph::vec<Fy, 4>& v_r) const noexcept
        {
            morph::quaternion<F> v_quat ( F{0}, static_cast<F>(v_r[0]), static_cast<F>(v_r[1]), static_cast<F>(v_r[2]) );
            morph::quaternion<F> v_rotated = (*this * v_quat) * this->conjugate();
            return { v_rotated.x, v_rotated.y, v_rotated.z, F{0} };
        }

        //! Rotate a vector v_r by this quaternion, returning the resulting rotated vector
        template <typename Fy=F, std::size_t N = 3, std::enable_if_t<(N==3||N==4), int> = 0>
        constexpr morph::vec<F, N> operator* (const morph::vec<Fy, N>& v_r) const noexcept
        {
            return this->rotate_vec (v_r);
        }

        //! Rotate a vector v_r by this quaternion by first forming a rotation matrix and then multiplying.  Returns the resulting rotated vector
        template <typename Fy=F, std::size_t N = 3, std::enable_if_t<(N==3||N==4), int> = 0>
        constexpr morph::vec<F, N> rotate_vec_matrix (const morph::vec<Fy, N>& v_r) const noexcept
        {
            // Do the rotation by extracting the rotation matrix and then rotating.
            std::array<F, 16> rotn_mat = { F{0} };

            this->rotationMatrix (rotn_mat);

            // Do matrix * vector
            morph::vec<F, 4> v = { F{0} };
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
        constexpr quaternion<F> operator/ (const quaternion<F>& q2) const noexcept
        {
            quaternion<F> q;
            F denom = (w*w + x*x + y*y + z*z);
            q.w = (this->w * q2.w + this->x * q2.x + this->y * q2.y + this->z * q2.z) / denom;
            q.x = (this->w * q2.x - this->x * q2.w - this->y * q2.z + this->z * q2.y) / denom;
            q.y = (this->w * q2.y + this->x * q2.z - this->y * q2.w - this->z * q2.x) / denom;
            q.z = (this->w * q2.z - this->x * q2.y + this->y * q2.x - this->z * q2.w) / denom;
            return q;
        }

        //! Division by a scalar
        constexpr quaternion<F> operator/ (const F f) const noexcept
        {
            quaternion<F> q;
            q.w = this->w / f;
            q.x = this->x / f;
            q.y = this->y / f;
            q.z = this->z / f;
            return q;
        }

        //! Invert the rotation represented by this quaternion and return the result.
        constexpr quaternion<F> invert() const noexcept
        {
            quaternion<F> qi = *this;
            qi.w = -this->w;
            return qi;
        }

        //! Conjugate of the quaternion. This happens to give a quaternion representing the same
        //! rotation as that returned by invert() because -q represents an quivalent rotation to q.
        constexpr quaternion<F> conjugate() const noexcept
        {
            quaternion<F> qconj (this->w, -this->x, -this->y, -this->z);
            return qconj;
        }

        //! Compute the inverse, q^-1. Also known as the reciprocal, q^-1 * q = I.
        constexpr quaternion<F> inverse() const noexcept
        {
            return (this->conjugate() / this->norm_squared());
        }

        //! Return the magnitude of the quaternion (aka the norm)
        constexpr F magnitude() const noexcept { return morph::math::sqrt (w*w + x*x + y*y + z*z); }
        //! Return the norm of the quaternion (aka the magnitude)
        constexpr F norm() const noexcept { return morph::math::sqrt (w*w + x*x + y*y + z*z); }
        //! Sometimes you'll want the norm squared. Save the morph::math::sqrt and a multiplication.
        constexpr F norm_squared() const noexcept { return (w*w + x*x + y*y + z*z); }

        //! Reset to a zero rotation
        constexpr void reset() noexcept
        {
            this->w = F{1};
            this->x = F{0};
            this->y = F{0};
            this->z = F{0};
        }

        //! Reset the quaternion and set the rotation about the given axis and angle in
        //! radians. This function was previously called initFromAxisAngle
        constexpr void set_rotation (const vec<F>& axis, const F& angle) noexcept
        {
            F halfangle = angle * F{0.5};
            F cosHalf = morph::math::cos(halfangle);
            F sinHalf = morph::math::sin(halfangle);
            vec<F> ax = axis;
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
        constexpr void rotate (const F& axis_x, const F& axis_y, const F& axis_z, const F& angle) noexcept
        {
            F halfangle = angle * F{0.5};
            F cosHalf = morph::math::cos (halfangle);
            F sinHalf = morph::math::sin (halfangle);
            quaternion<F> local(cosHalf, axis_x * sinHalf, axis_y * sinHalf, axis_z * sinHalf);
            this->premultiply (local);
            this->renormalize();
        }

        /*!
         * Change this quaternion to represent a new rotation by rotating it \a angle
         * (radians) around the axis given by \a axis. Renormalize to finish.
         */
        constexpr void rotate (const std::array<F, 3>& axis, const F& angle) noexcept
        {
            F halfangle = angle * F{0.5};
            F cosHalf = morph::math::cos (halfangle);
            F sinHalf = morph::math::sin (halfangle);
            quaternion<F> local(cosHalf, axis[0] * sinHalf, axis[1] * sinHalf, axis[2] * sinHalf);
            this->premultiply (local);
            this->renormalize();
        }

        /*!
         * Change this quaternion to represent a new rotation by rotating it \a angle
         * (radians) around the axis given by \a axis. Renormalize to finish.
         */
        constexpr void rotate (const vec<F, 3>& axis, const F& angle) noexcept
        {
            F halfangle = angle * F{0.5};
            F cosHalf = morph::math::cos (halfangle);
            F sinHalf = morph::math::sin (halfangle);
            quaternion<F> local(cosHalf, axis[0] * sinHalf, axis[1] * sinHalf, axis[2] * sinHalf);
            this->premultiply (local);
            this->renormalize();
        }

        /*!
         * Return a 4 element vec containing the axis (elements 0, 1 and 2) about which
         * to rotate an angle (element 3) in radians to make a rotation equivalent to
         * this quaternion
         */
        constexpr morph::vec<F, 4> axis_angle() const noexcept
        {
            morph::vec<F, 4> aa{F{0}};
            aa[3] =  2 * morph::math::acos (this->w);
            F s = morph::math::sqrt (F{1} - this->w * this->w);
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
        constexpr std::array<F, 16> rotationMatrix() const noexcept
        {
            std::array<F, 16> mat;
            this->rotationMatrix (mat);
            return mat;
        }

        /*!
         * Fill the matrix \a mat with the values to represent the rotation that is
         * represented by this quaternion. This function *does not assume that the
         * quaternion representing the rotation is a unit quaternion*.
         */
        constexpr void rotationMatrix (std::array<F, 16>& mat) const noexcept
        {
            mat[0] = w*w + x*x - y*y - z*z;
            mat[1] = F{2}*x*y + F{2}*w*z;
            mat[2] = F{2}*x*z - F{2}*w*y;
            mat[3] = F{0};

            mat[4] = F{2}*x*y - F{2}*w*z;
            mat[5] = w*w - x*x + y*y - z*z;
            mat[6] = F{2}*y*z + F{2}*w*x;
            mat[7] = F{0};

            mat[8] = F{2}*x*z + F{2}*w*y;
            mat[9] = F{2}*y*z - F{2}*w*x;
            mat[10] = w*w - x*x - y*y + z*z;
            mat[11] = F{0};

            mat[12] = F{0};
            mat[13] = F{0};
            mat[14] = F{0};
            mat[15] = F{1};

            // Without this renormalization, the quaternion *would* have to be unit.
            F one_over_norm_squared = F{1} / this->norm_squared();
            for (auto& e : mat) { e *= one_over_norm_squared; }
        }

        //! Obtain rotation matrix assuming this IS a unit quaternion
        constexpr std::array<F, 16> unitRotationMatrix() const noexcept
        {
            std::array<F, 16> mat;
            this->unitRotationMatrix (mat);
            return mat;
        }

        //! Rotate the matrix \a mat by this quaternion, *assuming it's a unit
        //! quaternion*.
        constexpr void unitRotationMatrix (std::array<F, 16>& mat) const noexcept
        {
            mat[0] = F{1} - F{2}*y*y - F{2}*z*z;
            mat[1] = F{2}*x*y + F{2}*w*z;
            mat[2] = F{2}*x*z - F{2}*w*y;
            mat[3] = F{0};

            mat[4] = F{2}*x*y - F{2}*w*z;
            mat[5] = F{1} - F{2}*x*x - F{2}*z*z;
            mat[6] = F{2}*y*z + F{2}*w*x;
            mat[7] = F{0};

            mat[8] = F{2}*x*z + F{2}*w*y;
            mat[9] = F{2}*y*z - F{2}*w*x;
            mat[10] = F{1} - F{2}*x*x - F{2}*y*y;
            mat[11] = F{0};

            mat[12] = F{0};
            mat[13] = F{0};
            mat[14] = F{0};
            mat[15] = F{1};
        }

        //! Overload the stream output operator
        friend std::ostream& operator<< <F> (std::ostream& os, const quaternion<F>& q);
    };

    template <typename F>
    std::ostream& operator<< (std::ostream& os, const quaternion<F>& q)
    {
        os << "quaternion[wxyz]=(" << q.w << "," << q.x << "," << q.y << "," << q.z << ")";
        return os;
    }

} // namespace morph
