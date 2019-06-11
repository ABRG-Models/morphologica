/*!
 * A Transformation matrix class
 */
#ifndef _TRANSFORMMATRIX_H_
#define _TRANSFORMMATRIX_H_

#include "Quaternion.h"
using morph::Quaternion;
#include "Vector3.h"
using morph::Vector3;

#include <cmath>
using std::abs;
using std::sqrt;
using std::cos;
using std::sin;

#include <array>
using std::array;

namespace morph {

    template <class Flt>
    class TransformMatrix
    {
    private:
        const Flt oneOver360   = 0.00277777777778;
        const Flt pi           = 3.14159265358979;
        const Flt piOver360    = 0.00872664625997;
        const Flt twoPiOver360 = 0.01745329251994;

    public:

        TransformMatrix (void) {}

        /*!
         * The transformation matrix data, arranged in column major
         * format to be OpenGL friendly.
         */
        alignas(array<Flt, 16>) array<Flt, 16> data;

        //! Self-explanatory
        void setToIdentity (void) {
            this->data.fill (static_cast<Flt>(0.0));
            this->data[0] = static_cast<Flt>(1.0);
            this->data[5] = static_cast<Flt>(1.0);
            this->data[10] = static_cast<Flt>(1.0);
            this->data[15] = static_cast<Flt>(1.0);
        }

        void translate (const Vector3<Flt>& v) {
            this->data[3] += v.x;
            this->data[7] += v.y;
            this->data[11] += v.z;
        }

        void translate (const Flt& x, const Flt& y, const Flt& z) {
            this->data[3] += x;
            this->data[7] += y;
            this->data[11] += z;
        }

        /*!
         * Algorithm from:
         * http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q54
         */
        void rotate (const Quaternion<float>& q) {

            array<Flt, 16> m;

            const float f2x = q.x + q.x;
            const float f2y = q.y + q.y;
            const float f2z = q.z + q.z;
            const float f2xw = f2x * q.w;
            const float f2yw = f2y * q.w;
            const float f2zw = f2z * q.w;
            const float f2xx = f2x * q.x;
            const float f2xy = f2x * q.y;
            const float f2xz = f2x * q.z;
            const float f2yy = f2y * q.y;
            const float f2yz = f2y * q.z;
            const float f2zz = f2z * q.z;

            m[0]  = 1.0f - (f2yy + f2zz);
            m[1]  =         f2xy - f2zw;
            m[2]  =         f2xz + f2yw;
            m[3]  = 0.0f;
            m[4]  =         f2xy + f2zw;
            m[5]  = 1.0f - (f2xx + f2zz);
            m[6]  =         f2yz - f2xw;
            m[7]  = 0.0f;
            m[8]  =         f2xz - f2yw;
            m[9]  =         f2yz + f2xw;
            m[10] = 1.0f - (f2xx + f2yy);
            m[11] = 0.0f;
            m[12] = 0.0f;
            m[13] = 0.0f;
            m[14] = 0.0f;
            m[15] = 1.0f;

            *this *= m;
        }

        void rotate (const Quaternion<double>& q) {

            array<Flt, 16> m;

            const double f2x = q.x + q.x;
            const double f2y = q.y + q.y;
            const double f2z = q.z + q.z;
            const double f2xw = f2x * q.w;
            const double f2yw = f2y * q.w;
            const double f2zw = f2z * q.w;
            const double f2xx = f2x * q.x;
            const double f2xy = f2x * q.y;
            const double f2xz = f2x * q.z;
            const double f2yy = f2y * q.y;
            const double f2yz = f2y * q.z;
            const double f2zz = f2z * q.z;

            m[0]  = 1.0 - (f2yy + f2zz);
            m[1]  =        f2xy - f2zw;
            m[2]  =        f2xz + f2yw;
            m[3]  = 0.0;
            m[4]  =        f2xy + f2zw;
            m[5]  = 1.0 - (f2xx + f2zz);
            m[6]  =        f2yz - f2xw;
            m[7]  = 0.0;
            m[8]  =        f2xz - f2yw;
            m[9]  =        f2yz + f2xw;
            m[10] = 1.0 - (f2xx + f2yy);
            m[11] = 0.0;
            m[12] = 0.0;
            m[13] = 0.0;
            m[14] = 0.0;
            m[15] = 1.0;

            *this *= m;
        }

