#include "cPhysics.h"
#include <iostream>
#include <map>
#include "GLCommon.h"
#include "cVAOManager.h"
#include "cParticleEmitter.h"

#define worldUp glm::vec3(0.0f, 1.0f, 0.0f)
#define worldRight glm::vec3(1.0f, 0.0f, 0.0f)
#define worldFront glm::vec3(0.0f, 0.0f, -1.0f)

extern std::map<std::string, cMesh> mapNameToMesh;
extern glm::mat4 calculateWorldMatrix(cGameObject* pCurrentObject);
extern std::vector<cGameObject*> g_vec_pGameObjects;
extern double averageDeltaTime;
std::map<unsigned long long /*ID*/, cAABB*> cPhysics::mapAABBs;
std::vector<glm::vec3> vec_sliceTempVertices;
const float cPhysics::AABBsize = 50.0f;
int count = 0;
extern cMesh transformedMesh;
//extern glm::vec3 movePosition;
extern cGameObject* pFindObjectByFriendlyName(std::string name);
//extern cGameObject* pDebugSphere;
//extern cGameObject* pCube;
extern GLuint mainShaderProgID;
extern cVAOManager* pVAOManager1;
//extern cParticleEmitter* collisionParticles;
void DrawObject(glm::mat4 worldMatrix, cGameObject* pCurrentObject, GLint mainShaderProgID, cVAOManager* pVAOManager);
//extern bool UP_DOWN_PRESS;
//extern bool LEFT_RIGHT_PRESS;
//extern bool Z_X_PRESS;
//extern bool dragOn;
//extern bool droneMode;
extern float lastTime;
extern float now;
extern float camerShakeAmplitude;
extern bool camerShakeCooldown;
extern float lerp(float a, float b, float f);
const float smooth = 3.0f;
const float SPEED_LIMIT = 60.0f;

cPhysics::cPhysics()
{
	std::cout << "A cPhysics() is created." << std::endl;
	this->m_Gravity = glm::vec3(0.0f, -60.0f, 0.0f);
	return;
}

void cPhysics::setGravity(glm::vec3 newGravityValue)
{
	this->m_Gravity = newGravityValue;
	return;
}

glm::vec3 cPhysics::getGravity(void)
{
	return this->m_Gravity;
}

void cPhysics::IntegrationStep(std::vector<cGameObject*>& vec_pGameObjects, float deltaTime)
{
	for (unsigned int index = 0; index != vec_pGameObjects.size(); index++)
	{
		// pointer to the current object
		cGameObject* pCurObj = vec_pGameObjects[index];

		if (pCurObj->isJumping)
		{
			pCurObj->acceleration = this->m_Gravity;

			pCurObj->velocity += pCurObj->acceleration * deltaTime;			

			//pCurObj->positionXYZ += pCurObj->velocity * deltaTime;
			glm::vec3 pos = pCurObj->getPosition();
			pos += pCurObj->velocity * deltaTime;
			pCurObj->setPosition(pos);
		}
		else
		{
			pCurObj->velocity = glm::vec3(0.0f, 0.0f, 0.0f);
		}

		if (pCurObj->inverseMass != 0.0f)
		{
			pCurObj->acceleration = this->m_Gravity;			
			
			pCurObj->velocity += pCurObj->acceleration * deltaTime;
			
			glm::vec3 pos = pCurObj->getPosition();
			pos += pCurObj->velocity * deltaTime;
			pCurObj->setPosition(pos);
		}
	}
	return;
}

