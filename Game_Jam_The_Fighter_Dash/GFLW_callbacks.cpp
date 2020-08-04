#include "GFLW_callbacks.h"
#include <stdio.h>
#include <iostream>
#include "cGameObject.h"
#include "cParticleEmitter.h"
#include "cCamera.h"
#include "cObstacleGenerator.h"

//extern cGameObject* pFindObjectByFriendlyName(std::string name);
extern double averageDeltaTime;
extern float camerShakeAmplitude;
extern bool debugMode;
extern bool wireframeMode;
extern bool normalPassMode;
extern bool albedoPassMode;
extern bool useFollowCamera;
extern bool running;
extern bool g_MouseLeftButtonIsDown;
extern double g_scrollY;
extern float g_FOV;
extern std::vector<cGameObject*> controlTargets;
extern unsigned int controlIndex;
extern cCamera camera;
extern cParticleEmitter* dustParticles;
bool isOnlyAltKeyDown(int mods);
extern bool camerShakeCooldown;
extern cObstacleGenerator* obstacleGenerator;
extern glm::vec3 playerResetPos;
extern bool playerInPlace;

void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GLFW_TRUE);
		running = false;
	}
		

	if (key == GLFW_KEY_UP && action == GLFW_PRESS)		// Jump
	{
		if (!controlTargets[controlIndex]->isJumping
			&& !controlTargets[controlIndex]->isRolling
			&& !controlTargets[controlIndex]->dead
			&& !controlTargets[controlIndex]->isMovingUp
			&& !controlTargets[controlIndex]->isMovingDown)
		{			
			controlTargets[controlIndex]->setCurrentAnimationName("Jump");
			controlTargets[controlIndex]->loopAnimationState = false;			
			controlTargets[controlIndex]->setAnimationFrameTime(0.0f);			
			controlTargets[controlIndex]->isJumping = true;
			dustParticles->disableNewParticles();
		}		
	}
	
	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)		// Roll
	{
		if (!controlTargets[controlIndex]->isJumping
			&& !controlTargets[controlIndex]->isRolling
			&& !controlTargets[controlIndex]->dead
			&& !controlTargets[controlIndex]->isMovingUp
			&& !controlTargets[controlIndex]->isMovingDown)
		{
			controlTargets[controlIndex]->setCurrentAnimationName("Roll");
			controlTargets[controlIndex]->isRolling = true;
			controlTargets[controlIndex]->loopAnimationState = false;
			controlTargets[controlIndex]->setAnimationFrameTime(0.0f);			
		}
	}

	if (key == GLFW_KEY_W && action == GLFW_PRESS)
	{
		if (!controlTargets[controlIndex]->isJumping
			&& !controlTargets[controlIndex]->isRolling
			&& !controlTargets[controlIndex]->dead
			&& !controlTargets[controlIndex]->isMovingUp
			&& !controlTargets[controlIndex]->isMovingDown)
		{
			if (controlTargets[controlIndex]->currentLane > -1)		// To match position Z
			{
				controlTargets[controlIndex]->isMovingUp = true;
				controlTargets[controlIndex]->currentLane--;
			}
			
		}
		
	}

	if (key == GLFW_KEY_S && action == GLFW_PRESS)
	{
		if (!controlTargets[controlIndex]->isJumping
			&& !controlTargets[controlIndex]->isRolling
			&& !controlTargets[controlIndex]->dead
			&& !controlTargets[controlIndex]->isMovingUp
			&& !controlTargets[controlIndex]->isMovingDown)
		{
			if (controlTargets[controlIndex]->currentLane < 1)		// To match position Z
			{
				controlTargets[controlIndex]->isMovingDown = true;
				controlTargets[controlIndex]->currentLane++;
			}
		}
		
	}

	if (key == GLFW_KEY_R && action == GLFW_PRESS)
	{
		controlTargets[controlIndex]->dead = false;
		controlTargets[controlIndex]->isJumping = false;
		controlTargets[controlIndex]->isRolling = false;
		controlTargets[controlIndex]->isMovingDown = false;
		controlTargets[controlIndex]->isMovingUp = false;
		controlTargets[controlIndex]->currentLane = 0;
		controlTargets[controlIndex]->loopAnimationState = true;
		controlTargets[controlIndex]->setCurrentAnimationName("Run");		
		controlTargets[controlIndex]->setAnimationFrameTime(0.0f);
		controlTargets[controlIndex]->velocity = glm::vec3(0.0f, 0.0f, 0.0f);
		dustParticles->enableNewParticles();
		obstacleGenerator->Reset();
		controlTargets[controlIndex]->setPosition(playerResetPos);
		playerInPlace = false;		
	}	
		

	if ((key == GLFW_KEY_LEFT && action == GLFW_RELEASE) || (key == GLFW_KEY_RIGHT && action == GLFW_RELEASE))
	{		
		//dustParticles->disableNewParticles();
	}

	if (key == GLFW_KEY_2 && action == GLFW_PRESS)
	{
		debugMode = !debugMode;
		debugMode ? printf("Debug mode enabled.\n") : printf("Debug mode disabled.\n");
	}
	if (key == GLFW_KEY_3 && action == GLFW_PRESS)
	{
		wireframeMode = !wireframeMode;
		wireframeMode ? printf("Wireframe mode enabled.\n") : printf("Wireframe mode disabled.\n");
		normalPassMode = false;
		albedoPassMode = false;
	}
	if (key == GLFW_KEY_4 && action == GLFW_PRESS)
	{
		normalPassMode = !normalPassMode;
		normalPassMode ? printf("Normal pass render mode enabled.\n") : printf("Normal pass render mode disabled.\n");
		wireframeMode = false;
		albedoPassMode = false;
	}
	if (key == GLFW_KEY_5 && action == GLFW_PRESS)
	{
		albedoPassMode = !albedoPassMode;
		albedoPassMode ? printf("Albedo pass render mode enabled.\n") : printf("Albedo pass render mode disabled.\n");
		wireframeMode = false;
		normalPassMode = false;
	}	
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && mods == GLFW_MOD_ALT)
	{
		g_MouseLeftButtonIsDown = true;
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
	{
		g_MouseLeftButtonIsDown = false;
	}
	//std::cout << "g_MouseLeftButtonIsDown: " << g_MouseLeftButtonIsDown << std::endl;
	return;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	// A regular mouse wheel returns the y value	
	//g_scrollY = yoffset;
	//std::cout << "Mouse wheel: " << yoffset << std::endl;
	if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) != GLFW_PRESS)	return;

	if (g_FOV >= 5.0f && g_FOV <= 90.0f)
		g_FOV -= yoffset * 2.0f;
	else if (g_FOV < 5.0f)
		g_FOV = 5.0f;
	else if (g_FOV > 90.0f)
		g_FOV = 90.0f;

	//std::cout << "g_FOV: " << g_FOV << std::endl;
	return;
}


bool isOnlyAltKeyDown(int mods)
{
	if (mods == GLFW_MOD_ALT)
	{
		return true;
	}
	return false;
}