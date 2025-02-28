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

#if defined __gl3_h_ || defined __gl_h_
// GL headers have been externally included
#else
# error "GL headers should have been included already"
#endif

#include <morph/gl/version.h>
#include <morph/quaternion.h>
#include <morph/mat44.h>
#include <morph/vec.h>
#include <morph/mathconst.h>
#include <morph/gl/util.h>
#include <morph/VisualCommon.h>
#include <morph/unicode.h>
#include <morph/TextGeometry.h>
#include <morph/TextFeatures.h>
#include <morph/VisualFace.h>
#include <morph/VisualResources.h>
#include <morph/colour.h>
#include <vector>
#include <array>
#include <map>
#include <limits>
#include <memory>

namespace morph {

    //! Forward declaration of a Visual class
    template <int>
    class Visual;

    /*!
     * A separate data-containing model which is used to render text. It is intended
     * that this could comprise part of a morph::Visual or a morph::VisualModel. It has
     * its own render call.
     */
    template <int glver = morph::gl::version_4_1>
    class VisualTextModel
    {
    public:
        //! Pass just the TextFeatures. parentVis, tshader etc, accessed by callbacks
        VisualTextModel (morph::TextFeatures _tfeatures)
        {
            this->tfeatures = _tfeatures;
            this->fontscale = tfeatures.fontsize / static_cast<float>(tfeatures.fontres);
        }

        virtual ~VisualTextModel()
        {
            if (this->vbos != nullptr) {
#ifdef GLAD_OPTION_GL_MX
                this->get_glfn(this->parentVis)->DeleteBuffers (numVBO, this->vbos.get());
                this->get_glfn(this->parentVis)->DeleteVertexArrays (1, &this->vao);
#else
                glDeleteBuffers (numVBO, this->vbos.get());
                glDeleteVertexArrays (1, &this->vao);
#endif
            }
        }

