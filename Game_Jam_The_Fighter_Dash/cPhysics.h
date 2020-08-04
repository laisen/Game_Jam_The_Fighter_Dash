#ifndef _cPhysics_HG_
#define _cPhysics_HG_

#include "cGameObject.h"
#include "cMesh.h"
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <vector>
#include <map>
#include "cAABB.h"
#include "DebugRenderer/iDebugRenderer.h"

class cPhysics
{
public:
	cPhysics();
	
	typedef glm::vec3 Point;
	typedef glm::vec3 Vector;

	static std::map<unsigned long long /*ID*/, cAABB*> mapAABBs;
	static const float AABBsize;

	struct Sphere
	{
		glm::vec3 c;
		float r;
	};

	struct sPhysicsTriangle
	{
		glm::vec3 verts[3];
		glm::vec3 normal;
	};

	struct sCollisionInfo
	{
		glm::vec3 closestPoint;
		glm::vec3 directionOfApproach;
		float penetrationDistance;
		glm::vec3 adjustmentVector;
		unsigned int Object1_ID;
		unsigned int Object2_ID;
	};

	// Forward Explicit Euler Inetegration
	void IntegrationStep(std::vector<cGameObject*>& vec_pGameObjects, float deltaTime);

	void CollisionTest(std::vector<cGameObject*>& vec_pGameObjects);

	// Returns all the triangles and the closest points
	void GetClosestTriangleToPoint(Point pointXYZ, cMesh& mesh, glm::vec3& closestPoint, sPhysicsTriangle& closestTriangle);	
	void GetAABBClosestTriangleToPoint(Point pointXYZ, cAABB* pAABB, glm::vec3& closestPoint, sPhysicsTriangle& closestTriangle);

	// Ericson's book:
	Point ClosestPtPointTriangle(Point p, Point a, Point b, Point c);
	int TestSphereTriangle(Sphere s, Point a, Point b, Point c, Point& p);

	void setGravity(glm::vec3 newGravityValue);
	glm::vec3 getGravity(void);

	// mesh in gameObject position
	static void CalculateTransformedMesh(cMesh& originalMesh, glm::mat4 worldMatrix, cMesh& transformedMesh);

	static void CalculateGameObjectAABBs(cGameObject* gameObject);

	static void sliceTriangleCheck(std::vector<glm::vec3>& triangleVertices);

	void setDebugRenderer(iDebugRenderer* pDebugRenderer);

private:

	// Returns true if collision, store collisionInfo
	bool DoSphereSphereCollisionTest(cGameObject* pA, cGameObject* pB, sCollisionInfo& collisionInfo);
	bool DoSphereMeshCollisionTest(cGameObject* pA, cGameObject* pB, sCollisionInfo& collisionInfo);
	void DoAABBCollisionTest(cGameObject* pGameobject, glm::vec3 testPoint, cAABB* pAABB);
	glm::vec3  m_Gravity;
	iDebugRenderer* m_pDebugRenderer;
};

#endif
