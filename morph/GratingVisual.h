/*
 * A VisualModel to show a grating of straight bars at any angle and in any two colours. A time can
 * be set so that the grating can be moved in time according to a 'front velocity'
 *
 * Author: Seb James
 * Date: July 2024
 */

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

            /**
             * Subroutine for finding the band vertices on the boundary given as a lambda
             */
            auto find_border_points =
            [bot_p, bot_q, top_p, top_q, left_p, left_q, right_p, right_q]
            (const vec<float, 2>& _p, const vec<float, 2>& _q,
             vec<float, 2>& fp, vec<float, 2>& fq,
             border_id& fp_id, border_id& fq_id,
             const std::bitset<2>& _bi,
             const std::bitset<2>& _ti,
             const std::bitset<2>& _li,
             const std::bitset<2>& _ri)
            {
                if (_bi.test(0)) { // bottom
                    fp = morph::MathAlgo::crossing_point (_p, _q, bot_p, bot_q);
                    fp_id = border_id::bottom;
                    if (_ti.test(0)) {
                        // bottom and top edges
                        fq = morph::MathAlgo::crossing_point (_p, _q, top_p, top_q);
                        fq_id = border_id::top;

                    } else if (_li.test(0)) {
                        // bottom and left edges
                        fq = morph::MathAlgo::crossing_point (_p, _q, left_p, left_q);
                        fq_id = border_id::left;

                    } else if (_ri.test(0)) {
                        // bottom and right edges
                        fq = morph::MathAlgo::crossing_point (_p, _q, right_p, right_q);
                        fq_id = border_id::right;

                    } else {
                        throw std::runtime_error ("unexpected1");
                    }
                } else if (_ti.test(0)) {
                    fp = morph::MathAlgo::crossing_point (_p, _q, top_p, top_q);
                    fp_id = border_id::top;

                    if (_li.test(0)) {
                        // top and left
                        fq = morph::MathAlgo::crossing_point (_p, _q, left_p, left_q);
                        fq_id = border_id::left;

                    } else if (_ri.test(0)) {
                        // top and right
                        fq = morph::MathAlgo::crossing_point (_p, _q, right_p, right_q);
                        fq_id = border_id::right;

                    } else {
                        throw std::runtime_error ("unexpected2");
                    }
                } else if (_li.test(0)) {
                    //std::cout << "li.test(0) is true - we have left intersection\n";
                    fp = morph::MathAlgo::crossing_point (_p, _q, left_p, left_q);
                    fp_id = border_id::left;

                    if (_ri.test(0)) {
                        // left and right
                        fq = morph::MathAlgo::crossing_point (_p, _q, right_p, right_q);
                        fq_id = border_id::right;
                    } else {
                        throw std::runtime_error ("unexpected3");
                    }
                } else if (_ri.test(0)) {
                    std::cout << "We have ri first\n";
                }

                std::cout << "find_border_points: We have fp " << fp << " crossing " << border_id_str(fp_id)
                          << " and fq " << fq << " crossing " << border_id_str(fq_id) << std::endl;
            }; // end of find_border_points()

            /**
             * Lambda to draw triangle/quadrilateral fill in shape given two points and their
             * border intersection identifications.
             */
            auto fill_in_shape = [this, top_left, bot_left, bot_right, top_right]
            (const vec<float, 2>& _p, const vec<float, 2>& fp, const vec<float, 2>& fq,
             const border_id& fp_id, const border_id& fq_id, const std::array<float, 3>& _col)
            {
                vec<float, 2> corner = { 0, 0 };
                vec<float, 2> corner_2 = { -100, -100 };
                if ((fp_id == border_id::left && fq_id == border_id::top)
                    || (fq_id == border_id::left && fp_id == border_id::top)) {
                    corner = top_left;
                } else if ((fp_id == border_id::left && fq_id == border_id::bottom)
                           || (fq_id == border_id::left && fp_id == border_id::bottom)) {
                    corner = bot_left;
                } else if ((fp_id == border_id::right && fq_id == border_id::bottom)
                           || (fq_id == border_id::right && fp_id == border_id::bottom)) {
                    corner = bot_right;
                } else if ((fp_id == border_id::right && fq_id == border_id::top)
                           || (fq_id == border_id::right && fp_id == border_id::top)) {
                    corner = top_right;

                } else if ((fp_id == border_id::bottom && fq_id == border_id::top)
                           || (fq_id == border_id::bottom && fp_id == border_id::top)) {
                    // vertical bands
                    std::cout << "fill in a v band\n";
                    float d_to_left = (_p - bot_left).length();
                    float d_to_right = (_p - bot_right).length();
                    corner = d_to_left < d_to_right ? bot_left : bot_right;
                    //corner_2 = d_to_left < d_to_right ? top_left : top_right;
                    return;
                } else if ((fp_id == border_id::left && fq_id == border_id::right)
                           || (fq_id == border_id::left && fp_id == border_id::right)) {
                    // horz bands. Top or bottom? As long as bands are smaller than the height
                    // of the rectangle, we can use the closest.
                    std::cout << "fill in an h band\n";
                    float d_to_top = (_p - top_left).length();
                    float d_to_bottom = (_p - bot_left).length();
                    corner = d_to_top < d_to_bottom ? top_left : bot_left;
                    //corner_2 = d_to_top < d_to_bottom ? top_right : bot_right;
                    return;
                } else {
                    throw std::runtime_error ("unexpected corner");
                }

                if (corner_2 == morph::vec<float, 2>{-100, -100 }) {
                    // Draw triangle
                    this->vertex_push (fp.plus_one_dim(), this->vertexPositions);
                    this->vertex_push (fq.plus_one_dim(), this->vertexPositions);
                    this->vertex_push (corner.plus_one_dim(), this->vertexPositions);
                    for (unsigned int vi = 0; vi < 3; ++vi) {
                        this->vertex_push (_col, this->vertexColors);
                        this->vertex_push (this->uz, this->vertexNormals);
                        this->indices.push_back (this->idx++);
                    }
                } else {
                    // Draw quatrilateral
                    this->vertex_push (fp.plus_one_dim(), this->vertexPositions);
                    this->vertex_push (corner.plus_one_dim(), this->vertexPositions);
                    this->vertex_push (fq.plus_one_dim(), this->vertexPositions);
                    this->vertex_push (corner_2.plus_one_dim(), this->vertexPositions);
                    for (unsigned int vi = 0; vi < 4; ++vi) {
                        this->vertex_push (_col, this->vertexColors);
                        this->vertex_push (this->uz, this->vertexNormals);
                    }
                    this->indices.push_back (this->idx);
                    this->indices.push_back (this->idx+1);
                    this->indices.push_back (this->idx+2);
                    this->indices.push_back (this->idx+2);
                    this->indices.push_back (this->idx+1);
                    this->indices.push_back (this->idx+3);
                    this->idx += 4;
                }
            }; // end of fill_in_shape()


            // How does one band wavelength project onto the x and y axes?
            float length_of_lambda_in_x = this->lambda / std::cos (morph::mathconst<float>::deg2rad * this->alpha);
            float length_of_lambda_in_y = this->lambda / std::sin (morph::mathconst<float>::deg2rad * this->alpha);

            // p_0 is our starting location to draw bands
            vec<float, 2> p_0 = { 0.0f, 0.0f };
            if (std::abs(length_of_lambda_in_x) > std::abs(dims[0])) {
                // In this case we have horizontal bands, so start from a p_0 on the y axis
                float y_lambdas_f = v_offset[1] / length_of_lambda_in_y;
                float lambdas_y = std::trunc (y_lambdas_f);
                float lambdas_y_offset = lambdas_y * length_of_lambda_in_y;
                p_0[1] = v_offset[1] - lambdas_y_offset;
            } else {
                // Bands are roughly vertical, place p_0 on the x axis
                float x_lambdas_f = v_offset[0] / length_of_lambda_in_x;
                float lambdas_x = std::trunc (x_lambdas_f);
                float lambdas_x_offset = lambdas_x * length_of_lambda_in_x;
                p_0[0] = v_offset[0] - lambdas_x_offset;
            }

            vec<float, 2> dx = dims.length() * u_alpha_perp;

            vec<float, 2> p1 = {0,0}, q1 = {0,0}, p2 = {0,0}, q2 = {0,0};
            vec<float, 2> fp1 = {0,0}, fp2 = {0,0}, fq1 = {0,0}, fq2 = {0,0};

            // Identifiers for the final crossing points.
            border_id fp1_id = border_id::unknown;
            border_id fq1_id = border_id::unknown;
            border_id fp2_id = border_id::unknown;
            border_id fq2_id = border_id::unknown;

            unsigned int i = 0;
            for (vec<float, 2> p = p_0; ; p += 0.5f * lambda * u_alpha) {

                std::cout << "\n\n";
                std::array<float, 3> col = i%2==0 ? colour1 : colour2;

                // Make test lines for start of field
                p1 = p + dx;
                q1 = p - dx;

                std::bitset<2> bi = morph::MathAlgo::segments_intersect (p1, q1, bot_p, bot_q);
                std::bitset<2> ti = morph::MathAlgo::segments_intersect (p1, q1, top_p, top_q);
                std::bitset<2> li = morph::MathAlgo::segments_intersect (p1, q1, left_p, left_q);
                std::bitset<2> ri = morph::MathAlgo::segments_intersect (p1, q1, right_p, right_q);

                if (bi.test(1) || ti.test(1) || li.test(1) || ri.test(1)) {
                    throw std::runtime_error ("Implement me for colinear");
                }

                // Test if we're off the rectangle
                if (!bi.test(0) && !ti.test(0) && !li.test(0) && !ri.test(0)) { break; }

                // From p1, q1 find fp1 and fp1_id
                std::cout << "find_border_points for p1/q1\n";
                find_border_points (p1, q1,  fp1, fq1, fp1_id, fq1_id, bi, ti, li, ri);

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

                // Test if the *second* line of a band is off the rectangle
                if (!bi.test(0) && !ti.test(0) && !li.test(0) && !ri.test(0)) {
                    std::cout << "Fill-in shape for fp1/fq1\n";
                    fill_in_shape (p, fp1, fq1, fp1_id, fq1_id, morph::colour::crimson);
                    break; // we're done (return true if break?
                }

                std::cout << "find_border_points for p2/q2 " << p2 << "/" << q2 << "\n";
                find_border_points (p2, q2,  fp2, fq2, fp2_id, fq2_id, bi, ti, li, ri);

                // sanity check
                if (fp1_id == border_id::unknown || fq1_id == border_id::unknown
                    || fp2_id == border_id::unknown || fq2_id == border_id::unknown) {
                    throw std::runtime_error ("unexpected border_id");
                }

                // We have our band points.
                std::cout << "fp1 to fq1 cross " << border_id_str (fp1_id) << " and " << border_id_str (fq1_id) << " resp.\n";
                std::cout << "fp2 to fq2 cross " << border_id_str (fp2_id) << " and " << border_id_str (fq2_id) << " resp.\n";

                // Does fp1-fp2 intersect with fq1-fq2? (if so triangles will draw badly so swap a pair)
                std::bitset<2> fpi = morph::MathAlgo::segments_intersect (fp1, fp2, fq1, fq2);
                if (fpi.test(0)) {
                    // swap one pair
                    std::cout << "Swapping fp2/fq2 order\n";
                    auto tmp = fq2;
                    auto tmp_id = fq2_id;
                    fq2 = fp2;
                    fq2_id = fp2_id;
                    fp2 = tmp;
                    fp2_id = tmp_id;
                }

                // Determine if fill-in triangles should be drawn
                if (fp1_id != fp2_id) {
                    std::cout << "Fill-in shape for fp1/fp2 (blue)\n";
                    fill_in_shape (p, fp1, fp2, fp1_id, fp2_id, morph::colour::royalblue);
                }

                if (fq1_id != fq2_id) {
                    std::cout << "Fill-in shape for fq1/fq2 (yellow)\n";
                    fill_in_shape (p, fq1, fq2, fq1_id, fq2_id, morph::colour::yellow);
                }

                // Now draw the band
                this->vertex_push (fp1.plus_one_dim(), this->vertexPositions);
                this->vertex_push (fq1.plus_one_dim(), this->vertexPositions);
                this->vertex_push (fp2.plus_one_dim(), this->vertexPositions);
                this->vertex_push (fq2.plus_one_dim(), this->vertexPositions);
                for (unsigned int vi = 0; vi < 4; ++vi) {
                    this->vertex_push (col, this->vertexColors);
                    this->vertex_push (this->uz, this->vertexNormals);
                }
                this->indices.push_back (this->idx);
                this->indices.push_back (this->idx+1);
                this->indices.push_back (this->idx+2);
                this->indices.push_back (this->idx+2);
                this->indices.push_back (this->idx+1);
                this->indices.push_back (this->idx+3);
                this->idx += 4;

                if constexpr (debug_line_points) {
                    this->computeSphere (this->idx, p1.plus_one_dim(), colour1, 0.02f, 16, 20);
                    this->computeSphere (this->idx, q1.plus_one_dim(), colour1, 0.02f, 16, 20);
                    this->computeSphere (this->idx, p2.plus_one_dim(), colour2, 0.02f, 16, 20);
                    this->computeSphere (this->idx, q2.plus_one_dim(), colour2, 0.02f, 16, 20);
                    //std::cout << "showing black spheres at fp1, fq1, fp2, fq2:\n"
                    //          <<  fp1 << " - " << fq1 << "\n" << fp2 << " - " << fq2 << std::endl;
                    this->computeSphere (this->idx, fp1.plus_one_dim(), morph::colour::crimson, 0.01f, 16, 20);
                    this->computeSphere (this->idx, fq1.plus_one_dim(), morph::colour::violetred2, 0.01f, 16, 20);
                    this->computeSphere (this->idx, fp2.plus_one_dim(), morph::colour::royalblue, 0.01f, 16, 20);
                    this->computeSphere (this->idx, fq2.plus_one_dim(), morph::colour::dodgerblue1, 0.01f, 16, 20);
                }

                ++i;
            }

            if constexpr (draw_border) { // Seeing boundary useful for debugging
                this->computeFlatLine (this->idx, bot_p.plus_one_dim(), bot_q.plus_one_dim(),
                                       this->uz, morph::colour::black, 0.01f);
                this->computeFlatLine (this->idx, right_p.plus_one_dim(), right_q.plus_one_dim(),
                                       this->uz, morph::colour::black, 0.01f);
                this->computeFlatLine (this->idx, top_p.plus_one_dim(), top_q.plus_one_dim(),
                                       this->uz, morph::colour::black, 0.01f);
                this->computeFlatLine (this->idx, left_p.plus_one_dim(), left_q.plus_one_dim(),
                                       this->uz, morph::colour::black, 0.01f);
            }
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
        //! Draw boundary for debugging?
        static constexpr bool draw_border = true;
        //! Spheres for line points for debugging?
        static constexpr bool debug_line_points = true;
    };

} // namespace morph
