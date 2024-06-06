#include <iostream>
#include <morph/geometry.h>

int main()
{
    // Note that we force the compile time evaluation of geometry_ce::icosahedron by returning a constexpr object
    constexpr morph::geometry_ce::polyhedron<double, 12, 20> ico = morph::geometry_ce::icosahedron<double>();
    std::cout << "constexpr vertices:\n" << ico.vertices.str() << std::endl;
    return 0;
}
