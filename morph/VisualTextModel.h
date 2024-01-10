/*!
 * \file
 *
 * Declares a class to hold vertices of the quads that are the backing for a sequence of
 * text characters.
 *
 * \author Seb James
 * \date Oct 2020
 */

#pragma once

#ifndef USE_GLEW
#ifdef __OSX__
# include <OpenGL/gl3.h>
#else
# include <GL3/gl3.h>
#endif
#endif
#include <morph/TransformMatrix.h>
#include <morph/vec.h>
#include <morph/mathconst.h>
#include <morph/gl/util.h>
#include <morph/VisualCommon.h>
#include <morph/unicode.h>
#include <morph/VisualFace.h>
#include <morph/VisualResources.h>
#include <morph/colour.h>
#include <vector>
#include <array>
#include <map>
#include <limits>

namespace morph {

    //! Forward declaration of a Visual class
    template <int, int, bool>
    class Visual;

    /*!
     * A class containing information about a text string, as it would be displayed on
     * the screen in some font, at a given size. The units should be the same as those
     * used to create the quads on which the text will be laid out.
     */
    struct TextGeometry
    {
        //! The sum of the advances of each glyph in the string
        float total_advance = 0.0f;
        //! The maximum extension above the baseline for any glyph in the string
        float max_bearingy = 0.0f;
        //! The maximum ymin - the extension of the lowest part of any glyph, like gpqy, etc.
        float max_drop = 0.0f;
        //! Return half the width of the string
        float half_width() { return this->total_advance * 0.5f; }
        //! The width is the total_advance.
        float width() { return this->total_advance; }
        //! The effective height is the maximum bearingy
        float height() { return this->max_bearingy; }
        //! Half the max_bearingy
        float half_height() { return this->max_bearingy * 0.5f; }
    };

    // A way to bundle up font size, colour, etc into a single object. Constructors chosen for max convenience.
    struct TextFeatures
    {
        TextFeatures(){};
        TextFeatures (const float _fontsize,
                      const int _fontres,
                      const bool _centre_horz,
                      const std::array<float, 3> _colour,
                      morph::VisualFont _font)
            : fontsize(_fontsize), fontres(_fontres), centre_horz(_centre_horz), colour(_colour), font(_font) {}

        TextFeatures (const float _fontsize, const bool _centre_horz = false)
            : fontsize(_fontsize)
        {
            this->centre_horz = _centre_horz;
        }

        TextFeatures (const float _fontsize, const std::array<float, 3> _colour, const bool _centre_horz = false)
            : fontsize(_fontsize), colour(_colour)
        {
            this->centre_horz = _centre_horz;
        }

        TextFeatures (const float _fontsize, const int _fontres, const std::array<float, 3> _colour = morph::colour::black, const bool _centre_horz = false)
            : fontsize(_fontsize), fontres(_fontres), colour(_colour)
        {
            this->centre_horz = _centre_horz;
        }

        float fontsize = 0.1f;
        int fontres = 24;
        bool centre_horz = false;
        std::array<float, 3> colour = morph::colour::black;
        morph::VisualFont font = morph::VisualFont::DVSans;
        // also things like rotate, centre_vert, etc
    };

    /*!
     * A separate data-containing model which is used to render text. It is intended
     * that this could comprise part of a morph::Visual or a morph::VisualModel. It has
     * its own render call.
     */
    template <int glv1, int glv2, bool gles>
    class VisualTextModel
    {
    public:
        /*!
         * Construct with given text shader program id, \a tsp, font \a _font, font
         * scaling factor \a fscale and a spatial \a _mv_offset (model view offset). The
         * text to be displayed is \a txt.
         */
        VisualTextModel (morph::Visual<glv1,glv2,gles>* _vis, GLuint tsp,
                         morph::VisualFont visualfont, float _m_width, int _fontpixels,
                         const morph::vec<float> _mv_offset, const std::string& _txt,
                         std::array<float, 3> _clr = {0,0,0})
        {
            this->parentVis = _vis;
            this->tshaderprog = tsp;

            // This is set with the model view offset of the parent model PLUS the
            // relative location of this VisualTextModel within the parent VisualModel
            // (or Visual).
            this->mv_offset = _mv_offset;

            // The rotation stored inside this text model should be applied BEFORE any of the other operations.
            this->viewmatrix.rotate (this->mv_rotation);

            // Question: At what point to apply translations to this VisualTextModel?
            // Should be acceptable here, because the parent model (or Visual) can
            // ensure that the mv_offset is correct.
            //this->mv_rotation = ?;
            this->viewmatrix.translate (this->mv_offset);
            this->m_width = _m_width;
            this->fontpixels = _fontpixels;
            this->fontscale = _m_width/(float)this->fontpixels;
            this->clr_text = _clr;

            // Set up a face to get characters. Choose font, and pixel size. A suitable
            // pixel size will depend on how large we're going to scale and should
            // probably be determined from this->fontscale.
            this->face = VisualResources<glv1,glv2,gles>::i().getVisualFace (visualfont, this->fontpixels, this->parentVis);
            this->setupText (_txt);
        }

