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

    return 0;
}
