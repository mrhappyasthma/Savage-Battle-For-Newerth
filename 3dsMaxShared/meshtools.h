// (C) 2002 S2 Games

// meshtools.h

// useful mesh functions for max plugins

#ifndef MESHTOOLS_H
#define MESHTOOLS_H

#include "Max.h"
#include "meshdlib.h"


#define FRAME(x) (GetTicksPerFrame() * x)


TriObject*	GetTriObject(Interface* ip, TimeValue time, INode* node, int &deleteIt);
void		BaryToXYZ(float u, float v, Point3& p0, Point3& p1, Point3& p2, Point3& out);
bool		TestMeshConvexity(Mesh& mesh);

#endif //MESHTOOLS_H