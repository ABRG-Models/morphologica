// Can you use a morph::vec as a key to an std::unordered_map?
//
#include <functional> // std::equal_to
#include <morph/vec.h>
#include <string>
#include <unordered_map>
#include <iostream>

// To make std::hash<morph::vec<int, 2>> work you have to define how to create the hash of
// morph::vec<int, 2>.
template<>
struct std::hash<morph::vec<int, 2>>
{
    std::size_t operator()(const morph::vec<int, 2>& v) const noexcept
    {
        std::size_t h1 = std::hash<int>{}(v[0]);
        std::size_t h2 = std::hash<int>{}(v[1]);
        return h1 ^ (h2 << 1);
    }
};

#if 0 // morph::vec<float, 3> examples
template<>
struct std::hash<morph::vec<float, 3>>
{
    std::size_t operator()(const morph::vec<float, 3>& v) const noexcept
    {
        // Note: Tiny precision differences make a different hash.
        std::size_t h1 = std::hash<float>{}(v[0]);
        std::size_t h2 = std::hash<float>{}(v[1]);
        std::size_t h3 = std::hash<float>{}(v[2]);
        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};

// Writing an equal_to operator()
template<>
struct std::equal_to<morph::vec<float, 3>>
{
    constexpr bool operator()(const morph::vec<float, 3>& lhs, const morph::vec<float, 3>& rhs) const
    {
        constexpr auto eps = std::numeric_limits<float>::epsilon();
        float d0 = std::abs(lhs[0] - rhs[0]);
        float d1 = std::abs(lhs[1] - rhs[1]);
        float d2 = std::abs(lhs[2] - rhs[2]);
        bool same = (d0 < eps) && (d1 < eps) && (d2 < eps);
        return same;
    }
};
#endif

int main()
{
    namespace m = morph;

    int rtn = 0;

    // Some keys
    m::vec<int, 2> k1 = {1,2};
    m::vec<int, 2> k2 = {1,3};
    m::vec<int, 2> k3 = {2,3};
    m::vec<int, 2> k4 = {2,4};
    
    // unordered_map uses std::hash. See above for making std::hash<m::vec<int, 2>> possible.
    std::unordered_map<m::vec<int, 2>, std::string> themap;

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
