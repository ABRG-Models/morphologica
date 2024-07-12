#pragma once

#include <array>
#include <bitset>

#include <morph/VisualModel.h>
#include <morph/vec.h>
#include <morph/mathconst.h>
#include <morph/colour.h>
#include <morph/MathAlgo.h>

namespace morph {

    // Do the line segments p1-q1 and p2-q2 intersect? Are they instead colinear? Return
    // these booleans in a bitset (bit0: intersection, bit1: colinear)
    std::bitset<2> segments_intersect (vec<float, 2>& p1, vec<float, 2> q1,
                                       vec<float, 2>& p2, vec<float, 2> q2)
    {
        std::cout << "Line " << p1 << " to " << q1 << " intersects with " << p2 << " to " << q2 << "??" << std::endl;
        std::bitset<2> rtn;
        morph::rotation_sense p1q1p2 = morph::MathAlgo::orientation (p1,q1,p2);
        morph::rotation_sense p1q1q2 = morph::MathAlgo::orientation (p1,q1,q2);
        morph::rotation_sense p2q2p1 = morph::MathAlgo::orientation (p2,q2,p1);
        morph::rotation_sense p2q2q1 = morph::MathAlgo::orientation (p2,q2,q1);
        if (p1q1p2 != p1q1q2 && p2q2p1 != p2q2q1) { // They intersect
             rtn.set(0, true);
        } else { // Are they colinear?
            if (p1q1p2 == morph::rotation_sense::colinear && morph::MathAlgo::onsegment (p1, p2, q1)) { rtn.set(1, true); }
            else if (p1q1q2 == morph::rotation_sense::colinear && morph::MathAlgo::onsegment (p1, q2, q1)) { rtn.set(1, true); }
            else if (p2q2p1 == morph::rotation_sense::colinear && morph::MathAlgo::onsegment (p2, p1, q2)) { rtn.set(1, true); }
            else if (p2q2q1 == morph::rotation_sense::colinear && morph::MathAlgo::onsegment (p2, q1, q2)) { rtn.set(1, true); }
        }
        return rtn;
    }

    // Find crossing point assuming segments intersect
    vec<float, 2> crossing_point (vec<float, 2>& p1, vec<float, 2>& q1, vec<float, 2>& p2, vec<float, 2>& q2)
    {
        vec<float, 2> p = p1;
        vec<float, 2> r = p1 - q1;
        vec<float, 2> q = p2;
        vec<float, 2> s = p2 - q2;
        auto t = (q - p).cross(s / r.cross(s));
        // auto u = (q - p).cross(r / r.cross(s)); // 2D cross products
        vec<float, 2> cross_point = p + t * r;
        return cross_point;
    }

    //! This class creates the vertices for a rectangular moving grating
    template<int glver = morph::gl::version_4_1>
    class GratingVisual : public VisualModel<glver>
    {
    public:
        GratingVisual() { this->mv_offset = {0.0, 0.0, 0.0}; }
        GratingVisual (const vec<float, 3> _offset) { this->init (_offset); }
        ~GratingVisual () {}

        void init (const vec<float, 3> _offset)
        {
            this->mv_offset = _offset;
            this->viewmatrix.translate (this->mv_offset);
        }

