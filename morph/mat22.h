/*!
 * \file
 *
 * A 2x2 matrix class, useful for 2D transformations
 *
 * \author Seb James
 * \date 2022
 */
#pragma once

#include <morph/vec.h>
#include <cmath>
#include <string>
#include <sstream>
#include <iostream>

namespace morph {

    // Forward declare class and stream operator
    template <typename F> class mat22;
    template <typename F> std::ostream& operator<< (std::ostream&, const mat22<F>&);

    /*!
     * This implements a 2x2 matrix, for use in 2D applications. The matrix data is
     * stored in mat22::mat, an array (morph::vec) of 4 floating point numbers.
     *
     * \templateparam F The floating point type in which to store the matrix's data.
     */
    template <typename F>
    class mat22
    {
    public:
        //! Default constructor
        mat22() noexcept { this->setToIdentity(); }

        /*!
         * The matrix data, arranged in column major format to be similar to
         * morph::mat44 (which is OpenGL friendly).
         */
        alignas(morph::vec<F, 4>) morph::vec<F, 4> mat;

        //! Return a string representation of the matrix
        std::string str() const noexcept
        {
            std::stringstream ss;
            ss <<"[ "<< mat[0]<<" , "<<mat[2]<<" ;\n";
            ss <<"  "<< mat[1]<<" , "<<mat[3]<<" ]\n";
            return ss.str();
        }

        //! Return a string representation of the passed-in column-major matrix
        static std::string str (const morph::vec<F, 4>& arr) noexcept
        {
            std::stringstream ss;
            ss <<"[ "<< arr[0]<<" , "<<arr[2]<<" ;\n";
            ss <<"  "<< arr[1]<<" , "<<arr[3]<<" ]\n";
            return ss.str();
        }

        void setToIdentity() noexcept
        {
            this->mat.zero();
            this->mat[0] = F{1};
            this->mat[3] = F{1};
        }

        //! Set this matrix up so that it would rotate a 2D vector by rot_rad radians, anticlockwise.
        void rotate (const F rot_rad) noexcept
        {
            this->mat[0] = std::cos (rot_rad);
            this->mat[1] = std::sin (rot_rad);
            this->mat[2] = -this->mat[1]; // -sin
            this->mat[3] = this->mat[0];
        }

        //! Access elements of the matrix
        F& operator[] (unsigned int idx) noexcept { return this->mat[idx]; }
        // note: assume F is a built-in type here (safe - F will be float or double)
        const F operator[] (unsigned int idx) const noexcept  { return this->mat[idx]; }

        //! Access a given row of the matrix
        morph::vec<F, 2> row (unsigned int idx) const noexcept // unsigned char would be enough capacity!
        {
            morph::vec<F, 2> r = {F{0}, F{0}};
            if (idx > 1U) { return r; }
            r[0] = this->mat[idx];
            r[1] = this->mat[idx+2];
            return r;
        }

        //! Access a given column of the matrix
        morph::vec<F, 2> col (unsigned int idx) const noexcept
        {
            morph::vec<F, 3> c = {F{0}, F{0}};
            if (idx > 1U) { return c; }
            idx *= 2U;
            c[0] = this->mat[idx];
            c[1] = this->mat[++idx];
            return c;
        }

        //! Transpose this matrix
        void transpose() noexcept
        {
            F a = this->mat[2];
            this->mat[2] = this->mat[1];
            this->mat[1] = a;
        }

        //! Transpose the matrix @matrx, returning the transposed version.
        morph::vec<F, 4> transpose (const morph::vec<F, 4>& matrx) const noexcept
        {
            morph::vec<F, 4> tposed;
            tposed[0] = matrx[0];
            tposed[1] = matrx[2];
            tposed[2] = matrx[1];
            tposed[3] = matrx[3];
            return tposed;
        }

