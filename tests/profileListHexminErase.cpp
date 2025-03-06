#include <iostream>
#include <chrono>
#include <list>
#include <cmath>

// Flags
#define HEX_HAS_NE                0x1
#define HEX_HAS_NNE               0x2
#define HEX_HAS_NNW               0x4
#define HEX_HAS_NW                0x8
#define HEX_HAS_NSW              0x10
#define HEX_HAS_NSE              0x20
#define HEX_HAS_NEIGHB_ALL       0x3f

class Hexmin
{
public:
    // Member attributes consume 100 bytes of data (on a 64 bit machine):
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
    std::list<Hexmin>::iterator nne;
    std::list<Hexmin>::iterator nnw;
    std::list<Hexmin>::iterator nw;
    std::list<Hexmin>::iterator nsw;
    std::list<Hexmin>::iterator nse;
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
    void set_ne (std::list<Hexmin>::iterator it)
    { this->ne = it; this->flags |= HEX_HAS_NE; }
    void set_nne (std::list<Hexmin>::iterator it)
    { this->nne = it; this->flags |= HEX_HAS_NNE; }
    void set_nnw (std::list<Hexmin>::iterator it)
    { this->nnw = it; this->flags |= HEX_HAS_NNW; }
    void set_nw (std::list<Hexmin>::iterator it)
    { this->nw = it; this->flags |= HEX_HAS_NW; }
    void set_nsw (std::list<Hexmin>::iterator it)
    { this->nsw = it; this->flags |= HEX_HAS_NSW; }
    void set_nse (std::list<Hexmin>::iterator it)
    { this->nse = it; this->flags |= HEX_HAS_NSE; }
    //! Return true if this Hex has a Neighbour to the East
    bool has_ne() const { return ((this->flags & HEX_HAS_NE) == HEX_HAS_NE); }
    bool has_nne() const { return ((this->flags & HEX_HAS_NNE) == HEX_HAS_NNE); }
    bool has_nnw() const { return ((this->flags & HEX_HAS_NNW) == HEX_HAS_NNW); }
    bool has_nw() const { return ((this->flags & HEX_HAS_NW) == HEX_HAS_NW); }
    bool has_nsw() const { return ((this->flags & HEX_HAS_NSW) == HEX_HAS_NSW); }
    bool has_nse() const { return ((this->flags & HEX_HAS_NSE) == HEX_HAS_NSE); }
    //! Set flags to say that this Hex has NO neighbour to East
    void unset_ne() { this->flags ^= HEX_HAS_NE; }
    void unset_nne() { this->flags ^= HEX_HAS_NNE; }
    void unset_nnw() { this->flags ^= HEX_HAS_NNW; }
    void unset_nw() { this->flags ^= HEX_HAS_NW; }
    void unset_nsw() { this->flags ^= HEX_HAS_NSW; }
    void unset_nse() { this->flags ^= HEX_HAS_NSE; }
    //! Un-set the pointers on all my neighbours so that THEY no longer point to ME.
    void disconnectNeighbours()
    {
        if (this->has_ne()) { if (this->ne->has_nw()) { this->ne->unset_nw(); } }
        if (this->has_nne()) { if (this->nne->has_nsw()) { this->nne->unset_nsw(); } }
        if (this->has_nnw()) { if (this->nnw->has_nse()) { this->nnw->unset_nse(); } }
        if (this->has_nw()) { if (this->nw->has_ne()) { this->nw->unset_ne(); } }
        if (this->has_nsw()) { if (this->nsw->has_nne()) { this->nsw->unset_nne(); } }
        if (this->has_nse()) { if (this->nse->has_nnw()) { this->nse->unset_nnw(); } }
    }
};

int main()
{
    // Make a list of Hexes
    std::list<Hexmin> hexen;
    static constexpr unsigned int n_hex = 100000; // 100000 makes about a 10MB list
    std::list<Hexmin>::iterator m1;
    std::list<Hexmin>::iterator m2;
    std::list<Hexmin>::iterator m3;
    std::list<Hexmin>::iterator m4;
    std::list<Hexmin>::iterator m5;
    std::list<Hexmin>::iterator m6;

    for (unsigned int i = 0; i < n_hex; ++i) {
        Hexmin h (i, 0.1f, static_cast<int>(i), 0);
        // Neighbour setup to ensure disconnect neighbours has work to do
        if (i == 5) {
            m6 = hexen.end();
            m1 = --m6;
            m2 = --m6;
            m3 = --m6;
            m4 = --m6;
            m5 = --m6;
        } else if (i > 5 && i < (n_hex-6)) {
            ++m1; ++m2; ++m3; ++m4; ++m5; ++m6;
            h.set_ne (m1);
            h.set_nne (m2);
            h.set_nnw (m3);
            h.set_nw (m4);
            h.set_nsw (m5);
            h.set_nse (m6);
        }
        hexen.emplace (hexen.end(), h);
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