// Returns all the triangles and the closest points
void cPhysics::GetClosestTriangleToPoint(Point pointXYZ, cMesh& mesh, glm::vec3& closestPoint, sPhysicsTriangle& closestTriangle)
{
	float closestDistanceSoFar = FLT_MAX;

	for (unsigned int triIndex = 0;	triIndex != mesh.vecTriangles.size(); triIndex++)
	{
		sMeshTriangle& curTriangle = mesh.vecTriangles[triIndex];

		// Get the vertices of the triangle
		sVertex_xyz_rgba_n_uv2_bt_4Bones triVert1 = mesh.vecVertices[curTriangle.vert_index_1];
		sVertex_xyz_rgba_n_uv2_bt_4Bones triVert2 = mesh.vecVertices[curTriangle.vert_index_2];
		sVertex_xyz_rgba_n_uv2_bt_4Bones triVert3 = mesh.vecVertices[curTriangle.vert_index_3];

		Point triVertPoint1;
		triVertPoint1.x = triVert1.x;
		triVertPoint1.y = triVert1.y;
		triVertPoint1.z = triVert1.z;

		Point triVertPoint2;
		triVertPoint2.x = triVert2.x;
		triVertPoint2.y = triVert2.y;
		triVertPoint2.z = triVert2.z;

		Point triVertPoint3;
		triVertPoint3.x = triVert3.x;
		triVertPoint3.y = triVert3.y;
		triVertPoint3.z = triVert3.z;

		glm::vec3 curClosetPoint = ClosestPtPointTriangle(pointXYZ,
			triVertPoint1, triVertPoint2, triVertPoint3);

		// get current distance
		float distanceNow = glm::distance(curClosetPoint, pointXYZ);

		// closer than the closest distance
		if (distanceNow <= closestDistanceSoFar)
		{
			closestDistanceSoFar = distanceNow;

			closestPoint = curClosetPoint;

			// Copy the triangle
			closestTriangle.verts[0].x = triVert1.x;
			closestTriangle.verts[0].y = triVert1.y;
			closestTriangle.verts[0].z = triVert1.z;
			closestTriangle.verts[1].x = triVert2.x;
			closestTriangle.verts[1].y = triVert2.y;
			closestTriangle.verts[1].z = triVert2.z;
			closestTriangle.verts[2].x = triVert3.x;
			closestTriangle.verts[2].y = triVert3.y;
			closestTriangle.verts[2].z = triVert3.z;
			
			// Average the normal of all 3 vertices
			glm::vec3 triVert1Norm = glm::vec3(triVert1.nx, triVert1.ny, triVert1.nz);
			glm::vec3 triVert2Norm = glm::vec3(triVert2.nx, triVert2.ny, triVert2.nz);
			glm::vec3 triVert3Norm = glm::vec3(triVert3.nx, triVert3.ny, triVert3.nz);			
			closestTriangle.normal = glm::normalize((triVert1Norm + triVert2Norm + triVert3Norm) / 3.0f);
		}
	}
	return;
}

