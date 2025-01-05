#include <cstdint>
#include <iostream>
#include "morph/flags.h"

enum class myflags : uint32_t
{
    one,
    two
};

int main()
{
    morph::flags<myflags> fl;
    fl.set (myflags::one);
    //fl.reset (myflags::one);
    fl |= myflags::two;
    fl ^= myflags::two;

    if (fl.test(myflags::one)) {
        std::cout << "flags: one is set" << std::endl;
    } else {
        std::cout << "flags: one is not set" << std::endl;
    }
    if (fl.test(myflags::two)) {
        std::cout << "flags: two is set" << std::endl;
    } else {
        std::cout << "flags: two is not set" << std::endl;
    }
    std::cout << "fl.bits = " << fl.get() << std::endl;
    return 0;
}
