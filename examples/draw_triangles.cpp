#include <morph/Visual.h>

#include <morph/vec.h>
#include <morph/VisualModel.h>
#include <morph/mathconst.h>
#include <array>

using namespace morph;

//! This class creates the vertices for a single triangle. It also draws some spheres and arrows
//! for coordinates and normal vectors, for illustration.
template<int glver = morph::gl::version_4_1>
class trivis : public VisualModel<glver>
{
public:
    trivis (const morph::vec<float> _offset = {0,0,0}) : morph::VisualModel<glver>::VisualModel (_offset) {}

    //! Compute a triangle from 3 coordinates
    void computeTriangle()
    {
        // v is the face normal
        vec<float> u1 = coords[0] - coords[1];
        vec<float> u2 = coords[1] - coords[2];
        this->normal = u1.cross(u2);
        this->normal.renormalize();
        // Push corner vertices, colours and normals
        for (unsigned int i = 0; i < 3U; ++i) {
            this->vertex_push (this->coords[i], this->vertexPositions);
            this->vertex_push (this->colours[i], this->vertexColors);
            this->vertex_push (this->normal, this->vertexNormals);
        }

        // This is just index 0, 1, 2
        this->indices.push_back (this->idx);
        this->indices.push_back (this->idx + 1);
        this->indices.push_back (this->idx + 2);

        this->idx += 3; // Move the index counter on for the next drawing
    }

    //! Initialize vertex buffer objects and vertex array object.
    void initializeVertices() {
        // Compute the triangle
        this->computeTriangle();

        // Show indices/coords JUST from the triangble
        vvec<unsigned int> indvv;
        indvv.set_from (this->indices);
        this->addLabel (std::string("Index draw order: ") + indvv.str(), {0, -0.6, 0}, morph::TextFeatures(0.16));

        this->addLabel (std::string("Vtx 0 ") + coords[0].str(), coords[0] + vec<float>{-0.3, -0.2, 0}, morph::TextFeatures(0.1));
        this->addLabel (std::string("Vtx 1 ") + coords[1].str(), coords[1] + vec<float>{-0.3, -0.2, 0}, morph::TextFeatures(0.1));
        this->addLabel (std::string("Vtx 2 ") + coords[2].str(), coords[2] + vec<float>{-0.3, 0.2, 0}, morph::TextFeatures(0.1));

        this->addLabel ("Vertex normals: " + this->normal.str(), {0, -0.9, 0}, morph::TextFeatures(0.16));

        // Add illustrative stuff
        for (unsigned int i = 0; i < 3U; ++i) {
            this->computeSphere (this->coords[i], this->colours[i], 0.05f, 16, 18);
            this->computeArrow (this->coords[i], this->coords[i] + this->normal, this->colours[i], 0.015f);
        }
    }

    //! The positions of the vertices of the triangle
    std::array<vec<float>, 3> coords = { vec<float>{0, 0, 0},
        vec<float>{2, 0, 0},
        vec<float>{0, 2, 0} };

    std::array<std::array<float, 3>, 3> colours = { morph::colour::firebrick,
        morph::colour::orchid1,
        morph::colour::navy };

    vec<float> normal = {0, 0, 0};
};

//! Like trivis, but with one extra vertex and drawing two triangles.
template<int glver = morph::gl::version_4_1>
class doubletrivis : public VisualModel<glver>
{
public:
    doubletrivis (const morph::vec<float> _offset = {0,0,0}) : morph::VisualModel<glver>::VisualModel (_offset) {}

    //! Compute two triangles from 4 corners
    void computeTriangles()
    {
        // v is the face normal
        vec<float> u1 = coords[0] - coords[1];
        vec<float> u2 = coords[1] - coords[2];
        this->normal = u1.cross(u2);
        this->normal.renormalize();

        // Push corner vertices, colours and normals
        for (unsigned int i = 0; i < 4U; ++i) {
            this->vertex_push (this->coords[i], this->vertexPositions);
            this->vertex_push (this->colours[i], this->vertexColors);
            this->vertex_push (this->normal, this->vertexNormals);
        }

        // First triangle 0, 1, 2
        this->indices.push_back (this->idx);
        this->indices.push_back (this->idx + 1);
        this->indices.push_back (this->idx + 2);

        // Second triangle 1, 3, 2
        this->indices.push_back (this->idx + 1);
        this->indices.push_back (this->idx + 3);
        this->indices.push_back (this->idx + 2);

        this->idx += 4; // Increase by the *number of vertices that we added*
    }

    //! Initialize vertex buffer objects and vertex array object.
    void initializeVertices() {
        // Compute the triangle
        this->computeTriangles();

        // Show indices/coords JUST from the triangble
        vvec<unsigned int> indvv;
        indvv.set_from (this->indices);
        this->addLabel (std::string("Index draw order: ") + indvv.str(), {0, -0.6, 0}, morph::TextFeatures(0.16));

        this->addLabel (std::string("Vtx 0 ") + coords[0].str(), coords[0] + vec<float>{-0.3, -0.2, 0}, morph::TextFeatures(0.1));
        this->addLabel (std::string("Vtx 1 ") + coords[1].str(), coords[1] + vec<float>{-0.3, -0.2, 0}, morph::TextFeatures(0.1));
        this->addLabel (std::string("Vtx 2 ") + coords[2].str(), coords[2] + vec<float>{-0.3, 0.2, 0}, morph::TextFeatures(0.1));
        this->addLabel (std::string("Vtx 3 ") + coords[3].str(), coords[3] + vec<float>{-0.3, 0.2, 0}, morph::TextFeatures(0.1));

        //this->addLabel ("Vertex normals: " + this->normal.str(), vec<float>{1.4, 1.4, 0}, morph::TextFeatures(0.16));

        // Add illustrative stuff
        for (unsigned int i = 0; i < 4U; ++i) {
            this->computeSphere (this->coords[i], this->colours[i], 0.05f, 16, 18);
            this->computeArrow (this->coords[i], this->coords[i] + this->normal, this->colours[i], 0.015f);
        }
    }

