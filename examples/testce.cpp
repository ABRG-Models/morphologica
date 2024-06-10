#include <iostream>
#include <morph/geometry.h>

int main()
{
    // Note that we force the compile time evaluation of geometry_ce::icosahedron by returning a constexpr object
    constexpr morph::geometry_ce::polyhedron<double, 12, 20> ico = morph::geometry_ce::icosahedron<double>();
    std::cout << "constexpr 1st vertices:\n" << static_cast<morph::vec<double, 3>>(ico.vertices[0]) << std::endl;

    constexpr
    auto geo = morph::geometry_ce::make_icosahedral_geodesic<double, 3>();
    std::cout << "constexpr geo.vertices: " << static_cast<morph::vec<double, 3>>(geo.poly.vertices[0]).str() << std::endl;
    return 0;
}
