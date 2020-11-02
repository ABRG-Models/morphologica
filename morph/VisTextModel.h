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

#ifdef __OSX__
# include <OpenGL/gl3.h>
#else
# include <GL3/gl3.h>
#endif
#include <morph/TransformMatrix.h>
#include <morph/Vector.h>
#include <morph/MathConst.h>
#include <vector>
#include <array>
#include <map>

// Common definitions
#include <morph/VisualCommon.h>

namespace morph {

    //! Forward declaration of a Visual class
    class Visual;

    /*!
     * A separate data-containing model which is used to render text. It is intended
     * that this could comprise part of a morph::Visual or a morph::VisualModel. It has
     * its own render call.
     */
    class VisTextModel
    {
    public:
        // Construct with given text shader program id, \a tsp and a spatial \a _offset.
        VisTextModel (GLuint tsp, const morph::Vector<float> _offset)
        {
            this->tshaderprog = tsp;
            this->offset = _offset;
            this->viewmatrix.translate (this->offset);
        }

        virtual ~VisTextModel()
        {
            glDeleteBuffers (numVBO, vbos);
            delete (this->vbos);
        }

        //! With the given text and font size information, create the quads for the text.
        void setupText (const std::string& txt, std::map<char, morph::Character>& _the_characters, float fscale = 1.0f)
        {
            this->fontscale = fscale;
            // With glyph information from txt, set up this->quads.
            this->quads.clear();
            this->quad_ids.clear();
            // Our string of letters starts at this location
            float letter_pos = this->offset[0];
            for (std::string::const_iterator c = txt.begin(); c != txt.end(); c++) {
                // Add a quad to this->quads
                Character ch = _the_characters[*c];

                float xpos = letter_pos + ch.Bearing.x() * this->fontscale;
                float ypos = this->offset[1] - (ch.Size.y() - ch.Bearing.y()) * this->fontscale;
                float w = ch.Size.x() * this->fontscale;
                float h = ch.Size.y() * this->fontscale;

                // What's the order of the vertices for the quads? It is:
                // Bottom left, Top left, top right, bottom right.
                std::array<float,12> tbox = { xpos,   ypos,     this->offset[2],
                                              xpos,   ypos+h,   this->offset[2],
                                              xpos+w, ypos+h,   this->offset[2],
                                              xpos+w, ypos,     this->offset[2] };
#ifdef __DEBUG__
                std::cout << "Text box added as quad from\n("
                          << tbox[0] << "," << tbox[1] << "," << tbox[2]
                          << ") to (" << tbox[3] << "," << tbox[4] << "," << tbox[5]
                          << ") to (" << tbox[6] << "," << tbox[7] << "," << tbox[8]
                          << ") to (" << tbox[9] << "," << tbox[10] << "," << tbox[11]
                          << "). w="<<w<<", h="<<h<<"\n";
                std::cout << "Texture ID for that character is: " << ch.TextureID << std::endl;
#endif
                this->quads.push_back (tbox);
                this->quad_ids.push_back (ch.TextureID);

                // The value in ch.Advance has to be divided by 64 to bring it into the
                // same units as the ch.Size and ch.Bearing values.
                letter_pos += ((ch.Advance>>6)*this->fontscale);
            }

            // Ensure we've cleared out vertex info
            this->vertexPositions.clear();
            this->vertexNormals.clear();
            this->vertexColors.clear();
            this->vertexTextures.clear();

            this->initializeVertices();

            this->postVertexInit();
        }

        //! The colour of the backing quad's vertices. Doesn't have any effect.
        std::array<float, 3> clr_backing = {1.0f, 1.0f, 1.0f};
        //! The colour of the text
        std::array<float, 3> clr_text = {0.0f, 0.0f, 0.0f};