        //! A more compact version of VisualTextModel(GLuint, VisualFont, float, int, const
        //! vec<float>, const string&, array<float, 3>), which takes a TextFeatures object
        VisualTextModel (morph::Visual<glv1,glv2,gles>* _vis,
                         GLuint tsp, const morph::vec<float> _mv_offset,
                         const std::string& _txt, morph::TextFeatures tfeatures)
        {
            this->parentVis = _vis;
            this->tshaderprog = tsp; // Fixme: Could be obtained from parentVis via friends/callbacks?
            this->mv_offset = _mv_offset;
            this->viewmatrix.rotate (this->mv_rotation);
            this->viewmatrix.translate (this->mv_offset);
            this->m_width = tfeatures.fontsize;
            this->fontpixels = tfeatures.fontres;
            this->fontscale = this->m_width/(float)this->fontpixels;
            this->clr_text = tfeatures.colour;
            this->face = VisualResources<glv1,glv2,gles>::i().getVisualFace (tfeatures.font, this->fontpixels, this->parentVis);
            this->setupText (_txt);
        }

        /*!
         * Construct with given text shader program id, \a tsp, font \a _font, font
         * scaling factor \a fscale. After this constructor, the method
         * getTextGeometry() can be called, prior to calling this->setupText (_txt,
         * position, clr);
         */
        VisualTextModel (morph::Visual<glv1,glv2,gles>* _vis,
                         GLuint tsp, morph::VisualFont visualfont, float _m_width, int _fontpixels)
        {
            this->parentVis = _vis;
            this->tshaderprog = tsp;
            this->m_width = _m_width;
            this->fontpixels = _fontpixels;
            this->fontscale = _m_width/(float)this->fontpixels;
            this->face = VisualResources<glv1,glv2,gles>::i().getVisualFace (visualfont, this->fontpixels, this->parentVis);
        }

        //! A more compact version of the VisualTextModel(GLuint, VisualFont, float, int), taking a TextFeatures object.
        VisualTextModel (morph::Visual<glv1,glv2,gles>* _vis,
                         GLuint tsp, morph::TextFeatures tfeatures)
        {
            this->parentVis = _vis;
            this->tshaderprog = tsp;
            this->m_width = tfeatures.fontsize;
            this->fontpixels = tfeatures.fontres;
            this->fontscale = this->m_width/(float)this->fontpixels;
            this->face = VisualResources<glv1,glv2,gles>::i().getVisualFace (tfeatures.font, this->fontpixels, this->parentVis);
        }

        virtual ~VisualTextModel()
        {
            if (this->vbos != nullptr) {
                glDeleteBuffers (numVBO, this->vbos);
                delete[] this->vbos;
                glDeleteVertexArrays (1, &this->vao);
            }
        }

