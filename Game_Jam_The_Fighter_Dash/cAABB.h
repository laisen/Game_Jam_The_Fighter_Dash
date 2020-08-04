#pragma once
#include "cMesh.h"
#include <glm/glm.hpp>
#include "cGameObject.h"

class cAABB
{
public:
	cAABB();

	glm::vec3 minXYZ;
	float length;
	cMesh* pTheMesh;
	std::vector< sMeshTriangle* > vecTriangles;
	cGameObject* gameObject;

	glm::vec3 getCentre();
	glm::vec3 getMaxXYZ();

	static unsigned long long checkAABB_ID(glm::vec3 checkLocation);	

	unsigned long long getID();

	bool checkVertexInside(glm::vec3 VertexXYZ);

	void createBoxGameObject();
};

