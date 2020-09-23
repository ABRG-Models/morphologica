#include <iostream>
#include "morph/Random.h"

int main()
{
    morph::RandString rs;
    std::cout << rs << std::endl;

    morph::RandString rs1(4);
    std::cout << rs1.get() << std::endl;

    morph::RandString rs2(20, morph::CharGroup::Alpha);
    std::cout << rs2.get() << std::endl;

    morph::RandString rs3(32, morph::CharGroup::Decimal);
    std::cout << rs3 << std::endl;

    morph::RandString rs4(32, morph::CharGroup::BinaryTF);
    std::cout << rs4 << std::endl;

    morph::RandString rs5(32, morph::CharGroup::Binary);
    std::cout << rs5 << std::endl;

    morph::RandString rs6(20, morph::CharGroup::AlphaNumericUpperCase);
    std::cout << rs6 << std::endl;

    morph::RandString rs7(20, morph::CharGroup::AlphaNumericLowerCase);
    std::cout << rs7 << std::endl;
    rs7.setCharGroup (morph::CharGroup::AlphaUpperCase);
    std::cout << rs7 << std::endl;

    morph::RandString rs8(20, morph::CharGroup::AlphaLowerCase);
    std::cout << rs8 << std::endl;

    morph::RandString rs9(20, morph::CharGroup::HexUpperCase);
    std::cout << rs9 << std::endl;

    morph::RandString rs10(20, morph::CharGroup::HexLowerCase);
    std::cout << rs10.get(50) << std::endl;

    morph::RandString rs11(20, morph::CharGroup::AlphaNumeric);
    std::cout << rs11 << std::endl;

    return 0;
}
