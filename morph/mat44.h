/*!
 * \file
 *
 * A 4x4 Transformation matrix class
 *
 * \author Seb James
 * \date 2019
 */
#pragma once

#include <morph/mathconst.h>
#include <morph/quaternion.h>
#include <morph/vec.h>
#include <morph/constexpr_math.h>
#include <array>
#include <string>
#include <sstream>
#include <iostream>
#include <type_traits>

namespace morph {

    // Forward declare class and stream operator
    template <typename F> class mat44;
    template <typename F> std::ostream& operator<< (std::ostream&, const mat44<F>&);

    /*!
     * This implements a 4x4 transformation matrix, for use in computer graphics
     * applications in which 3D coordinates are often given as 4D homogeneous
     * coordinates (with the fourth element chosen to be equal to 1). It's used a lot in
     * morph::Visual. The matrix data is stored in mat44::mat, an array of 16
     * floating point numbers.
     *
     * \templateparam F The floating point type in which to store the
     * mat44's data.
     */
    template <typename F>
    class mat44
    {
    public:
        //! Default constructor
        constexpr mat44() noexcept { this->setToIdentity(); }
        //! User-declared copy constructor
        constexpr mat44 (const mat44<F>& other) noexcept : mat(other.mat) {}
        //! User-declared copy assignment constructor
        constexpr mat44<F>& operator= (const mat44<F>& other) noexcept
        {
            std::copy (other.mat.begin(), other.mat.end(), mat.begin());
            return *this;
        }
        //! Explicitly defaulted  move constructor
        mat44 (mat44<F>&& other) noexcept = default;
        //! Explicitly defaulted move assignment constructor
        mat44<F>& operator= (mat44<F>&& other) noexcept = default;

        /*!
         * The transformation matrix data, arranged in column major format to be OpenGL
         * friendly.
         */
        alignas(std::array<F, 16>) std::array<F, 16> mat;

        //! Return a string representation of the matrix
        std::string str() const noexcept
        {
            std::stringstream ss;
            ss <<"[ "<< mat[0]<<" , "<<mat[4]<<" , "<<mat[8]<<" , "<<mat[12]<<" ;\n";
            ss <<"  "<< mat[1]<<" , "<<mat[5]<<" , "<<mat[9]<<" , "<<mat[13]<<" ;\n";
            ss <<"  "<< mat[2]<<" , "<<mat[6]<<" , "<<mat[10]<<" , "<<mat[14]<<" ;\n";
            ss <<"  "<< mat[3]<<" , "<<mat[7]<<" , "<<mat[11]<<" , "<<mat[15]<<" ]";
            return ss.str();
        }

        //! Return a string representation of the passed-in column-major array
        static std::string str (const std::array<F, 16>& arr) noexcept
        {
            std::stringstream ss;
            ss <<"[ "<< arr[0]<<" , "<<arr[4]<<" , "<<arr[8]<<" , "<<arr[12]<<" ;\n";
            ss <<"  "<< arr[1]<<" , "<<arr[5]<<" , "<<arr[9]<<" , "<<arr[13]<<" ;\n";
            ss <<"  "<< arr[2]<<" , "<<arr[6]<<" , "<<arr[10]<<" , "<<arr[14]<<" ;\n";
            ss <<"  "<< arr[3]<<" , "<<arr[7]<<" , "<<arr[11]<<" , "<<arr[15]<<" ]";
            return ss.str();
        }

        //! Self-explanatory
        constexpr void setToIdentity() noexcept
        {
            this->mat.fill (F{0});
            this->mat[0] = F{1};
            this->mat[5] = F{1};
            this->mat[10] = F{1};
            this->mat[15] = F{1};
        }

        //! Access elements of the matrix
        constexpr F& operator[] (unsigned int idx) noexcept { return this->mat[idx]; }

