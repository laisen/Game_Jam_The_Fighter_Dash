#ifndef _cGameObject_HG_
#define _cGameObject_HG_

#include <string>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>
#include <windows.h>

enum eShapeTypes
{
	AABB,
	SPHERE,
	CAPSULE,
	PLANE,
	MESH,
	UNKNOWN
};

enum eTextureBlend
{
	NO_TEXTURE,		// Plain diffuse colour only, no blend
	BASE/*Albedo + Normal*/,		// Show the 1st textures[0] as diffuse + 2nd textures[1] as normal map
	DIFFUSE_COLOUR,		// Use diffuse colour to colour blend base texture textures[0]
	COLOUR,		// Use 2nd textures[1] to colour blend base texture textures[0]
	MULTIPLY,		// Use 2nd textures[1] to multiply blend base texture textures[0]
	AVERAGE,		// Use half 2nd textures[1] to add blend half base texture textures[0]
	SOFT_LIGHT,		// Use 2nd textures[1] to soft light blend base texture textures[0]
	IMPOSTER,		// Base texture with black colour discarded
	CUBE_MAP,		// Skybox
	CHROME		// Cubemap reflection/refrcation
};

class cSimpleAssimpSkinnedMesh;

class cGameObject
{
public:
	cGameObject();
	~cGameObject();
	
	std::string meshName;
	std::string friendlyName;	
	glm::mat4 worldMatrix;		
	bool isWireframe;
	bool isVisible;	
	bool useTransparentMap;
	bool noLighting;
	glm::vec4 wireframeColour;
	eTextureBlend textureBlendMode;
	glm::vec3 velocity;
	glm::vec3 acceleration;
	float inverseMass;	// 0.0f = Doesn't move	
	eShapeTypes physicsShapeType;
	glm::vec3 AABB_min, AABB_max;
	float SPHERE_radius;
	std::vector<glm::vec3> vecCollidePoints;
	std::vector<glm::vec3> vecUpdatedCollidePoints;	
	glm::vec3 front;
	glm::vec3 right;
	glm::vec3 up;	
	cSimpleAssimpSkinnedMesh* pSM;
	
	float animationSpeed;		// time frame offset
	bool loopAnimationState;	// true for idle, walk, run	
	bool isJumping;
	bool isRolling;
	bool isMovingUp;
	bool isMovingDown;
	int currentLane;
	bool dead;
	

	bool disableDepthBufferTest;
	bool disableDepthBufferWrite;

	static const int NUMBEROFTEXTURES = 4;
	std::string textures[NUMBEROFTEXTURES];	
	float textureTile[NUMBEROFTEXTURES];

	glm::vec3 getPosition();
	void setPosition(const glm::vec3 newPosition);
	float getScale();
	void setScale(const float newScale);
	glm::vec4 getDiffuseColour();
	void setDiffuseColour(const glm::vec4 newColour);
	float getAnimationFrameTime();
	void setAnimationFrameTime(const float newTime);
	std::string getCurrentAnimationName();
	void setCurrentAnimationName(const std::string newName);
	glm::quat getQOrientation();
	// Overwrite the orientation
	void setOrientation(glm::vec3 EulerAngleDegreesXYZ);
	void setOrientation(glm::quat qAngle);
	// Updates the existing angle
	void updateOrientation(glm::vec3 EulerAngleDegreesXYZ);
	void updateOrientation(glm::quat qAngle);
	glm::vec3 getEulerAngle();
	void CalculateAABBMinMax();
	void UpdateWorldMatrix();
	void UpdateCollidePoints();
	glm::vec3 getUpdatedFront();
	glm::vec3 getUpdatedRight();
	glm::vec3 getUpdatedUp();
	unsigned int getUniqueID();

private:
	glm::quat m_qRotation;
	glm::vec3 positionXYZ;
	float scale;
	glm::vec4 diffuseColour;
	float animationFrameTime;
	std::string currentAnimationName;

	CRITICAL_SECTION gameObject_lock;

	static unsigned int next_uniqueID;
	unsigned int m_uniqueID;
};

#endif