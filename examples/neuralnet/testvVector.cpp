#include "_vVector.h"
using morph::vVector;
using std::cout;
using std::endl;
using std::array;

int main() {
    int rtn = 0;
    vVector<float> v1 (12000, 2.0f);

    vVector<float> v2 (12000, 3.0f);

    float a = 0.0f;
    for (size_t i = 0; i < 100000; ++i) {
        v2[0] = (float)i;
        a = v1.dot (v2);
    }

    cout << "a = " << a << endl;

    return rtn;
}