        //! Access a given row of the matrix
        constexpr morph::vec<F, 4> row (unsigned int idx) const noexcept
        {
            morph::vec<F, 4> r = {F{0}, F{0}, F{0}, F{0}};
            if (idx > 3U) { return r; }
            r[0] = this->mat[idx];
            r[1] = this->mat[idx+4];
            r[2] = this->mat[idx+8];
            r[3] = this->mat[idx+12];
            return r;
        }

        //! Access a given column of the matrix
        constexpr morph::vec<F, 4> col (unsigned int idx) const noexcept
        {
            morph::vec<F, 4> c = {F{0}, F{0}, F{0}, F{0}};
            if (idx > 3U) { return c; }
            idx *= 4U;
            c[0] = this->mat[idx];
            c[1] = this->mat[++idx];
            c[2] = this->mat[++idx];
            c[3] = this->mat[++idx];
            return c;
        }

#ifdef DETERMINE_FROM_PLACEHOLDER
        //! Compute a morphing transformation to turn simplex(ABCD) into simplex(EFGH). Avoid
        //! reflection. Keep the 'order' of ABC in DEF; if ABC defines a clockwise order
        //! of vertices, then so should DEF.
        void determineFrom (std::array<vec<F,4>, 4>& abcd,
                            std::array<vec<F,4>, 4>& efdg) noexcept
        {
            // The transformation is quite simple to determine. abcd defines a triangle
            // as 3 columns of 4-vectors (in homogeneous coordinates); efgh defines a
            // similar triangle as the target. Let there be a transformation, T such that
            // Let  T * abcd = efgh
            //   => T * abcd * inv(abcd) = efgh * inv(abc)
            //   => T = efgh * inv(abcd)
        }
#endif
        //! Apply translation specified by vector @dv
        template<typename T> requires std::is_arithmetic_v<T>
        constexpr void translate (const vec<T, 3>& dv) noexcept
        {
            this->mat[12] += dv[0];
            this->mat[13] += dv[1];
            this->mat[14] += dv[2];
        }

        //! Apply translation specified by vector @dv provided as array of three coordinates
        template<typename T> requires std::is_arithmetic_v<T>
        constexpr void translate (const std::array<T, 3>& dv) noexcept
        {
            this->mat[12] += dv[0];
            this->mat[13] += dv[1];
            this->mat[14] += dv[2];
        }

        //! Apply translation specified by coordinates @dx, @dy and @dz.
        template<typename T> requires std::is_arithmetic_v<T>
        constexpr void translate (const T& dx, const T& dy, const T& dz) noexcept
        {
            this->mat[12] += dx;
            this->mat[13] += dy;
            this->mat[14] += dz;
        }

        //! Scaling transformation by individual dims
        template<typename T> requires std::is_arithmetic_v<T>
        constexpr void scale (const T& scl_x, const T& scl_y, const T& scl_z) noexcept
        {
            // This is the rotation matrix multiplied by a diagonalized matrix made from scl_x/y/z
            this->mat[0] *= scl_x;
            this->mat[1] *= scl_x;
            this->mat[2] *= scl_x;

            this->mat[4] *= scl_y;
            this->mat[5] *= scl_y;
            this->mat[6] *= scl_y;

            this->mat[8] *= scl_z;
            this->mat[9] *= scl_z;
            this->mat[10] *= scl_z;
        }

        //! Scaling transformation by vector
        template<typename T> requires std::is_arithmetic_v<T>
        constexpr void scale (const vec<T, 3>& scl) noexcept
        {
            this->mat[0] *= scl[0];
            this->mat[1] *= scl[0];
            this->mat[2] *= scl[0];

            this->mat[4] *= scl[1];
            this->mat[5] *= scl[1];
            this->mat[6] *= scl[1];

            this->mat[8] *= scl[2];
            this->mat[9] *= scl[2];
            this->mat[10] *= scl[2];
        }

