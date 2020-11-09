#include "morph/Quaternion.h"
using morph::Quaternion;
#include "morph/Vector.h"
using morph::Vector;

int main()
{
    Quaternion<float> q;
    std::cout << q << std::endl;
    q.renormalize();
    std::cout << q << std::endl;

    Quaternion<float> rotationQuaternion;
    float angularSpeed = 0.2;
    Vector<float> rotationAxis = {1.0f, 0.0f, 0.0f};
    rotationQuaternion.initFromAxisAngle (rotationAxis, angularSpeed);
    std::cout << rotationQuaternion << std::endl;

    return 0;
}
