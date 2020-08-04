#include "cVAOManager.h"
#include "GLCommon.h"
#include <iostream>

cVAOManager::cVAOManager()
{
	std::cout << "A cVAOManager() is created." << std::endl;
	return;
}

cVAOManager::~cVAOManager()
{
	std::cout << "A cVAOManager() is destroyed." << std::endl;
	return;
}

sVertex_xyz_rgba_n_uv2_bt_4Bones::sVertex_xyz_rgba_n_uv2_bt_4Bones() :
	x(0.0f), y(0.0f), z(0.0f), w(1.0f),
	r(0.0f), g(0.0f), b(0.0f), a(1.0f),		// Note alpha is 1.0
	nx(0.0f), ny(0.0f), nz(0.0f), nw(1.0f),
	u0(0.0f), v0(0.0f), u1(0.0f), v1(0.0f),
	tx(0.0f), ty(0.0f), tz(0.0f), tw(1.0f),
	bx(0.0f), by(0.0f), bz(0.0f), bw(1.0f)
{
	//#ifdef _DEBUG
	memset(this->boneID, 0, sizeof(unsigned int) * NUMBEROFBONES);
	memset(this->boneWeights, 0, sizeof(float) * NUMBEROFBONES);

	//this->boneID[0] = 0;
	//this->boneID[1] = 0;
	//this->boneID[2] = 0;
	//this->boneID[3] = 0;

	return;
}

sModelDrawInfo::sModelDrawInfo()
{
	this->VAO_ID = 0;
	this->VertexBufferID = 0;
	this->VertexBuffer_Start_Index = 0;
	this->numberOfVertices = 0;
	this->IndexBufferID = 0;
	this->IndexBuffer_Start_Index = 0;
	this->numberOfIndices = 0;
	this->numberOfTriangles = 0;	
	this->pVertices = 0;
	this->pIndices = 0;
	this->maxX = this->maxY = this->maxZ = 0.0f;
	this->minX = this->minY = this->minZ = 0.0f;
	this->extentX = this->extentY = this->extentZ = this->maxExtent = 0.0f;
	return;
}

bool cVAOManager::LoadModelIntoVAO(
	std::string fileName,
	cMesh& theMesh,
	sModelDrawInfo& drawInfo,
	unsigned int shaderProgramID)

