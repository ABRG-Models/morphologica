#ifndef MESHUTIL_H
#define MESHUTIL_H

#include <vector>
#include <armadillo>
#include <cstdlib>
#include "../core/trianglemesh.h"

namespace morph{ namespace softmats{
/**
 * Class hierarchy to load or generate meshes
 * 
 * @author Alejandro Jimenez Rodriguez
 */

// General interface
class MeshProvider{
private:	

public:		
	/*
		Generates the triangulation of the polygon given by the polyhedron described by the given points
	*/
	virtual TriangleMesh* buildMesh() = 0;

};

/**
 * Procedural generation of a sphere mesh 
 * @author Alejandro Jimenez Rodriguez
 */
class SphereMeshProvider: public MeshProvider{
private:
	float toRadians( float );
public:
	enum SphereType{TYPICAL}; // More kinds of spheres expected to be supported in the future
	SphereType type;
	SphereMeshProvider( SphereType type );
	TriangleMesh* buildMesh();	
};

/**
 * Provides a mesh for the ground plane
 * @author Alejandro Jimenez Rodriguez
 */
class PlaneMeshProvider : public MeshProvider{
private:
public:
	PlaneMeshProvider( );
	TriangleMesh* buildMesh();
};

/**
 * Loads the mesh from a given obj file
 * 
 * @author Alejandro Jimenez Rodriguez
 */
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