        TransformMatrix<Flt> operator*= (const array<Flt, 16>& m2) {

            array<Flt, 16> result;
            // Top row
            result[0] = this->data[0] * m2[0]
                + this->data[4] * m2[1]
                + this->data[8] * m2[2]
                + this->data[12] * m2[3];
            result[4] = this->data[0] * m2[4]
                + this->data[4] * m2[5]
                + this->data[8] * m2[6]
                + this->data[12] * m2[7];
            result[8] = this->data[0] * m2[8]
                + this->data[4] * m2[9]
                + this->data[8] * m2[10]
                + this->data[12] * m2[11];
            result[12] = this->data[0] * m2[12]
                + this->data[4] * m2[13]
                + this->data[8] * m2[14]
                + this->data[12] * m2[15];

            // Second row
            result[1] = this->data[1] * m2[0]
                + this->data[5] * m2[1]
                + this->data[9] * m2[2]
                + this->data[13] * m2[3];
            result[5] = this->data[1] * m2[4]
                + this->data[5] * m2[5]
                + this->data[9] * m2[6]
                + this->data[13] * m2[7];
            result[9] = this->data[1] * m2[8]
                + this->data[5] * m2[9]
                + this->data[9] * m2[10]
                + this->data[13] * m2[11];
            result[13] = this->data[0] * m2[12]
                + this->data[4] * m2[13]
                + this->data[8] * m2[14]
                + this->data[12] * m2[15];

            // Third row
            result[2] = this->data[2] * m2[0]
                + this->data[6] * m2[1]
                + this->data[10] * m2[2]
                + this->data[14] * m2[3];
            result[6] = this->data[2] * m2[4]
                + this->data[6] * m2[5]
                + this->data[10] * m2[6]
                + this->data[14] * m2[7];
            result[10] = this->data[2] * m2[8]
                + this->data[6] * m2[9]
                + this->data[10] * m2[10]
                + this->data[14] * m2[11];
            result[14] = this->data[2] * m2[12]
                + this->data[6] * m2[13]
                + this->data[10] * m2[14]
                + this->data[14] * m2[15];

            // Bottom row
            result[3] = this->data[3] * m2[0]
                + this->data[7] * m2[1]
                + this->data[11] * m2[2]
                + this->data[15] * m2[3];
            result[7] = this->data[3] * m2[4]
                + this->data[7] * m2[5]
                + this->data[11] * m2[6]
                + this->data[15] * m2[7];
            result[11] = this->data[3] * m2[8]
                + this->data[7] * m2[9]
                + this->data[11] * m2[10]
                + this->data[15] * m2[11];
            result[15] = this->data[3] * m2[12]
                + this->data[7] * m2[13]
                + this->data[11] * m2[14]
                + this->data[15] * m2[15];

            this->data.swap (result);

            return *this;
        }

