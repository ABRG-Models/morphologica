#ifndef _DIRICHVTX_H_
#define _DIRICHVTX_H_

#include "MathConst.h"
#include <cmath>
using std::sqrt;
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
     * This is a class that can be used with an std::set. It's a container for a pair of Flts
     * (representing a pair of coordinates in 2D cartesian space) and a comparison operation which
     * ensures that coordinate pairs which are similar are treated as being equal.
     *
     * It has developed into a class which holds a vertex, and its vertex neighbour so that a
     * group of objects of this class type can define a single domain for Dirichlet-ness analysis
     * (after Honda 1983).
     */
    template <class Flt>
    class DirichVtx {
    public:
        //! The coordinate data for the main vertex represented.
        pair<Flt, Flt> v;

        /*!
         * The series of points that make up the edge between the this vertex (v) and its vertex
         * neighbour (vn). Should order by size.
         */
        list<pair<Flt, Flt> > pathto_neighbour;

        /*!
         * Series of points that make the edge between thsi vertex and the next one in the list.
         */
        list<pair<Flt, Flt> > pathto_next;

        /*! The location of the neighbouring vertex - necessary for computing a Dirichlet-ness
         * metric. Intended to be populated after a set of vertices has been created, in a "second
         * pass" of a program.
         */
        pair<Flt, Flt> vn;

        /*!
         * The value of the domain for which this vertex is a vertex. This is essentially the
         * domain's identity.
         */
        Flt f;

        /*!
         * A distance threshold that makes sense within the problem - probably some fraction of
         * the hex-to-hex distance, d is correct, because I'm using this to find hex vertices, and
         * they are spaced exactly one side length of a hex apart. Basing threshold upon this
         * metric means that we don't have to worry about the difficulties of comparing floating
         * point numbers of different sizes (where, because the mantissa is of variable number of
         * bits, the threshold that makes sense in a comparison is a function of the size of the
         * number represented in the Flt).
         *
         * Constructors taking the value of d (the hex-to-hex distance) will correctly set this
         * member.
         */
        Flt threshold = +0.00001f;

        /*!
         * The OTHER, neighbouring domains that this vertex divides. Always 3 domains are divided
         * by one vertex on an hexagonal grid. The first domain value is the identifier for the
         * Dirichlet domain for which this vertex is a vertex, which is stored in @f. The other 2
         * domains are stored here. If one of the domains is "outside" the boundary set -1.0f.
         */
        pair<Flt, Flt> neighb;

        /*!
         * An iterator into the accompanying list of Hexes. Intended to be an iterator into
         * HexGrid::hexen. This points to the hex containing the vertex that is this
         * DirichVtx. Important so that from one DirichVtx, we can find our way along an edge to
         * the next vertex.
         */
        list<Hex>::iterator hi;

        //! Parameters for line between A_i and P_i, as described in Honda 1983.
        Flt m;
        Flt c;
        //! P_i is a point on the line. In this code, I project A_i+1 onto the line to find the
        //! actual point P_i.
        pair<Flt, Flt> P_i;

        //! For marking vertices in a list as finsihed with, rather than erasing from that list.
        bool closed = false;

        //! I mark vertices as being on the boundary, too. This matters for the algorithms in
        //! ShapeAnalysis.
        bool onBoundary = false;

        //! Constructors
        //@{

        //! Default constructor
        DirichVtx (void) {
            v.first = numeric_limits<Flt>::max();
            v.second = numeric_limits<Flt>::max();
        }

        //! Construct with passed in float pair, using default threshold
        DirichVtx (const pair<float, Flt>& p) : v(p) {}

        /*!
         * Construct with passed in Flt pair, set the threshold on the basis of being passed in
         * the Hex to Hex distance d.
         */
        DirichVtx (const pair<Flt, Flt>& p, const Flt& d)
            : v(p) {
            // This is half of the shortest possible distance in the y direction between two
            // adjacent Hex vertices.
            this->threshold = d/(4.0f*morph::SQRT_OF_3_F);
        }

        /*!
         * Construct with passed in Flt pair; set the threshold on the basis of being passed in
         * the Hex to Hex distance d and also set the value of the vertex to be @id
         */
        DirichVtx (const pair<Flt, Flt>& p, const Flt& d, const Flt& id)
            : v(p), f(id) {
            this->threshold = d/(4.0f*morph::SQRT_OF_3_F);
        }

        /*!
         * Construct with passed in Flt pair; set the threshold on the basis of being passed in
         * the Hex to Hex distance d, set the value of the vertex to be @id and finally, set
         * this->neighb (with @oth).
         */
        DirichVtx (const pair<Flt, Flt>& p, const Flt& d, const Flt& id, const pair<Flt, Flt>& oth)
            : v(p), f(id), neighb(oth) {
            this->threshold = d/(4.0f*morph::SQRT_OF_3_F);
        }

        /*!
         * Construct with passed in Flt pair; set the threshold on the basis of being passed in
         * the Hex to Hex distance d, set the value of the vertex to be @id, set this->neighb
         * (with @oth) and finally, set the list<Hex> iterator @hex.
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

        //! Compare @other with this->v. Return true if they're the same.
        bool compare (const pair<Flt, Flt>& other) {
            // Equality too strong a test. Use this->threshold Is distance from v to other smaller
            // than threshold? If so return true.
            Flt distance = sqrt ( (other.first-v.first)*(other.first-v.first)
                                  + (other.second-v.second)*(other.second-v.second) );
            if (distance < this->threshold) {
                return true;
            }
            return false;
        }

#if 0
        static pair<Flt, Flt> default_coord (void) {
            return pair<Flt, Flt>(numeric_limits<Flt>::max(), numeric_limits<Flt>::max());
        }
#endif

        //! Is this DirichVtx unset? If its this->v value is (max,max), then yes.
        bool unset (void) {
            if (this->v.first == numeric_limits<Flt>::max()
                && this->v.second == numeric_limits<Flt>::max()) {
                return true;
            }
            return false;
        }

        /*!
         * Compute line length of the line between coordinates @coord0 and @coord1
         */
        Flt line_length (const pair<Flt, Flt>& coord0, const pair<Flt, Flt>& coord1) const {
            Flt c01 = sqrt ((coord0.first - coord1.first) * (coord0.first - coord1.first)
                              + (coord0.second - coord1.second) * (coord0.second - coord1.second));
            return c01;
        }

        /*!
         * For the three coordinates c0, c1, c2, compute the angle at coordinate number @angleFor
         * (counting from 0).
         *
         * Could go into morph::tools or morph::maths or something.
         */
        Flt compute_angle (const pair<Flt, Flt>& c0,
                           const pair<Flt, Flt>& c1,
                           const pair<Flt, Flt>& c2,
                           const unsigned int angleFor) const {
            Flt angle = -1.0f;
            if (angleFor == 0) {
                angle = atan2 (c2.second - c0.second, c2.first - c0.first)
                    - atan2 (c1.second - c0.second, c1.first - c0.first);

            } else if (angleFor == 1) {
                angle = atan2 (c0.second - c1.second, c0.first - c1.first)
                    - atan2 (c2.second - c1.second, c2.first - c1.first);

            } else if (angleFor == 2) {
                angle = atan2 (c1.second - c2.second, c1.first - c2.first)
                    - atan2 (c0.second - c2.second, c0.first - c2.first);
            } // else throw error?

            return angle;
        }

        /*!
         * Compute the equation for the line that is drawn towards the putative centre of the
         * Dirichlet domain. Needs to be passed in the coordinates of the vertex that is clockwise
         * in the domain. As per Honda 1983, p 196, @prev_vertex is A_{i-1}, @next_vertex is
         * A_{i+1}. A_i and B_i are stored here as this->v (A_i) and this->vn (B_i). With this
         * information, it's possible to compute the gradient of the line that emerges from A_i
         * and heads towards the Dirichlet domain centre.
         */
        void compute_line_to_centre (const pair<Flt, Flt>& Aim1, const pair<Flt, Flt>& Aip1) {

            /*
             * 1. Compute phi, the angle Bi Ai Ai-1 using law of cosines
             */
            Flt phi = this->compute_angle (vn, v, Aim1, 1);
            Flt theta = morph::PI_F - phi;
            DBG ("phi = " << phi << " and theta = " << theta);

            /*
             * 2. Compute the line P_i wrt to Ai and Ai+1
             */
            // 2a Project A_i+1 onto the line P_i to get the length to a point Pi on line Pi.
            Flt Aip1Ai = this->line_length (Aip1, this->v);
            // Distance that we'll travel from Ai to get to the new point Pi.
            Flt AiPi = Aip1Ai * cos (theta);
            // 2b Determine the coordinates of point Pi using theta and the angle from the x axis
            // to Aip1.
            Flt xi = atan2 ((Aip1.second - this->v.second), (Aip1.first - this->v.first));
            DBG ("xi is " << xi);

            Flt deltax = AiPi * cos (theta + xi);
            DBG ("deltax from Ai to Pi is " << deltax);
            Flt deltay = AiPi * sin (theta + xi);
            DBG ("deltay from Ai to Pi is " << deltay);

            pair<Flt, Flt> Pi = this->v;
            Pi.first += deltax;
            Pi.second += deltay;

            DBG ("Point Pi: " << Pi.first << "," << Pi.second << ")");
            this->P_i = Pi;

            /*
             * 3. Use A_i and P_i to compute gradient/offset of the line equation that passes
             * through point A_i. Store in this->m and this->c (or whatever is suitable).
             */
            if (this->P_i.first == this->v.first) {
                this->m = this->P_i.second < this->v.second ? numeric_limits<Flt>::lowest() : numeric_limits<Flt>::max();
                this->c = numeric_limits<Flt>::max();
                // And if c is max(), then it's a vertical line from A_i through P_i which means
                // that P_i.first == A_i.first and P_i.second can be anything.
            } else {
                this->m = (this->P_i.second - this->v.second) / (this->P_i.first - this->v.first);
                this->c = this->v.second - this->m * this->v.first;
            }
            DBG ("Pi line gradient is m=" << this->m << ", offset is c=" << this->c);
        }
    };

} // namespace morph

#endif // DIRICHVTX
