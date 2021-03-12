#include "meshutil.h"
#include "../core/point.h"
#include "../core/face.h"

using arma::vec;
using arma::mat;
using namespace morph::softmats;

// SphereMeshProvider
SphereMeshProvider::SphereMeshProvider( SphereType type ){
    this->type = type;
}


float SphereMeshProvider::toRadians( float degrees ){
    return (degrees *2.0f*3.141592f)/360.0;
}

TriangleMesh* SphereMeshProvider::buildMesh(){
    int prec = 16;
    int numVertices = (prec + 1)*(prec + 1);
    TriangleMesh *mesh = new TriangleMesh();
    std::vector<Point *>& vertices = mesh->getVertices();
    std::vector<Face *>& faces = mesh->getFaces();

    for( int i = 0; i < numVertices; ++i ){
        Point *p = new Point();
        p->x = {0, 0, 0};
        p->v = {0, 0, 0};
        vertices.push_back(p);
        p->uv = glm::vec2();
    }

    float x,y,z, r = 3.0f;
    int k;
    // Calculate triangle vertices
    for( int i = 0; i <= prec; ++i ){
        for( int j = 0; j <= prec; ++j ){
            k = i*(prec+1) +j;
            y = (float) r*cos(toRadians(180.0f - i*180.0f/prec) );
            x = -(float) r*cos(toRadians(j*360.0f/prec))*(float)fabs(cos(asin(y/r)));
            z = (float) r*sin(toRadians(j*360.0f/prec))*(float)fabs(cos(asin(y/r)));

            vertices[k]->x = {x, y, z};
            vertices[k]->uv = glm::vec2((float)j/(prec), (float)i/(prec));
            vertices[k]->normal = {x, y, z};
        }
    }

    // calculate triangle indices
    for( int i = 0; i < prec; ++i ){
        for( int j = 0; j < prec; ++j ){
            Face *f1 = new Face(vertices[i*(prec + 1) + j], vertices[i*(prec + 1) + j + 1],  vertices[(i+1)*(prec + 1) + j]);
            Face *f2 = new Face( vertices[i*(prec + 1) + j + 1], vertices[(i+1)*(prec + 1) + j + 1], vertices[(i+1)*(prec + 1) + j]);
            faces.push_back(f1);
            faces.push_back(f2);
        }
    }
    return mesh;
}

// PlaneMeshProvider
PlaneMeshProvider::PlaneMeshProvider(){

}

TriangleMesh* PlaneMeshProvider::buildMesh( ){

    TriangleMesh *mesh = new TriangleMesh();
    std::vector<Point *>& vertices = mesh->getVertices();
    std::vector<Face *>& faces = mesh->getFaces();
    float height = 0.0;

    float s = 8.0f;
    Point *p1 = new Point();
    p1->x = {s, height, s};
    p1->x_c = {s, height, s};
    p1->normal = {0,1,0};
    Point *p2 = new Point();
    p2->x = {s, height, -s};
    p2->x_c = {s, height, -s};
    p2->normal = {0,1,0};
    Point *p3 = new Point();
    p3->x = {-s, height,-s};
    p3->x_c = {-s, height,-s};
    p3->normal = {0,1,0};
    Point *p4 = new Point();
    p4->x = {-s, height, s};
    p4->x_c = {-s, height, s};
    p4->normal = {0,1,0};

    vertices.push_back(p1);
    vertices.push_back(p2);
    vertices.push_back(p3);
    vertices.push_back(p4);
    Face *f1 = new Face(vertices[0], vertices[1], vertices[2]);
    Face *f2 = new Face(vertices[0], vertices[2], vertices[3]);
    f1->normal = f1->normal = {0, 1, 0};
    f2->normal = f2->normal = {0, 1, 0};
    f1->normal_c = f1->normal = {0, 1, 0};
    f2->normal_c = f2->normal = {0, 1, 0};
    faces.push_back(f1);
    faces.push_back(f2);

    // texCoords
    p1->uv = {2.0f, 2.0f};
    p2->uv = {2.0f, 0.0f};
    p3->uv = {0.0f, 0.0f};
    p4->uv = {0.0f, 2.0f};

    return mesh;
}

