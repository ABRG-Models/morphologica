#include "morph/vec.h"
using morph::vec;
using std::cout;
using std::endl;
using std::array;

int main() {
    int rtn = 0;
    vec<float, 3> v;
    v[0] = 1.0f;
    v[1] = 2.0f;
    v[2] = 3.0f;
    // Test x(), y() etc members
    cout << "x: " << v.x() << endl;
    cout << "z: " << v.z() << endl;
    //cout << "w: " << v.w() << endl;
    // Test renormalize
    v.renormalize();
    cout << "After renormalize: " << v << endl;
    // Test checkunit
    cout << "is it unit? " << v.checkunit() << endl;
    // Test randomize
    v.randomize();
    cout << "After randomize: " << v << endl;
    // Check ints are ok, too
    vec<int, 2> vi;
    vi.randomize();
    cout << "After randomize of int vector: " << vi << endl;
    cout << "Length: " << vi.length() << endl;
    // Test assignment
    vec<int, 2> vi2 = vi;
    cout << "Copy of int vector: " << vi2 << endl;
    // Test comparison
    cout << "vi == vi2? " << (vi == vi2 ? "yes" : "no") << endl;
    // Test negate
    vec<int, 2> vi3 = -vi;
    cout << "-ve Copy of int vector: " << vi3 << endl;
    // Test comparison
    cout << "vi == vi3? " << (vi == vi3 ? "yes" : "no") << endl;
    // Test cross product (3D only
    vec<double, 3> a{}; a[0] = 1;
    vec<double, 3> b{}; b[1] = 1;
    vec<double, 3> c = a.cross(b);
    cout << a << " cross " << b << "=" << c << endl;
    // Test dot product
    vec<int, 2> vv1{}; vv1[0]=1; vv1[1]=1;
    vec<int, 2> vv2{}; vv2[0]=2; vv2[1]=2;
    int dp = vv1.dot (vv2);
    cout << vv1 << "." << vv2 << " = " << dp << endl;
#if 0 // No good:
    // Test init from array
    array<float, 3> arr = { 2,3,4 };
    //vec<float, 3> varr(arr);
    vec<float, 3> varr = arr;
    cout << "vec from array: " << varr << endl;
#endif
    // Test scalar multiply
    vv2 *= 2UL;
    cout << "vv2 after *2:" << vv2 << endl;
    vec<int, 2> vv4 = vv1 * 98;
    cout << "vv1 * 98:" << vv4 << endl;
    // Scalar division
    vec<double, 3> d = a/3.0;
    cout << "a/3.0:" << d << endl;
    // vec addition
    vec<double, 3> e = a+b;
    cout << "a+b:" << e << endl;
    // vec subtraction
    vec<double, 3> f = a-b;
    cout << "a-b:" << f << endl;
    // Test default template args
    vec<double> vd_def;
    vd_def.randomize();
    cout << vd_def << endl;
#if 0 // no good in c++-11 or c++-14
    vec v_def;
    v_def.randomize();
    cout << v_def << endl;
#endif
    // So you want to do the dot product of a 1000000 D vector? Easy
    vec<float, 1000> big1;
    vec<float, 1000> big2;
    big1.randomize();
    big2.randomize();
    cout << "DP..." << endl;
    float bdp = big1.dot(big2);
    cout << "big1.big2=" << bdp << endl;

    return rtn;
}
