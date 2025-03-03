/*!
 * \file
 *
 * Declares a VisualModel class to hold the vertices that make up some individual
 * model object that can be part of an OpenGL scene.
 *
 * \author Seb James
 * \date May 2019
 */

#pragma once

#if defined __gl3_h_ || defined __gl_h_
// GL headers have been externally included
#else
# error "GL headers should have been included already"
#endif

#include <morph/gl/version.h>
#include <morph/geometry.h>
#include <morph/quaternion.h>
#include <morph/mat44.h>
#include <morph/vvec.h>
#include <morph/vec.h>
#include <morph/range.h>
#include <morph/mathconst.h>
#include <morph/VisualCommon.h>
#ifdef GLAD_OPTION_GL_MX
# include <morph/gl/util_mx.h>
# include <morph/VisualTextModelMX.h>
#else
# include <morph/gl/util.h>
# include <morph/VisualTextModel.h>
#endif
#include <morph/colour.h>
#include <morph/base64.h>
#include <morph/MathAlgo.h>
#include <iostream>
#include <vector>
#include <array>
#include <algorithm>
#include <iterator>
#include <string>
#include <memory>
#include <functional>
#include <cstddef>
#include <cmath>

// Switches on some changes where I carefully unbind gl buffers after calling
// glBufferData() and rebind when changing the vertex model. Makes no difference on my
// Macbook Air, but should be more correct. Dotting my 'i's and 't's
// #define CAREFULLY_UNBIND_AND_REBIND 1

namespace morph {

    union float_bytes
    {
        float f;
        uint8_t bytes[sizeof(float)];
    };

    //! Forward declaration of a Visual class
    template <int>
    class VisualBase;

#ifdef GLAD_OPTION_GL_MX
    template <int>
    class VisualOwnableMX;
#endif

    /*!
     * OpenGL model base class
     *
     * This class is a base 'OpenGL model' class. It has the common code to create the vertices for
     * some individual OpengGL model which is to be rendered in a 3-D scene.
     *
     * Some OpenGL models are derived directly from VisualModel; see for example morph::CoordArrows.
     *
     * Other models in morphologica are derived via morph::VisualDataModel, which adds a common
     * mechanism for managing the data which is to be visualised by the final 'Visual' object (such
     * as morph::HexGridVisual or morph::ScatterVisual)
     *
     * This class contains some common 'object primitives' code, such as computeSphere and
     * computeCone, which compute the vertices that will make up sphere and cone, respectively.
     */
    template <int glver = morph::gl::version_4_1>
    class VisualModel
    {
        //! Debug rendering process with cout messages
        static constexpr bool debug_render = false;

    public:
        VisualModel()
        {
            this->mv_offset = { 0.0f, 0.0f, 0.0f };
            this->model_scaling.setToIdentity();
        }

        VisualModel (const vec<float> _mv_offset)
        {
            this->mv_offset = _mv_offset;
            this->viewmatrix.translate (this->mv_offset);
            this->model_scaling.setToIdentity();
        }

        //! destroy gl buffers in the deconstructor
        virtual ~VisualModel()
        {
            if (this->vbos != nullptr) {
#ifdef GLAD_OPTION_GL_MX
                //GladGLContext* _glfn = this->get_glfn(reinterpret_cast<morph::VisualOwnableMX<glver>*>(this->parentVis));
                GladGLContext* _glfn = this->get_glfn(this->parentVis);
                _glfn->DeleteBuffers (numVBO, this->vbos.get());
                _glfn->DeleteVertexArrays (1, &this->vao);
#else
                glDeleteBuffers (numVBO, this->vbos.get());
                glDeleteVertexArrays (1, &this->vao);
#endif
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
#ifdef GLAD_OPTION_GL_MX
            model->get_glfn = &morph::VisualOwnableMX<glver>::get_glfn;
#endif
            model->setContext = &morph::VisualBase<glver>::set_context;
            model->releaseContext = &morph::VisualBase<glver>::release_context;
        }

        bool postVertexInitRequired = false;
        //! Common code to call after the vertices have been set up. GL has to have been initialised.
        void postVertexInit()
        {
#ifdef GLAD_OPTION_GL_MX
            GladGLContext* _glfn = this->get_glfn(this->parentVis);

            // Do gl memory allocation of vertex array once only
            if (this->vbos == nullptr) {
                // Create vertex array object
                _glfn->GenVertexArrays (1, &this->vao); // Safe for OpenGL 4.4-
            }
            _glfn->BindVertexArray (this->vao);

            // Create the vertex buffer objects (once only)
            if (this->vbos == nullptr) {
                this->vbos = std::make_unique<GLuint[]>(numVBO);
                _glfn->GenBuffers (numVBO, this->vbos.get()); // OpenGL 4.4- safe
            }

            // Set up the indices buffer - bind and buffer the data in this->indices
            _glfn->BindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vbos[idxVBO]);

            std::size_t sz = this->indices.size() * sizeof(GLuint);
            _glfn->BufferData(GL_ELEMENT_ARRAY_BUFFER, sz, this->indices.data(), GL_STATIC_DRAW);

            // Binds data from the "C++ world" to the OpenGL shader world for
            // "position", "normalin" and "color"
            // (bind, buffer and set vertex array object attribute)
            this->setupVBO (this->vbos[posnVBO], this->vertexPositions, visgl::posnLoc);
            this->setupVBO (this->vbos[normVBO], this->vertexNormals, visgl::normLoc);
            this->setupVBO (this->vbos[colVBO], this->vertexColors, visgl::colLoc);

            // Unbind only the vertex array (not the buffers, that causes GL_INVALID_ENUM errors)
            _glfn->BindVertexArray(0); // carefully unbind and rebind
            morph::gl::Util::checkError (__FILE__, __LINE__, _glfn);

#else // not GLAD_OPTION_GL_MX

            // Do gl memory allocation of vertex array once only
            if (this->vbos == nullptr) {
                // Create vertex array object
                glGenVertexArrays (1, &this->vao); // Safe for OpenGL 4.4-
            }
            glBindVertexArray (this->vao);

            // Create the vertex buffer objects (once only)
            if (this->vbos == nullptr) {
                this->vbos = std::make_unique<GLuint[]>(numVBO);
                glGenBuffers (numVBO, this->vbos.get()); // OpenGL 4.4- safe
            }

            // Set up the indices buffer - bind and buffer the data in this->indices
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vbos[idxVBO]);

            std::size_t sz = this->indices.size() * sizeof(GLuint);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sz, this->indices.data(), GL_STATIC_DRAW);

            // Binds data from the "C++ world" to the OpenGL shader world for
            // "position", "normalin" and "color"
            // (bind, buffer and set vertex array object attribute)
            this->setupVBO (this->vbos[posnVBO], this->vertexPositions, visgl::posnLoc);
            this->setupVBO (this->vbos[normVBO], this->vertexNormals, visgl::normLoc);
            this->setupVBO (this->vbos[colVBO], this->vertexColors, visgl::colLoc);

            // Unbind only the vertex array (not the buffers, that causes GL_INVALID_ENUM errors)
            glBindVertexArray(0); // carefully unbind and rebind
            morph::gl::Util::checkError (__FILE__, __LINE__);
#endif
            this->postVertexInitRequired = false;
        }

        //! Initialize vertex buffer objects and vertex array object. Empty for 'text only' VisualModels.
        virtual void initializeVertices() {};

        /*!
         * Re-initialize the buffers. Client code might have appended to
         * vertexPositions/Colors/Normals and indices before calling this method.
         */
        void reinit_buffers()
        {
#ifdef GLAD_OPTION_GL_MX
            GladGLContext* _glfn = this->get_glfn(this->parentVis);
            if (this->setContext != nullptr) { this->setContext (this->parentVis); }
            if (this->postVertexInitRequired == true) { this->postVertexInit(); }
            // Now re-set up the VBOs
            _glfn->BindVertexArray (this->vao);                              // carefully unbind and rebind
            _glfn->BindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vbos[idxVBO]);  // carefully unbind and rebind

            std::size_t sz = this->indices.size() * sizeof(GLuint);
            _glfn->BufferData(GL_ELEMENT_ARRAY_BUFFER, sz, this->indices.data(), GL_STATIC_DRAW);
            this->setupVBO (this->vbos[posnVBO], this->vertexPositions, visgl::posnLoc);
            this->setupVBO (this->vbos[normVBO], this->vertexNormals, visgl::normLoc);
            this->setupVBO (this->vbos[colVBO], this->vertexColors, visgl::colLoc);

            _glfn->BindVertexArray(0);                                // carefully unbind and rebind
            morph::gl::Util::checkError (__FILE__, __LINE__, _glfn);  // carefully unbind and rebind

#else // not GLAD_OPTION_GL_MX
            if (this->setContext != nullptr) { this->setContext (this->parentVis); }
            if (this->postVertexInitRequired == true) { this->postVertexInit(); }
            // Now re-set up the VBOs
            glBindVertexArray (this->vao);                              // carefully unbind and rebind
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vbos[idxVBO]);  // carefully unbind and rebind

            std::size_t sz = this->indices.size() * sizeof(GLuint);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sz, this->indices.data(), GL_STATIC_DRAW);
            this->setupVBO (this->vbos[posnVBO], this->vertexPositions, visgl::posnLoc);
            this->setupVBO (this->vbos[normVBO], this->vertexNormals, visgl::normLoc);
            this->setupVBO (this->vbos[colVBO], this->vertexColors, visgl::colLoc);

            glBindVertexArray(0);                               // carefully unbind and rebind
            morph::gl::Util::checkError (__FILE__, __LINE__);   // carefully unbind and rebind
#endif
        }

        //! reinit ONLY vertexColors buffer
        void reinit_colour_buffer()
        {
            if (this->setContext != nullptr) { this->setContext (this->parentVis); }
            if (this->postVertexInitRequired == true) { this->postVertexInit(); }
#ifdef GLAD_OPTION_GL_MX
            GladGLContext* _glfn = this->get_glfn(this->parentVis);
            // Now re-set up the VBOs
            _glfn->BindVertexArray (this->vao);  // carefully unbind and rebind
            this->setupVBO (this->vbos[colVBO], this->vertexColors, visgl::colLoc);
            _glfn->BindVertexArray(0);  // carefully unbind and rebind
            morph::gl::Util::checkError (__FILE__, __LINE__, _glfn);
#else
            // Now re-set up the VBOs
            glBindVertexArray (this->vao);  // carefully unbind and rebind
            this->setupVBO (this->vbos[colVBO], this->vertexColors, visgl::colLoc);
            glBindVertexArray(0);  // carefully unbind and rebind
            morph::gl::Util::checkError (__FILE__, __LINE__);
#endif
        }

        void clearTexts() { this->texts.clear(); }

        //! Clear out the model, *including text models*
        void clear()
        {
            this->vertexPositions.clear();
            this->vertexNormals.clear();
            this->vertexColors.clear();
            this->indices.clear();
            this->clearTexts();
            this->idx = 0u;
            this->reinit_buffers();
        }

        //! Re-create the model - called after updating data
        void reinit()
        {
            if (this->setContext != nullptr) { this->setContext (this->parentVis); }
            // Fixme: Better not to clear, then repeatedly pushback here:
            this->vertexPositions.clear();
            this->vertexNormals.clear();
            this->vertexColors.clear();
            this->indices.clear();
            // NB: Do NOT call clearTexts() here! We're only updating the model itself.
            this->idx = 0u;
            this->initializeVertices();
            this->reinit_buffers();
        }

        /*!
         * For some models it's important to clear the texts when reinitialising. This is NOT the
         * same as VisualModel::clear() followed by initializeVertices(). For the same effect, you
         * can call clearTexts() then reinit().
         */
        void reinit_with_clearTexts()
        {
            if (this->setContext != nullptr) { this->setContext (this->parentVis); }
            this->vertexPositions.clear();
            this->vertexNormals.clear();
            this->vertexColors.clear();
            this->indices.clear();
            this->clearTexts();
            this->idx = 0u;
            this->initializeVertices();
            this->reinit_buffers();
        }

        void reserve_vertices (std::size_t n_vertices)
        {
            this->vertexPositions.reserve (3u * n_vertices);
            this->vertexNormals.reserve (3u * n_vertices);
            this->vertexColors.reserve (3u * n_vertices);
            this->indices.reserve (6u * n_vertices);
        }

        /*!
         * A function to call initialiseVertices and postVertexInit after any necessary attributes
         * have been set (see, for example, setting the colour maps up in VisualDataModel).
         */
        void finalize()
        {
            if (this->setContext != nullptr) { this->setContext (this->parentVis); }
            this->initializeVertices();
            this->postVertexInitRequired = true;
            // Release context after creating and finalizing this VisualModel. On Visual::render(),
            // context will be re-acquired.
            if (this->releaseContext != nullptr) { this->releaseContext (this->parentVis); }
        }

        //! Render the VisualModel. Note that it is assumed that the OpenGL context has been
        //! obtained by the parent Visual::render call.
        virtual void render()
        {
            if (this->hide == true) { return; }

            // Execute post-vertex init at render, as GL should be available.
            if (this->postVertexInitRequired == true) { this->postVertexInit(); }

            GLint prev_shader = 0;

#ifdef GLAD_OPTION_GL_MX
            GladGLContext* _glfn = this->get_glfn (this->parentVis);
            _glfn->GetIntegerv (GL_CURRENT_PROGRAM, &prev_shader);
            // Ensure the correct program is in play for this VisualModel
            _glfn->UseProgram (this->get_gprog(this->parentVis));

            if (!this->indices.empty()) {
                // It is only necessary to bind the vertex array object before rendering
                // (not the vertex buffer objects)
                _glfn->BindVertexArray (this->vao);

                // Pass this->float to GLSL so the model can have an alpha value.
                GLint loc_a = _glfn->GetUniformLocation (this->get_gprog(this->parentVis), static_cast<const GLchar*>("alpha"));
                if (loc_a != -1) { _glfn->Uniform1f (loc_a, this->alpha); }

                GLint loc_v = _glfn->GetUniformLocation (this->get_gprog(this->parentVis), static_cast<const GLchar*>("v_matrix"));
                if (loc_v != -1) { _glfn->UniformMatrix4fv (loc_v, 1, GL_FALSE, this->scenematrix.mat.data()); }

                // Should be able to apply scaling to the model matrix
                GLint loc_m = _glfn->GetUniformLocation (this->get_gprog(this->parentVis), static_cast<const GLchar*>("m_matrix"));
                if (loc_m != -1) { _glfn->UniformMatrix4fv (loc_m, 1, GL_FALSE, (this->model_scaling * this->viewmatrix).mat.data()); }

                if constexpr (debug_render) {
                    std::cout << "VisualModel::render: scenematrix:\n" << scenematrix << std::endl;
                    std::cout << "VisualModel::render: model viewmatrix:\n" << viewmatrix << std::endl;
                }

                // Draw the triangles
                _glfn->DrawElements (GL_TRIANGLES, static_cast<unsigned int>(this->indices.size()), GL_UNSIGNED_INT, 0);

                // Unbind the VAO
                _glfn->BindVertexArray(0);
            }
            morph::gl::Util::checkError (__FILE__, __LINE__, _glfn);
#else
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
                    std::cout << "VisualModel::render: scenematrix:\n" << scenematrix << std::endl;
                    std::cout << "VisualModel::render: model viewmatrix:\n" << viewmatrix << std::endl;
                }

                // Draw the triangles
                glDrawElements (GL_TRIANGLES, static_cast<unsigned int>(this->indices.size()), GL_UNSIGNED_INT, 0);

                // Unbind the VAO
                glBindVertexArray(0);
            }
            morph::gl::Util::checkError (__FILE__, __LINE__);