        //! Render the VisualTextModel
        void render()
        {
            if (this->hide == true) { return; }

            GLint prev_shader;
            GLuint tshaderprog = this->get_tprog (this->parentVis);
#ifdef GLAD_OPTION_GL_MX
            auto _glfn = this->get_glfn (this->parentVis);

            _glfn->GetIntegerv (GL_CURRENT_PROGRAM, &prev_shader);

            // Ensure the correct program is in play for this VisualModel
            _glfn->UseProgram (tshaderprog);

            // Set uniforms
            GLint loc_tc = _glfn->GetUniformLocation (tshaderprog, static_cast<const GLchar*>("textColor"));
            if (loc_tc != -1) { _glfn->Uniform3f (loc_tc, this->clr_text[0], this->clr_text[1], this->clr_text[2]); }
            GLint loc_a = _glfn->GetUniformLocation (tshaderprog, static_cast<const GLchar*>("alpha"));
            if (loc_a != -1) { _glfn->Uniform1f (loc_a, this->alpha); }
            GLint loc_v = _glfn->GetUniformLocation (tshaderprog, static_cast<const GLchar*>("v_matrix"));
            if (loc_v != -1) { _glfn->UniformMatrix4fv (loc_v, 1, GL_FALSE, this->scenematrix.mat.data()); }
            GLint loc_m = _glfn->GetUniformLocation (tshaderprog, static_cast<const GLchar*>("m_matrix"));
            if (loc_m != -1) { _glfn->UniformMatrix4fv (loc_m, 1, GL_FALSE, this->viewmatrix.mat.data()); }

            _glfn->ActiveTexture (GL_TEXTURE0);

            // It is only necessary to bind the vertex array object before rendering
            _glfn->BindVertexArray (this->vao);

            // We have a max of (2^32)-1 characters. Should be enough.
            for (unsigned int i = 0U; i < quads.size(); ++i) {
                // Bind the right texture for the quad.
                _glfn->BindTexture (GL_TEXTURE_2D, this->quad_ids[i]);
                // This is 'draw a subset of the elements from the vertex array
                // object'. You say how many indices to draw and which base *vertex* you
                // start from. In my scheme, I have 4 vertices for each two triangles
                // that are constructed. Thus, I draw 6 indices, but increment the base
                // vertex by 4 for each letter.
                _glfn->DrawElementsBaseVertex (GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, 4*i);
            }

            _glfn->BindVertexArray(0);
            _glfn->UseProgram (prev_shader);

            morph::gl::Util::checkError (__FILE__, __LINE__, _glfn);
#else
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
            for (unsigned int i = 0U; i < quads.size(); ++i) {
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
#endif
        }

        //! Set clr_text to a value suitable to be visible on the background colour bgcolour
        void setVisibleOn (const std::array<float, 4>& bgcolour)
        {
            constexpr float factor = 0.85f;
            this->clr_text = {1.0f - bgcolour[0] * factor, 1.0f - bgcolour[1] * factor, 1.0f - bgcolour[2] * factor};
        }

        //! Setter for VisualTextModel::viewmatrix, the model view
        void setViewMatrix (const mat44<float>& mv) { this->viewmatrix = mv; }

        //! Setter for VisualTextModel::scenematrix, the scene view
        void setSceneMatrix (const mat44<float>& sv) { this->scenematrix = sv; }

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
        void setSceneRotation (const quaternion<float>& r)
        {
            this->sv_rotation = r;
            this->scenematrix.setToIdentity();
            //std::cout << "Translate by sv_offset: "  << sv_offset << std::endl;
            this->scenematrix.translate (this->sv_offset);
            //std::cout << "Rotate by sv_rotn: "  << sv_rotation << std::endl;
            this->scenematrix.rotate (this->sv_rotation);
        }

        //! Add a rotation to the scene view matrix
        void addSceneRotation (const quaternion<float>& r)
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
        void setViewRotation (const quaternion<float>& r)
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
        void addViewRotation (const quaternion<float>& r)
        {
            this->mv_rotation.premultiply (r);
            this->viewmatrix.rotate (r);
        }

        //! Compute the geometry for a sample text.
        morph::TextGeometry getTextGeometry (const std::string& _txt)
        {
            morph::TextGeometry geom;
#ifdef GLAD_OPTION_GL_MX
            if (!this->get_glfn) { return geom; }
            if (this->face == nullptr) {
                this->face = VisualResources<glver>::i().getVisualFace (this->tfeatures, this->parentVis,
                                                                        this->get_glfn(this->parentVis));
            }
#else
            if (this->face == nullptr) {
                this->face = VisualResources<glver>::i().getVisualFace (tfeatures, this->parentVis);
            }
#endif
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
        morph::TextGeometry getTextGeometry()
        {
            morph::TextGeometry geom;
#ifdef GLAD_OPTION_GL_MX
            if (!this->get_glfn) { return geom; }
            if (this->face == nullptr) {
                this->face = VisualResources<glver>::i().getVisualFace (this->tfeatures, this->parentVis,
                                                                        this->get_glfn(this->parentVis));
            }
#else
            if (this->face == nullptr) {
                this->face = VisualResources<glver>::i().getVisualFace (tfeatures, this->parentVis);
            }
#endif
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

        static constexpr bool debug_textquads = false;

        //! With the given text and font size information, create the quads for the text.
        void setupText (const std::basic_string<char32_t>& _txt)
        {
#ifdef GLAD_OPTION_GL_MX
            if (this->face == nullptr) {
                this->face = VisualResources<glver>::i().getVisualFace (this->tfeatures, this->parentVis,
                                                                        this->get_glfn(this->parentVis));
            }
#else
            if (this->face == nullptr) {
                this->face = VisualResources<glver>::i().getVisualFace (tfeatures, this->parentVis);
            }
#endif
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

            unsigned int nquads = static_cast<unsigned int>(this->quads.size());

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
#ifdef GLAD_OPTION_GL_MX
            auto _glfn = this->get_glfn (this->parentVis);
            if (this->vbos == nullptr) {
                // Create vertex array object
                _glfn->GenVertexArrays (1, &this->vao); // Safe for OpenGL 4.4-
            }

            _glfn->BindVertexArray (this->vao);

            if (this->vbos == nullptr) {
                // Create the vertex buffer objects
                this->vbos = std::make_unique<GLuint[]>(numVBO);
                _glfn->GenBuffers (numVBO, this->vbos.get()); // OpenGL 4.4- safe
            }

            // Set up the indices buffer - bind and buffer the data in this->indices
            _glfn->BindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vbos[idxVBO]);

            //std::cout << "indices.size(): " << this->indices.size() << std::endl;
            std::size_t sz = this->indices.size() * sizeof(GLuint);
            _glfn->BufferData(GL_ELEMENT_ARRAY_BUFFER, sz, this->indices.data(), GL_STATIC_DRAW);

            // Binds data from the "C++ world" to the OpenGL shader world for
            // "position", "normalin" and "color"
            // (bind, buffer and set vertex array object attribute)
            this->setupVBO (this->vbos[posnVBO], this->vertexPositions, visgl::posnLoc);
            this->setupVBO (this->vbos[normVBO], this->vertexNormals, visgl::normLoc);
            this->setupVBO (this->vbos[colVBO], this->vertexColors, visgl::colLoc);
            this->setupVBO (this->vbos[textureVBO], this->vertexTextures, visgl::textureLoc);

# ifdef CAREFULLY_UNBIND_AND_REBIND
            // Possibly release (unbind) the vertex buffers, but have to unbind vertex
            // array object first.
            _glfn->BindVertexArray(0);
# endif
#else
            if (this->vbos == nullptr) {
                // Create vertex array object
                glGenVertexArrays (1, &this->vao); // Safe for OpenGL 4.4-
            }

            glBindVertexArray (this->vao);

            if (this->vbos == nullptr) {
                // Create the vertex buffer objects
                this->vbos = std::make_unique<GLuint[]>(numVBO);
                glGenBuffers (numVBO, this->vbos.get()); // OpenGL 4.4- safe
            }

            // Set up the indices buffer - bind and buffer the data in this->indices
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vbos[idxVBO]);

            //std::cout << "indices.size(): " << this->indices.size() << std::endl;
            std::size_t sz = this->indices.size() * sizeof(GLuint);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sz, this->indices.data(), GL_STATIC_DRAW);

            // Binds data from the "C++ world" to the OpenGL shader world for
            // "position", "normalin" and "color"
            // (bind, buffer and set vertex array object attribute)
            this->setupVBO (this->vbos[posnVBO], this->vertexPositions, visgl::posnLoc);
            this->setupVBO (this->vbos[normVBO], this->vertexNormals, visgl::normLoc);
            this->setupVBO (this->vbos[colVBO], this->vertexColors, visgl::colLoc);
            this->setupVBO (this->vbos[textureVBO], this->vertexTextures, visgl::textureLoc);
#endif
        }

