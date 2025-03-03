/*
 * A visual to label HSV graphs
 */

#pragma once

#include <morph/VisualModel.h>
#include <morph/mathconst.h>
#include <morph/vec.h>
#include <morph/GraphVisual.h>

namespace morph {

    template <typename F, int glver = morph::gl::version_4_1>
    class HSVWheelVisual : public VisualModel<glver>
    {
        using mc = morph::mathconst<F>;
    public:
        //! Constructor
        //! \param _offset The offset within morph::Visual space to place this model
        HSVWheelVisual (const vec<float> _offset)
        {
            this->mv_offset = _offset;
            this->viewmatrix.translate (this->mv_offset);
            // Set default text features
            this->tf.fontsize = 0.05f;
            this->tf.fontres = 48;
            this->tf.colour = this->framecolour;
            // Like graphs, hsv wheels don't rotate by default. If you want yours to, set this false in your client code.
            this->twodimensional = true;
        }

        void setTextColour (const std::array<float, 3>& c)
        {
            this->tf.colour = c;
        }

        void setFrameColour (const std::array<float, 3>& c)
        {
            this->framecolour = c;
        }

        void setColour (const std::array<float, 3>& c)
        {
            this->tf.colour = c;
            this->framecolour = c;
        }

        void initializeVertices()
        {
            // Auto-set ticklabelgap
            auto em = this->make_text_model (this->tf);
            morph::TextGeometry em_geom = em->getTextGeometry (std::string("m"));
            this->ticklabelgap = em_geom.width()/2.0f;
            this->drawFrame();
            this->drawTickLabels();
            this->fillFrameWithColour();
        }

        //! Draw a circular frame around the wheel
        void drawFrame()
        {
            // Draw an approximation of a circle.
            this->computeFlatCircleLine (vec<float>({0,0,this->z}), this->uz, this->radius + this->framelinewidth/2.0f,
                                         this->framelinewidth, this->framecolour, this->numsegs);
        }

        //! Draw the tick labels (the numbers or whatever text the client code has given us)
        void drawTickLabels()
        {
            // Reset these members
            this->ticklabelheight = 0.0f;
            this->ticklabelwidth = 0.0f;

            if (this->label_angles.empty()) {
                // Auto-fill label_angles based on labels size.
                // example order for 4: mc::pi_over_2, mc::pi, mc::three_pi_over_2, 0.0f
                this->label_angles.resize (this->labels.size());
                for (unsigned int i = 0; i < this->labels.size(); ++i) {
                    // North is pi/2, so that's the start:
                    this->label_angles[i] = mc::pi_over_2 + i * (mc::two_pi / this->labels.size());
                    // Rescale any that exceed 2pi:
                    this->label_angles[i] = this->label_angles[i] < F{0} ? this->label_angles[i] + mc::two_pi : this->label_angles[i];
                    this->label_angles[i] = this->label_angles[i] > mc::two_pi ? this->label_angles[i] - mc::two_pi : this->label_angles[i];
                }
            }

            for (unsigned int i = 0; i < this->label_angles.size(); ++i) {
                std::string s = this->labels[i];
                auto lbl = this->make_text_model (this->tf);
                morph::TextGeometry geom = lbl->getTextGeometry (s);
                this->ticklabelheight = geom.height() > this->ticklabelheight ? geom.height() : this->ticklabelheight;
                this->ticklabelwidth = geom.width() > this->ticklabelwidth ? geom.width() : this->ticklabelwidth;
                // Dep. on angle, the additional gap for the text will need to be based on different aspects of the text geometry
                float geom_gap = std::abs(std::cos(label_angles[i]) * geom.half_width()) + std::abs(std::sin(label_angles[i]) * geom.half_height());
                float lbl_r = this->radius + this->framelinewidth + this->ticklabelgap + geom_gap;
                morph::vec<float> lblpos = {
                    lbl_r * std::cos (label_angles[i]) - geom.half_width(),
                    lbl_r * std::sin (label_angles[i]) - geom.half_height(),
                    this->z
                };
                lbl->setupText (s, lblpos+this->mv_offset, this->tf.colour);
                this->texts.push_back (std::move(lbl));
            }
        }

