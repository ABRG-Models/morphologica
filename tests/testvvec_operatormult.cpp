/*
 * Test the different possibilities for multiplying a vvec of scalars/vecs by scalar/vec/vvec etc
 */

#include <morph/vvec.h>
#include <morph/vec.h>

int main()
{
    int rtn = 0;

    // Operands

    // vvec of scalars
    morph::vvec<int> v_scal = { 1, 2, 3 };

    // vvec of vecs
    morph::vvec<morph::vec<int, 2>> v_vec2 = { { 1, 1 },    { 2, 2 },    {3, 3 } };
    morph::vvec<morph::vec<int, 3>> v_vec3 = { { 1, 1, 1 }, { 2, 2, 2 }, {3, 3, 3 } };

    // vvec of vvecs
    morph::vvec<morph::vvec<int>> v_vvec2 =  { { 1, 1 },    { 2, 2 },    {3, 3 } };
    morph::vvec<morph::vvec<int>> v_vvec3 =  { { 1, 1, 1 }, { 2, 2, 2 }, {3, 3, 3 } };

    // A scalar for multiplications
    int s = 10;
    // Vecs for mults
    [[maybe_unused]] morph::vec<int, 2> vec2 = { 10, 100 };
    [[maybe_unused]] morph::vec<int, 3> vec3 = { 10, 100, 1000 };
    morph::vvec<int> vvec_f2 = { 10, 100 };
    morph::vvec<int> vvec_f3 = { 10, 100, 1000 };

    /**
     * vvec<scalars> times stuff
     */
    
    auto result1 = v_scal * s;
    std::cout << "01: " << result1 << std::endl;
    if (result1 != morph::vvec<int>{10, 20, 30}) { --rtn; }

#ifdef SHOULD_NOT_COMPILE
    auto result2 = v_scal * vec2; // Don't support vvec<scalar> * vec<scalar> (esp. not with diff dims)
    std::cout << "02: " << result2 << std::endl;
    auto result3 = v_scal * vec3; // Don't support vvec<scalar> * vec<scalar> (even though we could in principle with same dimensions)
    std::cout << "03: " << result3 << std::endl;
#endif
    
    auto result4 = v_scal * vvec_f3;
    std::cout << "04: " << result4 << std::endl;
    if (result4 != morph::vvec<int>{10, 200, 3000}) { --rtn; }

    try {
        auto result5 = v_scal * vvec_f2; // should not be able to multiply 3D vvec of scalars by 2D vvec of scalars
        std::cout << "05: " << result5 << std::endl;
        --rtn;
    } catch (const std::exception& e) {
        // Expected exception
        std::cout << "05: " << "Expected exception: " << e.what() << std::endl;
    }
    
    /**
     * vvec<vecs> times stuff
     */
    
    auto result6 = v_vec2 * s;
    std::cout << "06: " << result6 << std::endl;
    if (result6 != morph::vvec<morph::vec<int, 2>>{{10,10}, {20,20}, {30,30}}) { --rtn; }

    auto result7 = v_vec3 * s;
    std::cout << "07: " << result7 << std::endl;

    auto result8 = v_vec2 * vec2;
    std::cout << "08: " << result8 << std::endl;
#ifdef SHOULD_NOT_COMPILE
    auto result9 = v_vec2 * vec3; // vvec of 2D vecs times a 3D vec makes no sense. Compiler correctly refuses to comply:
                                  // vvec.h:1819:63: error: no match for 'operator*' (operand types are 'morph::vec<int, 2>' and 'const morph::vec<int, 3>')

    std::cout << "09: " << result9 << std::endl;
#endif

#ifdef SHOULD_NOT_COMPILE
    auto result10 = v_vec3 * vec2; // vvec of 3D vecs times a 2D vec makes no sense. Compiler correctly refuses to comply:
                                   // vvec.h:1819:63: error: no match for 'operator*' (operand types are 'morph::vec<int, 3>' and 'const morph::vec<int, 2>')

    std::cout << "10: " << result10 << std::endl;
#endif
    auto result11 = v_vec3 * vec3;
    std::cout << "11: " << result11 << std::endl;

    try { 
        auto result12 = v_vec2 * vvec_f2; // vvec<vec<int, 2> size 3 * vvec<int> size 2 compiles, but runtime error
        std::cout << "12: " << result12 << std::endl;
        --rtn;
    } catch (const std::exception& e) {
        // Expected exception
        std::cout << "12: " << "Expected exception: " << e.what() << std::endl;
    }
    
    auto result13 = v_vec2 * vvec_f3; // vvec<vec<int, 2> size 3 * vvec<int> size 3 does scalar multiplication of each vec<int, 2> by the 3 scalars in the vvec<int>
    std::cout << "13: " << result13 << std::endl;
    if (result13 != morph::vvec<morph::vec<int, 2>>{{10,10}, {200,200}, {3000,3000}}) { --rtn; }
    
    try { 
        auto result14 = v_vec3 * vvec_f2; // vvec<vec<int, 3> size 3 * vvec<int> size 2, but runtime error as vvecs have diff. sizes
        std::cout << "14: " << result14 << std::endl;
        --rtn;
    } catch (const std::exception& e) {
        // Expected exception
        std::cout << "14: " << "Expected exception: " << e.what() << std::endl;
    }
    
    auto result15 = v_vec3 * vvec_f3;
    std::cout << "15: " << result15 << std::endl;
    if (result15 != morph::vvec<morph::vec<int, 3>>{{10,10,10}, {200,200,200}, {3000,3000,3000}}) { --rtn; }



    /**
     * vvec<vvecs> times stuff
     */
    
    auto result16 = v_vvec2 * s;
    std::cout << "16: " << result16 << std::endl;
    if (result16 != morph::vvec<morph::vvec<int>>{{10,10}, {20,20}, {30,30}}) { --rtn; }

    auto result17 = v_vvec3 * s;
    std::cout << "17: " << result17 << std::endl;
    if (result17 != morph::vvec<morph::vvec<int>>{{10,10,10}, {20,20,20}, {30,30,30}}) { --rtn; }


#ifdef SHOULD_NOT_COMPILE
    auto result18 = v_vvec2 * vec2; // vvec of 2D vvecs times a vec of 2D elements does not compile
                                    // - attempts to compile the vvec times morph::vec operator*,
                                    // which then sub-calls down into vvec<int> times
                                    // morph::vec<int, 2> and cannot do int * vec<int, 2> is an
                                    // int. It's ok that this doesn't compile.
    std::cout << "18: " << result18 << std::endl;
    
    auto result19 = v_vvec2 * vec3; // Don't expect this to compile either
    std::cout << "19: " << result19 << std::endl;

    auto result20 = v_vvec3 * vec2;
    std::cout << "20: " << result20 << std::endl;

    auto result21 = v_vvec3 * vec3;
    std::cout << "21: " << result21 << std::endl;
#endif

    try { 
        auto result22 = v_vvec2 * vvec_f2; // vvec<vvec<int> size 2> size 3 * vvec<int> size 2 compiles, but runtime error due to size mis-match
        std::cout << "22: " << result22 << std::endl;
        --rtn;
    } catch (const std::exception& e) {
        // Expected exception
        std::cout << "22: " << "Expected exception: " << e.what() << std::endl;
    }

    // There's an argument to disable this one:
    auto result23 = v_vvec2 * vvec_f3; // vvec<vvec<int> size 2> size 3 * vvec<int> size 3 does scalar multiplication of each vec<int, 2> by the 3 scalars in the vvec<int>
    std::cout << "23: " << result23 << std::endl;
    if (result23 != morph::vvec<morph::vvec<int>>{{10,10}, {200,200}, {3000,3000}}) { --rtn; }
    
    try { 
        auto result24 = v_vvec3 * vvec_f2; // vvec<vvec<int> size 3> size 3 * vvec<int> size 2 compiles, but runtime error?
        std::cout << "24: " << result24 << std::endl;
    } catch (const std::exception& e) {
        // Expected exception
        std::cout << "24: " << "Expected exception: " << e.what() << std::endl;
    }
    
    auto result25 = v_vvec3 * vvec_f3; // vvec<vvec<int> size 3> size 3 * vvec<int> size 3 compiles and runs
    std::cout << "25: " << result25 << std::endl;
    if (result25 != morph::vvec<morph::vvec<int>>{{10,10,10}, {200,200,200}, {3000,3000,3000}}) { --rtn; }
    
    std::cout << "rtn: " << rtn << (rtn ? " [BAD]" : " [GOOD]") << std::endl;
    return rtn;
}