    public:
        //! The colour of the text
        std::array<float, 3> clr_text = {0.0f, 0.0f, 0.0f};
        //! Line spacing, in multiples of the height of an 'h'
        float line_spacing = 1.4f;
        //! Parent Visual
        morph::Visual<glver>* parentVis = nullptr;

#ifdef GLAD_OPTION_GL_MX
        GladGLContext* glfn = nullptr;
#endif
        /*!
         * Callbacks are analogous to those in VisualModel
         */
        std::function<morph::visgl::visual_shaderprogs(morph::Visual<glver>*)> get_shaderprogs;
        //! Get the graphics shader prog id
        std::function<GLuint(morph::Visual<glver>*)> get_gprog;
        //! Get the text shader prog id
        std::function<GLuint(morph::Visual<glver>*)> get_tprog;
#ifdef GLAD_OPTION_GL_MX
        //! Get the GladGLContext function pointer
        std::function<GladGLContext*(morph::Visual<glver>*)> get_glfn;
#endif
        //! Set OpenGL context. Should call parentVis->setContext(). Can be nullptr (if in OWNED_MODE).
        std::function<void(morph::Visual<glver>*)> setContext;
        //! Release OpenGL context. Should call parentVis->releaseContext(). Can be nullptr (if in OWNED_MODE).
        std::function<void(morph::Visual<glver>*)> releaseContext;

