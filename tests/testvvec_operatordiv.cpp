/*
 * Test the different possibilities for dividing a vvec of scalars/vecs by scalar/vec/vvec etc
 */

#include <morph/vvec.h>
#include <morph/vec.h>

int main()
{
    int rtn = 0;

    // Operands

    // vvec of scalars
    morph::vvec<float> v_scal = { 1000, 2000, 3000 };

    // vvec of vecs
    morph::vvec<morph::vec<float, 2>> v_vec2 = { { 1000, 1000 },    { 2000, 2000 },    {3000, 3000 } };
    morph::vvec<morph::vec<float, 3>> v_vec3 = { { 1000, 1000, 1000 }, { 2000, 2000, 2000 }, {3000, 3000, 3000 } };

    // vvec of vvecs
    morph::vvec<morph::vvec<float>> v_vvec2 =  { { 1000, 1000 },    { 2000, 2000 },    {3000, 3000 } };
    morph::vvec<morph::vvec<float>> v_vvec3 =  { { 1000, 1000, 1000 }, { 2000, 2000, 2000 }, {3000, 3000, 3000 } };

    // A scalar for divisions
    float s = 10;
    // Vecs for mults
    [[maybe_unused]] morph::vec<float, 2> vec2 = { 10, 100 };
    [[maybe_unused]] morph::vec<float, 3> vec3 = { 10, 100, 1000 };
    morph::vvec<float> vvec_f2 = { 10, 100 };
    morph::vvec<float> vvec_f3 = { 10, 100, 1000 };

    /**
     * vvec<scalars> div by stuff
     */
    
    auto result1 = v_scal / s;
    std::cout << "01: " << result1 << std::endl;
    if (result1 != morph::vvec<float>{100, 200, 300}) { std::cout << "01bad\n"; --rtn; }

#ifdef SHOULD_NOT_COMPILE
    auto result2 = v_scal / vec2;
    std::cout << "02: " << result2 << std::endl;
    auto result3 = v_scal / vec3;
    std::cout << "03: " << result3 << std::endl;
#endif
    
    auto result4 = v_scal / vvec_f3;
    std::cout << "04: " << result4 << std::endl;
    if (result4 != morph::vvec<float>{100, 20, 3}) { std::cout << "04bad\n"; --rtn; }

    try {
        auto result5 = v_scal / vvec_f2;
        std::cout << "05bad: " << result5 << std::endl;
        --rtn;
    } catch (const std::exception& e) {
        // Expected exception
        std::cout << "05: " << "Expected exception: " << e.what() << std::endl;
    }
    
    /**
     * vvec<vecs> div by stuff
     */
    
    auto result6 = v_vec2 / s;
    std::cout << "06: " << result6 << std::endl;
    if (result6 != morph::vvec<morph::vec<float, 2>>{{100,100}, {200,200}, {300,300}}) { std::cout << "06bad\n"; --rtn; }

    auto result7 = v_vec3 / s;
    std::cout << "07: " << result7 << std::endl;

    auto result8 = v_vec2 / vec2;
    std::cout << "08: " << result8 << std::endl;
#ifdef SHOULD_NOT_COMPILE
    auto result9 = v_vec2 / vec3;
    std::cout << "09: " << result9 << std::endl;
#endif

#ifdef SHOULD_NOT_COMPILE
    auto result10 = v_vec3 / vec2;
    std::cout << "10: " << result10 << std::endl;
#endif
    auto result11 = v_vec3 / vec3;
    std::cout << "11: " << result11 << std::endl;

    try { 
        auto result12 = v_vec2 / vvec_f2;
        std::cout << "12bad: " << result12 << std::endl;
        --rtn;
    } catch (const std::exception& e) {
        std::cout << "12: " << "Expected exception: " << e.what() << std::endl;
    }
    
    auto result13 = v_vec2 / vvec_f3;
    std::cout << "13: " << result13 << std::endl;
    if (result13 != morph::vvec<morph::vec<float, 2>>{{100,100}, {20,20}, {3,3}}) { std::cout << "13bad\n"; --rtn; }
    
    try { 
        auto result14 = v_vec3 / vvec_f2;
        std::cout << "14bad: " << result14 << std::endl;
        --rtn;
    } catch (const std::exception& e) {
        std::cout << "14: " << "Expected exception: " << e.what() << std::endl;
    }
    
    auto result15 = v_vec3 / vvec_f3;
    std::cout << "15: " << result15 << std::endl;
    if (result15 != morph::vvec<morph::vec<float, 3>>{{100,100,100}, {20,20,20}, {3,3,3}}) { std::cout << "15bad\n"; --rtn; }



    /**
     * vvec<vvecs> div by stuff
     */
    
    auto result16 = v_vvec2 / s;
    std::cout << "16: " << v_vvec2 << " / " << s << " = " << result16 << std::endl;
    if (result16 != morph::vvec<morph::vvec<float>>{{100,100}, {200,200}, {300,300}}) { std::cout << "16bad\n"; --rtn; }

    auto result17 = v_vvec3 / s;
    std::cout << "17: " << v_vvec3 << " / " << s << " = " << result17 << std::endl;
    if (result17 != morph::vvec<morph::vvec<float>>{{100,100,100}, {200,200,200}, {300,300,300}}) { std::cout << "17bad\n"; --rtn; }


#ifdef SHOULD_NOT_COMPILE
    auto result18 = v_vvec2 / vec2;
    std::cout << "18: " << result18 << std::endl;
    
    auto result19 = v_vvec2 / vec3;
    std::cout << "19: " << result19 << std::endl;

    auto result20 = v_vvec3 / vec2;
    std::cout << "20: " << result20 << std::endl;

    auto result21 = v_vvec3 / vec3;
    std::cout << "21: " << result21 << std::endl;
#endif

    try { 
        auto result22 = v_vvec2 / vvec_f2;
        std::cout << "22bad: " << result22 << std::endl;
        --rtn;
    } catch (const std::exception& e) {
        std::cout << "22: " << "Expected exception: " << e.what() << std::endl;
    }

    auto result23 = v_vvec2 / vvec_f3;
    std::cout << "23: " << result23 << std::endl;
    if (result23 != morph::vvec<morph::vvec<float>>{{100,100}, {20,20}, {3,3}}) { std::cout << "23bad\n"; --rtn; }
    
    try { 
        auto result24 = v_vvec3 / vvec_f2;
        std::cout << "24bad: " << result24 << std::endl;
        --rtn;
    } catch (const std::exception& e) {
        std::cout << "24: " << "Expected exception: " << e.what() << std::endl;
    }
    
    auto result25 = v_vvec3 / vvec_f3;
    std::cout << "25: " << result25 << std::endl;
    if (result25 != morph::vvec<morph::vvec<float>>{{100,100,100}, {20,20,20}, {3,3,3}}) { std::cout << "25bad\n"; --rtn; }
    
    std::cout << "rtn: " << rtn << (rtn ? " [BAD]" : " [GOOD]") << std::endl;
    return rtn;
}