void cPhysics::GetAABBClosestTriangleToPoint(Point pointXYZ, cAABB* pAABB, glm::vec3& closestPoint, sPhysicsTriangle& closestTriangle)
{
	float closestDistanceSoFar = FLT_MAX;
	
	for (unsigned int triIndex = 0; triIndex != pAABB->vecTriangles.size(); triIndex++)
	{
		sMeshTriangle* curTriangle = pAABB->vecTriangles[triIndex];

		// Get the vertices of the triangle
		sVertex_xyz_rgba_n_uv2_bt_4Bones triVert1 = pAABB->pTheMesh->vecVertices[curTriangle->vert_index_1];
		sVertex_xyz_rgba_n_uv2_bt_4Bones triVert2 = pAABB->pTheMesh->vecVertices[curTriangle->vert_index_2];
		sVertex_xyz_rgba_n_uv2_bt_4Bones triVert3 = pAABB->pTheMesh->vecVertices[curTriangle->vert_index_3];

		Point triVertPoint1;
		triVertPoint1.x = triVert1.x;
		triVertPoint1.y = triVert1.y;
		triVertPoint1.z = triVert1.z;

		Point triVertPoint2;
		triVertPoint2.x = triVert2.x;
		triVertPoint2.y = triVert2.y;
		triVertPoint2.z = triVert2.z;

		Point triVertPoint3;
		triVertPoint3.x = triVert3.x;
		triVertPoint3.y = triVert3.y;
		triVertPoint3.z = triVert3.z;

		glm::vec3 curClosetPoint = ClosestPtPointTriangle(pointXYZ,
			triVertPoint1, triVertPoint2, triVertPoint3);

		// get current distance
		float distanceNow = glm::distance(curClosetPoint, pointXYZ);

		// closer than the closest distance
		if (distanceNow <= closestDistanceSoFar)
		{
			closestDistanceSoFar = distanceNow;

			closestPoint = curClosetPoint;

			// Copy the triangle
			closestTriangle.verts[0].x = triVert1.x;
			closestTriangle.verts[0].y = triVert1.y;
			closestTriangle.verts[0].z = triVert1.z;
			closestTriangle.verts[1].x = triVert2.x;
			closestTriangle.verts[1].y = triVert2.y;
			closestTriangle.verts[1].z = triVert2.z;
			closestTriangle.verts[2].x = triVert3.x;
			closestTriangle.verts[2].y = triVert3.y;
			closestTriangle.verts[2].z = triVert3.z;

			// Average the normal of all 3 vertices
			glm::vec3 triVert1Norm = glm::vec3(triVert1.nx, triVert1.ny, triVert1.nz);
			glm::vec3 triVert2Norm = glm::vec3(triVert2.nx, triVert2.ny, triVert2.nz);
			glm::vec3 triVert3Norm = glm::vec3(triVert3.nx, triVert3.ny, triVert3.nz);
			closestTriangle.normal = glm::normalize((triVert1Norm + triVert2Norm + triVert3Norm) / 3.0f);
		}
	}
}


void cPhysics::CollisionTest(std::vector<cGameObject*>& vec_pGameObjects)
{
	for (size_t i = 0; i < vec_pGameObjects.size(); i++)
	{
		if (vec_pGameObjects[i]->getPosition().x < 2.5f)
		{			
			glm::vec3 pos = vec_pGameObjects[i]->getPosition();
			pos.x = 2.5f;
			vec_pGameObjects[i]->setPosition(pos);
		}
		if (vec_pGameObjects[i]->getPosition().x > 33.5f)
		{			
			glm::vec3 pos = vec_pGameObjects[i]->getPosition();
			pos.x = 33.5f;
			vec_pGameObjects[i]->setPosition(pos);
		}

		//if (vec_pGameObjects[i]->getPosition().y == 5.0f)
		//{
		//	if (vec_pGameObjects[i]->getPosition().x > 29.5f && vec_pGameObjects[i]->getPosition().x < 34.0f)
		//	{
		//		vec_pGameObjects[i]->isFalling = true;
		//		vec_pGameObjects[i]->loopAnimationState = false;				
		//		vec_pGameObjects[i]->setAnimationFrameTime(0.0f);
		//	}
		//	if (vec_pGameObjects[i]->getPosition().x > 18.5f && vec_pGameObjects[i]->getPosition().x < 24.5f)
		//	{
		//		vec_pGameObjects[i]->isFalling = true;
		//		vec_pGameObjects[i]->loopAnimationState = false;				
		//		vec_pGameObjects[i]->setAnimationFrameTime(0.0f);
		//	}
		//}

		//if (vec_pGameObjects[i]->getPosition().y == 0.0f)
		//{			
		//	if (vec_pGameObjects[i]->getPosition().x > 14.5f && vec_pGameObjects[i]->getPosition().x < 17.5f)
		//	{
		//		vec_pGameObjects[i]->isFalling = true;
		//		vec_pGameObjects[i]->loopAnimationState = false;				
		//		vec_pGameObjects[i]->setAnimationFrameTime(0.0f);
		//	}
		//}

		//if (vec_pGameObjects[i]->getPosition().y == 15.0f)
		//{
		//	if (vec_pGameObjects[i]->getPosition().x > 14.5f && vec_pGameObjects[i]->getPosition().x < 15.5f)
		//	{
		//		vec_pGameObjects[i]->isFalling = true;
		//		vec_pGameObjects[i]->loopAnimationState = false;				
		//		vec_pGameObjects[i]->setAnimationFrameTime(0.0f);
		//	}
		//	if (vec_pGameObjects[i]->getPosition().x > 18.5f && vec_pGameObjects[i]->getPosition().x < 19.5f)
		//	{
		//		vec_pGameObjects[i]->isFalling = true;
		//		vec_pGameObjects[i]->loopAnimationState = false;				
		//		vec_pGameObjects[i]->setAnimationFrameTime(0.0f);
		//	}
		//}

		//if (vec_pGameObjects[i]->getPosition().y == 10.0f)
		//{
		//	if (vec_pGameObjects[i]->getPosition().x > 2.0f && vec_pGameObjects[i]->getPosition().x < 5.5f)
		//	{
		//		vec_pGameObjects[i]->isFalling = true;
		//		vec_pGameObjects[i]->loopAnimationState = false;				
		//		vec_pGameObjects[i]->setAnimationFrameTime(0.0f);
		//	}
		//	if (vec_pGameObjects[i]->getPosition().x > 10.5f && vec_pGameObjects[i]->getPosition().x < 15.5f)
		//	{
		//		vec_pGameObjects[i]->isFalling = true;
		//		vec_pGameObjects[i]->loopAnimationState = false;				
		//		vec_pGameObjects[i]->setAnimationFrameTime(0.0f);
		//	}
		//	if (vec_pGameObjects[i]->getPosition().x > 18.5f && vec_pGameObjects[i]->getPosition().x < 19.5f)
		//	{
		//		vec_pGameObjects[i]->isFalling = true;
		//		vec_pGameObjects[i]->loopAnimationState = false;				
		//		vec_pGameObjects[i]->setAnimationFrameTime(0.0f);
		//	}
		//}
	}
}