        //! Setter for the parent pointer, parentVis
        void set_parent (morph::Visual<glver>* _vis)
        {
            //if (this->parentVis != nullptr) { throw std::runtime_error ("VisualTextModel: Set the parent pointer once only!"); }
            this->parentVis = _vis;
        }

    protected:
        // The text features for this VisualTextModel
        morph::TextFeatures tfeatures;
        //! A face for this text. The face is specfied by tfeatures.font
        morph::visgl::VisualFace* face = nullptr;
        //! The colour of the backing quad's vertices. Doesn't have any effect.
        std::array<float, 3> clr_backing = {1.0f, 1.0f, 0.0f};

        //! A scaling factor based on the desired width of an 'm'
        float fontscale = 1.0f; //  fontscale = tfeatures.fontsize/(float)tfeatures.fontres;

        //! model-view offset within the scene. Any model-view offset of the parent
        //! object should be incorporated into this offset. That is, if this
        //! VisualTextModel is the letter 'x' within a CoordArrows VisualModel, then the
        //! model-view offset here should be the CoordArrows model-view offset PLUS the
        //! length of the CoordArrow x axis length.
        vec<float> mv_offset;
        //! The model-view rotation of this text object. mv_offset and mv_rotation are
        //! together used to compute viewmatrix. Keep a copy so that it is easy to reset
        //! the viewmatrix and recompute it with either a new offset or a new rotation.
        quaternion<float> mv_rotation;

        //! A rotation of the parent model
        quaternion<float> parent_rotation;

        //! Scene view offset
        vec<float> sv_offset;
        //! Scene view rotation
        quaternion<float> sv_rotation;
        //! The text-model-specific view matrix and a scene matrix
        mat44<float> viewmatrix;
        //! Before, I wrote: We protect the scene matrix as updating it with the parent
        //! model's scene matrix likely involves also adding an additional
        //! translation. Now, I'm still slightly confused as to whether I *need* to have a
        //! copy of the scenematrix *here*.
        mat44<float> scenematrix;

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
        //! The OpenGL Vertex Array Object
        GLuint vao;
        //! Single vbo to use as in example
        GLuint vbo;
        //! Vertex Buffer Objects stored in an array
        std::unique_ptr<GLuint[]> vbos;
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
            std::size_t sz = dat.size() * sizeof(float);
#ifdef GLAD_OPTION_GL_MX
            auto _glfn = this->get_glfn (this->parentVis);
            _glfn->BindBuffer (GL_ARRAY_BUFFER, buf);
            _glfn->BufferData (GL_ARRAY_BUFFER, sz, dat.data(), GL_STATIC_DRAW);
            _glfn->VertexAttribPointer (bufferAttribPosition, 3, GL_FLOAT, GL_FALSE, 0, (void*)(0));
            _glfn->EnableVertexAttribArray (bufferAttribPosition);
#else
            glBindBuffer (GL_ARRAY_BUFFER, buf);
            glBufferData (GL_ARRAY_BUFFER, sz, dat.data(), GL_STATIC_DRAW);
            glVertexAttribPointer (bufferAttribPosition, 3, GL_FLOAT, GL_FALSE, 0, (void*)(0));
            glEnableVertexAttribArray (bufferAttribPosition);
#endif
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
