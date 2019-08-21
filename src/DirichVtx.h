#ifndef _DIRICHVTX_H_
#define _DIRICHVTX_H_

#include "MathConst.h"
#include <utility>
using std::pair;

namespace morph {

    /*!
     * Dirichlet domain vertex class.
     *
     * This is a class that can be used with an std::set. It's a
     * container for a pair of Flts (representing a pair of
     * coordinates in 2D cartesian space) and a comparison operation
     * which ensures that coordinate pairs which are similar are
     * treated as being equal.
     *
     * It has developed into a class which holds a vertex, and its
     * vertex neighbour so that a group of objects of this class type
     * can define a single domain for Dirichlet-ness analysis (after
     * Honda 1983).
     */
    template <class Flt>
    class DirichVtx {
    public:
        //! The coordinate data for the main vertex represented.
        pair<Flt, Flt> v;

        /*! The location of the neighbouring vertex - necessary for
         * computing a Dirichlet-ness metric. Intended to be
         * populated after a set of vertices has been created, in a
         * "second pass" of a program.
         */
        pair<Flt, Flt> vn;

        /*!
         * The value of the domain for which this vertex is a
         * vertex. This is essentially the domain's identity.
         */
        Flt f;

        /*!
         * A distance threshold that makes sense within the problem -
         * probably some fraction of the hex-to-hex distance, d is
         * correct, because I'm using this to find hex vertices, and
         * they are spaced exactly one side length of a hex
         * apart. Basing threshold upon this metric means that we
         * don't have to worry about the difficulties of comparing
         * floating point numbers of different sizes (where, because
         * the mantissa is of variable number of bits, the threshold
         * that makes sense in a comparison is a function of the size
         * of the number represented in the Flt).
         *
         * Constructors taking the value of d (the hex-to-hex
         * distance) will correctly set this member.
         */
        Flt threshold = +0.00001f;

        /*!
         * The OTHER, neighbouring domains that this vertex
         * divides. Always 3 domains are divided by one vertex on an
         * hexagonal grid. The first domain value is the identifier
         * for the Dirichlet domain for which this vertex is a vertex,
         * which is stored in @f. The other 2 domains are stored
         * here. If one of the domains is "outside" the boundary set
         * -1.0f.
         */
        pair<Flt, Flt> neighb;

        //! Two of the *Domain* neighbours of the *vertex* neighbour.
        pair<Flt, Flt> neighbn;

        //! Constructors
        //@{

        //! Construct with passed in float pair, using default threshold
        DirichVtx (const pair<float, Flt>& p) : v(p) {}

        /*!
         * Construct with passed in Flt pair, set the threshold on the
         * basis of being passed in the Hex to Hex distance d.
         */
        DirichVtx (const pair<Flt, Flt>& p, const Flt& d)
            : v(p) {
            // This is half of the shortest possible distance in the y
            // direction between two adjacent Hex vertices.
            this->threshold = d/(4.0f*morph::SQRT_OF_3_F);
        }

        /*!
         * Construct with passed in Flt pair; set the threshold on
         * the basis of being passed in the Hex to Hex distance d and
         * also set the value of the vertex to be @id
         */
        DirichVtx (const pair<Flt, Flt>& p, const Flt& d, const Flt& id)
            : v(p), f(id) {
            this->threshold = d/(4.0f*morph::SQRT_OF_3_F);
        }

        /*!
         * Construct with passed in Flt pair; set the threshold on the
         * basis of being passed in the Hex to Hex distance d, set the
         * value of the vertex to be @id and finally, set this->neighb
         * (with @oth).
         */
        DirichVtx (const pair<Flt, Flt>& p, const Flt& d, const Flt& id, const pair<Flt, Flt>& oth)
            : v(p), f(id), neighb(oth) {
            this->threshold = d/(4.0f*morph::SQRT_OF_3_F);
        }
        //@}

        //! Comparison operation. Note: Ignores this->vn!
        bool operator< (const DirichVtx<Flt>& rhs) const {
            // Compare value:
            if (this->f < rhs.f) {
                return true;
            }
            if (this->f > rhs.f) {
                return false;
            }
            // Values are equal, so compare coordinates:
            if (rhs.v.first - this->v.first > this->threshold) {
                return true;
            }
            if (this->v.first - rhs.v.first > this->threshold) {
                return false;
            }
            // Get here, then rhs.v.first and this->v.first are "equal"
            if (rhs.v.second - this->v.second > this->threshold) {
                return true;
            }
            if (rhs.v.second < this->v.second) {
                return false;
            }
            // Value and vertex coord equal; last check is on the vertex neighbour coord:
            if (rhs.vn.first - this->vn.first > this->threshold) {
                return true;
            }
            if (this->vn.first - rhs.vn.first > this->threshold) {
                return false;
            }
            // Get here, then rhs.v.first and this->v.first are "equal"
            if (rhs.vn.second - this->vn.second > this->threshold) {
                return true;
            }
            // Not necessary:
            // if (rhs.vn.second < this->vn.second) { return false; }
            return false;
        }
    };

} // namespace morph

#endif // DIRICHVTX