        //! Initialize the vertices that will represent the Quads.
        void initializeVertices (void) {

            unsigned int nquads = this->quads.size();

            for (unsigned int qi = 0; qi < nquads; ++qi) {

                std::array<float, 12> quad = this->quads[qi];
#ifdef __DEBUG__
                std::cout << "Quad box from (" << quad[0] << "," << quad[1] << "," << quad[2]
                          << ") to (" << quad[3] << "," << quad[4] << "," << quad[5]
                          << ") to (" << quad[6] << "," << quad[7] << "," << quad[8]
                          << ") to (" << quad[9] << "," << quad[10] << "," << quad[11] << ")" << std::endl;
#endif
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
                unsigned int ib = qi*4;
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
            // Create vertex array object
#ifdef __MACS_HAD_OPENGL_450__
            glCreateVertexArrays (1, &this->vao); // OpenGL 4.5 only
#else
            glGenVertexArrays (1, &this->vao); // Safe for OpenGL 4.4-
#endif
            morph::GLutil::checkError (__FILE__, __LINE__);

            glBindVertexArray (this->vao);
            morph::GLutil::checkError (__FILE__, __LINE__);

            // Create the vertex buffer objects
            this->vbos = new GLuint[numVBO];
#ifdef __MACS_HAD_OPENGL_450__
            glCreateBuffers (numVBO, this->vbos); // OpenGL 4.5 only
#else
            glGenBuffers (numVBO, this->vbos); // OpenGL 4.4- safe
#endif
            morph::GLutil::checkError (__FILE__, __LINE__);

            // Set up the indices buffer - bind and buffer the data in this->indices
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vbos[idxVBO]);
            morph::GLutil::checkError (__FILE__, __LINE__);

            //std::cout << "indices.size(): " << this->indices.size() << std::endl;
            int sz = this->indices.size() * sizeof(VBOint);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sz, this->indices.data(), GL_STATIC_DRAW);
            morph::GLutil::checkError (__FILE__, __LINE__);

            // Binds data from the "C++ world" to the OpenGL shader world for
            // "position", "normalin" and "color"
            // (bind, buffer and set vertex array object attribute)
            this->setupVBO (this->vbos[posnVBO], this->vertexPositions, posnLoc);
            this->setupVBO (this->vbos[normVBO], this->vertexNormals, normLoc);
            this->setupVBO (this->vbos[colVBO], this->vertexColors, colLoc);
            this->setupVBO (this->vbos[textureVBO], this->vertexTextures, textureLoc);

#ifdef CAREFULLY_UNBIND_AND_REBIND
            // Possibly release (unbind) the vertex buffers, but have to unbind vertex
            // array object first.
            glBindVertexArray(0);
            morph::GLutil::checkError (__FILE__, __LINE__);
#endif
        }

        //! Render the VisTextModel
        void render()
        {
            if (this->hide == true) { return; }

            // Ensure the correct program is in play for this VisualModel
            glUseProgram (this->tshaderprog);

            glUniform3f (glGetUniformLocation(this->tshaderprog, "textColor"),
                         this->clr_text[0], this->clr_text[1], this->clr_text[2]);

            glActiveTexture (GL_TEXTURE0);

            // It is only necessary to bind the vertex array object before rendering
            glBindVertexArray (this->vao);

            // Pass this->float to GLSL so the model can have an alpha value.
            GLint loc_a = glGetUniformLocation (this->tshaderprog, (const GLchar*)"alpha");
            if (loc_a != -1) { glUniform1f (loc_a, this->alpha); }

            for (size_t i = 0; i < quads.size(); ++i) {
                // Bind the right texture for the quad.
                glBindTexture (GL_TEXTURE_2D, this->quad_ids[i]);
                // This is 'draw a subset of the elements from the vertex array
                // object'. You say how many indices to draw and which base *vertex* you
                // start from. In my scheme, I have 4 vertices for each two triangles
                // that are constructed. Thus, I draw 6 indices, but increment the base
                // vertex by 4 for each letter.
                glDrawElementsBaseVertex (GL_TRIANGLES, 6, VBO_ENUM_TYPE, 0, 4*i);
            }

            glBindVertexArray(0);
            morph::GLutil::checkError (__FILE__, __LINE__);
        }

        //! The text-model-specific view matrix.
        TransformMatrix<float> viewmatrix;

    protected:

        //! Offset within the parent model or scene.
        Vector<float> offset;
        //! The Quads that form the 'medium' for the text textures. 12 float = 4 corners
        std::vector<std::array<float,12>> quads;
        //! The texture ID for each quad - so that we draw the right texture image over each quad.
        std::vector<unsigned int> quad_ids;
        //! A scaling factor for the text
        float fontscale = 1.0f;
        //! Position within vertex buffer object (if I use an array of VBO)
        enum VBOPos { posnVBO, normVBO, colVBO, idxVBO, textureVBO, numVBO };
        //! The parent Visual object - provides access to the shader prog
        const Visual* parent;
        //! A copy of the reference to the text shader program
        GLuint tshaderprog;
        //! The OpenGL Vertex Array Object
        GLuint vao;
        //! Single vbo to use as in example
        GLuint vbo;
        //! Vertex Buffer Objects stored in an array
        GLuint* vbos;
        //! CPU-side data for indices
        std::vector<VBOint> indices;
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
        //! Push morph::Vector of 3 floats onto the vector of floats \a vp
        void vertex_push (const Vector<float>& vec, std::vector<float>& vp)
        {
            std::copy (vec.begin(), vec.end(), std::back_inserter (vp));
        }
    };

} // namespace morph