        //! Render the VisualTextModel
        void render()
        {
            if (this->hide == true) { return; }

            GLint prev_shader;
            glGetIntegerv (GL_CURRENT_PROGRAM, &prev_shader);

            // Ensure the correct program is in play for this VisualModel
            glUseProgram (this->tshaderprog);

            // Set uniforms
            GLint loc_tc = glGetUniformLocation (this->tshaderprog, static_cast<const GLchar*>("textColor"));
            if (loc_tc != -1) { glUniform3f (loc_tc, this->clr_text[0], this->clr_text[1], this->clr_text[2]); }
            GLint loc_a = glGetUniformLocation (this->tshaderprog, static_cast<const GLchar*>("alpha"));
            if (loc_a != -1) { glUniform1f (loc_a, this->alpha); }
            GLint loc_v = glGetUniformLocation (this->tshaderprog, static_cast<const GLchar*>("v_matrix"));
            if (loc_v != -1) { glUniformMatrix4fv (loc_v, 1, GL_FALSE, this->scenematrix.mat.data()); }
            GLint loc_m = glGetUniformLocation (this->tshaderprog, static_cast<const GLchar*>("m_matrix"));
            if (loc_m != -1) { glUniformMatrix4fv (loc_m, 1, GL_FALSE, this->viewmatrix.mat.data()); }

            glActiveTexture (GL_TEXTURE0);

            // It is only necessary to bind the vertex array object before rendering
            glBindVertexArray (this->vao);

            for (size_t i = 0; i < quads.size(); ++i) {
                // Bind the right texture for the quad.
                glBindTexture (GL_TEXTURE_2D, this->quad_ids[i]);
                // This is 'draw a subset of the elements from the vertex array
                // object'. You say how many indices to draw and which base *vertex* you
                // start from. In my scheme, I have 4 vertices for each two triangles
                // that are constructed. Thus, I draw 6 indices, but increment the base
                // vertex by 4 for each letter.
                glDrawElementsBaseVertex (GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, 4*i);
            }

            glBindVertexArray(0);
            glUseProgram (prev_shader);

            morph::gl::Util::checkError (__FILE__, __LINE__);
        }

        //! Set clr_text to a value suitable to be visible on the background colour bgcolour
        void setVisibleOn (const std::array<float, 4>& bgcolour)
        {
            float factor = 0.85f;
            this->clr_text = {1.0f-bgcolour[0] * factor,
                              1.0f-bgcolour[1] * factor,
                              1.0f-bgcolour[2] * factor};
        }

        //! Setter for VisualTextModel::viewmatrix, the model view
        void setViewMatrix (const TransformMatrix<float>& mv) { this->viewmatrix = mv; }

        //! Setter for VisualTextModel::scenematrix, the scene view
        void setSceneMatrix (const TransformMatrix<float>& sv) { this->scenematrix = sv; }

        //! Set the translation specified by \a v0 into the scene translation
        void setSceneTranslation (const vec<float>& v0)
        {
            this->sv_offset = v0;
            this->scenematrix.setToIdentity();
            this->scenematrix.translate (this->sv_offset);
            this->scenematrix.rotate (this->sv_rotation);
        }

        //! Set a translation (only) into the scene view matrix
        void addSceneTranslation (const vec<float>& v0)
        {
            this->sv_offset += v0;
            this->scenematrix.translate (v0);
        }

        //! Set a rotation (only) into the scene view matrix
        void setSceneRotation (const Quaternion<float>& r)
        {
            this->sv_rotation = r;
            this->scenematrix.setToIdentity();
            //std::cout << "Translate by sv_offset: "  << sv_offset << std::endl;
            this->scenematrix.translate (this->sv_offset);
            //std::cout << "Rotate by sv_rotn: "  << sv_rotation << std::endl;
            this->scenematrix.rotate (this->sv_rotation);
        }

        //! Add a rotation to the scene view matrix
        void addSceneRotation (const Quaternion<float>& r)
        {
            this->sv_rotation.premultiply (r);
            this->scenematrix.rotate (r);
        }

        //! Set a translation to the model view matrix
        void setViewTranslation (const vec<float>& v0)
        {
            this->mv_offset = v0;
            this->viewmatrix.setToIdentity();
            this->viewmatrix.translate (this->mv_offset);
            this->viewmatrix.rotate (this->mv_rotation);
        }

        //! Add a translation to the model view matrix
        void addViewTranslation (const vec<float>& v0)
        {
            this->mv_offset += v0;
            this->viewmatrix.translate (v0);
        }

