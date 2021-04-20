/*
 * A net of locations, with information about what their ideal nearest neighbours are.
 */

#pragma once

#include <vector>
#include <morph/Vector.h>

template<typename T>
struct net
{
    //! Initialize a rectangular net of width w and height h. Note that this resizes p
    //! and arranges c, but does not fill p with positions.
    void init (size_t w, size_t h)
    {
        this->p.resize(w*h);
        for (size_t y = 0; y < h-1; ++y) {
            for (size_t x = 0; x < w; ++x) {
                // Connect down
                this->c.insert (morph::Vector<size_t, 2>({x+y*w, x+(y+1)*w}));
            }
        }
        for (size_t x = 0; x < w-1; ++x) {
            for (size_t y = 0; y < h; ++y) {
                // Connect right
                this->c.insert (morph::Vector<size_t, 2>({x+y*w, 1+x+y*w}));
            }
        }

    }
    //! Positions of the vertices of the net
    std::vector<morph::Vector<T, 3>> p;
    //! Connections of the net. The indices into p that are the ends of line segments
    std::set<morph::Vector<size_t, 2>> c;
};
