#include "TransformMatrix.h"
using morph::TransformMatrix;

int main()
{
    TransformMatrix<float> tm1;
    tm1.setToIdentity();
    tm1.translate(1,1,1);
    tm1.output();

    TransformMatrix<float> tm2;
    tm2.setToIdentity();
    tm2.translate(1.5,0.1,-1.2);
    tm2.output();

    TransformMatrix<float> tm3 = tm1 * tm2;
    tm3.output();

    // Test transpose
    TransformMatrix<float> tm4;
    tm4.mat[0] = 0;
    tm4.mat[1] = 1;
    tm4.mat[2] = 2;
    tm4.mat[3] = 3;
    tm4.mat[4] = 4;
    tm4.mat[5] = 5;
    tm4.mat[6] = 6;
    tm4.mat[7] = 7;
    tm4.mat[8] = 8;
    tm4.mat[9] = 9;
    tm4.mat[10] = 10;
    tm4.mat[11] = 11;
    tm4.mat[12] = 12;
    tm4.mat[13] = 13;
    tm4.mat[14] = 14;
    tm4.mat[15] = 15;

    cout << "pre:" << endl;
    tm4.output();
    cout << "transposed:" << endl;
    tm4.transpose();
    tm4.output();

    return 0;
}