        //! Set a rotation (only) into the model view matrix
        void setViewRotation (const Quaternion<float>& r)
        {
            this->mv_rotation = r;
            this->viewmatrix.setToIdentity();
            // Confirms that mv_offset contains the additional model offset
            //std::cout << "VTM::setViewRotation: setting mv_offset " << mv_offset << std::endl;
            this->viewmatrix.translate (this->mv_offset);
            //std::cout << "VTM::setViewRotation: rotating mv_rotation " << mv_rotation << std::endl;
            this->viewmatrix.rotate (this->mv_rotation);
        }

        //! Apply a further rotation to the model view matrix
        void addViewRotation (const Quaternion<float>& r)
        {
            this->mv_rotation.premultiply (r);
            this->viewmatrix.rotate (r);
        }

        //! Compute the geometry for a sample text.
        morph::TextGeometry getTextGeometry (const std::string& _txt) const
        {
            // First convert string from ASCII/UTF-8 into Unicode.
            std::basic_string<char32_t> utxt = morph::unicode::fromUtf8(_txt);
            morph::TextGeometry geom;
            for (std::basic_string<char32_t>::const_iterator c = utxt.begin(); c != utxt.end(); c++) {
                morph::visgl::CharInfo ci = this->face->glchars[*c];
                float drop = (ci.size.y() - ci.bearing.y()) * this->fontscale;
                geom.max_drop = (drop > geom.max_drop) ? drop : geom.max_drop;
                float bearingy = ci.bearing.y() * this->fontscale;
                geom.max_bearingy = (bearingy > geom.max_bearingy) ? bearingy : geom.max_bearingy;
                geom.total_advance += ((ci.advance>>6)*this->fontscale);
            }
            return geom;
        }

        //! Return the geometry for the stored txt
        morph::TextGeometry getTextGeometry() const
        {
            morph::TextGeometry geom;
            for (std::basic_string<char32_t>::const_iterator c = this->txt.begin(); c != this->txt.end(); c++) {
                morph::visgl::CharInfo ci = this->face->glchars[*c];
                float drop = (ci.size.y() - ci.bearing.y()) * this->fontscale;
                geom.max_drop = (drop > geom.max_drop) ? drop : geom.max_drop;
                float bearingy = ci.bearing.y() * this->fontscale;
                geom.max_bearingy = (bearingy > geom.max_bearingy) ? bearingy : geom.max_bearingy;
                geom.total_advance += ((ci.advance>>6)*this->fontscale);
            }
            return geom;
        }

        //! Set up a new text at a given position, with the given colour.
        void setupText (const std::string& _txt,
                        const morph::vec<float> _mv_offset, std::array<float, 3> _clr = {0,0,0})
        {
            this->mv_offset = _mv_offset;
            this->viewmatrix.translate (this->mv_offset);
            this->clr_text = _clr;
            this->setupText (_txt);
        }

        //! Set up a new text at a given position, with the given colour and a pre-rotation
        void setupText (const std::string& _txt,
                        const morph::Quaternion<float>& _rotation, const morph::vec<float> _mv_offset,
                        std::array<float, 3> _clr = {0,0,0})
        {
            this->mv_rotation = _rotation;
            this->viewmatrix.rotate (this->mv_rotation);
            this->mv_offset = _mv_offset;
            this->viewmatrix.translate (this->mv_offset);
            this->clr_text = _clr;
            this->setupText (_txt);
        }

        void setupText (const std::string& _txt)
        {
            // Convert std::string _txt to std::basic_string<uchar32_t> text and call the other setupText
            this->setupText (morph::unicode::fromUtf8 (_txt));
        }

        static constexpr bool debug_textquads = false;

