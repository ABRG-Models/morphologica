#include "morph/Vector.h"
using morph::Vector;
using std::cout;
using std::endl;
using std::array;

int main() {
    int rtn = 0;
    Vector<float, 4> v = {1,2,3};
    // Test x(), y() etc members
    cout << "x: " << v.x() << endl;
    cout << "z: " << v.z() << endl;
    cout << "w: " << v.w() << endl;
    // Test renormalize
    v.renormalize();
    cout << "After renormalize: " << v << endl;
    // Test checkunit
    cout << "is it unit? " << v.checkunit() << endl;
    // Test randomize
    v.randomize(3,4);
    cout << "After randomize: " << v << endl;
    // Check ints are ok, too
    Vector<int, 2> vi;
    vi.randomize(0,200);
    cout << "After randomize of int vector: " << vi << endl;
    cout << "Length: " << vi.length() << endl;
    // Test assignment
    Vector<int, 2> vi2 = vi;
    cout << "Copy of int vector: " << vi2 << endl;
    // Test comparison
    cout << "vi == vi2? " << (vi == vi2 ? "yes" : "no") << endl;
    // Test negate
    Vector<int, 2> vi3 = -vi;
    cout << "-ve Copy of int vector: " << vi3 << endl;
    // Test comparison
    cout << "vi == vi3? " << (vi == vi3 ? "yes" : "no") << endl;
    // Test cross product (3D only
    Vector<double, 3> a = {1,0,0};
    Vector<double, 3> b = {0,1,0};
    Vector<double, 3> c = a.cross(b);
    cout << a << " cross " << b << "=" << c << endl;
    // Test dot product
    Vector<int, 2> vv1 = {1,1};
    Vector<int, 2> vv2 = {2,2};
    int dp = vv1.dot (vv2);
    cout << vv1 << "." << vv2 << " = " << dp << endl;
#if 0 // No good:
    // Test init from array
    array<float, 3> arr = { 2,3,4 };
    Vector<float, 3> varr = arr; // Tried overloading operator= to no avail.
    cout << "Vector from array: " << varr << endl;
#endif
    // Test scalar multiply
    vv2 *= 2UL;
    cout << "vv2 after *2:" << vv2 << endl;
    Vector<int, 2> vv4 = vv1 * 98;
    cout << "vv1 * 98:" << vv4 << endl;
    // Scalar division
    Vector<double, 3> d = a/3.0;
    cout << "a/3.0:" << d << endl;
    // Vector addition
    Vector<double, 3> e = a+b;
    cout << "a+b:" << e << endl;
    // Vector subtraction
    Vector<double, 3> f = a-b;
    cout << "a-b:" << f << endl;
    // Test default template args
    Vector<double> vd_def;
    vd_def.randomize();
    cout << vd_def << endl;
    Vector<> v_def;
    v_def.randomize();
    cout << v_def << endl;

    // So you want to do the dot product of a 1000000 D vector? Easy
    Vector<float, 1000> big1;
    Vector<float, 1000> big2;
    big1.randomize(0,100);
    big2.randomize(0,20);
    cout << "DP..." << endl;
    float bdp = big1.dot(big2);
    cout << "big1.big2=" << bdp << endl;

    // Test setFrom
    Vector<double, 3> d1;
    array<double, 3> a1 = { 5,6,7 };
    d1.set_from (a1);
    cout << "d1 should be 5,6,7: " << d1 << endl;
    array<double, 4> a2 = { 5,6,8,8 };
    d1.set_from (a2);
    cout << "d1 should be 5,6,8: " << d1 << endl;
    d1.set_from (5.6);
    cout << "d1 should be 5.6 for all elements: " << d1 << endl;

    // Test hadamard operator* (elementwise multiplication)
    Vector<double, 3> h1 = {1.0, 2.0, 3.0};
    Vector<double, 3> h2 = {7.0, 6.0, 5.0};
    Vector<double, 3> h3 = h1 * h2;
    cout << h1 << "(o)" << h2 << " = " << h3 << endl;

    h1 *= h2;
    cout << "After h1 *= h2, h1: " << h1 << endl;

    // Test operator *= with different types. Ok if lhs is same type as result.
    Vector<int, 3> h4 = {2, 2, 2};
    //Vector<int, 3> h5 = h2 * h4; // Not ok
    Vector<int, 3> h6 = h4 * h2;
    Vector<double, 3> h7 = h2 * h4;
    //Vector<double, 3> h8 = h4 * h2; // Not ok
    cout << h2 << "(o)" << h4 << " = " << h6 << " or " << h7 << endl;

    Vector<float, 4> maxlongest = {-1.1f, -7.0f, 3.0f, 6.0f };
    cout << "For vector " << maxlongest
         << ", max: " << maxlongest.max() << " (at index "<< maxlongest.argmax()
         << "), longest component: " << maxlongest.longest() << " (at index "
         << maxlongest.arglongest() << ")\n";

    Vector<float, 4> totimes = { 1.0f, 2.0f, 3.0f, 4.0f };
    cout << "Cumulative product of " << totimes << " is " << totimes.product() << endl;

    Vector<float, 4> fordivision = { 1.0f, 2.0f, 3.0f, 4.0f };
    Vector<float, 4> divresult = 1.0f / fordivision;
    std::cout << 1.0f << " / " << fordivision << " = " << divresult << std::endl;

    return rtn;
}
