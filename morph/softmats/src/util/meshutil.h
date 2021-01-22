#ifndef MESHUTIL_H
#define MESHUTIL_H

#include <vector>
#include <armadillo>
#include <cstdlib>
#include "../core/trianglemesh.h"

namespace morph{ namespace softmats{

class MeshProvider{
private:	

public:		
	/*
		Generates the triangulation of the polygon given by the polyhedron described by the given points
	*/
	virtual TriangleMesh* buildMesh() = 0;

};

class SphereMeshProvider: public MeshProvider{
private:
	float toRadians( float );
public:
	enum SphereType{TYPICAL}; 
	SphereType type;

	SphereMeshProvider( SphereType type );
	TriangleMesh* buildMesh();	
};


class PlaneMeshProvider : public MeshProvider{
private:	

public:
	PlaneMeshProvider( );
	TriangleMesh* buildMesh();
};


class ObjMeshProvider : public MeshProvider{
private:
	const char* path;
public:
	ObjMeshProvider( const char* path );
	TriangleMesh* buildMesh();
};

class ObjMeshProccessChain{
private:
	ObjMeshProccessChain * next;
	virtual bool doProcess( FILE* f, char *s, TriangleMesh* mesh ) = 0;
public:
	ObjMeshProccessChain( ObjMeshProccessChain *next );
	void process( FILE *f, char *s, TriangleMesh* mesh );
};

class VertexChainLink : public ObjMeshProccessChain{
private:
	bool doProcess( FILE *f, char *s, TriangleMesh* mesh );
public:
	VertexChainLink(ObjMeshProccessChain *next);
};

class TextureChainLink : public ObjMeshProccessChain{
private:
	bool doProcess( FILE *f, char *s, TriangleMesh* mesh );
public:
	TextureChainLink(ObjMeshProccessChain *next);
};

class FaceChainLink : public ObjMeshProccessChain{
private:
	std::vector<arma::vec> normals;
	bool doProcess( FILE *f, char *s, TriangleMesh* mesh );
public:
	FaceChainLink(ObjMeshProccessChain *next);
};

}}
#endif