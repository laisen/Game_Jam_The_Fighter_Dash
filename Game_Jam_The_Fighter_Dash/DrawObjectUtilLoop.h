#pragma once

#include "GLCommon.h"
#include <glm/mat4x4.hpp>
#include "cGameObject.h"
#include "cVAOManager.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "cSimpleAssimpSkinnedMeshLoader_OneMesh.h"

extern cBasicTextureManager* g_pTextureManager;
extern bool wireframeMode;
extern bool normalPassMode;
extern bool albedoPassMode;
extern unsigned int indoorCubemapTexture;
extern unsigned int outdoorCubemapTexture;
extern GLuint shadowDepthShaderProgID;
extern GLuint mainShaderProgID;
extern glm::vec3 rightHand_BoneLocationFK;
extern std::vector< glm::mat4x4 > vecFinalTransformation;
extern std::vector< glm::mat4x4 > vecOffsets;
extern std::vector< glm::mat4x4 > vecObjectBoneTransformation;
extern double averageDeltaTime;
extern cGameObject* player1;
extern bool playerInPlace;
extern std::vector< glm::mat4 > vecInstanceMatrices;
extern float instanceAlpha[150];

glm::mat4 calculateWorldMatrix(cGameObject* pCurrentObject)
{
	glm::mat4 m = glm::mat4(1.0f);

	// TRANSLATION
	glm::mat4 matTrans = glm::translate(glm::mat4(1.0f),
			glm::vec3(pCurrentObject->getPosition().x, pCurrentObject->getPosition().y,	pCurrentObject->getPosition().z));
	m = m * matTrans;
	
	glm::mat4 matRotation = glm::mat4(pCurrentObject->getQOrientation());
	m = m * matRotation;

	// SCALE
	glm::mat4 scale = glm::scale(glm::mat4(1.0f),
		glm::vec3(pCurrentObject->getScale(), pCurrentObject->getScale(), pCurrentObject->getScale()));
	m = m * scale;

	return m;
}

static glm::vec2 groundOffset = glm::vec2(0.0f, 0.0f);
void SetUpTextureBindingsForObject(cGameObject* pCurrentObject, GLint shaderProgID)
{	
	// Tie the texture to the texture unit
	GLuint texSamp0_UL = g_pTextureManager->getTextureIDFromName(pCurrentObject->textures[0]);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texSamp0_UL);	

	GLuint texSamp1_UL = g_pTextureManager->getTextureIDFromName(pCurrentObject->textures[1]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texSamp1_UL);

	GLuint texSamp2_UL = g_pTextureManager->getTextureIDFromName(pCurrentObject->textures[2]);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, texSamp2_UL);

	GLuint texSamp3_UL = g_pTextureManager->getTextureIDFromName(pCurrentObject->textures[3]);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, texSamp3_UL);

	// Tie the texture units to the samplers in the shader
	GLint textSamp00_UL = glGetUniformLocation(shaderProgID, "textSamp00");
	glUniform1i(textSamp00_UL, 0);	// Texture unit 0

	GLint textSamp01_UL = glGetUniformLocation(shaderProgID, "textSamp01");
	glUniform1i(textSamp01_UL, 1);	// Texture unit 1

	GLint textSamp02_UL = glGetUniformLocation(shaderProgID, "textSamp02");
	glUniform1i(textSamp02_UL, 2);	// Texture unit 2

	GLint textSamp03_UL = glGetUniformLocation(shaderProgID, "textSamp03");
	glUniform1i(textSamp03_UL, 3);	// Texture unit 3

	GLint tex0_tile_UL = glGetUniformLocation(shaderProgID, "tex_0_3_tile");
	glUniform4f(tex0_tile_UL,
		pCurrentObject->textureTile[0],		// 1.0
		pCurrentObject->textureTile[1],
		pCurrentObject->textureTile[2],
		pCurrentObject->textureTile[3]);

	if (pCurrentObject->friendlyName == "ground" && shaderProgID == mainShaderProgID)
	{
		GLint groundOffset_UL = glGetUniformLocation(shaderProgID, "groundOffset");
		glUniform2f(groundOffset_UL, groundOffset.x, groundOffset.y);

		if (player1->dead || !playerInPlace)	return;

		groundOffset.x += 0.46f * averageDeltaTime;
	}
	else
	{
		GLint groundOffset_UL = glGetUniformLocation(shaderProgID, "groundOffset");
		glUniform2f(groundOffset_UL, 0.0f, 0.0f);
	}
	return;
}

