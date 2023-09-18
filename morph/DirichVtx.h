/*!
 * \file
 *
 * Dirichlet domain vertex class.
 *
 * \author Seb James
 * \date 2019
 */
#pragma once

#include <morph/mathconst.h>
#include <cmath>
#include <limits>
#include <list>
#include <string>
#include <morph/HexGrid.h>
#include <morph/HdfData.h>

namespace morph {

    /*!
     * Dirichlet domain vertex class.
     *
     * This is a class which holds a vertex of a Dirichlet domain, and its vertex neighbour so that
     * a group of objects of this class type can define a single domain for Dirichlet-ness analysis
     * (after Honda 1983). You'll find a list<> of these in a morph::DirichDom.
     */
    template <typename Flt>
    class DirichVtx {
    public:
        //! The coordinate data for the main vertex represented.
        morph::vec<Flt, 2> v = { Flt{0}, Flt{0} };

        /*!
         * The series of points that make up the edge between the this vertex (v) and its vertex
         * neighbour (vn). Should order by size.
         */
        std::list<morph::vec<Flt, 2> > pathto_neighbour;

        /*!
         * Series of points that make the edge between thsi vertex and the next one in the list.
         */
        std::list<morph::vec<Flt, 2> > pathto_next;

        /*! The location of the neighbouring vertex - necessary for computing a Dirichlet-ness
         * metric. Intended to be populated after a set of vertices has been created, in a "second
         * pass" of a program.
         */
        morph::vec<Flt, 2>vn = { Flt{0}, Flt{0} };

        /*!
         * The value of the domain for which this vertex is a vertex. This is essentially the
         * domain's identity.
         */
        Flt f = Flt{0};

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
        morph::vec<Flt, 2> neighb = { Flt{0}, Flt{0} };

        /*!
         * An iterator into the accompanying list of Hexes. Intended to be an iterator into
         * HexGrid::hexen. This points to the hex containing the vertex that is this
         * DirichVtx. Important so that from one DirichVtx, we can find our way along an edge to
         * the next vertex.
         */
        std::list<Hex>::iterator hi;

        /*!
         * P_i is a point on the line. In this code, I project A_i+1 onto the line to find the
         * actual point P_i.
         */
        morph::vec<Flt, 2> P_i = { Flt{0}, Flt{0} };

        //! For marking vertices in a list as finsihed with, rather than erasing from that list.
        bool closed = false;

        /*!
         * I mark vertices as being on the boundary, too. This matters for the algorithms in
         * ShapeAnalysis.
         */
        bool onBoundary = false;

        //! Default constructor
        DirichVtx()
        {
            v[0] = std::numeric_limits<Flt>::max();
            v[1] = std::numeric_limits<Flt>::max();
        }

        //! Construct with passed in float pair, using default threshold
#if 0
        DirichVtx (const std::pair<float, Flt>& p)
        {
            morph::vec<Flt, 2> _p = { static_cast<Flt>(p[0]), p[1] };
        }
#endif
        /*!
         * Construct with passed in Flt coord, set the threshold on the basis of being passed in
         * the Hex to Hex distance d.
         */
        DirichVtx (const morph::vec<Flt, 2>& p, const Flt& d)
            : v(p)
        {
            // This is half of the shortest possible distance in the y direction between two
            // adjacent Hex vertices.
            this->threshold = d/(4.0f*morph::mathconst<float>::sqrt_of_3);
        }

        /*!
         * Construct with passed in Flt pair; set the threshold on the basis of being passed in
         * the Hex to Hex distance d and also set the value of the vertex to be @id
         */
        DirichVtx (const morph::vec<Flt, 2>& p, const Flt& d, const Flt& id)
            : v(p), f(id)
        {
            this->threshold = d/(4.0f*morph::mathconst<float>::sqrt_of_3);
        }

        /*!
         * Construct with passed in Flt pair; set the threshold on the basis of being passed in
         * the Hex to Hex distance d, set the value of the vertex to be @id and finally, set
         * this->neighb (with @oth).
         */
        DirichVtx (const morph::vec<Flt, 2>& p, const Flt& d, const Flt& id, const morph::vec<Flt, 2>& oth)
            : v(p), f(id), neighb(oth)
        {
            this->threshold = d/(4.0f*morph::mathconst<float>::sqrt_of_3);
        }

        /*!
         * Construct with passed in Flt pair; set the threshold on the basis of being passed in
         * the Hex to Hex distance d, set the value of the vertex to be @id, set this->neighb
         * (with @oth) and finally, set the list<Hex> iterator @hex.
         */
        DirichVtx (const morph::vec<Flt, 2>& p, const Flt& d, const Flt& id,
                   const morph::vec<Flt, 2>& oth, const std::list<Hex>::iterator hex)
            : v(p), f(id), neighb(oth), hi(hex) {
            this->threshold = d/(4.0f*morph::mathconst<float>::sqrt_of_3);
        }

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
            if (rhs.v[0] - this->v[0] > this->threshold) {
                return true;
            }
            if (this->v[0] - rhs.v[0] > this->threshold) {
                return false;
            }
            // Get here, then rhs.v[0] and this->v[0] are "equal"
            if (rhs.v[1] - this->v[1] > this->threshold) {
                return true;
            }
            if (rhs.v[1] < this->v[1]) {
                return false;
            }
            // Value and vertex coord equal; last check is on the vertex neighbour coord:
            if (rhs.vn[0] - this->vn[0] > this->threshold) {
                return true;
            }
            if (this->vn[0] - rhs.vn[0] > this->threshold) {
                return false;
            }
            // Get here, then rhs.v[0] and this->v[0] are "equal"
            if (rhs.vn[1] - this->vn[1] > this->threshold) {
                return true;
            }
            // Not necessary:
            // if (rhs.vn[1] < this->vn[1]) { return false; }
            return false;
        }

