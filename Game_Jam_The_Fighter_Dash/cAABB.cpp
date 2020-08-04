#include "cAABB.h"
#include "cPhysics.h"
extern std::vector<cGameObject*> g_vec_pDebugGameObjects;

cAABB::cAABB()
{

	return;
};

unsigned long long cAABB::getID()
{
	unsigned long long ID = cAABB::checkAABB_ID(this->minXYZ);

	return ID;

}

bool cAABB::checkVertexInside(glm::vec3 VertexXYZ)
{
	if (VertexXYZ.x < minXYZ.x)	return false;
	if (VertexXYZ.x > (minXYZ.x + length)) return false;
	if (VertexXYZ.y < minXYZ.y)	return false;
	if (VertexXYZ.y > (minXYZ.y + length)) return false;
	if (VertexXYZ.z < minXYZ.z)	return false;
	if (VertexXYZ.z > (minXYZ.z + length)) return false;

	return true;
}

glm::vec3 cAABB::getCentre()
{
	float centreX = minXYZ.x + this->length / 2.0f;
	float centreY = minXYZ.y + this->length / 2.0f;
	float centreZ = minXYZ.z + this->length / 2.0f;

	glm::vec3 centre = glm::vec3(centreX, centreY, centreZ);

	return centre;
}

glm::vec3 cAABB::getMaxXYZ()
{
	float maxX = minXYZ.x + this->length;
	float maxY = minXYZ.y + this->length;
	float maxZ = minXYZ.z + this->length;

	glm::vec3 maxXYZ = glm::vec3(maxX, maxY, maxZ);

	return maxXYZ;
}


unsigned long long cAABB::checkAABB_ID(glm::vec3 checkLocation)
{
	unsigned long long ID, ulX = 0, ulY = 0, ulZ = 0;	
	
	// Are these negative
	if (checkLocation.x < 0.0f)	
		ulX += 100000;	
	if (checkLocation.y < 0.0f)	
		ulY += 100000;	
	if (checkLocation.z < 0.0f)	
		ulZ += 100000;	
	
	ulX += checkLocation.x < 0.0f ?
		(((unsigned long long) fabs(glm::floor(checkLocation.x / cPhysics::AABBsize)))) * cPhysics::AABBsize
		: (((unsigned long long) (checkLocation.x / cPhysics::AABBsize))) * cPhysics::AABBsize;
	ulY += checkLocation.y < 0.0f ?
		(((unsigned long long) fabs(glm::floor(checkLocation.y / cPhysics::AABBsize)))) * cPhysics::AABBsize
		: (((unsigned long long) (checkLocation.y / cPhysics::AABBsize))) * cPhysics::AABBsize;
	ulZ += checkLocation.z < 0.0f ?
		(((unsigned long long) fabs(glm::floor(checkLocation.z / cPhysics::AABBsize)))) * cPhysics::AABBsize
		: (((unsigned long long) (checkLocation.z / cPhysics::AABBsize))) * cPhysics::AABBsize;

	unsigned long long oneMillion = 1000000;
	ulX *= oneMillion * oneMillion;	
	ulY *= oneMillion;
	ulZ *= 1;

	ID = ulX + ulY + ulZ;
	
	return ID;
}


void cAABB::createBoxGameObject()
{
	this->gameObject = new cGameObject();
	this->gameObject->meshName = "cubeMesh";
	//this->gameObject->positionXYZ = this->getCentre();
	this->gameObject->setPosition(this->getCentre());
	//this->gameObject->scale = this->length / 2.0f;
	this->gameObject->setScale(this->length / 2.0f);
	this->gameObject->isWireframe = true;
	this->gameObject->wireframeColour = glm::vec4(0.3f, 0.9f, 0.3f, 1.0f);
	this->gameObject->friendlyName = "debugObject";
	g_vec_pDebugGameObjects.push_back(this->gameObject);
}