void DrawObject(glm::mat4 worldMatrix, cGameObject* pCurrentObject, GLint shaderProgID, cVAOManager* pVAOManager)
{
	if (pCurrentObject->isVisible == false)
	{
		return;
	}

	// Turns on "alpha transparency"
	//glEnable(GL_BLEND);	
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glEnable(GL_CULL_FACE);
	// ************************************************************************
	// Set the texture bindings and samplers

	// See if this is a skybox object?
	GLint bIsSkyBox_UL = glGetUniformLocation(shaderProgID, "bIsSkyBox");
	GLint skyBoxSampler_UL = glGetUniformLocation(shaderProgID, "skyBox");
	if (pCurrentObject->textureBlendMode != CUBE_MAP)
	{			
		// Is a regular 2D textured object
		SetUpTextureBindingsForObject(pCurrentObject, shaderProgID);
		glUniform1f(bIsSkyBox_UL, (float)GL_FALSE);

		// Don't draw back facing triangles (default)
		//glDisable(GL_CULL_FACE);
		glCullFace(GL_BACK);

	}
	else
	{
		// Draw the back facing triangles. 		
		//glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);		

		glUniform1f(bIsSkyBox_UL, (float)GL_TRUE);
		
		glActiveTexture(GL_TEXTURE10);
		if (pCurrentObject->friendlyName == "indoorSkybox")
		{
			glBindTexture(GL_TEXTURE_CUBE_MAP, indoorCubemapTexture);
		}			
		if (pCurrentObject->friendlyName == "outdoorSkybox")
		{
			glBindTexture(GL_TEXTURE_CUBE_MAP, outdoorCubemapTexture);
		}
		// Tie the texture units to the samplers in the shader
		//GLint skyBoxSampler_UL = glGetUniformLocation(shaderProgID, "skyBox");
		glUniform1i(skyBoxSampler_UL, 10);
	}
	// ***************************************************************************************

	worldMatrix = calculateWorldMatrix(pCurrentObject);

	GLint matModel_UL = glGetUniformLocation(shaderProgID, "matModel");

	glUniformMatrix4fv(matModel_UL, 1, GL_FALSE, glm::value_ptr(worldMatrix));

	// OpenGL use transpose inverse matrix to get normal
	GLint matModelIT_UL = glGetUniformLocation(shaderProgID, "matModelInverseTranspose");
	
	glm::mat4 matModelInverseTranspose = glm::transpose(glm::inverse(worldMatrix));
	glUniformMatrix4fv(matModelIT_UL, 1, GL_FALSE, glm::value_ptr(matModelInverseTranspose));	

	GLint diffuseColour_UL = glGetUniformLocation(shaderProgID, "diffuseColour");
	glUniform4f(diffuseColour_UL,
		pCurrentObject->getDiffuseColour().r,
		pCurrentObject->getDiffuseColour().g,
		pCurrentObject->getDiffuseColour().b,
		pCurrentObject->getDiffuseColour().a);		// Transparency

	GLint specularColour_UL = glGetUniformLocation(shaderProgID, "specularColour");
	glUniform4f(specularColour_UL, 1.0f, 1.0f, 1.0f, 1.0f);		// Same shininess for all objects
	
	GLint debugColour_UL = glGetUniformLocation(shaderProgID, "wireframeColour");
	GLint bWireframeMode_UL = glGetUniformLocation(shaderProgID, "bWireframeMode");
	GLint bNormalPassMode_UL = glGetUniformLocation(shaderProgID, "bNormalPassMode");
	GLint bAlbedoPassMode_UL = glGetUniformLocation(shaderProgID, "bAlbedoPassMode");
	GLint bUseTransparentMap_UL = glGetUniformLocation(shaderProgID, "bUseTransparentMap");
	GLint bNoTexture_UL = glGetUniformLocation(shaderProgID, "bNoTexture");
	GLint bIsImposter_UL = glGetUniformLocation(shaderProgID, "bIsImposter");
	GLint bBaseTexture_UL = glGetUniformLocation(shaderProgID, "bBaseTexture");
	GLint bDiffuseColourBlend_UL = glGetUniformLocation(shaderProgID, "bDiffuseColourBlend");
	GLint bColourBlend_UL = glGetUniformLocation(shaderProgID, "bColourBlend");
	GLint bMultiplyBlend_UL = glGetUniformLocation(shaderProgID, "bMultiplyBlend");
	GLint bAverageBlend_UL = glGetUniformLocation(shaderProgID, "bAverageBlend");
	GLint bSoftlightBlend_UL = glGetUniformLocation(shaderProgID, "bSoftlightBlend");
	GLint bNoLighting_UL = glGetUniformLocation(shaderProgID, "bNoLighting");
	GLint bIsChrome_UL = glGetUniformLocation(shaderProgID, "bIsChrome");	

	// Restore switch texture uniform to false
	glUniform1f(bNoTexture_UL, (float)GL_FALSE);
	glUniform1f(bIsImposter_UL, (float)GL_FALSE);
	glUniform1f(bBaseTexture_UL, (float)GL_FALSE);
	glUniform1f(bDiffuseColourBlend_UL, (float)GL_FALSE);
	glUniform1f(bColourBlend_UL, (float)GL_FALSE);
	glUniform1f(bMultiplyBlend_UL, (float)GL_FALSE);
	glUniform1f(bAverageBlend_UL, (float)GL_FALSE);
	glUniform1f(bSoftlightBlend_UL, (float)GL_FALSE);
	glUniform1f(bIsChrome_UL, (float)GL_FALSE);

	if (wireframeMode)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glUniform4f(debugColour_UL,
			pCurrentObject->wireframeColour.r,
			pCurrentObject->wireframeColour.g,
			pCurrentObject->wireframeColour.b,
			pCurrentObject->wireframeColour.a);
		glUniform1f(bWireframeMode_UL, (float)GL_TRUE);
	}
	else if (pCurrentObject->isWireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glUniform4f(debugColour_UL,
			pCurrentObject->wireframeColour.r,
			pCurrentObject->wireframeColour.g,
			pCurrentObject->wireframeColour.b,
			pCurrentObject->wireframeColour.a);
		glUniform1f(bWireframeMode_UL, (float)GL_TRUE);
	}
	else
	{
		glUniform1f(bWireframeMode_UL, (float)GL_FALSE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if (normalPassMode)	{	glUniform1f(bNormalPassMode_UL, (float)GL_TRUE);	}
	else	{	glUniform1f(bNormalPassMode_UL, (float)GL_FALSE);	}

	if (albedoPassMode) {	glUniform1f(bAlbedoPassMode_UL, (float)GL_TRUE);	}
	else	{	glUniform1f(bAlbedoPassMode_UL, (float)GL_FALSE);	}

	if (pCurrentObject->useTransparentMap)
	{
		glUniform1f(bUseTransparentMap_UL, (float)GL_TRUE);
	}
	else
	{
		glUniform1f(bUseTransparentMap_UL, (float)GL_FALSE);
	}	

	switch (pCurrentObject->textureBlendMode)
	{
	case NO_TEXTURE:		glUniform1f(bNoTexture_UL, (float)GL_TRUE);				break;
	case BASE:				glUniform1f(bBaseTexture_UL, (float)GL_TRUE);			break;
	case IMPOSTER:			glUniform1f(bIsImposter_UL, (float)GL_TRUE);			break;
	case DIFFUSE_COLOUR:	glUniform1f(bDiffuseColourBlend_UL, (float)GL_TRUE);	break;
	case COLOUR:			glUniform1f(bColourBlend_UL, (float)GL_TRUE);			break;
	case MULTIPLY:			glUniform1f(bMultiplyBlend_UL, (float)GL_TRUE);			break;
	case AVERAGE:			glUniform1f(bAverageBlend_UL, (float)GL_TRUE);			break;
	case SOFT_LIGHT:		glUniform1f(bSoftlightBlend_UL, (float)GL_TRUE);		break;
	case CHROME:
		glUniform1f(bIsChrome_UL, (float)GL_TRUE);
		glActiveTexture(GL_TEXTURE10);
		glBindTexture(GL_TEXTURE_CUBE_MAP, indoorCubemapTexture);		
		glUniform1i(skyBoxSampler_UL, 10);
		break;
	default:																		break;
	}

	if (pCurrentObject->noLighting)
	{
		glUniform1f(bNoLighting_UL, (float)GL_TRUE);
	}
	else
	{
		glUniform1f(bNoLighting_UL, (float)GL_FALSE);
	}


	if (pCurrentObject->disableDepthBufferTest)
	{
		glDisable(GL_DEPTH_TEST);					// DEPTH Test OFF
	}
	else
	{
		glEnable(GL_DEPTH_TEST);						// Turn ON depth test
	}

	if (pCurrentObject->disableDepthBufferWrite)
	{
		//glDisable(GL_DEPTH);						// DON'T Write to depth buffer
		glDepthMask(GL_FALSE);
	}
	else
	{
		//glEnable(GL_DEPTH);								// Write to depth buffer
		glDepthMask(GL_TRUE);
	}

	GLint isSkinnedMesh_UniLoc = glad_glGetUniformLocation(shaderProgID, "isSkinnedMesh");
	if (pCurrentObject->pSM != NULL)
	{
		glUniform1f(isSkinnedMesh_UniLoc, (float)GL_TRUE);
		
		// Copy all bones to the shader
		GLint matBonesArray_UniLoc = glGetUniformLocation(shaderProgID, "matBonesArray");
		GLint numBonesUsed = (GLint)vecFinalTransformation.size();
		if (numBonesUsed == 0)	return;		// Error check
		glUniformMatrix4fv(matBonesArray_UniLoc, numBonesUsed, GL_FALSE, glm::value_ptr(vecFinalTransformation[0]));

	}
	else
	{
		glUniform1f(isSkinnedMesh_UniLoc, (float)GL_FALSE);
	}
	
	if (shaderProgID == shadowDepthShaderProgID)
	{
		glCullFace(GL_FRONT);
	}

	GLint Instancing_UL = glGetUniformLocation(shaderProgID, "bInstancing");
	if (pCurrentObject->textureBlendMode == IMPOSTER && vecInstanceMatrices.size() != 0)
	{		
		glUniform1f(Instancing_UL, (float)GL_TRUE);		

		GLint matModelInstance_UL = glGetUniformLocation(shaderProgID, "matModelInstance");
		glUniformMatrix4fv(matModelInstance_UL, vecInstanceMatrices.size(), GL_FALSE, glm::value_ptr(vecInstanceMatrices[0]));

		GLint instanceTransparency_UL = glGetUniformLocation(shaderProgID, "instanceTransparency");
		
		glUniform1fv(instanceTransparency_UL, vecInstanceMatrices.size(), instanceAlpha);
	}
	else
	{
		glUniform1f(Instancing_UL, (float)GL_FALSE);
	}
	

	sModelDrawInfo drawInfo;
	if (pVAOManager->FindDrawInfoByModelName(pCurrentObject->meshName, drawInfo))
	{
		glBindVertexArray(drawInfo.VAO_ID);
		if (pCurrentObject->textureBlendMode == IMPOSTER && vecInstanceMatrices.size() != 0)
		{
			glDrawElementsInstanced(GL_TRIANGLES, drawInfo.numberOfIndices, GL_UNSIGNED_INT, 0, vecInstanceMatrices.size());
		}
		else
		{
			glDrawElements(GL_TRIANGLES, drawInfo.numberOfIndices, GL_UNSIGNED_INT, 0);
		}
		
		glBindVertexArray(0);
	}

	return;
}

