#include "Quaternion.h"
using morph::Quaternion;
#include "Vector3.h"
using morph::Vector3;

int main()
{
    Quaternion<float> q;
    q.output();
    q.renormalize();
    q.output();

    Quaternion<float> rotationQuaternion;
    float angularSpeed = 0.2;
    Vector3<float> rotationAxis(1.0f,0.0f,0.0f);
    rotationQuaternion.initFromAxisAngle (rotationAxis, angularSpeed);
    rotationQuaternion.output();

    return 0;
}