        //! With the given text and font size information, create the quads for the text.
        void setupText (const std::basic_string<char32_t>& _txt)
        {
            this->txt = _txt;
            // With glyph information from txt, set up this->quads.
            this->quads.clear();
            this->quad_ids.clear();
            // Our string of letters starts at this location
            float letter_pos = 0.0f;
            float letter_y = 0.0f;
            float text_epsilon = 0.0f;
            for (std::basic_string<char32_t>::const_iterator c = this->txt.begin(); c != this->txt.end(); c++) {

                if (*c == '\n') {
                    // Skip newline, but add a y offset and reset letter_pos
                    letter_pos = 0.0f;
                    morph::visgl::CharInfo ch = this->face->glchars['h'];
                    letter_y += this->line_spacing * -ch.size.y() * this->fontscale;
                    continue;
                }

                // Add a quad to this->quads
                morph::visgl::CharInfo ci = this->face->glchars[*c];

                float xpos = letter_pos + ci.bearing.x() * this->fontscale;
                float ypos = letter_y /*this->mv_offset[1]*/ - (ci.size.y() - ci.bearing.y()) * this->fontscale;
                float w = ci.size.x() * this->fontscale;
                float h = ci.size.y() * this->fontscale;

                // Update extents
                if (xpos < this->extents[0]) { this->extents[0] = xpos; } // left
                if (xpos+w > this->extents[1]) { this->extents[1] = xpos+w; } // right
                if (ypos < this->extents[2]) { this->extents[2] = ypos; } // bottom
                if (ypos+h > this->extents[3]) { this->extents[3] = ypos+h; } // top

                // What's the order of the vertices for the quads? It is:
                // Bottom left, Top left, top right, bottom right.
                std::array<float,12> tbox = { xpos,   ypos,     /*this->mv_offset[2]+*/text_epsilon,
                                              xpos,   ypos+h,   text_epsilon,
                                              xpos+w, ypos+h,   text_epsilon,
                                              xpos+w, ypos,     text_epsilon };
                text_epsilon -= 10.0f*std::numeric_limits<float>::epsilon();
                if constexpr (debug_textquads == true) {
                    std::cout << "Text box added as quad from\n("
                              << tbox[0] << "," << tbox[1] << "," << tbox[2]
                              << ") to (" << tbox[3] << "," << tbox[4] << "," << tbox[5]
                              << ") to (" << tbox[6] << "," << tbox[7] << "," << tbox[8]
                              << ") to (" << tbox[9] << "," << tbox[10] << "," << tbox[11]
                              << "). w="<<w<<", h="<<h<<"\n";
                    std::cout << "Texture ID for that character is: " << ci.textureID << std::endl;
                }
                this->quads.push_back (tbox);
                this->quad_ids.push_back (ci.textureID);

                // The value in ci.advance has to be divided by 64 to bring it into the
                // same units as the ci.size and ci.bearing values.
                letter_pos += ((ci.advance>>6)*this->fontscale);
            }

            //std::cout << "After setupText, extents are: (LRBT): " << this->extents << std::endl;

            // Ensure we've cleared out vertex info
            this->vertexPositions.clear();
            this->vertexNormals.clear();
            this->vertexColors.clear();
            this->vertexTextures.clear();
            this->indices.clear();

            this->initializeVertices();

            this->postVertexInit();
        }

        float width() const { return this->extents[1] - this->extents[0]; }
        float height() const { return this->extents[3] - this->extents[2]; }

    protected:
        //! Initialize the vertices that will represent the Quads.
        void initializeVertices() {

            unsigned int nquads = this->quads.size();

            for (unsigned int qi = 0; qi < nquads; ++qi) {

                std::array<float, 12> quad = this->quads[qi];

                if constexpr (debug_textquads == true) {
                    std::cout << "Quad box from (" << quad[0] << "," << quad[1] << "," << quad[2]
                              << ") to (" << quad[3] << "," << quad[4] << "," << quad[5]
                              << ") to (" << quad[6] << "," << quad[7] << "," << quad[8]
                              << ") to (" << quad[9] << "," << quad[10] << "," << quad[11] << ")" << std::endl;
                }

                this->vertex_push (quad[0], quad[1],  quad[2],  this->vertexPositions); //1
                this->vertex_push (quad[3], quad[4],  quad[5],  this->vertexPositions); //2
                this->vertex_push (quad[6], quad[7],  quad[8],  this->vertexPositions); //3
                this->vertex_push (quad[9], quad[10], quad[11], this->vertexPositions); //4

                // Add the info for drawing the textures on the quads
                this->vertex_push (0.0f, 1.0f, 0.0f, this->vertexTextures);
                this->vertex_push (0.0f, 0.0f, 0.0f, this->vertexTextures);
                this->vertex_push (1.0f, 0.0f, 0.0f, this->vertexTextures);
                this->vertex_push (1.0f, 1.0f, 0.0f, this->vertexTextures);

                // All same colours
                this->vertex_push (this->clr_backing, this->vertexColors);
                this->vertex_push (this->clr_backing, this->vertexColors);
                this->vertex_push (this->clr_backing, this->vertexColors);
                this->vertex_push (this->clr_backing, this->vertexColors);

                // All same normals
                this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
                this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
                this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
                this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);

                // Two triangles per quad
                // qi * 4 + 1, 2 3 or 4
                GLuint ib = (GLuint)qi*4;
                this->indices.push_back (ib++); // 0
                this->indices.push_back (ib++); // 1
                this->indices.push_back (ib);   // 2

                this->indices.push_back (ib++); // 2
                this->indices.push_back (ib);   // 3
                ib -= 3;
                this->indices.push_back (ib);   // 0
            }
        }

