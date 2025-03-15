/*!
 * \file
 *
 * Declares a VisualModelImpl implementation class, adding globally aliases GL function calls.
 *
 * \author Seb James
 * \date March 2025
 */

#pragma once

#if defined __gl3_h_ || defined __gl_h_
// GL headers have been externally included
#else
# error "GL headers should have been included already"
#endif

#include <type_traits>

#include <morph/VisualModelBase.h>

#include <morph/gl/util_nomx.h>
#include <morph/VisualTextModel.h>
#include <morph/TextGeometry.h>

namespace morph {

    //! Forward declaration of a Visual base class
    template <int> class VisualBase;

    /*!
     * Single context safe implementation (morph::gl::multicontext == 0)
     */
    template <int glver = morph::gl::version_4_1, int mx = morph::gl::multicontext, std::enable_if_t<mx==0, bool> = true >
    struct VisualModelImpl : public morph::VisualModelBase<glver>
    {
        VisualModelImpl() : morph::VisualModelBase<glver>::VisualModelBase() {}
        VisualModelImpl (const vec<float>& _mv_offset) : morph::VisualModelBase<glver>::VisualModelBase(_mv_offset) {}

        //! destroy gl buffers in the deconstructor
        virtual ~VisualModelImpl()
        {
            // Explicitly clear owned VisualTextModels
            this->texts.clear();
            if (this->vbos != nullptr) {
                glDeleteBuffers (this->numVBO, this->vbos.get());
                glDeleteVertexArrays (1, &this->vao);
            }
        }

        /*!
         * Set up the passed-in VisualTextModel with functions that need access to the parent Visual attributes.
         */
        template <typename T>
        void bindmodel (std::unique_ptr<T>& model)
        {
            if (this->parentVis == nullptr) {
                throw std::runtime_error ("Can't bind a model, because I am not bound");
            }
            model->set_parent (this->parentVis);
            model->get_shaderprogs = &morph::VisualBase<glver>::get_shaderprogs;
            model->get_gprog = &morph::VisualBase<glver>::get_gprog;
            model->get_tprog = &morph::VisualBase<glver>::get_tprog;
            model->setContext = &morph::VisualBase<glver>::set_context;
            model->releaseContext = &morph::VisualBase<glver>::release_context;
        }

        //! Common code to call after the vertices have been set up. GL has to have been initialised.
        void postVertexInit() final
        {
            // Do gl memory allocation of vertex array once only
            if (this->vbos == nullptr) {
                // Create vertex array object
                glGenVertexArrays (1, &this->vao); // Safe for OpenGL 4.4-
            }
            glBindVertexArray (this->vao);

            // Create the vertex buffer objects (once only)
            if (this->vbos == nullptr) {
                this->vbos = std::make_unique<GLuint[]>(this->numVBO);
                glGenBuffers (this->numVBO, this->vbos.get()); // OpenGL 4.4- safe
            }

            // Set up the indices buffer - bind and buffer the data in this->indices
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vbos[this->idxVBO]);

            std::size_t sz = this->indices.size() * sizeof(GLuint);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sz, this->indices.data(), GL_STATIC_DRAW);

            // Binds data from the "C++ world" to the OpenGL shader world for
            // "position", "normalin" and "color"
            // (bind, buffer and set vertex array object attribute)
            this->setupVBO (this->vbos[this->posnVBO], this->vertexPositions, visgl::posnLoc);
            this->setupVBO (this->vbos[this->normVBO], this->vertexNormals, visgl::normLoc);
            this->setupVBO (this->vbos[this->colVBO], this->vertexColors, visgl::colLoc);

            // Unbind only the vertex array (not the buffers, that causes GL_INVALID_ENUM errors)
            glBindVertexArray(0); // carefully unbind and rebind
            morph::gl::Util::checkError (__FILE__, __LINE__);