{
	// cMesh& theMesh to sModelDrawInfo& drawInfo
	drawInfo.numberOfVertices = (unsigned int)theMesh.vecVertices.size();
	drawInfo.pVertices = new sVertex_xyz_rgba_n_uv2_bt_4Bones[drawInfo.numberOfVertices];

	for (unsigned int index = 0; index != drawInfo.numberOfVertices; index++)
	{

		drawInfo.pVertices[index].x = theMesh.vecVertices[index].x;
		drawInfo.pVertices[index].y = theMesh.vecVertices[index].y;
		drawInfo.pVertices[index].z = theMesh.vecVertices[index].z;
		drawInfo.pVertices[index].w = 1.0f;

		drawInfo.pVertices[index].r = 1.0f;
		drawInfo.pVertices[index].g = 1.0f;
		drawInfo.pVertices[index].b = 1.0f;
		drawInfo.pVertices[index].a = 1.0f;

		drawInfo.pVertices[index].nx = theMesh.vecVertices[index].nx;
		drawInfo.pVertices[index].ny = theMesh.vecVertices[index].ny;
		drawInfo.pVertices[index].nz = theMesh.vecVertices[index].nz;
		drawInfo.pVertices[index].nw = 1.0f;
		
		drawInfo.pVertices[index].u0 = theMesh.vecVertices[index].u0;
		drawInfo.pVertices[index].v0 = theMesh.vecVertices[index].v0;
		drawInfo.pVertices[index].u1 = 1.0f;
		drawInfo.pVertices[index].v1 = 1.0f;

		drawInfo.pVertices[index].tx = theMesh.vecVertices[index].tx;
		drawInfo.pVertices[index].ty = theMesh.vecVertices[index].ty;
		drawInfo.pVertices[index].tz = theMesh.vecVertices[index].tz;
		drawInfo.pVertices[index].tw = 1.0f;

		drawInfo.pVertices[index].bx = theMesh.vecVertices[index].bx;
		drawInfo.pVertices[index].by = theMesh.vecVertices[index].by;
		drawInfo.pVertices[index].bz = theMesh.vecVertices[index].bz;
		drawInfo.pVertices[index].bw = 1.0f;
	}

	// copy the index information
	drawInfo.numberOfTriangles = (unsigned int)theMesh.vecTriangles.size();
	drawInfo.numberOfIndices = (unsigned int)theMesh.vecTriangles.size() * 3;

	drawInfo.pIndices = new unsigned int[drawInfo.numberOfIndices];

	unsigned int indexTri = 0;
	unsigned int indexIndex = 0;
	for (; indexTri != drawInfo.numberOfTriangles; indexTri++, indexIndex += 3)
	{
		drawInfo.pIndices[indexIndex + 0] = (unsigned int)theMesh.vecTriangles[indexTri].vert_index_1;
		drawInfo.pIndices[indexIndex + 1] = (unsigned int)theMesh.vecTriangles[indexTri].vert_index_2;
		drawInfo.pIndices[indexIndex + 2] = (unsigned int)theMesh.vecTriangles[indexTri].vert_index_3;
	}

	drawInfo.meshName = fileName;
	
	glGenVertexArrays(1, &(drawInfo.VAO_ID));		// new buffer ID	
	glBindVertexArray(drawInfo.VAO_ID);		// use this VAO buffer

	glGenBuffers(1, &(drawInfo.VertexBufferID));

	glBindBuffer(GL_ARRAY_BUFFER, drawInfo.VertexBufferID);

	glBufferData(GL_ARRAY_BUFFER,
		sizeof(sVertex_xyz_rgba_n_uv2_bt_4Bones) * drawInfo.numberOfVertices,
		(GLvoid*)drawInfo.pVertices,
		GL_STATIC_DRAW);

	// Copy the index buffer
	glGenBuffers(1, &(drawInfo.IndexBufferID));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, drawInfo.IndexBufferID);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		sizeof(unsigned int) * drawInfo.numberOfIndices,
		(GLvoid*)drawInfo.pIndices,
		GL_STATIC_DRAW);

	GLint vpos_location = glGetAttribLocation(shaderProgramID, "vPosition");
	GLint vcol_location = glGetAttribLocation(shaderProgramID, "vColour");
	GLint vnorm_location = glGetAttribLocation(shaderProgramID, "vNormal");
	GLint vUV_location = glGetAttribLocation(shaderProgramID, "vUVx2");
	GLint vTangent_location = glGetAttribLocation(shaderProgramID, "vTangent");
	GLint vBitangent_location = glGetAttribLocation(shaderProgramID, "vBitangent");

	// Set the vertex attributes for this shader
	glEnableVertexAttribArray(vpos_location);
	glVertexAttribPointer(vpos_location, 4,
		GL_FLOAT, GL_FALSE,
		sizeof(sVertex_xyz_rgba_n_uv2_bt_4Bones),
		(void*)(offsetof(sVertex_xyz_rgba_n_uv2_bt_4Bones, x)));

	glEnableVertexAttribArray(vcol_location);
	glVertexAttribPointer(vcol_location, 4,
		GL_FLOAT, GL_FALSE,
		sizeof(sVertex_xyz_rgba_n_uv2_bt_4Bones),
		(void*)(offsetof(sVertex_xyz_rgba_n_uv2_bt_4Bones, r)));

	//	float nx, ny, nz, nw;
	glEnableVertexAttribArray(vnorm_location);
	glVertexAttribPointer(vnorm_location, 4,
		GL_FLOAT, GL_FALSE,
		sizeof(sVertex_xyz_rgba_n_uv2_bt_4Bones),
		(void*)(offsetof(sVertex_xyz_rgba_n_uv2_bt_4Bones, nx)));

	//	float u0, v0, u1, v1;
	glEnableVertexAttribArray(vUV_location);
	glVertexAttribPointer(vUV_location, 4,
		GL_FLOAT, GL_FALSE,
		sizeof(sVertex_xyz_rgba_n_uv2_bt_4Bones),
		(void*)(offsetof(sVertex_xyz_rgba_n_uv2_bt_4Bones, u0)));

	glEnableVertexAttribArray(vTangent_location);
	glVertexAttribPointer(vTangent_location, 4,
		GL_FLOAT, GL_FALSE,
		sizeof(sVertex_xyz_rgba_n_uv2_bt_4Bones),
		(void*)(offsetof(sVertex_xyz_rgba_n_uv2_bt_4Bones, tx)));

	glEnableVertexAttribArray(vBitangent_location);
	glVertexAttribPointer(vBitangent_location, 4,
		GL_FLOAT, GL_FALSE,
		sizeof(sVertex_xyz_rgba_n_uv2_bt_4Bones),
		(void*)(offsetof(sVertex_xyz_rgba_n_uv2_bt_4Bones, bx)));

	// Now that all the parts are set up, set the VAO to zero
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDisableVertexAttribArray(vpos_location);
	glDisableVertexAttribArray(vcol_location);	
	glDisableVertexAttribArray(vnorm_location);
	glDisableVertexAttribArray(vUV_location);	
	glDisableVertexAttribArray(vTangent_location);
	glDisableVertexAttribArray(vBitangent_location);

	// Store the draw information into the map
	this->m_map_ModelName_to_VAOID[drawInfo.meshName] = drawInfo;

	return true;
}


