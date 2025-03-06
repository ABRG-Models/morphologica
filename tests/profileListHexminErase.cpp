#include <iostream>
#include <chrono>
#include <list>
#include <cmath>

// Flags
#define HEX_HAS_NE                0x1
#define HEX_HAS_NW                0x8

class Hexmin
{
public:
    // Member attributes
    unsigned int vi;
    unsigned int di = 0;
    float x = 0.0f;
    float y = 0.0f;
    float r = 0.0f;
    float phi = 0.0f;
    float z = 0.0f;
    float d = 1.0f;
    int ri = 0;
    int gi = 0;
    int bi = 0;
    float distToBoundary = -1.0f;
    std::list<Hexmin>::iterator ne;
    std::list<Hexmin>::iterator nw;

private:
    unsigned int flags = 0x0;

public:
    Hexmin (const unsigned int& idx, const float& d_, const int& r_, const int& g_)
    {
        this->vi = idx;
        this->d = d_;
        this->ri = r_;
        this->gi = g_;
        this->computeLocation();
    }
    void computeLocation()
    {
        // Compute Cartesian location
        this->x = this->d * this->ri + (this->d / 2.0f) * this->gi - (this->d / 2.0f) * this->bi;
        float v = this->getV();
        this->y = v * this->gi + v * this->bi;
        // And location in the Polar coordinate system
        this->r = std::sqrt (x*x + y*y);
        this->phi = std::atan2 (y, x);
    }
    //! The vertical distance between hex centres on adjacent rows.
    float getV() const { return (this->d * 0.866025403f); }
    //! Set that \a it is the Neighbour to the East
    void set_ne (std::list<Hexmin>::iterator it) { this->ne = it; this->flags |= HEX_HAS_NE; }
    void set_nw (std::list<Hexmin>::iterator it) { this->nw = it; this->flags |= HEX_HAS_NW; }

    //! Return true if this Hex has a Neighbour to the East
    bool has_ne() const { return ((this->flags & HEX_HAS_NE) == HEX_HAS_NE); }
    bool has_nw() const { return ((this->flags & HEX_HAS_NW) == HEX_HAS_NW); }
    //! Set flags to say that this Hex has NO neighbour to East
    void unset_ne() { this->flags ^= HEX_HAS_NE; }
    void unset_nw() { this->flags ^= HEX_HAS_NW; }

    //! Un-set the pointers on all my neighbours so that THEY no longer point to ME.
    void disconnectNeighbours()
    {
        if (this->has_ne()) { if (this->ne->has_nw()) { this->ne->unset_nw(); } }
        if (this->has_nw()) { if (this->nw->has_ne()) { this->nw->unset_ne(); } }
    }
};

int main()
{
    // Make a list of Hexes
    std::list<Hexmin> hexen;
    static constexpr unsigned int n_hex = 100000; // 100000 makes about a 10MB list
    std::list<Hexmin>::iterator neighbour; // Iterator to a neighbour

    for (unsigned int i = 0; i < n_hex; ++i) {
        Hexmin h(i, 0.1f, static_cast<int>(i), 0);
        // Neighbour setup to ensure disconnect neighbours has work to do
        if (i == 6) {
            neighbour = hexen.end();
            --neighbour;
            std::cout << " neighbour index: " << neighbour->vi << " self at i==6: " << h.vi << std::endl;
        } else if (i > 6 && i < n_hex - 7) {
            h.set_nw(neighbour);
            hexen.emplace(hexen.end(), h);
            std::list<Hexmin>::iterator inserted = hexen.end();
            --inserted;
            neighbour->set_ne(inserted); // The reciprocal neighbour relationship
            ++neighbour;
        } else {
            hexen.emplace(hexen.end(), h);
        }
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
}
