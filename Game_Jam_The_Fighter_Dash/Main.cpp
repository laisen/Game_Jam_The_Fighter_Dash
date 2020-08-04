#include <iostream>	
#include <fstream>
#include "GLCommon.h"
#include "GFLW_callbacks.h"
#include "cWindow.h"
#include "cCamera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>		// glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp>		// glm::value_ptr
#include "cModelLoader.h"
#include "cVAOManager.h"
#include "cGameObject.h"
#include "cShaderManager.h"
#include "cBasicTextureManager.h"
#include "cPhysics.h"
#include "cParticleEmitter.h"
#include "cObstacleGenerator.h"
#include "DrawObjectUtilLoop.h"
//#include "cCommandGroup.h"
//#include "cMoveTo.h"
//#include "cMoveToRelative.h"
//#include "cScaleTo.h"
//#include "cOrientToRelative.h"
//#include "cOrientTo.h"
//#include "cFollowCurve.h"
//#include "cFollowObject.h"
//#include "cTrigger.h"
//#include "cLuaBrain/cLuaBrain.h"
#include "cFBO.h"
#include "globalUtil.h"
#include "DebugRenderer/cDebugRenderer.h"
#include <thread>


int main(void)
{
	// Load file IO threading
	std::thread thread_load_player(Load_player);
	std::thread thread_open_meshPath(Open_meshPath);
	std::thread thread_load_gameObjects(Load_gameObjects);
	std::thread thread_open_light_config(Open_lightConfig);
	std::thread thread_open_texture_config(Open_textureConfig);

	srand(time(0));	

	cWindow* myWindow = new cWindow();
	myWindow->createWindow();
	// center mouse position
	lastX = myWindow->windowWidth / 2.0f, lastY = myWindow->windowHeight / 2.0f;	
	Cam.OnWindowResize(myWindow->windowWidth, myWindow->windowHeight);

	Load_shader();	

	thread_open_meshPath.join();
	thread_load_player.join();

	// Player bone threading
	InitializeCriticalSection(&BoneTransform_lock);
	std::thread thread_update_BoneTransform(UpdateBoneTransform);
	thread_update_BoneTransform.detach();

	Load_meshes();

	thread_open_light_config.join();
	Load_light();

	thread_open_texture_config.join();
	Load_texture();

	GLint eyeLocation_UL = glGetUniformLocation(mainShaderProgID, "eyeLocation");	
	GLint matView_UL = glGetUniformLocation(mainShaderProgID, "matView");
	GLint matProj_UL = glGetUniformLocation(mainShaderProgID, "matProj");
	
	// Tie depth map texture to the sampler for shader
	glUniform1i(glGetUniformLocation(mainShaderProgID, "shadowMap"), 6);		// GL_TEXTURE6	
	glUseProgram(quadShaderProgID);
	//glUniform1i(glGetUniformLocation(quadShaderProgID, "depthMap"), 5);		// GL_TEXTURE5
	glUseProgram(blurShaderProgID);	
	glUniform1i(glGetUniformLocation(blurShaderProgID, "image"), 7);		// GL_TEXTURE7
	glUseProgram(bloomShaderProgID);
	glUniform1i(glGetUniformLocation(bloomShaderProgID, "scene"), 8);		// GL_TEXTURE8
	glUniform1i(glGetUniformLocation(bloomShaderProgID, "bloomBlur"), 9);		// GL_TEXTURE9

	cLowPassFilter avgDeltaTimeThingy;	

	thread_load_gameObjects.join();	

	CreateDepthMapFBO();

	ConfigLightViewMatrix();

	CreateHDRFBO();

	CreateBlurFBO();

	obstacleGenerator = new cObstacleGenerator();

	// Animation threading
	InitializeCriticalSection(&animation_lock);
	std::thread thread_animation_step(StepAnimation);
	thread_animation_step.detach();	
	

	// Main loop start ============================================================================================
	while (!glfwWindowShouldClose(myWindow->window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		const double SOME_HUGE_TIME = 0.1;	// 100 ms;
		if (deltaTime > SOME_HUGE_TIME)		deltaTime = SOME_HUGE_TIME;
		avgDeltaTimeThingy.addValue(deltaTime);
		averageDeltaTime = avgDeltaTimeThingy.getAverage();

		processInput(myWindow->window);
		
		// Handle obstacles
		obstacleGenerator->Step((float)averageDeltaTime);
		obstacleGenerator->GetObstacle(vecObstacles);

		//now = clock() / float(CLOCKS_PER_SEC);
		//if (now - lastTime >= 0.5f)
		//{			
		//	lastTime = now;
		//}

		// Shadow depth pass, render scene to shadow map from light's perspective----------------------------------
		glUseProgram(shadowDepthShaderProgID);
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		for (int index = 0; index != g_vec_pGameObjects.size(); index++)
		{
			DrawObject(glm::mat4(1.0f), g_vec_pGameObjects[index], shadowDepthShaderProgID, pVAOManager2);
		}

		for (std::vector<cObstacle*>::iterator itO = vecObstacles.begin();
			itO != vecObstacles.end(); itO++)
		{
			obstacle->setPosition((*itO)->position);
			if ((*itO)->type == LOW)
				obstacle->meshName = "lowObstacleMesh";			
			else if ((*itO)->type == HIGH)
				obstacle->meshName = "highObstacleMesh";
			DrawObject(glm::mat4(1.0f), obstacle, shadowDepthShaderProgID, pVAOManager2);
		}

		// Main scene render-----------------------------------------------------------------------------------
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(mainShaderProgID);
		float ratio;
		int width, height;
		glfwGetFramebufferSize(myWindow->window, &width, &height);
		//glViewport(0, 0, width, height);
		glViewport(0, 0, 1280, 720);
		ratio = width / (float)height;
		glm::mat4 projection = glm::perspective(glm::radians(g_FOV), ratio, 0.1f, 500.0f);
		glUniformMatrix4fv(matProj_UL, 1, GL_FALSE, glm::value_ptr(projection));		
		
		if (useFollowCamera)
		{			
			Cam.OnWindowResize(width, height);
			Cam.GetViewMatrix(view);
			//glm::vec3 pos = controlTargets[controlIndex]->getPosition();
			//pos.y += 1.5f;
			glm::vec3 pos(0.0f, 0.0f, (controlTargets[controlIndex]->getPosition().z - 4.0f)/2.0f);
			Cam.SetTargetTransform(pos);
			Cam.Update(deltaTime);
			glm::vec3 eye;
			Cam.GetEyePosition(eye);			
			glUniform4f(eyeLocation_UL, eye.x, eye.y, eye.z, 1.0f);			
		}
		else
		{
			view = camera.GetViewMatrix();
			glUniform4f(eyeLocation_UL, camera.Position.x, camera.Position.y, camera.Position.z, 1.0f);
		}
		
		glUniformMatrix4fv(matView_UL, 1, GL_FALSE, glm::value_ptr(view));

		//UpdateAnimationFrame(player1);

		// Physis loop
		//pPhysics->IntegrationStep(controlTargets, (float)averageDeltaTime);
		// Collision loop
		//pPhysics->CollisionTest(controlTargets);

		//dustParticles->Step(static_cast<float>(deltaTime));		

		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		DrawUpdate();		

		// Double blur pass for bright fragments---------------------------------------------------------
		bool horizontal = true, first_iteration = true;
		unsigned int blurTimes = 4;		
		glUseProgram(blurShaderProgID);		
		glActiveTexture(GL_TEXTURE7);
		for (unsigned int i = 0; i < blurTimes; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, blurFBO[horizontal]);			
			glUniform1i(glGetUniformLocation(blurShaderProgID, "horizontal"), horizontal);
			glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : blurColorbuffers[!horizontal]);  // Only bind original bright colour texture at first iteration
			renderQuad();
			horizontal = !horizontal;
			if (first_iteration)	first_iteration = false;
		}		

		// Final bloom pass to the fullscreen quad---------------------------------------------------------
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		
		glUseProgram(bloomShaderProgID);		
		glViewport(0, 0, width, height);
		glActiveTexture(GL_TEXTURE8);
		glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
		glActiveTexture(GL_TEXTURE9);
		glBindTexture(GL_TEXTURE_2D, blurColorbuffers[!horizontal]);		
		renderQuad();
		

		glfwSwapBuffers(myWindow->window);		
		glfwPollEvents();
	}

	glfwDestroyWindow(myWindow->window);
	glfwTerminate();

	delete pVAOManager1;
	delete pVAOManager2;
	delete pPhysics;
	delete myWindow;
	DeleteCriticalSection(&animation_lock);
	DeleteCriticalSection(&BoneTransform_lock);
	exit(EXIT_SUCCESS);
}