#include "morph/Quaternion.h"
#include "morph/vec.h"

int main()
{
    int rtn = 0;

    morph::Quaternion<float> q;
    std::cout << q << std::endl;
    q.renormalize();
    std::cout << q << std::endl;

    morph::Quaternion<float> rotationQuaternion;
    float angularSpeed = 0.2;
    morph::vec<float> rotationAxis = {1.0f, 0.0f, 0.0f};
    rotationQuaternion.initFromAxisAngle (rotationAxis, angularSpeed);
    std::cout << rotationQuaternion << std::endl;

    morph::Quaternion<float> p = q;
    if (p == q) {  } else { rtn++; }
    if (p != q) { rtn++; }

    return rtn;
}
