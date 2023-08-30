/*
 * Because morph::vec acts funny as a key in std::map, here's an example showing that std::array
 * behaves nicely. For an example of how to use morph::vec as a key to an std::map, see
 * testvec_asmapkey.cpp.
 */

#include <array>
#include <map>
#include <string>
#include <iostream>

int main()
{
    int rtn = 0;

    // Some keys
    std::array<int, 2> k1 = {1,2};
    std::array<int, 2> k2 = {1,3};
    std::array<int, 2> k3 = {2,3};
    std::array<int, 2> k4 = {2,4};

    if (k1 < k2) {
        std::cout << "k1 < k2 as expected\n";
    } else {
        std::cout << "we have a problem with the lessthan comparison operator\n";
    }
    if (k1 < k3) {
        std::cout << "k1 < k3 as expected, but by accident\n";
    }

    // No need to specify a less-than comparator with std::array as the key:
    std::map<std::array<int, 2>, std::string> themap;

    themap[k1] = std::string("value1");
    themap[k2] = std::string("value2");
    themap[k3] = std::string("value3");
    themap[k4] = std::string("value4");

    std::cout << "Map size is " << themap.size() << std::endl;
    if (themap.size() != 4) { --rtn; }


    std::cout << "map with k1 returns " << themap[k1] << " (should be \"value1\")" << std::endl;
    if (themap[k1] != std::string("value1")) { --rtn; }
    std::cout << "map with k2 returns " << themap[k2] << " (should be \"value2\")" << std::endl;
    if (themap[k2] != std::string("value2")) { --rtn; }
    std::cout << "map with k3 returns " << themap[k3] << " (should be \"value3\")" << std::endl;
    if (themap[k3] != std::string("value3")) { --rtn; }
    std::cout << "map with k4 returns " << themap[k4] << " (should be \"value4\")" << std::endl;
    if (themap[k4] != std::string("value4")) { --rtn; }

    return rtn;
}
