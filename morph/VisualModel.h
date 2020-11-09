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

#ifdef __OSX__
# include <OpenGL/gl3.h>
#else
# include <GL3/gl3.h>
#endif
#include <morph/TransformMatrix.h>
#include <morph/Vector.h>
#include <morph/MathConst.h>
#include <morph/VisualCommon.h>
#include <morph/VisualTextModel.h>
#include <iostream>
#include <vector>
#include <array>
#include <algorithm>
#include <iterator>

// Switches on some changes where I carefully unbind gl buffers after calling
// glBufferData() and rebind when changing the vertex model. Makes no difference on my
// Macbook Air, but should be more correct. Dotting my 'i's and 't's
#define CAREFULLY_UNBIND_AND_REBIND 1

namespace morph {

    //! Forward declaration of a Visual class
    class Visual;

    /*!
     * OpenGL model base class
     *
     * This class is a base 'OpenGL model' class. It has the common code to create the
     * vertices for some individual OpengGL model which is to be rendered in a 3-D
     * scene.
     *
     * Some OpenGL models are derived directly from VisualModel; see for example
     * morph::CoordArrows.
     *
     * Most of the models in morphologica are derived via morph::VisualDataModel,
     * which adds a common mechanism for managing the data which is to be visualised
     * by the final 'Visual' object (such as morph::HexGridVisual or
     * morph::ScatterVisual)
     *
     * This class contains some common 'object primitives' code, such as computeSphere
     * and computeCone, which compute the vertices that will make up sphere and cone,
     * respectively.
     */
    class VisualModel
    {
    public:
        VisualModel () { this->mv_offset = {0.0, 0.0, 0.0}; }

        VisualModel (GLuint sp, const Vector<float> _mv_offset)
        {
            // Set up...
            this->shaderprog = sp;
            this->mv_offset = _mv_offset;
            this->viewmatrix.translate (this->mv_offset);

            // In derived constructor: Do the computations to initialize the vertices
            // that will represent the model
            // this->initializeVertices();

            // Then common code for postVertexInit:
            // this->postVertexInit();

            // Here's how to unbind the VAO. Is that necessary? Seems not
            // glBindVertexArray(0);
        }

        //! destroy gl buffers in the deconstructor
        virtual ~VisualModel()
        {
            for (auto& tm : this->texts) { delete (tm); }
            glDeleteBuffers (numVBO, vbos);
            morph::gl::Util::checkError (__FILE__, __LINE__);
            delete (this->vbos);
        }

        //! Common code to call after the vertices have been set up.
        void postVertexInit (void)
        {
            // Create vertex array object
#ifdef __MACS_HAD_OPENGL_450__
            glCreateVertexArrays (1, &this->vao); // OpenGL 4.5 only
#else
            glGenVertexArrays (1, &this->vao); // Safe for OpenGL 4.4-
#endif
            morph::gl::Util::checkError (__FILE__, __LINE__);

            glBindVertexArray (this->vao);
            morph::gl::Util::checkError (__FILE__, __LINE__);

            // Create the vertex buffer objects
            this->vbos = new GLuint[numVBO];
#ifdef __MACS_HAD_OPENGL_450__
            glCreateBuffers (numVBO, this->vbos); // OpenGL 4.5 only
#else
            glGenBuffers (numVBO, this->vbos); // OpenGL 4.4- safe
#endif
            morph::gl::Util::checkError (__FILE__, __LINE__);

            // Set up the indices buffer - bind and buffer the data in this->indices
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vbos[idxVBO]);
            morph::gl::Util::checkError (__FILE__, __LINE__);

            //std::cout << "indices.size(): " << this->indices.size() << std::endl;
            int sz = this->indices.size() * sizeof(VBOint);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sz, this->indices.data(), GL_STATIC_DRAW);
            morph::gl::Util::checkError (__FILE__, __LINE__);

