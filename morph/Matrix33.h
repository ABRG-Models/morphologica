/*!
 * \file
 *
 * A 3x3 matrix class, useful for 2D transformations
 *
 * \author Seb James
 * \date 2020
 */
#pragma once

#include <morph/vec.h>
#include <cmath>
#include <array>
#include <string>
#include <sstream>
#include <iostream>

namespace morph {

    // Forward declare class and stream operator
    template <typename Flt> class Matrix33;
    template <typename Flt> std::ostream& operator<< (std::ostream&, const Matrix33<Flt>&);

    /*!
     * This implements a general purpose 3x3 matrix, for use in 2D applications. The
     * matrix data is stored in Matrix33::mat, an array of 9 floating point
     * numbers.
     *
     * \templateparam Flt The floating point type in which to store the matrix's data.
     */
    template <typename Flt>
    class Matrix33
    {
    public:
        //! Default constructor
        Matrix33() { this->setToIdentity(); }

        /*!
         * The matrix data, arranged in column major format to be similar to
         * morph::TransformMatrix (which is OpenGL friendly).
         */
        alignas(std::array<Flt, 9>) std::array<Flt, 9> mat;

        //! Return a string representation of the matrix
        std::string str() const
        {
            std::stringstream ss;
            ss <<"[ "<< mat[0]<<" , "<<mat[3]<<" , "<<mat[6]<<" ;\n";
            ss <<"  "<< mat[1]<<" , "<<mat[4]<<" , "<<mat[7]<<" ;\n";
            ss <<"  "<< mat[2]<<" , "<<mat[5]<<" , "<<mat[8]<<" ]\n";
            return ss.str();
        }

        //! Return a string representation of the passed-in column-major array
        static std::string str (const std::array<Flt, 9>& arr)
        {
            std::stringstream ss;
            ss <<"[ "<< arr[0]<<" , "<<arr[3]<<" , "<<arr[6]<<" ;\n";
            ss <<"  "<< arr[1]<<" , "<<arr[4]<<" , "<<arr[7]<<" ;\n";
            ss <<"  "<< arr[2]<<" , "<<arr[5]<<" , "<<arr[8]<<" ]\n";
            return ss.str();
        }

        void setToIdentity()
        {
            this->mat.fill (Flt{0});
            this->mat[0] = Flt{1};
            this->mat[4] = Flt{1};
            this->mat[8] = Flt{1};
        }

        //! Access elements of the matrix
        Flt& operator[] (size_t idx) { return this->mat[idx]; }
        // note: assume Flt is a built-in type here (safe - Flt will be float or double)
        const Flt operator[] (size_t idx) const  { return this->mat[idx]; }

        //! Access a given row of the matrix
        morph::vec<Flt, 3> row (size_t idx) const
        {
            morph::vec<Flt, 3> r = {0,0,0};
            if (idx > 2) { return r; }
            r[0] = this->mat[idx];
            r[1] = this->mat[idx+3];
            r[2] = this->mat[idx+6];
            return r;
        }

        //! Access a given column of the matrix
        morph::vec<Flt, 3> col (size_t idx) const
        {
            morph::vec<Flt, 3> c = {0,0,0};
            if (idx > 2) { return c; }
            idx *= 3;
            c[0] = this->mat[idx];
            c[1] = this->mat[++idx];
            c[2] = this->mat[++idx];
            return c;
        }

        //! Transpose this matrix
        void transpose()
        {
            std::array<Flt, 3> a;
            a[0] = this->mat[1];
            a[1] = this->mat[2];
            a[2] = this->mat[5];

            this->mat[1] = this->mat[3];
            this->mat[2] = this->mat[6];
            this->mat[5] = this->mat[7];

            this->mat[3] = a[0];
            this->mat[6] = a[1];
            this->mat[7] = a[2];
        }

        //! Transpose the matrix @matrx, returning the transposed version.
        std::array<Flt, 9> transpose (const std::array<Flt, 9>& matrx) const
        {
            std::array<Flt, 9> tposed;
            tposed[0] = matrx[0];
            tposed[1] = matrx[3];
            tposed[2] = matrx[6];
            tposed[3] = matrx[1];
            tposed[4] = matrx[4];
            tposed[5] = matrx[7];
            tposed[6] = matrx[2];
            tposed[7] = matrx[5];
            tposed[8] = matrx[8];
            return tposed;
        }