        void initializeVertices()
        {
            this->vertexPositions.clear();
            this->vertexNormals.clear();
            this->vertexColors.clear();
            this->indices.clear();

            // The velocity offset for each location of each front
            vec<float, 2> v_offset = this->v_front * this->t;

            // rectangle window centre
            //vec<float, 2> r_centre = this->mv_offset.less_one_dim() + dims * 0.5f;

            // length of diagonal across window
            //float diag_length = dims.length();

            // unit vector in x dirn
            vec<float, 2> u_x = { 1.0f, 0.0f };

            // The unit vector perpendicular to the front angle
            vec<float, 2> u_alpha = u_x;
            u_alpha.set_angle (morph::mathconst<float>::deg2rad * this->alpha);
            vec<float, 2> u_alpha_perp = u_x;
            u_alpha_perp.set_angle (morph::mathconst<float>::pi_over_2 + morph::mathconst<float>::deg2rad * this->alpha);

            // Line segments of the borders. Will need these for computing line crossings. each line segment is 'pq'
            vec<float, 2> bot_p = v_offset + vec<float, 2>{ this->mv_offset[0],  this->mv_offset[1] };
            vec<float, 2> bot_q = v_offset + vec<float, 2>{ this->mv_offset[0] + dims[0],   this->mv_offset[1] };

            vec<float, 2> top_p = v_offset + vec<float, 2>{ this->mv_offset[0],             this->mv_offset[1] + dims[1] };
            vec<float, 2> top_q = v_offset + vec<float, 2>{ this->mv_offset[0] + dims[0],   this->mv_offset[1] + dims[1] };

            vec<float, 2> left_p = v_offset + vec<float, 2>{ this->mv_offset[0],            this->mv_offset[1] };
            vec<float, 2> left_q = v_offset + vec<float, 2>{ this->mv_offset[0],            this->mv_offset[1] + dims[1] };

            vec<float, 2> right_p = v_offset + vec<float, 2>{ this->mv_offset[0] + dims[0], this->mv_offset[1] };
            vec<float, 2> right_q = v_offset + vec<float, 2>{ this->mv_offset[0] + dims[0], this->mv_offset[1] + dims[1] };

            // Consider a point on wave 0. (x0, y0). It is now at (x', y') = (x0, y0) + t * v;
            vec<float, 2> num_lambdas = v_offset / this->lambda;

            float lambdas_x = std::floor (num_lambdas.dot (u_x)); // Number of wavelengths that the point that was at origin at t0 has moved

            float x_0 = v_offset[0] - lambdas_x; // Here's where we draw from.
            vec<float, 2> p_0 = { x_0, 0.0f };

            auto dx = dims.length() * u_alpha_perp;

            vec<float, 2> p1, q1;
            vec<float, 2> fp1, fp2, fq1, fq2;

            unsigned int i = 0;
            for (vec<float, 2> p = p_0; ; p += 0.5f * lambda * u_alpha) { // Steps fwd and back along u_alpha until all fronts were drawn

                // Make test lines for start of field
                p1 = p + dx;
                q1 = p - dx;

                auto bi = segments_intersect (p1, q1, bot_p, bot_q);
                auto ti = segments_intersect (p1, q1, top_p, top_q);
                auto li = segments_intersect (p1, q1, left_p, left_q);
                auto ri = segments_intersect (p1, q1, right_p, right_q);

                std::cout << "bot intersect: " << bi << std::endl;
                std::cout << "top intersect: " << ti << std::endl;
                std::cout << "left intersect: " << li << std::endl;
                std::cout << "right intersect: " << ri << std::endl;

                if (bi.test(1) || ti.test(1) || li.test(1) || ri.test(1)) {
                    // Use the relevant edge.
                    throw std::runtime_error ("Implement me");
                }

                if (!bi.test(0) && !ti.test(0) && !li.test(0) && !ri.test(0)) {
                    // We're off the rectangle
                    std::cout << "break on loop " << i << std::endl;
                    break;
                }

                if (bi.test(0)) { // bottom
                    fp1 = crossing_point (p1, q1, bot_p, bot_q);
                    if (ti.test(0)) {
                        // bottom and top edges
                        fq1 = crossing_point (p1, q1, top_p, top_q);

                    } else if (li.test(0)) {
                        // bottom and left edges
                        fq1 = crossing_point (p1, q1, top_p, top_q);

                    } else if (ri.test(0)) {
                        // bottom and right edges
                        fq1 = crossing_point (p1, q1, right_p, right_q);
                    } else {
                        throw std::runtime_error ("unexpected1");
                    }
                } else if (ti.test(0)) {
                    fp1 = crossing_point (p1, q1, bot_p, bot_q);
                    if (li.test(0)) {
                        // top and left
                        fq1 = crossing_point (p1, q1, left_p, left_q);
                    } else if (ri.test(0)) {
                        // top and right
                        fq1 = crossing_point (p1, q1, right_p, right_q);
                    } else {
                        throw std::runtime_error ("unexpected2");
                    }
                } else if (li.test(0)) {
                    fp1 = crossing_point (p1, q1, left_p, left_q);
                    if (ri.test(0)) {
                        // left and right
                        fq1 = crossing_point (p1, q1, right_p, right_q);
                    } else {
                        throw std::runtime_error ("unexpected3");
                    }
                }

                // Second line
                p1 = p + 0.5f * lambda * u_alpha + dx;
                q1 = p + 0.5f * lambda * u_alpha - dx;

                // repeat for setting fp2, fq2
                bi = segments_intersect (p1, q1, bot_p, bot_q);
                ti = segments_intersect (p1, q1, top_p, top_q);
                li = segments_intersect (p1, q1, left_p, left_q);
                ri = segments_intersect (p1, q1, right_p, right_q);

                if (bi.test(1) || ti.test(1) || li.test(1) || ri.test(1)) {
                    // Use the relevant edge.
                    throw std::runtime_error ("Implement me");
                }

                if (bi.test(0)) { // bottom
                    fp2 = crossing_point (p1, q1, bot_p, bot_q);
                    if (ti.test(0)) {
                        // bottom and top edges
                        fq2 = crossing_point (p1, q1, top_p, top_q);

                    } else if (li.test(0)) {
                        // bottom and left edges
                        fq2 = crossing_point (p1, q1, top_p, top_q);

                    } else if (ri.test(0)) {
                        // bottom and right edges
                        fq2 = crossing_point (p1, q1, right_p, right_q);
                    } else {
                        throw std::runtime_error ("unexpected1");
                    }
                } else if (ti.test(0)) {
                    fp2 = crossing_point (p1, q1, bot_p, bot_q);
                    if (li.test(0)) {
                        // top and left
                        fq2 = crossing_point (p1, q1, left_p, left_q);
                    } else if (ri.test(0)) {
                        // top and right
                        fq2 = crossing_point (p1, q1, right_p, right_q);
                    } else {
                        throw std::runtime_error ("unexpected2");
                    }
                } else if (li.test(0)) {
                    fp2 = crossing_point (p1, q1, left_p, left_q);
                    if (ri.test(0)) {
                        // left and right
                        fq2 = crossing_point (p1, q1, right_p, right_q);
                    } else {
                        throw std::runtime_error ("unexpected3");
                    }
                }

                // Now build
                this->vertex_push (fp1.plus_one_dim(), this->vertexPositions);
                this->vertex_push (fq1.plus_one_dim(), this->vertexPositions);
                this->vertex_push (fp2.plus_one_dim(), this->vertexPositions);
                this->vertex_push (fq2.plus_one_dim(), this->vertexPositions);
                this->vertex_push (i%2==0 ? colour1 : colour2, this->vertexColors);
                this->vertex_push (i%2==0 ? colour1 : colour2, this->vertexColors);
                this->vertex_push (i%2==0 ? colour1 : colour2, this->vertexColors);
                this->vertex_push (i%2==0 ? colour1 : colour2, this->vertexColors);
                this->vertex_push (this->uz, this->vertexNormals);
                this->vertex_push (this->uz, this->vertexNormals);
                this->vertex_push (this->uz, this->vertexNormals);
                this->vertex_push (this->uz, this->vertexNormals);
                this->indices.push_back (this->idx);
                this->indices.push_back (this->idx+1);
                this->indices.push_back (this->idx+2);

                this->indices.push_back (this->idx+2);
                this->indices.push_back (this->idx+1);
                this->indices.push_back (this->idx+3);

                this->idx += 4;

                ++i;
            }

            // Lastly, draw border
            this->computeFlatLine (this->idx, bot_p.plus_one_dim(), bot_q.plus_one_dim(),
                                   this->uz, morph::colour::black, 0.05f);

            this->computeFlatLine (this->idx, right_p.plus_one_dim(), right_q.plus_one_dim(),
                                   this->uz, morph::colour::black, 0.05f);

            this->computeFlatLine (this->idx, top_p.plus_one_dim(), top_q.plus_one_dim(),
                                   this->uz, morph::colour::black, 0.05f);

            this->computeFlatLine (this->idx, left_p.plus_one_dim(), left_q.plus_one_dim(),
                                   this->uz, morph::colour::black, 0.05f);

            std::cout << "Init vertices done\n";
        }

        //! The colours of the bands
        std::array<float, 3> colour1 = morph::colour::darkorchid3;
        std::array<float, 3> colour2 = morph::colour::violet;
        //! The velocity of the fronts.
        vec<float, 2> v_front = { 0.0f, 0.0f };
        //! The wavelength of the fronts
        float lambda = 0.1f;
        //! The angle of the fronts, wrt x.
        float alpha = 45.0f;
        //! Width, height (after any rotation?)
        vec<float, 2> dims = { 2.0f, 1.0f };
        //! Current time
        unsigned long long int t = 0;
        //! How many bands? -1 means fill the field
        int num_bands = -1;
    };

} // namespace morph
