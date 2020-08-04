#include "cGameObject.h"
#include <iostream>
#include "cMesh.h"
#include "cPhysics.h"

extern glm::mat4 calculateWorldMatrix(cGameObject* pCurrentObject);
extern std::map<std::string, cMesh> mapNameToMesh;

cGameObject::cGameObject()
{
	this->positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);	
	this->m_qRotation = glm::quat(glm::vec3(0.0f, 0.0f, 0.0f));
	this->scale = 1.0f;
	this->diffuseColour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);	
	this->isWireframe = false;
	this->isVisible = true;	
	this->useTransparentMap = false;
	this->noLighting = false;
	this->wireframeColour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	this->textureBlendMode = NO_TEXTURE;
	this->velocity = glm::vec3(0.0f, 0.0f, 0.0f);
	this->acceleration = glm::vec3(0.0f, 0.0f, 0.0f);
	this->inverseMass = 0.0f;
	this->physicsShapeType = UNKNOWN;
	this->SPHERE_radius = 1.0f;
	this->m_uniqueID = cGameObject::next_uniqueID;
	cGameObject::next_uniqueID++;
	this->disableDepthBufferTest = false;
	this->disableDepthBufferWrite = false;
	this->textures[0] = "Half_grey.bmp";
	this->textures[1] = "default_normal.bmp";
	this->textures[2] = "White.bmp";		// Cutout alpha map, discard black pixel
	this->textures[3] = "White.bmp";		// Transparent map, white is opaque	
	this->textureTile[0] = 1.0f;
	this->textureTile[1] = 1.0f;
	this->textureTile[2] = 1.0f;
	this->textureTile[3] = 1.0f;	
	this->front = glm::vec3(0.0f, 0.0f, 1.0f);
	this->right = glm::vec3(-1.0f, 0.0f, 0.0f);
	this->up = glm::vec3(0.0f, 1.0f, 0.0f);
	this->pSM = NULL;
	this->currentAnimationName = "Run";
	this->animationFrameTime = 0.0f;
	this->animationSpeed = 0.01f;
	this->loopAnimationState = true;
	this->isJumping = false;
	this->isRolling = false;
	this->isMovingUp = false;
	this->isMovingDown = false;
	this->currentLane = 0;
	this->dead = false;

	InitializeCriticalSection(&gameObject_lock);
	//std::cout << "A cGameObject() is created." << std::endl;
	return;
}

cGameObject::~cGameObject()
{
	DeleteCriticalSection(&gameObject_lock);
	std::cout << "A cGameObject() is destroyed." << std::endl;
	return;
}

unsigned int cGameObject::next_uniqueID = 1000;
unsigned int cGameObject::getUniqueID(void)
{
	return this->m_uniqueID;
}

glm::quat cGameObject::getQOrientation(void)
{
	EnterCriticalSection(&gameObject_lock);
	const auto temp = this->m_qRotation;
	LeaveCriticalSection(&gameObject_lock);
	return temp;
}

// Overwrite the orientation
void cGameObject::setOrientation(glm::vec3 EulerAngleDegreesXYZ)
{
	// c'tor of the glm quaternion converts Euler 
	//	to quaternion representation. 
	EnterCriticalSection(&gameObject_lock);
	glm::vec3 EulerAngleRadians;
	EulerAngleRadians.x = glm::radians(EulerAngleDegreesXYZ.x);
	EulerAngleRadians.y = glm::radians(EulerAngleDegreesXYZ.y);
	EulerAngleRadians.z = glm::radians(EulerAngleDegreesXYZ.z);
	this->m_qRotation = glm::quat(EulerAngleRadians);
	LeaveCriticalSection(&gameObject_lock);
}

void cGameObject::setOrientation(glm::quat qAngle)
{
	EnterCriticalSection(&gameObject_lock);
	this->m_qRotation = qAngle;
	LeaveCriticalSection(&gameObject_lock);
}

// Updates the existing angle
void cGameObject::updateOrientation(glm::vec3 EulerAngleDegreesXYZ)
{
	// Create a quaternion of this angle...
	EnterCriticalSection(&gameObject_lock);
	glm::vec3 EulerAngleRadians;
	EulerAngleRadians.x = glm::radians(EulerAngleDegreesXYZ.x);
	EulerAngleRadians.y = glm::radians(EulerAngleDegreesXYZ.y);
	EulerAngleRadians.z = glm::radians(EulerAngleDegreesXYZ.z);

	glm::quat angleChange = glm::quat(EulerAngleRadians);
	// ...apply it to the exiting rotation
	this->m_qRotation *= angleChange;		// This multiply order will rotate according to local axis, not world axis	
	LeaveCriticalSection(&gameObject_lock);
}

void cGameObject::updateOrientation(glm::quat qAngleChange)
{
	EnterCriticalSection(&gameObject_lock);
	this->m_qRotation = qAngleChange * m_qRotation;
	LeaveCriticalSection(&gameObject_lock);
}

glm::vec3 cGameObject::getEulerAngle(void)
{
	EnterCriticalSection(&gameObject_lock);
	// In glm::gtx (a bunch of helpful things there)
	glm::vec3 EulerAngle = glm::eulerAngles(this->m_qRotation);
	return EulerAngle;
	LeaveCriticalSection(&gameObject_lock);
}

