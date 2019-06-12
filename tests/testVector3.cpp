#include "Vector3.h"
using morph::Vector3;

int main()
{
    Vector3<float> v1(1,0,0);
    v1.output();
    Vector3<float> v2(0,1,0);
    v2.output();
    (v1*v2).output();
    v1*=v2;
    v1.output();
    v2*=(unsigned char)3;
    v2.output();
    v2 += (int)5;
    v2.output();

    cout << (v1 == v2) << endl;
    cout << (v2 == v2) << endl;
    return 0;
}
