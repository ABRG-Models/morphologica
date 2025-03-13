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
    template <typename F> class mat33;
    template <typename F> std::ostream& operator<< (std::ostream&, const mat33<F>&);

    /*!
     * This implements a general purpose 3x3 matrix, for use in 2D applications. The
     * matrix data is stored in mat33::mat, an array of 9 floating point
     * numbers.
     *
     * \templateparam F The floating point type in which to store the matrix's data.
     */
    template <typename F>
    class mat33
    {
    public:
        //! Default constructor
        mat33() noexcept { this->setToIdentity(); }
        //! User-declared destructor
        ~mat33() noexcept {}
        //! User-declared copy constructor
        mat33 (const mat33<F>& other) noexcept : mat(other.mat) {}
        //! User-declared copy assignment constructor
        mat33<F>& operator= (mat33<F>& other) noexcept
        {
            std::copy (other.mat.begin(), other.mat.end(), mat.begin());
            return *this;
        }
        //! Explicitly defaulted  move constructor
        mat33 (mat33<F>&& other) noexcept = default;
        //! Explicitly defaulted move assignment constructor
        mat33<F>& operator= (mat33<F>&& other) noexcept = default;

        /*!
         * The matrix data, arranged in column major format to be similar to
         * morph::mat44 (which is OpenGL friendly).
         */
        alignas(std::array<F, 9>) std::array<F, 9> mat;

        //! Return a string representation of the matrix
        std::string str() const noexcept
        {
            std::stringstream ss;
            ss <<"[ "<< mat[0]<<" , "<<mat[3]<<" , "<<mat[6]<<" ;\n";
            ss <<"  "<< mat[1]<<" , "<<mat[4]<<" , "<<mat[7]<<" ;\n";
            ss <<"  "<< mat[2]<<" , "<<mat[5]<<" , "<<mat[8]<<" ]\n";
            return ss.str();
        }

        //! Return a string representation of the passed-in column-major array
        static std::string str (const std::array<F, 9>& arr) noexcept
        {
            std::stringstream ss;
            ss <<"[ "<< arr[0]<<" , "<<arr[3]<<" , "<<arr[6]<<" ;\n";
            ss <<"  "<< arr[1]<<" , "<<arr[4]<<" , "<<arr[7]<<" ;\n";
            ss <<"  "<< arr[2]<<" , "<<arr[5]<<" , "<<arr[8]<<" ]\n";
            return ss.str();
        }

        void setToIdentity() noexcept
        {
            this->mat.fill (F{0});
            this->mat[0] = F{1};
            this->mat[4] = F{1};
            this->mat[8] = F{1};
        }

        //! Access elements of the matrix
        F& operator[] (unsigned int idx) noexcept { return this->mat[idx]; }
        // note: assume F is a built-in type here (safe - F will be float or double)
        const F operator[] (unsigned int idx) const noexcept { return this->mat[idx]; }

        //! Access a given row of the matrix
        morph::vec<F, 3> row (unsigned int idx) const noexcept
        {
            morph::vec<F, 3> r = {F{0}, F{0}, F{0}};
            if (idx > 2U) { return r; }
            r[0] = this->mat[idx];
            r[1] = this->mat[idx+3];
            r[2] = this->mat[idx+6];
            return r;
        }

        //! Access a given column of the matrix
        morph::vec<F, 3> col (unsigned int idx) const noexcept
        {
            morph::vec<F, 3> c = {F{0}, F{0}, F{0}};
            if (idx > 2U) { return c; }
            idx *= 3U;
            c[0] = this->mat[idx];
            c[1] = this->mat[++idx];
            c[2] = this->mat[++idx];
            return c;
        }

        //! Transpose this matrix
        void transpose() noexcept
        {
            std::array<F, 3> a;
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
        std::array<F, 9> transpose (const std::array<F, 9>& matrx) const noexcept
        {
            std::array<F, 9> tposed;
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
        static F determinant (std::array<F, 4> cm) noexcept
        {
            return ((cm[0]*cm[3]) - (cm[1]*cm[2]));
        }

        //! Compute determinant for 3x3 matrix @cm
        static F determinant (std::array<F, 9> cm) noexcept
        {
            F det = (cm[0]*cm[4]*cm[8])
                + (cm[3]*cm[7]*cm[2])
                + (cm[6]*cm[1]*cm[5])
                - (cm[6]*cm[4]*cm[2])
                - (cm[0]*cm[7]*cm[5])
                - (cm[3]*cm[1]*cm[8]);
            return det;
        }

        F determinant() const noexcept
        {
            F det = (mat[0]*mat[4]*mat[8])
                + (mat[3]*mat[7]*mat[2])
                + (mat[6]*mat[1]*mat[5])
                - (mat[6]*mat[4]*mat[2])
                - (mat[0]*mat[7]*mat[5])
                - (mat[3]*mat[1]*mat[8]);
            return det;
        }

        std::array<F, 9> adjugate() const noexcept
        {
            return this->transpose (this->cofactor());
        }

        static constexpr bool debug_cofactors = false;
        std::array<F, 9> cofactor() const noexcept
        {
            std::array<F, 9> cofac;

            // Keep to column-major format for all matrices. The elements of the matrix
            // of minors is found, but the cofactor matrix is populated, applying the
            // alternating pattern of +/- as we go.

            // 0.
            std::array<F, 4> minorElem;
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

            if constexpr (debug_cofactors) {
                std::cout << "cofactor:\n";
                std::cout <<"[ "<< cofac[0]<<" , "<<cofac[3]<<" , "<<cofac[6]<<" ;\n";
                std::cout <<"  "<< cofac[1]<<" , "<<cofac[4]<<" , "<<cofac[7]<<" ;\n";
                std::cout <<"  "<< cofac[2]<<" , "<<cofac[5]<<" , "<<cofac[8]<<" ;\n";
            }
            return cofac;
        }

        mat33<F> invert() noexcept
        {
            F det = this->determinant();
            mat33<F> rtn;
            if (det == F{0}) {
                std::cout << "NB: The transform matrix has no inverse (determinant is 0)" << std::endl;
                rtn.mat.fill (F{0});
            } else {
                rtn.mat = this->adjugate();
                rtn *= (F{1}/det);
            }
            return rtn;
        }

        //! *= operator for a scalar value.
        template <typename T=F>
        void operator*= (const T& f) noexcept
        {
            for (unsigned int i = 0; i<9; ++i) { this->mat[i] *= f; }
        }

        //! Right-multiply this->mat with m2.
        void operator*= (const std::array<F, 9>& m2) noexcept
        {
            std::array<F, 9> result;
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
        void operator*= (const mat33<F>& m2) noexcept
        {
            std::array<F, 9> result;
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
        mat33<F> operator* (const std::array<F, 9>& m2) const noexcept
        {
            mat33<F> result;
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
        mat33<F> operator* (const mat33<F>& m2) const noexcept
        {
            mat33<F> result;
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
        std::array<F, 3> operator* (const std::array<F, 3>& v1) const noexcept
        {
            std::array<F, 3> v;
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
        vec<F, 3> operator* (const vec<F, 3>& v1) const noexcept
        {
            vec<F, 3> v;
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

        //! Equality operator. True if all elements match
        bool operator== (const mat33<F>& rhs) const noexcept
        {
            unsigned int ndiff = 0;
            for (unsigned int i = 0; i < 9 && ndiff == 0; ++i) {
                ndiff += this->mat[i] == rhs.mat[i] ? 0 : 1;
            }
            return ndiff == 0;
        }

        //! Not equals
        bool operator!= (const mat33<F>& rhs) const noexcept
        {
            unsigned int ndiff = 0;
            for (unsigned int i = 0; i < 9 && ndiff == 0; ++i) {
                ndiff += this->mat[i] == rhs.mat[i] ? 0 : 1;
            }
            return ndiff > 0;
        }


        //! Overload the stream output operator
        friend std::ostream& operator<< <F> (std::ostream& os, const mat33<F>& tm);
    };

    template <typename F>
    std::ostream& operator<< (std::ostream& os, const mat33<F>& tm)
    {
        os << tm.str();
        return os;
    }

} // namespace morph