void cGameObject::CalculateAABBMinMax()
{
	cMesh transformedGameobjectMesh;
	cPhysics::CalculateTransformedMesh(mapNameToMesh[this->meshName], this->worldMatrix, transformedGameobjectMesh);
	
	// See if we have any vertices loaded...
	if (transformedGameobjectMesh.vecVertices.size() == 0)
	{
		return;
	}
	
	float minX, minY, minZ, maxX, maxY, maxZ;

	// Assume the 1st vertex is the max and min (so we can compare)
	minX = maxX = transformedGameobjectMesh.vecVertices[0].x;
	minY = maxY = transformedGameobjectMesh.vecVertices[0].y;
	minZ = maxZ = transformedGameobjectMesh.vecVertices[0].z;
	
	for (unsigned int index = 0; index != transformedGameobjectMesh.vecVertices.size(); index++)
	{
		if (transformedGameobjectMesh.vecVertices[index].x < minX) { minX = transformedGameobjectMesh.vecVertices[index].x; }
		if (transformedGameobjectMesh.vecVertices[index].y < minY) { minY = transformedGameobjectMesh.vecVertices[index].y; }
		if (transformedGameobjectMesh.vecVertices[index].z < minZ) { minZ = transformedGameobjectMesh.vecVertices[index].z; }

		if (transformedGameobjectMesh.vecVertices[index].x > maxX) { maxX = transformedGameobjectMesh.vecVertices[index].x; }
		if (transformedGameobjectMesh.vecVertices[index].y > maxY) { maxY = transformedGameobjectMesh.vecVertices[index].y; }
		if (transformedGameobjectMesh.vecVertices[index].z > maxZ) { maxZ = transformedGameobjectMesh.vecVertices[index].z; }
	}

	//this->extentX = this->maxX - this->minX;
	//this->extentY = this->maxY - this->minY;
	//this->extentZ = this->maxZ - this->minZ;

	//this->maxExtent = this->extentX;
	//if (this->extentY > this->maxExtent) { this->maxExtent = this->extentY; }
	//if (this->extentZ > this->maxExtent) { this->maxExtent = this->extentZ; }

	this->AABB_min = glm::vec3(minX, minY, minZ);
	this->AABB_max = glm::vec3(maxX, maxY, maxZ);

	return;
}

void cGameObject::UpdateWorldMatrix()
{
	EnterCriticalSection(&gameObject_lock);
	this->worldMatrix = calculateWorldMatrix(this);
	LeaveCriticalSection(&gameObject_lock);
	return;
}

void cGameObject::UpdateCollidePoints()
{
	UpdateWorldMatrix();

	this->vecUpdatedCollidePoints.clear();

	for (std::vector<glm::vec3>::iterator itCP = this->vecCollidePoints.begin();
		itCP != this->vecCollidePoints.end(); itCP++)
	{		
		this->vecUpdatedCollidePoints.push_back(glm::vec3(this->worldMatrix * glm::vec4(*itCP, 1.0f)));
	}

	return;
}

glm::vec3 cGameObject::getUpdatedFront()
{
	glm::mat4 matRotation = glm::toMat4(this->getQOrientation());

	glm::vec3 updatedFront = glm::normalize(glm::vec3(matRotation * glm::vec4(this->front, 1.0f)));

	return updatedFront;
}

glm::vec3 cGameObject::getUpdatedRight()
{
	glm::mat4 matRotation = glm::toMat4(this->getQOrientation());

	glm::vec3 updatedRight = glm::normalize(glm::vec3(matRotation * glm::vec4(this->right, 1.0f)));

	return updatedRight;
}

glm::vec3 cGameObject::getUpdatedUp()
{
	glm::mat4 matRotation = glm::toMat4(this->getQOrientation());

	glm::vec3 updatedUp = glm::normalize(glm::vec3(matRotation * glm::vec4(this->up, 1.0f)));

	return updatedUp;
}

glm::vec3 cGameObject::getPosition()
{
	EnterCriticalSection(&gameObject_lock);
	const auto temp = this->positionXYZ;
	LeaveCriticalSection(&gameObject_lock);
	return temp;
}

void cGameObject::setPosition(const glm::vec3 newPosition)
{
	EnterCriticalSection(&gameObject_lock);
	this->positionXYZ = newPosition;
	LeaveCriticalSection(&gameObject_lock);
}

float cGameObject::getScale()
{
	EnterCriticalSection(&gameObject_lock);
	const auto temp = this->scale;
	LeaveCriticalSection(&gameObject_lock);
	return temp;
}

void cGameObject::setScale(const float newScale)
{
	EnterCriticalSection(&gameObject_lock);
	this->scale = newScale;
	LeaveCriticalSection(&gameObject_lock);
}

glm::vec4 cGameObject::getDiffuseColour()
{
	EnterCriticalSection(&gameObject_lock);
	const auto temp = this->diffuseColour;
	LeaveCriticalSection(&gameObject_lock);
	return temp;
}

void cGameObject::setDiffuseColour(const glm::vec4 newColour)
{
	EnterCriticalSection(&gameObject_lock);
	this->diffuseColour = newColour;
	LeaveCriticalSection(&gameObject_lock);
}

float cGameObject::getAnimationFrameTime()
{
	EnterCriticalSection(&gameObject_lock);
	const auto temp = this->animationFrameTime;
	LeaveCriticalSection(&gameObject_lock);
	return temp;
}

void cGameObject::setAnimationFrameTime(const float newTime)
{
	EnterCriticalSection(&gameObject_lock);
	this->animationFrameTime = newTime;
	LeaveCriticalSection(&gameObject_lock);
}

std::string cGameObject::getCurrentAnimationName()
{
	EnterCriticalSection(&gameObject_lock);
	const auto temp = this->currentAnimationName;
	LeaveCriticalSection(&gameObject_lock);
	return temp;
}

void cGameObject::setCurrentAnimationName(const std::string newName)
{
	EnterCriticalSection(&gameObject_lock);
	this->currentAnimationName = newName;
	LeaveCriticalSection(&gameObject_lock);
}