        template<typename T> requires std::is_arithmetic_v<T>
        constexpr void scale (const std::array<T, 3>& scl) noexcept
        {
            this->mat[0] *= scl[0];
            this->mat[1] *= scl[0];
            this->mat[2] *= scl[0];

            this->mat[4] *= scl[1];
            this->mat[5] *= scl[1];
            this->mat[6] *= scl[1];

            this->mat[8] *= scl[2];
            this->mat[9] *= scl[2];
            this->mat[10] *= scl[2];
        }

        template<typename T> requires std::is_arithmetic_v<T>
        constexpr void scale (const T& scl) noexcept
        {
            this->mat[0] *= scl;
            this->mat[1] *= scl;
            this->mat[2] *= scl;

            this->mat[4] *= scl;
            this->mat[5] *= scl;
            this->mat[6] *= scl;

            this->mat[8] *= scl;
            this->mat[9] *= scl;
            this->mat[10] *= scl;
        }

        //! Compute determinant for 3x3 matrix @cm
        constexpr F determinant3x3 (std::array<F, 9> cm) const noexcept
        {
            F det = (cm[0]*cm[4]*cm[8])
                + (cm[3]*cm[7]*cm[2])
                + (cm[6]*cm[1]*cm[5])
                - (cm[6]*cm[4]*cm[2])
                - (cm[0]*cm[7]*cm[5])
                - (cm[3]*cm[1]*cm[8]);
            return det;
        }

        //! Compute determinant for 4x4 matrix @cm
        constexpr F determinant (std::array<F, 16> cm) const noexcept
        {
            // Configure the 3x3 matrices that have to be evaluated to get the 4x4 det.
            std::array<F, 9> cm00;
            cm00[0] = cm[5];
            cm00[1] = cm[6];
            cm00[2] = cm[7];
            cm00[3] = cm[9];
            cm00[4] = cm[10];
            cm00[5] = cm[11];
            cm00[6] = cm[13];
            cm00[7] = cm[14];
            cm00[8] = cm[15];

            std::array<F, 9> cm01;
            cm01[0] = cm[1];
            cm01[1] = cm[2];
            cm01[2] = cm[3];
            cm01[3] = cm[9];
            cm01[4] = cm[10];
            cm01[5] = cm[11];
            cm01[6] = cm[13];
            cm01[7] = cm[14];
            cm01[8] = cm[15];

            std::array<F, 9> cm02;
            cm02[0] = cm[1];
            cm02[1] = cm[2];
            cm02[2] = cm[3];
            cm02[3] = cm[5];
            cm02[4] = cm[6];
            cm02[5] = cm[7];
            cm02[6] = cm[13];
            cm02[7] = cm[14];
            cm02[8] = cm[15];

            std::array<F, 9> cm03;
            cm03[0] = cm[1];
            cm03[1] = cm[2];
            cm03[2] = cm[3];
            cm03[3] = cm[5];
            cm03[4] = cm[6];
            cm03[5] = cm[7];
            cm03[6] = cm[9];
            cm03[7] = cm[10];
            cm03[8] = cm[11];

            F det = cm[0] * this->determinant3x3 (cm00)
                - cm[4] * this->determinant3x3 (cm01)
                + cm[8] * this->determinant3x3 (cm02)
                - cm[12] * this->determinant3x3 (cm03);

            return det;
        }

