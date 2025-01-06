#include <cstdint>
#include <iostream>
#include "morph/flags.h"

// You need an enumerated class to use morph::flags.
enum class myflags : uint32_t
{
    one,
    two,
    three,
    four
};

int main()
{
    int rtn = 0;

    morph::flags<myflags> fl;
    fl.set (myflags::one);
    if (fl.test(myflags::one)) {
        std::cout << "flags: one is set" << std::endl;
    } else {
        std::cout << "flags: one is not set" << std::endl;
    }
    std::cout << "After set one, fl bits: " << fl.get() << std::endl;
    std::cout << "Bit count: " << fl.count() << std::endl;
    if (fl.get() != uint32_t{1}) { --rtn; }

    fl.reset (myflags::one);
    if (fl.test(myflags::one)) {
        std::cout << "flags: one is set" << std::endl;
    } else {
        std::cout << "flags: one is not set" << std::endl;
    }
    std::cout << "After reset one, fl bits: " << fl.get() << std::endl;
    std::cout << "Bit count: " << fl.count() << std::endl;
    if (fl.get() != uint32_t{0}) { --rtn; }

    fl.reset (myflags::one);
    if (fl.test(myflags::one)) {
        std::cout << "flags: one is set" << std::endl;
    } else {
        std::cout << "flags: one is not set" << std::endl;
    }
    std::cout << "After another reset one, fl bits: " << fl.get() << std::endl;
    std::cout << "Bit count: " << fl.count() << std::endl;
    if (fl.get() != uint32_t{0}) { --rtn; }

    fl |= myflags::one;
    fl |= (myflags::two);
    if (fl.test(myflags::two)) {
        std::cout << "flags: two is set" << std::endl;
    } else {
        std::cout << "flags: two is not set" << std::endl;
    }
    std::cout << "After set two, fl bits: " << fl.get() << std::endl;
    std::cout << "Bit count: " << fl.count() << std::endl;
    if (fl.get() != uint32_t{3}) { --rtn; }

    fl |= (myflags::three);
    if (fl.test(myflags::three)) {
        std::cout << "flags: three is set" << std::endl;
    } else {
        std::cout << "flags: three is not set" << std::endl;
    }
    std::cout << "After set three, fl bits: " << fl.get() << std::endl;
    std::cout << "Bit count: " << fl.count() << std::endl;
    if (fl.get() != uint32_t{7}) { --rtn; }

    fl |= (myflags::four);
    if (fl.test(myflags::four)) {
        std::cout << "flags: four is set" << std::endl;
    } else {
        std::cout << "flags: four is not set" << std::endl;
    }
    std::cout << "After set four, fl bits: " << fl.get() << std::endl;
    std::cout << "Bit count: " << fl.count() << std::endl;
    if (fl.get() != uint32_t{15}) { --rtn; }

    // flip 2
    fl ^= myflags::two;
    if (fl.get() != uint32_t{13}) { --rtn; }

    if (fl.test (myflags::two) == true) { --rtn; }
    if (fl.test (myflags::one) == false) { --rtn; }

    fl.reset();
    std::cout << "After reset, fl bits: " << fl.get() << std::endl;
    std::cout << "Bit count: " << fl.count() << std::endl;
    if (fl.get() != uint32_t{0}) { --rtn; }

    morph::flags<myflags> fl2;
    fl2.set (myflags::three);
    fl2.set (myflags::one);
    std::cout << "fl2 = " << fl2.get() << std::endl;

    morph::flags fl3 = (fl2 | myflags::four);
    fl3 |= myflags::two;
    if (fl3.get() != uint32_t{15}) { --rtn; }


    morph::flags fl4 = myflags::four & fl3;
    std::cout << "fl4.get() = " << fl4.get() << " cf " << (8 & fl3.get()) << std::endl;
    if (fl4.get() != uint32_t{8}) { --rtn; }

    if (morph::flags<myflags>(myflags::one).get() != 1) { --rtn; }
    if (morph::flags<myflags>(myflags::two).get() != 2) { --rtn; }
    if (morph::flags<myflags>(myflags::three).get() != 4) { --rtn; }
    if (morph::flags<myflags>(myflags::four).get() != 8) { --rtn; }
    std::cout << "myflags::one : " << morph::flags<myflags>(myflags::one).get() << std::endl;
    std::cout << "myflags::two : " << morph::flags<myflags>(myflags::two).get() << std::endl;
    std::cout << "myflags::three : " << morph::flags<myflags>(myflags::three).get() << std::endl;
    std::cout << "myflags::four : " << morph::flags<myflags>(myflags::four).get() << std::endl;

    std::cout << (rtn ? "Failed\n" : "Success\n");
    return rtn;
}
