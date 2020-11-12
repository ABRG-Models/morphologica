/*!
 * A Quaternion class for computing rotations in the visualization classes
 * (morph::Visual, morph::HexGridVisual, etc).
 */
#pragma once

#include <morph/Vector.h>
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
    private:
        const Flt oneOver360   = 0.00277777777778;
        const Flt pi           = 3.14159265358979;
        const Flt piOver360    = 0.00872664625997;
        const Flt twoPiOver360 = 0.01745329251994;

    public:
        // Note, we need a Quaternion which has magnitude 1 as the default.
        Quaternion (void)
            : w(1.0)
            , x(0.0)
            , y(0.0)
            , z(0.0) {}

        Quaternion (Flt _w, Flt _x, Flt _y, Flt _z)
            : w(_w)
            , x(_x)
            , y(_y)
            , z(_z) {}

        alignas(Flt) Flt w;
        alignas(Flt) Flt x;
        alignas(Flt) Flt y;
        alignas(Flt) Flt z;

        //! An "output to stdout" function
        //void output (void) const { std::cout << this->str() << std::endl; }

        //! String output
        std::string str() const
        {
            std::stringstream ss;
            ss << "Quaternion[wxyz]=(" << w << "," << x << "," << y << "," << z << ")";
            return ss.str();
        }

        /*!
         * Renormalize the Quaternion, in case floating point precision errors have
         * caused it to have a magnitude significantly different from 1.
         */
        void renormalize (void)
        {
            Flt oneovermag = 1.0 / std::sqrt (w*w + x*x + y*y + z*z);
            this->w *= oneovermag;
            this->x *= oneovermag;
            this->y *= oneovermag;
            this->z *= oneovermag;
        }

        /*!
         * The threshold outside of which the Quaternion is no longer considered to be a
         * unit Quaternion.
         */
        const Flt unitThresh = 0.001;

        //! Test to see if this Quaternion is a unit Quaternion.
        bool checkunit (void)
        {
            bool rtn = true;
            Flt metric = 1.0 - (w*w + x*x + y*y + z*z);
            if (std::abs(metric) > morph::Quaternion<Flt>::unitThresh) {
                rtn = false;
            }
            return rtn;
        }

        //! Initialize the Quaternion from the given axis and angle *in degrees*
        void initFromAxisAngle (const Vector<Flt>& axis, const Flt& angle)
        {
            Flt a = piOver360 * angle; // angle/2 converted to rads
            Flt s = std::sin(a);
            Flt c = std::cos(a);
            Vector<Flt> ax = axis;
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

        //! Overload * operator. q1 is 'this->'
        Quaternion<Flt> operator* (const Quaternion<Flt>& q2)
        {
            Quaternion<Flt> q;
            q.w = this->w * q2.w - this->x * q2.x - this->y * q2.y - this->z * q2.z;
            q.x = this->w * q2.x + this->x * q2.w + this->y * q2.z - this->z * q2.y;
            q.y = this->w * q2.y - this->x * q2.z + this->y * q2.w + this->z * q2.x;
            q.z = this->w * q2.z + this->x * q2.y - this->y * q2.x + this->z * q2.w;
            return q;
        }

        //! Invert the rotation represented by this Quaternion and return the result
        Quaternion<Flt> invert() const
        {
            Quaternion<Flt> qi = *this;
            qi.w = -this->w;
            return qi;
        }

        //! Multiply this quaternion by other as: this = this * q2, i.e. q1 is 'this->'
        void postmultiply (const Quaternion<Flt>& q2)
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
        void premultiply (const Quaternion<Flt>& q1)
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
        void rotate (const Flt axis_x, const Flt axis_y, const Flt axis_z, const Flt angle)
        {
            Flt halfangle = angle*0.5;
            Flt cosHalf = std::cos (halfangle);
            Flt sinHalf = std::sin (halfangle);
            Quaternion<Flt> local(cosHalf, axis_x * sinHalf, axis_y * sinHalf, axis_z * sinHalf);
            this->premultiply (local);
        }

        /*!
         * Change this Quaternion to represent a new rotation by rotating it \a angle
         * (radians) around the axis given by \a axis.
         */
        void rotate (const std::array<Flt, 3>& axis, const Flt angle)
        {
            Flt halfangle = angle*0.5;
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
        std::array<Flt, 16> rotationMatrix (void) const
        {
            std::array<Flt, 16> mat;
            this->rotationMatrix (mat);
            return mat;
        }

        //! Rotate the matrix \a mat by this Quaternion witout assuming it's a unit Quaternion
        void rotationMatrix (std::array<Flt, 16>& mat) const
        {
            mat[0] = w*w + x*x - y*y - z*z;
            mat[1] = 2*x*y + 2*w*z;
            mat[2] = 2*x*z - 2*w*y;
            mat[3] = 0.0;

            mat[4] = 2*x*y - 2*w*z;
            mat[5] = w*w - x*x + y*y - z*z;
            mat[6] = 2*y*z - 2*w*x;
            mat[7] = 0.0;

            mat[8] = 2*x*z + 2*w*y;
            mat[9] = 2*y*z + 2*w*x;
            mat[10] = w*w - x*x - y*y + z*z;
            mat[11] = 0.0;

            mat[12] = 0.0;
            mat[13] = 0.0;
            mat[14] = 0.0;
            mat[15] = 1.0;
        }

        //! Obtain rotation matrix assuming this IS a unit Quaternion
        std::array<Flt, 16> unitRotationMatrix (void) const
        {
            std::array<Flt, 16> mat;
            this->unitRotationMatrix (mat);
            return mat;
        }

        //! Rotate the matrix \a mat by this Quaternion, assuming it's a unit Quaternion
        void unitRotationMatrix (std::array<Flt, 16>& mat) const
        {
            mat[0] = 1.0 - 2*y*y - 2*z*z;
            mat[1] = 2*x*y + 2*w*z;
            mat[2] = 2*x*z - 2*w*y;
            mat[3] = 0.0;

            mat[4] = 2*x*y - 2*w*z;
            mat[5] = 1.0 - 2*x*x - 2*z*z;
            mat[6] = 2*y*z - 2*w*x;
            mat[7] = 0.0;

            mat[8] = 2*x*z + 2*w*y;
            mat[9] = 2*y*z + 2*w*x;
            mat[10] = 1.0 - 2*x*x - 2*y*y;
            mat[11] = 0.0;

            mat[12] = 0.0;
            mat[13] = 0.0;
            mat[14] = 0.0;
            mat[15] = 1.0;
        }

        //! Overload the stream output operator
        friend std::ostream& operator<< <Flt> (std::ostream& os, const Quaternion<Flt>& q);
    };

    template <typename Flt>
    std::ostream& operator<< (std::ostream& os, const Quaternion<Flt>& q)
    {
        os << q.str();
        return os;
    }

} // namespace morph