        //! Compute determinant for this->mat
        constexpr F determinant() const noexcept
        {
            // Configure the 3x3 matrices that have to be evaluated to get the 4x4 det.
            std::array<F, 9> cm00;
            cm00[0] = this->mat[5];
            cm00[1] = this->mat[6];
            cm00[2] = this->mat[7];
            cm00[3] = this->mat[9];
            cm00[4] = this->mat[10];
            cm00[5] = this->mat[11];
            cm00[6] = this->mat[13];
            cm00[7] = this->mat[14];
            cm00[8] = this->mat[15];

            std::array<F, 9> cm01;
            cm01[0] = this->mat[1];
            cm01[1] = this->mat[2];
            cm01[2] = this->mat[3];
            cm01[3] = this->mat[9];
            cm01[4] = this->mat[10];
            cm01[5] = this->mat[11];
            cm01[6] = this->mat[13];
            cm01[7] = this->mat[14];
            cm01[8] = this->mat[15];

            std::array<F, 9> cm02;
            cm02[0] = this->mat[1];
            cm02[1] = this->mat[2];
            cm02[2] = this->mat[3];
            cm02[3] = this->mat[5];
            cm02[4] = this->mat[6];
            cm02[5] = this->mat[7];
            cm02[6] = this->mat[13];
            cm02[7] = this->mat[14];
            cm02[8] = this->mat[15];

            std::array<F, 9> cm03;
            cm03[0] = this->mat[1];
            cm03[1] = this->mat[2];
            cm03[2] = this->mat[3];
            cm03[3] = this->mat[5];
            cm03[4] = this->mat[6];
            cm03[5] = this->mat[7];
            cm03[6] = this->mat[9];
            cm03[7] = this->mat[10];
            cm03[8] = this->mat[11];

            F det = this->mat[0] * this->determinant3x3 (cm00)
                - this->mat[4] * this->determinant3x3 (cm01)
                + this->mat[8] * this->determinant3x3 (cm02)
                - this->mat[12] * this->determinant3x3 (cm03);

            return det;
        }

        /*!
         * The adjugate is the transpose of the cofactor matrix. Recipe:
         * 1. Get the cofactor matrix (with this->cofactor())
         * 2. Obtain the adjugate matrix by transposing the cofactor matrix
         */
        constexpr std::array<F, 16> adjugate() const noexcept
        {
            std::array<F, 16> adj = this->transpose (this->cofactor());
            return adj;
        }

        /*!
         * Compute the cofactor matrix of this->mat. Recipe:
         * 1. Create matrix of minors
         * 2. Multiply matrix of minors by a checkerboard pattern to give the cofactor matrix
         */
        constexpr std::array<F, 16> cofactor() const noexcept
        {
            std::array<F, 16> cofac;

            // Keep to column-major format for all matrices. The elements of the matrix
            // of minors is found, but the cofactor matrix is populated, applying the
            // alternating pattern of +/- as we go.

            // 0.
            std::array<F, 9> minorElem;
            minorElem[0] = this->mat[5];
            minorElem[3] = this->mat[9];
            minorElem[6] = this->mat[13];

            minorElem[1] = this->mat[6];
            minorElem[4] = this->mat[10];
            minorElem[7] = this->mat[14];

            minorElem[2] = this->mat[7];
            minorElem[5] = this->mat[11];
            minorElem[8] = this->mat[15];

            cofac[0] = this->determinant3x3 (minorElem);

            // 1. Next minor elem matrix has only 3 elements changed
            minorElem[0] = this->mat[4];
            minorElem[3] = this->mat[8];
            minorElem[6] = this->mat[12];
            cofac[1] = -this->determinant3x3 (minorElem);

            // 2
            minorElem[1] = this->mat[5];
            minorElem[4] = this->mat[9];
            minorElem[7] = this->mat[13];
            cofac[2] = this->determinant3x3 (minorElem);

            // 3
            minorElem[2] = this->mat[6];
            minorElem[5] = this->mat[10];
            minorElem[8] = this->mat[14];
            cofac[3] = -this->determinant3x3 (minorElem);

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

            cofac[4] = -this->determinant3x3 (minorElem);

            // 5.
            minorElem[0] = this->mat[0];
            minorElem[3] = this->mat[8];
            minorElem[6] = this->mat[12];
            cofac[5] = this->determinant3x3 (minorElem);

            // 6.
            minorElem[1] = this->mat[1];
            minorElem[4] = this->mat[9];
            minorElem[7] = this->mat[13];
            cofac[6] = -this->determinant3x3 (minorElem);

            // 7.
            minorElem[2] = this->mat[2];
            minorElem[5] = this->mat[10];
            minorElem[8] = this->mat[14];
            cofac[7] = this->determinant3x3 (minorElem);

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

            cofac[8] = this->determinant3x3 (minorElem);

            // 9.
            minorElem[0] = this->mat[0];
            minorElem[3] = this->mat[4];
            minorElem[6] = this->mat[12];
            cofac[9] = -this->determinant3x3 (minorElem);

            // 10.
            minorElem[1] = this->mat[1];
            minorElem[4] = this->mat[5];
            minorElem[7] = this->mat[13];
            cofac[10] = this->determinant3x3 (minorElem);

            // 11.
            minorElem[2] = this->mat[2];
            minorElem[5] = this->mat[6];
            minorElem[8] = this->mat[14];
            cofac[11] = -this->determinant3x3 (minorElem);

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

            cofac[12] = -this->determinant3x3 (minorElem);

            // 13.
            minorElem[0] = this->mat[0];
            minorElem[3] = this->mat[4];
            minorElem[6] = this->mat[8];
            cofac[13] = this->determinant3x3 (minorElem);

            // 14.
            minorElem[1] = this->mat[1];
            minorElem[4] = this->mat[5];
            minorElem[7] = this->mat[9];
            cofac[14] = -this->determinant3x3 (minorElem);

            // 15.
            minorElem[2] = this->mat[2];
            minorElem[5] = this->mat[6];
            minorElem[8] = this->mat[10];
            cofac[15] = this->determinant3x3 (minorElem);

            return cofac;
        }