        //! Compute determinant for column-major 2x2 matrix @cm
        static Flt determinant (std::array<Flt, 4> cm)
        {
            return ((cm[0]*cm[3]) - (cm[1]*cm[2]));
        }

        //! Compute determinant for 3x3 matrix @cm
        static Flt determinant (std::array<Flt, 9> cm)
        {
            Flt det = (cm[0]*cm[4]*cm[8])
                + (cm[3]*cm[7]*cm[2])
                + (cm[6]*cm[1]*cm[5])
                - (cm[6]*cm[4]*cm[2])
                - (cm[0]*cm[7]*cm[5])
                - (cm[3]*cm[1]*cm[8]);
            return det;
        }

        Flt determinant() const
        {
            Flt det = (mat[0]*mat[4]*mat[8])
                + (mat[3]*mat[7]*mat[2])
                + (mat[6]*mat[1]*mat[5])
                - (mat[6]*mat[4]*mat[2])
                - (mat[0]*mat[7]*mat[5])
                - (mat[3]*mat[1]*mat[8]);
            return det;
        }

        std::array<Flt, 9> adjugate() const
        {
            return this->transpose (this->cofactor());
        }

        std::array<Flt, 9> cofactor() const
        {
            std::array<Flt, 9> cofac;

            // Keep to column-major format for all matrices. The elements of the matrix
            // of minors is found, but the cofactor matrix is populated, applying the
            // alternating pattern of +/- as we go.

            // 0.
            std::array<Flt, 4> minorElem;
            minorElem[0] = this->mat[4];
            minorElem[1] = this->mat[5];
            minorElem[2] = this->mat[7];
            minorElem[3] = this->mat[8];
            cofac[0] = this->determinant (minorElem);

            // 3. Next minor elem matrix has only 2 elements changed
            minorElem[0] = this->mat[1];
            minorElem[1] = this->mat[2];
            cofac[3] = -this->determinant (minorElem);

            // 6.
            minorElem[2] = this->mat[4];
            minorElem[3] = this->mat[5];
            cofac[6] = this->determinant (minorElem);

            // 1.
            minorElem[0] = this->mat[3];
            minorElem[1] = this->mat[5];
            minorElem[2] = this->mat[6];
            minorElem[3] = this->mat[8];
            cofac[1] = -this->determinant (minorElem);

            // 4.
            minorElem[0] = this->mat[0];
            minorElem[1] = this->mat[2];
            cofac[4] = this->determinant (minorElem);

            // 7.
            minorElem[2] = this->mat[3];
            minorElem[3] = this->mat[5];
            cofac[7] = -this->determinant (minorElem);

            // 2.
            minorElem[0] = this->mat[3];
            minorElem[1] = this->mat[4];
            minorElem[2] = this->mat[6];
            minorElem[3] = this->mat[7];
            cofac[2] = this->determinant (minorElem);

            // 5.
            minorElem[0] = this->mat[0];
            minorElem[1] = this->mat[1];
            cofac[5] = -this->determinant (minorElem);

            // 8.
            minorElem[2] = this->mat[3];
            minorElem[3] = this->mat[4];
            cofac[8] = this->determinant (minorElem);
#if 0
            std::cout << "cofactor:\n";
            std::cout <<"[ "<< cofac[0]<<" , "<<cofac[3]<<" , "<<cofac[6]<<" ;\n";
            std::cout <<"  "<< cofac[1]<<" , "<<cofac[4]<<" , "<<cofac[7]<<" ;\n";
            std::cout <<"  "<< cofac[2]<<" , "<<cofac[5]<<" , "<<cofac[8]<<" ;\n";
#endif
            return cofac;
        }

