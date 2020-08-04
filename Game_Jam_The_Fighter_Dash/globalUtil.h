#pragma once

#include "cGameObject.h"
#include <vector>
#include <map>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/constants.hpp>
#include <sstream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "cSimpleAssimpSkinnedMeshLoader_OneMesh.h"
#include "c3rdPersonCamera.h"
#include <time.h>		/* time */
#include <chrono>
#include "cLowPassFilter.h"

#define worldUp glm::vec3(0.0f, 1.0f, 0.0f)
#define worldRight glm::vec3(1.0f, 0.0f, 0.0f)
#define worldFront glm::vec3(0.0f, 0.0f, 1.0f)
#define playerRunningPositionX -4.0f
#define LaneWidth 4.0f
#define playerResetPosition  glm::vec3(-15.0f, 0.0f, 0.0f)
glm::vec3 playerResetPos = playerResetPosition;

std::map<std::string, cMesh> mapNameToMesh;
GLuint mainShaderProgID;
GLuint shadowDepthShaderProgID;
GLuint quadShaderProgID;
GLuint blurShaderProgID;
GLuint bloomShaderProgID;
cVAOManager* pVAOManager1 = new cVAOManager();
cVAOManager* pVAOManager2 = new cVAOManager();
cPhysics* pPhysics = new cPhysics();
cBasicTextureManager* g_pTextureManager = new cBasicTextureManager();
cParticleEmitter* dustParticles = new cParticleEmitter();
const unsigned int SHADOW_WIDTH = 512, SHADOW_HEIGHT = 512;
unsigned int depthMapFBO;
unsigned int hdrFBO;
unsigned int blurFBO[2];
unsigned int depthMap;
glm::vec3 lightPos(6.0f, 12.0f, 6.0f);
glm::mat4 lightSpaceMatrix;
unsigned int colorBuffers[2];		// create 2 floating point color buffers (1 for normal rendering, other for brightness treshold values)
unsigned int blurColorbuffers[2];		// Double blur pass
cObstacleGenerator* obstacleGenerator;
bool playerInPlace = false;
std::vector< glm::mat4 > vecInstanceMatrices;
float instanceAlpha[150];

bool firstMouse = true;
float lastX, lastY;
bool g_MouseLeftButtonIsDown = false;
double g_mouseX, g_mouseY, g_scrollY;
float g_FOV = 45.0f;
glm::quat g_cameraQuat;

cCamera camera;
c3rdPersonCamera Cam;
glm::vec3 followCameraTarget;
glm::mat4 view;
glm::vec3 cameraVelocity = glm::vec3(0.0f, 0.0f, 0.0f);
float camerShakeAmplitude = 0.0f;
bool camerShakeCooldown = true;

float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame
float lastTime = 0.0f;
float now = 0.0f;
double averageDeltaTime;

const float LightConstAtten = 0.0000001f;

glm::vec3 movePosition = glm::vec3(0.0f, 0.0f, 0.0f);

std::vector<cGameObject*> g_vec_pGameObjects;
std::map<std::string /*FriendlyName*/, cGameObject*> g_map_GameObjectsByFriendlyName;
std::vector<cGameObject*> g_vec_pDebugGameObjects;
std::vector<cGameObject*> g_vec_pTransparentGameObjects;
std::vector<cParticle*> vecParticles;
std::vector<cObstacle*> vecObstacles;
cGameObject* pFindObjectByFriendlyName(std::string name);

cMesh transformedMesh;
cGameObject* player1;
cGameObject* pQuadImposter;
cGameObject* obstacle;

std::vector<cGameObject*> controlTargets;
unsigned int controlIndex = 0;
std::vector< glm::mat4x4 > vecFinalTransformation;
std::vector< glm::mat4x4 > vecOffsets;
std::vector< glm::mat4x4 > vecObjectBoneTransformation;

bool debugMode = true;
bool wireframeMode = false;
bool normalPassMode = false;
bool albedoPassMode = false;
bool useFollowCamera = true;
bool running = true;

float SPEED_CHANGE = 0.1f;

unsigned int indoorCubemapTexture;
unsigned int outdoorCubemapTexture;
unsigned int loadCubemap(std::vector<std::string> faces);
unsigned int loadTexture(char const* path, bool gammaCorrection);

