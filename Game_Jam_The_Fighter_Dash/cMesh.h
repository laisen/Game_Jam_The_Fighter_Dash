#ifndef _cMesh_HG_
#define _cMesh_HG_

#include <vector>
#include "sVertex_xyz_rgba_n_uv2_bt_4Bones.h"

struct sMeshTriangle
{
	unsigned int vert_index_1, vert_index_2, vert_index_3;
};

class cMesh
{
public:
	cMesh() {};
	~cMesh() {};	
	std::vector<sVertex_xyz_rgba_n_uv2_bt_4Bones> vecVertices;	
	std::vector<sMeshTriangle> vecTriangles;
};

#endif 