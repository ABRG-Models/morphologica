// Can you use a morph::vec as a key to an std::map?
//
// Answer is yes, but you have to tell the map whcih comparison operator you're going to
// use. Similar to the issue of storing morph::vecs in an std::set.
#include <morph/vec.h>
#include <map>
#include <string>
#include <iostream>

int main()
{
    namespace m = morph;

    int rtn = 0;

    // Some keys
    m::vec<int, 2> k1 = {1,2};
    m::vec<int, 2> k2 = {1,3};
    m::vec<int, 2> k3 = {2,3};
    m::vec<int, 2> k4 = {2,4};

    if (k1 < k2) {
        std::cout << "k1 < k2 as expected\n";
    } else {
        std::cout << "we have a problem with the lessthan comparison operator\n";
    }
    if (k1 < k3) {
        std::cout << "k1 < k3 as expected, but by accident\n";
    }

    // To make the map work, we have to tell it to use lexical_lessthan:
    auto _cmp = [](m::vec<int,2> a, m::vec<int,2> b){return a.lexical_lessthan(b);};
    std::map<m::vec<int, 2>, std::string, decltype(_cmp)> themap(_cmp);

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