            // Binds data from the "C++ world" to the OpenGL shader world for
            // "position", "normalin" and "color"
            // (bind, buffer and set vertex array object attribute)
            this->setupVBO (this->vbos[posnVBO], this->vertexPositions, gl::posnLoc);
            this->setupVBO (this->vbos[normVBO], this->vertexNormals, gl::normLoc);
            this->setupVBO (this->vbos[colVBO], this->vertexColors, gl::colLoc);

#ifdef CAREFULLY_UNBIND_AND_REBIND
            // Possibly release (unbind) the vertex buffers, but have to unbind vertex
            // array object first.
            glBindVertexArray(0);
            morph::gl::Util::checkError (__FILE__, __LINE__);

# if 0
            // These calls generate GL_INVALID_ENUM errors
            glBindBuffer (0, this->vbos[posnVBO]);
            morph::gl::Util::checkError (__FILE__, __LINE__);
            glBindBuffer (0, this->vbos[normVBO]);
            morph::gl::Util::checkError (__FILE__, __LINE__);
            glBindBuffer (0, this->vbos[colVBO]);
            morph::gl::Util::checkError (__FILE__, __LINE__);
            glBindBuffer (0, this->vbos[idxVBO]);
            morph::gl::Util::checkError (__FILE__, __LINE__);
# endif
            // Instead, maybe:
            // glDeleteBuffers (4, this->vbos); But that's not *unbinding* as such, and I do that in the deconstructor
#endif
        }

        //! Initialize vertex buffer objects and vertex array object.
        virtual void initializeVertices (void) = 0;

        //! Render the VisualModel
        void render (void)
        {
            if (this->hide == true) { return; }

            GLint prev_shader;
            glGetIntegerv (GL_CURRENT_PROGRAM, &prev_shader);

            // Ensure the correct program is in play for this VisualModel
            glUseProgram (this->shaderprog);

            // It is only necessary to bind the vertex array object before rendering
            // (not the vertex buffer objects)
            glBindVertexArray (this->vao);

            // Pass this->float to GLSL so the model can have an alpha value.
            GLint loc_a = glGetUniformLocation (this->shaderprog, (const GLchar*)"alpha");
            if (loc_a != -1) { glUniform1f (loc_a, this->alpha); }

            GLint loc_v = glGetUniformLocation (this->shaderprog, (const GLchar*)"v_matrix");
            if (loc_v != -1) { glUniformMatrix4fv (loc_v, 1, GL_FALSE, this->scenematrix.mat.data()); }

            GLint loc_m = glGetUniformLocation (this->shaderprog, (const GLchar*)"m_matrix");
            if (loc_m != -1) { glUniformMatrix4fv (loc_m, 1, GL_FALSE, this->viewmatrix.mat.data()); }

            std::cout << "VisualModel::render: scenematrix:\n" << scenematrix << std::endl;
            std::cout << "VisualModel::render: model viewmatrix:\n" << viewmatrix << std::endl;

            // Draw the triangles
            glDrawElements (GL_TRIANGLES, this->indices.size(), VBO_ENUM_TYPE, 0);

            // Unbind the VAO
            glBindVertexArray(0);

            // Now render any VisualTextModels
            // Here, could set anything that should be true for ALL texts in this model
            // glUseProgram (this->tshaderprog);
            // etc

            for (auto t : this->texts) { t->render(); }

            glUseProgram (prev_shader);

            morph::gl::Util::checkError (__FILE__, __LINE__);
        }

    public:

        //! Setter for the viewmatrix
        void setViewMatrix (const TransformMatrix<float>& mv) { this->viewmatrix = mv; }

        //! When setting the scene matrix, also have to set the text's scene matrices.
        void setSceneMatrix (const TransformMatrix<float>& sv)
        {
            this->scenematrix = sv;
            // For each text model, also set scene matrix
            for (auto& t : this->texts) { t->setSceneMatrix (sv); }
        }

/////////// This applied in Visual::render
        //! Set a translation into the scene and into any child texts
        void setSceneTranslation (const Vector<float>& v0)
        {
            std::cout << "VisualModel::setSceneTranslation\n";
            this->scenematrix.setToIdentity();
            this->sv_offset = v0;
            this->scenematrix.translate (this->sv_offset);
            this->scenematrix.rotate (this->sv_rotation);

            for (auto& t : this->texts) {
                std::cout << "setSceneTranslation in a child text to " << v0 << std::endl;
                t->setSceneTranslation (v0);
            }
        }

        //! Set a translation (only) into the scene view matrix
        void addSceneTranslation (const Vector<float>& v0)
        {
            this->sv_offset += v0;
            this->scenematrix.translate (v0);
        }

        //! Set a rotation (only) into the scene view matrix
        void setSceneRotation (const Quaternion<float>& r)
        {
            this->scenematrix.setToIdentity();
            this->sv_rotation = r;
            this->scenematrix.translate (this->sv_offset);
            this->scenematrix.rotate (this->sv_rotation);
        }

        //! Add a rotation to the scene view matrix
        void addSceneRotation (const Quaternion<float>& r)
        {
            this->sv_rotation.premultiply (r); // combines rotations
            this->scenematrix.rotate (r);
        }

        //! Set a translation to the model view matrix
        void setViewTranslation (const Vector<float>& v0)
        {
            this->viewmatrix.setToIdentity();
            this->mv_offset = v0;
            this->viewmatrix.translate (this->mv_offset);
            this->viewmatrix.rotate (this->mv_rotation);
        }

        //! Add a translation to the model view matrix
        void addViewTranslation (const Vector<float>& v0)
        {
            this->mv_offset += v0;
            this->viewmatrix.translate (v0);
        }

