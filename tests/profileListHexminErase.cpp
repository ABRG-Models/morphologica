#include <iostream>
#include <chrono>
#include <list>

#define HEX_HAS_NE 0x1
#define HEX_HAS_NW 0x2

struct Hex
{
    Hex(const unsigned int& idx, const int& r_) : vi(idx), ri(r_) {}
    void set_ne (std::list<Hex>::iterator it) { this->ne = it; this->flags |= HEX_HAS_NE; } // Set eastern neighbour
    void set_nw (std::list<Hex>::iterator it) { this->nw = it; this->flags |= HEX_HAS_NW; }
    bool has_ne() const { return ((this->flags & HEX_HAS_NE) == HEX_HAS_NE); } // test for eastern neighbour
    bool has_nw() const { return ((this->flags & HEX_HAS_NW) == HEX_HAS_NW); }
    void unset_ne() { this->flags ^= HEX_HAS_NE; } // unset eastern neighbour
    void unset_nw() { this->flags ^= HEX_HAS_NW; }
    void disconnectNeighbours() // Unset all neighbours so they no longer point to ME.
    {
        if (this->has_ne()) { if (this->ne->has_nw()) { this->ne->unset_nw(); } }
        if (this->has_nw()) { if (this->nw->has_ne()) { this->nw->unset_ne(); } }
    }
    unsigned int vi; // an 'index'
    int ri = 0;      // a position
    std::list<Hex>::iterator ne; // iterator to the neighbour to the east
    std::list<Hex>::iterator nw; // iterator to the western neighbour
private:
    unsigned int flags = 0x0;
};

int main()
{
    static constexpr unsigned int n_hex = 100000;
    // Make a list of Hexes initialized with the first hex (index 0u, position (ri) 0)
    std::list<Hex> hexen = { Hex(0u, 0) };
    std::list<Hex>::iterator neighbour = hexen.begin(); // Iterator used to point to the 'west' neighbour
    for (unsigned int i = 1u; i < n_hex; ++i) {
        Hex h(i, static_cast<int>(i));
        h.set_nw(neighbour); // Make one neighbour releationship before emplacing into hexen
        hexen.emplace(hexen.end(), h);
        std::list<Hex>::iterator inserted = hexen.end();
        neighbour->set_ne(--inserted); // Make the reciprocal neighbour relationship
        ++neighbour;
    }

    using namespace std::chrono;
    using sc = std::chrono::steady_clock;

    std::cout << "std::list<Hex> created. Now erase Hexes from list...\n";
    auto t0 = sc::now(); // timing the erase process
    auto hi = hexen.begin();
    while (hi != hexen.end()) {
        if (hi->vi % 2 == 0) {
            hi->disconnectNeighbours();
            hi = hexen.erase (hi);
        } else {
            ++hi;
        }
    }
    std::cout << "It took " << duration_cast<microseconds>(sc::duration{sc::now() - t0}).count()
              << " us to delete " << (n_hex - hexen.size()) << " hexes from the std::list\n";
}