bool cPhysics::DoSphereSphereCollisionTest(cGameObject* pA, cGameObject* pB, sCollisionInfo& collisionInfo)
{	
	//if (glm::distance(pA->positionXYZ, pB->positionXYZ) <= pA->SPHERE_radius + pB->SPHERE_radius)
	//{
	//	// change both spheres to red
	//	//pA->diffuseColour = glm::vec4(0.7f, 0.3f, 0.3f, 1.0f);
	//	//pB->diffuseColour = glm::vec4(0.7f, 0.3f, 0.3f, 1.0f);

	//	// get penetration distance
	//	float penetrationDistance = pA->SPHERE_radius + pB->SPHERE_radius - glm::length(pA->positionXYZ - pB->positionXYZ);

	//	// reverse penetration		
	//	glm::vec3 vecDirectionA = glm::normalize(pA->velocity);
	//	glm::vec3 vecPositionAdjustA = (-vecDirectionA) * penetrationDistance;
	//	pA->positionXYZ += vecPositionAdjustA;

	//	glm::vec3 vecDirectionB = glm::normalize(pB->velocity);
	//	glm::vec3 vecPositionAdjustB = (-vecDirectionB) * penetrationDistance;
	//	pB->positionXYZ += vecPositionAdjustB;

	//	// get bounce vector
	//	glm::vec3 bounceVectorA = glm::reflect(glm::normalize(pA->velocity), glm::normalize(pA->positionXYZ - pB->positionXYZ));
	//	bounceVectorA = glm::normalize(bounceVectorA);

	//	glm::vec3 bounceVectorB = glm::reflect(glm::normalize(pB->velocity), glm::normalize(pB->positionXYZ - pA->positionXYZ));
	//	bounceVectorB = glm::normalize(bounceVectorB);

	//	// bounce velocity
	//	pA->velocity = bounceVectorA * glm::length(pA->velocity);
	//	pB->velocity = bounceVectorB * glm::length(pB->velocity);

	//	collisionInfo.Object1_ID = pA->getUniqueID();
	//	collisionInfo.Object2_ID = pB->getUniqueID();
	//	collisionInfo.penetrationDistance = pA->SPHERE_radius + pB->SPHERE_radius - glm::length(pA->positionXYZ - pB->positionXYZ);
	//	return true;
	//}	

	return false;
}