bool cVAOManager::LoadModelDrawInfoIntoVAO(sModelDrawInfo& drawInfo,
	unsigned int shaderProgramID)
{
	// Ask OpenGL for a new buffer ID...
	glGenVertexArrays(1, &(drawInfo.VAO_ID));
	// "Bind" this buffer:
	// - aka "make this the 'current' VAO buffer
	glBindVertexArray(drawInfo.VAO_ID);

	// Now ANY state that is related to vertex or index buffer
	//	and vertex attribute layout, is stored in the 'state' 
	//	of the VAO... 


	// NOTE: OpenGL error checks have been omitted for brevity
	//	glGenBuffers(1, &vertex_buffer);
	glGenBuffers(1, &(drawInfo.VertexBufferID));

	//	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, drawInfo.VertexBufferID);
	// sVert vertices[3]
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(sVertex_xyz_rgba_n_uv2_bt_4Bones) * drawInfo.numberOfVertices,	// ::g_NumberOfVertsToDraw,	// sizeof(vertices), 
		(GLvoid*)drawInfo.pVertices,							// pVertices,			//vertices, 
		GL_DYNAMIC_DRAW);
	// GL_STATIC_DRAW );


// Copy the index buffer into the video card, too
// Create an index buffer.
	glGenBuffers(1, &(drawInfo.IndexBufferID));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, drawInfo.IndexBufferID);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER,			// Type: Index element array
		sizeof(unsigned int) * drawInfo.numberOfIndices,
		(GLvoid*)drawInfo.pIndices,
		GL_STATIC_DRAW);

	// Set the vertex attributes.

	
	GLint vpos_location = glGetAttribLocation(shaderProgramID, "vPosition");	// program
	GLint vcol_location = glGetAttribLocation(shaderProgramID, "vColour");	// program;
	GLint vnorm_location = glGetAttribLocation(shaderProgramID, "vNormal");	// program;
	GLint vUV_location = glGetAttribLocation(shaderProgramID, "vUVx2");	// program;	
	GLint vTangent_location = glGetAttribLocation(shaderProgramID, "vTangent");
	GLint vBitangent_location = glGetAttribLocation(shaderProgramID, "vBitangent");
	GLint vBoneID_location = glGetAttribLocation(shaderProgramID, "vBoneID");
	GLint vBoneWeight_location = glGetAttribLocation(shaderProgramID, "vBoneWeight");


	// Set the vertex attributes for this shader
	glEnableVertexAttribArray(vpos_location);	// vPos
	glVertexAttribPointer(vpos_location, 4,		// now a vec4
		GL_FLOAT, GL_FALSE,
		sizeof(sVertex_xyz_rgba_n_uv2_bt_4Bones),						// sizeof(float) * 6,
		(void*)(offsetof(sVertex_xyz_rgba_n_uv2_bt_4Bones, x)));

	glEnableVertexAttribArray(vcol_location);	// vCol
	glVertexAttribPointer(vcol_location, 4,		// vCol
		GL_FLOAT, GL_FALSE,
		sizeof(sVertex_xyz_rgba_n_uv2_bt_4Bones),
		(void*)(offsetof(sVertex_xyz_rgba_n_uv2_bt_4Bones, r)));


	//	float nx, ny, nz, nw;
	glEnableVertexAttribArray(vnorm_location);	// vNormal
	glVertexAttribPointer(vnorm_location, 4,		// vNormal
		GL_FLOAT, GL_FALSE,
		sizeof(sVertex_xyz_rgba_n_uv2_bt_4Bones),
		(void*)(offsetof(sVertex_xyz_rgba_n_uv2_bt_4Bones, nx)));

	//	float u0, v0, u1, v1;
	glEnableVertexAttribArray(vUV_location);		// vUVx2
	glVertexAttribPointer(vUV_location, 4,		// vUVx2
		GL_FLOAT, GL_FALSE,
		sizeof(sVertex_xyz_rgba_n_uv2_bt_4Bones),
		(void*)(offsetof(sVertex_xyz_rgba_n_uv2_bt_4Bones, u0)));

	// New stuff added in animation (for bump mapping and skinned mesh)
	glEnableVertexAttribArray(vTangent_location);
	glVertexAttribPointer(vTangent_location, 4,
		GL_FLOAT, GL_FALSE,
		sizeof(sVertex_xyz_rgba_n_uv2_bt_4Bones),
		(void*)(offsetof(sVertex_xyz_rgba_n_uv2_bt_4Bones, tx)));

	glEnableVertexAttribArray(vBitangent_location);
	glVertexAttribPointer(vBitangent_location, 4,
		GL_FLOAT, GL_FALSE,
		sizeof(sVertex_xyz_rgba_n_uv2_bt_4Bones),
		(void*)(offsetof(sVertex_xyz_rgba_n_uv2_bt_4Bones, bx)));

	glEnableVertexAttribArray(vBoneID_location);
	glVertexAttribPointer(vBoneID_location, 4,
		GL_FLOAT, GL_FALSE,
		sizeof(sVertex_xyz_rgba_n_uv2_bt_4Bones),
		(void*)(offsetof(sVertex_xyz_rgba_n_uv2_bt_4Bones, boneID[0])));

	glEnableVertexAttribArray(vBoneWeight_location);
	glVertexAttribPointer(vBoneWeight_location, 4,
		GL_FLOAT, GL_FALSE,
		sizeof(sVertex_xyz_rgba_n_uv2_bt_4Bones),
		(void*)(offsetof(sVertex_xyz_rgba_n_uv2_bt_4Bones, boneWeights[0])));

	// Now that all the parts are set up, set the VAO to zero
	// At this point, whatever state the:
	// - vertex buffer (VBO)
	// - index buffer
	// - vertex layout 
	// ...is "remembered"
	glBindVertexArray(0);
	// Now, that VAO is "not active" (bound), so OpenGL is 
	//  not "pay attention" to any changes to any of those 3 things


	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDisableVertexAttribArray(vpos_location);
	glDisableVertexAttribArray(vcol_location);
	glDisableVertexAttribArray(vnorm_location);
	glDisableVertexAttribArray(vUV_location);
	// And the other ones
	glDisableVertexAttribArray(vTangent_location);
	glDisableVertexAttribArray(vBitangent_location);
	glDisableVertexAttribArray(vBoneID_location);
	glDisableVertexAttribArray(vBoneWeight_location);

	// Store the draw information into the map
	this->m_map_ModelName_to_VAOID[drawInfo.meshName] = drawInfo;

	return true;
}


