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

#include <morph/VisualTextModelBase.h>

#if defined __gl3_h_ || defined __gl_h_
// GL headers have been externally included
#else
# error "GL headers should have been included already"
#endif

#include <morph/gl/util.h>
#include <morph/VisualFace.h>
#include <morph/VisualResources.h>

namespace morph {

    //! Forward declaration of a VisualBase class
    template <int> class VisualBase;

    /*!
     * A separate data-containing model which is used to render text. It is intended
     * that this could comprise part of a morph::Visual or a morph::VisualModel. It has
     * its own render call.
     */
    template <int glad_type = 0, int glver = morph::gl::version_4_1>
    struct VisualTextModelImpl : public morph::VisualTextModelBase<glver>
    {
        VisualTextModelImpl (morph::TextFeatures _tfeatures)
            : VisualTextModelBase<glver>::VisualTextModelBase (_tfeatures) {}

        virtual ~VisualTextModelImpl()
        {
            if (this->vbos != nullptr) {
                glDeleteBuffers (this->numVBO, this->vbos.get());
                glDeleteVertexArrays (1, &this->vao);
            }
        }

        //! Render the VisualTextModel
        void render() final
        {
            if (this->hide == true) { return; }

            GLint prev_shader;
            GLuint tshaderprog = this->get_tprog (this->parentVis);

            glGetIntegerv (GL_CURRENT_PROGRAM, &prev_shader);

            // Ensure the correct program is in play for this VisualModel
            glUseProgram (tshaderprog);

            // Set uniforms
            GLint loc_tc = glGetUniformLocation (tshaderprog, static_cast<const GLchar*>("textColor"));
            if (loc_tc != -1) { glUniform3f (loc_tc, this->clr_text[0], this->clr_text[1], this->clr_text[2]); }
            GLint loc_a = glGetUniformLocation (tshaderprog, static_cast<const GLchar*>("alpha"));
            if (loc_a != -1) { glUniform1f (loc_a, this->alpha); }
            GLint loc_v = glGetUniformLocation (tshaderprog, static_cast<const GLchar*>("v_matrix"));
            if (loc_v != -1) { glUniformMatrix4fv (loc_v, 1, GL_FALSE, this->scenematrix.mat.data()); }
            GLint loc_m = glGetUniformLocation (tshaderprog, static_cast<const GLchar*>("m_matrix"));
            if (loc_m != -1) { glUniformMatrix4fv (loc_m, 1, GL_FALSE, this->viewmatrix.mat.data()); }

            glActiveTexture (GL_TEXTURE0);

            // It is only necessary to bind the vertex array object before rendering
            glBindVertexArray (this->vao);

            // We have a max of (2^32)-1 characters. Should be enough.
            for (unsigned int i = 0U; i < this->quads.size(); ++i) {
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

        //! Compute the geometry for a sample text.
        morph::TextGeometry getTextGeometry (const std::string& _txt) final
        {
            morph::TextGeometry geom;

            if (this->face == nullptr) {
                this->face = VisualResources<glver>::i().getVisualFace (this->tfeatures, this->parentVis);
            }

            // First convert string from ASCII/UTF-8 into Unicode.
            std::basic_string<char32_t> utxt = morph::unicode::fromUtf8(_txt);
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
        morph::TextGeometry getTextGeometry() final
        {
            morph::TextGeometry geom;

            if (this->face == nullptr) {
                this->face = VisualResources<glver>::i().getVisualFace (this->tfeatures, this->parentVis);
            }

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

        //! For some reason, I can't place these setupText functions in the base class. Compiler
        //! gets confused wtih std::string aka std::__cxx11::basic_string<char> and
        //! std::__cxx11::basic_string<char32_t>
        //!{
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
                        const morph::quaternion<float>& _rotation, const morph::vec<float> _mv_offset,
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
        //!}

        //! With the given text and font size information, create the quads for the text.
        void setupText (const std::basic_string<char32_t>& _txt)
        {
            if (this->face == nullptr) {
                this->face = VisualResources<glver>::i().getVisualFace (this->tfeatures, this->parentVis);
            }

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
                text_epsilon -= 10.0f * std::numeric_limits<float>::epsilon();
                if constexpr (morph::VisualTextModelBase<glver>::debug_textquads == true) {
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

    protected:

        //! Common code to call after the vertices have been set up.
        void postVertexInit() final
        {
            if (this->vbos == nullptr) {
                // Create vertex array object
                glGenVertexArrays (1, &this->vao); // Safe for OpenGL 4.4-
            }

            glBindVertexArray (this->vao);

            if (this->vbos == nullptr) {
                // Create the vertex buffer objects
                this->vbos = std::make_unique<GLuint[]>(this->numVBO);
                glGenBuffers (this->numVBO, this->vbos.get()); // OpenGL 4.4- safe
            }

            // Set up the indices buffer - bind and buffer the data in this->indices
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vbos[this->idxVBO]);

            //std::cout << "indices.size(): " << this->indices.size() << std::endl;
            std::size_t sz = this->indices.size() * sizeof(GLuint);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sz, this->indices.data(), GL_STATIC_DRAW);

            // Binds data from the "C++ world" to the OpenGL shader world for
            // "position", "normalin" and "color"
            // (bind, buffer and set vertex array object attribute)
            this->setupVBO (this->vbos[this->posnVBO], this->vertexPositions, visgl::posnLoc);
            this->setupVBO (this->vbos[this->normVBO], this->vertexNormals, visgl::normLoc);
            this->setupVBO (this->vbos[this->colVBO], this->vertexColors, visgl::colLoc);
            this->setupVBO (this->vbos[this->textureVBO], this->vertexTextures, visgl::textureLoc);

            glBindVertexArray(0); // carefully unbind
        }

        //! A face for this text. The face is specfied by tfeatures.font
        morph::visgl::VisualFace* face = nullptr;

        //! Set up a vertex buffer object - bind, buffer and set vertex array object attribute
        void setupVBO (GLuint& buf, std::vector<float>& dat, unsigned int bufferAttribPosition) final
        {
            std::size_t sz = dat.size() * sizeof(float);
            glBindBuffer (GL_ARRAY_BUFFER, buf);
            glBufferData (GL_ARRAY_BUFFER, sz, dat.data(), GL_STATIC_DRAW);
            glVertexAttribPointer (bufferAttribPosition, 3, GL_FLOAT, GL_FALSE, 0, (void*)(0));
            glEnableVertexAttribArray (bufferAttribPosition);
        }
    };

} // namespace morph
