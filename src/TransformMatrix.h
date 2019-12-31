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
            cout <<"| "<< mat[0]<<" , "<<mat[4]<<" , "<<mat[8]<<" , "<<mat[12]<<" |\n";
            cout <<"| "<< mat[1]<<" , "<<mat[5]<<" , "<<mat[9]<<" , "<<mat[13]<<" |\n";
            cout <<"| "<< mat[2]<<" , "<<mat[6]<<" , "<<mat[10]<<" , "<<mat[14]<<" |\n";
            cout <<"| "<< mat[3]<<" , "<<mat[7]<<" , "<<mat[11]<<" , "<<mat[15]<<" |\n";
        }

        //! Output array to stdout
        static void output (const array<Flt, 16>& arr) {
            cout<<"| "<<arr[0]<<" , "<<arr[4]<<" , "<<arr[8]<<" , "<<arr[12]<<" |\n";
            cout<<"| "<<arr[1]<<" , "<<arr[5]<<" , "<<arr[9]<<" , "<<arr[13]<<" |\n";
            cout<<"| "<<arr[2]<<" , "<<arr[6]<<" , "<<arr[10]<<" , "<<arr[14]<<" |\n";
            cout<<"| "<<arr[3]<<" , "<<arr[7]<<" , "<<arr[11]<<" , "<<arr[15]<<" |\n";
        }

        //! Output adj array (in row-major format) to stdout
        static void outputadj (const array<Flt, 32>& arr) {
            for (unsigned int i = 0; i<4; ++i) {
                cout << "| ";
                for (unsigned int j = i*8; j < (i*8)+8; ++j) {
                    cout << arr[j] << ",";
                }
                cout << endl;
            }
        }

        //! Self-explanatory
        void setToIdentity (void) {
            this->mat.fill (static_cast<Flt>(0.0));
            this->mat[0] = static_cast<Flt>(1.0);
            this->mat[5] = static_cast<Flt>(1.0);
            this->mat[10] = static_cast<Flt>(1.0);
            this->mat[15] = static_cast<Flt>(1.0);
        }

        //! Apply translation specified by vector @v
        void translate (const Vector3<Flt>& v) {
            this->mat[12] += v.x;
            this->mat[13] += v.y;
            this->mat[14] += v.z;
        }

        //! Apply translation specified by coordinates @x, @y and @z.
        void translate (const Flt& x, const Flt& y, const Flt& z) {
            this->mat[12] += x;
            this->mat[13] += y;
            this->mat[14] += z;
        }

        //! Compute determinant for 3x3 matrix @cm
        Flt determinant (array<Flt, 9> cm) const {
            Flt det = (cm[0]*cm[4]*cm[8])
                + (cm[3]*cm[7]*cm[2])
                + (cm[6]*cm[1]*cm[5])
                - (cm[6]*cm[4]*cm[2])
                - (cm[0]*cm[7]*cm[5])
                - (cm[3]*cm[1]*cm[8]);
#ifdef DEBUG__
            cout << "Determinant of \n"
                 << "| " << cm[0] << " , " << cm[3] << " , " << cm[6] << " |\n"
                 << "| " << cm[1] << " , " << cm[4] << " , " << cm[7] << " |\n"
                 << "| " << cm[2] << " , " << cm[5] << " , " << cm[8] << " | is...\n"
                 << det << endl;
#endif
            return det;
        }

        //! Compute determinant for 4x4 matrix @cm
        Flt determinant (array<Flt, 16> cm) const {
            // Configure the 3x3 matrices that have to be evaluated to get the 4x4 det.
            array<Flt, 9> cm00;
            cm00[0] = cm[5];
            cm00[1] = cm[6];
            cm00[2] = cm[7];
            cm00[3] = cm[9];
            cm00[4] = cm[10];
            cm00[5] = cm[11];
            cm00[6] = cm[13];
            cm00[7] = cm[14];
            cm00[8] = cm[15];

            array<Flt, 9> cm01;
            cm01[0] = cm[1];
            cm01[1] = cm[2];
            cm01[2] = cm[3];
            cm01[3] = cm[9];
            cm01[4] = cm[10];
            cm01[5] = cm[11];
            cm01[6] = cm[13];
            cm01[7] = cm[14];
            cm01[8] = cm[15];

            array<Flt, 9> cm02;
            cm02[0] = cm[1];
            cm02[1] = cm[2];
            cm02[2] = cm[3];
            cm02[3] = cm[5];
            cm02[4] = cm[6];
            cm02[5] = cm[7];
            cm02[6] = cm[13];
            cm02[7] = cm[14];
            cm02[8] = cm[15];

            array<Flt, 9> cm03;
            cm03[0] = cm[1];
            cm03[1] = cm[2];
            cm03[2] = cm[3];
            cm03[3] = cm[5];
            cm03[4] = cm[6];
            cm03[5] = cm[7];
            cm03[6] = cm[9];
            cm03[7] = cm[10];
            cm03[8] = cm[11];

            Flt det = cm[0] * this->determinant (cm00)
                - cm[4] * this->determinant (cm01)
                + cm[8] * this->determinant (cm02)
                - cm[12] * this->determinant (cm03);
#ifdef DEBUG__
            cout << "Determinant of \n"
                 << "| " << cm[0] << " , " << cm[4] << " , " << cm[8] << " , " << cm[12] << " |\n"
                 << "| " << cm[1] << " , " << cm[5] << " , " << cm[9] << " , " << cm[13] << " |\n"
                 << "| " << cm[2] << " , " << cm[6] << " , " << cm[10] << " , " << cm[14] << " |\n"
                 << "| " << cm[3] << " , " << cm[7] << " , " << cm[11] << " , " << cm[15] << " |\n"
                 << det << endl;
#endif
            return det;
        }

        //! The adjugate is the transpose of the cofactor matrix
        array<Flt, 16> adjugate (void) const {
            array<Flt, 16> adj = this->transpose (this->cofactor());
            return adj;
        }

        //! Compute the cofactor matrix of this->mat
        array<Flt, 16> cofactor (void) const {
            array<Flt, 16> cofac;

            // Keep to column-major format for all matrices. The cofactor matrix is
            // actually populated, applying the alternating pattern of +/- as we go.

            // 0.
            array<Flt, 9> minorElem;
            minorElem[0] = this->mat[5];
            minorElem[3] = this->mat[9];
            minorElem[6] = this->mat[13];

            minorElem[1] = this->mat[6];
            minorElem[4] = this->mat[10];
            minorElem[7] = this->mat[14];

            minorElem[2] = this->mat[7];
            minorElem[5] = this->mat[11];
            minorElem[8] = this->mat[15];

            cofac[0] = this->determinant (minorElem);

            // 1. Next minor elem matrix has only 3 elements changed
            minorElem[0] = this->mat[4];
            minorElem[3] = this->mat[8];
            minorElem[6] = this->mat[12];
            cofac[1] = -this->determinant (minorElem);

            // 2
            minorElem[1] = this->mat[5];
            minorElem[4] = this->mat[9];
            minorElem[7] = this->mat[13];
            cofac[2] = this->determinant (minorElem);

            // 3
            minorElem[2] = this->mat[6];
            minorElem[5] = this->mat[10];
            minorElem[8] = this->mat[14];
            cofac[3] = -this->determinant (minorElem);

            // 4.
            minorElem[0] = this->mat[1];
            minorElem[3] = this->mat[9];
            minorElem[6] = this->mat[13];

            minorElem[1] = this->mat[2];
            minorElem[4] = this->mat[10];
            minorElem[7] = this->mat[14];

            minorElem[2] = this->mat[3];
            minorElem[5] = this->mat[11];
            minorElem[8] = this->mat[15];

            cofac[4] = -this->determinant (minorElem);

            // 5.
            minorElem[0] = this->mat[0];
            minorElem[3] = this->mat[8];
            minorElem[6] = this->mat[12];
            cofac[5] = this->determinant (minorElem);

            // 6.
            minorElem[1] = this->mat[1];
            minorElem[4] = this->mat[9];
            minorElem[7] = this->mat[13];
            cofac[6] = -this->determinant (minorElem);

            // 7.
            minorElem[2] = this->mat[2];
            minorElem[5] = this->mat[10];
            minorElem[8] = this->mat[14];
            cofac[7] = this->determinant (minorElem);

            // 8.
            minorElem[0] = this->mat[1];
            minorElem[3] = this->mat[5];
            minorElem[6] = this->mat[13];

            minorElem[1] = this->mat[2];
            minorElem[4] = this->mat[6];
            minorElem[7] = this->mat[14];

            minorElem[2] = this->mat[3];
            minorElem[5] = this->mat[7];
            minorElem[8] = this->mat[15];

            cofac[8] = this->determinant (minorElem);

            // 9.
            minorElem[0] = this->mat[0];
            minorElem[3] = this->mat[4];
            minorElem[6] = this->mat[12];
            cofac[9] = -this->determinant (minorElem);

            // 10.
            minorElem[1] = this->mat[1];
            minorElem[4] = this->mat[5];
            minorElem[7] = this->mat[13];
            cofac[10] = this->determinant (minorElem);

            // 11.
            minorElem[2] = this->mat[2];
            minorElem[5] = this->mat[6];
            minorElem[8] = this->mat[14];
            cofac[11] = -this->determinant (minorElem);

            // 12.
            minorElem[0] = this->mat[1];
            minorElem[3] = this->mat[5];
            minorElem[6] = this->mat[9];

            minorElem[1] = this->mat[2];
            minorElem[4] = this->mat[6];
            minorElem[7] = this->mat[10];

            minorElem[2] = this->mat[3];
            minorElem[5] = this->mat[7];
            minorElem[8] = this->mat[11];

            cofac[12] = -this->determinant (minorElem);

            // 13.
            minorElem[0] = this->mat[0];
            minorElem[3] = this->mat[4];
            minorElem[6] = this->mat[8];
            cofac[13] = this->determinant (minorElem);

            // 14.
            minorElem[1] = this->mat[1];
            minorElem[4] = this->mat[5];
            minorElem[7] = this->mat[9];
            cofac[14] = -this->determinant (minorElem);

            // 15.
            minorElem[2] = this->mat[2];
            minorElem[5] = this->mat[6];
            minorElem[8] = this->mat[10];
            cofac[15] = this->determinant (minorElem);

            return cofac;
        }

        /*!
         * Implement inversion using determinant method. inverse is (1/det) x adjugate
         * matrix.
         */
        TransformMatrix<Flt> invert (void) {
            // 1. Create matrix of minors
            // array<Flt, 16> minors = this->makeminors();
            // 2. Multiply mofminors by a checkerboard pattern to give the cofactor matrix
            // 3. Compute determinant of this->mat (if 0, there's no inverse)
            // 4. multiply 1/determinant of mat by the adjugate of mat (transpose of
            //    cofactor matrix) to get inverse.
            Flt det = this->determinant (this->mat);
            TransformMatrix<Flt> rtn;
            if (det == static_cast<Flt>(0.0)) {
                // Then there's no inverse
                rtn.mat.fill (static_cast<Flt>(0.0));
            } else {
                array<Flt, 16> adjugate = this->adjugate();
                rtn.mat = adjugate;
                rtn *= (static_cast<Flt>(1.0)/det);
            }
            return rtn;
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

        //! Rotate, but this time with a Quaternion made of doubles, rather than floats.
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

        //! Right-multiply this->mat with m2.
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

        //! Right-multiply this->mat with m2.mat.
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

        //! Right multiply this->mat with m2.mat.
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

        //! Do matrix times vector multiplication, v = mat * v1
        array<Flt, 4> operator* (const array<Flt, 4>& v1) const {
            array<Flt, 4> v;
            v[0] = this->mat[0] * v1[0]
                + this->mat[4] * v1[1]
                + this->mat[8] * v1[2]
                + this->mat[12] * v1[3];
            v[1] = this->mat[1] * v1[0]
                + this->mat[5] * v1[1]
                + this->mat[9] * v1[2]
                + this->mat[13] * v1[3];
            v[2] = this->mat[2] * v1[0]
                + this->mat[6] * v1[1]
                + this->mat[10] * v1[2]
                + this->mat[14] * v1[3];
            v[3] = this->mat[3] * v1[0]
                + this->mat[7] * v1[1]
                + this->mat[11] * v1[2]
                + this->mat[15] * v1[3];
            return v;
        }

        void operator*= (const float& f) {
            for (unsigned int i = 0; i<16; ++i) {
                this->mat[i] *= f;
            }
        }

        void operator*= (const double& f) {
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

        //! Transpose the matrix @matrx, returning the transposed version.
        array<Flt, 16> transpose (const array<Flt, 16>& matrx) const {
            array<Flt, 16> tposed;
            tposed[0] = matrx[0];
            tposed[4] = matrx[1];
            tposed[8] = matrx[2];
            tposed[12] = matrx[3];
            tposed[1] = matrx[4];
            tposed[5] = matrx[5];
            tposed[9] = matrx[6];
            tposed[13] = matrx[7];
            tposed[2] = matrx[8];
            tposed[6] = matrx[9];
            tposed[10] = matrx[10];
            tposed[14] = matrx[11];
            tposed[3] = matrx[12];
            tposed[7] = matrx[13];
            tposed[11] = matrx[14];
            tposed[15] = matrx[15];
            return tposed;
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