bool cPhysics::DoSphereMeshCollisionTest(cGameObject* pA, cGameObject* pB,	sCollisionInfo& collisionInfo)
{

	//glm::vec3 closestPoint = glm::vec3(0.0f, 0.0f, 0.0f);
	//cPhysics::sPhysicsTriangle closestTriangle;

	//glm::mat4 worldMatrix = calculateWorldMatrix(pB);

	//cMesh transformedMesh;
	//CalculateTransformedMesh(mapNameToMesh[pB->meshName], worldMatrix, transformedMesh);
	//GetClosestTriangleToPoint(pA->positionXYZ, transformedMesh, closestPoint, closestTriangle);	

	//float distance = glm::distance(pA->positionXYZ, closestPoint);

	//// hit the triangle
	//if (distance <= pA->SPHERE_radius)
	//{
	//	
	//	// sphere position to closest point vector
	//	//glm::vec3 sphereToClosestPoint = closestPoint - pA->positionXYZ;

	//	// get penetration distance
	//	//float penetrationDistance = pA->SPHERE_radius - glm::length(sphereToClosestPoint);

	//	// reverse penetration		
	//	//glm::vec3 vecDirection = glm::normalize(pA->velocity);
	//	//glm::vec3 vecPositionAdjust = (-vecDirection) * penetrationDistance;
	//	//pA->positionXYZ += vecPositionAdjust;

	//	// get bounce vector
	//	//glm::vec3 bounceVector = glm::reflect(glm::normalize(pA->velocity), glm::normalize(closestTriangle.normal));
	//	//bounceVector = glm::normalize(bounceVector);
	//	
	//	// bounce velocity
	//	//pA->velocity = bounceVector * glm::length(pA->velocity);

	//	//collisionInfo.closestPoint = closestPoint;
	//	//collisionInfo.adjustmentVector = vecPositionAdjust;
	//	//collisionInfo.directionOfApproach = bounceVector;
	//	//collisionInfo.penetrationDistance = penetrationDistance;
	//	//collisionInfo.Object1_ID = pA->getUniqueID();
	//	//collisionInfo.Object2_ID = pB->getUniqueID();

	//	return true;
	//}

	return false;
}