            this->postVertexInitRequired = false;
        }

        //! Initialize vertex buffer objects and vertex array object. Empty for 'text only' VisualModels.
        void initializeVertices() {};

        /*!
         * Re-initialize the buffers. Client code might have appended to
         * vertexPositions/Colors/Normals and indices before calling this method.
         */
        void reinit_buffers() final
        {
            if (this->setContext != nullptr) { this->setContext (this->parentVis); }
            if (this->postVertexInitRequired == true) { this->postVertexInit(); }
            // Now re-set up the VBOs
            glBindVertexArray (this->vao);                              // carefully unbind and rebind
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vbos[this->idxVBO]);  // carefully unbind and rebind

            std::size_t sz = this->indices.size() * sizeof(GLuint);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sz, this->indices.data(), GL_STATIC_DRAW);
            this->setupVBO (this->vbos[this->posnVBO], this->vertexPositions, visgl::posnLoc);
            this->setupVBO (this->vbos[this->normVBO], this->vertexNormals, visgl::normLoc);
            this->setupVBO (this->vbos[this->colVBO], this->vertexColors, visgl::colLoc);

            glBindVertexArray(0);                               // carefully unbind and rebind
            morph::gl::Util::checkError (__FILE__, __LINE__);   // carefully unbind and rebind
        }

        //! reinit ONLY vertexColors buffer
        void reinit_colour_buffer() final
        {
            if (this->setContext != nullptr) { this->setContext (this->parentVis); }
            if (this->postVertexInitRequired == true) { this->postVertexInit(); }
            // Now re-set up the VBOs
            glBindVertexArray (this->vao);  // carefully unbind and rebind
            this->setupVBO (this->vbos[this->colVBO], this->vertexColors, visgl::colLoc);
            glBindVertexArray(0);  // carefully unbind and rebind
            morph::gl::Util::checkError (__FILE__, __LINE__);
        }

        void clearTexts() { this->texts.clear(); }

        static constexpr bool debug_render = false;
        //! Render the VisualModel. Note that it is assumed that the OpenGL context has been
        //! obtained by the parent Visual::render call.
        void render() // not final
        {
            if (this->hide == true) { return; }

            // Execute post-vertex init at render, as GL should be available.
            if (this->postVertexInitRequired == true) { this->postVertexInit(); }

            GLint prev_shader = 0;

            glGetIntegerv (GL_CURRENT_PROGRAM, &prev_shader);
            // Ensure the correct program is in play for this VisualModel
            glUseProgram (this->get_gprog(this->parentVis));

            if (!this->indices.empty()) {
                // It is only necessary to bind the vertex array object before rendering
                // (not the vertex buffer objects)
                glBindVertexArray (this->vao);

                // Pass this->float to GLSL so the model can have an alpha value.
                GLint loc_a = glGetUniformLocation (this->get_gprog(this->parentVis), static_cast<const GLchar*>("alpha"));
                if (loc_a != -1) { glUniform1f (loc_a, this->alpha); }

                GLint loc_v = glGetUniformLocation (this->get_gprog(this->parentVis), static_cast<const GLchar*>("v_matrix"));
                if (loc_v != -1) { glUniformMatrix4fv (loc_v, 1, GL_FALSE, this->scenematrix.mat.data()); }

                // Should be able to apply scaling to the model matrix
                GLint loc_m = glGetUniformLocation (this->get_gprog(this->parentVis), static_cast<const GLchar*>("m_matrix"));
                if (loc_m != -1) { glUniformMatrix4fv (loc_m, 1, GL_FALSE, (this->model_scaling * this->viewmatrix).mat.data()); }

                if constexpr (debug_render) {
                    std::cout << "VisualModelImpl::render: scenematrix:\n" << this->scenematrix << std::endl;
                    std::cout << "VisualModelImpl::render: model viewmatrix:\n" << this->viewmatrix << std::endl;
                }

                // Draw the triangles
                glDrawElements (GL_TRIANGLES, static_cast<unsigned int>(this->indices.size()), GL_UNSIGNED_INT, 0);

                // Unbind the VAO
                glBindVertexArray(0);
            }
            morph::gl::Util::checkError (__FILE__, __LINE__);

            // Now render any VisualTextModels
            auto ti = this->texts.begin();
            while (ti != this->texts.end()) { (*ti)->render(); ti++; }

            glUseProgram (prev_shader);
            morph::gl::Util::checkError (__FILE__, __LINE__);
        }

        /*!
         * Helper to make a VisualTextModel and bind it ready for use.
         *
         * You could write it out explicitly as:
         *
         * std::unique_ptr<morph::VisualTextModel<glver>> vtm1 = this->makeVisualTextModel (tfca);
         *
         * Or use auto to help:
         *
         * auto vtm1 = this->makeVisualTextModel (tfca);
         *
         * See GraphVisual.h for examples.
         */
        std::unique_ptr<morph::VisualTextModel<glver>> makeVisualTextModel (const morph::TextFeatures& tfeatures)
        {
            auto tmup = std::make_unique<morph::VisualTextModel<glver>> (tfeatures);
            this->bindmodel (tmup);
            return tmup;
        }

        /*!
         * Add a text label to the model at location (within the model coordinates)
         * toffset. Return the text geometry of the added label so caller can place
         * associated text correctly.  Control font size, resolution, colour and font
         * face with tfeatures.
         */
        morph::TextGeometry addLabel (const std::string& _text,
                                      const morph::vec<float, 3>& _toffset,
                                      const morph::TextFeatures& tfeatures = morph::TextFeatures())
        {
            if (this->get_shaderprogs(this->parentVis).tprog == 0) {
                throw std::runtime_error ("No text shader prog. Did your VisualModel-derived class set it up?");
            }

            if (this->setContext != nullptr) { this->setContext (this->parentVis); } // For VisualTextModel

            auto tmup = this->makeVisualTextModel (tfeatures);

            if (tfeatures.centre_horz == true) {
                morph::TextGeometry tg = tmup->getTextGeometry(_text);
                morph::vec<float, 3> centred_locn = _toffset;
                centred_locn[0] = -tg.half_width();
                tmup->setupText (_text, centred_locn+this->mv_offset, tfeatures.colour);
            } else {
                tmup->setupText (_text, _toffset+this->mv_offset, tfeatures.colour);
            }

            this->texts.push_back (std::move(tmup));

            // As this is a setup function, release the context
            if (this->releaseContext != nullptr) { this->releaseContext (this->parentVis); }

            return this->texts.back()->getTextGeometry();
        }

        /*!
         * Add a text label, with given offset _toffset and the specified tfeatures. The
         * reference to a pointer, tm, allows client code to change the text of the
         * VisualTextModel as necessary, after the label has been added.
         */
        morph::TextGeometry addLabel (const std::string& _text,
                                      const morph::vec<float, 3>& _toffset,
                                      morph::VisualTextModel<glver>*& tm,
                                      const morph::TextFeatures& tfeatures = morph::TextFeatures())
        {
            if (this->get_shaderprogs(this->parentVis).tprog == 0) {
                throw std::runtime_error ("No text shader prog. Did your VisualModel-derived class set it up?");
            }

            if (this->setContext != nullptr) { this->setContext (this->parentVis); } // For VisualTextModel

            auto tmup = this->makeVisualTextModel (tfeatures);

            if (tfeatures.centre_horz == true) {
                morph::TextGeometry tg = tmup->getTextGeometry(_text);
                morph::vec<float, 3> centred_locn = _toffset;
                centred_locn[0] = -tg.half_width();
                tmup->setupText (_text, centred_locn+this->mv_offset, tfeatures.colour);
            } else {
                tmup->setupText (_text, _toffset+this->mv_offset, tfeatures.colour);
            }

            this->texts.push_back (std::move(tmup));
            tm = this->texts.back().get();

            // As this is a setup function, release the context
            if (this->releaseContext != nullptr) { this->releaseContext (this->parentVis); }

            return this->texts.back()->getTextGeometry();
        }

        void setSceneMatrixTexts (const mat44<float>& sv) final
        {
            auto ti = this->texts.begin();
            while (ti != this->texts.end()) { (*ti)->setSceneMatrix (sv); ti++; }
        }

        void setSceneTranslationTexts (const vec<float>& v0) final
        {
            auto ti = this->texts.begin();
            while (ti != this->texts.end()) { (*ti)->setSceneTranslation (v0); ti++; }
        }

        void setViewRotationTexts (const quaternion<float>& r)
        {
            // When rotating a model that contains texts, we need to rotate the scene
            // for the texts and also inverse-rotate the view of the texts.
            auto ti = this->texts.begin();
            while (ti != this->texts.end()) {
                // Rotate the scene. Note this won't work if the VisualModel has a
                // mv_offset that is away from the origin.
                (*ti)->setSceneRotation (r); // Need this to rotate about mv_offset. BUT the
                                             // translation is already there in the text,
                                             // but in the MODEL view.

                // Rotate the view of the text an opposite amount, to keep it facing forwards
                (*ti)->setViewRotation (r.invert());
                ti++;
            }
        }

        void addViewRotationTexts (const quaternion<float>& r)
        {
            auto ti = this->texts.begin();
            while (ti != this->texts.end()) { (*ti)->addViewRotation (r); ti++; }
        }

    protected:

        //! A vector of pointers to text models that should be rendered.
        std::vector<std::unique_ptr<morph::VisualTextModel<glver>>> texts;

        //! Set up a vertex buffer object - bind, buffer and set vertex array object attribute
        void setupVBO (GLuint& buf, std::vector<float>& dat, unsigned int bufferAttribPosition) final
        {
            std::size_t sz = dat.size() * sizeof(float);

            glBindBuffer (GL_ARRAY_BUFFER, buf);
            morph::gl::Util::checkError (__FILE__, __LINE__);
            glBufferData (GL_ARRAY_BUFFER, sz, dat.data(), GL_STATIC_DRAW);
            morph::gl::Util::checkError (__FILE__, __LINE__);
            glVertexAttribPointer (bufferAttribPosition, 3, GL_FLOAT, GL_FALSE, 0, (void*)(0));
            morph::gl::Util::checkError (__FILE__, __LINE__);
            glEnableVertexAttribArray (bufferAttribPosition);
            morph::gl::Util::checkError (__FILE__, __LINE__);
        }
    };

} // namespace morph