#endif

            // Now render any VisualTextModels
            auto ti = this->texts.begin();
            while (ti != this->texts.end()) { (*ti)->render(); ti++; }
#ifdef GLAD_OPTION_GL_MX
            _glfn->UseProgram (prev_shader);
            morph::gl::Util::checkError (__FILE__, __LINE__, _glfn);
#else
            glUseProgram (prev_shader);
            morph::gl::Util::checkError (__FILE__, __LINE__);
#endif
        }

        //! Helper to make the right kind of text model
        auto make_text_model(const morph::TextFeatures& tfeatures)
        {
#ifdef GLAD_OPTION_GL_MX
            auto tmup = std::make_unique<morph::VisualTextModelMX<glver>> (tfeatures);
#else
            auto tmup = std::make_unique<morph::VisualTextModel<glver>> (tfeatures);
#endif
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

            auto tmup = this->make_text_model (tfeatures);

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
#ifdef GLAD_OPTION_GL_MX
                                      morph::VisualTextModelMX<glver>*& tm,
#else
                                      morph::VisualTextModel<glver>*& tm,
#endif
                                      const morph::TextFeatures& tfeatures = morph::TextFeatures())
        {
            if (this->get_shaderprogs(this->parentVis).tprog == 0) {
                throw std::runtime_error ("No text shader prog. Did your VisualModel-derived class set it up?");
            }

            if (this->setContext != nullptr) { this->setContext (this->parentVis); } // For VisualTextModel

            auto tmup = this->make_text_model (tfeatures);

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

        //! Setter for the viewmatrix
        void setViewMatrix (const mat44<float>& mv) { this->viewmatrix = mv; }

        //! When setting the scene matrix, also have to set the text's scene matrices.
        void setSceneMatrix (const mat44<float>& sv)
        {
            this->scenematrix = sv;
            // For each text model, also set scene matrix
            auto ti = this->texts.begin();
            while (ti != this->texts.end()) { (*ti)->setSceneMatrix (sv); ti++; }
        }

        //! Set a translation into the scene and into any child texts
        void setSceneTranslation (const vec<float>& v0)
        {
            this->scenematrix.setToIdentity();
            this->sv_offset = v0;
            this->scenematrix.translate (this->sv_offset);
            this->scenematrix.rotate (this->sv_rotation);
            auto ti = this->texts.begin();
            while (ti != this->texts.end()) { (*ti)->setSceneTranslation (v0); ti++; }
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
            this->scenematrix.setToIdentity();
            this->sv_rotation = r;
            this->scenematrix.translate (this->sv_offset);
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
            this->viewmatrix.setToIdentity();
            this->mv_offset = v0;
            this->viewmatrix.translate (this->mv_offset);
            this->viewmatrix.rotate (this->mv_rotation);
        }

        //! Add a translation to the model view matrix
        void addViewTranslation (const vec<float>& v0)
        {
            this->mv_offset += v0;
            this->viewmatrix.translate (v0);
        }

        void setViewRotationFixTexts (const quaternion<float>& r)
        {
            this->viewmatrix.setToIdentity();
            this->mv_rotation = r;
            this->viewmatrix.translate (this->mv_offset);
            this->viewmatrix.rotate (this->mv_rotation);
        }

        //! Set a rotation (only) into the view
        void setViewRotation (const quaternion<float>& r)
        {
            this->viewmatrix.setToIdentity();
            this->mv_rotation = r;
            this->viewmatrix.translate (this->mv_offset);
            this->viewmatrix.rotate (this->mv_rotation);

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

        //! Apply a further rotation to the model view matrix
        void addViewRotation (const quaternion<float>& r)
        {
            this->mv_rotation.premultiply (r);
            this->viewmatrix.rotate (r);
            std::cout << "VisualModel::addViewRotation: FIXME? or t->addSceneRotation(r)?\n";
            auto ti = this->texts.begin();
            while (ti != this->texts.end()) { (*ti)->addViewRotation (r); ti++; }
        }

        // The alpha attribute accessors
        void setAlpha (const float _a) { this->alpha = _a; }
        float getAlpha() const { return this->alpha; }
        void incAlpha()
        {
            this->alpha += 0.1f;
            this->alpha = this->alpha > 1.0f ? 1.0f : this->alpha;
        }
        void decAlpha()
        {
            this->alpha -= 0.1f;
            this->alpha = this->alpha < 0.0f ? 0.0f : this->alpha;
        }

        // The hide attribute accessors
        void setHide (const bool _h = true) { this->hide = _h; }
        void toggleHide() { this->hide = this->hide ? false : true; }
        float hidden() const { return this->hide; }

        /*
         * Methods used by Visual::savegltf()
         */

        //! Get mv_offset in a json-friendly string
        std::string translation_str() { return this->mv_offset.str_mat(); }
        //! And a simple getter for mv_offset
        vec<float> get_mv_offset() { return this->mv_offset; }

        //! Return the number of elements in this->indices
        std::size_t indices_size() { return this->indices.size(); }
        float indices_max() { return this->idx_max; }
        float indices_min() { return this->idx_min; }
        std::size_t indices_bytes() { return this->indices.size() * sizeof (GLuint); }
        //! Return base64 encoded version of indices
        std::string indices_base64()
        {
            std::vector<std::uint8_t> idx_bytes (this->indices.size() << 2, 0);
            std::size_t b = 0u;
            for (auto i : this->indices) {
                idx_bytes[b++] = i & 0xff;
                idx_bytes[b++] = i >> 8 & 0xff;
                idx_bytes[b++] = i >> 16 & 0xff;
                idx_bytes[b++] = i >> 24 & 0xff;
            }
            return base64::encode (idx_bytes);
        }

        /*!
         * Find the extents of this VisualModel, returning it as the x range, the y range and the z range.
         */
        morph::vec<morph::range<float>, 3> extents()
        {
            morph::vec<morph::range<float>, 3> axis_extents;
            for (unsigned int i = 0; i < 3; ++i) { axis_extents[i].search_init(); }
            for (unsigned int j = 0; j < static_cast<unsigned int>(this->vertexPositions.size() - 2); j += 3) {
                for (unsigned int i = 0; i < 3; ++i) { axis_extents[i].update (this->vertexPositions[j+i]); }
            }
            return axis_extents;
        }

        /*!
         * Compute the max and min values of indices and vertexPositions/Colors/Normals for use
         * when saving gltf files
         */
        void computeVertexMaxMins()
        {
            // Compute index maxmins
            for (std::size_t i = 0u; i < this->indices.size(); ++i) {
                idx_max = this->indices[i] > idx_max ? this->indices[i] : idx_max;
                idx_min = this->indices[i] < idx_min ? this->indices[i] : idx_min;
            }
            // Check every 0th entry in vertex Positions, every 1st, etc for max in the

            if (this->vertexPositions.size() != this->vertexColors.size()
                ||this->vertexPositions.size() != this->vertexNormals.size()) {
                throw std::runtime_error ("Expect vertexPositions, Colors and Normals vectors all to have same size");
            }

            for (std::size_t i = 0u; i < this->vertexPositions.size(); i+=3u) {
                vpos_maxes[0] =  (vertexPositions[i] > vpos_maxes[0]) ? vertexPositions[i] : vpos_maxes[0];
                vpos_maxes[1] =  (vertexPositions[i+1] > vpos_maxes[1]) ? vertexPositions[i+1] : vpos_maxes[1];
                vpos_maxes[2] =  (vertexPositions[i+2] > vpos_maxes[2]) ? vertexPositions[i+2] : vpos_maxes[2];
                vcol_maxes[0] =  (vertexColors[i] > vcol_maxes[0]) ? vertexColors[i] : vcol_maxes[0];
                vcol_maxes[1] =  (vertexColors[i+1] > vcol_maxes[1]) ? vertexColors[i+1] : vcol_maxes[1];
                vcol_maxes[2] =  (vertexColors[i+2] > vcol_maxes[2]) ? vertexColors[i+2] : vcol_maxes[2];
                vnorm_maxes[0] =  (vertexNormals[i] > vnorm_maxes[0]) ? vertexNormals[i] : vnorm_maxes[0];
                vnorm_maxes[1] =  (vertexNormals[i+1] > vnorm_maxes[1]) ? vertexNormals[i+1] : vnorm_maxes[1];
                vnorm_maxes[2] =  (vertexNormals[i+2] > vnorm_maxes[2]) ? vertexNormals[i+2] : vnorm_maxes[2];

                vpos_mins[0] =  (vertexPositions[i] < vpos_mins[0]) ? vertexPositions[i] : vpos_mins[0];
                vpos_mins[1] =  (vertexPositions[i+1] < vpos_mins[1]) ? vertexPositions[i+1] : vpos_mins[1];
                vpos_mins[2] =  (vertexPositions[i+2] < vpos_mins[2]) ? vertexPositions[i+2] : vpos_mins[2];
                vcol_mins[0] =  (vertexColors[i] < vcol_mins[0]) ? vertexColors[i] : vcol_mins[0];
                vcol_mins[1] =  (vertexColors[i+1] < vcol_mins[1]) ? vertexColors[i+1] : vcol_mins[1];
                vcol_mins[2] =  (vertexColors[i+2] < vcol_mins[2]) ? vertexColors[i+2] : vcol_mins[2];
                vnorm_mins[0] =  (vertexNormals[i] < vnorm_mins[0]) ? vertexNormals[i] : vnorm_mins[0];
                vnorm_mins[1] =  (vertexNormals[i+1] < vnorm_mins[1]) ? vertexNormals[i+1] : vnorm_mins[1];
                vnorm_mins[2] =  (vertexNormals[i+2] < vnorm_mins[2]) ? vertexNormals[i+2] : vnorm_mins[2];
            }
        }

        std::size_t vpos_size() { return this->vertexPositions.size(); }
        std::string vpos_max() { return this->vpos_maxes.str_mat(); }
        std::string vpos_min() { return this->vpos_mins.str_mat(); }
        std::size_t vpos_bytes() { return this->vertexPositions.size() * sizeof (float); }
        std::string vpos_base64()
        {
            std::vector<std::uint8_t> _bytes (this->vertexPositions.size() << 2, 0);
            std::size_t b = 0u;
            float_bytes fb;
            for (auto i : this->vertexPositions) {
                fb.f = i;
                _bytes[b++] = fb.bytes[0];
                _bytes[b++] = fb.bytes[1];
                _bytes[b++] = fb.bytes[2];
                _bytes[b++] = fb.bytes[3];
            }
            return base64::encode (_bytes);
        }
        std::size_t vcol_size() { return this->vertexColors.size(); }
        std::string vcol_max() { return this->vcol_maxes.str_mat(); }
        std::string vcol_min() { return this->vcol_mins.str_mat(); }
        std::size_t vcol_bytes() { return this->vertexColors.size() * sizeof (float); }
        std::string vcol_base64()
        {
            std::vector<std::uint8_t> _bytes (this->vertexColors.size() << 2, 0);
            std::size_t b = 0u;
            float_bytes fb;
            for (auto i : this->vertexColors) {
                fb.f = i;
                _bytes[b++] = fb.bytes[0];
                _bytes[b++] = fb.bytes[1];
                _bytes[b++] = fb.bytes[2];
                _bytes[b++] = fb.bytes[3];
            }
            return base64::encode (_bytes);
        }
        std::size_t vnorm_size() { return this->vertexNormals.size(); }
        std::string vnorm_max() { return this->vnorm_maxes.str_mat(); }
        std::string vnorm_min() { return this->vnorm_mins.str_mat(); }
        std::size_t vnorm_bytes() { return this->vertexNormals.size() * sizeof (float); }
        std::string vnorm_base64()
        {
            std::vector<std::uint8_t> _bytes (this->vertexNormals.size()<<2, 0);
            std::size_t b = 0u;
            float_bytes fb;
            for (auto i : this->vertexNormals) {
                fb.f = i;
                _bytes[b++] = fb.bytes[0];
                _bytes[b++] = fb.bytes[1];
                _bytes[b++] = fb.bytes[2];
                _bytes[b++] = fb.bytes[3];
            }
            return base64::encode (_bytes);
        }
        // end Visual::savegltf() methods

        //! If true, then this VisualModel should always be viewed in a plane - it's a 2D model
        bool twodimensional = false;

        //! The current indices index
        GLuint idx = 0u;

        //! Set scaling in all dimensions
        void setSizeScale (const float scl)
        {
            this->model_scaling.setToIdentity();
            this->model_scaling[0] = scl;
            this->model_scaling[5] = scl;
            this->model_scaling[10] = scl;
        }
        //! Set scaling in xy only
        void setSizeScale (const float xscl, const float yscl)
        {
            this->model_scaling.setToIdentity();
            this->model_scaling[0] = xscl;
            this->model_scaling[5] = yscl;
        }

        /*!
         * A function that will be runtime defined to get_shaderprogs from a pointer to
         * Visual (saving a boilerplate argument and avoiding that killer circular
         * dependency at the cost of one line of boilerplate in client programs)
         */
        std::function<morph::visgl::visual_shaderprogs(morph::VisualBase<glver>*)> get_shaderprogs;
        //! Get the graphics shader prog id
        std::function<GLuint(morph::VisualBase<glver>*)> get_gprog;
        //! Get the text shader prog id
        std::function<GLuint(morph::VisualBase<glver>*)> get_tprog;
#ifdef GLAD_OPTION_GL_MX
        //! Get the GladGLContext function pointer
        std::function<GladGLContext*(morph::VisualBase<glver>*)> get_glfn;
#endif
        //! Set OpenGL context. Should call parentVis->setContext().
        std::function<void(morph::VisualBase<glver>*)> setContext;
        //! Release OpenGL context. Should call parentVis->releaseContext().
        std::function<void(morph::VisualBase<glver>*)> releaseContext;

        //! Setter for the parent pointer, parentVis
        void set_parent (morph::VisualBase<glver>* _vis)
        {
            if (this->parentVis != nullptr) { throw std::runtime_error ("VisualModel: Set the parent pointer once only!"); }
            this->parentVis = _vis;
        }

    protected:

        //! The model-specific view matrix.
        mat44<float> viewmatrix;
        //! The model-specific scene view matrix.
        mat44<float> scenematrix;
        //! An additional scaling applied to viewmatrix to scale the size of the model [see render()]
        mat44<float> model_scaling;

        /*!
         * The spatial offset of this VisualModel within the morph::Visual 'scene
         * view'. Note that this is not incorporated into the computation of the
         * vertices, but is instead applied when the object is rendered as part of the
         * model->world transformation - it's applied as a translation in
         * VisualModel::viewmatrix.
         */
        vec<float> mv_offset;
        //! Model view rotation
        quaternion<float> mv_rotation;

        //! Scene view offset
        vec<float> sv_offset;
        //! Scene view rotation
        quaternion<float> sv_rotation;

        //! A vector of pointers to text models that should be rendered.
#ifdef GLAD_OPTION_GL_MX
        std::vector<std::unique_ptr<morph::VisualTextModelMX<glver>>> texts;
#else
        std::vector<std::unique_ptr<morph::VisualTextModel<glver>>> texts;
#endif

        //! This enum contains the positions within the vbo array of the different
        //! vertex buffer objects
        enum VBOPos { posnVBO, normVBO, colVBO, idxVBO, numVBO };

        //! A unit vector in the x direction
        morph::vec<float, 3> ux = { 1.0f, 0.0f, 0.0f };
        //! A unit vector in the y direction
        morph::vec<float, 3> uy = { 0.0f, 1.0f, 0.0f };
        //! A unit vector in the z direction
        morph::vec<float, 3> uz = { 0.0f, 0.0f, 1.0f };

        /*
         * Compute positions and colours of vertices for the hexes and store in these:
         */

        //! The OpenGL Vertex Array Object
        GLuint vao;

        //! Vertex Buffer Objects stored in an array
        std::unique_ptr<GLuint[]> vbos;

        //! CPU-side data for indices
        std::vector<GLuint> indices;
        //! CPU-side data for vertex positions
        std::vector<float> vertexPositions;
        //! CPU-side data for vertex normals
        std::vector<float> vertexNormals;
        //! CPU-side data for vertex colours
        std::vector<float> vertexColors;

        static constexpr float _max = std::numeric_limits<float>::max();
        static constexpr float _low = std::numeric_limits<float>::lowest();

        // The max and min values in the next 8 attributes are only computed if gltf files are going
        // to be output by Visual::savegltf()

        //! Max values of 0th, 1st and 2nd coordinates in vertexPositions
        morph::vec<float, 3> vpos_maxes = { _low, _low, _low };
        //! Min values in vertexPositions
        morph::vec<float, 3> vpos_mins = { _max, _max, _max };
        morph::vec<float, 3> vcol_maxes = { _low, _low, _low };
        morph::vec<float, 3> vcol_mins = { _max, _max, _max };
        morph::vec<float, 3> vnorm_maxes = { _low, _low, _low };
        morph::vec<float, 3> vnorm_mins = { _max, _max, _max };
        //! Max value in indices
        GLuint idx_max = 0u;
        //! Min value in indices.
        GLuint idx_min = std::numeric_limits<GLuint>::max();

        //! A model-wide alpha value for the shader
        float alpha = 1.0f;
        //! If true, then calls to VisualModel::render should return
        bool hide = false;

        // The morph::VisualBase in which this model exists.
        morph::VisualBase<glver>* parentVis = nullptr;

        //! Push three floats onto the vector of floats \a vp
        void vertex_push (const float& x, const float& y, const float& z, std::vector<float>& vp)
        {
            vec<float> vec = { x, y, z };
            std::copy (vec.begin(), vec.end(), std::back_inserter (vp));
        }
        //! Push array of 3 floats onto the vector of floats \a vp
        void vertex_push (const std::array<float, 3>& arr, std::vector<float>& vp)
        {
            std::copy (arr.begin(), arr.end(), std::back_inserter (vp));
        }
        //! Push morph::vec of 3 floats onto the vector of floats \a vp
        void vertex_push (const vec<float>& vec, std::vector<float>& vp)
        {
            std::copy (vec.begin(), vec.end(), std::back_inserter (vp));
        }

        //! Set up a vertex buffer object - bind, buffer and set vertex array object attribute
        void setupVBO (GLuint& buf, std::vector<float>& dat, unsigned int bufferAttribPosition)
        {
            std::size_t sz = dat.size() * sizeof(float);
#ifdef GLAD_OPTION_GL_MX
            GladGLContext* _glfn = this->get_glfn(this->parentVis);
            _glfn->BindBuffer (GL_ARRAY_BUFFER, buf);
            morph::gl::Util::checkError (__FILE__, __LINE__, _glfn);
            _glfn->BufferData (GL_ARRAY_BUFFER, sz, dat.data(), GL_STATIC_DRAW);
            morph::gl::Util::checkError (__FILE__, __LINE__, _glfn);
            _glfn->VertexAttribPointer (bufferAttribPosition, 3, GL_FLOAT, GL_FALSE, 0, (void*)(0));
            morph::gl::Util::checkError (__FILE__, __LINE__, _glfn);
            _glfn->EnableVertexAttribArray (bufferAttribPosition);
            morph::gl::Util::checkError (__FILE__, __LINE__, _glfn);
#else
            glBindBuffer (GL_ARRAY_BUFFER, buf);
            morph::gl::Util::checkError (__FILE__, __LINE__);
            glBufferData (GL_ARRAY_BUFFER, sz, dat.data(), GL_STATIC_DRAW);
            morph::gl::Util::checkError (__FILE__, __LINE__);
            glVertexAttribPointer (bufferAttribPosition, 3, GL_FLOAT, GL_FALSE, 0, (void*)(0));
            morph::gl::Util::checkError (__FILE__, __LINE__);
            glEnableVertexAttribArray (bufferAttribPosition);
            morph::gl::Util::checkError (__FILE__, __LINE__);
#endif
        }

        /*!
         * Create a tube from \a start to \a end, with radius \a r and a colour which
         * transitions from the colour \a colStart to \a colEnd.
         *
         * This version simply sub-calls into computeFlaredTube which will randomly choose the angle
         * of the vertices around the centre of each end cap.
         *
         * \param idx The index into the 'vertex array'
         * \param start The start of the tube
         * \param end The end of the tube
         * \param colStart The tube starting colour
         * \param colEnd The tube's ending colour
         * \param r Radius of the tube
         * \param segments Number of segments used to render the tube
         */
        void computeTube (vec<float> start, vec<float> end,
                          std::array<float, 3> colStart, std::array<float, 3> colEnd,
                          float r = 1.0f, int segments = 12)
        {
            this->computeFlaredTube (start, end, colStart, colEnd, r, r, segments);
        }

        /*!
         * Compute a tube. This version requires unit vectors for orientation of the
         * tube end faces/vertices (useful for graph markers). The other version uses a
         * randomly chosen vector to do this.
         *
         * Create a tube from \a start to \a end, with radius \a r and a colour which
         * transitions from the colour \a colStart to \a colEnd.
         *
         * \param start The start of the tube
         * \param end The end of the tube
         * \param _ux a vector in the x axis direction for the end face
         * \param _uy a vector in the y axis direction
         * \param colStart The tube starting colour
         * \param colEnd The tube's ending colour
         * \param r Radius of the tube
         * \param segments Number of segments used to render the tube
         * \param rotation A rotation in the _ux/_uy plane to orient the vertices of the
         * tube. Useful if this is to be a short tube used as a graph marker.
         */
        void computeTube (vec<float> start, vec<float> end,
                          vec<float> _ux, vec<float> _uy,
                          std::array<float, 3> colStart, std::array<float, 3> colEnd,
                          float r = 1.0f, int segments = 12, float rotation = 0.0f)
        {
            // The vector from start to end defines direction of the tube
            vec<float> vstart = start;
            vec<float> vend = end;

            // v is a face normal
            vec<float> v = _uy.cross(_ux);
            v.renormalize();

            // Push the central point of the start cap - this is at location vstart
            this->vertex_push (vstart, this->vertexPositions);
            this->vertex_push (-v, this->vertexNormals);
            this->vertex_push (colStart, this->vertexColors);

            // Start cap vertices (a triangle fan)
            for (int j = 0; j < segments; j++) {
                // t is the angle of the segment
                float t = rotation + j * morph::mathconst<float>::two_pi/(float)segments;
                vec<float> c = _ux * std::sin(t) * r + _uy * std::cos(t) * r;
                this->vertex_push (vstart+c, this->vertexPositions);
                this->vertex_push (-v, this->vertexNormals);
                this->vertex_push (colStart, this->vertexColors);
            }

            // Intermediate, near start cap. Normals point in direction c
            for (int j = 0; j < segments; j++) {
                float t = rotation + j * morph::mathconst<float>::two_pi/(float)segments;
                vec<float> c = _ux * std::sin(t) * r + _uy * std::cos(t) * r;
                this->vertex_push (vstart+c, this->vertexPositions);
                c.renormalize();
                this->vertex_push (c, this->vertexNormals);
                this->vertex_push (colStart, this->vertexColors);
            }

            // Intermediate, near end cap. Normals point in direction c
            for (int j = 0; j < segments; j++) {
                float t = rotation + (float)j * morph::mathconst<float>::two_pi/(float)segments;
                vec<float> c = _ux * std::sin(t) * r + _uy * std::cos(t) * r;
                this->vertex_push (vend+c, this->vertexPositions);
                c.renormalize();
                this->vertex_push (c, this->vertexNormals);
                this->vertex_push (colEnd, this->vertexColors);
            }

            // Bottom cap vertices
            for (int j = 0; j < segments; j++) {
                float t = rotation + (float)j * morph::mathconst<float>::two_pi/(float)segments;
                vec<float> c = _ux * std::sin(t) * r + _uy * std::cos(t) * r;
                this->vertex_push (vend+c, this->vertexPositions);
                this->vertex_push (v, this->vertexNormals);
                this->vertex_push (colEnd, this->vertexColors);
            }

            // Bottom cap. Push centre vertex as the last vertex.
            this->vertex_push (vend, this->vertexPositions);
            this->vertex_push (v, this->vertexNormals);
            this->vertex_push (colEnd, this->vertexColors);

            // Number of vertices = segments * 4 + 2.
            int nverts = (segments * 4) + 2;

            // After creating vertices, push all the indices.
            GLuint capMiddle = this->idx;
            GLuint capStartIdx = this->idx + 1u;
            GLuint endMiddle = this->idx + (GLuint)nverts - 1u;
            GLuint endStartIdx = capStartIdx + (3u * segments);

            // Start cap indices
            for (int j = 0; j < segments-1; j++) {
                this->indices.push_back (capMiddle);
                this->indices.push_back (capStartIdx + j);
                this->indices.push_back (capStartIdx + 1 + j);
            }
            // Last one
            this->indices.push_back (capMiddle);
            this->indices.push_back (capStartIdx + segments - 1);
            this->indices.push_back (capStartIdx);

            // Middle sections
            for (int lsection = 0; lsection < 3; ++lsection) {
                capStartIdx = this->idx + 1 + lsection*segments;
                endStartIdx = capStartIdx + segments;
                for (int j = 0; j < segments; j++) {
                    this->indices.push_back (capStartIdx + j);
                    if (j == (segments-1)) {
                        this->indices.push_back (capStartIdx);
                    } else {
                        this->indices.push_back (capStartIdx + 1 + j);
                    }
                    this->indices.push_back (endStartIdx + j);
                    this->indices.push_back (endStartIdx + j);
                    if (j == (segments-1)) {
                        this->indices.push_back (endStartIdx);
                    } else {
                        this->indices.push_back (endStartIdx + 1 + j);
                    }
                    if (j == (segments-1)) {
                        this->indices.push_back (capStartIdx);
                    } else {
                        this->indices.push_back (capStartIdx + j + 1);
                    }
                }
            }

            // bottom cap
            for (int j = 0; j < segments-1; j++) {
                this->indices.push_back (endMiddle);
                this->indices.push_back (endStartIdx + j);
                this->indices.push_back (endStartIdx + 1 + j);
            }
            this->indices.push_back (endMiddle);
            this->indices.push_back (endStartIdx + segments - 1);
            this->indices.push_back (endStartIdx);

            // Update idx
            this->idx += nverts;
        } // end computeTube with ux/uy vectors for faces

        /*!
         * A 'draw an arrow' primitive. This is a 3D, tubular arrow made of a tube and a cone.
         *
         * \param start Start coordinate of the arrow
         *
         * \param end End coordinate of the arrow
         *
         * \param clr The colour for the arrow
         *
         * \param tube_radius Radius of arrow shaft. If < 0, then set from (end-start).length()
         *
         * \param arrowhead_prop The proportion of the arrow length that the head should take up
         *
         * \param cone_radius Radisu of cone that make the arrow head. If < 0, then set from
         * tube_radius
         *
         * \param shapesides How many facets to draw tube/cone with
         */
        void computeArrow (const vec<float>& start, const vec<float>& end,
                           const std::array<float, 3> clr,
                           float tube_radius = -1.0f,
                           float arrowhead_prop = -1.0f,
                           float cone_radius = -1.0f,
                           const int shapesides = 18)
        {
            // The right way to draw an arrow.
            vec<float> arrow_line = end - start;
            float len = arrow_line.length();
            // Unless client code specifies, compute tube radius from length of arrow
            if (tube_radius < 0.0f) { tube_radius = len / 40.0f; }
            if (arrowhead_prop < 0.0f) { arrowhead_prop = 0.15f; }
            if (cone_radius < 0.0f) { cone_radius = 1.75f * tube_radius; }
            // We don't draw the full tube
            vec<float> cone_start = arrow_line.shorten (len * arrowhead_prop);
            cone_start += start;
            this->computeTube (start, cone_start, clr, clr, tube_radius, shapesides);
            float conelen = (end-cone_start).length();
            if (arrow_line.length() > conelen) {
                this->computeCone (cone_start, end, 0.0f, clr, cone_radius, shapesides);
            }
        }

        /*!
         * Create a flared tube from \a start to \a end, with radius \a r at the start and a colour
         * which transitions from the colour \a colStart to \a colEnd. The radius of the end is
         * determined by the given angle, flare, in radians.
         *
         * \param idx The index into the 'vertex array'
         * \param start The start of the tube
         * \param end The end of the tube
         * \param colStart The tube starting colour
         * \param colEnd The tube's ending colour
         * \param r Radius of the tube
         * \param segments Number of segments used to render the tube
         * \param flare The angle, measured wrt the direction of the tube in radians, by which the
         * tube 'flares'
         */
        void computeFlaredTube (morph::vec<float> start, morph::vec<float> end,
                                std::array<float, 3> colStart, std::array<float, 3> colEnd,
                                float r = 1.0f, int segments = 12, float flare = 0.0f)
        {
            // Find the length of the tube
            morph::vec<float> v = end - start;
            float l = v.length();
            // Compute end radius from the length and the flare angle:
            float r_add = l * std::tan (std::abs(flare)) * (flare > 0.0f ? 1.0f : -1.0f);
            float r_end = r + r_add;
            // Now call into the other overload:
            this->computeFlaredTube (start, end, colStart, colEnd, r, r_end, segments);
        }

        /*!
         * Create a flared tube from \a start to \a end, with radius \a r at the start and a colour
         * which transitions from the colour \a colStart to \a colEnd. The radius of the end is
         * r_end, given as a function argument.
         *
         * \param start The start of the tube
         * \param end The end of the tube
         * \param colStart The tube starting colour
         * \param colEnd The tube's ending colour
         * \param r Radius of the tube's start cap
         * \param r_end radius of the end cap
         * \param segments Number of segments used to render the tube
         */
        void computeFlaredTube (morph::vec<float> start, morph::vec<float> end,
                                std::array<float, 3> colStart, std::array<float, 3> colEnd,
                                float r = 1.0f, float r_end = 1.0f, int segments = 12)
        {
            // The vector from start to end defines a vector and a plane. Find a
            // 'circle' of points in that plane.
            morph::vec<float> vstart = start;
            morph::vec<float> vend = end;
            morph::vec<float> v = vend - vstart;
            v.renormalize();

            // circle in a plane defined by a point (v0 = vstart or vend) and a normal
            // (v) can be found: Choose random vector vr. A vector inplane = vr ^ v. The
            // unit in-plane vector is inplane.normalise. Can now use that vector in the
            // plan to define a point on the circle. Note that this starting point on
            // the circle is at a random position, which means that this version of
            // computeTube is useful for tubes that have quite a few segments.
            morph::vec<float> rand_vec;
            rand_vec.randomize();
            morph::vec<float> inplane = rand_vec.cross(v);
            inplane.renormalize();

            // Now use parameterization of circle inplane = p1-x1 and
            // c1(t) = ( (p1-x1).normalized std::sin(t) + v.normalized cross (p1-x1).normalized * std::cos(t) )
            // c1(t) = ( inplane std::sin(t) + v * inplane * std::cos(t)
            morph::vec<float> v_x_inplane = v.cross(inplane);

            // Push the central point of the start cap - this is at location vstart
            this->vertex_push (vstart, this->vertexPositions);
            this->vertex_push (-v, this->vertexNormals);
            this->vertex_push (colStart, this->vertexColors);

            // Start cap vertices. Draw as a triangle fan, but record indices so that we
            // only need a single call to glDrawElements.
            for (int j = 0; j < segments; j++) {
                // t is the angle of the segment
                float t = j * morph::mathconst<float>::two_pi/(float)segments;
                morph::vec<float> c = inplane * std::sin(t) * r + v_x_inplane * std::cos(t) * r;
                this->vertex_push (vstart+c, this->vertexPositions);
                this->vertex_push (-v, this->vertexNormals);
                this->vertex_push (colStart, this->vertexColors);
            }

            // Intermediate, near start cap. Normals point in direction c
            for (int j = 0; j < segments; j++) {
                float t = j * morph::mathconst<float>::two_pi/(float)segments;
                morph::vec<float> c = inplane * std::sin(t) * r + v_x_inplane * std::cos(t) * r;
                this->vertex_push (vstart+c, this->vertexPositions);
                c.renormalize();
                this->vertex_push (c, this->vertexNormals);
                this->vertex_push (colStart, this->vertexColors);
            }

            // Intermediate, near end cap. Normals point in direction c
            for (int j = 0; j < segments; j++) {
                float t = (float)j * morph::mathconst<float>::two_pi/(float)segments;
                morph::vec<float> c = inplane * std::sin(t) * r_end + v_x_inplane * std::cos(t) * r_end;
                this->vertex_push (vend+c, this->vertexPositions);
                c.renormalize();
                this->vertex_push (c, this->vertexNormals);
                this->vertex_push (colEnd, this->vertexColors);
            }

            // Bottom cap vertices
            for (int j = 0; j < segments; j++) {
                float t = (float)j * morph::mathconst<float>::two_pi/(float)segments;
                morph::vec<float> c = inplane * std::sin(t) * r_end + v_x_inplane * std::cos(t) * r_end;
                this->vertex_push (vend+c, this->vertexPositions);
                this->vertex_push (v, this->vertexNormals);
                this->vertex_push (colEnd, this->vertexColors);
            }

            // Bottom cap. Push centre vertex as the last vertex.
            this->vertex_push (vend, this->vertexPositions);
            this->vertex_push (v, this->vertexNormals);
            this->vertex_push (colEnd, this->vertexColors);

            // Note: number of vertices = segments * 4 + 2.
            int nverts = (segments * 4) + 2;

            // After creating vertices, push all the indices.
            GLuint capMiddle = this->idx;
            GLuint capStartIdx = this->idx + 1u;
            GLuint endMiddle = this->idx + (GLuint)nverts - 1u;
            GLuint endStartIdx = capStartIdx + (3u * segments);

            // Start cap
            for (int j = 0; j < segments-1; j++) {
                this->indices.push_back (capMiddle);
                this->indices.push_back (capStartIdx + j);
                this->indices.push_back (capStartIdx + 1 + j);
            }
            // Last one
            this->indices.push_back (capMiddle);
            this->indices.push_back (capStartIdx + segments - 1);
            this->indices.push_back (capStartIdx);

            // Middle sections
            for (int lsection = 0; lsection < 3; ++lsection) {
                capStartIdx = this->idx + 1 + lsection*segments;
                endStartIdx = capStartIdx + segments;
                // This does sides between start and end. I want to do this three times.
                for (int j = 0; j < segments; j++) {
                    // Triangle 1
                    this->indices.push_back (capStartIdx + j);
                    if (j == (segments-1)) {
                        this->indices.push_back (capStartIdx);
                    } else {
                        this->indices.push_back (capStartIdx + 1 + j);
                    }
                    this->indices.push_back (endStartIdx + j);
                    // Triangle 2
                    this->indices.push_back (endStartIdx + j);
                    if (j == (segments-1)) {
                        this->indices.push_back (endStartIdx);
                    } else {
                        this->indices.push_back (endStartIdx + 1 + j);
                    }
                    if (j == (segments-1)) {
                        this->indices.push_back (capStartIdx);
                    } else {
                        this->indices.push_back (capStartIdx + j + 1);
                    }
                }
            }

            // Bottom cap
            for (int j = 0; j < segments-1; j++) {
                this->indices.push_back (endMiddle);
                this->indices.push_back (endStartIdx + j);
                this->indices.push_back (endStartIdx + 1 + j);
            }
            // Last one
            this->indices.push_back (endMiddle);
            this->indices.push_back (endStartIdx + segments - 1);
            this->indices.push_back (endStartIdx);

            // Update idx
            this->idx += nverts;
        } // end computeFlaredTube with randomly initialized end vertices

        /*!
         * Create an open (no end caps) flared tube from \a start to \a end, with radius
         * \a r at the start and a colour which transitions from the colour \a colStart
         * to \a colEnd. The radius of the end is r_end, given as a function argument.
         *
         * This has a normal vector for the start and end of the tube, so that the
         * circles can be angled.
         *
         * \param start The start of the tube
         * \param end The end of the tube
         * \param colStart The tube starting colour
         * \param colEnd The tube's ending colour
         * \param n_start The normal of the start 'face'
         * \param n_end The normal of the end 'face'
         *
         * \param z_start A vector pointing to the first vertex on the tube. allows
         * orientation of tube faces for connected tubes (which is what this primitive
         * is all about)
         *
         * \param r Radius of the tube's start circle
         * \param r_end radius of the end circle
         * \param segments Number of segments used to render the tube
         */
        void computeOpenFlaredTube (morph::vec<float> start, morph::vec<float> end,
                                    morph::vec<float> n_start, morph::vec<float> n_end,
                                    std::array<float, 3> colStart, std::array<float, 3> colEnd,
                                    float r = 1.0f, float r_end = 1.0f, int segments = 12)
        {
            // The vector from start to end defines a vector and a plane. Find a
            // 'circle' of points in that plane.
            morph::vec<float> vstart = start;
            morph::vec<float> vend = end;
            morph::vec<float> v = vend - vstart;
            v.renormalize();

            // Two rotations about our face normals
            morph::quaternion<float> rotn_start (n_start, morph::mathconst<float>::pi_over_2);
            morph::quaternion<float> rotn_end (-n_end, morph::mathconst<float>::pi_over_2);

            morph::vec<float> inplane = v.cross (n_start);
            // The above is no good if n_start and v are colinear. In that case choose random inplane:
            if (inplane.length() < std::numeric_limits<float>::epsilon()) {
                vec<float> rand_vec;
                rand_vec.randomize();
                inplane = rand_vec.cross(v);
            }
            inplane.renormalize();

            // inplane defines a plane, n_start defines a plane. Our first point is the
            // intersection of the two planes and the circle of the end.
            morph::vec<float> v_x_inplane = n_start.cross (inplane);// rotn_start * inplane;
            v_x_inplane.renormalize();

            // If r == r_end we want a circular cross section tube (and not an elliptical cross section).
            float r_mod = r / v_x_inplane.cross (v).length();

            // Start ring of vertices. Normals point in direction c
            // Now use parameterization of circle inplane = p1-x1 and
            // c1(t) = ( (p1-x1).normalized std::sin(t) + v.normalized cross (p1-x1).normalized * std::cos(t) )
            // c1(t) = ( inplane std::sin(t) + v * inplane * std::cos(t)
            for (int j = 0; j < segments; j++) {
                float t = j * morph::mathconst<float>::two_pi/(float)segments;
                morph::vec<float> c = inplane * std::sin(t) * r + v_x_inplane * std::cos(t) * r_mod;
                this->vertex_push (vstart+c, this->vertexPositions);
                c.renormalize();
                this->vertex_push (c, this->vertexNormals);
                this->vertex_push (colStart, this->vertexColors);
            }

            // end ring of vertices. Normals point in direction c
            v_x_inplane = inplane.cross (n_end);
            v_x_inplane.renormalize();
            r_mod = r_end / v_x_inplane.cross (v).length();

            for (int j = 0; j < segments; j++) {
                float t = (float)j * morph::mathconst<float>::two_pi/(float)segments;
                morph::vec<float> c = inplane * std::sin(t) * r_end + v_x_inplane * std::cos(t) * r_mod;
                this->vertex_push (vend+c, this->vertexPositions);
                c.renormalize();
                this->vertex_push (c, this->vertexNormals);
                this->vertex_push (colEnd, this->vertexColors);
            }

            // Number of vertices
            int nverts = (segments * 2);

            // After creating vertices, push all the indices.
            GLuint sIdx = this->idx;
            GLuint eIdx = sIdx + segments;
            // This does sides between start and end
            for (int j = 0; j < segments; j++) {
                // Triangle 1
                this->indices.push_back (sIdx + j);
                if (j == (segments-1)) {
                    this->indices.push_back (sIdx);
                } else {
                    this->indices.push_back (sIdx + 1 + j);
                }
                this->indices.push_back (eIdx + j);
                // Triangle 2
                this->indices.push_back (eIdx + j);
                if (j == (segments-1)) {
                    this->indices.push_back (eIdx);
                } else {
                    this->indices.push_back (eIdx + 1 + j);
                }
                if (j == (segments-1)) {
                    this->indices.push_back (sIdx);
                } else {
                    this->indices.push_back (sIdx + j + 1);
                }
            }

            // Update idx
            this->idx += nverts;
        } // end computeOpenFlaredTube

        // An open, but un-flared tube with no end caps
        void computeOpenTube (morph::vec<float> start, morph::vec<float> end,
                              morph::vec<float> n_start, morph::vec<float> n_end,
                              std::array<float, 3> colStart, std::array<float, 3> colEnd,
                              float r = 1.0f, int segments = 12)
        {
            this->computeOpenFlaredTube (start, end, n_start, n_end, colStart, colEnd, r, r, segments);
        }


        //! Compute a Quad from 4 arbitrary corners which must be ordered clockwise around the quad.
        void computeFlatQuad (vec<float> c1, vec<float> c2,
                              vec<float> c3, vec<float> c4,
                              std::array<float, 3> col)
        {
            // v is the face normal
            vec<float> u1 = c1-c2;
            vec<float> u2 = c2-c3;
            vec<float> v = u2.cross(u1);
            v.renormalize();

            // Push corner vertices
            size_t vpsz = this->vertexPositions.size();
            this->vertexPositions.resize (vpsz + 12);
            for (unsigned int i = 0; i < 3u; ++i) { this->vertexPositions[vpsz++] = c1[i]; }
            for (unsigned int i = 0; i < 3u; ++i) { this->vertexPositions[vpsz++] = c2[i]; }
            for (unsigned int i = 0; i < 3u; ++i) { this->vertexPositions[vpsz++] = c3[i]; }
            for (unsigned int i = 0; i < 3u; ++i) { this->vertexPositions[vpsz++] = c4[i]; }

            // Colours/normals
            size_t vcsz = this->vertexColors.size();
            size_t vnsz = this->vertexNormals.size();
            this->vertexColors.resize (vcsz + 12);
            this->vertexNormals.resize (vnsz + 12);
            for (unsigned int i = 0; i < 4u; ++i) {
                for (unsigned int j = 0; j < 3u; ++j) {
                    this->vertexColors[vcsz++] = col[j];
                    this->vertexNormals[vnsz++] = v[j];
                }
            }

            size_t i0 = this->indices.size();
            this->indices.resize (i0 + 6, 0);
            this->indices[i0++] = this->idx;
            this->indices[i0++] = this->idx + 1;
            this->indices[i0++] = this->idx + 2;
            this->indices[i0++] = this->idx;
            this->indices[i0++] = this->idx + 2;
            this->indices[i0++] = this->idx + 3;

            this->idx += 4;
        }

        /*!
         * Compute a tube. This version requires unit vectors for orientation of the
         * tube end faces/vertices (useful for graph markers). The other version uses a
         * randomly chosen vector to do this.
         *
         * Create a tube from \a start to \a end, with radius \a r and a colour which
         * transitions from the colour \a colStart to \a colEnd.
         *
         * \param idx The index into the 'vertex array'
         * \param vstart The centre of the polygon
         * \param _ux a vector in the x axis direction for the end face
         * \param _uy a vector in the y axis direction
         * \param col The polygon colour
         * \param r Radius of the tube
         * \param segments Number of segments used to render the tube
         * \param rotation A rotation in the ux/uy plane to orient the vertices of the
         * tube. Useful if this is to be a short tube used as a graph marker.
         */
        void computeFlatPoly (vec<float> vstart,
                              vec<float> _ux, vec<float> _uy,
                              std::array<float, 3> col,
                              float r = 1.0f, int segments = 12, float rotation = 0.0f)
        {
            // v is a face normal
            vec<float> v = _uy.cross(_ux);
            v.renormalize();

            // Push the central point of the start cap - this is at location vstart
            this->vertex_push (vstart, this->vertexPositions);
            this->vertex_push (-v, this->vertexNormals);
            this->vertex_push (col, this->vertexColors);

            // Polygon vertices (a triangle fan)
            for (int j = 0; j < segments; j++) {
                // t is the angle of the segment
                float t = rotation + j * morph::mathconst<float>::two_pi/(float)segments;
                vec<float> c = _ux * std::sin(t) * r + _uy * std::cos(t) * r;
                this->vertex_push (vstart+c, this->vertexPositions);
                this->vertex_push (-v, this->vertexNormals);
                this->vertex_push (col, this->vertexColors);
            }

            // Number of vertices
            int nverts = segments + 1;

            // After creating vertices, push all the indices.
            GLuint capMiddle = this->idx;
            GLuint capStartIdx = this->idx + 1;

            // Start cap indices
            for (int j = 0; j < segments-1; j++) {
                this->indices.push_back (capMiddle);
                this->indices.push_back (capStartIdx + j);
                this->indices.push_back (capStartIdx + 1 + j);
            }
            // Last one
            this->indices.push_back (capMiddle);
            this->indices.push_back (capStartIdx + segments - 1);
            this->indices.push_back (capStartIdx);

            // Update idx
            this->idx += nverts;
        } // end computeFlatPloy with ux/uy vectors for faces

        /*!
         * Make a ring of radius r, comprised of flat segments
         *
         * \param ro position of the centre of the ring
         * \param rc The ring colour.
         * \param r Radius of the ring
         * \param t Thickness of the ring
         * \param segments Number of tube segments used to render the ring
         */
        void computeRing (vec<float> ro, std::array<float, 3> rc, float r = 1.0f,
                          float t = 0.1f, int segments = 12)
        {
            for (int j = 0; j < segments; j++) {
                float segment = morph::mathconst<float>::two_pi * static_cast<float>(j) / segments;
                // x and y of inner point
                float xin = (r-(t*0.5f)) * std::cos(segment);
                float yin = (r-(t*0.5f)) * std::sin(segment);
                float xout = (r+(t*0.5f)) * std::cos(segment);
                float yout = (r+(t*0.5f)) * std::sin(segment);
                int segjnext = (j+1) % segments;
                float segnext = morph::mathconst<float>::two_pi * static_cast<float>(segjnext) / segments;
                float xin_n = (r-(t*0.5f)) * std::cos(segnext);
                float yin_n = (r-(t*0.5f)) * std::sin(segnext);
                float xout_n = (r+(t*0.5f)) * std::cos(segnext);
                float yout_n = (r+(t*0.5f)) * std::sin(segnext);

                // Now draw a quad
                vec<float> c4 = { xin, yin, 0.0f };
                vec<float> c3 = { xout, yout, 0.0f };
                vec<float> c2 = { xout_n, yout_n, 0.0f };
                vec<float> c1 = { xin_n, yin_n, 0.0f };
                this->computeFlatQuad (ro+c1, ro+c2, ro+c3, ro+c4, rc);
            }
        }

        /*!
         * Sphere, geodesic polygon version.
         *
         * This function creates an object with exactly one OpenGL vertex per 'geometric
         * vertex of the polyhedron'. That means that colouring this object must be
         * achieved by colouring the vertices and faces cannot be coloured
         * distinctly. Pass in a single colour for the initial object. To recolour,
         * modify the content of vertexColors.
         *
         * \tparam F The type used for the polyhedron computation. Use float or double.
         *
         * \param so The sphere offset. Where to place this sphere...
         * \param sc The sphere colour.
         * \param r Radius of the sphere
         * \param iterations how many iterations of the geodesic polygon algo to go
         * through. Determines faces:
         *
         * For 0 iterations, get a geodesic with 20 faces        *0
         * For 1 iterations, get a geodesic with 80 faces
         * For 2 iterations, get a geodesic with 320 faces       *1
         * For 3 iterations, get a geodesic with 1280 faces      *2
         * For 4 iterations, get a geodesic with 5120 faces      *3
         * For 5 iterations, get a geodesic with 20480 faces     *4
         * For 6 iterations, get a geodesic with 81920 faces
         * For 7 iterations, get a geodesic with 327680 faces
         * For 8 iterations, get a geodesic with 1310720 faces
         * For 9 iterations, get a geodesic with 5242880 faces
         *
         * *0: You'll get an icosahedron
         * *1: decent graphical results
         * *2: excellent graphical results
         * *3: You can *just about* see a difference between 4 iterations and 3, but not
         *  between 4 and 5.
         * *4: The iterations limit if F is float (you'll get a runtime error 'vertices
         *  has wrong size' for iterations>5)
         *
         * \return The number of vertices in the generated geodesic sphere
         */
        template<typename F=float>
        int computeSphereGeo (vec<float> so, std::array<float, 3> sc, float r = 1.0f, int iterations = 2)
        {
            if (iterations < 0) { throw std::runtime_error ("computeSphereGeo: iterations must be positive"); }
            // test if type F is float
            if constexpr (std::is_same<std::decay_t<F>, float>::value == true) {
                if (iterations > 5) {
                    throw std::runtime_error ("computeSphereGeo: For iterations > 5, F needs to be double precision");
                }
            } else {
                if (iterations > 10) {
                    throw std::runtime_error ("computeSphereGeo: This is an abitrary iterations limit (10 gives 20971520 faces)");
                }
            }
            // Note that we need double precision to compute higher iterations of the geodesic (iterations > 5)
            morph::geometry::icosahedral_geodesic<F> geo = morph::geometry::make_icosahedral_geodesic<F> (iterations);

            // Now essentially copy geo into vertex buffers
            for (auto v : geo.poly.vertices) {
                this->vertex_push (v.as_float() * r + so, this->vertexPositions);
                this->vertex_push (v.as_float(), this->vertexNormals);
                this->vertex_push (sc, this->vertexColors);
            }
            for (auto f : geo.poly.faces) {
                this->indices.push_back (this->idx + f[0]);
                this->indices.push_back (this->idx + f[1]);
                this->indices.push_back (this->idx + f[2]);
            }
            // idx is the *vertex index* and should be incremented by the number of vertices in the polyhedron
            int n_verts = static_cast<int>(geo.poly.vertices.size());
            this->idx += n_verts;

            return n_verts;
        }

        /*!
         * Sphere, geodesic polygon version with coloured faces
         *
         * To colour the faces of this polyhedron, update this->vertexColors (for an
         * example see morph::GeodesicVisual). To make faces distinctly colourizable, we
         * have to generate 3 OpenGL vertices for each of the geometric vertices in the
         * polyhedron.
         *
         * \tparam F The type used for the polyhedron computation. Use float or double.
         *
         * \param so The sphere offset. Where to place this sphere...
         * \param sc The default colour
         * \param r Radius of the sphere
         * \param iterations how many iterations of the geodesic polygon algo to go
         * through. Determines number of faces
         */
        template<typename F=float>
        int computeSphereGeoFaces (morph::vec<float> so, std::array<float, 3> sc, float r = 1.0f, int iterations = 2)
        {
            if (iterations < 0) { throw std::runtime_error ("computeSphereGeo: iterations must be positive"); }
            // test if type F is float
            if constexpr (std::is_same<std::decay_t<F>, float>::value == true) {
                if (iterations > 5) {
                    throw std::runtime_error ("computeSphereGeo: For iterations > 5, F needs to be double precision");
                }
            } else {
                if (iterations > 10) {
                    throw std::runtime_error ("computeSphereGeo: This is an abitrary iterations limit (10 gives 20971520 faces)");
                }
            }
            // Note that we need double precision to compute higher iterations of the geodesic (iterations > 5)
            morph::geometry::icosahedral_geodesic<F> geo = morph::geometry::make_icosahedral_geodesic<F> (iterations);
            int n_faces = static_cast<int>(geo.poly.faces.size());

            for (int i = 0; i < n_faces; ++i) { // For each face in the geodesic...
                morph::vec<F, 3> norm = { F{0}, F{0}, F{0} };
                for (auto vtx : geo.poly.faces[i]) { // For each vertex in face...
                    norm += vtx; // Add to the face norm
                    this->vertex_push (geo.poly.vertices[vtx].as_float() * r + so, this->vertexPositions);
                }
                morph::vec<float, 3> nf = (norm / F{3}).as_float();
                for (int j = 0; j < 3; ++j) { // Faces all have size 3
                    this->vertex_push (nf, this->vertexNormals);
                    this->vertex_push (sc, this->vertexColors); // A default colour
                    this->indices.push_back (this->idx + (3 * i) + j); // indices is vertex index
                }
            }
            // An index for each vertex of each face.
            this->idx += 3 * n_faces;

            return n_faces;
        }

        //! Fast computeSphereGeo, which uses constexpr make_icosahedral_geodesic. The
        //! resulting vertices and faces are NOT in any kind of order, but ok for
        //! plotting, e.g. scatter graph spheres.
        template<typename F=float, int iterations = 2>
        int computeSphereGeoFast (vec<float> so, std::array<float, 3> sc, float r = 1.0f)
        {
            // test if type F is float
            if constexpr (std::is_same<std::decay_t<F>, float>::value == true) {
                static_assert (iterations <= 5, "computeSphereGeoFast: For iterations > 5, F needs to be double precision");
            } else {
                static_assert (iterations <= 10, "computeSphereGeoFast: This is an abitrary iterations limit (10 gives 20971520 faces)");
            }
            // Note that we need double precision to compute higher iterations of the geodesic (iterations > 5)
            constexpr morph::geometry_ce::icosahedral_geodesic<F, iterations>  geo = morph::geometry_ce::make_icosahedral_geodesic<F, iterations>();

            // Now essentially copy geo into vertex buffers
            for (auto v : geo.poly.vertices) {
                this->vertex_push (v.as_float() * r + so, this->vertexPositions);
                this->vertex_push (v.as_float(), this->vertexNormals);
                this->vertex_push (sc, this->vertexColors);
            }
            for (auto f : geo.poly.faces) {
                this->indices.push_back (this->idx + f[0]);
                this->indices.push_back (this->idx + f[1]);
                this->indices.push_back (this->idx + f[2]);
            }
            // idx is the *vertex index* and should be incremented by the number of vertices in the polyhedron
            int n_verts = static_cast<int>(geo.poly.vertices.size());
            this->idx += n_verts;

            return n_verts;
        }

        /*!
         * Sphere, 1 colour version.
         *
         * Code for creating a sphere as part of this model. I'll use a sphere at the centre of the arrows.
         *
         * \param so The sphere offset. Where to place this sphere...
         * \param sc The sphere colour.
         * \param r Radius of the sphere
         * \param rings Number of rings used to render the sphere
         * \param segments Number of segments used to render the sphere
         *
         * Number of faces should be (2 + rings) * segments
         */
        void computeSphere (vec<float> so, std::array<float, 3> sc,
                            float r = 1.0f, int rings = 10, int segments = 12)
        {
            // First cap, draw as a triangle fan, but record indices so that
            // we only need a single call to glDrawElements.
            float rings0 = -morph::mathconst<float>::pi_over_2;
            float _z0  = std::sin(rings0);
            float z0  = r * _z0;
            float r0 =  std::cos(rings0);
            float rings1 = morph::mathconst<float>::pi * (-0.5f + 1.0f / rings);
            float _z1 = std::sin(rings1);
            float z1 = r * _z1;
            float r1 = std::cos(rings1);
            // Push the central point
            this->vertex_push (so[0]+0.0f, so[1]+0.0f, so[2]+z0, this->vertexPositions);
            this->vertex_push (0.0f, 0.0f, -1.0f, this->vertexNormals);
            this->vertex_push (sc, this->vertexColors);

            GLuint capMiddle = this->idx++;
            GLuint ringStartIdx = this->idx;
            GLuint lastRingStartIdx = this->idx;

            bool firstseg = true;
            for (int j = 0; j < segments; j++) {
                float segment = morph::mathconst<float>::two_pi * static_cast<float>(j) / segments;
                float x = std::cos(segment);
                float y = std::sin(segment);

                float _x1 = x*r1;
                float x1 = _x1*r;
                float _y1 = y*r1;
                float y1 = _y1*r;

                this->vertex_push (so[0]+x1, so[1]+y1, so[2]+z1, this->vertexPositions);
                this->vertex_push (_x1, _y1, _z1, this->vertexNormals);
                this->vertex_push (sc, this->vertexColors);

                if (!firstseg) {
                    this->indices.push_back (capMiddle);
                    this->indices.push_back (this->idx-1);
                    this->indices.push_back (this->idx++);
                } else {
                    this->idx++;
                    firstseg = false;
                }
            }
            this->indices.push_back (capMiddle);
            this->indices.push_back (this->idx-1);
            this->indices.push_back (capMiddle+1);

            // Now add the triangles around the rings
            for (int i = 2; i < rings; i++) {

                rings0 = morph::mathconst<float>::pi * (-0.5f + static_cast<float>(i) / rings);
                _z0  = std::sin(rings0);
                z0  = r * _z0;
                r0 =  std::cos(rings0);

                for (int j = 0; j < segments; j++) {

                    // "current" segment
                    float segment = morph::mathconst<float>::two_pi * static_cast<float>(j) / segments;
                    float x = std::cos(segment);
                    float y = std::sin(segment);

                    // One vertex per segment
                    float _x0 = x*r0;
                    float x0 = _x0*r;
                    float _y0 = y*r0;
                    float y0 = _y0*r;

                    // NB: Only add ONE vertex per segment. ALREADY have the first ring!
                    this->vertex_push (so[0]+x0, so[1]+y0, so[2]+z0, this->vertexPositions);
                    // The vertex normal of a vertex that makes up a sphere is
                    // just a normal vector in the direction of the vertex.
                    this->vertex_push (_x0, _y0, _z0, this->vertexNormals);
                    this->vertex_push (sc, this->vertexColors);

                    if (j == segments - 1) {
                        // Last vertex is back to the start
                        this->indices.push_back (ringStartIdx++);
                        this->indices.push_back (this->idx);
                        this->indices.push_back (lastRingStartIdx);
                        this->indices.push_back (lastRingStartIdx);
                        this->indices.push_back (this->idx++);
                        this->indices.push_back (lastRingStartIdx+segments);
                    } else {
                        this->indices.push_back (ringStartIdx++);
                        this->indices.push_back (this->idx);
                        this->indices.push_back (ringStartIdx);
                        this->indices.push_back (ringStartIdx);
                        this->indices.push_back (this->idx++);
                        this->indices.push_back (this->idx);
                    }
                }
                lastRingStartIdx += segments;
            }

            // bottom cap
            rings0 = morph::mathconst<float>::pi_over_2;
            _z0  = std::sin(rings0);
            z0  = r * _z0;
            r0 =  std::cos(rings0);
            // Push the central point of the bottom cap
            this->vertex_push (so[0]+0.0f, so[1]+0.0f, so[2]+z0, this->vertexPositions);
            this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
            this->vertex_push (sc, this->vertexColors);
            capMiddle = this->idx++;
            firstseg = true;
            // No more vertices to push, just do the indices for the bottom cap
            ringStartIdx = lastRingStartIdx;
            for (int j = 0; j < segments; j++) {
                if (j != segments - 1) {
                    this->indices.push_back (capMiddle);
                    this->indices.push_back (ringStartIdx++);
                    this->indices.push_back (ringStartIdx);
                } else {
                    // Last segment
                    this->indices.push_back (capMiddle);
                    this->indices.push_back (ringStartIdx);
                    this->indices.push_back (lastRingStartIdx);
                }
            }
        } // end of sphere calculation

        /*!
         * Sphere, two colour version.
         *
         * Code for creating a sphere as part of this model. I'll use a sphere at the
         * centre of the arrows.
         *
         * \param so The sphere offset. Where to place this sphere...
         * \param sc The sphere colour.
         * \param sc2 The sphere's second colour - used for cap and first ring
         * \param r Radius of the sphere
         * \param rings Number of rings used to render the sphere
         * \param segments Number of segments used to render the sphere
         */
        void computeSphere (vec<float> so, std::array<float, 3> sc, std::array<float, 3> sc2,
                            float r = 1.0f, int rings = 10, int segments = 12)
        {
            // First cap, draw as a triangle fan, but record indices so that
            // we only need a single call to glDrawElements.
            float rings0 = -morph::mathconst<float>::pi_over_2;
            float _z0  = std::sin(rings0);
            float z0  = r * _z0;
            float r0 =  std::cos(rings0);
            float rings1 = morph::mathconst<float>::pi * (-0.5f + 1.0f / rings);
            float _z1 = std::sin(rings1);
            float z1 = r * _z1;
            float r1 = std::cos(rings1);
            // Push the central point
            this->vertex_push (so[0]+0.0f, so[1]+0.0f, so[2]+z0, this->vertexPositions);
            this->vertex_push (0.0f, 0.0f, -1.0f, this->vertexNormals);
            this->vertex_push (sc2, this->vertexColors);

            GLuint capMiddle = this->idx++;
            GLuint ringStartIdx = this->idx;
            GLuint lastRingStartIdx = this->idx;

            bool firstseg = true;
            for (int j = 0; j < segments; j++) {
                float segment = morph::mathconst<float>::two_pi * static_cast<float>(j) / segments;
                float x = std::cos(segment);
                float y = std::sin(segment);

                float _x1 = x*r1;
                float x1 = _x1*r;
                float _y1 = y*r1;
                float y1 = _y1*r;

                this->vertex_push (so[0]+x1, so[1]+y1, so[2]+z1, this->vertexPositions);
                this->vertex_push (_x1, _y1, _z1, this->vertexNormals);
                this->vertex_push (sc2, this->vertexColors);

                if (!firstseg) {
                    this->indices.push_back (capMiddle);
                    this->indices.push_back (this->idx-1);
                    this->indices.push_back (this->idx++);
                } else {
                    this->idx++;
                    firstseg = false;
                }
            }
            this->indices.push_back (capMiddle);
            this->indices.push_back (this->idx-1);
            this->indices.push_back (capMiddle+1);

            // Now add the triangles around the rings
            for (int i = 2; i < rings; i++) {

                rings0 = morph::mathconst<float>::pi * (-0.5f + static_cast<float>(i) / rings);
                _z0  = std::sin(rings0);
                z0  = r * _z0;
                r0 =  std::cos(rings0);

                for (int j = 0; j < segments; j++) {

                    // "current" segment
                    float segment = morph::mathconst<float>::two_pi * static_cast<float>(j) / segments;
                    float x = std::cos(segment);
                    float y = std::sin(segment);

                    // One vertex per segment
                    float _x0 = x*r0;
                    float x0 = _x0*r;
                    float _y0 = y*r0;
                    float y0 = _y0*r;

                    // NB: Only add ONE vertex per segment. ALREADY have the first ring!
                    this->vertex_push (so[0]+x0, so[1]+y0, so[2]+z0, this->vertexPositions);
                    // The vertex normal of a vertex that makes up a sphere is
                    // just a normal vector in the direction of the vertex.
                    this->vertex_push (_x0, _y0, _z0, this->vertexNormals);
                    if (i == 2 || i > (rings-2)) {
                        this->vertex_push (sc2, this->vertexColors);
                    } else {
                        this->vertex_push (sc, this->vertexColors);
                    }
                    if (j == segments - 1) {
                        // Last vertex is back to the start
                        this->indices.push_back (ringStartIdx++);
                        this->indices.push_back (this->idx);
                        this->indices.push_back (lastRingStartIdx);
                        this->indices.push_back (lastRingStartIdx);
                        this->indices.push_back (this->idx++);
                        this->indices.push_back (lastRingStartIdx+segments);
                    } else {
                        this->indices.push_back (ringStartIdx++);
                        this->indices.push_back (this->idx);
                        this->indices.push_back (ringStartIdx);
                        this->indices.push_back (ringStartIdx);
                        this->indices.push_back (this->idx++);
                        this->indices.push_back (this->idx);
                    }
                }
                lastRingStartIdx += segments;
            }

            // bottom cap
            rings0 = morph::mathconst<float>::pi_over_2;
            _z0  = std::sin(rings0);
            z0  = r * _z0;
            r0 =  std::cos(rings0);
            // Push the central point of the bottom cap
            this->vertex_push (so[0]+0.0f, so[1]+0.0f, so[2]+z0, this->vertexPositions);
            this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
            this->vertex_push (sc2, this->vertexColors);
            capMiddle = this->idx++;
            firstseg = true;
            // No more vertices to push, just do the indices for the bottom cap
            ringStartIdx = lastRingStartIdx;
            for (int j = 0; j < segments; j++) {
                if (j != segments - 1) {
                    this->indices.push_back (capMiddle);
                    this->indices.push_back (ringStartIdx++);
                    this->indices.push_back (ringStartIdx);
                } else {
                    // Last segment
                    this->indices.push_back (capMiddle);
                    this->indices.push_back (ringStartIdx);
                    this->indices.push_back (lastRingStartIdx);
                }
            }
        }

        /*!
         * Compute vertices for an icosahedron.
         */
        void computeIcosahedron (vec<float> centre,
                                 std::array<std::array<float, 3>, 20> face_colours,
                                 float r = 1.0f) // radius or side length?
        {
            morph::geometry::polyhedron<float> ico = morph::geometry::icosahedron<float>();

            for (int j = 0; j < 20; ++j) {
                // Compute the face normal
                morph::vec<float, 3> norml = (ico.vertices[ico.faces[j][0]] + ico.vertices[ico.faces[j][1]] + ico.vertices[ico.faces[j][2]])/3.0f;
                this->vertex_push (centre + (ico.vertices[ico.faces[j][0]] * r), this->vertexPositions);
                this->vertex_push (centre + (ico.vertices[ico.faces[j][1]] * r), this->vertexPositions);
                this->vertex_push (centre + (ico.vertices[ico.faces[j][2]] * r), this->vertexPositions);
                for (int i = 0; i < 3; ++i) {
                    this->vertex_push (norml, this->vertexNormals);
                    this->vertex_push (face_colours[j], this->vertexColors);
                }
                // Indices...
                this->indices.push_back (this->idx);
                this->indices.push_back (this->idx+1);
                this->indices.push_back (this->idx+2);
                this->idx += 3;
            }
        }

        /*!
         * Create a cone.
         *
         * \param centre The centre of the cone - would be the end of the line
         *
         * \param tip The tip of the cone
         *
         * \param ringoffset Move the ring forwards or backwards along the vector from
         * \a centre to \a tip. This is positive or negative proportion of tip - centre.
         *
         * \param col The cone colour
         *
         * \param r Radius of the ring
         *
         * \param segments Number of segments used to render the tube
         */
        void computeCone (vec<float> centre,
                          vec<float> tip,
                          float ringoffset,
                          std::array<float, 3> col,
                          float r = 1.0f, int segments = 12)
        {
            // Cone is drawn as a base ring around a centre-of-the-base vertex, an
            // intermediate ring which is on the base ring, but has different normals, a
            // 'ring' around the tip (with suitable normals) and a 'tip' vertex

            vec<float> vbase = centre;
            vec<float> vtip = tip;
            vec<float> v = vtip - vbase;
            v.renormalize();

            // circle in a plane defined by a point and a normal
            vec<float> rand_vec;
            rand_vec.randomize();
            vec<float> inplane = rand_vec.cross(v);
            inplane.renormalize();
            vec<float> v_x_inplane = v.cross(inplane);

            // Push the central point of the start cap - this is at location vstart
            this->vertex_push (vbase, this->vertexPositions);
            this->vertex_push (-v, this->vertexNormals);
            this->vertex_push (col, this->vertexColors);

            // Base ring with normals in direction -v
            for (int j = 0; j < segments; j++) {
                float t = j * morph::mathconst<float>::two_pi / static_cast<float>(segments);
                vec<float> c = inplane * std::sin(t) * r + v_x_inplane * std::cos(t) * r;
                // Subtract the vector which makes this circle
                c = c + (v * ringoffset);
                this->vertex_push (vbase+c, this->vertexPositions);
                this->vertex_push (-v, this->vertexNormals);
                this->vertex_push (col, this->vertexColors);
            }

            // Intermediate ring of vertices around/aligned with the base ring with normals in direction c
            for (int j = 0; j < segments; j++) {
                float t = j * morph::mathconst<float>::two_pi / static_cast<float>(segments);
                vec<float> c = inplane * std::sin(t) * r + v_x_inplane * std::cos(t) * r;
                c = c + (v * ringoffset);
                this->vertex_push (vbase+c, this->vertexPositions);
                c.renormalize();
                this->vertex_push (c, this->vertexNormals);
                this->vertex_push (col, this->vertexColors);
            }

            // Intermediate ring of vertices around the tip with normals direction c
            for (int j = 0; j < segments; j++) {
                float t = j * morph::mathconst<float>::two_pi / static_cast<float>(segments);
                vec<float> c = inplane * std::sin(t) * r + v_x_inplane * std::cos(t) * r;
                c = c + (v * ringoffset);
                this->vertex_push (vtip, this->vertexPositions);
                c.renormalize();
                this->vertex_push (c, this->vertexNormals);
                this->vertex_push (col, this->vertexColors);
            }

            // Push tip vertex as the last vertex, normal is in direction v
            this->vertex_push (vtip, this->vertexPositions);
            this->vertex_push (v, this->vertexNormals);
            this->vertex_push (col, this->vertexColors);

            // Number of vertices = segments * 3 + 2.
            int nverts = segments * 3 + 2;

            // After creating vertices, push all the indices.
            GLuint capMiddle = this->idx;
            GLuint capStartIdx = this->idx + 1;
            GLuint endMiddle = this->idx + (GLuint)nverts - 1u;
            GLuint endStartIdx = capStartIdx;

            // Base of the cone
            for (int j = 0; j < segments-1; j++) {
                this->indices.push_back (capMiddle);
                this->indices.push_back (capStartIdx + j);
                this->indices.push_back (capStartIdx + 1 + j);
            }
            // Last tri of base
            this->indices.push_back (capMiddle);
            this->indices.push_back (capStartIdx + segments - 1);
            this->indices.push_back (capStartIdx);

            // Middle sections
            for (int lsection = 0; lsection < 2; ++lsection) {
                capStartIdx = this->idx + 1 + lsection*segments;
                endStartIdx = capStartIdx + segments;
                for (int j = 0; j < segments; j++) {
                    // Triangle 1:
                    this->indices.push_back (capStartIdx + j);
                    if (j == (segments-1)) {
                        this->indices.push_back (capStartIdx);
                    } else {
                        this->indices.push_back (capStartIdx + 1 + j);
                    }
                    this->indices.push_back (endStartIdx + j);
                    // Triangle 2:
                    this->indices.push_back (endStartIdx + j);
                    if (j == (segments-1)) {
                        this->indices.push_back (endStartIdx);
                    } else {
                        this->indices.push_back (endStartIdx + 1 + j);
                    }
                    if (j == (segments-1)) {
                        this->indices.push_back (capStartIdx);
                    } else {
                        this->indices.push_back (capStartIdx + j + 1);
                    }
                }
            }

            // tip
            for (int j = 0; j < segments-1; j++) {
                this->indices.push_back (endMiddle);
                this->indices.push_back (endStartIdx + j);
                this->indices.push_back (endStartIdx + 1 + j);
            }
            // Last triangle of tip
            this->indices.push_back (endMiddle);
            this->indices.push_back (endStartIdx + segments - 1);
            this->indices.push_back (endStartIdx);

            // Update idx
            this->idx += nverts;
        } // end of cone calculation

        //! Compute a line with a single colour
        void computeLine (vec<float> start, vec<float> end,
                          vec<float> _uz,
                          std::array<float, 3> col,
                          float w = 0.1f, float thickness = 0.01f, float shorten = 0.0f)
        {
            this->computeLine (start, end, _uz, col, col, w, thickness, shorten);
        }

        /*!
         * Create a line from \a start to \a end, with width \a w and a colour which
         * transitions from the colour \a colStart to \a colEnd. The thickness of the
         * line in the z direction is \a thickness
         *
         * \param start The start of the tube
         * \param end The end of the tube
         * \param _uz Dirn of z (up) axis for end face of line. Should be normalized.
         * \param colStart The tube staring colour
         * \param colEnd The tube's ending colour
         * \param w width of line
         * \param thickness The thickness/depth of the line in uy direction
         * \param shorten An amount by which to shorten the length of the line at each end.
         */
        void computeLine (vec<float> start, vec<float> end,
                          vec<float> _uz,
                          std::array<float, 3> colStart, std::array<float, 3> colEnd,
                          float w = 0.1f, float thickness = 0.01f, float shorten = 0.0f)
        {
            // There are always 8 segments for this line object, 2 at each of 4 corners
            const int segments = 8;

            // The vector from start to end defines direction of the tube
            vec<float> vstart = start;
            vec<float> vend = end;
            vec<float> v = vend - vstart;
            v.renormalize();

            // If shorten is not 0, then modify vstart and vend
            if (shorten > 0.0f) {
                vstart = start + v * shorten;
                vend = end - v * shorten;
            }

            // vv is normal to v and _uz
            vec<float> vv = v.cross(_uz);
            vv.renormalize();

            // Push the central point of the start cap - this is at location vstart
            this->vertex_push (vstart, this->vertexPositions);
            this->vertex_push (-v, this->vertexNormals);
            this->vertex_push (colStart, this->vertexColors);

            // Compute the 'face angles' that will give the correct width and thickness for the line
            std::array<float, 8> angles;
            float w_ = w * 0.5f;
            float d_ = thickness * 0.5f;
            float r = std::sqrt (w_ * w_ + d_ * d_);
            angles[0] = std::acos (w_ / r);
            angles[1] = angles[0];
            angles[2] = morph::mathconst<float>::pi - angles[0];
            angles[3] = angles[2];
            angles[4] = morph::mathconst<float>::pi + angles[0];
            angles[5] = angles[4];
            angles[6] = morph::mathconst<float>::two_pi - angles[0];
            angles[7] = angles[6];
            // The normals for the vertices around the line
            std::array<vec<float>, 8> norms = { vv, _uz, _uz, -vv, -vv, -_uz, -_uz, vv };

            // Start cap vertices (a triangle fan)
            for (int j = 0; j < segments; j++) {
                vec<float> c = _uz * std::sin(angles[j]) * r + vv * std::cos(angles[j]) * r;
                this->vertex_push (vstart+c, this->vertexPositions);
                this->vertex_push (-v, this->vertexNormals);
                this->vertex_push (colStart, this->vertexColors);
            }

            // Intermediate, near start cap. Normals point outwards. Need Additional vertices
            for (int j = 0; j < segments; j++) {
                vec<float> c = _uz * std::sin(angles[j]) * r + vv * std::cos(angles[j]) * r;
                this->vertex_push (vstart+c, this->vertexPositions);
                this->vertex_push (norms[j], this->vertexNormals);
                this->vertex_push (colStart, this->vertexColors);
            }

            // Intermediate, near end cap. Normals point in direction c
            for (int j = 0; j < segments; j++) {
                vec<float> c = _uz * std::sin(angles[j]) * r + vv * std::cos(angles[j]) * r;
                this->vertex_push (vend+c, this->vertexPositions);
                this->vertex_push (norms[j], this->vertexNormals);
                this->vertex_push (colEnd, this->vertexColors);
            }

            // Bottom cap vertices
            for (int j = 0; j < segments; j++) {
                vec<float> c = _uz * std::sin(angles[j]) * r + vv * std::cos(angles[j]) * r;
                this->vertex_push (vend+c, this->vertexPositions);
                this->vertex_push (v, this->vertexNormals);
                this->vertex_push (colEnd, this->vertexColors);
            }

            // Bottom cap. Push centre vertex as the last vertex.
            this->vertex_push (vend, this->vertexPositions);
            this->vertex_push (v, this->vertexNormals);
            this->vertex_push (colEnd, this->vertexColors);

            // Number of vertices = segments * 4 + 2.
            int nverts = (segments * 4) + 2;

            // After creating vertices, push all the indices.
            GLuint capMiddle = this->idx;
            GLuint capStartIdx = this->idx + 1u;
            GLuint endMiddle = this->idx + (GLuint)nverts - 1u;
            GLuint endStartIdx = capStartIdx + (3u * segments);

            // Start cap indices
            for (int j = 0; j < segments-1; j++) {
                this->indices.push_back (capMiddle);
                this->indices.push_back (capStartIdx + j);
                this->indices.push_back (capStartIdx + 1 + j);
            }
            // Last one
            this->indices.push_back (capMiddle);
            this->indices.push_back (capStartIdx + segments - 1);
            this->indices.push_back (capStartIdx);

            // Middle sections
            for (int lsection = 0; lsection < 3; ++lsection) {
                capStartIdx = this->idx + 1 + lsection*segments;
                endStartIdx = capStartIdx + segments;
                for (int j = 0; j < segments; j++) {
                    this->indices.push_back (capStartIdx + j);
                    if (j == (segments-1)) {
                        this->indices.push_back (capStartIdx);
                    } else {
                        this->indices.push_back (capStartIdx + 1 + j);
                    }
                    this->indices.push_back (endStartIdx + j);
                    this->indices.push_back (endStartIdx + j);
                    if (j == (segments-1)) {
                        this->indices.push_back (endStartIdx);
                    } else {
                        this->indices.push_back (endStartIdx + 1 + j);
                    }
                    if (j == (segments-1)) {
                        this->indices.push_back (capStartIdx);
                    } else {
                        this->indices.push_back (capStartIdx + j + 1);
                    }
                }
            }

            // bottom cap
            for (int j = 0; j < segments-1; j++) {
                this->indices.push_back (endMiddle);
                this->indices.push_back (endStartIdx + j);
                this->indices.push_back (endStartIdx + 1 + j);
            }
            this->indices.push_back (endMiddle);
            this->indices.push_back (endStartIdx + segments - 1);
            this->indices.push_back (endStartIdx);

            // Update idx
            this->idx += nverts;
        } // end computeLine

        // Like computeLine, but this line has no thickness.
        void computeFlatLine (vec<float> start, vec<float> end,
                              vec<float> _uz,
                              std::array<float, 3> col,
                              float w = 0.1f, float shorten = 0.0f)
        {
            // The vector from start to end defines direction of the tube
            vec<float> vstart = start;
            vec<float> vend = end;
            vec<float> v = vend - vstart;
            v.renormalize();

            // If shorten is not 0, then modify vstart and vend
            if (shorten > 0.0f) {
                vstart = start + v * shorten;
                vend = end - v * shorten;
            }

            // vv is normal to v and _uz
            vec<float> vv = v.cross(_uz);
            vv.renormalize();

            // corners of the line, and the start angle is determined from vv and w
            vec<float> ww = vv * w * 0.5f;
            vec<float> c1 = vstart + ww;
            vec<float> c2 = vstart - ww;
            vec<float> c3 = vend - ww;
            vec<float> c4 = vend + ww;

            this->vertex_push (c1, this->vertexPositions);
            this->vertex_push (_uz, this->vertexNormals);
            this->vertex_push (col, this->vertexColors);

            this->vertex_push (c2, this->vertexPositions);
            this->vertex_push (_uz, this->vertexNormals);
            this->vertex_push (col, this->vertexColors);

            this->vertex_push (c3, this->vertexPositions);
            this->vertex_push (_uz, this->vertexNormals);
            this->vertex_push (col, this->vertexColors);

            this->vertex_push (c4, this->vertexPositions);
            this->vertex_push (_uz, this->vertexNormals);
            this->vertex_push (col, this->vertexColors);

            // Number of vertices = segments * 4 + 2.
            int nverts = 4;

            // After creating vertices, push all the indices.
            this->indices.push_back (this->idx);
            this->indices.push_back (this->idx+1);
            this->indices.push_back (this->idx+2);

            this->indices.push_back (this->idx);
            this->indices.push_back (this->idx+2);
            this->indices.push_back (this->idx+3);

            // Update idx
            this->idx += nverts;

        } // end computeFlatLine

        // Like computeFlatLine but with option to add rounded start/end caps (I lazily
        // draw a whole circle around start/end to achieve this, rather than figuring
        // out a semi-circle).
        void computeFlatLineRnd (vec<float> start, vec<float> end,
                                 vec<float> _uz,
                                 std::array<float, 3> col,
                                 float w = 0.1f, float shorten = 0.0f, bool startcaps = true, bool endcaps = true)
        {
            // The vector from start to end defines direction of the tube
            vec<float> vstart = start;
            vec<float> vend = end;
            vec<float> v = vend - vstart;
            v.renormalize();

            // If shorten is not 0, then modify vstart and vend
            if (shorten > 0.0f) {
                vstart = start + v * shorten;
                vend = end - v * shorten;
            }

            // vv is normal to v and _uz
            vec<float> vv = v.cross(_uz);
            vv.renormalize();

            // corners of the line, and the start angle is determined from vv and w
            vec<float> ww = vv * w * 0.5f;
            vec<float> c1 = vstart + ww;
            vec<float> c2 = vstart - ww;
            vec<float> c3 = vend - ww;
            vec<float> c4 = vend + ww;

            int segments = 12;
            float r = 0.5f * w;
            unsigned int startvertices = 0u;
            if (startcaps) {
                // Push the central point of the start cap - this is at location vstart
                this->vertex_push (vstart, this->vertexPositions);
                this->vertex_push (_uz, this->vertexNormals);
                this->vertex_push (col, this->vertexColors);
                ++startvertices;
                // Start cap vertices (a triangle fan)
                for (int j = 0; j < segments; j++) {
                    float t = j * morph::mathconst<float>::two_pi / static_cast<float>(segments);
                    morph::vec<float> c = { std::sin(t) * r, std::cos(t) * r, 0.0f };
                    this->vertex_push (vstart+c, this->vertexPositions);
                    this->vertex_push (_uz, this->vertexNormals);
                    this->vertex_push (col, this->vertexColors);
                    ++startvertices;
                }
            }

            this->vertex_push (c1, this->vertexPositions);
            this->vertex_push (_uz, this->vertexNormals);
            this->vertex_push (col, this->vertexColors);

            this->vertex_push (c2, this->vertexPositions);
            this->vertex_push (_uz, this->vertexNormals);
            this->vertex_push (col, this->vertexColors);

            this->vertex_push (c3, this->vertexPositions);
            this->vertex_push (_uz, this->vertexNormals);
            this->vertex_push (col, this->vertexColors);

            this->vertex_push (c4, this->vertexPositions);
            this->vertex_push (_uz, this->vertexNormals);
            this->vertex_push (col, this->vertexColors);

            unsigned int endvertices = 0u;
            if (endcaps) {
                // Push the central point of the end cap - this is at location vend
                this->vertex_push (vend, this->vertexPositions);
                this->vertex_push (_uz, this->vertexNormals);
                this->vertex_push (col, this->vertexColors);
                ++endvertices;
                // End cap vertices (a triangle fan)
                for (int j = 0; j < segments; j++) {
                    float t = j * morph::mathconst<float>::two_pi / static_cast<float>(segments);
                    morph::vec<float> c = { std::sin(t) * r, std::cos(t) * r, 0.0f };
                    this->vertex_push (vend+c, this->vertexPositions);
                    this->vertex_push (_uz, this->vertexNormals);
                    this->vertex_push (col, this->vertexColors);
                    ++endvertices;
                }
            }

            // After creating vertices, push all the indices.

            if (startcaps) { // prolly startcaps, for flexibility
                GLuint topcap = this->idx;
                for (int j = 0; j < segments; j++) {
                    int inc1 = 1+j;
                    int inc2 = 1+((j+1)%segments);
                    this->indices.push_back (topcap);
                    this->indices.push_back (topcap+inc1);
                    this->indices.push_back (topcap+inc2);
                }
                this->idx += startvertices;
            }

            // The line itself
            this->indices.push_back (this->idx);
            this->indices.push_back (this->idx+1);
            this->indices.push_back (this->idx+2);
            this->indices.push_back (this->idx);
            this->indices.push_back (this->idx+2);
            this->indices.push_back (this->idx+3);
            // Update idx
            this->idx += 4;

            if (endcaps) {
                GLuint botcap = this->idx;
                for (int j = 0; j < segments; j++) {
                    int inc1 = 1+j;
                    int inc2 = 1+((j+1)%segments);
                    this->indices.push_back (botcap);
                    this->indices.push_back (botcap+inc1);
                    this->indices.push_back (botcap+inc2);
                }
                this->idx += endvertices;
            }
        } // end computeFlatLine

        /*!
         * Like computeFlatLine, but this line has no thickness and you can provide the
         * previous and next data points so that this line, the previous line and the
         * next line can line up perfectly without drawing a circular rounded 'end cap'!
         *
         * This code assumes that the coordinates prev, start, end, next all lie on a 2D
         * plane normal to _uz. In fact, the 3D coordinates start, end, prev and next
         * will all be projected onto the plane defined by _uz, so that they can be
         * reduced to 2D coordinates. This then allows crossing points of lines to be
         * computed.
         *
         * If you want to make a ribbon between points that do *not* lie on a 2D plane,
         * you'll need to write another graphics primitive function.
         */
        void computeFlatLine (vec<float> start, vec<float> end,
                              vec<float> prev, vec<float> next,
                              vec<float> _uz,
                              std::array<float, 3> col,
                              float w = 0.1f)
        {
            // Corner coordinates for this line section
            vec<float> c1 = { 0.0f };
            vec<float> c2 = { 0.0f };
            vec<float> c3 = { 0.0f };
            vec<float> c4 = { 0.0f };

            // Ensure _uz is a unit vector
            vec<float> __uz = _uz;
            __uz.renormalize();

            // First find the rotation to make __uz into the actual unit z dirn
            morph::quaternion<float> rotn;
            morph::vec<float> basis_rotn_axis = __uz.cross (this->uz);
            if (basis_rotn_axis.length() > 0.0f) {
                float basis_rotn_angle = __uz.angle (this->uz, basis_rotn_axis);
                rotn.rotate (basis_rotn_axis, basis_rotn_angle);
            } // else nothing to do  - basis rotn is null

            // Transform so that start is the origin
            // vec<float> s_o = { 0.0f }; // by defn
            vec<float> e_o = end - start;
            vec<float> p_o = prev - start;
            vec<float> n_o = next - start;

            // Apply basis rotation just to the end point. e_b: 'end point in rotated basis'
            vec<float> e_b = rotn * e_o;

            // Use the vector from start to end as the in-plane x dirn. Do this AFTER
            // first coord rotn.  In other words: find the rotation about the new unit z
            // direction to force the end point to be on the x axis
            vec<float> plane_x = e_b; // - s_b but s_b is (0,0,0) by defn
            plane_x.renormalize();
            vec<float> plane_y = this->uz.cross (plane_x);
            plane_y.renormalize();
            // Find the in-plane coordinates in the rotated plane system
            vec<float> e_p = { plane_x.dot (e_b), plane_y.dot (e_b), this->uz.dot (e_b) };

            // One epsilon is exacting
            if (std::abs(e_p[2]) > std::numeric_limits<float>::epsilon()) {
                throw std::runtime_error ("uz not orthogonal to the line start -> end?");
            }

            // From e_p and e_b (which should both be in a 2D plane) figure out what
            // angle of rotation brings e_b into the x axis
            float inplane_rotn_angle = e_b.angle (e_p, this->uz);
            morph::quaternion<float> inplane_rotn (this->uz, inplane_rotn_angle);

            // Apply the in-plane rotation to the basis rotation
            rotn.premultiply (inplane_rotn);

            // Transform points
            vec<float> p_p = rotn * p_o;
            vec<float> n_p = rotn * n_o;
            //vec<float> s_p = rotn * s_o; // not necessary, s_p = (0,0,0) by defn

            // Line crossings time.
            vec<float, 2> c1_p = { 0.0f }; // 2D crossing coords that we're going to find
            vec<float, 2> c2_p = { 0.0f };
            vec<float, 2> c3_p = e_p.less_one_dim();
            vec<float, 2> c4_p = e_p.less_one_dim();

            // 3 lines on each side. l_p, l_c (current) and l_n. Each has two ends. l_p_1, l_p_2 etc.

            // 'prev' 'cur' and 'next' vectors
            vec<float, 2> p_vec = (/*s_p*/ -p_p).less_one_dim();
            vec<float, 2> c_vec = e_p.less_one_dim();
            vec<float, 2> n_vec = (n_p - e_p).less_one_dim();

            vec<float, 2> p_ortho = (/*s_p*/ - p_p).cross (this->uz).less_one_dim();
            p_ortho.renormalize();
            vec<float, 2> c_ortho = (e_p /*- s_p*/).cross (this->uz).less_one_dim();
            c_ortho.renormalize();
            vec<float, 2> n_ortho = (n_p - e_p).cross (this->uz).less_one_dim();
            n_ortho.renormalize();

            const float hw = w / 2.0f;

            vec<float, 2> l_p_1 = p_p.less_one_dim() + (p_ortho * hw) - p_vec; // makes it 3 times as long as the line.
            vec<float, 2> l_p_2 = /*s_p.less_one_dim() +*/ (p_ortho * hw) + p_vec;
            vec<float, 2> l_c_1 = /*s_p.less_one_dim() +*/ (c_ortho * hw) - c_vec;
            vec<float, 2> l_c_2 = e_p.less_one_dim() + (c_ortho * hw) + c_vec;
            vec<float, 2> l_n_1 = e_p.less_one_dim() + (n_ortho * hw) - n_vec;
            vec<float, 2> l_n_2 = n_p.less_one_dim() + (n_ortho * hw) + n_vec;

            std::bitset<2> isect = morph::MathAlgo::segments_intersect<float> (l_p_1, l_p_2, l_c_1, l_c_2);
            if (isect.test(0) == true && isect.test(1) == false) { // test for intersection but not colinear
                c1_p = morph::MathAlgo::crossing_point (l_p_1, l_p_2, l_c_1, l_c_2);
            } else if (isect.test(0) == true && isect.test(1) == true) {
                c1_p = /*s_p.less_one_dim() +*/ (c_ortho * hw);
            } else { // no intersection. prev could have been start
                c1_p = /*s_p.less_one_dim() +*/ (c_ortho * hw);
            }
            isect = morph::MathAlgo::segments_intersect<float> (l_c_1, l_c_2, l_n_1, l_n_2);
            if (isect.test(0) == true && isect.test(1) == false) {
                c4_p = morph::MathAlgo::crossing_point (l_c_1, l_c_2, l_n_1, l_n_2);
            } else if (isect.test(0) == true && isect.test(1) == true) {
                c4_p = e_p.less_one_dim() + (c_ortho * hw);
            } else { // no intersection, prev could have been end
                c4_p = e_p.less_one_dim() + (c_ortho * hw);
            }

            // o for 'other side'. Could re-use vars in future version. Or just subtract (*_ortho * w) from each.
            vec<float, 2> o_l_p_1 = p_p.less_one_dim() - (p_ortho * hw) - p_vec; // makes it 3 times as long as the line.
            vec<float, 2> o_l_p_2 = /*s_p.less_one_dim()*/ - (p_ortho * hw) + p_vec;
            vec<float, 2> o_l_c_1 = /*s_p.less_one_dim()*/ - (c_ortho * hw) - c_vec;
            vec<float, 2> o_l_c_2 = e_p.less_one_dim() - (c_ortho * hw) + c_vec;
            vec<float, 2> o_l_n_1 = e_p.less_one_dim() - (n_ortho * hw) - n_vec;
            vec<float, 2> o_l_n_2 = n_p.less_one_dim() - (n_ortho * hw) + n_vec;

            isect = morph::MathAlgo::segments_intersect<float> (o_l_p_1, o_l_p_2, o_l_c_1, o_l_c_2);
            if (isect.test(0) == true && isect.test(1) == false) { // test for intersection but not colinear
                c2_p = morph::MathAlgo::crossing_point (o_l_p_1, o_l_p_2, o_l_c_1, o_l_c_2);
            } else if (isect.test(0) == true && isect.test(1) == true) {
                c2_p = /*s_p.less_one_dim()*/ - (c_ortho * hw);
            } else { // no intersection. prev could have been start
                c2_p = /*s_p.less_one_dim()*/ - (c_ortho * hw);
            }

            isect = morph::MathAlgo::segments_intersect<float> (o_l_c_1, o_l_c_2, o_l_n_1, o_l_n_2);
            if (isect.test(0) == true && isect.test(1) == false) {
                c3_p = morph::MathAlgo::crossing_point (o_l_c_1, o_l_c_2, o_l_n_1, o_l_n_2);
            } else if (isect.test(0) == true && isect.test(1) == true) {
                c3_p = e_p.less_one_dim() - (c_ortho * hw);
            } else { // no intersection. next could have been end
                c3_p = e_p.less_one_dim() - (c_ortho * hw);
            }

            // Transform and rotate back into c1-c4
            morph::quaternion<float> rotn_inv = rotn.invert();
            c1 = rotn_inv * c1_p.plus_one_dim() + start;
            c2 = rotn_inv * c2_p.plus_one_dim() + start;
            c3 = rotn_inv * c3_p.plus_one_dim() + start;
            c4 = rotn_inv * c4_p.plus_one_dim() + start;

            // Now create the vertices from these four corners, c1-c4
            this->vertex_push (c1, this->vertexPositions);
            this->vertex_push (_uz, this->vertexNormals);
            this->vertex_push (col, this->vertexColors);

            this->vertex_push (c2, this->vertexPositions);
            this->vertex_push (_uz, this->vertexNormals);
            this->vertex_push (col, this->vertexColors);

            this->vertex_push (c3, this->vertexPositions);
            this->vertex_push (_uz, this->vertexNormals);
            this->vertex_push (col, this->vertexColors);

            this->vertex_push (c4, this->vertexPositions);
            this->vertex_push (_uz, this->vertexNormals);
            this->vertex_push (col, this->vertexColors);

            this->indices.push_back (this->idx);
            this->indices.push_back (this->idx+1);
            this->indices.push_back (this->idx+2);

            this->indices.push_back (this->idx);
            this->indices.push_back (this->idx+2);
            this->indices.push_back (this->idx+3);

            // Update idx
            this->idx += 4;
        } // end computeFlatLine that joins perfectly

        //! Make a joined up line with previous.
        void computeFlatLineP (vec<float> start, vec<float> end,
                               vec<float> prev,
                               vec<float> _uz,
                               std::array<float, 3> col,
                               float w = 0.1f)
        {
            this->computeFlatLine (start, end, prev, end, _uz, col, w);
        } // end computeFlatLine that joins perfectly with prev

        //! Flat line, joining up with next
        void computeFlatLineN (vec<float> start, vec<float> end,
                               vec<float> next,
                               vec<float> _uz,
                               std::array<float, 3> col,
                               float w = 0.1f)
        {
            this->computeFlatLine (start, end, start, next, _uz, col, w);
        }

        // Like computeLine, but this line has no thickness and it's dashed.
        // dashlen: the length of dashes
        // gap prop: The proportion of dash length used for the gap
        void computeFlatDashedLine (vec<float> start, vec<float> end,
                                    vec<float> _uz,
                                    std::array<float, 3> col,
                                    float w = 0.1f, float shorten = 0.0f,
                                    float dashlen = 0.1f, float gapprop = 0.3f)
        {
            if (dashlen == 0.0f) { return; }

            // The vector from start to end defines direction of the line
            vec<float> vstart = start;
            vec<float> vend = end;

            vec<float> v = vend - vstart;
            float linelen = v.length();
            v.renormalize();

            // If shorten is not 0, then modify vstart and vend
            if (shorten > 0.0f) {
                vstart = start + v * shorten;
                vend = end - v * shorten;
                linelen = v.length() - shorten * 2.0f;
            }

            // vv is normal to v and _uz
            vec<float> vv = v.cross(_uz);
            vv.renormalize();

            // Loop, creating the dashes
            vec<float> dash_s = vstart;
            vec<float> dash_e = dash_s + v * dashlen;
            vec<float> dashes = dash_e - vstart;

            while (dashes.length() < linelen) {

                // corners of the line, and the start angle is determined from vv and w
                vec<float> ww = vv * w * 0.5f;
                vec<float> c1 = dash_s + ww;
                vec<float> c2 = dash_s - ww;
                vec<float> c3 = dash_e - ww;
                vec<float> c4 = dash_e + ww;

                this->vertex_push (c1, this->vertexPositions);
                this->vertex_push (_uz, this->vertexNormals);
                this->vertex_push (col, this->vertexColors);

                this->vertex_push (c2, this->vertexPositions);
                this->vertex_push (_uz, this->vertexNormals);
                this->vertex_push (col, this->vertexColors);

                this->vertex_push (c3, this->vertexPositions);
                this->vertex_push (_uz, this->vertexNormals);
                this->vertex_push (col, this->vertexColors);

                this->vertex_push (c4, this->vertexPositions);
                this->vertex_push (_uz, this->vertexNormals);
                this->vertex_push (col, this->vertexColors);

                // Number of vertices = segments * 4 + 2.
                int nverts = 4;

                // After creating vertices, push all the indices.
                this->indices.push_back (this->idx);
                this->indices.push_back (this->idx+1);
                this->indices.push_back (this->idx+2);

                this->indices.push_back (this->idx);
                this->indices.push_back (this->idx+2);
                this->indices.push_back (this->idx+3);

                // Update idx
                this->idx += nverts;

                // Next dash
                dash_s = dash_e + v * dashlen * gapprop;
                dash_e = dash_s + v * dashlen;
                dashes = dash_e - vstart;
            }

        } // end computeFlatDashedLine

        // Compute a flat line circle outline
        void computeFlatCircleLine (vec<float> centre, vec<float> norm, float radius,
                                    float linewidth, std::array<float, 3> col, int segments = 128)
        {
            // circle in a plane defined by a point (v0 = vstart or vend) and a normal
            // (v) can be found: Choose random vector vr. A vector inplane = vr ^ v. The
            // unit in-plane vector is inplane.normalise. Can now use that vector in the
            // plan to define a point on the circle. Note that this starting point on
            // the circle is at a random position, which means that this version of
            // computeTube is useful for tubes that have quite a few segments.
            vec<float> rand_vec;
            rand_vec.randomize();
            vec<float> inplane = rand_vec.cross(norm);
            inplane.renormalize();
            vec<float> norm_x_inplane = norm.cross(inplane);

            float half_lw = linewidth / 2.0f;
            float r_in = radius - half_lw;
            float r_out = radius + half_lw;
            // Inner ring at radius radius-linewidth/2 with normals in direction norm;
            // Outer ring at radius radius+linewidth/2 with normals also in direction norm
            for (int j = 0; j < segments; j++) {
                float t = j * morph::mathconst<float>::two_pi / static_cast<float>(segments);
                vec<float> c_in = inplane * std::sin(t) * r_in + norm_x_inplane * std::cos(t) * r_in;
                this->vertex_push (centre+c_in, this->vertexPositions);
                this->vertex_push (norm, this->vertexNormals);
                this->vertex_push (col, this->vertexColors);
                vec<float> c_out = inplane * std::sin(t) * r_out + norm_x_inplane * std::cos(t) * r_out;
                this->vertex_push (centre+c_out, this->vertexPositions);
                this->vertex_push (norm, this->vertexNormals);
                this->vertex_push (col, this->vertexColors);
            }
            // Added 2*segments vertices to vertexPositions

            // After creating vertices, push all the indices.
            for (int j = 0; j < segments; j++) {
                int jn = (segments + ((j+1) % segments)) % segments;
                this->indices.push_back (this->idx+(2*j));
                this->indices.push_back (this->idx+(2*jn));
                this->indices.push_back (this->idx+(2*jn+1));
                this->indices.push_back (this->idx+(2*j));
                this->indices.push_back (this->idx+(2*jn+1));
                this->indices.push_back (this->idx+(2*j+1));
            }
            this->idx += 2 * segments; // nverts

        } // end computeFlatCircle

        // Compute triangles to form a true cuboid from 8 corners.
        void computeCuboid (const std::array<vec<float>, 8>& v, const std::array<float, 3>& clr)
        {
            this->computeFlatQuad (v[0], v[1], v[2], v[3], clr);
            this->computeFlatQuad (v[0], v[4], v[5], v[1], clr);
            this->computeFlatQuad (v[1], v[5], v[6], v[2], clr);
            this->computeFlatQuad (v[2], v[6], v[7], v[3], clr);
            this->computeFlatQuad (v[3], v[7], v[4], v[0], clr);
            this->computeFlatQuad (v[7], v[6], v[5], v[4], clr);
        }

        // Compute a rhombus using the four defining coordinates. The coordinates are named as if
        // they were the origin, x, y and z of a right-handed 3D coordinate system. These define three edges
        void computeRhombus (const vec<float>& o, const vec<float>& x, const vec<float>& y, const vec<float>& z,
                             const std::array<float, 3>& clr)
        {
            // Edge vectors
            vec<float> edge1 = x - o;
            vec<float> edge2 = y - o;
            vec<float> edge3 = z - o;

            // Compute the face normals
            vec<float> _n1 = edge1.cross (edge2);
            _n1.renormalize();
            vec<float> _n2 = edge2.cross (edge3);
            _n2.renormalize();
            vec<float> _n3 = edge1.cross (edge3);
            _n3.renormalize();

            // Push positions and normals for 24 vertices to make up the rhombohedron; 4 for each face.
            // Front face
            this->vertex_push (o,                        this->vertexPositions);
            this->vertex_push (o + edge1,                this->vertexPositions);
            this->vertex_push (o + edge3,                this->vertexPositions);
            this->vertex_push (o + edge1 + edge3,        this->vertexPositions);
            for (unsigned short i = 0U; i < 4U; ++i) { this->vertex_push (_n3, this->vertexNormals); }
            // Top face
            this->vertex_push (o + edge3,                 this->vertexPositions);
            this->vertex_push (o + edge1 + edge3,         this->vertexPositions);
            this->vertex_push (o + edge2 + edge3,         this->vertexPositions);
            this->vertex_push (o + edge2 + edge1 + edge3, this->vertexPositions);
            for (unsigned short i = 0U; i < 4U; ++i) { this->vertex_push (_n1, this->vertexNormals); }
            // Back face
            this->vertex_push (o + edge2 + edge3,         this->vertexPositions);
            this->vertex_push (o + edge2 + edge1 + edge3, this->vertexPositions);
            this->vertex_push (o + edge2,                 this->vertexPositions);
            this->vertex_push (o + edge2 + edge1,         this->vertexPositions);
            for (unsigned short i = 0U; i < 4U; ++i) { this->vertex_push (-_n3, this->vertexNormals); }
            // Bottom face
            this->vertex_push (o + edge2,                 this->vertexPositions);
            this->vertex_push (o + edge2 + edge1,         this->vertexPositions);
            this->vertex_push (o,                         this->vertexPositions);
            this->vertex_push (o + edge1,                 this->vertexPositions);
            for (unsigned short i = 0U; i < 4U; ++i) { this->vertex_push (-_n1, this->vertexNormals); }
            // Left face
            this->vertex_push (o + edge2,                 this->vertexPositions);
            this->vertex_push (o,                         this->vertexPositions);
            this->vertex_push (o + edge2 + edge3,         this->vertexPositions);
            this->vertex_push (o + edge3,                 this->vertexPositions);
            for (unsigned short i = 0U; i < 4U; ++i) { this->vertex_push (-_n2, this->vertexNormals); }
            // Right face
            this->vertex_push (o + edge1,                 this->vertexPositions);
            this->vertex_push (o + edge1 + edge2,         this->vertexPositions);
            this->vertex_push (o + edge1 + edge3,         this->vertexPositions);
            this->vertex_push (o + edge1 + edge2 + edge3, this->vertexPositions);
            for (unsigned short i = 0U; i < 4U; ++i) { this->vertex_push (_n2, this->vertexNormals); }

            // Vertex colours are all the same
            for (unsigned short i = 0U; i < 24U; ++i) { this->vertex_push (clr, this->vertexColors); }

            // Indices for 6 faces
            for (unsigned short i = 0U; i < 6U; ++i) {
                this->indices.push_back (this->idx++);
                this->indices.push_back (this->idx++);
                this->indices.push_back (this->idx--);
                this->indices.push_back (this->idx++);
                this->indices.push_back (this->idx++);
                this->indices.push_back (this->idx++);
            }
        } // computeCuboid

        // Compute a rectangular cuboid of width (in x), height (in y) and depth (in z).
        void computeRectCuboid (const vec<float>& o, const float wx, const float hy, const float dz,
                                const std::array<float, 3>& clr)
        {
            vec<float> px = o + vec<float>{wx, 0, 0};
            vec<float> py = o + vec<float>{0, hy, 0};
            vec<float> pz = o + vec<float>{0, 0, dz};
            this->computeRhombus (o, px, py, pz, clr);
        }
    };

} // namespace morph