void processInput(GLFWwindow* window)
{
	glfwGetCursorPos(window, &g_mouseX, &g_mouseY);

	if (!useFollowCamera)
	{
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
			camera.ProcessKeyboard(FORWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
			camera.ProcessKeyboard(BACKWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
			camera.ProcessKeyboard(LEFT, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
			camera.ProcessKeyboard(RIGHT, deltaTime);
	}	

}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	if (!useFollowCamera)
	{
		camera.ProcessMouseMovement(xoffset, yoffset);
	}
	//camera.ProcessMouseMovement(xoffset, yoffset);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

cGameObject* pFindObjectByFriendlyName(std::string name)
{
	for (unsigned int index = 0; index != g_vec_pGameObjects.size(); index++)
	{
		if (::g_vec_pGameObjects[index]->friendlyName == name)
		{
			return ::g_vec_pGameObjects[index];
		}
	}

	for (unsigned int index = 0; index != g_vec_pTransparentGameObjects.size(); index++)
	{
		if (::g_vec_pTransparentGameObjects[index]->friendlyName == name)
		{
			return ::g_vec_pTransparentGameObjects[index];
		}
	}

	return NULL;
}

cGameObject* pFindObjectByFriendlyNameMap(std::string name)
{
	return ::g_map_GameObjectsByFriendlyName[name];
}


void Load_gameObjects()
{	
	std::ifstream GameObjectFile("config/GameObject_config.txt");
	if (!GameObjectFile.is_open())
	{
		std::cout << "Can't find the GameObject config file." << std::endl;
		return;
	}
	std::string line = "";
	int number_of_gameObjects = 0;
	while (std::getline(GameObjectFile, line))
		++number_of_gameObjects;

	std::cout << number_of_gameObjects - 1 << " GameObject(s) will be created by config." << std::endl;

	//::g_vec_pGameObjects.resize(number_of_gameObjects - 1);

	GameObjectFile.clear();
	GameObjectFile.seekg(0, GameObjectFile.beg);
	std::string discardLine;
	GameObjectFile >> discardLine;
	for (unsigned int index = 0; index != number_of_gameObjects - 1; index++)
	{
		std::string mesh_name, friendly_name, physics_shape_type, texture_0, texture_1, texture_blend_mode;
		float positionX, positionY, positionZ,
			rotationX, rotationY, rotationZ,
			scale,
			diffuseColourR, diffuseColourG, diffuseColourB, transparency;

		GameObjectFile >> mesh_name >> friendly_name
			>> positionX >> positionY >> positionZ
			>> rotationX >> rotationY >> rotationZ
			>> scale
			>> diffuseColourR >> diffuseColourG >> diffuseColourB >> transparency
			>> physics_shape_type
			>> texture_0 >> texture_1 >> texture_blend_mode;

		//::g_vec_pGameObjects[index] = new cGameObject();
		cGameObject* GO = new cGameObject();
		GO->meshName = mesh_name;
		GO->friendlyName = friendly_name;		
		GO->setPosition(glm::vec3(positionX, positionY, positionZ));
		GO->setOrientation(glm::vec3(rotationX, rotationY, rotationZ));
		GO->setScale(scale);
		GO->setDiffuseColour(glm::vec4(diffuseColourR, diffuseColourG, diffuseColourB, transparency));
		std::map<std::string, eShapeTypes> stringToPSTEnum = { {"AABB", AABB}, {"SPHERE", SPHERE}, {"CAPSULE", CAPSULE}, {"PLANE", PLANE}, {"MESH", MESH}, {"UNKNOWN", UNKNOWN} };
		GO->physicsShapeType = stringToPSTEnum[physics_shape_type];
		GO->textures[0] = texture_0;
		GO->textures[1] = texture_1;
		std::map<std::string, eTextureBlend> stringToTBMEnum = { {"NO_TEXTURE", NO_TEXTURE}, {"BASE", BASE}, {"DIFFUSE_COLOUR", DIFFUSE_COLOUR}, {"COLOUR", COLOUR}, {"MULTIPLY", MULTIPLY}, {"AVERAGE", AVERAGE}, {"SOFT_LIGHT", SOFT_LIGHT}, {"IMPOSTER", IMPOSTER}, {"CUBE_MAP", CUBE_MAP}, {"CHROME", CHROME} };
		GO->textureBlendMode = stringToTBMEnum[texture_blend_mode];
		GO->UpdateWorldMatrix();
		GO->CalculateAABBMinMax();
		g_vec_pGameObjects.push_back(GO);		
	}

	pFindObjectByFriendlyName("ground")->textureTile[0] = 3.0f;
	pFindObjectByFriendlyName("ground")->textureTile[1] = 3.0f;

	// Transfer transparent GameObjects
	//std::vector<cGameObject*>::iterator itTransfer = g_vec_pGameObjects.begin();
	//while (itTransfer != g_vec_pGameObjects.end())
	//{
	//	cGameObject* pCurGameObject = *itTransfer;	// Dereference iterator, giving us the original type	//	if (pCurGameObject->getDiffuseColour().a < 1.0f || pCurGameObject->useTransparentMap == true)
	//	{
	//		g_vec_pTransparentGameObjects.push_back(pCurGameObject);
	//		itTransfer = g_vec_pGameObjects.erase(itTransfer);
	//	}
	//	else
	//	{
	//		++itTransfer;
	//	}
	//}
	
	// Dust particle
	cParticleEmitter::sParticleCreationSettings dustParticleSet;
	dustParticleSet.minLifeSeconds = 0.8f;
	dustParticleSet.maxLifeInSeconds = 1.5f;
	dustParticleSet.particleCreationVolume = cParticleEmitter::sParticleCreationSettings::SPHERE;
	dustParticleSet.minStartingScale = 0.2f;
	dustParticleSet.maxStartingScale = 0.4f;
	dustParticleSet.minScaleChangePercentPerSecond = 0.8f;
	dustParticleSet.maxScaleChangePercentPerSecond = 1.8f;
	dustParticleSet.numParticlesPerSecond = 100.0f;
	dustParticleSet.isImposterFaceCamera = true;
	dustParticleSet.bFadeOutOverLifetime = true;
	dustParticleSet.minVelocity = glm::vec3(-8.0f, 0.3f, -2.0f);
	dustParticleSet.maxVelocity = glm::vec3(-4.0f, 2.5f, 2.0f);
	dustParticleSet.minRotationalChangePerSecond = glm::quat(glm::vec3(0.0f, 0.0f, -360.0f));
	dustParticleSet.maxRotationalChangePerSecond = glm::quat(glm::vec3(0.0f, 0.0f, 360.0f));
	dustParticles->Initialize(dustParticleSet);	
	dustParticles->enableNewParticles();

	// For dust particles
	pQuadImposter = new cGameObject();
	pQuadImposter->meshName = "quadMesh";
	pQuadImposter->friendlyName = "quadImposter";
	pQuadImposter->textures[0] = "dust.bmp";
	pQuadImposter->textureBlendMode = IMPOSTER;

	obstacle = new cGameObject();
	obstacle->friendlyName = "obstacle";
	obstacle->setDiffuseColour(glm::vec4(0.8f, 0.2f, 0.2f, 1.0f));
	obstacle->textureBlendMode = BASE;
	obstacle->textures[0] = "Obstacle_Albedo.bmp";
	obstacle->textures[1] = "Obstacle_Normal.bmp";

}

std::ifstream meshPath;
int number_of_meshs = 0;

void Open_meshPath()
{
	meshPath.open("config/mesh_path.txt");
	if (!meshPath.is_open())
	{
		std::cout << "Can't open the mesh path file." << std::endl;
		return;
	}

	std::string line;
	while (std::getline(meshPath, line))
		++number_of_meshs;

	std::cout << number_of_meshs << " meshes will be loaded by config." << std::endl;
	//g_vec_sModelDrawInfo.resize(number_of_meshs);
	meshPath.clear();
	meshPath.seekg(0, meshPath.beg);	
}

void Load_meshes()
{
	cModelLoader* pTheModelLoader = new cModelLoader();
	for (unsigned int index = 0; index != number_of_meshs; index++)
	{
		std::string mesh_name, mesh_path, errStr;
		meshPath >> mesh_name >> mesh_path;
		if (pTheModelLoader->LoadModel_Assimp(mesh_path.c_str(), mapNameToMesh[mesh_name], errStr))
		{
			sModelDrawInfo DI1;
			pVAOManager1->LoadModelIntoVAO(mesh_name, mapNameToMesh[mesh_name], DI1, mainShaderProgID);
			sModelDrawInfo DI2;
			pVAOManager2->LoadModelIntoVAO(mesh_name, mapNameToMesh[mesh_name], DI2, shadowDepthShaderProgID);
		}
	}
	delete pTheModelLoader;

	sModelDrawInfo* pDI1 = player1->pSM->CreateModelDrawInfoObjectFromCurrentModel();
	if (pDI1)
	{
		pVAOManager1->LoadModelDrawInfoIntoVAO(*pDI1, mainShaderProgID);		
	}

	sModelDrawInfo* pDI2 = player1->pSM->CreateModelDrawInfoObjectFromCurrentModel();
	if (pDI2)
	{		
		pVAOManager2->LoadModelDrawInfoIntoVAO(*pDI2, shadowDepthShaderProgID);
	}
}

void Load_shader()
{
	cShaderManager* pTheShaderManager = new cShaderManager();
	cShaderManager::cShader vertexShad;
	vertexShad.fileName = "assets/shaders/MainVertexShader.glsl";
	cShaderManager::cShader fragShader;
	fragShader.fileName = "assets/shaders/MainFragmentShader.glsl";
	if (!pTheShaderManager->createProgramFromFile("MainShader", vertexShad, fragShader))
	{
		std::cout << "Error: didn't compile the MainShader" << std::endl;
		std::cout << pTheShaderManager->getLastError();
		return;
	}
	mainShaderProgID = pTheShaderManager->getIDFromFriendlyName("MainShader");

	cShaderManager::cShader shadowDepthVertexShad;
	shadowDepthVertexShad.fileName = "assets/shaders/ShadowDepth_VertexShader.glsl";
	cShaderManager::cShader shadowDepthFragShader;
	shadowDepthFragShader.fileName = "assets/shaders/ShadowDepth_FragmentShader.glsl";
	if (!pTheShaderManager->createProgramFromFile("ShadowDepthShader", shadowDepthVertexShad, shadowDepthFragShader))
	{
		std::cout << "Error: didn't compile the ShadowDepthShader" << std::endl;
		std::cout << pTheShaderManager->getLastError();
		return;
	}
	shadowDepthShaderProgID = pTheShaderManager->getIDFromFriendlyName("ShadowDepthShader");

	//cShaderManager::cShader quadVertexShad;
	//quadVertexShad.fileName = "assets/shaders/QuadDepth_VertexShader.glsl";
	//cShaderManager::cShader quadFragShader;
	//quadFragShader.fileName = "assets/shaders/QuadDepth_FragmentShader.glsl";
	//if (!pTheShaderManager->createProgramFromFile("QuadShader", quadVertexShad, quadFragShader))
	//{
	//	std::cout << "Error: didn't compile the QuadShader" << std::endl;
	//	std::cout << pTheShaderManager->getLastError();
	//	return;
	//}
	//quadShaderProgID = pTheShaderManager->getIDFromFriendlyName("QuadShader");

	cShaderManager::cShader blurVertexShad;
	blurVertexShad.fileName = "assets/shaders/Blur_VertexShader.glsl";
	cShaderManager::cShader blurFragShader;
	blurFragShader.fileName = "assets/shaders/Blur_FragmentShader.glsl";
	if (!pTheShaderManager->createProgramFromFile("BlurShader", blurVertexShad, blurFragShader))
	{
		std::cout << "Error: didn't compile the BlurShader" << std::endl;
		std::cout << pTheShaderManager->getLastError();
		return;
	}
	blurShaderProgID = pTheShaderManager->getIDFromFriendlyName("BlurShader");

	cShaderManager::cShader bloomVertexShad;
	bloomVertexShad.fileName = "assets/shaders/Bloom_VertexShader.glsl";
	cShaderManager::cShader bloomFragShader;
	bloomFragShader.fileName = "assets/shaders/Bloom_FragmentShader.glsl";
	if (!pTheShaderManager->createProgramFromFile("BloomShader", bloomVertexShad, bloomFragShader))
	{
		std::cout << "Error: didn't compile the BloomShader" << std::endl;
		std::cout << pTheShaderManager->getLastError();
		return;
	}
	bloomShaderProgID = pTheShaderManager->getIDFromFriendlyName("BloomShader");

	delete pTheShaderManager;
}

void Load_player()
{
	player1 = new cGameObject();
	player1->pSM = new cSimpleAssimpSkinnedMesh();
	if (player1->pSM->LoadMeshFromFile("skinnedMesh", "assets/models/Standard_Run.fbx"))
	{
		std::cout << "player1 mesh loaded" << std::endl;
	}
	else
	{
		std::cout << "player1 mesh load fail" << std::endl;
		return;
	}
	
	player1->pSM->LoadMeshAnimation("Run", "assets/models/Standard_Run.fbx");
	player1->pSM->LoadMeshAnimation("Jump", "assets/models/Jump.fbx");
	player1->pSM->LoadMeshAnimation("Roll", "assets/models/Sprinting_Forward_Roll.fbx");
	player1->pSM->LoadMeshAnimation("Die", "assets/models/Flying_Back_Death.fbx");	
	player1->meshName = "skinnedMesh";
	player1->friendlyName = "player1";
	player1->setScale(0.02f);
	player1->setOrientation(glm::vec3(0.0f, 90.0f, 0.0f));
	player1->setPosition(playerResetPos);	
	player1->textureBlendMode = BASE;
	player1->textures[0] = "Ch43_1001_Diffuse.bmp";
	player1->textures[1] = "Ch43_1001_Normal.bmp";
	g_vec_pGameObjects.push_back(player1);
	controlTargets.push_back(player1);
}

std::ifstream lightFile;
int number_of_lights = 0;
void Open_lightConfig()
{
	lightFile.open("config/light_config.txt");
	if (!lightFile.is_open())
	{
		std::cout << "Can't find the light config file." << std::endl;
		return;
	}
	std::string line = "", discardLine;
	
	while (std::getline(lightFile, line))
		++number_of_lights;

	std::cout << number_of_lights - 1 << " light(s) will be created by config." << std::endl;
	lightFile.clear();
	lightFile.seekg(0, lightFile.beg);
	lightFile >> discardLine;
}

void Load_light()
{
	glUseProgram(mainShaderProgID);
	for (unsigned int index = 0; index != number_of_lights - 1; index++)
	{
		float positionX, positionY, positionZ,
			diffuseR, diffuseG, diffuseB,
			specularR, specularG, specularB, specularPower,
			ambientR, ambientG, ambientB,
			linearAtten, quadraticAtten,
			directionX, directionY, directionZ,
			lightType, innerAngle, outerAngle,
			lightSwitch;

		lightFile >> positionX >> positionY >> positionZ
			>> diffuseR >> diffuseG >> diffuseB
			>> specularR >> specularG >> specularB >> specularPower
			>> ambientR >> ambientG >> ambientB
			>> linearAtten >> quadraticAtten
			>> directionX >> directionY >> directionZ
			>> lightType >> innerAngle >> outerAngle
			>> lightSwitch;

		std::ostringstream oss;
		oss << "theLights[" << index << "].position";
		GLint position = glGetUniformLocation(mainShaderProgID, oss.str().c_str());

		oss.clear();
		oss.str("");
		oss << "theLights[" << index << "].diffuse";
		GLint diffuse = glGetUniformLocation(mainShaderProgID, oss.str().c_str());

		oss.clear();
		oss.str("");
		oss << "theLights[" << index << "].specular";
		GLint specular = glGetUniformLocation(mainShaderProgID, oss.str().c_str());

		oss.clear();
		oss.str("");
		oss << "theLights[" << index << "].ambient";
		GLint ambient = glGetUniformLocation(mainShaderProgID, oss.str().c_str());

		oss.clear();
		oss.str("");
		oss << "theLights[" << index << "].atten";
		GLint atten = glGetUniformLocation(mainShaderProgID, oss.str().c_str());

		oss.clear();
		oss.str("");
		oss << "theLights[" << index << "].direction";
		GLint direction = glGetUniformLocation(mainShaderProgID, oss.str().c_str());

		oss.clear();
		oss.str("");
		oss << "theLights[" << index << "].param1";
		GLint param1 = glGetUniformLocation(mainShaderProgID, oss.str().c_str());

		oss.clear();
		oss.str("");
		oss << "theLights[" << index << "].param2";
		GLint param2 = glGetUniformLocation(mainShaderProgID, oss.str().c_str());

		glUniform4f(position, positionX, positionY, positionZ, 1.0f);
		glUniform4f(diffuse, diffuseR, diffuseG, diffuseB, 1.0f);
		glUniform4f(specular, specularR, specularG, specularB, specularPower);
		glUniform4f(ambient, ambientR, ambientG, ambientB, 1.0f);
		glUniform4f(atten, LightConstAtten, linearAtten, quadraticAtten, 1000000.0f);
		glm::vec3 normalizedDirection = glm::normalize(glm::vec3(directionX, directionY, directionZ));
		glUniform4f(direction, normalizedDirection.x, normalizedDirection.y, normalizedDirection.z, 1.0f);
		glUniform4f(param1, lightType, innerAngle, outerAngle, 1.0f);
		glUniform4f(param2, lightSwitch, 0.0f, 0.0f, 1.0f);
	}
}

std::ifstream textureFile;
int number_of_textures = 0;
void Open_textureConfig()
{
	textureFile.open("config/texture_config.txt");
	if (!textureFile.is_open())
	{
		std::cout << "LCan't find the texture config file." << std::endl;
		return;
	}
	std::string line = "";	
	while (std::getline(textureFile, line))
		++number_of_textures;
	std::cout << number_of_textures - 1 << " texture(s) will be loaded by config." << std::endl;
	textureFile.clear();
	textureFile.seekg(0, textureFile.beg);
	std::string basePath;
	textureFile >> basePath;
	g_pTextureManager->SetBasePath(basePath);
}

void Load_texture()
{
	for (unsigned int index = 0; index != number_of_textures - 1; index++)
	{
		std::string fileName, gammaCorrection;
		textureFile >> fileName >> gammaCorrection;
		if (gammaCorrection == "gammaCorrectionTrue")
			g_pTextureManager->Create2DTextureFromBMPFile(fileName, true, true);		
		else
			g_pTextureManager->Create2DTextureFromBMPFile(fileName, true, false);		
	}	
}


static glm::vec4 RBGcircleUpdate(glm::vec4& colour)
{
	float circleSpeed = 0.005f;
	float upperBound = 0.9f;
	float lowerBound = 0.3f;

	if (colour.r >= upperBound && colour.g <= lowerBound && colour.b < upperBound)
	{
		colour.r = upperBound;
		colour.g = lowerBound;
		colour.b += circleSpeed;
	}
	else if (colour.r > lowerBound && colour.g <= lowerBound && colour.b >= upperBound)
	{
		colour.r -= circleSpeed;
		colour.g = lowerBound;
		colour.b = upperBound;
	}
	else if (colour.r <= lowerBound && colour.g < upperBound && colour.b >= upperBound)
	{
		colour.r = lowerBound;
		colour.g += circleSpeed;
		colour.b = upperBound;
	}
	else if (colour.r <= lowerBound && colour.g >= upperBound && colour.b > lowerBound)
	{
		colour.r = lowerBound;
		colour.g = upperBound;
		colour.b -= circleSpeed;
	}
	else if (colour.r < upperBound && colour.g >= upperBound && colour.b <= lowerBound)
	{
		colour.r += circleSpeed;
		colour.g = upperBound;
		colour.b = lowerBound;
	}
	else if (colour.r >= upperBound && colour.g > lowerBound && colour.b <= lowerBound)
	{
		colour.r = upperBound;
		colour.g -= circleSpeed;
		colour.b = lowerBound;
	}

	return colour;
}

//static glm::vec4 breathingColourBrightness(glm::vec4& baseColour)
//{
//	const float frequency = 0.1f;
//	glm::vec4 breathingColour;
//
//	breathingColour.r = baseColour.r * 0.5 * (1.0f + sin(glm::two_pi<float>() * glfwGetTime() * frequency));
//	breathingColour.g = baseColour.g * 0.5 * (1.0f + sin(glm::two_pi<float>() * glfwGetTime() * frequency));
//	breathingColour.b = baseColour.b * 0.5 * (1.0f + sin(glm::two_pi<float>() * glfwGetTime() * frequency));
//
//	return breathingColour;
//}

float lerp(float a, float b, float f)
{
	return a + f * (b - a);
}

static float SmoothDamp(float current, float target, float& currentSpeed, float smoothTime, float deltaTime)
{	
	float num = 2.0f / smoothTime;
	float num2 = num * deltaTime;
	float num3 = 1.0f / (1.0f + num2 + 0.48f * num2 * num2 + 0.235f * num2 * num2 * num2);
	float num4 = current - target;
	float num5 = target;	
	target = current - num4;
	float num7 = (currentSpeed + num * num4) * deltaTime;
	currentSpeed = (currentSpeed - num * num7) * num3;
	float num8 = target + (num4 + num7) * num3;
	if (num5 - current > 0.0f == num8 > num5)
	{
		num8 = num5;
		currentSpeed = (num8 - num5) / deltaTime;
	}
	return num8;
}

glm::vec3 SmoothFollow(glm::vec3 current, glm::vec3 target, glm::vec3& currentVelocity, float smoothTime, float deltaTime)
{
	glm::vec3 result;
	result.x = SmoothDamp(current.x, target.x, currentVelocity.x, smoothTime, deltaTime);
	result.y = SmoothDamp(current.y, target.y, currentVelocity.y, smoothTime, deltaTime);
	result.z = SmoothDamp(current.z, target.z, currentVelocity.z, smoothTime, deltaTime);

	return result;
}

static void BubbleSortSinglePass(std::vector<cGameObject*>& transparentGOs)
{
	glm::vec3 eye;
	Cam.GetEyePosition(eye);
	if (transparentGOs.size() != 0)
	{
		for (unsigned int index = 0; index != (transparentGOs.size() - 1); index++)
		{			
			glm::vec3 ObjA = transparentGOs[index]->getPosition();			
			glm::vec3 ObjB = transparentGOs[index + 1]->getPosition();

			if (glm::distance2(ObjA, eye) < glm::distance2(ObjB, eye))
			{				
				cGameObject* pTemp = transparentGOs[index];
				transparentGOs[index] = transparentGOs[index + 1];
				transparentGOs[index + 1] = pTemp;
			}
		}
	}	
}

//static void BubbleSortSinglePass(std::vector<cParticle*>& vecParticles)
//{
//	glm::vec3 eye;
//	Cam.GetEyePosition(eye);
//	if (vecParticles.size() != 0)
//	{
//		for (unsigned int index = 0; index != (vecParticles.size() - 1); index++)
//		{			
//			glm::vec3 ObjA = vecParticles[index]->location;			
//			glm::vec3 ObjB = vecParticles[index + 1]->location;
//
//			if (glm::distance2(ObjA, eye) < glm::distance2(ObjB, eye))
//			{				
//				cParticle* pTemp = vecParticles[index];
//				vecParticles[index] = vecParticles[index + 1];
//				vecParticles[index + 1] = pTemp;
//			}
//		}
//	}
//}

static void DrawUpdate()
{
	for (int index = 0; index != g_vec_pGameObjects.size(); index++)
	{		
		DrawObject(glm::mat4(1.0f), g_vec_pGameObjects[index], mainShaderProgID, pVAOManager1);
	}

	for (std::vector<cObstacle*>::iterator itO = vecObstacles.begin();
		itO != vecObstacles.end(); itO++)
	{
		obstacle->setPosition((*itO)->position);
		if ((*itO)->type == LOW)
			obstacle->meshName = "lowObstacleMesh";
		else if ((*itO)->type == HIGH)
			obstacle->meshName = "highObstacleMesh";
		DrawObject(glm::mat4(1.0f), obstacle, mainShaderProgID, pVAOManager1);
	}

	//BubbleSortSinglePass(g_vec_pGameObjects);

	//for (unsigned int i = 0; i < g_vec_pTransparentGameObjects.size(); i++)
	//{
	//	DrawObject(glm::mat4(1.0f), g_vec_pTransparentGameObjects[i], mainShaderProgID, pVAOManager1);
	//}
	
	glm::vec3 eye;
	Cam.GetEyePosition(eye);
	dustParticles->getParticles(vecParticles, eye);
	vecInstanceMatrices.clear();	
	int i(0);
	// Prepare Instancing info for particles
	for (std::vector<cParticle*>::iterator itPart = vecParticles.begin();
		itPart != vecParticles.end(); itPart++)
	{	
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, (*itPart)->location);
		model = glm::scale(model, glm::vec3((*itPart)->scale));
		model *= glm::mat4((*itPart)->qOrientation);
		vecInstanceMatrices.push_back(model);
		
		instanceAlpha[i++] = (*itPart)->colourRGBA.a * 0.5f;		
	}
	if (vecInstanceMatrices.size() == 0)		return;		// No a single particle
	DrawObject(glm::mat4(1.0f), pQuadImposter, mainShaderProgID, pVAOManager1);		// Will call glDrawElementsInstanced
	
}

glm::quat RotationBetweenVectors(glm::vec3 start, glm::vec3 dest)
{
	start = glm::normalize(start);
	dest = glm::normalize(dest);

	float cosTheta = glm::dot(start, dest);
	glm::vec3 rotationAxis;

	if (cosTheta < -1 + 0.001f)
	{
		// special case when vectors in opposite directions:
		// there is no "ideal" rotation axis
		// So guess one; any will do as long as it's perpendicular to start
		rotationAxis = glm::cross(-worldFront, start);
		if (glm::length2(rotationAxis) < 0.01) // bad luck, they were parallel, try again!
			rotationAxis = glm::cross(worldRight, start);

		rotationAxis = normalize(rotationAxis);
		return glm::angleAxis(glm::radians(180.0f), rotationAxis);
	}

	rotationAxis = glm::cross(start, dest);

	float s = sqrt((1 + cosTheta) * 2);
	float invs = 1 / s;

	return glm::quat(s * 0.5f, rotationAxis.x * invs, rotationAxis.y * invs, rotationAxis.z * invs);
}

void static CameraShake()
{
	const float frequency = 10.0f;

	if (camerShakeAmplitude <= 0.0f)
	{
		camerShakeAmplitude = 0.0f;
		return;
	}
	
	camera.Position.y += sin(glm::two_pi<float>() * glfwGetTime() * frequency) * camerShakeAmplitude;
	camera.Position.x += cos(glm::two_pi<float>() * glfwGetTime() * frequency) * camerShakeAmplitude;
	camera.Position.z += sin(glm::pi<float>() * glfwGetTime() * frequency) * camerShakeAmplitude;
	camerShakeAmplitude -= 0.01f;	
}

void CamShake(glm::vec3& eyeOut)
{
	const float frequency = 10.0f;

	if (camerShakeAmplitude <= 0.0f)
	{
		camerShakeAmplitude = 0.0f;
		return;
	}

	eyeOut.y += sin(glm::two_pi<float>() * glfwGetTime() * frequency) * camerShakeAmplitude;
	eyeOut.x += cos(glm::two_pi<float>() * glfwGetTime() * frequency) * camerShakeAmplitude;
	eyeOut.z += sin(glm::pi<float>() * glfwGetTime() * frequency) * camerShakeAmplitude;
	camerShakeAmplitude -= 0.01f;
}

glm::vec3 QuadraticBezierCurve(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, float t /*0~1*/)
{
	glm::vec3 result;

	result.x = (1 - t) * (1 - t) * p1.x + 2 * (1 - t) * t * p2.x + t * t * p3.x;
	result.y = (1 - t) * (1 - t) * p1.y + 2 * (1 - t) * t * p2.y + t * t * p3.y;
	result.z = (1 - t) * (1 - t) * p1.z + 2 * (1 - t) * t * p2.z + t * t * p3.z;

	return result;
}

static float easeInSine(float ratio)
{
	return 1.0f - cos(ratio * (glm::pi<float>() / 2.0f));
}

static float easeOutSine(float ratio)
{
	return sin(ratio * (glm::pi<float>() / 2.0f));
}

float easeInOutSine(float ratio)
{
	ratio = ratio * 2.0f;
	return (ratio < 1.0f) ? 0.5f * easeInSine(ratio) : 0.5f * easeOutSine(ratio - 1.0f) + 0.5f;
}

static float easeInQuadratic(float ratio)
{
	return ratio * ratio;
}

static float easeOutQuadratic(float ratio)
{
	ratio = 1.0f - ratio;
	return 1.0f - ratio * ratio;
}

float easeInOutQuadratic(float ratio)
{
	ratio = ratio * 2.0f;
	return (ratio < 1.0f) ? 0.5f * easeInQuadratic(ratio) : 0.5f * easeOutQuadratic(ratio - 1.0f) + 0.5f;
}

static float easeInQuartic(float ratio)
{
	return ratio * ratio * ratio * ratio;
}

static float easeOutQuartic(float ratio)
{
	ratio = 1.0f - ratio;
	return 1.0f - ratio * ratio * ratio * ratio;
}

float easeInOutQuartic(float ratio)
{
	ratio = ratio * 2.0f;
	return (ratio < 1.0f) ? 0.5f * easeInQuartic(ratio) : 0.5f * easeOutQuartic(ratio - 1.0f) + 0.5f;
}

unsigned int loadTexture(char const* path, bool gammaCorrection)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum internalFormat;
		GLenum dataFormat;
		if (nrComponents == 1)
		{
			internalFormat = dataFormat = GL_RED;
		}
		else if (nrComponents == 3)
		{
			internalFormat = gammaCorrection ? GL_SRGB : GL_RGB;
			dataFormat = GL_RGB;
		}
		else if (nrComponents == 4)
		{
			internalFormat = gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA;
			dataFormat = GL_RGBA;
		}

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

unsigned int loadCubemap(std::vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

CRITICAL_SECTION BoneTransform_lock;
void UpdateBoneTransform()
{
	while (running)
	{
		EnterCriticalSection(&BoneTransform_lock);
		player1->pSM->BoneTransform(player1->getAnimationFrameTime(),
			player1->getCurrentAnimationName(),
			vecFinalTransformation,
			vecObjectBoneTransformation,
			vecOffsets);
		LeaveCriticalSection(&BoneTransform_lock);
	}
}


void UpdateAnimationFrame(cGameObject* animationGO, float deltaTime)
{
	// Set animation play speed
	animationGO->animationSpeed = deltaTime;	

	// Check animation time
	if (animationGO->loopAnimationState)		// Running
	{		
		animationGO->setAnimationFrameTime(animationGO->getAnimationFrameTime() + animationGO->animationSpeed);
		dustParticles->location = controlTargets[controlIndex]->getPosition() + glm::vec3(-0.5f, 0.1f, 0.0f);
	}
	else
	{
		if (animationGO->getCurrentAnimationName() == "Jump")
		{
			if (animationGO->getAnimationFrameTime() >= 0.75f)
			{
				animationGO->loopAnimationState = true;
				animationGO->isJumping = false;				
				animationGO->velocity = glm::vec3(0.0f, 0.0f, 0.0f);
				glm::vec3 pos = animationGO->getPosition();				
				animationGO->setPosition(glm::vec3(pos.x, 0.0f, pos.z));
				dustParticles->enableNewParticles();
				animationGO->setCurrentAnimationName("Run");
			}
			else
			{
				animationGO->setAnimationFrameTime(animationGO->getAnimationFrameTime() + animationGO->animationSpeed);
			}
		}
		else if (animationGO->getCurrentAnimationName() == "Roll")
		{
			if (animationGO->getAnimationFrameTime() >= 1.2f)
			{
				animationGO->isRolling = false;
				animationGO->loopAnimationState = true;
				animationGO->setCurrentAnimationName("Run");
			}
			else if (animationGO->getAnimationFrameTime() >= 0.6f)
			{
				dustParticles->location = controlTargets[controlIndex]->getPosition() + glm::vec3(2.0f, 0.0f, 0.0f);
				animationGO->setAnimationFrameTime(animationGO->getAnimationFrameTime() + animationGO->animationSpeed);
			}
			else
			{
				animationGO->setAnimationFrameTime(animationGO->getAnimationFrameTime() + animationGO->animationSpeed);
			}
		}
		else if (animationGO->getCurrentAnimationName() == "Die")
		{
			if (animationGO->getAnimationFrameTime() <= 3.0f)
			{
				animationGO->setAnimationFrameTime(animationGO->getAnimationFrameTime() + animationGO->animationSpeed);
			}
		}		
	}

	if (animationGO->dead)	return;

	float speed = 7.0f;

	if (!playerInPlace)
	{
		
		animationGO->setPosition(animationGO->getPosition() + glm::vec3(1.0f, 0.0f, 0.0f) * speed * deltaTime);
		if (animationGO->getPosition().x >= playerRunningPositionX)
		{
			playerInPlace = true;
		}
	}

	if (animationGO->isMovingDown)
	{		
		animationGO->setPosition(animationGO->getPosition() + glm::vec3(0.0f, 0.0f, 1.0f) * speed * deltaTime);
		if (animationGO->getPosition().z >= animationGO->currentLane * LaneWidth)
		{
			glm::vec3 pos = animationGO->getPosition();
			pos.z = animationGO->currentLane * LaneWidth;
			animationGO->setPosition(pos);
			animationGO->isMovingDown = false;
		}
	}

	if (animationGO->isMovingUp)
	{		
		animationGO->setPosition(animationGO->getPosition() + glm::vec3(0.0f, 0.0f, -1.0f) * speed * deltaTime);
		if (animationGO->getPosition().z <= animationGO->currentLane * LaneWidth)
		{
			glm::vec3 pos = animationGO->getPosition();
			pos.z = animationGO->currentLane * LaneWidth;
			animationGO->setPosition(pos);
			animationGO->isMovingUp = false;
		}
	}

}

CRITICAL_SECTION animation_lock;
void StepAnimation()
{
	auto last_time = std::chrono::steady_clock::now();
	auto current_time = std::chrono::steady_clock::now();
	cLowPassFilter avgDeltaTime;
	const double SOME_HUGE_TIME = 0.1;	// 100 ms;
	double avgDT;

	while (running)
	{
		current_time = std::chrono::steady_clock::now();
		auto duration = current_time - last_time;
		float delta_time = duration.count() / float(CLOCKS_PER_SEC) / float(CLOCKS_PER_SEC) / float(CLOCKS_PER_SEC);
		last_time = current_time;
		if (delta_time > SOME_HUGE_TIME)		delta_time = SOME_HUGE_TIME;
		avgDeltaTime.addValue(delta_time);
		avgDT = avgDeltaTime.getAverage();

		EnterCriticalSection(&animation_lock);
		
		UpdateAnimationFrame(player1, static_cast<float>(avgDT));		

		dustParticles->Step(static_cast<float>(avgDT));

		LeaveCriticalSection(&animation_lock);
	}
}


void CreateDepthMapFBO()
{	
	glGenFramebuffers(1, &depthMapFBO);
	// create depth texture	
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ConfigLightViewMatrix()
{	
	float near_plane = 0.3f, far_plane = 20.0f;
	glm::mat4 lightProjection = glm::ortho(-12.0f, 12.0f, -12.0f, 12.0f, near_plane, far_plane);
	glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
	lightSpaceMatrix = lightProjection * lightView;
	glUseProgram(shadowDepthShaderProgID);
	glUniformMatrix4fv(glGetUniformLocation(shadowDepthShaderProgID, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
	glUseProgram(mainShaderProgID);
	glUniformMatrix4fv(glGetUniformLocation(mainShaderProgID, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

}

void CreateHDRFBO()
{	
	glGenFramebuffers(1, &hdrFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

	// create 2 floating point color buffers (1 for normal rendering, other for brightness treshold values)	
	glGenTextures(2, colorBuffers);
	for (unsigned int i = 0; i < 2; i++)
	{
		glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 1280, 720, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		// attach texture to framebuffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
	}
	// create and attach depth buffer (renderbuffer)
	unsigned int rboDepth;
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1280, 720);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
	// tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);
	// finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void CreateBlurFBO()
{	
	glGenFramebuffers(2, blurFBO);
	glGenTextures(2, blurColorbuffers);
	for (unsigned int i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, blurFBO[i]);
		glBindTexture(GL_TEXTURE_2D, blurColorbuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 1280, 720, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurColorbuffers[i], 0);
		
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Framebuffer not complete!" << std::endl;
	}

}

// Renders a 1x1 XY quad in NDC
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}