// ObjMeshProvider
ObjMeshProvider::ObjMeshProvider( std::string path ){
    this->path = path;
}


TriangleMesh* ObjMeshProvider::buildMesh(){

    FILE *f = fopen( this->path.c_str(), "r" );
    ObjMeshProccessChain *chain = new VertexChainLink(
        new TextureChainLink(
            new FaceChainLink(NULL)));
    TriangleMesh* mesh = new TriangleMesh();

    if( f == NULL ){
        std::cerr << "No mesh file present." << std::endl;
        exit(0);
        return NULL;
    }

    char lineheader[128];

    while( fscanf( f, "%s", lineheader ) != EOF ){
        chain->process( f, lineheader, mesh );
    }

    fclose( f );

    mesh->updateVertexNormals();
    mesh->center();

    return mesh;
}

// ObjMeshProccessChain
ObjMeshProccessChain::ObjMeshProccessChain( ObjMeshProccessChain *next ){
    this->next = next;
}

void ObjMeshProccessChain::process( FILE *f, char *s, TriangleMesh* mesh ){
    this->doProcess( f, s, mesh );

    if( this->next != NULL )
        this->next->process( f, s, mesh );
}

// VertexChainLink
VertexChainLink::VertexChainLink(ObjMeshProccessChain *next):ObjMeshProccessChain( next ){}

bool VertexChainLink::doProcess( FILE *f, char *s, TriangleMesh* mesh ){
    if( strcmp( s, "v" ) == 0 ){

        float x, y, z;
        fscanf(f, "%f %f %f\n", &x, &y, &z );

        Point *p = new Point();
        p->x = { x, y, z };
        mesh->getVertices().push_back( p );
        return true;
    }

    return false;
}

// TextureChainLink
TextureChainLink::TextureChainLink(ObjMeshProccessChain *next):ObjMeshProccessChain( next ){}

bool TextureChainLink::doProcess( FILE *f, char *s, TriangleMesh* mesh ){
    // do nothing
    if( strcmp( s, "vt" ) == 0 ){
        std::cout << "Not processing textures so far\n";
    }

    return false;
}

// FaceChainLink
FaceChainLink::FaceChainLink(ObjMeshProccessChain *next):ObjMeshProccessChain( next ){}

bool FaceChainLink::doProcess( FILE *f, char *s, TriangleMesh* mesh ){
    if( strcmp( s, "vn" ) == 0 ){
        float x, y, z;
        fscanf(f, "%f %f %f\n", &x, &y, &z );
        vec pos = { x, y, z };
        this->normals.push_back( pos );
        return true;
    }

    if( strcmp( s, "f" ) == 0 ){

        // FIXME alejandro: What does this do? It's causing lots of warnings, but if I remove the fscanf line, then the code crashes! Please fix the warnings ;)
        int vi, vj, vk, ti, tj, tk, ni, nj, nk;
        // int matches = fscanf(f, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vi, &ti, &ni, &vj, &tj, &nj, &vk, &tk, &nk );
        int matches = fscanf(f, "%d//%d %d//%d %d//%d\n", &vi, &ni, &vj, &nj, &vk, &nk );
        // if (matches != 9){
        //     cerr<<"Error in the format of the mesh file"<<endl;
        //     return false;
        // }
        //cout << "Adding face: "<< vi << ", "<< vj << ", "<< vk <<endl;
        std::vector<Point *>& vertices = mesh->getVertices();
        std::vector<Face *>& faces = mesh->getFaces();

        Point *p1 = vertices[--vi];
        Point *p2 = vertices[--vj];
        Point *p3 = vertices[--vk];
        Face *f = new Face( p1, p2, p3 );
        f->normal = normals[--nk];
        faces.push_back( f );

        return true;
    }

    return false;
}
