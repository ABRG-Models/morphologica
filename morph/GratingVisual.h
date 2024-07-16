#pragma once

#include <array>
#include <bitset>

#include <morph/VisualModel.h>
#include <morph/vec.h>
#include <morph/mathconst.h>
#include <morph/colour.h>
#include <morph/MathAlgo.h>

namespace morph {

    enum class border_id {
        top,
        bottom,
        left,
        right,
        unknown
    };

    std::string border_id_str (const border_id& id)
    {
        std::string rtn("unknown");
        if (id == border_id::top) {
            rtn = "top";
        } else if (id == border_id::bottom) {
            rtn = "bottom";
        } else if (id == border_id::left) {
            rtn = "left";
        } else if (id == border_id::right) {
            rtn = "right";
        }
        return rtn;
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

            // unit vector in x dirn
            vec<float, 2> u_x = { 1.0f, 0.0f };

            // The unit vector perpendicular to the front angle
            vec<float, 2> u_alpha = u_x;
            u_alpha.set_angle (morph::mathconst<float>::deg2rad * this->alpha);
            vec<float, 2> u_alpha_perp = u_x;
            u_alpha_perp.set_angle (morph::mathconst<float>::pi_over_2 + morph::mathconst<float>::deg2rad * this->alpha);

            // Corners
            vec<float, 2> top_left =  vec<float, 2>{ this->mv_offset[0],             this->mv_offset[1] + dims[1] };
            vec<float, 2> bot_left =  vec<float, 2>{ this->mv_offset[0],             this->mv_offset[1]           }; // or mv_offset
            vec<float, 2> top_right = vec<float, 2>{ this->mv_offset[0] + dims[0],   this->mv_offset[1] + dims[1] }; // or mv_offset + dims
            vec<float, 2> bot_right = vec<float, 2>{ this->mv_offset[0] + dims[0],   this->mv_offset[1]           };

            // Line segments of the borders. Will need these for computing line crossings. each line segment is 'pq'
            vec<float, 2> bot_p = bot_left;
            vec<float, 2> bot_q = bot_right;
            //
            vec<float, 2> top_p = top_left;
            vec<float, 2> top_q = top_right;
            //
            vec<float, 2> left_p = bot_left;
            vec<float, 2> left_q = top_left;
            //
            vec<float, 2> right_p = bot_right;
            vec<float, 2> right_q = top_right;

            // Consider a point on wave 0. (x0, y0). It is now at (x', y') = (x0, y0) + t * v;
            vec<float, 2> num_lambdas = v_offset / this->lambda;
            std::cout << "num_lambdas moved: " << num_lambdas << std::endl;
            std::cout << "num_lambdas moved effectively: " << num_lambdas.dot (u_alpha) << std::endl;

            float x_lambdas_f = num_lambdas.dot (u_alpha);
            //std::cout << "num_lambdas . u_x: " << dp << ", trunc("<<dp<<") = " << std::trunc (dp) << std::endl;
            float lambdas_x = std::trunc (x_lambdas_f); // Number of wavelengths that the point that was at origin at t0 has moved
            std::cout << "That's: " << lambdas_x << " bands along the x axis" << std::endl;

            float length_of_lambda_in_x = this->lambda / std::cos(morph::mathconst<float>::deg2rad * this->alpha);
            float lambdas_x_offset = lambdas_x * length_of_lambda_in_x;
            std::cout << "length_of_lambda_in_x = " << length_of_lambda_in_x << " so  lambdas_x_offset = " <<  lambdas_x_offset  << std::endl;

            float x_0 = v_offset[0] - lambdas_x_offset;

            std::cout << "x_0 = v_offset[0] - lambdas_x_offset = " <<  v_offset[0]
                      << " - " << lambdas_x_offset << " = " << x_0 << std::endl;

            vec<float, 2> p_0 = { x_0, 0.0f };

            auto dx = dims.length() * u_alpha_perp;

            vec<float, 2> p1 = {0,0}, q1 = {0,0}, p2 = {0,0}, q2 = {0,0};
            vec<float, 2> fp1 = {0,0}, fp2 = {0,0}, fq1 = {0,0}, fq2 = {0,0};

            // Identifiers for the final crossing points.
            border_id fp1_id = border_id::unknown;
            border_id fq1_id = border_id::unknown;
            border_id fp2_id = border_id::unknown;
            border_id fq2_id = border_id::unknown;

            unsigned int i = 0;
            for (vec<float, 2> p = p_0; ; p += 0.5f * lambda * u_alpha) { // Steps fwd and back along u_alpha until all fronts were drawn

                auto col = i%2==0 ? colour1 : colour2;

                // Make test lines for start of field
                p1 = p + dx;
                q1 = p - dx;

                auto bi = morph::MathAlgo::segments_intersect (p1, q1, bot_p, bot_q);
                auto ti = morph::MathAlgo::segments_intersect (p1, q1, top_p, top_q);
                auto li = morph::MathAlgo::segments_intersect (p1, q1, left_p, left_q);
                auto ri = morph::MathAlgo::segments_intersect (p1, q1, right_p, right_q);

                if (bi.test(1) || ti.test(1) || li.test(1) || ri.test(1)) {
                    // Use the relevant edge.
                    throw std::runtime_error ("Implement me");
                }

                if (!bi.test(0) && !ti.test(0) && !li.test(0) && !ri.test(0)) {
                    // We're off the rectangle
                    //std::cout << "break on loop " << i << std::endl;
                    break;
                }

                if (bi.test(0)) { // bottom
                    fp1 = morph::MathAlgo::crossing_point (p1, q1, bot_p, bot_q);
                    fp1_id = border_id::bottom;
                    if (ti.test(0)) {
                        // bottom and top edges
                        fq1 = morph::MathAlgo::crossing_point (p1, q1, top_p, top_q);
                        fq1_id = border_id::top;

                    } else if (li.test(0)) {
                        // bottom and left edges
                        fq1 = morph::MathAlgo::crossing_point (p1, q1, left_p, left_q);
                        fq1_id = border_id::left;

                    } else if (ri.test(0)) {
                        // bottom and right edges
                        fq1 = morph::MathAlgo::crossing_point (p1, q1, right_p, right_q);
                        fq1_id = border_id::right;

                    } else {
                        throw std::runtime_error ("unexpected1");
                    }
                } else if (ti.test(0)) {
                    fp1 = morph::MathAlgo::crossing_point (p1, q1, top_p, top_q);
                    fp1_id = border_id::top;

                    if (li.test(0)) {
                        // top and left
                        fq1 = morph::MathAlgo::crossing_point (p1, q1, left_p, left_q);
                        fq1_id = border_id::left;

                    } else if (ri.test(0)) {
                        // top and right
                        fq1 = morph::MathAlgo::crossing_point (p1, q1, right_p, right_q);
                        fq1_id = border_id::right;

                    } else {
                        throw std::runtime_error ("unexpected2");
                    }
                } else if (li.test(0)) {
                    fp1 = morph::MathAlgo::crossing_point (p1, q1, left_p, left_q);
                    fp1_id = border_id::left;

                    if (ri.test(0)) {
                        // left and right
                        fq1 = morph::MathAlgo::crossing_point (p1, q1, right_p, right_q);
                        fq1_id = border_id::right;

                    } else {
                        throw std::runtime_error ("unexpected3");
                    }
                }

                // Second line
                p2 = p + 0.5f * lambda * u_alpha + dx;
                q2 = p + 0.5f * lambda * u_alpha - dx;

                // repeat for setting fp2, fq2
                bi = morph::MathAlgo::segments_intersect (p2, q2, bot_p, bot_q);
                ti = morph::MathAlgo::segments_intersect (p2, q2, top_p, top_q);
                li = morph::MathAlgo::segments_intersect (p2, q2, left_p, left_q);
                ri = morph::MathAlgo::segments_intersect (p2, q2, right_p, right_q);

                if (bi.test(1) || ti.test(1) || li.test(1) || ri.test(1)) {
                    // Use the relevant edge.
                    throw std::runtime_error ("Implement me");
                }

                if (bi.test(0)) { // bottom
                    fp2 = morph::MathAlgo::crossing_point (p2, q2, bot_p, bot_q);
                    fp2_id = border_id::bottom;

                    if (ti.test(0)) {
                        // bottom and top edges
                        fq2 = morph::MathAlgo::crossing_point (p2, q2, top_p, top_q);
                        fq2_id = border_id::top;

                    } else if (li.test(0)) {
                        // bottom and left edges
                        fq2 = morph::MathAlgo::crossing_point (p2, q2, left_p, left_q);
                        fq2_id = border_id::left;

                    } else if (ri.test(0)) {
                        // bottom and right edges
                        fq2 = morph::MathAlgo::crossing_point (p2, q2, right_p, right_q);
                        fq2_id = border_id::right;

                    } else {
                        throw std::runtime_error ("unexpected1");
                    }
                } else if (ti.test(0)) {
                    fp2 = morph::MathAlgo::crossing_point (p2, q2, top_p, top_q);
                    fp2_id = border_id::top;

                    if (li.test(0)) {
                        // top and left
                        fq2 = morph::MathAlgo::crossing_point (p2, q2, left_p, left_q);
                        fq2_id = border_id::left;

                    } else if (ri.test(0)) {
                        // top and right
                        fq2 = morph::MathAlgo::crossing_point (p2, q2, right_p, right_q);
                        fq2_id = border_id::right;

                    } else {
                        throw std::runtime_error ("unexpected2");
                    }
                } else if (li.test(0)) {
                    fp2 = morph::MathAlgo::crossing_point (p2, q2, left_p, left_q);
                    fp2_id = border_id::left;

                    if (ri.test(0)) {
                        // left and right
                        fq2 = morph::MathAlgo::crossing_point (p2, q2, right_p, right_q);
                        fq2_id = border_id::right;

                    } else {
                        throw std::runtime_error ("unexpected3");
                    }
                }

#if 0
                std::cout << "Borders crossed: fp1: "
                          << border_id_str(fp1_id) << ", fp2: "
                          << border_id_str(fp2_id) << ", fq1: "
                          << border_id_str(fq1_id) << ", fq2: "
                          << border_id_str(fq2_id) << "\n";
#endif
                // sanity check
                if (fp1_id == border_id::unknown
                    || fq1_id == border_id::unknown
                    || fp2_id == border_id::unknown
                    || fq2_id == border_id::unknown) {
                    throw std::runtime_error ("unexpected border_id");
                }

                // We may end up with an 'incongruent' order of the final ps and qs and may wish to
                // switch one pair:
                if (fp2_id == fq1_id && fp1_id != fp2_id) {
                    // Swap p2 and q2 order
                    //std::cout << "Swapping fp2/fq2 order\n";
                    auto tmp = fq2;
                    auto tmp_id = fq2_id;
                    fq2 = fp2;
                    fq2_id = fp2_id;
                    fp2 = tmp;
                    fp2_id = tmp_id;

                } else if (fq2_id == fp1_id && fq1_id != fq2_id) {
                    // Swap p1 and q1 order
                    //std::cout << "Swapping fp1/fq1 order\n";
                    auto tmp = fq1;
                    auto tmp_id = fq1_id;
                    fq1 = fp1;
                    fq1_id = fp1_id;
                    fp1 = tmp;
                    fp1_id = tmp_id;
                }

                // Determine if fill-in triangles should be drawn
                if (fp1_id != fp2_id) {

                    vec<float, 2> corner1 = { 0, 0 };
                    if ((fp1_id == border_id::left && fp2_id == border_id::top)
                        || (fp2_id == border_id::left && fp1_id == border_id::top)) {
                        corner1 = top_left;
                    } else if ((fp1_id == border_id::left && fp2_id == border_id::bottom)
                               || (fp2_id == border_id::left && fp1_id == border_id::bottom)) {
                        corner1 = bot_left;
                    } else if ((fp1_id == border_id::right && fp2_id == border_id::bottom)
                               || (fp2_id == border_id::right && fp1_id == border_id::bottom)) {
                        corner1 = bot_right;
                    } else if ((fp1_id == border_id::right && fp2_id == border_id::top)
                               || (fp2_id == border_id::right && fp1_id == border_id::top)) {
                        corner1 = top_right;
                    } else {
                        throw std::runtime_error ("unexpected corner1");
                        corner1 = {-100.0f,-100.0f};
                    }

                    if (corner1[0] != -100.0f) {
                        // Draw triangle points fp1_id, fp2_id and corner1
                        this->vertex_push (fp1.plus_one_dim(), this->vertexPositions);
                        this->vertex_push (fp2.plus_one_dim(), this->vertexPositions);
                        this->vertex_push (corner1.plus_one_dim(), this->vertexPositions);
                        this->vertex_push (col, this->vertexColors);
                        this->vertex_push (col, this->vertexColors);
                        this->vertex_push (col, this->vertexColors);
                        this->vertex_push (this->uz, this->vertexNormals);
                        this->vertex_push (this->uz, this->vertexNormals);
                        this->vertex_push (this->uz, this->vertexNormals);
                        this->indices.push_back (this->idx);
                        this->indices.push_back (this->idx+1);
                        this->indices.push_back (this->idx+2);
                        this->idx += 3;
                    }
                }

                if (fq1_id != fq2_id) {

                    vec<float, 2> corner2 = { 0, 0 };
                    if ((fq1_id == border_id::left && fq2_id == border_id::top)
                        || (fq2_id == border_id::left && fq1_id == border_id::top)) {
                        corner2 = top_left;
                    } else if ((fq1_id == border_id::left && fq2_id == border_id::bottom)
                               || (fq2_id == border_id::left && fq1_id == border_id::bottom)) {
                        corner2 = bot_left;
                    } else if ((fq1_id == border_id::right && fq2_id == border_id::bottom)
                               || (fq2_id == border_id::right && fq1_id == border_id::bottom)) {
                        corner2 = bot_right;
                    } else if ((fq1_id == border_id::right && fq2_id == border_id::top)
                               || (fq2_id == border_id::right && fq1_id == border_id::top)) {
                        corner2 = top_right;
                    } else {
                        throw std::runtime_error ("unexpected corner2");
                    }

                    // Draw triangle points fq1_id, fq2_id and corner1
                    this->vertex_push (fq1.plus_one_dim(), this->vertexPositions);
                    this->vertex_push (fq2.plus_one_dim(), this->vertexPositions);
                    this->vertex_push (corner2.plus_one_dim(), this->vertexPositions);
                    this->vertex_push (col, this->vertexColors);
                    this->vertex_push (col, this->vertexColors);
                    this->vertex_push (col, this->vertexColors);
                    this->vertex_push (this->uz, this->vertexNormals);
                    this->vertex_push (this->uz, this->vertexNormals);
                    this->vertex_push (this->uz, this->vertexNormals);
                    this->indices.push_back (this->idx);
                    this->indices.push_back (this->idx+1);
                    this->indices.push_back (this->idx+2);
                    this->idx += 3;
                }

                // Now build the band
                this->vertex_push (fp1.plus_one_dim(), this->vertexPositions);
                this->vertex_push (fq1.plus_one_dim(), this->vertexPositions);
                this->vertex_push (fp2.plus_one_dim(), this->vertexPositions);
                this->vertex_push (fq2.plus_one_dim(), this->vertexPositions);
                this->vertex_push (col, this->vertexColors);
                this->vertex_push (col, this->vertexColors);
                this->vertex_push (col, this->vertexColors);
                this->vertex_push (col, this->vertexColors);
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

#if 1 // debug line points
                this->computeSphere (this->idx, p1.plus_one_dim(), colour1, 0.02f, 16, 20);
                this->computeSphere (this->idx, q1.plus_one_dim(), colour1, 0.02f, 16, 20);
                this->computeSphere (this->idx, p2.plus_one_dim(), colour2, 0.02f, 16, 20);
                this->computeSphere (this->idx, q2.plus_one_dim(), colour2, 0.02f, 16, 20);
#endif

                ++i;
            }

#if 0
            // Lastly, draw border
            this->computeFlatLine (this->idx, bot_p.plus_one_dim(), bot_q.plus_one_dim(),
                                   this->uz, morph::colour::black, 0.01f);

            this->computeFlatLine (this->idx, right_p.plus_one_dim(), right_q.plus_one_dim(),
                                   this->uz, morph::colour::black, 0.01f);

            this->computeFlatLine (this->idx, top_p.plus_one_dim(), top_q.plus_one_dim(),
                                   this->uz, morph::colour::black, 0.01f);

            this->computeFlatLine (this->idx, left_p.plus_one_dim(), left_q.plus_one_dim(),
                                   this->uz, morph::colour::black, 0.01f);
#endif
        }

        //! The colours of the bands
        std::array<float, 3> colour1 = morph::colour::mediumorchid1;
        std::array<float, 3> colour2 = morph::colour::plum2;
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