    //! The positions of the vertices of the triangle
    std::array<vec<float>, 4> coords = { vec<float>{0, 0, 0},
        vec<float>{2, 0, 0},
        vec<float>{0, 2, 0},
        vec<float>{2, 2, 0} };

    std::array<std::array<float, 3>, 4> colours = { morph::colour::firebrick,
        morph::colour::orchid1,
        morph::colour::navy,
        morph::colour::lightblue2 };

    vec<float> normal = {0, 0, 0};
};

//! This class creates the vertices for two triangles where you can see the colour difference -
//! this has to be made with 6 vertices, four of which share two locations.
template<int glver = morph::gl::version_4_1>
class twocolourtri : public VisualModel<glver>
{
public:
    twocolourtri (const morph::vec<float> _offset = {0,0,0}) : morph::VisualModel<glver>::VisualModel (_offset) {}

    //! Compute a triangle from 3 arbitrary corners
    void computeTriangles()
    {
        // v is the face normal
        vec<float> u1 = coords[0] - coords[1];
        vec<float> u2 = coords[1] - coords[2];
        this->normals[0] = u1.cross(u2);
        this->normals[0].renormalize();

        u1 = coords[3] - coords[4];
        u2 = coords[4] - coords[5];
        this->normals[1] = u1.cross(u2);
        this->normals[1].renormalize();

        // Push corner vertices, colours and normals
        for (unsigned int i = 0; i < 6U; ++i) {
            this->vertex_push (this->coords[i], this->vertexPositions);
            this->vertex_push (this->colours[i], this->vertexColors);
            this->vertex_push (this->normals[i<3?0:1], this->vertexNormals);
        }

        // First triangle 0, 1, 2
        this->indices.push_back (this->idx);
        this->indices.push_back (this->idx + 1);
        this->indices.push_back (this->idx + 2);

        // Second triangle 1, 3, 2
        this->indices.push_back (this->idx + 3);
        this->indices.push_back (this->idx + 4);
        this->indices.push_back (this->idx + 5);

        this->idx += 6; // Increase by the *number of vertices that we added*
    }

    //! Initialize vertex buffer objects and vertex array object.
    void initializeVertices() {
        // Compute the triangle
        this->computeTriangles();

        // Show indices/coords JUST from the triangble
        vvec<unsigned int> indvv;
        indvv.set_from (this->indices);
        this->addLabel (std::string("Index draw order: ") + indvv.str(), {0, -0.6, 0}, morph::TextFeatures(0.16));

        this->addLabel (std::string("Vtx 0 ") + coords[0].str(), coords[0] + vec<float>{-0.3, -0.2, 0}, morph::TextFeatures(0.1));
        this->addLabel (std::string("Vtx 1 & 3 ") + coords[1].str(), coords[1] + vec<float>{-0.3, -0.2, 0}, morph::TextFeatures(0.1));
        this->addLabel (std::string("Vtx 2 & 5") + coords[2].str(), coords[2] + vec<float>{-0.3, 0.2, 0}, morph::TextFeatures(0.1));
        this->addLabel (std::string("Vtx 4 ") + coords[4].str(), coords[4] + vec<float>{-0.3, 0.2, 0}, morph::TextFeatures(0.1));


        // Add illustrative stuff
        for (unsigned int i = 0; i < 6U; ++i) {
            this->computeSphere (this->coords[i], this->colours[i], 0.05f, 16, 18);
            this->computeArrow (this->coords[i], this->coords[i] + this->normals[i<3?0:1], this->colours[i], 0.015f);
        }
    }

    //! The positions of the vertices of the triangle
    std::array<vec<float>, 6> coords = { vec<float>{0, 0, -0.2},
        vec<float>{2, 0, 0},
        vec<float>{0, 2, 0},

        vec<float>{2, 0, 0},
        vec<float>{2, 2, -0.2},
        vec<float>{0, 2, 0} };

    std::array<std::array<float, 3>, 6> colours = { morph::colour::orchid1,
        morph::colour::firebrick,
        morph::colour::firebrick,

        morph::colour::navy,
        morph::colour::orchid1,
        morph::colour::navy };

    std::array<vec<float>, 2> normals = { vec<float>{0, 0, 0}, vec<float>{0, 0, 0} };
};

int main()
{
    morph::Visual v(1024, 768, "Drawing with triangles");
    v.lightingEffects();

    auto tv = std::make_unique<trivis<>>(morph::vec<float>{0,0,0});
    v.bindmodel (tv);
    tv->finalize();
    v.addVisualModel (tv);

    auto dtv = std::make_unique<doubletrivis<>>(morph::vec<float>{3,0,0});
    v.bindmodel (dtv);
    dtv->finalize();
    v.addVisualModel (dtv);

    auto tctv = std::make_unique<twocolourtri<>>(morph::vec<float>{6,0,0});
    v.bindmodel (tctv);
    tctv->finalize();
    v.addVisualModel (tctv);

    v.keepOpen();
    return 0;
}