        //! Common code to call after the vertices have been set up.
        void postVertexInit()
        {
            if (this->vbos == nullptr) {
                // Create vertex array object
#ifdef __MACS_HAD_OPENGL_450__
                glCreateVertexArrays (1, &this->vao); // OpenGL 4.5 only
#else
                glGenVertexArrays (1, &this->vao); // Safe for OpenGL 4.4-
#endif
                morph::gl::Util::checkError (__FILE__, __LINE__);
            }

            glBindVertexArray (this->vao);
            morph::gl::Util::checkError (__FILE__, __LINE__);

            if (this->vbos == nullptr) {
                // Create the vertex buffer objects
                this->vbos = new GLuint[numVBO];
#ifdef __MACS_HAD_OPENGL_450__
                glCreateBuffers (numVBO, this->vbos); // OpenGL 4.5 only
#else
                glGenBuffers (numVBO, this->vbos); // OpenGL 4.4- safe
#endif
                morph::gl::Util::checkError (__FILE__, __LINE__);
            }

            // Set up the indices buffer - bind and buffer the data in this->indices
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vbos[idxVBO]);
            morph::gl::Util::checkError (__FILE__, __LINE__);

            //std::cout << "indices.size(): " << this->indices.size() << std::endl;
            int sz = this->indices.size() * sizeof(GLuint);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sz, this->indices.data(), GL_STATIC_DRAW);
            morph::gl::Util::checkError (__FILE__, __LINE__);

            // Binds data from the "C++ world" to the OpenGL shader world for
            // "position", "normalin" and "color"
            // (bind, buffer and set vertex array object attribute)
            this->setupVBO (this->vbos[posnVBO], this->vertexPositions, visgl::posnLoc);
            this->setupVBO (this->vbos[normVBO], this->vertexNormals, visgl::normLoc);
            this->setupVBO (this->vbos[colVBO], this->vertexColors, visgl::colLoc);
            this->setupVBO (this->vbos[textureVBO], this->vertexTextures, visgl::textureLoc);

#ifdef CAREFULLY_UNBIND_AND_REBIND
            // Possibly release (unbind) the vertex buffers, but have to unbind vertex
            // array object first.
            glBindVertexArray(0);
            morph::gl::Util::checkError (__FILE__, __LINE__);
