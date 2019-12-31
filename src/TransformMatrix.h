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
            //cout << "3x3 determinant computation..." << endl;
            Flt det = (cm[0]*cm[4]*cm[8])
                + (cm[3]*cm[7]*cm[2])
                + (cm[6]*cm[1]*cm[5])
                - (cm[6]*cm[4]*cm[2])
                - (cm[0]*cm[7]*cm[5])
                - (cm[3]*cm[1]*cm[8]);
            cout << "Determinant of \n"
                 << "| " << cm[0] << " , " << cm[3] << " , " << cm[6] << " |\n"
                 << "| " << cm[1] << " , " << cm[4] << " , " << cm[7] << " |\n"
                 << "| " << cm[2] << " , " << cm[5] << " , " << cm[8] << " | is...\n"
                 << det << endl;
            return det;
        }

        //! Compute determinant for 4x4 matrix @cm
        Flt determinant (array<Flt, 16> cm) const {
            //cout << "4x4 determinant computation..." << endl;
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

            cout << "Determinant of \n"
                 << "| " << cm[0] << " , " << cm[4] << " , " << cm[8] << " , " << cm[12] << " |\n"
                 << "| " << cm[1] << " , " << cm[5] << " , " << cm[9] << " , " << cm[13] << " |\n"
                 << "| " << cm[2] << " , " << cm[6] << " , " << cm[10] << " , " << cm[14] << " |\n"
                 << "| " << cm[3] << " , " << cm[7] << " , " << cm[11] << " , " << cm[15] << " |\n"
                 << det << endl;

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

        // Implement inversion using determinant method.
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

        //! First attempt at inverting this matrix and return the inverted
        //! copy. Computational implementation of Gauss-Jordan elimination.
        TransformMatrix<Flt> invertGJ (void) {

            // 1. Create adjunct matrix |AI| *in row-major format* (where this->mat is in col-major)
            array<Flt, 32> adj;

            // Copy mat (which is column-major) into left-box of adj (which is row-major):
            adj[0] = this->mat[0];
            adj[1] = this->mat[4];
            adj[2] = this->mat[8];
            adj[3] = this->mat[12];

            adj[8] = this->mat[1];
            adj[9] = this->mat[5];
            adj[10] = this->mat[9];
            adj[11] = this->mat[13];

            adj[16] = this->mat[2];
            adj[17] = this->mat[6];
            adj[18] = this->mat[10];
            adj[19] = this->mat[14];

            adj[24] = this->mat[3];
            adj[25] = this->mat[7];
            adj[26] = this->mat[11];
            adj[27] = this->mat[15];

            // Set I in right-box of adj:
            adj[4] = 1.0;  // 4+0
            adj[5] = 0;
            adj[6] = 0;
            adj[7] = 0;

            adj[12] = 0;
            adj[13] = 1.0; // 12+1
            adj[14] = 0;
            adj[15] = 0;

            adj[20] = 0;
            adj[21] = 0;
            adj[22] = 1.0; // 20+2
            adj[23] = 0;

            adj[28] = 0;
            adj[29] = 0;
            adj[30] = 0;
            adj[31] = 1.0; // 28+3

            // Perform Gauss-Jordan elimination
            this->gaussJordan (adj);

            // FIXME: Now test whether the matrix was invertible (by looking at left most
            // matrix, and seeing if it's the identity)

            // Finally, select the right hand 4x4, which should be the inverse matrix
            // and return it.
            TransformMatrix<Flt> invm;
            invm.mat[0] = adj[4];
            invm.mat[4] = adj[5];
            invm.mat[8] = adj[6];
            invm.mat[12] = adj[7];

            invm.mat[1] = adj[12];
            invm.mat[5] = adj[13];
            invm.mat[9] = adj[14];
            invm.mat[13] = adj[15];

            invm.mat[2] = adj[20];
            invm.mat[6] = adj[21];
            invm.mat[10] = adj[22];
            invm.mat[14] = adj[23];

            invm.mat[3] = adj[28];
            invm.mat[7] = adj[29];
            invm.mat[11] = adj[30];
            invm.mat[15] = adj[31];

            return invm;
        }

        //! Perform Gauss-Jordan elimination on the 4x8 matrix @adj
        void gaussJordan (array<Flt, 32>& adj) {
#if 0
            // Step 1. In practice, no row will be all zero, because we hang an identity
            // matrix to the right of the original transform matrix.
            bool haveZero = false;
            for (int i = 3; i>=0; --i) {
                Flt sumi = static_cast<Flt>(0.0);
                for (unsigned int j = (i*4); j<(i*4)+8; ++j) { sumi += adj[j]; }
                if (sumi == static_cast<Flt>(0.0)) {
                    haveZero = true;
                }
            }

            if (haveZero == true) {
                cout << "HAD ZERO ROW!" << endl;
                // FIXME: Debug this:
                // Swap rows so that zero rows are at bottom:
                for (unsigned int i = 4; i>0; --i) {
                    Flt sumi = static_cast<Flt>(0.0);
                    for (unsigned int j = ((i-1)*4); j<((i-1)*4)+8; ++j) { sumi += adj[j]; }
                    if (sumi > 0.0) {
                        // Put this row in position 0.
                        // First make a memory of this row
                        array<Flt, 32> arrcopy;
                        // Shift all the preceding data along
                        unsigned int k = 0;
                        for (unsigned int j = 8; j<(i*8); ++j) {
                            arrcopy[j] = adj[k++];
                        }
                        // Copy the sum-0 section across
                        k = (i-1)*8;
                        for (unsigned int j = 0; j<8; ++j) {
                            arrcopy[j] = adj[k++];
                        }
                        // Now copy it back into adj
                        for (unsigned int j = 0; j<32; ++j) {
                            adj[j] = arrcopy[j];
                        }

                        cout << "----" << endl;
                        outputadj(adj);
                        cout << "----" << endl;
                    }
                }
            }
#endif
            cout << "---- Step1: " << endl;
            outputadj(adj);
            cout << "----" << endl;

            bool finished = false;
            unsigned int loopcount = 0;
            while (!finished) {
                // Step 2. Swap the rows so that the row with the largest, leftmost nonzero
                // entry is on top.
                array<Flt, 8> rowcopy;
                unsigned int swapped = 1;
                unsigned int col = loopcount;
                cout << "Loop for column " << col << endl;
                cout << "---- (in) Step2: " << endl;
                outputadj(adj);
                cout << "----" << endl;

                while (swapped) {
                    swapped = 0;
                    unsigned int nonzero_incol = 0;
                    // Bubble sort... sorta
                    for (unsigned int i = 0; i < 3; ++i) {
                        unsigned int left = (i*8);
                        unsigned int leftnext = ((1+i)*8);
                        unsigned int coli = left+col;
                        unsigned int colinext = leftnext+col;
                        // Check the columns before col
                        Flt leftsum = 0.0;
                        Flt leftnextsum = 0.0;
                        for (unsigned int j = 0; j<col; ++j) {
                            leftsum += adj[left+j];
                            leftnextsum += adj[leftnext+j];
                        }
                        if (adj[coli] == 0.0 && adj[colinext] == 0.0
                            || leftsum > 0.0 || leftnextsum > 0.0) {
                            // If both zero, OR preceding cols non zero, move on.
                        } else {
                            if (adj[coli] != 0.0) {
                                nonzero_incol++;
                            }
                            // On last row, count both the current row and the next row
                            // so that nonzero_incol may count up to the correct number
                            // of rows
                            if (i==2 && adj[colinext] != 0.0) {
                                nonzero_incol++;
                            }
                            // This is not quite such a simple operation.  If coli is 0
                            // and colinext is -ve, then swap If coli is finite and
                            // colinext is 0, then swap anyway?  BUT what about col--?
                            // YES I need to only swap if all leading cols are zero -
                            // otherwise, I swap the rows above. (FIXME)
                            //
                            bool doswap = false;
                            if (adj[coli] == 0.0) {
                                // swap
                                doswap = true;
                            } else if (adj[colinext] == 0) {
                                // Don't swap
                            } else if (adj[coli] < adj[colinext]) {
                                doswap = true;
                            }
                            // BUT we might miss something? FIXME, think harder.
                            if (doswap) {
                                unsigned int k = 0;
                                // Copy 'left' row into rowcopy
                                for (unsigned int j = left; j<left+8; ++j) {
                                    rowcopy[k++] = adj[j];
                                }
                                // Copy leftnext into left
                                k = left;
                                for (unsigned int j = leftnext; j<leftnext+8; ++j) {
                                    adj[k++] = adj[j];
                                }
                                // Copy rowcopy (ie left) into leftnext
                                k = 0;
                                for (unsigned int j = leftnext; j<leftnext+8; ++j) {
                                    adj[j] = rowcopy[k++];
                                }
                                swapped++;

                                cout << "---- (in) Step 2 swapped: " << endl;
                                outputadj(adj);
                                cout << "----" << endl;
                            }
                        }
                    }
                    cout << "Col " << col << " num non-zero: " << nonzero_incol << endl;
                    // if swapped > 0, then some rows were swapped on that pass.
                    if (swapped == 0 && nonzero_incol == 0 && col < 7) {
                        swapped = 1; // To move onto next loop in while
                        col += 1; // To look at the next column
                        cout << "Move to next column " << col << endl;
                    } // otherwise, we had a non-zero entry in the column, and it must have been at the top
                } // end bubble sort on left most entry.
                cout << "---- Step 2 (end): " << endl;
                outputadj(adj);
                cout << "----" << endl;

                // Step 3. Multiply the top row by a scalar so that top row's leading entry
                // becomes 1. This needs to be the 'top' row, so based on loopcount.
                col = loopcount; // reset col
                cout << "At Step 3. loopcount=" << loopcount << ", col=" << col << endl;
                Flt scalar = static_cast<Flt>(1.0)/adj[loopcount*8+col];
                cout << "scalar = " << scalar << " (1/" << adj[loopcount*8+col] << ")" << endl;
                for (unsigned int j = loopcount*8; j<(loopcount*8)+8; ++j) {
                    adj[j] = adj[j]*scalar;
                }
                cout << "---- Step 3: " << endl;
                outputadj(adj);
                cout << "----" << endl;

                // Step 4. Add/subtract multiples of the top row to the other rows so that
                // all other entries in the column containing the top row's leading entry
                // are all zero.
                // This must be based on loopcount which points to the top row
                for (unsigned int i = loopcount+1; i<4; ++i) {
                    unsigned int left = (i*8);
                    unsigned int coli = left+col;
                    if (adj[coli] != 0.0) {
                        Flt factor = adj[coli];
                        cout << "Step 4. Factor = " << factor << endl;
                        // Now multiply 'top' row by factor and subtract from row i
                        unsigned int l = loopcount*8;
                        for (unsigned int j = left; j<left+8; ++j) {
                            Flt f = adj[l++] * factor;
                            cout << "adj[" << j << "] = " << adj[j] << " - " << f << endl;
                            adj[j] = adj[j] - f;
                        }
                    }
                }
                cout << "---- Step 4: " << endl;
                outputadj(adj);
                cout << "----" << endl;

                // Step 5: Repeat steps 2 to 4 until finished.
                ++loopcount;
                if (loopcount == 4) {
                    finished = true;
                }
            }
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

        //! Do v = mat * v1
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
