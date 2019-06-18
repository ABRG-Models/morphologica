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

#include <iostream>
using std::cout;
using std::endl;

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
        //! Default constructor
        TransformMatrix (void) { this->setToIdentity(); }

        /*!
         * The transformation matrix data, arranged in column major
         * format to be OpenGL friendly.
         */
        alignas(array<Flt, 16>) array<Flt, 16> mat;

        //! Output to stdout
        void output (void) const {
            cout << "| " << mat[0] << " , " << mat[4] << " , " << mat[8] << " , " << mat[12] << " |\n";
            cout << "| " << mat[1] << " , " << mat[5] << " , " << mat[9] << " , " << mat[13] << " |\n";
            cout << "| " << mat[2] << " , " << mat[6] << " , " << mat[10] << " , " << mat[14] << " |\n";
            cout << "| " << mat[3] << " , " << mat[7] << " , " << mat[11] << " , " << mat[15] << " |\n";
        }

        //! Output array to stdout
        static void output (const array<Flt, 16>& arr) {
            cout << "| " << arr[0] << " , " << arr[4] << " , " << arr[8] << " , " << arr[12] << " |\n";
            cout << "| " << arr[1] << " , " << arr[5] << " , " << arr[9] << " , " << arr[13] << " |\n";
            cout << "| " << arr[2] << " , " << arr[6] << " , " << arr[10] << " , " << arr[14] << " |\n";
            cout << "| " << arr[3] << " , " << arr[7] << " , " << arr[11] << " , " << arr[15] << " |\n";
        }

        //! Self-explanatory
        void setToIdentity (void) {
            this->mat.fill (static_cast<Flt>(0.0));
            this->mat[0] = static_cast<Flt>(1.0);
            this->mat[5] = static_cast<Flt>(1.0);
            this->mat[10] = static_cast<Flt>(1.0);
            this->mat[15] = static_cast<Flt>(1.0);
        }

        void translate (const Vector3<Flt>& v) {
            this->mat[12] += v.x;
            this->mat[13] += v.y;
            this->mat[14] += v.z;
        }

        void translate (const Flt& x, const Flt& y, const Flt& z) {
            this->mat[12] += x;
            this->mat[13] += y;
            this->mat[14] += z;
        }

        /*!
         * This algorithm was obtained from:
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

        void operator*= (const array<Flt, 16>& m2) {

            array<Flt, 16> result;
            // Top row
            result[0] = this->mat[0] * m2[0]
                + this->mat[4] * m2[1]
                + this->mat[8] * m2[2]
                + this->mat[12] * m2[3];
            result[4] = this->mat[0] * m2[4]
                + this->mat[4] * m2[5]
                + this->mat[8] * m2[6]
                + this->mat[12] * m2[7];
            result[8] = this->mat[0] * m2[8]
                + this->mat[4] * m2[9]
                + this->mat[8] * m2[10]
                + this->mat[12] * m2[11];
            result[12] = this->mat[0] * m2[12]
                + this->mat[4] * m2[13]
                + this->mat[8] * m2[14]
                + this->mat[12] * m2[15];

            // Second row
            result[1] = this->mat[1] * m2[0]
                + this->mat[5] * m2[1]
                + this->mat[9] * m2[2]
                + this->mat[13] * m2[3];
            result[5] = this->mat[1] * m2[4]
                + this->mat[5] * m2[5]
                + this->mat[9] * m2[6]
                + this->mat[13] * m2[7];
            result[9] = this->mat[1] * m2[8]
                + this->mat[5] * m2[9]
                + this->mat[9] * m2[10]
                + this->mat[13] * m2[11];
            result[13] = this->mat[1] * m2[12]
                + this->mat[5] * m2[13]
                + this->mat[9] * m2[14]
                + this->mat[13] * m2[15];

            // Third row
            result[2] = this->mat[2] * m2[0]
                + this->mat[6] * m2[1]
                + this->mat[10] * m2[2]
                + this->mat[14] * m2[3];
            result[6] = this->mat[2] * m2[4]
                + this->mat[6] * m2[5]
                + this->mat[10] * m2[6]
                + this->mat[14] * m2[7];
            result[10] = this->mat[2] * m2[8]
                + this->mat[6] * m2[9]
                + this->mat[10] * m2[10]
                + this->mat[14] * m2[11];
            result[14] = this->mat[2] * m2[12]
                + this->mat[6] * m2[13]
                + this->mat[10] * m2[14]
                + this->mat[14] * m2[15];

            // Bottom row
            result[3] = this->mat[3] * m2[0]
                + this->mat[7] * m2[1]
                + this->mat[11] * m2[2]
                + this->mat[15] * m2[3];
            result[7] = this->mat[3] * m2[4]
                + this->mat[7] * m2[5]
                + this->mat[11] * m2[6]
                + this->mat[15] * m2[7];
            result[11] = this->mat[3] * m2[8]
                + this->mat[7] * m2[9]
                + this->mat[11] * m2[10]
                + this->mat[15] * m2[11];
            result[15] = this->mat[3] * m2[12]
                + this->mat[7] * m2[13]
                + this->mat[11] * m2[14]
                + this->mat[15] * m2[15];

            this->mat.swap (result);
        }

        void operator*= (const TransformMatrix<Flt>& m2) {

            array<Flt, 16> result;
            // Top row
            result[0] = this->mat[0] * m2.mat[0]
                + this->mat[4] * m2.mat[1]
                + this->mat[8] * m2.mat[2]
                + this->mat[12] * m2.mat[3];
            result[4] = this->mat[0] * m2.mat[4]
                + this->mat[4] * m2.mat[5]
                + this->mat[8] * m2.mat[6]
                + this->mat[12] * m2.mat[7];
            result[8] = this->mat[0] * m2.mat[8]
                + this->mat[4] * m2.mat[9]
                + this->mat[8] * m2.mat[10]
                + this->mat[12] * m2.mat[11];
            result[12] = this->mat[0] * m2.mat[12]
                + this->mat[4] * m2.mat[13]
                + this->mat[8] * m2.mat[14]
                + this->mat[12] * m2.mat[15];

            // Second row
            result[1] = this->mat[1] * m2.mat[0]
                + this->mat[5] * m2.mat[1]
                + this->mat[9] * m2.mat[2]
                + this->mat[13] * m2.mat[3];
            result[5] = this->mat[1] * m2.mat[4]
                + this->mat[5] * m2.mat[5]
                + this->mat[9] * m2.mat[6]
                + this->mat[13] * m2.mat[7];
            result[9] = this->mat[1] * m2.mat[8]
                + this->mat[5] * m2.mat[9]
                + this->mat[9] * m2.mat[10]
                + this->mat[13] * m2.mat[11];
            result[13] = this->mat[1] * m2.mat[12]
                + this->mat[5] * m2.mat[13]
                + this->mat[9] * m2.mat[14]
                + this->mat[13] * m2.mat[15];

            // Third row
            result[2] = this->mat[2] * m2.mat[0]
                + this->mat[6] * m2.mat[1]
                + this->mat[10] * m2.mat[2]
                + this->mat[14] * m2.mat[3];
            result[6] = this->mat[2] * m2.mat[4]
                + this->mat[6] * m2.mat[5]
                + this->mat[10] * m2.mat[6]
                + this->mat[14] * m2.mat[7];
            result[10] = this->mat[2] * m2.mat[8]
                + this->mat[6] * m2.mat[9]
                + this->mat[10] * m2.mat[10]
                + this->mat[14] * m2.mat[11];
            result[14] = this->mat[2] * m2.mat[12]
                + this->mat[6] * m2.mat[13]
                + this->mat[10] * m2.mat[14]
                + this->mat[14] * m2.mat[15];

            // Bottom row
            result[3] = this->mat[3] * m2.mat[0]
                + this->mat[7] * m2.mat[1]
                + this->mat[11] * m2.mat[2]
                + this->mat[15] * m2.mat[3];
            result[7] = this->mat[3] * m2.mat[4]
                + this->mat[7] * m2.mat[5]
                + this->mat[11] * m2.mat[6]
                + this->mat[15] * m2.mat[7];
            result[11] = this->mat[3] * m2.mat[8]
                + this->mat[7] * m2.mat[9]
                + this->mat[11] * m2.mat[10]
                + this->mat[15] * m2.mat[11];
            result[15] = this->mat[3] * m2.mat[12]
                + this->mat[7] * m2.mat[13]
                + this->mat[11] * m2.mat[14]
                + this->mat[15] * m2.mat[15];

            this->mat.swap (result);
        }

        TransformMatrix<Flt> operator* (const array<Flt, 16>& m2) const {

            TransformMatrix<Flt> result;
            // Top row
            result.mat[0] = this->mat[0] * m2[0]
                + this->mat[4] * m2[1]
                + this->mat[8] * m2[2]
                + this->mat[12] * m2[3];
            result.mat[4] = this->mat[0] * m2[4]
                + this->mat[4] * m2[5]
                + this->mat[8] * m2[6]
                + this->mat[12] * m2[7];
            result.mat[8] = this->mat[0] * m2[8]
                + this->mat[4] * m2[9]
                + this->mat[8] * m2[10]
                + this->mat[12] * m2[11];
            result.mat[12] = this->mat[0] * m2[12]
                + this->mat[4] * m2[13]
                + this->mat[8] * m2[14]
                + this->mat[12] * m2[15];

            // Second row
            result.mat[1] = this->mat[1] * m2[0]
                + this->mat[5] * m2[1]
                + this->mat[9] * m2[2]
                + this->mat[13] * m2[3];
            result.mat[5] = this->mat[1] * m2[4]
                + this->mat[5] * m2[5]
                + this->mat[9] * m2[6]
                + this->mat[13] * m2[7];
            result.mat[9] = this->mat[1] * m2[8]
                + this->mat[5] * m2[9]
                + this->mat[9] * m2[10]
                + this->mat[13] * m2[11];
            result.mat[13] = this->mat[1] * m2[12]
                + this->mat[5] * m2[13]
                + this->mat[9] * m2[14]
                + this->mat[13] * m2[15];

            // Third row
            result.mat[2] = this->mat[2] * m2[0]
                + this->mat[6] * m2[1]
                + this->mat[10] * m2[2]
                + this->mat[14] * m2[3];
            result.mat[6] = this->mat[2] * m2[4]
                + this->mat[6] * m2[5]
                + this->mat[10] * m2[6]
                + this->mat[14] * m2[7];
            result.mat[10] = this->mat[2] * m2[8]
                + this->mat[6] * m2[9]
                + this->mat[10] * m2[10]
                + this->mat[14] * m2[11];
            result.mat[14] = this->mat[2] * m2[12]
                + this->mat[6] * m2[13]
                + this->mat[10] * m2[14]
                + this->mat[14] * m2[15];

            // Bottom row
            result.mat[3] = this->mat[3] * m2[0]
                + this->mat[7] * m2[1]
                + this->mat[11] * m2[2]
                + this->mat[15] * m2[3];
            result.mat[7] = this->mat[3] * m2[4]
                + this->mat[7] * m2[5]
                + this->mat[11] * m2[6]
                + this->mat[15] * m2[7];
            result.mat[11] = this->mat[3] * m2[8]
                + this->mat[7] * m2[9]
                + this->mat[11] * m2[10]
                + this->mat[15] * m2[11];
            result.mat[15] = this->mat[3] * m2[12]
                + this->mat[7] * m2[13]
                + this->mat[11] * m2[14]
                + this->mat[15] * m2[15];

            return result;
        }

        TransformMatrix<Flt> operator* (const TransformMatrix<Flt>& m2) const {

            TransformMatrix<Flt> result;
            // Top row
            result.mat[0] = this->mat[0] * m2.mat[0]
                + this->mat[4] * m2.mat[1]
                + this->mat[8] * m2.mat[2]
                + this->mat[12] * m2.mat[3];
            result.mat[4] = this->mat[0] * m2.mat[4]
                + this->mat[4] * m2.mat[5]
                + this->mat[8] * m2.mat[6]
                + this->mat[12] * m2.mat[7];
            result.mat[8] = this->mat[0] * m2.mat[8]
                + this->mat[4] * m2.mat[9]
                + this->mat[8] * m2.mat[10]
                + this->mat[12] * m2.mat[11];
            result.mat[12] = this->mat[0] * m2.mat[12]
                + this->mat[4] * m2.mat[13]
                + this->mat[8] * m2.mat[14]
                + this->mat[12] * m2.mat[15];

            // Second row
            result.mat[1] = this->mat[1] * m2.mat[0]
                + this->mat[5] * m2.mat[1]
                + this->mat[9] * m2.mat[2]
                + this->mat[13] * m2.mat[3];
            result.mat[5] = this->mat[1] * m2.mat[4]
                + this->mat[5] * m2.mat[5]
                + this->mat[9] * m2.mat[6]
                + this->mat[13] * m2.mat[7];
            result.mat[9] = this->mat[1] * m2.mat[8]
                + this->mat[5] * m2.mat[9]
                + this->mat[9] * m2.mat[10]
                + this->mat[13] * m2.mat[11];
            result.mat[13] = this->mat[1] * m2.mat[12]
                + this->mat[5] * m2.mat[13]
                + this->mat[9] * m2.mat[14]
                + this->mat[13] * m2.mat[15];

            // Third row
            result.mat[2] = this->mat[2] * m2.mat[0]
                + this->mat[6] * m2.mat[1]
                + this->mat[10] * m2.mat[2]
                + this->mat[14] * m2.mat[3];
            result.mat[6] = this->mat[2] * m2.mat[4]
                + this->mat[6] * m2.mat[5]
                + this->mat[10] * m2.mat[6]
                + this->mat[14] * m2.mat[7];
            result.mat[10] = this->mat[2] * m2.mat[8]
                + this->mat[6] * m2.mat[9]
                + this->mat[10] * m2.mat[10]
                + this->mat[14] * m2.mat[11];
            result.mat[14] = this->mat[2] * m2.mat[12]
                + this->mat[6] * m2.mat[13]
                + this->mat[10] * m2.mat[14]
                + this->mat[14] * m2.mat[15];

            // Bottom row
            result.mat[3] = this->mat[3] * m2.mat[0]
                + this->mat[7] * m2.mat[1]
                + this->mat[11] * m2.mat[2]
                + this->mat[15] * m2.mat[3];
            result.mat[7] = this->mat[3] * m2.mat[4]
                + this->mat[7] * m2.mat[5]
                + this->mat[11] * m2.mat[6]
                + this->mat[15] * m2.mat[7];
            result.mat[11] = this->mat[3] * m2.mat[8]
                + this->mat[7] * m2.mat[9]
                + this->mat[11] * m2.mat[10]
                + this->mat[15] * m2.mat[11];
            result.mat[15] = this->mat[3] * m2.mat[12]
                + this->mat[7] * m2.mat[13]
                + this->mat[11] * m2.mat[14]
                + this->mat[15] * m2.mat[15];

            return result;
        }

        void operator*= (const float& f) {
            for (unsigned int i = 0; i<16; ++i) {
                this->mat[i] *= f;
            }
        }

        //! Transpose this matrix
        void transpose (void) {
            array<Flt, 6> a;
            a[0] = this->mat[4];
            a[1] = this->mat[8];
            a[2] = this->mat[9];
            a[3] = this->mat[12];
            a[4] = this->mat[13];
            a[5] = this->mat[14];

            this->mat[4] = this->mat[1];
            this->mat[8] = this->mat[2];
            this->mat[9] = this->mat[6];
            this->mat[12] = this->mat[3];
            this->mat[13] = this->mat[7];
            this->mat[14] = this->mat[11];

            this->mat[1] = a[0];  // mat[4]
            this->mat[2] = a[1];  // mat[8]
            this->mat[3] = a[3];  // mat[12]
            this->mat[6] = a[2];  // mat[9]
            this->mat[7] = a[4];  // mat[13]
            this->mat[11] = a[5]; // mat[14]
        }

        //! Make a perspective projection
        void perspective (Flt fovDeg, Flt aspect, Flt zNear, Flt zFar) {

            // Bail out if the projection volume is zero-sized.
            if (zNear == zFar || aspect == 0.0f) {
                return;
            }

            Flt fovRad = fovDeg * piOver360; // fovDeg/2 converted to radians
            Flt sineFov = std::sin (fovRad);
            if (sineFov == static_cast<Flt>(0.0)) {
                return;
            }
            Flt cotanFov = std::cos (fovRad) / sineFov;
            Flt clip = zFar - zNear;

            // Perspective matrix to multiply self by
            array<Flt, 16> persMat;
            persMat.fill (0.0);
            persMat[0] = cotanFov/aspect;
            persMat[5] = cotanFov;
            persMat[10] = -(zNear+zFar)/clip;
            persMat[11] = -1.0;
            persMat[14] = -(2.0 * zNear * zFar)/clip;

            (*this) *= persMat;
        }
    };

} // namespace morph

#endif // _TRANSFORMMATRIX_H_
