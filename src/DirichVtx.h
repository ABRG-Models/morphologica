#ifndef _DIRICHVTX_H_
#define _DIRICHVTX_H_

#include "MathConst.h"
#include <limits>
using std::numeric_limits;
#include <utility>
using std::pair;
#include <list>
using std::list;
#include <vector>
using std::vector;
#include "HexGrid.h"
using morph::Hex;

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

        /*!
         * The series of points that make up the edge between the
         * this vertex (v) and its vertex neighbour (vn). Should order by size.
         */
        set<pair<Flt, Flt> > pathto_neighbour;

        /*!
         * Series of points that make the edge between thsi vertex and
         * the next one in the list.
         */
        set<pair<Flt, Flt> > pathto_next;

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

        /*!
         * An iterator into the accompanying list of Hexes. Intended
         * to be an iterator into HexGrid::hexen. This points to the
         * hex containing the vertex that is this
         * DirichVtx. Important so that from one DirichVtx, we can
         * find our way along and edge to the next vertex.
         */
        list<Hex>::iterator hi;

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

        /*!
         * Construct with passed in Flt pair; set the threshold on the
         * basis of being passed in the Hex to Hex distance d, set the
         * value of the vertex to be @id, set this->neighb (with @oth)
         * and finally, set the list<Hex> iterator @hex.
         */
        DirichVtx (const pair<Flt, Flt>& p, const Flt& d, const Flt& id,
                   const pair<Flt, Flt>& oth, const list<Hex>::iterator hex)
            : v(p), f(id), neighb(oth), hi(hex) {
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

        //! Parameters for line
        Flt m;
        Flt c;
        pair<Flt, Flt> P_i;

        /*!
         * Compute the equation for the line that is drawn towards the
         * putative centre of the Dirichlet domain. Needs to be passed
         * in the coordinates of the vertex that is clockwise in the
         * domain. As per Honda 1983, p 196, @prev_vertex is A_{i-1},
         * @next_vertex is A_{i+1}. A_i and B_i are stored here as
         * this->v (A_i) and this->vn (B_i). With this information,
         * it's possible to compute the gradient of the line that
         * emerges from A_i and heads towards the Dirichlet domain
         * centre.
         */
        void compute_line_to_centre (const DirichVtx<Flt>& prev_vertex,
                                     const DirichVtx<Flt>& next_vertex) {

            /*
             * 1. Find angle A_{i+1} A_i P_i from A_i, B_i and A_{i-1}
             */
            pair<Flt, Flt> Aim1, Aip1, Bi, Ai;
            Aim1 = prev_vertex.v;
            Aip1 = next_vertex.v;
            Bi = vn;
            Ai = v;  // Get rid of these copies of the pair<Flt,Flt>s in the final code

            // Triangle side lengths (or rather squares of lengths)
            // x is side from A_i-1 to Bi
            // y is side from Bi to Ai
            // z is side from Ai to A_i-1
            Flt xsq = (Bi.first - Aim1.first)*(Bi.first - Aim1.first)
                + (Bi.second - Aim1.second)*(Bi.second - Aim1.second);

            Flt ysq = (Bi.first - Ai.first)*(Bi.first - Ai.first)
                + (Bi.second - Ai.second)*(Bi.second - Ai.second);

            Flt zsq = (Ai.first - Aim1.first)*(Ai.first - Aim1.first)
                + (Ai.second - Aim1.second)*(Ai.second - Aim1.second);

            // Angle X is the angle B_i A_i A_i-1
            //
            // X = arccos( (1/2yz) * (ysq+zsq-xsq) )
            Flt X = arccos ( (0.5/(sqrt(ysq)*sqrt(zsq))) * (ysq+zsq-xsq) );

            /*
             * 2. Find one point P_i.
             *
             * Choose the distance from A_i to P_i to be equal to the
             * distance from A_i+1 to A_i.  In this case, distance
             * from A_i+1 to P_i is x, where:
             */

            Flt y = sqrt ((Ai.first - Aip1.first)*(Ai.first - Aip1.first)
                          + (Ai.second - Aip1.second)*(Ai.second - Aip1.second));
            // NB: zsq = ysq;

            Flt Y = 0.5*(180-X);
            // NB Flt Z = Y;

            Flt x = sin (X) * (y/sin(Y));

            // With x, Y, Z, A_i and A_i+1, can now find P_i coordinates

            // Find angle between line joining A_i+1 and A_i and vertical.
            Flt theta = arctan2 (Ai.first-Aip1.first, Ai.second-Aip1.second);

            // The angle phi is
            Flt phi = theta+Y-90;

            // The changes in x/y from point A_i+1 are:
            Flt dx = x * cos(phi);
            Flt dy = x * sin(phi);

            this->P_i.first = Aip1.first + dx;
            this->P_i.second = Aip1.second + dy;

            /*
             * 3. Use A_i and P_i to compute gradient/offset of the
             * line equation that passes through point A_i. Store in
             * this->m and this->c (or whatever is suitable).
             */
            if (this->P_i.first == Ai.first) {
                this->m = this->P_i.second < Ai.second ? numeric_limits<Flt>::lowest() : numeric_limits<Flt>::max();
                this->c = numeric_limits<Flt>::max();
                // And if c is max(), then it's a vertical line from
                // A_i through P_i which means that P_i.first ==
                // A_i.first and P_i.second can be anything.

            } else {
                this->m = (this->P_i.second - Ai.second) / (this->P_i.first - Ai.first);
                this->c = Ai.second - this->m * Ai.first;
            }
        }
    };

} // namespace morph

#endif // DIRICHVTX
