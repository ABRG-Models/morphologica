#include <iostream>
#include <morph/geometry.h>

int main()
{
    // Note that we force the compile time evaluation of geometry_ce::icosahedron by returning a constexpr object
    constexpr morph::geometry_ce::polyhedron<double, 12, 20> ico = morph::geometry_ce::icosahedron<double>();
    std::cout << "constexpr vertices:\n" << ico.vertices.str() << std::endl;

    constexpr auto geo = morph::geometry_ce::make_icosahedral_geodesic<double, 2>();
    std::cout << "geo.vertices: " << geo.poly.vertices.str() << std::endl;
    return 0;
}