        //! Compare @other with this->v. Return true if they're the same.
        bool compare (const morph::vec<Flt, 2>& other) {
            // Equality too strong a test. Use this->threshold Is distance from v to other smaller
            // than threshold? If so return true.
            Flt distance = std::sqrt ( (other[0]-v[0])*(other[0]-v[0])
                                       + (other[1]-v[1])*(other[1]-v[1]) );
            if (distance < this->threshold) {
                return true;
            }
            return false;
        }

        //! Is this DirichVtx unset? If its this->v value is (max,max), then yes.
        bool unset() {
            if (this->v[0] == std::numeric_limits<Flt>::max()
                && this->v[1] == std::numeric_limits<Flt>::max()) {
                return true;
            }
            return false;
        }

        /*!
         * Compute line length of the line between coordinates @coord0 and @coord1
         */
        static Flt line_length (const morph::vec<Flt, 2>& coord0,
                                const morph::vec<Flt, 2>& coord1) {
            Flt c01 = std::sqrt ((coord0[0] - coord1[0]) * (coord0[0] - coord1[0])
                                 + (coord0[1] - coord1[1]) * (coord0[1] - coord1[1]));
            return c01;
        }

        /*!
         * For the three coordinates c0, c1, c2, compute the angle at coordinate number @angleFor
         * (counting from 0).
         *
         * Could go into morph::tools or morph::maths or something.
         */
        static Flt compute_angle (const morph::vec<Flt, 2>& c0,
                                  const morph::vec<Flt, 2>& c1,
                                  const morph::vec<Flt, 2>& c2,
                                  const unsigned int angleFor) {
            Flt angle = -1.0f;
            if (angleFor == 0) {
                angle = std::atan2 (c2[1] - c0[1], c2[0] - c0[0])
                    - std::atan2 (c1[1] - c0[1], c1[0] - c0[0]);

            } else if (angleFor == 1) {
                angle = std::atan2 (c0[1] - c1[1], c0[0] - c1[0])
                    - std::atan2 (c2[1] - c1[1], c2[0] - c1[0]);

            } else if (angleFor == 2) {
                angle = std::atan2 (c1[1] - c2[1], c1[0] - c2[0])
                    - std::atan2 (c0[1] - c2[1], c0[0] - c2[0]);
            } // else throw error?

            return angle;
        }

        /*!
         * Find the minimum distance from the point p to the Pi line defined in this object by P_i,
         * A_i (aka v).
         */
        Flt compute_distance_to_line (const morph::vec<Flt, 2>& p) const {
            // Find angle between Ai--Pi and Ai--p
            Flt angle = DirichVtx<Flt>::compute_angle (p, this->v, this->P_i, 1);
            // And distance from p to Ai
            Flt p_to_v = DirichVtx<Flt>::line_length (p, this->v);
            // Return projection of p onto line Ai--Pi
            return (p_to_v * sin (angle));
        }

        /*!
         * Compute the equation for the line that is drawn towards the putative centre of the
         * Dirichlet domain. Needs to be passed in the coordinates of the vertex that is clockwise
         * in the domain. As per Honda 1983, p 196, @prev_vertex is A_{i-1}, @next_vertex is
         * A_{i+1}. A_i and B_i are stored here as this->v (A_i) and this->vn (B_i). With this
         * information, it's possible to compute the gradient of the line that emerges from A_i
         * and heads towards the Dirichlet domain centre.
         */
        void compute_line_to_centre (const morph::vec<Flt, 2>& Aim1,
                                     const morph::vec<Flt, 2>& Aip1) {

            /*
             * 1. Compute phi, the angle Bi Ai Ai-1 using law of cosines
             */
            Flt phi = this->compute_angle (vn, v, Aim1, 1);
            Flt theta = morph::mathconst<float>::pi - phi;

            /*
             * 2. Compute the line P_i wrt to Ai and Ai+1
             */
            // 2a Project A_i+1 onto the line P_i to get the length to a point Pi on line Pi.
            Flt Aip1Ai = this->line_length (Aip1, this->v);
            // Distance that we'll travel from Ai to get to the new point Pi.
            Flt AiPi = Aip1Ai * cos (theta);
            // 2b Determine the coordinates of point Pi using theta and the angle from the x axis
            // to Aip1.
            Flt xi = std::atan2 ((Aip1[1] - this->v[1]), (Aip1[0] - this->v[0]));

            Flt deltax = AiPi * cos (theta + xi);
            Flt deltay = AiPi * sin (theta + xi);

            this->P_i = this->v;
            this->P_i[0] += deltax;
            this->P_i[1] += deltay;
        }

        //! Save data from the DirichVtx. Not saving ALL members of this class (e.g. omitting threshold)
        void save (HdfData& data, const std::string& pathroot) const
        {
            std::string p("");
            p = pathroot + "/v";
            data.add_contained_vals (p.c_str(), this->v);
            p = pathroot + "/vn";
            data.add_contained_vals (p.c_str(), this->vn);
            p = pathroot + "/f";
            data.add_val (p.c_str(), this->f);
            p = pathroot + "/neighb";
            data.add_contained_vals (p.c_str(), this->neighb);
            p = pathroot + "/P_i";
            data.add_contained_vals (p.c_str(), this->P_i);
            p = pathroot + "/onBoundary";
            data.add_val (p.c_str(), this->onBoundary);
            // Finally,
            p = pathroot + "/pathto_neighbour";
            data.add_contained_vals (p.c_str(), this->pathto_neighbour);
            p = pathroot + "/pathto_next";
            data.add_contained_vals (p.c_str(), this->pathto_next);
        }
    };

} // namespace morph
