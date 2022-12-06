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
    template <typename Flt> class Matrix22;
    template <typename Flt> std::ostream& operator<< (std::ostream&, const Matrix22<Flt>&);

    /*!
     * This implements a 2x2 matrix, for use in 2D applications. The matrix data is
     * stored in Matrix22::mat, an array (morph::vec) of 4 floating point numbers.
     *
     * \templateparam Flt The floating point type in which to store the matrix's data.
     */
    template <typename Flt>
    class Matrix22
    {
    public:
        //! Default constructor
        Matrix22() { this->setToIdentity(); }

        /*!
         * The matrix data, arranged in column major format to be similar to
         * morph::TransformMatrix (which is OpenGL friendly).
         */
        alignas(morph::vec<Flt, 4>) morph::vec<Flt, 4> mat;

        //! Return a string representation of the matrix
        std::string str() const
        {
            std::stringstream ss;
            ss <<"[ "<< mat[0]<<" , "<<mat[2]<<" ;\n";
            ss <<"  "<< mat[1]<<" , "<<mat[3]<<" ]\n";
            return ss.str();
        }

        //! Return a string representation of the passed-in column-major matrix
        static std::string str (const morph::vec<Flt, 4>& arr)
        {
            std::stringstream ss;
            ss <<"[ "<< arr[0]<<" , "<<arr[2]<<" ;\n";
            ss <<"  "<< arr[1]<<" , "<<arr[3]<<" ]\n";
            return ss.str();
        }

        void setToIdentity()
        {
            this->mat.zero();
            this->mat[0] = Flt{1};
            this->mat[3] = Flt{1};
        }

        //! Set this matrix up so that it would rotate a 2D vector by rot_rad radians, anticlockwise.
        void rotate (const Flt rot_rad)
        {
            this->mat[0] = std::cos (rot_rad);
            this->mat[1] = std::sin (rot_rad);
            this->mat[2] = -this->mat[1]; // -sin
            this->mat[3] = this->mat[0];
        }

        //! Access elements of the matrix
        Flt& operator[] (size_t idx) { return this->mat[idx]; }
        // note: assume Flt is a built-in type here (safe - Flt will be float or double)
        const Flt operator[] (size_t idx) const  { return this->mat[idx]; }

        //! Access a given row of the matrix
        morph::vec<Flt, 2> row (size_t idx) const
        {
            morph::vec<Flt, 2> r = {Flt{0}, Flt{0}};
            if (idx > 1) { return r; }
            r[0] = this->mat[idx];
            r[1] = this->mat[idx+2];
            return r;
        }

        //! Access a given column of the matrix
        morph::vec<Flt, 2> col (size_t idx) const
        {
            morph::vec<Flt, 3> c = {Flt{0}, Flt{0}};
            if (idx > 1) { return c; }
            idx *= 2;
            c[0] = this->mat[idx];
            c[1] = this->mat[++idx];
            return c;
        }

        //! Transpose this matrix
        void transpose()
        {
            Flt a = this->mat[2];
            this->mat[2] = this->mat[1];
            this->mat[1] = a;
        }

        //! Transpose the matrix @matrx, returning the transposed version.
        morph::vec<Flt, 4> transpose (const morph::vec<Flt, 4>& matrx) const
        {
            morph::vec<Flt, 4> tposed;
            tposed[0] = matrx[0];
            tposed[1] = matrx[2];
            tposed[2] = matrx[1];
            tposed[3] = matrx[3];
            return tposed;
        }

        //! Compute determinant for column-major 2x2 matrix @cm
        static Flt determinant (morph::vec<Flt, 4> cm)
        {
            return ((cm[0]*cm[3]) - (cm[1]*cm[2]));
        }

        Flt determinant() const
        {
            return ((this->mat[0]*this->mat[3]) - (this->mat[1]*this->mat[2]));
        }

        morph::vec<Flt, 4> adjugate() const
        {
            morph::vec<Flt, 4> adj = { this->mat[3], -this->mat[1], -this->mat[2], this->mat[0] };
            return adj;
        }

        Matrix22<Flt> invert()
        {
            Matrix22<Flt> rtn;
            Flt det = this->determinant();
            if (det == 0) {
                std::cout << "NB: The transform matrix has no inverse (determinant is 0)" << std::endl;
                rtn.mat.zero();
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
            for (unsigned int i = 0; i<4; ++i) { this->mat[i] *= f; }
        }

        //! Right-multiply this->mat with m2.
        void operator*= (const morph::vec<Flt, 4>& m2)
        {
            morph::vec<Flt, 4> result;
            // Top row
            result[0] = this->mat[0] * m2[0] + this->mat[2] * m2[1];
            result[2] = this->mat[0] * m2[2] + this->mat[2] * m2[3];
            // Second row
            result[1] = this->mat[1] * m2[0] + this->mat[3] * m2[1];
            result[3] = this->mat[1] * m2[2] + this->mat[3] * m2[3];
            this->mat.swap (result);
        }

        //! Right-multiply this->mat with m2.
        void operator*= (const Matrix22<Flt>& m2)
        {
            morph::vec<Flt, 4> result;
            // Top row
            result[0] = this->mat[0] * m2.mat[0] + this->mat[2] * m2.mat[1];
            result[2] = this->mat[0] * m2.mat[2] + this->mat[2] * m2.mat[3];
            // Second row
            result[1] = this->mat[1] * m2.mat[0] + this->mat[3] * m2.mat[1];
            result[3] = this->mat[1] * m2.mat[2] + this->mat[3] * m2.mat[3];
            this->mat.swap (result);
        }

        //! Return this->mat * m2
        Matrix22<Flt> operator* (const morph::vec<Flt, 4>& m2) const
        {
            Matrix22<Flt> result;
            // Top row
            result.mat[0] = this->mat[0] * m2[0] + this->mat[2] * m2[1];
            result.mat[2] = this->mat[0] * m2[2] + this->mat[2] * m2[3];
            // Second row
            result.mat[1] = this->mat[1] * m2[0] + this->mat[3] * m2[1];
            result.mat[3] = this->mat[1] * m2[2] + this->mat[3] * m2[3];
            return result;
        }

        //! Return this-> mat * m2
        Matrix22<Flt> operator* (const Matrix22<Flt>& m2) const
        {
            Matrix22<Flt> result;
            // Top row
            result.mat[0] = this->mat[0] * m2.mat[0] + this->mat[2] * m2.mat[1];
            result.mat[2] = this->mat[0] * m2.mat[2] + this->mat[2] * m2.mat[3];
            // Second row
            result.mat[1] = this->mat[1] * m2.mat[0] + this->mat[3] * m2.mat[1];
            result.mat[3] = this->mat[1] * m2.mat[2] + this->mat[3] * m2.mat[3];
            return result;
        }

        //! Do matrix times vector multiplication, v = mat * v1
        morph::vec<Flt, 2> operator* (const morph::vec<Flt, 2>& v1) const
        {
            morph::vec<Flt, 2> v = {
                this->mat[0] * v1[0] + this->mat[2] * v1[1],
                this->mat[1] * v1[0] + this->mat[3] * v1[1]
            };
            return v;
        }

        //! Overload the stream output operator
        friend std::ostream& operator<< <Flt> (std::ostream& os, const Matrix22<Flt>& tm);
    };

    template <typename Flt>
    std::ostream& operator<< (std::ostream& os, const Matrix22<Flt>& tm)
    {
        os << tm.str();
        return os;
    }

} // namespace morph