void cPhysics::DoAABBCollisionTest(cGameObject* pGameobject, glm::vec3 testPoint, cAABB* pAABB)
{
	//glm::vec3 closestPoint = glm::vec3(0.0f, 0.0f, 0.0f);
	//cPhysics::sPhysicsTriangle closestTriangle;
	//
	//GetAABBClosestTriangleToPoint(testPoint, pAABB, closestPoint, closestTriangle);

	//this->m_pDebugRenderer->addLine(closestTriangle.verts[0], closestTriangle.verts[1], glm::vec3(0.9f, 0.9f, 0.3f), 0.0f);
	//this->m_pDebugRenderer->addLine(closestTriangle.verts[1], closestTriangle.verts[2], glm::vec3(0.9f, 0.9f, 0.3f), 0.0f);
	//this->m_pDebugRenderer->addLine(closestTriangle.verts[2], closestTriangle.verts[0], glm::vec3(0.9f, 0.9f, 0.3f), 0.0f);

	////pDebugSphere->positionXYZ = testPoint;

	//float distance = glm::distance(testPoint, closestPoint);

	//// hit the triangle
	//if (distance <= pGameobject->SPHERE_radius)
	//{
	//	//pGameobject->diffuseColour = glm::vec4(0.9f, 0.3f, 0.3f, 1.0f);
	//	//pDebugSphere->wireframeColour = glm::vec4(0.9f, 0.3f, 0.3f, 1.0f);

	//	//DrawObject(glm::mat4(1.0f), pDebugSphere, mainShaderProgID, pVAOManager1);

	//	//printf("Collide location [%f, %f, %f] inside AABB [%u]\n", closestPoint.x, closestPoint.y, closestPoint.z, (int)pAABB->getID());
	//	if (camerShakeCooldown)
	//	{
	//		camerShakeAmplitude = 1.5f * glm::length(pGameobject->velocity) / 100.0f;
	//		camerShakeCooldown = false;
	//	}

	//	// get penetration distance		
	//	float penetrationDistance = pGameobject->SPHERE_radius - glm::distance(testPoint, closestPoint);
	//	//penetrationDistance *= 10.0f;

	//	// reverse penetration		
	//	glm::vec3 vecDirection = glm::normalize(closestTriangle.normal);
	//	glm::vec3 vecPositionAdjust = vecDirection * penetrationDistance;		
	//	pGameobject->positionXYZ += vecPositionAdjust;

	//	// Get the cos included angle between velocity and reversed closestTriangle.normal
	//	float cosAngle = glm::dot( -glm::normalize(closestTriangle.normal), glm::normalize(pGameobject->velocity) );

	//	// Calculate the length of velocity in collide triangle normal direction
	//	float collideNormalLength = glm::length(pGameobject->velocity) * cosAngle;

	//	// The collide velocity in reversed collide triangle normal direction
	//	glm::vec3 collideVelocity = -glm::normalize(closestTriangle.normal) * collideNormalLength;

	//	// After compensating this collideVelocity to get the velocity paralleled with collide triangle
	//	pGameobject->velocity -= collideVelocity;

	//	// Zero acceleration
	//	pGameobject->acceleration = glm::vec3(0.0f);		

	//	//collisionParticles->location = closestPoint;
	//	//collisionParticles->acceleration = glm::normalize(closestTriangle.normal) * glm::length(pGameobject->velocity);
	//	//collisionParticles->enableNewParticles();
	//	lastTime = now;
	//}
	//else
	//{
	//	//pGameobject->diffuseColour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	//	//pDebugSphere->wireframeColour = glm::vec4(0.3f, 0.9f, 0.3f, 1.0f);
	//	//DrawObject(glm::mat4(1.0f), pDebugSphere, mainShaderProgID, pVAOManager1);
	//}
}

void cPhysics::CalculateTransformedMesh(cMesh& originalMesh, glm::mat4 worldMatrix, cMesh& transformedMesh)
{	
	transformedMesh = originalMesh;

	for (std::vector<sVertex_xyz_rgba_n_uv2_bt_4Bones>::iterator itVert = transformedMesh.vecVertices.begin();
		itVert != transformedMesh.vecVertices.end(); itVert++)
	{
		glm::vec4 vertex = glm::vec4(itVert->x, itVert->y, itVert->z, 1.0f);		
		glm::vec4 transformedVertex = worldMatrix * vertex;

		itVert->x = transformedVertex.x;
		itVert->y = transformedVertex.y;
		itVert->z = transformedVertex.z;

		// normal need inverse transpose world matrix
		glm::vec4 normal = glm::vec4(itVert->nx, itVert->ny, itVert->nz, 1.0f);
		glm::vec4 transformedNormal = glm::inverse(glm::transpose(worldMatrix)) * normal;

		itVert->nx = transformedNormal.x;
		itVert->ny = transformedNormal.y;
		itVert->nz = transformedNormal.z;
	}
	return;
}

