#pragma once

#include <vector>
#include <morph/Vector.h>

// A retinotectal axon branch class. Holds current and historical positions, a preferred
// termination zone, and the algorithm for computing the next position.
template<typename T>
struct branch
{
    // Compute the next position for this branch, using information from all other
    // branches and the parameters vector, m
    void compute_next (const std::vector<branch<T>>& branches, const morph::Vector<T, 4>& m)
    {
        // Current location is named k
        morph::Vector<T, 2> k = path.back();
        // Chemoaffinity is G
        morph::Vector<T, 2> G = this->tz - k;
        // Competition, C, and Axon-axon interactions, I, computed during the same loop
        // over the other branches
        morph::Vector<T, 2> C = {0, 0};
        morph::Vector<T, 2> I = {0, 0};
        morph::Vector<T, 2> nullvec = {0, 0}; // null vector
        for (auto b : branches) {
            if (b.id == this->id) { continue; } // Don't interact with self
            morph::Vector<T, 2> bk = k - b.path.back();
            T d = bk.length();
            T W = d <= this->two_r ? (T{1} - d/this->two_r) : T{0};
            T Q = b.EphA / this->EphA; // forward signalling (used predominantly in paper)
            //T Q = this->EphA / b.EphA; // reverse signalling
            //T Q = std::max(b.EphA / this->EphA, this->EphA / b.EphA); // bi-dir signalling
            bk.renormalize();
            I += Q > this->s ? bk * W : nullvec;
            C += bk * W;
        }
        C.renormalize(); // achieves 1/|Bb|
        I.renormalize();

        // Border effect. A force perpendicular to the boundary, falling off over the
        // distance r.
        morph::Vector<T, 2> B = {0, 0};
        // Test k, to see if it's near the border. Use winding number to see if it's
        // inside? Then, if outside, find out which edge it's nearest and apply that
        // force. Too complex. Instead, look at k's location. If x<0, then add component
        // to B[0]; if y<0 then add component to B[1], etc.
        if (k[0] < T{0}) {
            G = {0,0};
            I = {0,0};
            C = {0,0};
            B[0] = T{1};
        } else if (k[0] < r) {
            B[0] = T{1} * (T{1} - k[0]/r); // B[0] prop 1 - k/r
        } else if (k[0] > 1) {
            G = {0,0};
            I = {0,0};
            C = {0,0};
            B[0] = T{-1};
        } else if (k[0] > (1-r)) {
            B[0] = -(k[0] + r - T{1})/r; // B[0] prop (k+r-1)/r
        }

        if (k[1] < T{0}) {
            G = {0,0};
            I = {0,0};
            C = {0,0};
            B[1] = T{1};
        } else if (k[1] < r) {
            B[1] = T{1} - k[1]/r;
        } else if (k[1] > 1) {
            G = {0,0};
            I = {0,0};
            C = {0,0};
            B[1] = T{-1};
        } else if (k[1] > (1-r)) {
            B[1] = -(k[1] + r - T{1})/r; // B[1] prop (k+r-1)/r
        }

#ifdef _DEBUG
        morph::Vector<T, 2> thestep = (G * m[0] + C * m[1] + I * m[2] + B * m[3]);
        if (thestep.length() > 0.1f) {
            std::cout << "Step size is " << thestep.length() << std::endl;
        }
        k += thestep; // * v where v=1
#else
        // Paper equation 1
        k += (G * m[0] + C * m[1] + I * m[2] + B * m[3]);
#endif
        this->next = k;
    }
    // The location and all previous locations of this branch.
    std::vector<morph::Vector<T, 2>> path;
    // Place the next computed location for path in 'next' so that while computing, we
    // don't modify the numbers we're working from. After looping through all branches,
    // add this to path.
    morph::Vector<T, 2> next;
    // Termination zone for this branch
    morph::Vector<T, 2> tz = {0, 0};
    // EphA expression for this branch
    T EphA = 0;
    // A sequence id
    int id = 0;
    // Parameter vector (hardcoded, see Table 2 in paper) where here, m[3] (the last
    // element) is the border effect magnitude.
    // The competition seems too strong.
    //static constexpr morph::Vector<T, 4> m = { T{0.02}, T{0.2}, T{0.15}, T{0.1} };
    // Distance parameter r is used as 2r
    static constexpr T two_r = T{0.1};
    static constexpr T r = T{0.05};
    // Signalling ratio parameter
    static constexpr T s = T{1.1};
};