        //! Compute determinant for column-major 2x2 matrix @cm
        static F determinant (morph::vec<F, 4> cm) noexcept
        {
            return ((cm[0]*cm[3]) - (cm[1]*cm[2]));
        }

        F determinant() const noexcept
        {
            return ((this->mat[0]*this->mat[3]) - (this->mat[1]*this->mat[2]));
        }

        morph::vec<F, 4> adjugate() const noexcept
        {
            morph::vec<F, 4> adj = { this->mat[3], -this->mat[1], -this->mat[2], this->mat[0] };
            return adj;
        }

        mat22<F> invert() noexcept
        {
            mat22<F> rtn;
            F det = this->determinant();
            if (det == 0) {
                std::cout << "NB: The transform matrix has no inverse (determinant is 0)" << std::endl;
                rtn.mat.zero();
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
            for (unsigned int i = 0; i<4; ++i) { this->mat[i] *= f; }
        }

        //! Right-multiply this->mat with m2.
        void operator*= (const morph::vec<F, 4>& m2) noexcept
        {
            morph::vec<F, 4> result;
            // Top row
            result[0] = this->mat[0] * m2[0] + this->mat[2] * m2[1];
            result[2] = this->mat[0] * m2[2] + this->mat[2] * m2[3];
            // Second row
            result[1] = this->mat[1] * m2[0] + this->mat[3] * m2[1];
            result[3] = this->mat[1] * m2[2] + this->mat[3] * m2[3];
            this->mat.swap (result);
        }

        //! Right-multiply this->mat with m2.
        void operator*= (const mat22<F>& m2) noexcept
        {
            morph::vec<F, 4> result;
            // Top row
            result[0] = this->mat[0] * m2.mat[0] + this->mat[2] * m2.mat[1];
            result[2] = this->mat[0] * m2.mat[2] + this->mat[2] * m2.mat[3];
            // Second row
            result[1] = this->mat[1] * m2.mat[0] + this->mat[3] * m2.mat[1];
            result[3] = this->mat[1] * m2.mat[2] + this->mat[3] * m2.mat[3];
            this->mat.swap (result);
        }

        //! Return this->mat * m2
        mat22<F> operator* (const morph::vec<F, 4>& m2) const noexcept
        {
            mat22<F> result;
            // Top row
            result.mat[0] = this->mat[0] * m2[0] + this->mat[2] * m2[1];
            result.mat[2] = this->mat[0] * m2[2] + this->mat[2] * m2[3];
            // Second row
            result.mat[1] = this->mat[1] * m2[0] + this->mat[3] * m2[1];
            result.mat[3] = this->mat[1] * m2[2] + this->mat[3] * m2[3];
            return result;
        }

        //! Return this-> mat * m2
        mat22<F> operator* (const mat22<F>& m2) const noexcept
        {
            mat22<F> result;
            // Top row
            result.mat[0] = this->mat[0] * m2.mat[0] + this->mat[2] * m2.mat[1];
            result.mat[2] = this->mat[0] * m2.mat[2] + this->mat[2] * m2.mat[3];
            // Second row
            result.mat[1] = this->mat[1] * m2.mat[0] + this->mat[3] * m2.mat[1];
            result.mat[3] = this->mat[1] * m2.mat[2] + this->mat[3] * m2.mat[3];
            return result;
        }

        //! Do matrix times vector multiplication, v = mat * v1
        morph::vec<F, 2> operator* (const morph::vec<F, 2>& v1) const noexcept
        {
            morph::vec<F, 2> v = {
                this->mat[0] * v1[0] + this->mat[2] * v1[1],
                this->mat[1] * v1[0] + this->mat[3] * v1[1]
            };
            return v;
        }

        //! Overload the stream output operator
        friend std::ostream& operator<< <F> (std::ostream& os, const mat22<F>& tm);
    };

    template <typename F>
    std::ostream& operator<< (std::ostream& os, const mat22<F>& tm)
    {
        os << tm.str();
        return os;
    }

} // namespace morph