void cPhysics::CalculateGameObjectAABBs(cGameObject* gameObject)
{
	float WorldMinX, WorldMinY, WorldMinZ, WorldMaxX, WorldMaxY, WorldMaxZ;
	WorldMinX = (gameObject->AABB_min.x != 0.0f) ?
		( (gameObject->AABB_min.x < 0.0f) ?
		(glm::floor(gameObject->AABB_min.x / AABBsize) * AABBsize) : (glm::ceil(gameObject->AABB_min.x / AABBsize) * AABBsize - AABBsize))
		: 0.0f;
	WorldMinY = (gameObject->AABB_min.y != 0.0f) ?
		((gameObject->AABB_min.y < 0.0f) ?
		(glm::floor(gameObject->AABB_min.y / AABBsize) * AABBsize) : (glm::ceil(gameObject->AABB_min.y / AABBsize) * AABBsize - AABBsize))
		: 0.0f;
	WorldMinZ = (gameObject->AABB_min.z != 0.0f) ?
		((gameObject->AABB_min.z < 0.0f) ?
		(glm::floor(gameObject->AABB_min.z / AABBsize) * AABBsize) : (glm::ceil(gameObject->AABB_min.z / AABBsize) * AABBsize - AABBsize))
		: 0.0f;
	WorldMaxX = (gameObject->AABB_max.x != 0.0f) ?
		((gameObject->AABB_max.x < 0.0f) ?
		(glm::floor(gameObject->AABB_max.x / AABBsize) * AABBsize + AABBsize) : (glm::ceil(gameObject->AABB_max.x / AABBsize) * AABBsize))
		: 0.0f;
	WorldMaxY = (gameObject->AABB_max.y != 0.0f) ?
		((gameObject->AABB_max.y < 0.0f) ?
		(glm::floor(gameObject->AABB_max.y / AABBsize) * AABBsize + AABBsize) : (glm::ceil(gameObject->AABB_max.y / AABBsize) * AABBsize))
		: 0.0f;
	WorldMaxZ = (gameObject->AABB_max.z != 0.0f) ?
		((gameObject->AABB_max.z < 0.0f) ?
		(glm::floor(gameObject->AABB_max.z / AABBsize) * AABBsize + AABBsize) : (glm::ceil(gameObject->AABB_max.z / AABBsize) * AABBsize))
		: 0.0f;	
	
	
	//cMesh transformedMesh;
	cPhysics::CalculateTransformedMesh(mapNameToMesh[gameObject->meshName], gameObject->worldMatrix, transformedMesh);
	for (float x = WorldMinX; x <= WorldMaxX; x += AABBsize)
	{
		for (float y = WorldMinY; y <= WorldMaxY; y += AABBsize)
		{
			for (float z = WorldMinZ; z <= WorldMaxZ; z += AABBsize)
			{
				cAABB* pAABB = new cAABB();
				pAABB->minXYZ.x = x;
				pAABB->minXYZ.y = y;
				pAABB->minXYZ.z = z;
				pAABB->length = AABBsize;				
				pAABB->pTheMesh = &transformedMesh;
				unsigned long long ID = pAABB->getID();
				mapAABBs[ID] = pAABB;
			}
		}
	}

	// Check which AABB this triangle is in
	
	for (std::vector<sMeshTriangle>::iterator itTri = transformedMesh.vecTriangles.begin();
		itTri != transformedMesh.vecTriangles.end(); itTri++)
	{
		glm::vec3 vertex1;
		vertex1.x = transformedMesh.vecVertices[itTri->vert_index_1].x;
		vertex1.y = transformedMesh.vecVertices[itTri->vert_index_1].y;
		vertex1.z = transformedMesh.vecVertices[itTri->vert_index_1].z;
		glm::vec3 vertex2;
		vertex2.x = transformedMesh.vecVertices[itTri->vert_index_2].x;
		vertex2.y = transformedMesh.vecVertices[itTri->vert_index_2].y;
		vertex2.z = transformedMesh.vecVertices[itTri->vert_index_2].z;
		glm::vec3 vertex3;
		vertex3.x = transformedMesh.vecVertices[itTri->vert_index_3].x;
		vertex3.y = transformedMesh.vecVertices[itTri->vert_index_3].y;
		vertex3.z = transformedMesh.vecVertices[itTri->vert_index_3].z;		
		
		vec_sliceTempVertices.push_back(vertex1);
		vec_sliceTempVertices.push_back(vertex2);
		vec_sliceTempVertices.push_back(vertex3);
		
		// Slice the triangle.
		sliceTriangleCheck(vec_sliceTempVertices);		

		for (std::map<unsigned long long, cAABB* >::iterator itAABB = cPhysics::mapAABBs.begin(); itAABB != cPhysics::mapAABBs.end(); itAABB++)
		{			
			std::vector<glm::vec3>::iterator itVert_check = vec_sliceTempVertices.begin();
			while (itVert_check != vec_sliceTempVertices.end())
			{
				if (itAABB->second->checkVertexInside(*itVert_check))
				{
					itAABB->second->vecTriangles.push_back(&(*itTri));					
					break;
				}
				++itVert_check;
			}
		}

		vec_sliceTempVertices.clear();
	}

	std::cout << "Sliced triangle count = " << count << std::endl;
	//std::cout << "Before erase mapAABBs size = " << mapAABBs.size() << std::endl;
	std::map<unsigned long long, cAABB* >::iterator itAABB_erase = cPhysics::mapAABBs.begin();
	while (itAABB_erase != mapAABBs.end())
	{
		if (itAABB_erase->second->vecTriangles.size() == 0)
		{
			itAABB_erase = mapAABBs.erase(itAABB_erase);
		}
		else
		{
			itAABB_erase->second->createBoxGameObject();
			++itAABB_erase;
		}
	}

	//std::cout << "After erase mapAABBs size = " << mapAABBs.size() << std::endl;

	return;
}