/////////// This applied in Visual::render
        //! Set a rotation (only) into the view
        void setViewRotation (const Quaternion<float>& r)
        {
            this->viewmatrix.setToIdentity();
            this->mv_rotation = r;
            this->viewmatrix.translate (this->mv_offset);
            this->viewmatrix.rotate (this->mv_rotation);

            // When rotating a model that contains texts, we need to rotate the scene
            // for the texts and also inverse-rotate the view of the texts.
            for (auto& t : this->texts) {
                // Rotate the scene
                t->setSceneRotation (r);
                // Rotate the view of the text an opposite amount, to keep it facing forwards
                t->setViewRotation (r.invert());
            }
        }

        //! Apply a further rotation to the model view matrix
        void addViewRotation (const Quaternion<float>& r)
        {
            this->mv_rotation.premultiply (r);
            this->viewmatrix.rotate (r);
            std::cout << "VisualModel::addViewRotation: FIXME? or t->addSceneRotation(r)?\n";
            for (auto& t : this->texts) { t->addViewRotation (r); }
        }

#if 0 // Use setViewTranslation instead
        //! Setter for mv_offset, also updates viewmatrix. The offset is an offset within the model view.
        void setOffset (const Vector<float>& _mv_offset)
        {
            this->mv_offset = _mv_offset;
            this->viewmatrix.rotate (this->mv_rotation);
            this->setViewTranslation (this->mv_offset);
        }
#endif
#if 0 // Use addViewTranslation instead
        //! Shift the mv_offset, also updates viewmatrix.
        void shiftOffset (const Vector<float>& _mv_offset)
        {
            this->mv_offset += _mv_offset;
            this->viewmatrix.translate (this->mv_offset);
        }