bool cVAOManager::FindDrawInfoByModelName(std::string filename,	sModelDrawInfo& drawInfo)
{
	std::map< std::string /*model name*/, sModelDrawInfo >::iterator
		itDrawInfo = this->m_map_ModelName_to_VAOID.find(filename);

	if (itDrawInfo == this->m_map_ModelName_to_VAOID.end())
	{		
		return false;
	}

	drawInfo = itDrawInfo->second;
	return true;
}

void sModelDrawInfo::CalcExtents(void)
{
	// See if we have any vertices loaded...
	if ((this->numberOfVertices == 0) || (this->pVertices == 0))
	{
		return;
	}

	// We're good

	// Assume the 1st vertex is the max and min (so we can compare)
	this->minX = this->maxX = this->pVertices[0].x;
	this->minY = this->maxY = this->pVertices[0].y;
	this->minZ = this->maxZ = this->pVertices[0].z;

	for (unsigned int index = 0; index != this->numberOfVertices; index++)
	{
		if (this->pVertices[index].x < this->minX) { this->minX = this->pVertices[index].x; }
		if (this->pVertices[index].y < this->minY) { this->minY = this->pVertices[index].y; }
		if (this->pVertices[index].z < this->minZ) { this->minZ = this->pVertices[index].z; }

		if (this->pVertices[index].x > this->maxX) { this->maxX = this->pVertices[index].x; }
		if (this->pVertices[index].y > this->maxY) { this->maxY = this->pVertices[index].y; }
		if (this->pVertices[index].z > this->maxZ) { this->maxZ = this->pVertices[index].z; }
	}

	this->extentX = this->maxX - this->minX;
	this->extentY = this->maxY - this->minY;
	this->extentZ = this->maxZ - this->minZ;

	this->maxExtent = this->extentX;
	if (this->extentY > this->maxExtent) { this->maxExtent = this->extentY; }
	if (this->extentZ > this->maxExtent) { this->maxExtent = this->extentZ; }

	return;
}
