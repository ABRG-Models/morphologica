#include <limits>
#include <map>
#include <morph/vec.h>
#include <morph/vvec.h>
#include <morph/geometry.h>

int main()
{
    // Want to say "if it's within 1% of the characteristic vertex-vertex distance, then it's the same.
    const float z_thresh = 0.5f / 10.0f; // std::numeric_limits<float>::epsilon();

    // Special comparison function to order vertices in a Geodesic polyhedron
    auto _vtx_cmp = [z_thresh](morph::vec<float, 3> a, morph::vec<float, 3> b)
    {
        std::cout << "_vtx_cmp(): Is " << a << " < " << b << " ?\n";
        // Compare first by vertex's z location
        bool is_less_than = false;

        std::cout << "std::abs(a[2] - b[2]) = " << std::abs(a[2] - b[2])
                  << " and epsilon: " << std::numeric_limits<float>::epsilon() << std::endl;
        if (std::abs(a[2] - b[2]) < z_thresh) {
            // and then by rotational angle in the x-y plane
            float angle_a = std::atan2 (a[1], a[0]);
            float angle_b = std::atan2 (b[1], b[0]);
            std::cout << "  angle_a: " << angle_a << " and angle_b: " << angle_b << std::endl;
            if (angle_a < angle_b) {
                std::cout << "  a is < b on account of angle\n";
                is_less_than = true;
            } else {
                std::cout << "  a not < b as angle_a !< angle_b\n";
            }
        } else if (a[2] < b[2]) {
            std::cout << "  a < b on account of z component\n";
            is_less_than = true;
        } else {
            std::cout << "  a not < b as a[2] > b[2]\n";
        }
        return is_less_than;
    };

    // Make a keyed container for the vertices, as we will need to reorder them.
    std::map<morph::vec<float, 3>, int, decltype(_vtx_cmp)> vertices_map(_vtx_cmp);

    // Some vertices defined in a vvec
    morph::vvec<morph::vec<float, 3>> vertices(12, {0.0f, 0.0f, 0.0f});
    morph::vvec<morph::vec<int, 3>> icofaces(20, {0, 0, 0});
    morph::geometry::icosahedron (vertices, icofaces);

    // First, copy the initial vertices (from the icosahedron) into the map
    int i = 0;
    for (auto v : vertices) {
        std::cout << "Adding " << i << " to vertices map for vector " << v << std::endl;
        vertices_map[v] = i++;
    }
    std::cout << "vertices_map size: " << vertices_map.size() << std::endl; // size is 11 anyhow

    auto iter = vertices_map.begin();
    while (iter != vertices_map.end()) {
        std::cout << "Vector map index is " << iter->first << " and integer map value is " << iter->second << std::endl;
        ++iter;
    }

    return 0;
}