        TransformMatrix<Flt> operator*= (const TransformMatrix<Flt>& m2) {

            array<Flt, 16> result;
            // Top row
            result[0] = this->data[0] * m2[0]
                + this->data[4] * m2[1]
                + this->data[8] * m2[2]
                + this->data[12] * m2[3];
            result[4] = this->data[0] * m2[4]
                + this->data[4] * m2[5]
                + this->data[8] * m2[6]
                + this->data[12] * m2[7];
            result[8] = this->data[0] * m2[8]
                + this->data[4] * m2[9]
                + this->data[8] * m2[10]
                + this->data[12] * m2[11];
            result[12] = this->data[0] * m2[12]
                + this->data[4] * m2[13]
                + this->data[8] * m2[14]
                + this->data[12] * m2[15];

            // Second row
            result[1] = this->data[1] * m2[0]
                + this->data[5] * m2[1]
                + this->data[9] * m2[2]
                + this->data[13] * m2[3];
            result[5] = this->data[1] * m2[4]
                + this->data[5] * m2[5]
                + this->data[9] * m2[6]
                + this->data[13] * m2[7];
            result[9] = this->data[1] * m2[8]
                + this->data[5] * m2[9]
                + this->data[9] * m2[10]
                + this->data[13] * m2[11];
            result[13] = this->data[0] * m2[12]
                + this->data[4] * m2[13]
                + this->data[8] * m2[14]
                + this->data[12] * m2[15];

            // Third row
            result[2] = this->data[2] * m2[0]
                + this->data[6] * m2[1]
                + this->data[10] * m2[2]
                + this->data[14] * m2[3];
            result[6] = this->data[2] * m2[4]
                + this->data[6] * m2[5]
                + this->data[10] * m2[6]
                + this->data[14] * m2[7];
            result[10] = this->data[2] * m2[8]
                + this->data[6] * m2[9]
                + this->data[10] * m2[10]
                + this->data[14] * m2[11];
            result[14] = this->data[2] * m2[12]
                + this->data[6] * m2[13]
                + this->data[10] * m2[14]
                + this->data[14] * m2[15];

            // Bottom row
            result[3] = this->data[3] * m2[0]
                + this->data[7] * m2[1]
                + this->data[11] * m2[2]
                + this->data[15] * m2[3];
            result[7] = this->data[3] * m2[4]
                + this->data[7] * m2[5]
                + this->data[11] * m2[6]
                + this->data[15] * m2[7];
            result[11] = this->data[3] * m2[8]
                + this->data[7] * m2[9]
                + this->data[11] * m2[10]
                + this->data[15] * m2[11];
            result[15] = this->data[3] * m2[12]
                + this->data[7] * m2[13]
                + this->data[11] * m2[14]
                + this->data[15] * m2[15];

            this->data.swap (result);

            return *this;
        }

        TransformMatrix<Flt> operator* (const array<Flt, 16>& m2) {

            array<Flt, 16> result;
            // Top row
            result[0] = this->data[0] * m2[0]
                + this->data[4] * m2[1]
                + this->data[8] * m2[2]
                + this->data[12] * m2[3];
            result[4] = this->data[0] * m2[4]
                + this->data[4] * m2[5]
                + this->data[8] * m2[6]
                + this->data[12] * m2[7];
            result[8] = this->data[0] * m2[8]
                + this->data[4] * m2[9]
                + this->data[8] * m2[10]
                + this->data[12] * m2[11];
            result[12] = this->data[0] * m2[12]
                + this->data[4] * m2[13]
                + this->data[8] * m2[14]
                + this->data[12] * m2[15];

            // Second row
            result[1] = this->data[1] * m2[0]
                + this->data[5] * m2[1]
                + this->data[9] * m2[2]
                + this->data[13] * m2[3];
            result[5] = this->data[1] * m2[4]
                + this->data[5] * m2[5]
                + this->data[9] * m2[6]
                + this->data[13] * m2[7];
            result[9] = this->data[1] * m2[8]
                + this->data[5] * m2[9]
                + this->data[9] * m2[10]
                + this->data[13] * m2[11];
            result[13] = this->data[0] * m2[12]
                + this->data[4] * m2[13]
                + this->data[8] * m2[14]
                + this->data[12] * m2[15];

            // Third row
            result[2] = this->data[2] * m2[0]
                + this->data[6] * m2[1]
                + this->data[10] * m2[2]
                + this->data[14] * m2[3];
            result[6] = this->data[2] * m2[4]
                + this->data[6] * m2[5]
                + this->data[10] * m2[6]
                + this->data[14] * m2[7];
            result[10] = this->data[2] * m2[8]
                + this->data[6] * m2[9]
                + this->data[10] * m2[10]
                + this->data[14] * m2[11];
            result[14] = this->data[2] * m2[12]
                + this->data[6] * m2[13]
                + this->data[10] * m2[14]
                + this->data[14] * m2[15];

            // Bottom row
            result[3] = this->data[3] * m2[0]
                + this->data[7] * m2[1]
                + this->data[11] * m2[2]
                + this->data[15] * m2[3];
            result[7] = this->data[3] * m2[4]
                + this->data[7] * m2[5]
                + this->data[11] * m2[6]
                + this->data[15] * m2[7];
            result[11] = this->data[3] * m2[8]
                + this->data[7] * m2[9]
                + this->data[11] * m2[10]
                + this->data[15] * m2[11];
            result[15] = this->data[3] * m2[12]
                + this->data[7] * m2[13]
                + this->data[11] * m2[14]
                + this->data[15] * m2[15];

            return result;
        }

