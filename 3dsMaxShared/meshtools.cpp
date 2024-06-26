// (C) 2002 S2 Games

// meshtools.cpp

// useful mesh functions for max plugins


#include "meshtools.h"

extern "C"
{
#include "../core/savage_common.h"
}

/*
#include "magicfmlibtype.h"
//using namespace mgc;
#include "mgcConvexHull3d.h"
//using namespace std;
*/
#define	_FACEVTX(mesh, face, idx) mesh.verts[mesh.faces[face].v[idx]]


void BaryToXYZ(float u, float v, Point3& p0, Point3& p1, Point3& p2, Point3& out)
{
	Point3 a,b,c;

	a = p0 * (1-u-v);
	b = p1 * u;
	c = p2 * v;

	out = a + b + c;
}

//determines if a mesh is convex (may fail if the mesh is degenerate?)
//probably a better way to do this...
bool	TestMeshConvexity(Mesh& mesh)
{
	//get the adjacency information
	AdjEdgeList adjedge = AdjEdgeList(mesh);
	AdjFaceList adj = AdjFaceList(mesh, adjedge);

	for (int i=0; i<mesh.numFaces; i++)
	{
		Point3 pointInTri;

		//get a point on the center of the triangle (could be any point on the triangle that isn't on an edge
		BaryToXYZ(0.5, 0.5, _FACEVTX(mesh,i,0), _FACEVTX(mesh,i,1), _FACEVTX(mesh,i,2), pointInTri);

		for (int n=0; n<3; n++)
		{
			int faceidx;
			
			faceidx = adj[i].f[n];
			
			if (faceidx == i)
				continue;
			if (faceidx == UNDEFINED)
			{
//				MessageBox(theModelExport.hPanel, fmt("UNDEFINED face vert on %i %i\n",n,i),"",MB_OK);
				return false;
			}

			Point3 p0,p1,p2;

			p0 = _FACEVTX(mesh,faceidx,0);
			p1 = _FACEVTX(mesh,faceidx,1);
			p2 = _FACEVTX(mesh,faceidx,2);

			//test the triangle center against adjacent planes
			plane_t plane;
			
			M_CalcPlane((float*)(p0),(float*)(p1),(float*)(p2), &plane);
			float *pit = (float*)(pointInTri);
			if (M_DotProduct(pit, plane.normal) - plane.dist > EPSILON)
			{
	/*			MessageBox(theModelExport.hPanel, fmt("plane test failed: n=%i, i=%i, nml: %f %f %f dist: %f\npit: %f %f %f p0: %f %f %f p1: %f %f %f p2: %f %f %f\nEQ: %f\n",
							n,i,plane.normal[0],plane.normal[1],plane.normal[2],plane.dist,pointInTri[0],pointInTri[1],pointInTri[2],
							p0.x,p0.y,p0.z,
							p1.x,p1.y,p1.z,
							p2.x,p2.y,p2.z,
							M_DotProduct(pit, plane.normal) + plane.dist),"TestMeshConvexity()",MB_OK);*/
				return false;
			}
		}
	}

	return true;
}



TriObject* GetTriObject(Interface* ip, TimeValue time, INode* node, int &deleteIt)
{
	if (!node || !ip) return NULL;

	deleteIt = FALSE;
    Object* obj = node->EvalWorldState(time).obj;
	if (!obj) return NULL;

	if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0)))
	{
		TriObject* tri = (TriObject*)obj->ConvertToType(time, Class_ID(TRIOBJ_CLASS_ID, 0));		
		if (obj != tri) deleteIt = TRUE; 
		return tri;
	}
	return NULL;
}
/*
Mgc::ConvexHull3D *GetHull(Mesh &mesh)
{
	Mgc::Vector3 *mgcarray = new Mgc::Vector3[mesh.numVerts];

	for (int v = 0; v<mesh.numVerts; v++)
	{
		mgcarray[v] = Mgc::Vector3(mesh.verts[v].x, mesh.verts[v].y, mesh.verts[v].z);
	}

	Mgc::ConvexHull3D *hull = new Mgc::ConvexHull3D(mesh.numVerts, mgcarray);

	delete [] mgcarray;

	return hull;
}
*/