        // Draw the actual HSV stuff
        void fillFrameWithColour()
        {
            vec<float> centre = {0,0,this->z};

            for (int ring = this->numrings; ring > 0; ring--) {

                float r_out = this->radius * static_cast<float>(ring)/this->numrings;
                float r_in = this->radius * static_cast<float>(ring-1)/this->numrings;
                float norm_r_out = r_out / this->radius; // range 0->1
                float norm_r_in = r_in / this->radius;

                for (int j = 0; j < static_cast<int>(this->numsegs); j++) {

                    // The colour will change for each j
                    float colour_angle = (static_cast<float>(j)/this->numsegs) * morph::mathconst<float>::two_pi;
                    std::array<float, 3> col_out = this->cm.convert_angular (colour_angle, norm_r_out);
                    std::array<float, 3> col_in = this->cm.convert_angular (colour_angle, norm_r_in);

                    float t = j * morph::mathconst<float>::two_pi/static_cast<float>(this->numsegs);
                    vec<float> c_in = this->uy * sin(t) * r_in + this->ux * cos(t) * r_in;
                    this->vertex_push (centre+c_in, this->vertexPositions);
                    this->vertex_push (this->uz, this->vertexNormals);
                    this->vertex_push (col_in, this->vertexColors);
                    vec<float> c_out = this->uy * sin(t) * r_out + this->ux * cos(t) * r_out;
                    this->vertex_push (centre+c_out, this->vertexPositions);
                    this->vertex_push (this->uz, this->vertexNormals);
                    this->vertex_push (col_out, this->vertexColors);
                }
                // Added 2*segments vertices to vertexPositions

                for (int j = 0; j < static_cast<int>(this->numsegs); j++) {
                    int jn = (numsegs + ((j+1) % numsegs)) % numsegs;
                    this->indices.push_back (this->idx+(2*j));
                    this->indices.push_back (this->idx+(2*jn));
                    this->indices.push_back (this->idx+(2*jn+1));
                    this->indices.push_back (this->idx+(2*j));
                    this->indices.push_back (this->idx+(2*jn+1));
                    this->indices.push_back (this->idx+(2*j+1));
                }
                this->idx += 2 * this->numsegs; // nverts
            }
        }

        //! The ColourMap to show (copy in). Should be type ColourMapType::HSV
        morph::ColourMap<F> cm;
        //! The radius of the HSVwheel
        float radius = 1.0f;
        //! Position in z in model space. Default is just 0.
        float z = 0.0f;
        //! colour for the axis box/lines. Text colour is in TextFeatures tf.colour
        std::array<float, 3> framecolour = morph::colour::black;
        //! The line width of the frame
        float framelinewidth = 0.01f;
        //! The label strings  that should be displayed. Order the elements anti-clockwise, starting from the 'north' element.
        std::deque<std::string> labels = {"N", "W", "S", "E"};
        //! The positions, as angles for the labels. If empty, these will be auto-computed
        std::deque<F> label_angles = { /*mc::pi_over_2, mc::pi, mc::three_pi_over_2, 0.0f*/ };
        // Stores all the text features for this ColourBar (font, colour, font size, font res)
        morph::TextFeatures tf;
        //! Gap to x axis tick labels. Gets auto-set
        float ticklabelgap = 0.05f;
        //! The number of segments to make in each ring of the colourmap fill
        unsigned int numsegs = 128;
        //! How many rings of colour?
        unsigned int numrings = 64;
    protected:
        //! tick label height
        float ticklabelheight = 0.0f;
        //! tick label width
        float ticklabelwidth = 0.0f;
    };

} // namespace