#endif
        }

    public:
        //! The colour of the text
        std::array<float, 3> clr_text = {0.0f, 0.0f, 0.0f};
        //! Line spacing, in multiples of the height of an 'h'
        float line_spacing = 1.4f;
        //! Parent Visual
        morph::Visual<glv1,glv2,gles>* parentVis = nullptr;
    protected:
        //! A face for this text
        morph::gl::VisualFace* face = nullptr;
        //! The colour of the backing quad's vertices. Doesn't have any effect.
        std::array<float, 3> clr_backing = {1.0f, 1.0f, 0.0f};
        //! the desired width of an 'm'.
        float m_width = 1.0f;
        //! A scaling factor based on the desired width of an 'm'
        float fontscale = 1.0f;
        //! How many pixels in the font? Depends on m_width. Should also depend on 'the
        //! proportion of the screen that an 'm' will subtend' or 'the width in screen
        //! pixels that an 'm' takes up.
        int fontpixels = 100;

        //! model-view offset within the scene. Any model-view offset of the parent
        //! object should be incorporated into this offset. That is, if this
        //! VisualTextModel is the letter 'x' within a CoordArrows VisualModel, then the
        //! model-view offset here should be the CoordArrows model-view offset PLUS the
        //! length of the CoordArrow x axis length.
        vec<float> mv_offset;
        //! The model-view rotation of this text object. mv_offset and mv_rotation are
        //! together used to compute viewmatrix. Keep a copy so that it is easy to reset
        //! the viewmatrix and recompute it with either a new offset or a new rotation.
        Quaternion<float> mv_rotation;

        //! A rotation of the parent model
        Quaternion<float> parent_rotation;

        //! Scene view offset
        vec<float> sv_offset;
        //! Scene view rotation
        Quaternion<float> sv_rotation;
        //! The text-model-specific view matrix and a scene matrix
        TransformMatrix<float> viewmatrix;
        //! Before, I wrote: We protect the scene matrix as updating it with the parent
        //! model's scene matrix likely involves also adding an additional
        //! translation. Now, I'm still slightly confused as to whether I *need* to have a
        //! copy of the scenematrix *here*.
        TransformMatrix<float> scenematrix;

        //! The text string stored for debugging
        std::basic_string<char32_t> txt;
        //! The Quads that form the 'medium' for the text textures. 12 float = 4 corners
        std::vector<std::array<float,12>> quads;
        //! left, right, top and bottom extents of the text for this
        //! VisualTextModel. setupText should modify these as it sets up quads. Order of
        //! numbers is left, right, bottom, top
        vec<float, 4> extents = { 1e7, -1e7, 1e7, -1e7 };
        //! The texture ID for each quad - so that we draw the right texture image over each quad.
        std::vector<unsigned int> quad_ids;
        //! Position within vertex buffer object (if I use an array of VBO)
        enum VBOPos { posnVBO, normVBO, colVBO, idxVBO, textureVBO, numVBO };
        //! A copy of the reference to the text shader program
        GLuint tshaderprog;
        //! The OpenGL Vertex Array Object
        GLuint vao;
        //! Single vbo to use as in example
        GLuint vbo;
        //! Vertex Buffer Objects stored in an array
        GLuint* vbos = nullptr;
        //! CPU-side data for indices
        std::vector<GLuint> indices;
        //! CPU-side data for quad vertex positions
        std::vector<float> vertexPositions;
        //! CPU-side data for quad vertex normals
        std::vector<float> vertexNormals;
        //! CPU-side data for vertex colours
        std::vector<float> vertexColors;
        //! data for textures
        std::vector<float> vertexTextures;
        //! A model-wide alpha value for the shader
        float alpha = 1.0f;
        //! If true, then calls to VisualModel::render should return
        bool hide = false;

        //! Set up a vertex buffer object - bind, buffer and set vertex array object attribute
        void setupVBO (GLuint& buf, std::vector<float>& dat, unsigned int bufferAttribPosition)
        {
            int sz = dat.size() * sizeof(float);
            glBindBuffer (GL_ARRAY_BUFFER, buf);
            glBufferData (GL_ARRAY_BUFFER, sz, dat.data(), GL_STATIC_DRAW);
            glVertexAttribPointer (bufferAttribPosition, 3, GL_FLOAT, GL_FALSE, 0, (void*)(0));
            glEnableVertexAttribArray (bufferAttribPosition);
        }

        //! Push three floats onto the vector of floats \a vp
        void vertex_push (const float& x, const float& y, const float& z, std::vector<float>& vp)
        {
            vp.push_back (x);
            vp.push_back (y);
            vp.push_back (z);
        }
        //! Push array of 3 floats onto the vector of floats \a vp
        void vertex_push (const std::array<float, 3>& arr, std::vector<float>& vp)
        {
            vp.push_back (arr[0]);
            vp.push_back (arr[1]);
            vp.push_back (arr[2]);
        }
        //! Push morph::vec of 3 floats onto the vector of floats \a vp
        void vertex_push (const vec<float>& vec, std::vector<float>& vp)
        {
            std::copy (vec.begin(), vec.end(), std::back_inserter (vp));
        }
    };

} // namespace morph
