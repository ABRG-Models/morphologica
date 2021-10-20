#pragma once

#include <vector>
#include <deque>
#include <morph/Vector.h>

// A retinotectal axon branch class. Holds current and historical positions, a preferred
// termination zone, and the algorithm for computing the next position. Could derive
// from an 'agent' base class.
template<typename T>
struct branch
{
    // Compute the next position for this branch, using information from all other
    // branches and the parameters vector, m
    void compute_next (const std::vector<branch<T>>& branches, const morph::Vector<T, 4>& m)
    {
        // Current location is named b
        morph::Vector<T, 2> b = path.back();
        // Chemoaffinity is G
        morph::Vector<T, 2> G = this->tz - b; // or x_b0 - x_b, in paper
        // Competition, C, and Axon-axon interactions, I, computed during the same loop
        // over the other branches
        morph::Vector<T, 2> C = {0, 0};
        morph::Vector<T, 2> I = {0, 0};
        morph::Vector<T, 2> nullvec = {0, 0}; // null vector
        // Other branches are called k, making a set B_b, with a number of members that I call n_k
        T n_k = T{0};
        for (auto k : branches) {
            if (k.id == this->id) { continue; } // Don't interact with self
            // Paper deals with U_C(b,k) - the vector from branch b to branch k - and
            // sums these. However, that gives a competition term with a sign error. So
            // here, sum up the unit vectors kb.
            morph::Vector<T, 2> kb = b - k.path.back();
            T d = kb.length();
            T W = d <= this->two_r ? (T{1} - d/this->two_r) : T{0};
            T Q = k.EphA / this->EphA; // forward signalling (used predominantly in paper)
            //T Q = this->EphA / k.EphA; // reverse signalling
            //T Q = std::max(k.EphA / this->EphA, this->EphA / k.EphA); // bi-dir signalling
            kb.renormalize(); // as in paper, vector bk is a unit vector
            I += Q > this->s ? kb * W : nullvec;
            C += kb * W;
            if (W > T{0}) { n_k += T{1}; }
        }

        // Do the 1/|B_b| multiplication
        if (n_k > T{0}) {
            C = C/n_k;
            I = I/n_k;
        } // else C and I will be {0,0} still

        // Border effect. A force perpendicular to the boundary, falling off over the
        // distance r.
        morph::Vector<T, 2> B = {0, 0};
        // Test b, to see if it's near the border. Use winding number to see if it's
        // inside? Then, if outside, find out which edge it's nearest and apply that
        // force. Too complex. Instead, look at b's location. If x<0, then add component
        // to B[0]; if y<0 then add component to B[1], etc.
        if (b[0] < T{0}) {
            G = {0,0};
            I = {0,0};
            C = {0,0};
            B[0] = T{1};
        } else if (b[0] < r) {
            B[0] = T{1} * (T{1} - b[0]/r); // B[0] prop 1 - b/r
        } else if (b[0] > 1) {
            G = {0,0};
            I = {0,0};
            C = {0,0};
            B[0] = T{-1};
        } else if (b[0] > (1-r)) {
            B[0] = -(b[0] + r - T{1})/r; // B[0] prop (b+r-1)/r
        }

        if (b[1] < T{0}) {
            G = {0,0};
            I = {0,0};
            C = {0,0};
            B[1] = T{1};
        } else if (b[1] < r) {
            B[1] = T{1} - b[1]/r;
        } else if (b[1] > 1) {
            G = {0,0};
            I = {0,0};
            C = {0,0};
            B[1] = T{-1};
        } else if (b[1] > (1-r)) {
            B[1] = -(b[1] + r - T{1})/r; // B[1] prop (b+r-1)/r
        }

        // Paper equation 1
        b += (G * m[0] + C * m[1] + I * m[2] + B * m[3]);

        this->next = b;
    }
    // The location and all previous locations of this branch.
    std::deque<morph::Vector<T, 2>> path;
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
    // Distance parameter r is used as 2r
    static constexpr T two_r = T{0.1};
    static constexpr T r = T{0.05};
    // Signalling ratio parameter
    static constexpr T s = T{1.1};
};