        /*!
         * Implement inversion using determinant method. inverse is (1/det) x adjugate
         * matrix.
         *
         * 1. Compute determinant of this->mat (if 0, then there's no inverse)
         * 2. Obtain the adjugate matrix
         * 3. Get the inverse by multiplying 1/determinant by the adjugate
         */
        constexpr mat44<F> invert() noexcept
        {
            F det = this->determinant();
            mat44<F> rtn;
            if (det == F{0}) {
                // The transform matrix has no inverse (determinant is 0)
                rtn.mat.fill (F{0});
            } else {
                rtn.mat = this->adjugate();
                rtn *= (F{1}/det);
            }
            return rtn;
        }

        /*!
         * This algorithm was obtained from:
         * http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q54 (but was it transposed?
         * seems so. See also https://www.songho.ca/opengl/gl_quaternion.html#overview
         * and https://danceswithcode.net/engineeringnotes/quaternions/quaternions.html)
         */
        template <typename T = float> requires std::is_arithmetic_v<T>
        constexpr void rotate (const quaternion<T>& q) noexcept
        {
            std::array<F, 16> m;

            const T f2x = q.x + q.x;
            const T f2y = q.y + q.y;
            const T f2z = q.z + q.z;
            const T f2xw = f2x * q.w;
            const T f2yw = f2y * q.w;
            const T f2zw = f2z * q.w;
            const T f2xx = f2x * q.x;
            const T f2xy = f2x * q.y;
            const T f2xz = f2x * q.z;
            const T f2yy = f2y * q.y;
            const T f2yz = f2y * q.z;
            const T f2zz = f2z * q.z;

            m[0]  = T{1} - (f2yy + f2zz);
            m[1]  =         f2xy + f2zw;
            m[2]  =         f2xz - f2yw;
            m[3]  = T{0};
            m[4]  =         f2xy - f2zw;
            m[5]  = T{1} - (f2xx + f2zz);
            m[6]  =         f2yz + f2xw;
            m[7]  = T{0};
            m[8]  =         f2xz + f2yw;
            m[9]  =         f2yz - f2xw;
            m[10] = T{1} - (f2xx + f2yy);
            m[11] = T{0};
            m[12] = T{0};
            m[13] = T{0};
            m[14] = T{0};
            m[15] = T{1};

            *this *= m;
        }