#endif
        void setAlpha (const float _a) { this->alpha = _a; }
        float getAlpha() const { return this->alpha; }

        void setHide (const bool _h = true) { this->hide = _h; }
        void toggleHide() { this->hide = this->hide ? false : true; }
        float hidden() const { return this->hide; }

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

    protected:

        //! The model-specific view matrix.
        TransformMatrix<float> viewmatrix;
        //! The model-specific scene view matrix.
        TransformMatrix<float> scenematrix;

        /*!
         * The spatial offset of this VisualModel within the morph::Visual 'model
         * view'. Note that this is not incorporated into the computation of the
         * vertices, but is instead applied when the object is rendered as part of the
         * model->world transformation - it's applied as a translation in
         * VisualModel::viewmatrix.
         */
        Vector<float> mv_offset;
        //! Model view rotation
        Quaternion<float> mv_rotation;

        //! Scene view offset
        Vector<float> sv_offset;
        //! Scene view rotation
        Quaternion<float> sv_rotation;

        //! A vector of pointers to text models that should be rendered.
        std::vector<morph::VisualTextModel*> texts;

        //! This enum contains the positions within the vbo array of the different
        //! vertex buffer objects
        enum VBOPos { posnVBO, normVBO, colVBO, idxVBO, numVBO };

        //! The parent Visual object - provides access to the shader prog
        const Visual* parent;

        //! A copy of the reference to the shader program
        GLuint shaderprog;

        //! A copy of the reference to the text-specific shader program
        GLuint tshaderprog;

        /*
         * Compute positions and colours of vertices for the hexes and store in these:
         */

        //! The OpenGL Vertex Array Object
        GLuint vao;

        //! Vertex Buffer Objects stored in an array
        GLuint* vbos;

        //! CPU-side data for indices
        std::vector<VBOint> indices;
        //! CPU-side data for vertex positions
        std::vector<float> vertexPositions;
        //! CPU-side data for vertex normals
        std::vector<float> vertexNormals;
        //! CPU-side data for vertex colours
        std::vector<float> vertexColors;
        //! A model-wide alpha value for the shader
        float alpha = 1.0f;
        //! If true, then calls to VisualModel::render should return
        bool hide = false;

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
        //! Push morph::Vector of 3 floats onto the vector of floats \a vp
        void vertex_push (const Vector<float>& vec, std::vector<float>& vp)
        {
            std::copy (vec.begin(), vec.end(), std::back_inserter (vp));
        }

        //! Set up a vertex buffer object - bind, buffer and set vertex array object attribute
        void setupVBO (GLuint& buf, std::vector<float>& dat, unsigned int bufferAttribPosition)
        {
            int sz = dat.size() * sizeof(float);
            glBindBuffer (GL_ARRAY_BUFFER, buf);
            morph::gl::Util::checkError (__FILE__, __LINE__);
            glBufferData (GL_ARRAY_BUFFER, sz, dat.data(), GL_STATIC_DRAW);
            morph::gl::Util::checkError (__FILE__, __LINE__);
            glVertexAttribPointer (bufferAttribPosition, 3, GL_FLOAT, GL_FALSE, 0, (void*)(0));
            morph::gl::Util::checkError (__FILE__, __LINE__);
            glEnableVertexAttribArray (bufferAttribPosition);
            morph::gl::Util::checkError (__FILE__, __LINE__);
        }

        /*!
         * Create a tube from \a start to \a end, with radius \a r and a colour which
         * transitions from the colour \a colStart to \a colEnd.
         *
         * \param idx The index into the 'vertex array'
         * \param start The start of the tube
         * \param end The end of the tube
         * \param colStart The tube staring colour
         * \param colEnd The tube's ending colour
         * \param r Radius of the tube
         * \param segments Number of segments used to render the tube
         */
        void computeTube (VBOint& idx, Vector<float> start, Vector<float> end,
                          std::array<float, 3> colStart, std::array<float, 3> colEnd,
                          float r = 1.0f, int segments = 12)
        {
            // First cap, draw as a triangle fan, but record indices so that
            // we only need a single call to glDrawElements.

            // The vector from start to end defines a vector and a plane. Find a 'circle' of points in that plane.
            Vector<float> vstart = start;
            Vector<float> vend = end;
            //std::cout << "Compute tube from " << vstart << "to " << vend << std::endl;
            Vector<float> v = vend - vstart;
            v.renormalize();
            //std::cout << "Normal vector v is " << v << std::endl;

            // circle in a plane defined by a point (v0 = vstart or vend) and a normal
            // (v) can be found: Choose random vector vr. A vector inplane = vr ^
            // v. The unit in-plane vector is inplane.normalise. Can now use that
            // vector in the plan to define a point on the circle.
            Vector<float> rand_vec;
            rand_vec.randomize();
            Vector<float> inplane = rand_vec.cross(v);
            inplane.renormalize();
            //std::cout << "in-plane vector is " << inplane << std::endl;

            // Now use parameterization of circle inplane = p1-x1 and
            // c1(t) = ( (p1-x1).normalized sin(t) + v.normalized cross (p1-x1).normalized * cos(t) )
            // c1(t) = ( inplane sin(t) + v * inplane * cos(t)
            Vector<float> v_x_inplane = v.cross(inplane);
            //std::cout << "v ^ inplane vector is " << v_x_inplane << std::endl;
            // Point on circle: Vector<float> c = inplane * sin(t) + v_x_inplane * cos(t);
            //std::cout << "Start cap...\n";
            // Push the central point of the start cap - this is at location vstart
            this->vertex_push (vstart, this->vertexPositions);
            //std::cout << "Central point of vstart cap is " << vstart << std::endl;
            this->vertex_push (-v, this->vertexNormals);
            this->vertex_push (colStart, this->vertexColors);

            // Start cap vertices
            for (int j = 0; j < segments; j++) {
                // t is the angle of the segment
                float t = j * morph::TWO_PI_F/(float)segments;
                Vector<float> c = inplane * sin(t) * r + v_x_inplane * cos(t) * r;
                this->vertex_push (vstart+c, this->vertexPositions);
                this->vertex_push (-v, this->vertexNormals);
                this->vertex_push (colStart, this->vertexColors);
            }

            // Intermediate, near start cap. Normals point in direction c
            for (int j = 0; j < segments; j++) {
                float t = j * morph::TWO_PI_F/(float)segments;
                Vector<float> c = inplane * sin(t) * r + v_x_inplane * cos(t) * r;
                this->vertex_push (vstart+c, this->vertexPositions);
                c.renormalize();
                this->vertex_push (c, this->vertexNormals);
                this->vertex_push (colStart, this->vertexColors);
            }

            // Intermediate, near end cap. Normals point in direction c
            for (int j = 0; j < segments; j++) {
                float t = (float)j * morph::TWO_PI_F/(float)segments;
                Vector<float> c = inplane * sin(t) * r + v_x_inplane * cos(t) * r;
                this->vertex_push (vend+c, this->vertexPositions);
                c.renormalize();
                this->vertex_push (c, this->vertexNormals);
                this->vertex_push (colEnd, this->vertexColors);
            }

            // Bottom cap vertices
            for (int j = 0; j < segments; j++) {
                float t = (float)j * morph::TWO_PI_F/(float)segments;
                Vector<float> c = inplane * sin(t) * r + v_x_inplane * cos(t) * r;
                this->vertex_push (vend+c, this->vertexPositions);
                this->vertex_push (v, this->vertexNormals);
                this->vertex_push (colEnd, this->vertexColors);
            }

            // Bottom cap. Push centre vertex as the last vertex.
            this->vertex_push (vend, this->vertexPositions);
            //std::cout << "vend cap is " << vend << std::endl;
            this->vertex_push (v, this->vertexNormals);
            this->vertex_push (colEnd, this->vertexColors);

            // Note: number of vertices = segments * 4 + 2.
            int nverts = (segments * 4) + 2;

            // After creating vertices, push all the indices.
            VBOint capMiddle = idx;
            VBOint capStartIdx = idx + 1;
            VBOint endMiddle = idx + (VBOint)nverts - 1;
            VBOint endStartIdx = capStartIdx + (3*segments);

            //std::cout << "start cap" << std::endl;
            for (int j = 0; j < segments-1; j++) {
                this->indices.push_back (capMiddle);
                //std::cout << "add " << capMiddle << " to indices\n";
                this->indices.push_back (capStartIdx + j);
                //std::cout << "add " << (capStartIdx+j) << " to indices\n";
                this->indices.push_back (capStartIdx + 1 + j);
                //std::cout << "add " << (capStartIdx+1+j) << " to indices\n";
            }
            // Last one
            this->indices.push_back (capMiddle);
            //std::cout << "add " << capMiddle << " to indices\n";
            this->indices.push_back (capStartIdx + segments - 1);
            //std::cout << "add " << (capStartIdx + segments - 1) << " to indices\n";
            this->indices.push_back (capStartIdx);
            //std::cout << "add " << (capStartIdx) << " to indices\n";

            // MIDDLE SECTIONS
            for (int lsection = 0; lsection < 3; ++lsection) {
                capStartIdx = idx + 1 + lsection*segments;
                endStartIdx = capStartIdx + segments;
                //std::cout << "For lsection " << lsection << " capStartIdx=" << capStartIdx
                //          << ", and endStartIdx=" << endStartIdx << std::endl;
                // This does sides between start and end. I want to do this three times.
                for (int j = 0; j < segments; j++) {
                    //std::cout << "Triangle 1\n";
                    this->indices.push_back (capStartIdx + j);
                    //std::cout << "1. add " << (capStartIdx + j) << " to indices\n";
                    if (j == (segments-1)) {
                        this->indices.push_back (capStartIdx);
                        //std::cout << "1. add " << (capStartIdx) << " to indices\n";
                    } else {
                        this->indices.push_back (capStartIdx + 1 + j);
                        //std::cout << "1. add " << (capStartIdx + j + 1) << " to indices\n";
                    }
                    this->indices.push_back (endStartIdx + j);
                    //std::cout << "1. add " << (endStartIdx + j) << " to indices\n";
                    // 2:
                    //std::cout << "Triangle 2\n";
                    this->indices.push_back (endStartIdx + j);
                    //std::cout << "2. add " << (endStartIdx + j) << " to indices\n";
                    if (j == (segments-1)) {
                        this->indices.push_back (endStartIdx);
                        //std::cout << "2. add " << (endStartIdx) << " to indices\n";
                    } else {
                        this->indices.push_back (endStartIdx + 1 + j);
                        //std::cout << "2. add " << (endStartIdx + 1 + j) << " to indices\n";
                    }
                    if (j == (segments-1)) {
                        this->indices.push_back (capStartIdx);
                        //std::cout << "2. add " << (capStartIdx) << " to indices\n";
                    } else {
                        this->indices.push_back (capStartIdx + j + 1);
                        //std::cout << "2. add " << (capStartIdx + j + 1) << " to indices\n";
                    }
                }
            }
            //std::cout << "endStartIdx after loop = " << endStartIdx << std::endl;

            // bottom cap
            //std::cout << "vend cap" << std::endl;
            for (int j = 0; j < segments-1; j++) {
                this->indices.push_back (endMiddle);
                //std::cout << "add " << (endMiddle) << " to indices\n";
                this->indices.push_back (endStartIdx + j);
                //std::cout << "add " << (endStartIdx + j) << " to indices\n";
                this->indices.push_back (endStartIdx + 1 + j);
                //std::cout << "add " << (endStartIdx + 1 + j) << " to indices\n---\n";
            }
            // Last one
            this->indices.push_back (endMiddle);
            //std::cout << "add " << (endMiddle) << " to indices\n";
            this->indices.push_back (endStartIdx + segments - 1);
            //std::cout << "add " << (endStartIdx - 1 + segments) << " to indices\n";
            this->indices.push_back (endStartIdx);
            //std::cout << "add " << (endStartIdx) << " to indices\n";

            // Update idx
            idx += nverts;
        }

        /*!
         * Create a tube from \a start to \a end, with radius \a r and a colour which
         * transitions from the colour \a colStart to \a colEnd.
         *
         * This version of computeTube computes an absolute minimal object, which is
         * acceptable if lighting with ambient light, but which does not have sufficient
         * vertices for the normals to look right when the tube is lighted with a
         * diffuse, positioned light source.
         *
         * \param idx The index into the 'vertex array'
         * \param start The start of the tube
         * \param end The end of the tube
         * \param colStart The tube staring colour
         * \param colEnd The tube's ending colour
         * \param r Radius of the tube
         * \param segments Number of segments used to render the tube
         */
        void computeTubeMin (VBOint& idx, Vector<float> start, Vector<float> end,
                             std::array<float, 3> colStart, std::array<float, 3> colEnd,
                             float r = 1.0f, int segments = 12)
        {
            // The vector from start to end defines a vector and a plane. Find a 'circle' of points in that plane.
            Vector<float> vstart = start;
            Vector<float> vend = end;
            Vector<float> v = vend - vstart;
            v.renormalize();

            // circle in a plane defined by a point (v0 = vstart or vend) and a normal
            // (v) can be found: Choose random vector vr. A vector inplane = vr ^
            // v. The unit in-plane vector is inplane.normalise. Can now use that
            // vector in the plan to define a point on the circle.
            Vector<float> rand_vec;
            rand_vec.randomize();
            Vector<float> inplane = rand_vec.cross(v);
            inplane.renormalize();
            Vector<float> v_x_inplane = v.cross(inplane);

            // Push the central point of the start cap - this is at location vstart
            this->vertex_push (vstart, this->vertexPositions);
            this->vertex_push (-v, this->vertexNormals);
            this->vertex_push (colStart, this->vertexColors);

            for (int j = 0; j < segments; j++) {
                float t = j * morph::TWO_PI_F/(float)segments;
                Vector<float> c = inplane * sin(t) * r + v_x_inplane * cos(t) * r;
                this->vertex_push (vstart+c, this->vertexPositions);
                this->vertex_push (-v, this->vertexNormals); // -v
                this->vertex_push (colStart, this->vertexColors);
            }

            for (int j = 0; j < segments; j++) {
                float t = (float)j * morph::TWO_PI_F/(float)segments;
                Vector<float> c = inplane * sin(t) * r + v_x_inplane * cos(t) * r;
                this->vertex_push (vend+c, this->vertexPositions);
                this->vertex_push (v, this->vertexNormals); // +v
                this->vertex_push (colEnd, this->vertexColors);
            }

            // Bottom cap. Push centre vertex as the last vertex.
            this->vertex_push (vend, this->vertexPositions);
            this->vertex_push (v, this->vertexNormals);
            this->vertex_push (colEnd, this->vertexColors);

            // Note: number of vertices = segments * 2 + 2.
            int nverts = (segments * 2) + 2;

            // After creating vertices, push all the indices.
            VBOint capMiddle = idx;
            VBOint capStartIdx = idx + 1;
            VBOint endMiddle = idx + (VBOint)nverts - 1;
            VBOint endStartIdx = capStartIdx + segments;

            for (int j = 0; j < segments-1; j++) {
                this->indices.push_back (capMiddle);
                this->indices.push_back (capStartIdx + j);
                this->indices.push_back (capStartIdx + 1 + j);
            }
            // Last one
            this->indices.push_back (capMiddle);
            this->indices.push_back (capStartIdx + segments - 1);
            this->indices.push_back (capStartIdx);

            for (int j = 0; j < segments; j++) {
                // Two triangles per side; 1:
                this->indices.push_back (capStartIdx + j);
                if (j == (segments-1)) {
                    this->indices.push_back (capStartIdx);
                } else {
                    this->indices.push_back (capStartIdx + 1 + j);
                }
                this->indices.push_back (endStartIdx + j);
                // 2:
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

            // bottom cap
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
            idx += nverts;
        }

        /*!
         * Code for creating a sphere as part of this model. I'll use a sphere at the centre of the arrows.
         *
         * \param idx The index into the 'vertex indices array'
         * \param so The sphere offset. Where to place this sphere...
         * \param sc The sphere colour.
         * \param r Radius of the sphere
         * \param rings Number of rings used to render the sphere
         * \param segments Number of segments used to render the sphere
         */
        void computeSphere (VBOint& idx, Vector<float> so, std::array<float, 3> sc, float r = 1.0f,
                            int rings = 10, int segments = 12)
        {
            // First cap, draw as a triangle fan, but record indices so that
            // we only need a single call to glDrawElements.
            float rings0 = M_PI * -0.5;
            float _z0  = sin(rings0);
            float z0  = r * _z0;
            float r0 =  cos(rings0);
            float rings1 = M_PI * (-0.5 + 1.0f / rings);
            float _z1 = sin(rings1);
            float z1 = r * _z1;
            float r1 = cos(rings1);
            // Push the central point
            this->vertex_push (so[0]+0.0f, so[1]+0.0f, so[2]+z0, this->vertexPositions);
            this->vertex_push (0.0f, 0.0f, -1.0f, this->vertexNormals);
            this->vertex_push (sc, this->vertexColors);

            VBOint capMiddle = idx++;
            VBOint ringStartIdx = idx;
            VBOint lastRingStartIdx = idx;

            bool firstseg = true;
            for (int j = 0; j < segments; j++) {
                float segment = 2 * M_PI * (float) (j) / segments;
                float x = cos(segment);
                float y = sin(segment);

                float _x1 = x*r1;
                float x1 = _x1*r;
                float _y1 = y*r1;
                float y1 = _y1*r;

                this->vertex_push (so[0]+x1, so[1]+y1, so[2]+z1, this->vertexPositions);
                this->vertex_push (_x1, _y1, _z1, this->vertexNormals);
                this->vertex_push (sc, this->vertexColors);

                if (!firstseg) {
                    this->indices.push_back (capMiddle);
                    this->indices.push_back (idx-1);
                    this->indices.push_back (idx++);
                } else {
                    idx++;
                    firstseg = false;
                }
            }
            this->indices.push_back (capMiddle);
            this->indices.push_back (idx-1);
            this->indices.push_back (capMiddle+1);

            // Now add the triangles around the rings
            for (int i = 2; i < rings; i++) {

                rings0 = M_PI * (-0.5 + (float) (i) / rings);
                _z0  = sin(rings0);
                z0  = r * _z0;
                r0 =  cos(rings0);

                for (int j = 0; j < segments; j++) {

                    // "current" segment
                    float segment = 2 * M_PI * (float)j / segments;
                    float x = cos(segment);
                    float y = sin(segment);

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
                        this->indices.push_back (idx);
                        this->indices.push_back (lastRingStartIdx);
                        this->indices.push_back (lastRingStartIdx);
                        this->indices.push_back (idx++);
                        this->indices.push_back (lastRingStartIdx+segments);
                    } else {
                        this->indices.push_back (ringStartIdx++);
                        this->indices.push_back (idx);
                        this->indices.push_back (ringStartIdx);
                        this->indices.push_back (ringStartIdx);
                        this->indices.push_back (idx++);
                        this->indices.push_back (idx);
                    }
                }
                lastRingStartIdx += segments;
            }

            // bottom cap
            rings0 = M_PI * 0.5;
            _z0  = sin(rings0);
            z0  = r * _z0;
            r0 =  cos(rings0);
            // Push the central point of the bottom cap
            this->vertex_push (so[0]+0.0f, so[1]+0.0f, so[2]+z0, this->vertexPositions);
            this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
            this->vertex_push (sc, this->vertexColors);
            capMiddle = idx++;
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
            // end of sphere calculation
        }

        /*!
         * Create a cone.
         *
         * \param idx The index into the 'vertex array'
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
        void computeCone (VBOint& idx,
                          Vector<float> centre,
                          Vector<float> tip,
                          float ringoffset,
                          std::array<float, 3> col,
                          float r = 1.0f, int segments = 12)
        {
            // Cone is drawn as a base ring around a centre-of-the-base vertex, an
            // intermediate ring which is on the base ring, but has different normals, a
            // 'ring' around the tip (with suitable normals) and a 'tip' vertex

            Vector<float> vbase = centre;
            Vector<float> vtip = tip;
            Vector<float> v = vtip - vbase;
            v.renormalize();

            // circle in a plane defined by a point and a normal
            Vector<float> rand_vec;
            rand_vec.randomize();
            Vector<float> inplane = rand_vec.cross(v);
            inplane.renormalize();
            Vector<float> v_x_inplane = v.cross(inplane);

            // Push the central point of the start cap - this is at location vstart
            this->vertex_push (vbase, this->vertexPositions);
            this->vertex_push (-v, this->vertexNormals);
            this->vertex_push (col, this->vertexColors);

            // Base ring with normals in direction -v
            for (int j = 0; j < segments; j++) {
                float t = j * morph::TWO_PI_F/(float)segments;
                Vector<float> c = inplane * sin(t) * r + v_x_inplane * cos(t) * r;
                // Subtract the vector which makes this circle
                c = c + (c * ringoffset);
                this->vertex_push (vbase+c, this->vertexPositions);
                this->vertex_push (-v, this->vertexNormals); // -v
                this->vertex_push (col, this->vertexColors);
            }

            // Intermediate ring of vertices around/aligned with the base ring with normals in direction c
            for (int j = 0; j < segments; j++) {
                float t = j * morph::TWO_PI_F/(float)segments;
                Vector<float> c = inplane * sin(t) * r + v_x_inplane * cos(t) * r;
                c = c + (c * ringoffset);
                this->vertex_push (vbase+c, this->vertexPositions);
                c.renormalize();
                this->vertex_push (c, this->vertexNormals); // -v
                this->vertex_push (col, this->vertexColors);
            }

            // Intermediate ring of vertices around the tip with normals direction c
            for (int j = 0; j < segments; j++) {
                float t = j * morph::TWO_PI_F/(float)segments;
                Vector<float> c = inplane * sin(t) * r + v_x_inplane * cos(t) * r;
                c = c + (c * ringoffset);
                this->vertex_push (vtip, this->vertexPositions);
                c.renormalize();
                this->vertex_push (c, this->vertexNormals); // -v
                this->vertex_push (col, this->vertexColors);
            }

            // Push tip vertex as the last vertex, normal is in direction v
            this->vertex_push (vtip, this->vertexPositions);
            this->vertex_push (v, this->vertexNormals);
            this->vertex_push (col, this->vertexColors);

            // Number of vertices = segments*3 + 2.
            int nverts = segments*3 + 2;

            // After creating vertices, push all the indices.
            VBOint capMiddle = idx;
            VBOint capStartIdx = idx + 1;
            VBOint endMiddle = idx + (VBOint)nverts - 1;
            VBOint endStartIdx = capStartIdx /*+ segments*/;

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
                capStartIdx = idx + 1 + lsection*segments;
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
            idx += nverts;
        }
    };

} // namespace morph