        Matrix33<Flt> invert()
        {
            Flt det = this->determinant();
            Matrix33<Flt> rtn;
            if (det == Flt{0}) {
                std::cout << "NB: The transform matrix has no inverse (determinant is 0)" << std::endl;
                rtn.mat.fill (Flt{0});
            } else {
                rtn.mat = this->adjugate();
                rtn *= (Flt{1}/det);
            }
            return rtn;
        }

        //! *= operator for a scalar value.
        template <typename T=Flt>
        void operator*= (const T& f)
        {
            for (unsigned int i = 0; i<9; ++i) { this->mat[i] *= f; }
        }

        //! Right-multiply this->mat with m2.
        void operator*= (const std::array<Flt, 9>& m2)
        {
            std::array<Flt, 9> result;
            // Top row
            result[0] = this->mat[0] * m2[0]
            + this->mat[3] * m2[1]
            + this->mat[6] * m2[2];
            result[3] = this->mat[0] * m2[3]
            + this->mat[3] * m2[4]
            + this->mat[6] * m2[5];
            result[6] = this->mat[0] * m2[6]
            + this->mat[3] * m2[7]
            + this->mat[6] * m2[8];

            // Second row
            result[1] = this->mat[1] * m2[0]
            + this->mat[4] * m2[1]
            + this->mat[7] * m2[2];
            result[4] = this->mat[1] * m2[3]
            + this->mat[4] * m2[4]
            + this->mat[7] * m2[5];
            result[7] = this->mat[1] * m2[6]
            + this->mat[4] * m2[7]
            + this->mat[7] * m2[8];

            // Third row
            result[2] = this->mat[2] * m2[0]
            + this->mat[5] * m2[1]
            + this->mat[8] * m2[2];
            result[5] = this->mat[2] * m2[3]
            + this->mat[5] * m2[4]
            + this->mat[8] * m2[5];
            result[8] = this->mat[2] * m2[6]
            + this->mat[5] * m2[7]
            + this->mat[8] * m2[8];

            this->mat.swap (result);
        }

        //! Right-multiply this->mat with m2.
        void operator*= (const Matrix33<Flt>& m2)
        {
            std::array<Flt, 9> result;
            // Top row
            result[0] = this->mat[0] * m2.mat[0]
            + this->mat[3] * m2.mat[1]
            + this->mat[6] * m2.mat[2];
            result[3] = this->mat[0] * m2.mat[3]
            + this->mat[3] * m2.mat[4]
            + this->mat[6] * m2.mat[5];
            result[6] = this->mat[0] * m2.mat[6]
            + this->mat[3] * m2.mat[7]
            + this->mat[6] * m2.mat[8];

            // Second row
            result[1] = this->mat[1] * m2.mat[0]
            + this->mat[4] * m2.mat[1]
            + this->mat[7] * m2.mat[2];
            result[4] = this->mat[1] * m2.mat[3]
            + this->mat[4] * m2.mat[4]
            + this->mat[7] * m2.mat[5];
            result[7] = this->mat[1] * m2.mat[6]
            + this->mat[4] * m2.mat[7]
            + this->mat[7] * m2.mat[8];

            // Third row
            result[2] = this->mat[2] * m2.mat[0]
            + this->mat[5] * m2.mat[1]
            + this->mat[8] * m2.mat[2];
            result[5] = this->mat[2] * m2.mat[3]
            + this->mat[5] * m2.mat[4]
            + this->mat[8] * m2.mat[5];
            result[8] = this->mat[2] * m2.mat[6]
            + this->mat[5] * m2.mat[7]
            + this->mat[8] * m2.mat[8];

            this->mat.swap (result);
        }

