#include <iostream>
#include <chrono>
#include <list>
#define OMIT_BEZCOORD_FROM_HEX
#include <morph/Hex.h>

int main()
{
    // Make a list of Hexes
    std::list<morph::Hex> hexen;

    static constexpr unsigned int n_hex = 100000;

    // Place a row of 100000 hexes in the list
    for (unsigned int i = 0; i < n_hex; ++i) {
        hexen.emplace (hexen.end(), morph::Hex(i, 0.1f, static_cast<int>(i), 0));
    }

    using namespace std::chrono;
    using sc = std::chrono::steady_clock;
    auto t0 = sc::now();

    auto hi = hexen.begin();
    while (hi != hexen.end()) {
        if (hi->vi % 2 == 0) {
            hi->disconnectNeighbours();
            hi = hexen.erase (hi);
        } else {
            ++hi;
        }
    }
    auto t1 = sc::now();

    unsigned int n_deleted = n_hex - hexen.size();
    sc::duration t_d = t1 - t0;
    std::cout << "Took " << duration_cast<microseconds>(t_d).count() << " us to delete " << n_deleted << " hexes from the std::list\n";


    std::cout << "Done\n";
}
