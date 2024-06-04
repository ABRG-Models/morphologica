#include <morph/vec.h>
#include <morph/vvec.h>
#include <morph/Quaternion.h>
#include <morph/TransformMatrix.h>

namespace morph {
    namespace geometry {

        //! Compute the vertices and face indices for an icosahedron
        static void icosahedron (morph::vvec<morph::vec<float, 3>>& vertices,
                                 morph::vvec<morph::vec<int, 3>>& faces)
        {
            constexpr float phi = (1.0f + sqrt(5.0f)) / 2.0f;

            // Arranged 'in spiral order', going with positive angle in x/y plane (i.e. around z axis)
            vertices = {
                {-1.0f,  phi,   0.0f },

                { 1.0f,  phi,   0.0f },
                { 0.0f,  1.0f, -phi  },
                {-phi,   0.0f, -1.0f },
                {-phi,   0.0f,  1.0f },
                { 0.0f,  1.0f,  phi  },

                { phi,   0.0f,  1.0f },
                { phi,   0.0f, -1.0f },
                { 0.0f, -1.0f, -phi  },
                {-1.0f, -phi,   0.0f },
                { 0.0f, -1.0f,  phi  },

                { 1.0f, -phi,   0.0f }
            };

            // Set up the transform matrix for our rotation, made up of a rotation about the z axis...
            morph::Quaternion<float> rotn1;
            rotn1.rotate (0.0f, 0.0f, 1.0f, std::atan2(1.0f, phi));
            // ...and a rotation about the x axis:
            morph::Quaternion<float> rotn2;
            rotn2.rotate (1.0f, 0.0f, 0.0f, -morph::mathconst<float>::pi_over_2);
            // We then translate the quaternions into a transform matrix:
            morph::TransformMatrix<float> rmat;
            rmat.rotate (rotn1 * rotn2);

            // For each vertex, apply rotational transform and renormalize
            morph::vec<float, 4> v4;
            for (auto &vertex : vertices) {
                v4 = rmat * vertex;         // Apply the rotation (returns 4D vector)
                vertex = v4.less_one_dim(); // Extract 3D vector
                vertex.renormalize();       // Make it length 1
            }
            // after this, the vertex order is no longer spiral from top to bottom...

            // Each face is defined by three vertices
            faces = {
                {0, 1, 2}, {0, 2, 3}, {0, 3, 4}, {0, 4, 5}, {0, 5, 1},
                {5, 6, 1}, {1, 6, 7}, {1, 7, 2}, {2, 7, 8}, {2, 8, 3}, {3, 8, 9}, {3, 9, 4}, {4, 9, 10}, {4, 10, 5}, {5, 10, 6},
                {10, 11, 6}, {6, 11, 7}, {7, 11, 8}, {8, 11, 9}, {9, 11, 10}
            };
        }

    } // geometry
} // morph
