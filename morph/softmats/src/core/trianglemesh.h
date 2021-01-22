#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "face.h"
#include "point.h"
#include "edge.h"

namespace morph{ namespace softmats{

class TriangleMesh{
private:
    std::vector<Face*> faces;
    std::vector<Point*> vertices;
    std::vector<glm::vec2> texCoords;
public:

    TriangleMesh();

    std::vector<Edge> getFaceEdges( Face* f );
    std::vector<Edge> getPointEdges( Point* p );

    void computeNormals( bool candidate );
    void updateVertexNormals();

    std::vector<Point *>& getVertices();
    std::vector<Face *>& getFaces();
    std::vector<glm::vec2>& getTexCoords();
    int getNumVertices();

    void translate( float x, float y, float z );
    void scale( float s );
    void center();

    ~TriangleMesh();
};

}}