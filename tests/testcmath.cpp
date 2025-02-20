#include <morph/constexpr_math.h>
#include <iostream>

int main()
{
    int rtn = 0;

    //constexpr float input = 4.3f;

    //constexpr float theceil = morph::math::ceil (input);
    //if (theceil < 5.0f) { --rtn; }

    //constexpr float thefloor = morph::math::floor (input);
    //if (theceil > 4.0f) { --rtn; }

    //std::cout << "For input " << input << ", ceil: " << theceil << " and floor: " << thefloor << std::endl;

    constexpr float at2 = morph::math::atan2 (0.5f, 0.5f);
    std::cout << "atan2(.5, .5) - pi/4 = " << (at2 - morph::mathconst<float>::pi_over_4) << std::endl;
    if (at2 - morph::mathconst<float>::pi_over_4 > 0.00001f) { --rtn; }

    constexpr int pow43 = morph::math::pow (4, 3);
    std::cout << "4 ^ 3 = " << pow43 << std::endl;

    if (pow43 != 64) { --rtn; }

    return rtn;
}