        //! Return this-> mat * m2
        Matrix33<Flt> operator* (const std::array<Flt, 9>& m2) const
        {
            Matrix33<Flt> result;
            // Top row
            result.mat[0] = this->mat[0] * m2[0]
            + this->mat[3] * m2[1]
            + this->mat[6] * m2[2];
            result.mat[3] = this->mat[0] * m2[3]
            + this->mat[3] * m2[4]
            + this->mat[6] * m2[5];
            result.mat[6] = this->mat[0] * m2[6]
            + this->mat[3] * m2[7]
            + this->mat[6] * m2[8];

            // Second row
            result.mat[1] = this->mat[1] * m2[0]
            + this->mat[4] * m2[1]
            + this->mat[7] * m2[2];
            result.mat[4] = this->mat[1] * m2[3]
            + this->mat[4] * m2[4]
            + this->mat[7] * m2[5];
            result.mat[7] = this->mat[1] * m2[6]
            + this->mat[4] * m2[7]
            + this->mat[7] * m2[8];

            // Third row
            result.mat[2] = this->mat[2] * m2[0]
            + this->mat[5] * m2[1]
            + this->mat[8] * m2[2];
            result.mat[5] = this->mat[2] * m2[3]
            + this->mat[5] * m2[4]
            + this->mat[8] * m2[5];
            result.mat[8] = this->mat[2] * m2[6]
            + this->mat[5] * m2[7]
            + this->mat[8] * m2[8];

            return result;
        }

        //! Return this-> mat * m2
        Matrix33<Flt> operator* (const Matrix33<Flt>& m2) const
        {
            Matrix33<Flt> result;
            // Top row
            result.mat[0] = this->mat[0] * m2.mat[0]
            + this->mat[3] * m2.mat[1]
            + this->mat[6] * m2.mat[2];
            result.mat[3] = this->mat[0] * m2.mat[3]
            + this->mat[3] * m2.mat[4]
            + this->mat[6] * m2.mat[5];
            result.mat[6] = this->mat[0] * m2.mat[6]
            + this->mat[3] * m2.mat[7]
            + this->mat[6] * m2.mat[8];

            // Second row
            result.mat[1] = this->mat[1] * m2.mat[0]
            + this->mat[4] * m2.mat[1]
            + this->mat[7] * m2.mat[2];
            result.mat[4] = this->mat[1] * m2.mat[3]
            + this->mat[4] * m2.mat[4]
            + this->mat[7] * m2.mat[5];
            result.mat[7] = this->mat[1] * m2.mat[6]
            + this->mat[4] * m2.mat[7]
            + this->mat[7] * m2.mat[8];

            // Third row
            result.mat[2] = this->mat[2] * m2.mat[0]
            + this->mat[5] * m2.mat[1]
            + this->mat[8] * m2.mat[2];
            result.mat[5] = this->mat[2] * m2.mat[3]
            + this->mat[5] * m2.mat[4]
            + this->mat[8] * m2.mat[5];
            result.mat[8] = this->mat[2] * m2.mat[6]
            + this->mat[5] * m2.mat[7]
            + this->mat[8] * m2.mat[8];

            return result;
        }

        //! Do matrix times vector multiplication, v = mat * v1
        std::array<Flt, 3> operator* (const std::array<Flt, 3>& v1) const
        {
            std::array<Flt, 3> v;
            v[0] = this->mat[0] * v1[0]
            + this->mat[3] * v1[1]
            + this->mat[6] * v1[2];
            v[1] = this->mat[1] * v1[0]
            + this->mat[4] * v1[1]
            + this->mat[7] * v1[2];
            v[2] = this->mat[2] * v1[0]
            + this->mat[5] * v1[1]
            + this->mat[8] * v1[2];
            return v;
        }

        //! Do matrix times vector multiplication, v = mat * v1
        vec<Flt, 3> operator* (const vec<Flt, 3>& v1) const
        {
            vec<Flt, 3> v;
            v[0] = this->mat[0] * v1[0]
            + this->mat[3] * v1[1]
            + this->mat[6] * v1[2];
            v[1] = this->mat[1] * v1[0]
            + this->mat[4] * v1[1]
            + this->mat[7] * v1[2];
            v[2] = this->mat[2] * v1[0]
            + this->mat[5] * v1[1]
            + this->mat[8] * v1[2];
            return v;
        }

        //! Overload the stream output operator
        friend std::ostream& operator<< <Flt> (std::ostream& os, const Matrix33<Flt>& tm);
    };

    template <typename Flt>
    std::ostream& operator<< (std::ostream& os, const Matrix33<Flt>& tm)
    {
        os << tm.str();
        return os;
    }

} // namespace morph
