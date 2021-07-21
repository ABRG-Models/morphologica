#pragma once
#include <vector>
#include "face.h"
#include "point.h"
#include "edge.h"

namespace morph{ namespace softmats{

class TriangleMesh{
 /**
 * Mesh data structure
 *
 * Represents a mesh for a given body. This implementation can be made more
 * efficient through a half-edge implementation (TO-DO).
 *
 * @author Alejandro Jimenez Rodriguez
 */
private:
    // Triangular faces
    std::vector<Face*> faces;
    // Vertices
    std::vector<Point*> vertices;
public:

    TriangleMesh();

    // Computes the edges of given face
    std::vector<Edge> getFaceEdges( Face* f );
    // Computes the edges incident on a given point
    std::vector<Edge> getPointEdges( Point* p );

    /**
     * Recomputes the normals of the faces.
     *
     * @param candidate - If true, the normals are computed for the candidate position
     * @see pbdim.h
     */
    void computeNormals( bool candidate );
    // Updates the vertex normals based upon the face normals
    void updateVertexNormals();

    // getters
    std::vector<Point *>& getVertices();
    std::vector<Face *>& getFaces();
    int getNumVertices();

    // Mesh wide Transformations
    void translate( float x, float y, float z );
    void scale( float s );
    void center();

    ~TriangleMesh();
};

}}
