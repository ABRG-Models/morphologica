#include "morph/Quaternion.h"
#include "morph/vec.h"

int main()
{
    morph::Quaternion<float> q;
    std::cout << q << std::endl;
    q.renormalize();
    std::cout << q << std::endl;

    morph::Quaternion<float> rotationQuaternion;
    float angularSpeed = 0.2;
    morph::vec<float> rotationAxis = {1.0f, 0.0f, 0.0f};
    rotationQuaternion.initFromAxisAngle (rotationAxis, angularSpeed);
    std::cout << rotationQuaternion << std::endl;

    return 0;
}