        //! Rotate an angle theta radians about axis
        template <typename T> requires std::is_arithmetic_v<T>
        constexpr void rotate (const std::array<T, 3>& axis, const T& theta) noexcept
        {
            quaternion<T> q;
            q.rotate (axis, theta);
            this->rotate<T> (q);
        }

        template <typename T> requires std::is_arithmetic_v<T>
        constexpr void rotate (const morph::vec<T, 3>& axis, const T& theta) noexcept
        {
            quaternion<T> q;
            q.rotate (axis, theta);
            this->rotate<T> (q);
        }

        //! Right-multiply this->mat with m2.
        constexpr void operator*= (const std::array<F, 16>& m2) noexcept
        {
            std::array<F, 16> result;
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
        constexpr void operator*= (const mat44<F>& m2) noexcept
        {
            std::array<F, 16> result;
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

        constexpr mat44<F> operator* (const std::array<F, 16>& m2) const noexcept
        {
            mat44<F> result;
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
        constexpr mat44<F> operator* (const mat44<F>& m2) const noexcept
        {
            mat44<F> result;
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
        constexpr std::array<F, 4> operator* (const std::array<F, 4>& v1) const noexcept
        {
            std::array<F, 4> v;
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

        //! Do matrix times vector multiplication, v = mat * v1
        constexpr vec<F, 4> operator* (const vec<F, 4>& v1) const noexcept
        {
            vec<F, 4> v;
            v[0] = this->mat[0] * v1.x()
                + this->mat[4] * v1.y()
                + this->mat[8] * v1.z()
                + this->mat[12] * v1.w();
            v[1] = this->mat[1] * v1.x()
                + this->mat[5] * v1.y()
                + this->mat[9] * v1.z()
                + this->mat[13] * v1.w();
            v[2] = this->mat[2] * v1.x()
                + this->mat[6] * v1.y()
                + this->mat[10] * v1.z()
                + this->mat[14] * v1.w();
            v[3] = this->mat[3] * v1.x()
                + this->mat[7] * v1.y()
                + this->mat[11] * v1.z()
                + this->mat[15] * v1.w();
            return v;
        }

        //! Do matrix times vector multiplication, v = mat * v1.
        constexpr vec<F, 4> operator* (const vec<F, 3>& v1) const noexcept
        {
            vec<F, 4> v;
            v[0] = this->mat[0] * v1.x()
                + this->mat[4] * v1.y()
                + this->mat[8] * v1.z()
                + this->mat[12]; // * 1
            v[1] = this->mat[1] * v1.x()
                + this->mat[5] * v1.y()
                + this->mat[9] * v1.z()
                + this->mat[13];
            v[2] = this->mat[2] * v1.x()
                + this->mat[6] * v1.y()
                + this->mat[10] * v1.z()
                + this->mat[14];
            v[3] = this->mat[3] * v1.x()
                + this->mat[7] * v1.y()
                + this->mat[11] * v1.z()
                + this->mat[15];
            return v;
        }

        //! *= operator for a scalar value.
        template <typename T=F> requires std::is_arithmetic_v<T>
        constexpr void operator*= (const T& f) noexcept
        {
            for (unsigned int i = 0; i<16; ++i) { this->mat[i] *= f; }
        }

        //! Equality operator. True if all elements match
        constexpr bool operator== (const mat44<F>& rhs) const noexcept
        {
            unsigned int ndiff = 0;
            for (unsigned int i = 0; i < 16 && ndiff == 0; ++i) {
                ndiff += this->mat[i] == rhs.mat[i] ? 0 : 1;
            }
            return ndiff == 0;
        }

        //! Not equals
        constexpr bool operator!= (const mat44<F>& rhs) const noexcept
        {
            unsigned int ndiff = 0;
            for (unsigned int i = 0; i < 16 && ndiff == 0; ++i) {
                ndiff += this->mat[i] == rhs.mat[i] ? 0 : 1;
            }
            return ndiff > 0;
        }

        //! Transpose this matrix
        constexpr void transpose() noexcept
        {
            std::array<F, 6> a;
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
        constexpr std::array<F, 16> transpose (const std::array<F, 16>& matrx) const noexcept
        {
            std::array<F, 16> tposed;
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

        /*!
         * Make a (frustrum) perspective projection
         *
         * @fovDeg Field of view, in degrees. Measured from the top of the field to
         * the bottom of the field (rather than from the left to the right).
         *
         * @aspect The field's aspect ratio. For a field which is wider than it is
         * high, this will be >1. That is, this is "the number of multiples of the
         * height that the width is"
         *
         * @zNear The near/projection plane.
         *
         * @zFar The far plane.
         */
        constexpr void perspective (F fovDeg, F aspect, F zNear, F zFar) noexcept
        {
            // Aspect is going to be about 1.33 for a typical rectangular window wider
            // than it is high.

            // Bail out if the projection volume is zero-sized.
            if (zNear == zFar || aspect == F{0}) { return; }

            F fovRad_ov2 = fovDeg * morph::mathconst<F>::pi_over_360; // fovDeg/2 converted to radians
            F sineFov = morph::math::sin (fovRad_ov2);
            if (sineFov == F{0}) { return; }
            F cotanFov = morph::math::cos (fovRad_ov2) / sineFov;
            F clip = zFar - zNear;

            // Perspective matrix to multiply self by
            std::array<F, 16> persMat;
            persMat.fill (F{0});
            persMat[0] = cotanFov/aspect; // n/(width/2) = 2n/width, or generally 2n/r-l
            persMat[5] = cotanFov;        // n/(height/2) = 2n/height, or generally 2n/t-b
            // For fully general frustrum not centered on the z axis, we would add these:
            //persMat[8] = r+l/r-l
            //persMat[9] = t+b/t-b
            persMat[10] = -(zNear+zFar)/clip;
            persMat[11] = F{-1};
            persMat[14] = -(F{2} * zNear * zFar)/clip;

            (*this) *= persMat;
        }

        /*!
         * Make an orthographic projection
         *
         * \param rt Right-top coordinate. rt[0] is 'x' and thus the right and rt[1] ('y') is the top
         *
         * \param lb Left-bottom coordinate. lb[0] is 'x' and thus the left and lb[1] 'y' and is the bottom
         *
         * \param zFar The 'far' z coordinate of the canonical viewing volume
         *
         * \param zNear The 'near' z coordinate of the canonical viewing volume
         */
        constexpr void orthographic (const vec<F, 2>& lb, const vec<F, 2>& rt,
                                     const F zNear, const F zFar) noexcept
        {
            if (zNear == zFar) { return; }

            // Orthographic matrix to multiply self by
            std::array<F, 16> orthoMat;
            orthoMat.fill (F{0});
            orthoMat[0] = F{2}/(rt[0]-lb[0]);             //      2/(r-l)
            orthoMat[5] = F{2}/(rt[1]-lb[1]);             //      2/(t-b)
            orthoMat[10] = F{-2}/(zFar-zNear);            //     -2/(f-n)
            orthoMat[12] = -(rt[0]+lb[0])/(rt[0]-lb[0]);    // -(r+l)/(r-l)
            orthoMat[13] = -(rt[1]+lb[1])/(rt[1]-lb[1]);    // -(t+b)/(t-b)
            orthoMat[14] = -(zFar+zNear)/(zFar-zNear);      // -(f+n)/(f-n)
            orthoMat[15] = F{1};

            (*this) *= orthoMat;
        }

        //! Overload the stream output operator
        friend std::ostream& operator<< <F> (std::ostream& os, const mat44<F>& tm);
    };

    template <typename F>
    std::ostream& operator<< (std::ostream& os, const mat44<F>& tm)
    {
        os << tm.str();
        return os;
    }

} // namespace morph