void cPhysics::sliceTriangleCheck(std::vector<glm::vec3>& triangleVertices)
{
	if (glm::distance(triangleVertices[0], triangleVertices[1]) >= AABBsize
		|| glm::distance(triangleVertices[0], triangleVertices[2]) >= AABBsize
		|| glm::distance(triangleVertices[1], triangleVertices[2]) >= AABBsize)
	{
		//std::cout << "Slice the triangle." << std::endl;
		count++;
		glm::vec3 sliceVertex1 = glm::vec3((triangleVertices[0].x + triangleVertices[1].x) / 2.0f,
			(triangleVertices[0].y + triangleVertices[1].y) / 2.0f,
			(triangleVertices[0].z + triangleVertices[1].z) / 2.0f);
		glm::vec3 sliceVertex2 = glm::vec3((triangleVertices[1].x + triangleVertices[2].x) / 2.0f,
			(triangleVertices[1].y + triangleVertices[2].y) / 2.0f,
			(triangleVertices[1].z + triangleVertices[2].z) / 2.0f);
		glm::vec3 sliceVertex3 = glm::vec3((triangleVertices[0].x + triangleVertices[2].x) / 2.0f,
			(triangleVertices[0].y + triangleVertices[2].y) / 2.0f,
			(triangleVertices[0].z + triangleVertices[2].z) / 2.0f);
		vec_sliceTempVertices.push_back(sliceVertex1);
		vec_sliceTempVertices.push_back(sliceVertex2);
		vec_sliceTempVertices.push_back(sliceVertex3);

		std::vector<glm::vec3> tempTriangle1;
		tempTriangle1.push_back(triangleVertices[0]);
		tempTriangle1.push_back(sliceVertex1);
		tempTriangle1.push_back(sliceVertex3);

		std::vector<glm::vec3> tempTriangle2;
		tempTriangle2.push_back(sliceVertex1);
		tempTriangle2.push_back(triangleVertices[1]);
		tempTriangle2.push_back(sliceVertex2);

		std::vector<glm::vec3> tempTriangle3;
		tempTriangle3.push_back(sliceVertex2);
		tempTriangle3.push_back(sliceVertex3);
		tempTriangle3.push_back(sliceVertex1);

		std::vector<glm::vec3> tempTriangle4;
		tempTriangle4.push_back(sliceVertex3);
		tempTriangle4.push_back(sliceVertex2);
		tempTriangle4.push_back(triangleVertices[2]);

		sliceTriangleCheck(tempTriangle1);
		sliceTriangleCheck(tempTriangle2);
		sliceTriangleCheck(tempTriangle3);
		sliceTriangleCheck(tempTriangle4);
	}
	return;
}

void cPhysics::setDebugRenderer(iDebugRenderer* pDebugRenderer)
{
	this->m_pDebugRenderer = pDebugRenderer;
	return;
}