        TransformMatrix<Flt> operator* (const TransformMatrix<Flt>& m2) {

            array<Flt, 16> result;
            // Top row
            result[0] = this->data[0] * m2[0]
                + this->data[4] * m2[1]
                + this->data[8] * m2[2]
                + this->data[12] * m2[3];
            result[4] = this->data[0] * m2[4]
                + this->data[4] * m2[5]
                + this->data[8] * m2[6]
                + this->data[12] * m2[7];
            result[8] = this->data[0] * m2[8]
                + this->data[4] * m2[9]
                + this->data[8] * m2[10]
                + this->data[12] * m2[11];
            result[12] = this->data[0] * m2[12]
                + this->data[4] * m2[13]
                + this->data[8] * m2[14]
                + this->data[12] * m2[15];

            // Second row
            result[1] = this->data[1] * m2[0]
                + this->data[5] * m2[1]
                + this->data[9] * m2[2]
                + this->data[13] * m2[3];
            result[5] = this->data[1] * m2[4]
                + this->data[5] * m2[5]
                + this->data[9] * m2[6]
                + this->data[13] * m2[7];
            result[9] = this->data[1] * m2[8]
                + this->data[5] * m2[9]
                + this->data[9] * m2[10]
                + this->data[13] * m2[11];
            result[13] = this->data[0] * m2[12]
                + this->data[4] * m2[13]
                + this->data[8] * m2[14]
                + this->data[12] * m2[15];

            // Third row
            result[2] = this->data[2] * m2[0]
                + this->data[6] * m2[1]
                + this->data[10] * m2[2]
                + this->data[14] * m2[3];
            result[6] = this->data[2] * m2[4]
                + this->data[6] * m2[5]
                + this->data[10] * m2[6]
                + this->data[14] * m2[7];
            result[10] = this->data[2] * m2[8]
                + this->data[6] * m2[9]
                + this->data[10] * m2[10]
                + this->data[14] * m2[11];
            result[14] = this->data[2] * m2[12]
                + this->data[6] * m2[13]
                + this->data[10] * m2[14]
                + this->data[14] * m2[15];

            // Bottom row
            result[3] = this->data[3] * m2[0]
                + this->data[7] * m2[1]
                + this->data[11] * m2[2]
                + this->data[15] * m2[3];
            result[7] = this->data[3] * m2[4]
                + this->data[7] * m2[5]
                + this->data[11] * m2[6]
                + this->data[15] * m2[7];
            result[11] = this->data[3] * m2[8]
                + this->data[7] * m2[9]
                + this->data[11] * m2[10]
                + this->data[15] * m2[11];
            result[15] = this->data[3] * m2[12]
                + this->data[7] * m2[13]
                + this->data[11] * m2[14]
                + this->data[15] * m2[15];

            return result;
        }

        void perspective (Flt fovDeg, Flt aspect, Flt zNear, Flt zFar) {
            Flt fovRad = fovDeg * piOver360; // fovDeg/2 converted to radians
            Flt sineFov = std::sin (fovRad);
            Flt cotanFov = std::cos (fovRad) / sineFov;
            Flt clip = zFar - zNear;

            // Perspective matrix to multiply self by
            array<Flt, 16> persMat;
            persMat.fill (0.0);
            persMat[0] = cotanFov/aspect;
            persMat[5] = cotanFov;
            persMat[10] = -(zNear+zFar)/clip;
            persMat[11] = -(2.0 * zNear * zFar)/clip;
            persMat[14] = -1.0;

            (*this) *= persMat;
        }
    };

} // namespace morph

#endif // _TRANSFORMMATRIX_H_
