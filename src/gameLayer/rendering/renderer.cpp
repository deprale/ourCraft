#include "rendering/renderer.h"
#include <ctime>
#include "blocks.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat3x3.hpp>
#include <glm/gtx/transform.hpp>
#include <blocksLoader.h>
#include <chunkSystem.h>
#include <gamePlayLogic.h>
#include <iostream>
#include <rendering/sunShadow.h>
#include <platformTools.h>
#include <gameplay/entityManagerClient.h>
#include <rendering/frustumCulling.h>
#include <rendering/model.h>
#include <glm/gtx/quaternion.hpp>

#define GET_UNIFORM(s, n) n = s.getUniform(#n);
#define GET_UNIFORM2(s, n) s. n = s.shader.getUniform(#n);



float vertexDataOriginal[] = {
	//front
	0.5, 0.5, 0.5,
	-0.5, 0.5, 0.5,
	-0.5, -0.5, 0.5,
	-0.5, -0.5, 0.5,
	0.5, -0.5, 0.5,
	0.5, 0.5, 0.5,

	//back
	-0.5, -0.5, -0.5,
	-0.5, 0.5, -0.5,
	0.5, 0.5, -0.5,
	0.5, 0.5, -0.5,
	0.5, -0.5, -0.5,
	-0.5, -0.5, -0.5,

	//top
	-0.5, 0.5, -0.5,
	-0.5, 0.5, 0.5,
	0.5, 0.5, 0.5,
	0.5, 0.5, 0.5,
	0.5, 0.5, -0.5,
	-0.5, 0.5, -0.5,

	//bottom
	0.5, -0.5, 0.5,
	-0.5, -0.5, 0.5,
	-0.5, -0.5, -0.5,
	-0.5, -0.5, -0.5,
	0.5, -0.5, -0.5,
	0.5, -0.5, 0.5,

	//left
	-0.5, -0.5, 0.5,
	-0.5, 0.5, 0.5,
	-0.5, 0.5, -0.5,
	-0.5, 0.5, -0.5,
	-0.5, -0.5, -0.5,
	-0.5, -0.5, 0.5,

	//right
	0.5, 0.5, -0.5,
	0.5, 0.5, 0.5,
	0.5, -0.5, 0.5,
	0.5, -0.5, 0.5,
	0.5, -0.5, -0.5,
	0.5, 0.5, -0.5,
};

float vertexUVOriginal[] = {
	//front
	1, 1,
	0, 1,
	0, 0,
	0, 0,
	1, 0,
	1, 1,

	//back
	0, 0,
	0, 1,
	1, 1,
	1, 1,
	1, 0,
	0, 0,

	//top
	1, 1,
	0, 1,
	0, 0,
	0, 0,
	1, 0,
	1, 1,

	//bottom
	1, 1,
	0, 1,
	0, 0,
	0, 0,
	1, 0,
	1, 1,

	//left
	1, 0,
	1, 1,
	0, 1,
	0, 1,
	0, 0,
	1, 0,

	//right
	1, 1,
	0, 1,
	0, 0,
	0, 0,
	1, 0,
	1, 1,
};

float vertexData[] = {
	//front
	0.5, 0.5, 0.5,
	-0.5, 0.5, 0.5,
	-0.5, -0.5, 0.5,
	0.5, -0.5, 0.5,

	//back
	-0.5, -0.5, -0.5,
	-0.5, 0.5, -0.5,
	0.5, 0.5, -0.5,
	0.5, -0.5, -0.5,

	//top
	-0.5, 0.5, -0.5,
	-0.5, 0.5, 0.5,
	0.5, 0.5, 0.5,
	0.5, 0.5, -0.5,

	//bottom
	0.5, -0.5, 0.5,
	-0.5, -0.5, 0.5,
	-0.5, -0.5, -0.5,
	0.5, -0.5, -0.5,

	//left
	-0.5, -0.5, 0.5,
	-0.5, 0.5, 0.5,
	-0.5, 0.5, -0.5,
	-0.5, -0.5, -0.5,

	//right
	0.5, 0.5, -0.5,
	0.5, 0.5, 0.5,
	0.5, -0.5, 0.5,
	0.5, -0.5, -0.5,

	//grass
	0.5, 0.5, 0.5,
	-0.5, 0.5, -0.5,
	-0.5, -0.5, -0.5,
	0.5, -0.5, 0.5,

	-0.5, -0.5, -0.5,
	-0.5, 0.5, -0.5,
	0.5, 0.5, 0.5,
	0.5, -0.5, 0.5,

	-0.5, 0.5, 0.5,
	0.5, 0.5, -0.5,
	0.5, -0.5, -0.5,
	-0.5, -0.5, 0.5,

	-0.5, -0.5, 0.5,
	0.5, -0.5, -0.5,
	0.5, 0.5, -0.5,
	-0.5, 0.5, 0.5,
	

	//moving leaves
	//front
	0.5, 0.5, 0.5,
	-0.5, 0.5, 0.5,
	-0.5, -0.5, 0.5,
	0.5, -0.5, 0.5,

	//back
	-0.5, -0.5, -0.5,
	-0.5, 0.5, -0.5,
	0.5, 0.5, -0.5,
	0.5, -0.5, -0.5,

	//top
	-0.5, 0.5, -0.5,
	-0.5, 0.5, 0.5,
	0.5, 0.5, 0.5,
	0.5, 0.5, -0.5,

	//bottom
	0.5, -0.5, 0.5,
	-0.5, -0.5, 0.5,
	-0.5, -0.5, -0.5,
	0.5, -0.5, -0.5,

	//left
	-0.5, -0.5, 0.5,
	-0.5, 0.5, 0.5,
	-0.5, 0.5, -0.5,
	-0.5, -0.5, -0.5,

	//right
	0.5, 0.5, -0.5,
	0.5, 0.5, 0.5,
	0.5, -0.5, 0.5,
	0.5, -0.5, -0.5,


	//torch
	//front
	0.0625, 0.125, 0.0625,
	-0.0625, 0.125, 0.0625,
	-0.0625, -0.5, 0.0625,
	0.0625, -0.5, 0.0625,

	//back
	-0.0625, -0.5, -0.0625,
	-0.0625, 0.125, -0.0625,
	0.0625, 0.125, -0.0625,
	0.0625, -0.5, -0.0625,

	//top
	-0.0625, 0.125, -0.0625,
	-0.0625, 0.125, 0.0625,
	0.0625, 0.125, 0.0625,
	0.0625, 0.125, -0.0625,

	//bottom
	0.0625, -0.5, 0.0625,
	-0.0625, -0.5, 0.0625,
	-0.0625, -0.5, -0.0625,
	0.0625, -0.5, -0.0625,

	//left
	-0.0625, -0.5, 0.0625,
	-0.0625, 0.125, 0.0625,
	-0.0625, 0.125, -0.0625,
	-0.0625, -0.5, -0.0625,

	//right
	0.0625, 0.125, -0.0625,
	0.0625, 0.125, 0.0625,
	0.0625, -0.5, 0.0625,
	0.0625, -0.5, -0.0625,

	//water
	//front
	0.5, 0.375, 0.5,
	-0.5, 0.375, 0.5,
	-0.5, -0.5, 0.5,
	0.5, -0.5, 0.5,

	//back
	-0.5, -0.5, -0.5,
	-0.5, 0.375, -0.5,
	0.5, 0.375, -0.5,
	0.5, -0.5, -0.5,

	//top
	-0.5, 0.375, -0.5,
	-0.5, 0.375, 0.5,
	0.5, 0.375, 0.5,
	0.5, 0.375, -0.5,

	//bottom
	0.5, -0.5, 0.5,
	-0.5, -0.5, 0.5,
	-0.5, -0.5, -0.5,
	0.5, -0.5, -0.5,

	//left
	-0.5, -0.5, 0.5,
	-0.5, 0.375, 0.5,
	-0.5, 0.375, -0.5,
	-0.5, -0.5, -0.5,

	//right
	0.5, 0.375, -0.5,
	0.5, 0.375, 0.5,
	0.5, -0.5, 0.5,
	0.5, -0.5, -0.5,
};

float vertexUV[] = {
	//front
	1, 1,
	0, 1,
	0, 0,
	1, 0,

	//back
	0, 0,
	0, 1,
	1, 1,
	1, 0,

	//top
	1, 1,
	0, 1,
	0, 0,
	1, 0,

	//bottom
	1, 1,
	0, 1,
	0, 0,
	1, 0,

	//left
	1, 0,
	1, 1,
	0, 1,
	0, 0,

	//right
	1, 1,
	0, 1,
	0, 0,
	1, 0,


	//grass
	//front
	1, 1,
	0, 1,
	0, 0,
	1, 0,

	0, 0,
	0, 1,
	1, 1,
	1, 0,

	1, 1,
	0, 1,
	0, 0,
	1, 0,

	1, 0,
	0, 0,
	0, 1,
	1, 1,


	//leaves////////////////////////
	//front
	1, 1,
	0, 1,
	0, 0,
	1, 0,

	//back
	0, 0,
	0, 1,
	1, 1,
	1, 0,

	//top
	1, 1,
	0, 1,
	0, 0,
	1, 0,

	//bottom
	1, 1,
	0, 1,
	0, 0,
	1, 0,

	//left
	1, 0,
	1, 1,
	0, 1,
	0, 0,

	//right
	1, 1,
	0, 1,
	0, 0,
	1, 0,


	//torches ///////
	//front
	0.5625, 0.625,
	0.4375, 0.625,
	0.4375, 0,
	0.5625, 0,

	//back
	0.4375, 0,
	0.4375, 0.625,
	0.5625, 0.625,
	0.5625, 0,

	//top
	0.5625, 0.625,
	0.4375, 0.625,
	0.4375, 0.5,
	0.5625, 0.5,

	//bottom
	0.5625, 0.125,
	0.4375, 0.125,
	0.4375, 0,
	0.5625, 0,

	//left
	0.5625, 0,
	0.5625, 0.625,
	0.4375, 0.625,
	0.4375, 0,

	//right
	0.5625, 0.625,
	0.4375, 0.625,
	0.4375, 0,
	0.5625, 0,

	//water
	//front
	1, 0.875,
	0, 0.875,
	0, 0,
	1, 0,

	//back
	0, 0,
	0, 0.875,
	1, 0.875,
	1, 0,

	//top
	1, 1,
	0, 1,
	0, 0,
	1, 0,

	//bottom
	1, 1,
	0, 1,
	0, 0,
	1, 0,

	//left
	1, 0,
	1, 0.875,
	0, 0.875,
	0, 0,

	//right
	1, 0.875,
	0, 0.875,
	0, 0,
	1, 0,
};


constexpr float uv = 1;
constexpr float cubeEntityData[] = {
	-0.5f, +0.5f, +0.5f, // 0
	+0.0f, +1.0f, +0.0f, // Normal
	0, 0,				 //uv

	+0.5f, +0.5f, +0.5f, // 1
	+0.0f, +1.0f, +0.0f, // Normal
	1 * uv, 0,				 //uv

	+0.5f, +0.5f, -0.5f, // 2
	+0.0f, +1.0f, +0.0f, // Normal
	1 * uv, 1 * uv,				 //uv

	-0.5f, +0.5f, -0.5f, // 3
	+0.0f, +1.0f, +0.0f, // Normal
	0, 1 * uv,				 //uv

	-0.5f, +0.5f, -0.5f, // 4
	 0.0f, +0.0f, -1.0f, // Normal
	 0, 1 * uv,				 //uv

	 +0.5f, +0.5f, -0.5f, // 5
	  0.0f, +0.0f, -1.0f, // Normal
	  1 * uv, 1 * uv,				 //uv

	   +0.5f, -0.5f, -0.5f, // 6
	   0.0f, +0.0f, -1.0f, // Normal
	   1 * uv, 0,				 //uv

	  -0.5f, -0.5f, -0.5f, // 7
	   0.0f, +0.0f, -1.0f, // Normal
	   0, 0,				 //uv

	   +0.5f, +0.5f, -0.5f, // 8
	   +1.0f, +0.0f, +0.0f, // Normal
	   1 * uv, 0,				 //uv

	   +0.5f, +0.5f, +0.5f, // 9
	   +1.0f, +0.0f, +0.0f, // Normal
	   1 * uv, 1 * uv,				 //uv

	   +0.5f, -0.5f, +0.5f, // 10
	   +1.0f, +0.0f, +0.0f, // Normal
	   0, 1 * uv,				 //uv

	   +0.5f, -0.5f, -0.5f, // 11
	   +1.0f, +0.0f, +0.0f, // Normal
	   0, 0,				 //uv

	   -0.5f, +0.5f, +0.5f, // 12
	   -1.0f, +0.0f, +0.0f, // Normal
	   1 * uv, 1 * uv,				 //uv

	   -0.5f, +0.5f, -0.5f, // 13
	   -1.0f, +0.0f, +0.0f, // Normal
	   1 * uv, 0,				 //uv

	   -0.5f, -0.5f, -0.5f, // 14
	   -1.0f, +0.0f, +0.0f, // Normal
	   0, 0,				 //uv

	   -0.5f, -0.5f, +0.5f, // 15
	   -1.0f, +0.0f, +0.0f, // Normal
	   0, 1 * uv,				 //uv

	   +0.5f, +0.5f, +0.5f, // 16
	   +0.0f, +0.0f, +1.0f, // Normal
	   1 * uv, 1 * uv,				 //uv

	   -0.5f, +0.5f, +0.5f, // 17
	   +0.0f, +0.0f, +1.0f, // Normal
	   0, 1 * uv,				 //uv

	   -0.5f, -0.5f, +0.5f, // 18
	   +0.0f, +0.0f, +1.0f, // Normal
	   0, 0,				 //uv

	   +0.5f, -0.5f, +0.5f, // 19
	   +0.0f, +0.0f, +1.0f, // Normal
	   1 * uv, 0,				 //uv


	   +0.5f, -0.5f, -0.5f, // 20
	   +0.0f, -1.0f, +0.0f, // Normal
	   1 * uv, 0,				 //uv

	   -0.5f, -0.5f, -0.5f, // 21
	   +0.0f, -1.0f, +0.0f, // Normal
	   0, 0,				 //uv

	   -0.5f, -0.5f, +0.5f, // 22
	   +0.0f, -1.0f, +0.0f, // Normal
	   0, 1 * uv,				 //uv

	   +0.5f, -0.5f, +0.5f, // 23
	   +0.0f, -1.0f, +0.0f, // Normal
	   1 * uv, 1 * uv,				 //uv

};


unsigned int cubeEntityIndices[] = {
16, 17, 18, 16, 18, 19, // Front
4,   5,  6,  4,  6,  7, // Back
0,   1,  2,  0,  2,  3, // Top
20, 22, 21, 20, 23, 22, // Bottom
12, 13, 14, 12, 14, 15, // Left
8,   9, 10,  8, 10, 11, // Right
};


void Renderer::create(BlocksLoader &blocksLoader)
{

	fboCoppy.create(true, true);
	fboMain.create(true, true, GL_RGB16F, GL_RGB16UI);
	glBindTexture(GL_TEXTURE_2D, fboMain.secondaryColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	fboLastFrame.create(true, false);
	fboLastFramePositions.create(GL_RGB16F, false);
	fboHBAO.create(GL_RED, false);
	fboSkyBox.create(true, false);

	skyBoxRenderer.create();
	skyBoxLoaderAndDrawer.createGpuData();
	//skyBoxLoaderAndDrawer.loadTexture(RESOURCES_PATH "sky/skybox.png", defaultSkyBox);
	//skyBoxLoaderAndDrawer.loadTexture(RESOURCES_PATH "sky/nightsky.png", defaultSkyBox);
	skyBoxLoaderAndDrawer.loadTexture(RESOURCES_PATH "sky/twilightsky.png", defaultSkyBox);
	sunTexture.loadFromFile(RESOURCES_PATH "sky/sun.png", false, false);

	brdfTexture.loadFromFile(RESOURCES_PATH "otherTextures/brdf.png", false, false);

	{
		glGenVertexArrays(1, &vaoQuad);
		glBindVertexArray(vaoQuad);
		glGenBuffers(1, &vertexBufferQuad);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferQuad);

		//how to use
		//glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};

		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));

		glBindVertexArray(0);
	};


	glGenBuffers(1, &defaultShader.shadingSettingsBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, defaultShader.shadingSettingsBuffer);
	

	reloadShaders();

	
	
	glGenBuffers(1, &vertexDataBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertexDataBuffer);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(vertexData), vertexData, GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, vertexDataBuffer);

	
	glGenBuffers(1, &vertexUVBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertexUVBuffer);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(vertexUV), vertexUV, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, vertexUVBuffer);

	glGenBuffers(1, &textureSamplerersBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, textureSamplerersBuffer);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint64) * blocksLoader.gpuIds.size(), blocksLoader.gpuIds.data(), 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, textureSamplerersBuffer);
	

	
	glGenBuffers(1, &lightBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	
	//todo remove?
	//not in use anymore?
	glCreateBuffers(1, &vertexBuffer);
	//glNamedBufferData(vertexBuffer, sizeof(data), data, GL_DYNAMIC_DRAW);

	glCreateVertexArrays(1, &vao);
	glBindVertexArray(vao);
		
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

		glEnableVertexAttribArray(0);
		glVertexAttribIPointer(0, 1, GL_SHORT, 4 * sizeof(int), 0);
		glVertexAttribDivisor(0, 1);

		glEnableVertexAttribArray(1);
		glVertexAttribIPointer(1, 1, GL_SHORT, 4 * sizeof(int), (void *)(1 * sizeof(short)));
		glVertexAttribDivisor(1, 1);

		glEnableVertexAttribArray(2);
		glVertexAttribIPointer(2, 3, GL_INT, 4 * sizeof(int), (void *)(1 * sizeof(int)));
		glVertexAttribDivisor(2, 1);
		
		//glEnableVertexAttribArray(3);
		//glVertexAttribIPointer(3, 1, GL_INT, 5 * sizeof(int), (void *)(4 * sizeof(int)));
		//glVertexAttribDivisor(3, 1);
		//
		//glEnableVertexAttribArray(4);
		//glVertexAttribIPointer(4, 1, GL_INT, 6 * sizeof(int), (void *)(5 * sizeof(int)));
		//glVertexAttribDivisor(4, 1);

	glBindVertexArray(0);

	glGenBuffers(1, &drawCommandsOpaqueBuffer);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, drawCommandsOpaqueBuffer);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);



#pragma region basic entity renderer

	//GLuint vaoCube = 0;
	//GLuint vertexBufferCube = 0;
	//GLuint indexBufferCube = 0;

	glCreateVertexArrays(1, &entityRenderer.blockEntityshader.vaoCube);
	glBindVertexArray(entityRenderer.blockEntityshader.vaoCube);

	glGenBuffers(1, &entityRenderer.blockEntityshader.vertexBufferCube);
	glBindBuffer(GL_ARRAY_BUFFER, entityRenderer.blockEntityshader.vertexBufferCube);

	//glBufferData(GL_ARRAY_BUFFER, sizeof(cubeEntityData), cubeEntityData, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));

	std::vector<float> data;
	for (int face = 0; face < 6; face++)
	{
		for (int i = 0; i < 6; i++)
		{
			
			data.push_back(vertexDataOriginal[i * 3 + face * 3 * 6 + 0]);
			data.push_back(vertexDataOriginal[i * 3 + face * 3 * 6 + 1]);
			data.push_back(vertexDataOriginal[i * 3 + face * 3 * 6 + 2]);

			//todo normal
			data.push_back(0);
			data.push_back(1);
			data.push_back(0);

			data.push_back(vertexUVOriginal[i * 2 + face * 2 * 6 + 0]);
			data.push_back(vertexUVOriginal[i * 2 + face * 2 * 6 + 1]);

		}
	}

	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * data.size(), data.data(), GL_STATIC_DRAW);



	//glGenBuffers(1, &entityRenderer.basicEntityshader.indexBufferCube);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, entityRenderer.basicEntityshader.indexBufferCube);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeEntityIndices), cubeEntityIndices, GL_STATIC_DRAW);

	glBindVertexArray(0);



	glGenBuffers(1, &skinningMatrixSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, skinningMatrixSSBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, skinningMatrixSSBO);
	

	glGenBuffers(1, &perEntityDataSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, perEntityDataSSBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, perEntityDataSSBO);

#pragma endregion




}

void Renderer::reloadShaders()
{

	defaultShader.shader.clear();

	defaultShader.shader.loadShaderProgramFromFile(RESOURCES_PATH "defaultShader.vert", RESOURCES_PATH "defaultShader.frag");
	defaultShader.shader.bind();

	GET_UNIFORM2(defaultShader, u_viewProjection);
	GET_UNIFORM2(defaultShader, u_typesCount);
	GET_UNIFORM2(defaultShader, u_positionInt);
	GET_UNIFORM2(defaultShader, u_positionFloat);
	GET_UNIFORM2(defaultShader, u_texture);
	GET_UNIFORM2(defaultShader, u_time);
	GET_UNIFORM2(defaultShader, u_showLightLevels);
	GET_UNIFORM2(defaultShader, u_skyLightIntensity);
	GET_UNIFORM2(defaultShader, u_lightsCount);
	GET_UNIFORM2(defaultShader, u_pointPosF);
	GET_UNIFORM2(defaultShader, u_pointPosI);
	GET_UNIFORM2(defaultShader, u_sunDirection);
	GET_UNIFORM2(defaultShader, u_metallic);
	GET_UNIFORM2(defaultShader, u_roughness);
	GET_UNIFORM2(defaultShader, u_underWater);
	GET_UNIFORM2(defaultShader, u_waterColor);
	GET_UNIFORM2(defaultShader, u_depthPeelwaterPass);
	GET_UNIFORM2(defaultShader, u_depthTexture);
	GET_UNIFORM2(defaultShader, u_hasPeelInformation);
	GET_UNIFORM2(defaultShader, u_PeelTexture);
	GET_UNIFORM2(defaultShader, u_dudv);
	GET_UNIFORM2(defaultShader, u_dudvNormal);
	GET_UNIFORM2(defaultShader, u_waterMove);
	GET_UNIFORM2(defaultShader, u_near);
	GET_UNIFORM2(defaultShader, u_far);
	GET_UNIFORM2(defaultShader, u_caustics);
	GET_UNIFORM2(defaultShader, u_inverseProjMat);
	GET_UNIFORM2(defaultShader, u_lightSpaceMatrix);
	GET_UNIFORM2(defaultShader, u_lightPos);
	GET_UNIFORM2(defaultShader, u_sunShadowTexture);
	GET_UNIFORM2(defaultShader, u_timeGrass);
	GET_UNIFORM2(defaultShader, u_writeScreenSpacePositions);
	GET_UNIFORM2(defaultShader, u_lastFrameColor);
	GET_UNIFORM2(defaultShader, u_lastFramePositionViewSpace);
	GET_UNIFORM2(defaultShader, u_cameraProjection);
	GET_UNIFORM2(defaultShader, u_inverseView);
	GET_UNIFORM2(defaultShader, u_view);
	GET_UNIFORM2(defaultShader, u_brdf);
	GET_UNIFORM2(defaultShader, u_inverseViewProjMat);
	GET_UNIFORM2(defaultShader, u_lastViewProj);
	GET_UNIFORM2(defaultShader, u_skyTexture);
	

	defaultShader.u_shadingSettings
		= glGetUniformBlockIndex(defaultShader.shader.id, "ShadingSettings");
	glBindBufferBase(GL_UNIFORM_BUFFER, 
		defaultShader.u_shadingSettings, defaultShader.shadingSettingsBuffer);

	//todo enum for all samplers
	defaultShader.u_vertexData = getStorageBlockIndex(defaultShader.shader.id, "u_vertexData");
	glShaderStorageBlockBinding(defaultShader.shader.id, defaultShader.u_vertexData, 1);

	defaultShader.u_vertexUV = getStorageBlockIndex(defaultShader.shader.id, "u_vertexUV");
	glShaderStorageBlockBinding(defaultShader.shader.id, defaultShader.u_vertexUV, 2);

	defaultShader.u_textureSamplerers = getStorageBlockIndex(defaultShader.shader.id, "u_textureSamplerers");
	glShaderStorageBlockBinding(defaultShader.shader.id, defaultShader.u_textureSamplerers, 3);

	defaultShader.u_lights = getStorageBlockIndex(defaultShader.shader.id, "u_lights");
	glShaderStorageBlockBinding(defaultShader.shader.id, defaultShader.u_lights, 4);

#pragma region zpass
	{

		zpassShader.shader.clear();

		zpassShader.shader.loadShaderProgramFromFile(RESOURCES_PATH "zpass.vert", RESOURCES_PATH "zpass.frag");
		zpassShader.shader.bind();

		GET_UNIFORM2(zpassShader, u_viewProjection);
		GET_UNIFORM2(zpassShader, u_positionInt);
		GET_UNIFORM2(zpassShader, u_positionFloat);
		GET_UNIFORM2(zpassShader, u_renderOnlyWater);
		GET_UNIFORM2(zpassShader, u_timeGrass);

		zpassShader.u_vertexData = getStorageBlockIndex(zpassShader.shader.id, "u_vertexData");
		glShaderStorageBlockBinding(zpassShader.shader.id, zpassShader.u_vertexData, 1);

		zpassShader.u_vertexUV = getStorageBlockIndex(zpassShader.shader.id, "u_vertexUV");
		glShaderStorageBlockBinding(zpassShader.shader.id, zpassShader.u_vertexUV, 2);

		zpassShader.u_textureSamplerers = getStorageBlockIndex(zpassShader.shader.id, "u_textureSamplerers");
		glShaderStorageBlockBinding(zpassShader.shader.id, zpassShader.u_textureSamplerers, 3);

	}
#pragma endregion

#pragma region basic entity renderer

	entityRenderer.blockEntityshader.shader.clear();

	entityRenderer.blockEntityshader.shader.loadShaderProgramFromFile
	(RESOURCES_PATH "shaders/blockEntity.vert", RESOURCES_PATH "shaders/blockEntity.frag");
	entityRenderer.blockEntityshader.shader.bind();

	GET_UNIFORM2(entityRenderer.blockEntityshader, u_entityPositionInt);
	GET_UNIFORM2(entityRenderer.blockEntityshader, u_entityPositionFloat);
	GET_UNIFORM2(entityRenderer.blockEntityshader, u_viewProjection);
	GET_UNIFORM2(entityRenderer.blockEntityshader, u_modelMatrix);
	GET_UNIFORM2(entityRenderer.blockEntityshader, u_cameraPositionInt);
	GET_UNIFORM2(entityRenderer.blockEntityshader, u_cameraPositionFloat);
	GET_UNIFORM2(entityRenderer.blockEntityshader, u_texture);
	GET_UNIFORM2(entityRenderer.blockEntityshader, u_view);


	entityRenderer.basicEntityShader.shader.clear();
	entityRenderer.basicEntityShader.shader.loadShaderProgramFromFile
	(RESOURCES_PATH "shaders/basicEntity.vert", RESOURCES_PATH "shaders/basicEntity.frag");
	entityRenderer.basicEntityShader.shader.bind();

	GET_UNIFORM2(entityRenderer.basicEntityShader, u_cameraPositionInt);
	GET_UNIFORM2(entityRenderer.basicEntityShader, u_cameraPositionFloat);
	GET_UNIFORM2(entityRenderer.basicEntityShader, u_viewProjection);
	GET_UNIFORM2(entityRenderer.basicEntityShader, u_modelMatrix);
	GET_UNIFORM2(entityRenderer.basicEntityShader, u_view);
	GET_UNIFORM2(entityRenderer.basicEntityShader, u_bonesPerModel);
	
	//
	entityRenderer.basicEntityShader.u_entityTextureSamplerers = getStorageBlockIndex(entityRenderer.basicEntityShader.shader.id, "u_entityTextureSamplerers");
	glShaderStorageBlockBinding(entityRenderer.basicEntityShader.shader.id, entityRenderer.basicEntityShader.u_entityTextureSamplerers, 5);

	entityRenderer.basicEntityShader.u_skinningMatrix = getStorageBlockIndex(entityRenderer.basicEntityShader.shader.id, "u_skinningMatrix");
	glShaderStorageBlockBinding(entityRenderer.basicEntityShader.shader.id, entityRenderer.basicEntityShader.u_skinningMatrix, 6);

	entityRenderer.basicEntityShader.u_perEntityData = getStorageBlockIndex(entityRenderer.basicEntityShader.shader.id, "u_perEntityData");
	glShaderStorageBlockBinding(entityRenderer.basicEntityShader.shader.id, entityRenderer.basicEntityShader.u_perEntityData, 7);


#pragma endregion

#pragma region post process

	hbaoShader.shader.clear();
	hbaoShader.shader.loadShaderProgramFromFile(
		RESOURCES_PATH "shaders/postProcess/drawQuads.vert",
		RESOURCES_PATH "shaders/postProcess/hbao.frag");

	GET_UNIFORM2(hbaoShader, u_gPosition);
	GET_UNIFORM2(hbaoShader, u_gNormal);
	GET_UNIFORM2(hbaoShader, u_view);
	GET_UNIFORM2(hbaoShader, u_projection);

	applyHBAOShader.shader.clear();
	applyHBAOShader.shader.loadShaderProgramFromFile(
		RESOURCES_PATH "shaders/postProcess/drawQuads.vert",
		RESOURCES_PATH "shaders/postProcess/applyHBAO.frag");

	GET_UNIFORM2(applyHBAOShader, u_hbao);
	GET_UNIFORM2(applyHBAOShader, u_currentViewSpace);

	applyHBAOShader.u_shadingSettings
		= glGetUniformBlockIndex(applyHBAOShader.shader.id, "ShadingSettings");
	glBindBufferBase(GL_UNIFORM_BUFFER,
		applyHBAOShader.u_shadingSettings, defaultShader.shadingSettingsBuffer);


	warpShader.shader.clear();
	warpShader.shader.loadShaderProgramFromFile(
		RESOURCES_PATH "shaders/postProcess/drawQuads.vert",
		RESOURCES_PATH "shaders/postProcess/warp.frag");
	GET_UNIFORM2(warpShader, u_color);
	GET_UNIFORM2(warpShader, u_time);
	GET_UNIFORM2(warpShader, u_underwaterColor);
	GET_UNIFORM2(warpShader, u_currentViewSpace);


#pragma endregion
	
}


struct DrawElementsIndirectCommand
{
	GLuint count; // The number of elements to be rendered.
	GLuint instanceCount; // The number of instances of the specified range of elements to be rendered.
	GLuint firstIndex; // The starting index in the enabled arrays.
	GLuint baseVertex; // The starting vertex index in the vertex buffer.
	GLuint baseInstance; // The base instance for use in fetching instanced vertex attributes.
};

void Renderer::renderFromBakedData(SunShadow &sunShadow, ChunkSystem &chunkSystem, Camera &c,
	ProgramData &programData, BlocksLoader &blocksLoader, ClientEntityManager &entityManager, ModelsManager &modelsManager
	, bool showLightLevels, int skyLightIntensity, glm::dvec3 pointPos, bool underWater, int screenX, int screenY,
	float deltaTime)
{
	glViewport(0, 0, screenX, screenY);

	fboCoppy.updateSize(screenX, screenY);
	fboCoppy.clearFBO();

	fboMain.updateSize(screenX, screenY);
	fboMain.clearFBO();

	fboLastFrame.updateSize(screenX, screenY);
	fboLastFramePositions.updateSize(screenX, screenY);
	fboHBAO.updateSize(screenX / 2, screenY / 2);

	fboSkyBox.updateSize(screenX, screenY);
	fboSkyBox.clearFBO();

	glm::vec3 posFloat = {};
	glm::ivec3 posInt = {};
	glm::ivec3 blockPosition = from3DPointToBlock(c.position);
	c.decomposePosition(posFloat, posInt);

	auto viewMatrix = c.getViewMatrix();
	auto vp = c.getProjectionMatrix() * viewMatrix;
	auto chunkVectorCopy = chunkSystem.loadedChunks;

	float timeGrass = std::clock() / 1000.f;


#pragma region frustum culling

	FrustumVolume cameraFrustum(c.getViewProjectionWithPositionMatrixDouble());

	for (auto c : chunkSystem.loadedChunks)
	{
		if (c)
		{
			c->setCulled(0);

			if (frustumCulling)
			{
				AABBVolume chunkAABB;

				chunkAABB.minVertex = {c->data.x * CHUNK_SIZE, 0, c->data.z * CHUNK_SIZE};
				chunkAABB.maxVertex = {(c->data.x + 1) * CHUNK_SIZE, CHUNK_HEIGHT + 1, (c->data.z + 1) * CHUNK_SIZE};

				if (!CheckFrustumVsAABB(cameraFrustum, chunkAABB))
				{
					c->setCulled(1);
				}
			};
		}
	}

#pragma endregion



#pragma region render sky box 0

	glBindFramebuffer(GL_FRAMEBUFFER, fboMain.fboOnlyFirstTarget);

	programData.renderer.skyBoxLoaderAndDrawer.drawBefore(c.getProjectionMatrix() * c.getViewMatrix(),
		programData.renderer.defaultSkyBox,programData.renderer.sunTexture,
		programData.renderer.skyBoxRenderer.sunPos);

	fboSkyBox.copyColorFromOtherFBO(fboMain.color,
		fboMain.size.x, fboMain.size.y);

#pragma endregion


#pragma region setup uniforms and stuff
	{

		defaultShader.shadingSettings.fogDistance =
			(chunkSystem.squareSize / 2.f) * CHUNK_SIZE - CHUNK_SIZE;


		glNamedBufferData(defaultShader.shadingSettingsBuffer,
			sizeof(defaultShader.shadingSettings), &defaultShader.shadingSettings,
			GL_STREAM_DRAW);

		zpassShader.shader.bind();
		glUniformMatrix4fv(zpassShader.u_viewProjection, 1, GL_FALSE, &vp[0][0]);
		glUniform3fv(zpassShader.u_positionFloat, 1, &posFloat[0]);
		glUniform3iv(zpassShader.u_positionInt, 1, &posInt[0]);
		glUniform1i(zpassShader.u_renderOnlyWater, 0);
		glUniform1f(zpassShader.u_timeGrass, timeGrass);



		defaultShader.shader.bind();
		glUniformMatrix4fv(defaultShader.u_viewProjection, 1, GL_FALSE, &vp[0][0]);
		glUniform3fv(defaultShader.u_positionFloat, 1, &posFloat[0]);
		glUniform3iv(defaultShader.u_positionInt, 1, &posInt[0]);
		glUniform1i(defaultShader.u_typesCount, BlocksCount);	//remove
		glUniform1f(defaultShader.u_time, std::clock() / 400.f);
		glUniform1i(defaultShader.u_showLightLevels, showLightLevels);
		glUniform1i(defaultShader.u_skyLightIntensity, skyLightIntensity);
		glUniform3fv(defaultShader.u_sunDirection, 1, &skyBoxRenderer.sunPos[0]);
		glUniform1f(defaultShader.u_metallic, metallic);
		glUniform1f(defaultShader.u_roughness, roughness);
		glUniform1i(defaultShader.u_underWater, underWater);
		glUniform3fv(defaultShader.u_waterColor, 1, &defaultShader.shadingSettings.waterColor[0]);
		glUniform1i(defaultShader.u_depthPeelwaterPass, 0);
		glUniform1i(defaultShader.u_hasPeelInformation, 0);
		glUniform1f(defaultShader.u_waterMove, waterTimer);
		glUniform1f(defaultShader.u_near, c.closePlane);
		glUniform1f(defaultShader.u_far, c.farPlane);
		glUniformMatrix4fv(defaultShader.u_inverseProjMat, 1, 0,
			&glm::inverse(c.getProjectionMatrix())[0][0]);
		glUniformMatrix4fv(defaultShader.u_lightSpaceMatrix, 1, 0,
			&sunShadow.lightSpaceMatrix[0][0]);
		glUniform3iv(defaultShader.u_lightPos, 1, &sunShadow.lightSpacePosition[0]);
		glUniform1i(defaultShader.u_writeScreenSpacePositions, 1);//todo remove

		glUniformMatrix4fv(defaultShader.u_inverseViewProjMat, 1, 0,
			&glm::inverse(c.getProjectionMatrix() * viewMatrix)[0][0]);

		glUniformMatrix4fv(defaultShader.u_lastViewProj, 1, 0,
			&(c.lastFrameViewProjMatrix)[0][0]);

	#pragma region textures

		programData.numbersTexture.bind(0);

		programData.dudv.bind(1);
		glUniform1i(defaultShader.u_dudv, 1);

		programData.dudvNormal.bind(2);
		glUniform1i(defaultShader.u_dudvNormal, 2);

		programData.causticsTexture.bind(3);
		glUniform1i(defaultShader.u_caustics, 3);

		glActiveTexture(GL_TEXTURE0 + 4);
		glBindTexture(GL_TEXTURE_2D, sunShadow.shadowMap.depth);
		glUniform1i(defaultShader.u_sunShadowTexture, 4);

		glActiveTexture(GL_TEXTURE0 + 5);
		glBindTexture(GL_TEXTURE_2D, fboLastFrame.color);
		glUniform1i(defaultShader.u_lastFrameColor, 5);

		glActiveTexture(GL_TEXTURE0 + 6);
		glBindTexture(GL_TEXTURE_2D, fboLastFramePositions.color);
		glUniform1i(defaultShader.u_lastFramePositionViewSpace, 6);

		glActiveTexture(GL_TEXTURE0 + 7);
		glBindTexture(GL_TEXTURE_2D, brdfTexture.id);
		glUniform1i(defaultShader.u_brdf, 7);

		glActiveTexture(GL_TEXTURE0 + 8);
		glBindTexture(GL_TEXTURE_2D, fboSkyBox.color);
		glUniform1i(defaultShader.u_skyTexture, 8);

	#pragma endregion

		glUniform1f(defaultShader.u_timeGrass, timeGrass);

		glUniformMatrix4fv(defaultShader.u_cameraProjection, 1, GL_FALSE, glm::value_ptr(c.getProjectionMatrix()));

		glUniformMatrix4fv(defaultShader.u_inverseView, 1, GL_FALSE,
			glm::value_ptr(glm::inverse(viewMatrix)));

		glUniformMatrix4fv(defaultShader.u_view, 1, GL_FALSE,
			glm::value_ptr(viewMatrix));

		waterTimer += deltaTime * 0.09;
		if (waterTimer > 20)
		{
			waterTimer -= 20;
		}

		//waterTimer += deltaTime * 0.4;
		//if (waterTimer > 1)
		//{
		//	waterTimer -= 1;
		//}


		{
			glm::ivec3 i;
			glm::vec3 f;
			decomposePosition(pointPos, f, i);

			glUniform3fv(defaultShader.u_pointPosF, 1, &f[0]);
			glUniform3iv(defaultShader.u_pointPosI, 1, &i[0]);
		}

	#pragma region lights
		{

			if (chunkSystem.shouldUpdateLights)
			{
				chunkSystem.shouldUpdateLights = 0;

				//std::cout << "Updated lights\n";

				lightsBufferCount = 0;

				for (auto c : chunkSystem.loadedChunks)
				{
					if (c)
					{
						lightsBufferCount += c->lightsElementCountSize;
					}
				}

				glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightBuffer);
				glBufferData(GL_SHADER_STORAGE_BUFFER, lightsBufferCount * sizeof(glm::ivec4), NULL, GL_STREAM_COPY);

				glBindBuffer(GL_COPY_WRITE_BUFFER, lightBuffer);

				//todo only copy chunks that are close
				//todo max lights
				size_t offset = 0;
				for (auto c : chunkSystem.loadedChunks)
				{

					if (c)
					{
						glBindBuffer(GL_COPY_READ_BUFFER, c->lightsBuffer);

						// Copy data from the first existing SSBO to the new SSBO
						glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, offset,
							c->lightsElementCountSize * sizeof(glm::ivec4));

						offset += c->lightsElementCountSize * sizeof(glm::ivec4);
					}
				}

				glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightBuffer);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, lightBuffer);
			};

			glUniform1i(defaultShader.u_lightsCount, lightsBufferCount);

			//auto c = chunkSystem.getChunkSafeFromBlockPos(posInt.x, posInt.z);
			//
			//if (c)
			//{
			//	glBindBuffer(GL_SHADER_STORAGE_BUFFER, c->lightsBuffer);
			//	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, c->lightsBuffer);
			//	glUniform1i(u_lightsCount, c->lightsElementCountSize);
			//}
		}
	#pragma endregion

		//sort chunks
		{
			std::sort(chunkVectorCopy.begin(), chunkVectorCopy.end(),
				[x = divideChunk(blockPosition.x), z = divideChunk(blockPosition.z)](Chunk *b, Chunk *a)
			{
				if (a == nullptr) { return false; }
				if (b == nullptr) { return true; }

				int ax = a->data.x - x;
				int az = a->data.z - z;

				int bx = b->data.x - x;
				int bz = b->data.z - z;

				unsigned long reza = ax * ax + az * az;
				unsigned long rezb = bx * bx + bz * bz;

				return reza < rezb;
			}
			);
		}
	}
#pragma endregion


	//configure opaque geometry
	static std::vector<DrawElementsIndirectCommand> drawCommands;

	if(unifiedGeometry)
	{
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, drawCommandsOpaqueBuffer);
		drawCommands.clear();

		if (sortChunks)
		{
			int s = chunkVectorCopy.size();
			for (int i = s - 1; i >= 0; i--)
			{
				auto chunk = chunkVectorCopy[i];
				if (chunk)
				{
					if (!chunk->isDontDrawYet() &&
						!chunk->isCulled())
					{
						int facesCount = chunk->elementCountSize;
						if (facesCount)
						{

							auto entry = chunkSystem.gpuBuffer
								.getEntry({chunk->data.x,chunk->data.z});

							permaAssertComment(
								(int)entry.size / (4 * sizeof(int)) == facesCount,
								"Gib Gpu Buffer desync");

							// Prepare draw command for this chunk
							DrawElementsIndirectCommand command;
							command.count = 4; // Assuming you're drawing quads
							command.instanceCount = facesCount;
							command.firstIndex = 0;
							command.baseVertex = 0;
							command.baseInstance = entry.beg / (4 * sizeof(int)); 

							// Add draw command to the array
							drawCommands.push_back(command);

						}
					}
				}
			}
		}
		else
		{
			for (auto &chunk : chunkVectorCopy)
			{
				if (chunk)
				{
					if (!chunk->isDontDrawYet()
						&& ! chunk->isCulled()
						)
					{
						int facesCount = chunk->elementCountSize;
						if (facesCount)
						{
							auto entry = chunkSystem.gpuBuffer
								.getEntry({chunk->data.x,chunk->data.z});

							permaAssertComment(
								(int)entry.size / (4 * sizeof(int)) == facesCount,
								"Gib Gpu Buffer desync");

							// Prepare draw command for this chunk
							DrawElementsIndirectCommand command;
							command.count = 4; // Assuming you're drawing quads
							command.instanceCount = facesCount;
							command.firstIndex = 0;
							command.baseVertex = 0;
							command.baseInstance = entry.beg / (4 * sizeof(int));

							// Add draw command to the array
							drawCommands.push_back(command);

						}
					}
				}
			}
		}

		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, drawCommandsOpaqueBuffer);
		glBufferData(GL_DRAW_INDIRECT_BUFFER, drawCommands.size() * sizeof(DrawElementsIndirectCommand), drawCommands.data(), GL_STREAM_DRAW);
	}

	auto renderStaticGeometry = [&]()
	{

		if (unifiedGeometry)
		{
			glBindBuffer(GL_DRAW_INDIRECT_BUFFER, drawCommandsOpaqueBuffer);

			glBindVertexArray(chunkSystem.gpuBuffer.vao);

			glMultiDrawElementsIndirect(GL_TRIANGLE_FAN, GL_UNSIGNED_BYTE,
				nullptr, drawCommands.size(), sizeof(DrawElementsIndirectCommand));
			
		}
		else
		{


			if (sortChunks)
			{
				int s = chunkVectorCopy.size();
				for (int i = s - 1; i >= 0; i--)
				{
					auto chunk = chunkVectorCopy[i];
					if (chunk)
					{
						if (!chunk->isDontDrawYet() && !chunk->isCulled())
						{
							int facesCount = chunk->elementCountSize;
							if (facesCount)
							{
								glBindVertexArray(chunk->vao);
								glDrawElementsInstanced(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_BYTE, 0, facesCount);
							}
						}
					}
				}
			}
			else
			{
				for (auto &chunk : chunkVectorCopy)
				{
					if (chunk)
					{
						if (!chunk->isDontDrawYet() && !chunk->isCulled())
						{
							int facesCount = chunk->elementCountSize;
							if (facesCount)
							{
								glBindVertexArray(chunk->vao);
								glDrawElementsInstanced(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_BYTE, 0, facesCount);
							}
						}
					}
				}
			}

		}

	};

	auto renderTransparentGeometry = [&]()
	{
		for (auto &chunk : chunkVectorCopy)
		{
			if (chunk)
			{
				if (!chunk->isDontDrawYet() && !chunk->isCulled())
				{
					int facesCount = chunk->transparentElementCountSize;
					if (facesCount)
					{
						glBindVertexArray(chunk->transparentVao);
						glDrawElementsInstanced(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_BYTE, 0, facesCount);
					}
				}
			}
		}
	};

	auto depthPrePass = [&]()
	{
		if (zprepass)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, fboMain.fbo);
			glDepthFunc(GL_LESS);
			glDisable(GL_BLEND);
			glColorMask(0, 0, 0, 0);
			zpassShader.shader.bind();
			renderStaticGeometry();
		}
	};
	
	auto solidPass = [&]()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fboMain.fbo);
		glColorMask(1, 1, 1, 1);

		if (zprepass)
		{
			glDepthFunc(GL_EQUAL);
		}
		else
		{
			glDepthFunc(GL_LESS);
		}

		defaultShader.shader.bind();
		glDisable(GL_BLEND);
		//glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		renderStaticGeometry();
	};

	auto copyToMainFbo = [&]()
	{
		fboMain.writeAllToOtherFbo(0, screenX, screenY);
		fboLastFrame.copyColorFromOtherFBO(fboMain.fboOnlyFirstTarget, screenX, screenY);
		fboLastFramePositions.copyColorFromOtherFBO(fboMain.fboOnlySecondTarget, screenX, screenY);
	};

	auto copyToMainFboOnlyLastFrameStuff = [&]()
	{
		fboLastFrame.copyColorFromOtherFBO(fboMain.fboOnlyFirstTarget, screenX, screenY);
		fboLastFramePositions.copyColorFromOtherFBO(fboMain.fboOnlySecondTarget, screenX, screenY);
	};

	auto renderTransparentGeometryPhaze = [&](bool hasPeelInformation)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fboMain.fbo);
		defaultShader.shader.bind();
		glColorMask(1, 1, 1, 1);

		glEnablei(GL_BLEND, 0);
		glBlendFunci(0, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisablei(GL_BLEND, 1);


		glColorMask(1, 1, 1, 1);
		glUniform1i(defaultShader.u_depthPeelwaterPass, 0);
		glUniform1i(defaultShader.u_hasPeelInformation, hasPeelInformation);

		glActiveTexture(GL_TEXTURE0 + 10);
		glBindTexture(GL_TEXTURE_2D, fboCoppy.color);
		glUniform1i(defaultShader.u_PeelTexture, 10);

		glActiveTexture(GL_TEXTURE0 + 9);
		glBindTexture(GL_TEXTURE_2D, fboCoppy.depth);
		glUniform1i(defaultShader.u_depthTexture, 9);


		glDepthFunc(GL_LESS);
		glDisable(GL_CULL_FACE); //todo change
		renderTransparentGeometry();
		glEnable(GL_CULL_FACE);
	};

	if (waterRefraction)
	{

	#pragma region depth pre pass 1
		depthPrePass();
	#pragma endregion


	#pragma region solid pass 2
		solidPass();
	#pragma endregion


	#pragma region render entities
		renderEntities(deltaTime, modelsManager, blocksLoader,
			entityManager, vp, viewMatrix, posFloat, posInt);
	#pragma endregion


	#pragma region copy depth 3
		fboCoppy.copyDepthFromOtherFBO(fboMain.fbo, screenX, screenY);
	#pragma endregion


	#pragma region render only water geometry to depth 4
		defaultShader.shader.bind();
		glBindFramebuffer(GL_FRAMEBUFFER, fboCoppy.fbo);
		glDepthFunc(GL_LESS);
		zpassShader.shader.bind();
		glUniform1i(zpassShader.u_renderOnlyWater, 1);
		glEnablei(GL_BLEND, 0);
		glBlendFunci(0, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisablei(GL_BLEND, 1);
		glColorMask(0, 0, 0, 0);

		renderTransparentGeometry();
	#pragma endregion


	#pragma region render with depth peel first part of the transparent of the geometry 5
		glBindFramebuffer(GL_FRAMEBUFFER, fboMain.fbo);
		defaultShader.shader.bind();

		glEnablei(GL_BLEND, 0);
		glBlendFunci(0, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisablei(GL_BLEND, 1);

		glColorMask(1, 1, 1, 1);
		glUniform1i(defaultShader.u_depthPeelwaterPass, true);

		glActiveTexture(GL_TEXTURE0 + 9);
		glBindTexture(GL_TEXTURE_2D, fboCoppy.depth);
		glUniform1i(defaultShader.u_depthTexture, 9);

		glDepthFunc(GL_LESS);
		glDisable(GL_CULL_FACE); //todo change
		//todo disable ssr for this step?
		renderTransparentGeometry();
	#pragma endregion


	#pragma region copy color buffer and last depth for later use 6
		fboCoppy.copyDepthAndColorFromOtherFBO(fboMain.fbo, screenX, screenY);
	#pragma endregion


	#pragma region render transparent geometry last phaze 7
		renderTransparentGeometryPhaze(true);
	#pragma endregion


	}
	else
	{
		depthPrePass();

		solidPass();

		renderEntities(deltaTime, modelsManager, blocksLoader,
			entityManager, vp, viewMatrix, posFloat, posInt);

		fboCoppy.copyDepthFromOtherFBO(fboMain.fbo, screenX, screenY);

		renderTransparentGeometryPhaze(false);


	}

#pragma region post process

	//hbao
	if (1)
	{
		glViewport(0, 0, fboHBAO.size.x, fboHBAO.size.y);

		glBindFramebuffer(GL_FRAMEBUFFER, fboHBAO.fbo);
		glClear(GL_COLOR_BUFFER_BIT);

		hbaoShader.shader.bind();
		glBindVertexArray(vaoQuad);
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, fboMain.secondaryColor);
		glUniform1i(hbaoShader.u_gPosition, 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, fboMain.thirdColor);
		glUniform1i(hbaoShader.u_gNormal, 1);

		glUniformMatrix4fv(hbaoShader.u_view, 1, GL_FALSE,
			glm::value_ptr(viewMatrix));

		glUniformMatrix4fv(hbaoShader.u_projection, 1, GL_FALSE,
			glm::value_ptr(c.getProjectionMatrix()));
		//GET_UNIFORM2(hbaoShader, u_texNoise);
		
		
		
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		//todo optimize texture binding so it is done only once at the beginning, not changed that often
		
		//apply hbao
		glDisable(GL_DEPTH_TEST);
		glViewport(0, 0, screenX, screenY);

		glBindFramebuffer(GL_FRAMEBUFFER, fboMain.fboOnlyFirstTarget);
		
		
		applyHBAOShader.shader.bind();
		glEnable(GL_BLEND);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, fboHBAO.color);
		glUniform1i(applyHBAOShader.u_hbao, 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, fboMain.secondaryColor);
		glUniform1i(applyHBAOShader.u_currentViewSpace, 1);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


	};

	//warp
	if (underWater)
	{
		glBindVertexArray(vaoQuad);

		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		warpShader.shader.bind();

		glm::vec3 underWater = defaultShader.shadingSettings.underWaterColor;

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, fboMain.color);
		glUniform1i(warpShader.u_color, 0);
		glUniform1f(warpShader.u_time, timeGrass); //todo change
		glUniform3f(warpShader.u_underwaterColor, underWater.x,
			underWater.y, underWater.z);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, fboMain.secondaryColor);
		glUniform1i(warpShader.u_currentViewSpace, 1);


		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		copyToMainFboOnlyLastFrameStuff();
	}
	else
	{
		copyToMainFbo();
	}

#pragma endregion



	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glBindVertexArray(0);

}

void Renderer::renderEntities(
	float deltaTime,
	ModelsManager &modelsManager,
	BlocksLoader &blocksLoader, ClientEntityManager &entityManager,
	glm::mat4 &vp,
	glm::mat4 &viewMatrix,
	glm::vec3 posFloat,
	glm::ivec3 posInt
	)
{

#pragma region setup pipeline

	glBindFramebuffer(GL_FRAMEBUFFER, fboMain.fbo);
	glDepthFunc(GL_LESS);

#pragma endregion


#pragma region cubeEntities
	{

		entityRenderer.blockEntityshader.shader.bind();

		glBindVertexArray(entityRenderer.blockEntityshader.vaoCube);


		glUniformMatrix4fv(entityRenderer.blockEntityshader.u_viewProjection, 1, GL_FALSE, &vp[0][0]);
		glUniformMatrix4fv(entityRenderer.blockEntityshader.u_view, 1, GL_FALSE, &viewMatrix[0][0]);
		glUniformMatrix4fv(entityRenderer.blockEntityshader.u_modelMatrix, 1, GL_FALSE,
			glm::value_ptr(glm::scale(glm::vec3{0.4f})));
		glUniform3fv(entityRenderer.blockEntityshader.u_cameraPositionFloat, 1, &posFloat[0]);
		glUniform3iv(entityRenderer.blockEntityshader.u_cameraPositionInt, 1, &posInt[0]);

		//debug stuff
		if (0)
		{
			//todo something better here lol
			std::uint64_t textures[6] = {};

			for (int i = 0; i < 6; i++)
			{
				textures[i] = blocksLoader.gpuIds[getGpuIdIndexForBlock(BlockTypes::bookShelf, i)];
			}

			glUniformHandleui64vARB(entityRenderer.blockEntityshader.u_texture, 6, textures);


			//todo instance rendering
			for (auto &e : entityRenderer.itemEntitiesToRender)
			{
				glm::vec3 entityFloat = {};
				glm::ivec3 entityInt = {};

				decomposePosition(e.position, entityFloat, entityInt);

				entityFloat += glm::vec3(0, 0.2, 0);

				glUniform3fv(entityRenderer.blockEntityshader.u_entityPositionFloat, 1, &entityFloat[0]);
				glUniform3iv(entityRenderer.blockEntityshader.u_entityPositionInt, 1, &entityInt[0]);

				glDrawArrays(GL_TRIANGLES, 0, 36);

			}

		}

		//real entities
		{

			//todo instance rendering
			for (auto &e : entityManager.droppedItems)
			{
				//todo something better here lol
				std::uint64_t textures[6] = {};

				for (int i = 0; i < 6; i++)
				{
					textures[i] = blocksLoader.gpuIds
						[getGpuIdIndexForBlock(e.second.entity.type, i)];
				}

				glUniformHandleui64vARB(entityRenderer.blockEntityshader.u_texture, 6, textures);

				glm::vec3 entityFloat = {};
				glm::ivec3 entityInt = {};

				//decomposePosition(e.second.item.position, entityFloat, entityInt);
				decomposePosition(e.second.getRubberBandPosition(), entityFloat, entityInt);

				entityFloat += glm::vec3(0, 0.2, 0);

				glUniform3fv(entityRenderer.blockEntityshader.u_entityPositionFloat, 1, &entityFloat[0]);
				glUniform3iv(entityRenderer.blockEntityshader.u_entityPositionInt, 1, &entityInt[0]);

				glDrawArrays(GL_TRIANGLES, 0, 36);
			}

		}


	}
#pragma endregion


#pragma region other entities setup shader
	
	entityRenderer.basicEntityShader.shader.bind();

	glUniformMatrix4fv(entityRenderer.basicEntityShader.u_viewProjection, 1, GL_FALSE, &vp[0][0]);
	glUniformMatrix4fv(entityRenderer.basicEntityShader.u_view, 1, GL_FALSE, &viewMatrix[0][0]);
	glUniformMatrix4fv(entityRenderer.basicEntityShader.u_modelMatrix, 1, GL_FALSE,
		&glm::mat4(1.f)[0][0]);
	glUniform3fv(entityRenderer.basicEntityShader.u_cameraPositionFloat, 1, &posFloat[0]);
	glUniform3iv(entityRenderer.basicEntityShader.u_cameraPositionInt, 1, &posInt[0]);

#pragma endregion


	auto renderModel = [&](glm::dvec3 pos, int vertexCount)
	{
		glm::vec3 entityFloat = {};
		glm::ivec3 entityInt = {};
		decomposePosition(pos, entityFloat, entityInt);


		glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, nullptr);
	};


	struct PerEntityData
	{
		glm::ivec3 entityPositionInt = {}; 
		glm::vec3 entityPositionFloat = {};
		GLuint64 textureId;
	};

	static std::vector<glm::mat4> skinningMatrix;
	skinningMatrix.clear();

	static std::vector<PerEntityData> entityData;
	entityData.clear();

	auto renderAllEntitiesOfOneType = [&](Model &model, auto &container, int textureIndex)
	{
		
		glBindVertexArray(model.vao);

		glUniformMatrix4fv(entityRenderer.basicEntityShader.u_modelMatrix, 1, GL_FALSE,
			&glm::mat4(1.f)
			[0][0]);

		glUniform1i(entityRenderer.basicEntityShader.u_bonesPerModel, model.transforms.size());

		skinningMatrix.clear();
		entityData.clear();

		skinningMatrix.reserve(model.transforms.size() *container.size());
		entityData.reserve(model.transforms.size() *container.size());
		
		for (auto &e : container)
		{
			auto rotMatrix = e.second.getBodyRotationMatrix();

			for (int i = 0; i < model.transforms.size(); i++)
			{
				skinningMatrix.push_back(rotMatrix * model.transforms[i]);
			}

			e.second.setEntityMatrix(skinningMatrix.data() + (skinningMatrix.size() - model.transforms.size()));

			PerEntityData data = {};
			data.textureId = modelsManager.gpuIds[textureIndex];

			decomposePosition(e.second.getRubberBandPosition(), data.entityPositionFloat, data.entityPositionInt);
			entityData.push_back(data);
		}

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, skinningMatrixSSBO);
		//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, skinningMatrixSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, skinningMatrix.size() * sizeof(glm::mat4),
			&skinningMatrix[0][0][0], GL_STREAM_DRAW);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, perEntityDataSSBO);
		//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, perEntityDataSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, entityData.size() * sizeof(entityData[0]),
			entityData.data(), GL_STREAM_DRAW);


		//int instance = 0;
		//for (auto &e : container)
		//{
		//	glUniform1i(entityRenderer.basicEntityShader.u_instance, instance);
		//	renderModel(e.second.getRubberBandPosition(), model.vertexCount);
		//
		//	instance++;
		//}

		glDrawElementsInstanced(GL_TRIANGLES, model.vertexCount, GL_UNSIGNED_INT, nullptr,
			container.size());

	};

	//todo remove
	entityRenderer.itemEntitiesToRender.clear();

	renderAllEntitiesOfOneType(modelsManager.human, entityManager.players, ModelsManager::SteveTexture);
	renderAllEntitiesOfOneType(modelsManager.human, entityManager.zombies, ModelsManager::ZombieTexture);
	renderAllEntitiesOfOneType(modelsManager.pig, entityManager.pigs, ModelsManager::PigTexture);



	glBindVertexArray(0);


}

//https://www.youtube.com/watch?v=lUo7s-i9Gy4&ab_channel=OREONENGINE
void computeFrustumDimensions(
	glm::vec3 position, glm::vec3 viewDirection,
	float fovRadians, float aspectRatio, float nearPlane, float farPlane,
	glm::vec2 &nearDimensions, glm::vec2 &farDimensions, glm::vec3 &centerNear, glm::vec3 &centerFar)
{
	float tanFov2 = tan(fovRadians) * 2;

	nearDimensions.y = tanFov2 * nearPlane;			//hNear
	nearDimensions.x = nearDimensions.y * aspectRatio;	//wNear

	farDimensions.y = tanFov2 * farPlane;			//hNear
	farDimensions.x = farDimensions.y * aspectRatio;	//wNear

	centerNear = position + viewDirection * farPlane;
	centerFar = position + viewDirection * farPlane;

}

void generateTangentSpace(glm::vec3 v, glm::vec3 &outUp, glm::vec3 &outRight)
{
	glm::vec3 up(0, 1, 0);
	if (v == up)
	{
		outRight = glm::vec3(1, 0, 0);
	}
	else
	{
		outRight = normalize(glm::cross(v, up));
	}
	outUp = normalize(cross(outRight, v));
}

//https://www.youtube.com/watch?v=lUo7s-i9Gy4&ab_channel=OREONENGINE
void computeFrustumSplitCorners(glm::vec3 directionVector,
	glm::vec2 nearDimensions, glm::vec2 farDimensions, glm::vec3 centerNear, glm::vec3 centerFar,
	glm::vec3 &nearTopLeft, glm::vec3 &nearTopRight, glm::vec3 &nearBottomLeft, glm::vec3 &nearBottomRight,
	glm::vec3 &farTopLeft, glm::vec3 &farTopRight, glm::vec3 &farBottomLeft, glm::vec3 &farBottomRight)
{

	glm::vec3 rVector = {};
	glm::vec3 upVectpr = {};

	generateTangentSpace(directionVector, upVectpr, rVector);

	nearTopLeft = centerNear + upVectpr * nearDimensions.y / 2.f - rVector * nearDimensions.x / 2.f;
	nearTopRight = centerNear + upVectpr * nearDimensions.y / 2.f + rVector * nearDimensions.x / 2.f;
	nearBottomLeft = centerNear - upVectpr * nearDimensions.y / 2.f - rVector * nearDimensions.x / 2.f;
	nearBottomRight = centerNear - upVectpr * nearDimensions.y / 2.f + rVector * nearDimensions.x / 2.f;

	farTopLeft = centerNear + upVectpr * farDimensions.y / 2.f - rVector * farDimensions.x / 2.f;
	farTopRight = centerNear + upVectpr * farDimensions.y / 2.f + rVector * farDimensions.x / 2.f;
	farBottomLeft = centerNear - upVectpr * farDimensions.y / 2.f - rVector * farDimensions.x / 2.f;
	farBottomRight = centerNear - upVectpr * farDimensions.y / 2.f + rVector * farDimensions.x / 2.f;

}

glm::mat4 lookAtSafe(glm::vec3 const &eye, glm::vec3 const &center, glm::vec3 const &upVec)
{
	glm::vec3 up = glm::normalize(upVec);

	glm::vec3 f;
	glm::vec3 s;
	glm::vec3 u;

	f = (normalize(center - eye));
	if (f == up || f == -up)
	{
		s = glm::vec3(up.z, up.x, up.y);
		u = (cross(s, f));

	}
	else
	{
		s = (normalize(cross(f, up)));
		u = (cross(s, f));
	}

	glm::mat4 Result(1);
	Result[0][0] = s.x;
	Result[1][0] = s.y;
	Result[2][0] = s.z;
	Result[0][1] = u.x;
	Result[1][1] = u.y;
	Result[2][1] = u.z;
	Result[0][2] = -f.x;
	Result[1][2] = -f.y;
	Result[2][2] = -f.z;
	Result[3][0] = -dot(s, eye);
	Result[3][1] = -dot(u, eye);
	Result[3][2] = dot(f, eye);
	return Result;
}

glm::mat4 calculateLightProjectionMatrix(Camera &camera, glm::vec3 lightDir,
	float nearPlane, float farPlane,
	float zOffset)
{
	glm::vec3 direction = lightDir;
	glm::vec3 eye = camera.position - glm::dvec3(direction);
	glm::vec3 center = eye + direction;

	glm::mat4 lightView = lookAtSafe(eye, center, {0.f,1.f,0.f});//todo option to add a custom vector direction

	glm::vec3 rVector = {};
	glm::vec3 upVectpr = {};
	generateTangentSpace(lightDir, upVectpr, rVector);

	glm::vec2 nearDimensions{};
	glm::vec2 farDimensions{};
	glm::vec3 centerNear{};
	glm::vec3 centerFar{};

	computeFrustumDimensions(camera.position, camera.viewDirection, camera.fovRadians, camera.aspectRatio,
		nearPlane, farPlane, nearDimensions, farDimensions, centerNear, centerFar);

	glm::vec3 nearTopLeft{};
	glm::vec3 nearTopRight{};
	glm::vec3 nearBottomLeft{};
	glm::vec3 nearBottomRight{};
	glm::vec3 farTopLeft{};
	glm::vec3 farTopRight{};
	glm::vec3 farBottomLeft{};
	glm::vec3 farBottomRight{};

	computeFrustumSplitCorners(camera.viewDirection, nearDimensions, farDimensions, centerNear, centerFar,
		nearTopLeft,
		nearTopRight,
		nearBottomLeft,
		nearBottomRight,
		farTopLeft,
		farTopRight,
		farBottomLeft,
		farBottomRight
	);


	glm::vec3 corners[] =
	{
		nearTopLeft,
		nearTopRight,
		nearBottomLeft,
		nearBottomRight,
		farTopLeft,
		farTopRight,
		farBottomLeft,
		farBottomRight,
	};

	float longestDiagonal = glm::distance(nearTopLeft, farBottomRight);

	glm::vec3 minVal{};
	glm::vec3 maxVal{};

	for (int i = 0; i < 8; i++)
	{
		glm::vec4 corner(corners[i], 1);

		glm::vec4 lightViewCorner = lightView * corner;

		if (i == 0)
		{
			minVal = lightViewCorner;
			maxVal = lightViewCorner;
		}
		else
		{
			if (lightViewCorner.x < minVal.x) { minVal.x = lightViewCorner.x; }
			if (lightViewCorner.y < minVal.y) { minVal.y = lightViewCorner.y; }
			if (lightViewCorner.z < minVal.z) { minVal.z = lightViewCorner.z; }

			if (lightViewCorner.x > maxVal.x) { maxVal.x = lightViewCorner.x; }
			if (lightViewCorner.y > maxVal.y) { maxVal.y = lightViewCorner.y; }
			if (lightViewCorner.z > maxVal.z) { maxVal.z = lightViewCorner.z; }

		}

	}

	//keep them square and the same size:
	//https://www.youtube.com/watch?v=u0pk1LyLKYQ&t=99s&ab_channel=WesleyLaFerriere
	if (1)
	{
		float firstSize = maxVal.x - minVal.x;
		float secondSize = maxVal.y - minVal.y;
		float thirdSize = maxVal.z - minVal.z;

		{
			float ratio = longestDiagonal / firstSize;

			glm::vec2 newVecValues = {minVal.x, maxVal.x};
			float dimension = firstSize;
			float dimensionOver2 = dimension / 2.f;

			newVecValues -= glm::vec2(minVal.x + dimensionOver2, minVal.x + dimensionOver2);
			newVecValues *= ratio;
			newVecValues += glm::vec2(minVal.x + dimensionOver2, minVal.x + dimensionOver2);

			minVal.x = newVecValues.x;
			maxVal.x = newVecValues.y;
		}

		{
			float ratio = longestDiagonal / secondSize;

			glm::vec2 newVecValues = {minVal.y, maxVal.y};
			float dimension = secondSize;
			float dimensionOver2 = dimension / 2.f;

			newVecValues -= glm::vec2(minVal.y + dimensionOver2, minVal.y + dimensionOver2);
			newVecValues *= ratio;
			newVecValues += glm::vec2(minVal.y + dimensionOver2, minVal.y + dimensionOver2);

			minVal.y = newVecValues.x;
			maxVal.y = newVecValues.y;
		}

		{//this size probably can be far-close
			float ratio = longestDiagonal / thirdSize;

			glm::vec2 newVecValues = {minVal.z, maxVal.z};
			float dimension = thirdSize;
			float dimensionOver2 = dimension / 2.f;

			newVecValues -= glm::vec2(minVal.z + dimensionOver2, minVal.z + dimensionOver2);
			newVecValues *= ratio;
			newVecValues += glm::vec2(minVal.z + dimensionOver2, minVal.z + dimensionOver2);

			minVal.z = newVecValues.x;
			maxVal.z = newVecValues.y;
		}

	}

	float near_plane = minVal.z - zOffset;
	float far_plane = maxVal.z;


	glm::vec2 ortoMin = {minVal.x, minVal.y};
	glm::vec2 ortoMax = {maxVal.x, maxVal.y};

	//remove shadow flicker
	if (1)
	{
		glm::vec2 shadowMapSize(2048, 2048);
		glm::vec2 worldUnitsPerTexel = (ortoMax - ortoMin) / shadowMapSize;

		ortoMin /= worldUnitsPerTexel;
		ortoMin = glm::floor(ortoMin);
		ortoMin *= worldUnitsPerTexel;

		ortoMax /= worldUnitsPerTexel;
		ortoMax = glm::floor(ortoMax);
		ortoMax *= worldUnitsPerTexel;

		float zWorldUnitsPerTexel = (far_plane - near_plane) / 2048;

		near_plane /= zWorldUnitsPerTexel;
		far_plane /= zWorldUnitsPerTexel;
		near_plane = glm::floor(near_plane);
		far_plane = glm::floor(far_plane);
		near_plane *= zWorldUnitsPerTexel;
		far_plane *= zWorldUnitsPerTexel;

	}

	glm::mat4 lightProjection = glm::ortho(ortoMin.x, ortoMax.x, ortoMin.y, ortoMax.y, near_plane, far_plane);

	return lightProjection * lightView;

};


void Renderer::renderShadow(SunShadow &sunShadow,
	ChunkSystem &chunkSystem, Camera &c, ProgramData &programData)
{
	glEnable(GL_DEPTH_TEST);
	glColorMask(0, 0, 0, 0);

	glViewport(0, 0, 2048, 2048);
	glBindFramebuffer(GL_FRAMEBUFFER, sunShadow.shadowMap.fbo);
	glClear(GL_DEPTH_BUFFER_BIT);


	glm::ivec3 newPos = c.position;

	{
		//newPos.y = 120;
		newPos.y += 50;

		glm::vec3 moveDir = skyBoxRenderer.sunPos;

		float l = glm::length(moveDir);
		if (l > 0.0001)
		{
			moveDir /= l;
			newPos += moveDir * (float)CHUNK_SIZE * 12.f;
		}
	}

	//glm::vec3 posFloat = {};
	//glm::ivec3 posInt = {};
	//c.decomposePosition(posFloat, posInt);

	glm::vec3 posFloat = {};
	glm::ivec3 posInt = newPos;
	
	glm::ivec3 playerPos = c.position;

	glm::vec3 vectorToPlayer = glm::normalize(glm::vec3(playerPos - newPos));

	//posInt += glm::vec3(0, 0, 10);

	float projectSize = 50;

	float near_plane = 1.0f, far_plane = 260.f;
	glm::mat4 lightProjection = glm::ortho(-projectSize, projectSize, -projectSize, projectSize,
		near_plane,		far_plane);
	//auto mvp = lightProjection * glm::lookAt({},
	//	-skyBoxRenderer.sunPos, glm::vec3(0, 1, 0));

	auto mvp = lightProjection * glm::lookAt({},
		vectorToPlayer, glm::vec3(0, 1, 0));

	

	//auto mat = calculateLightProjectionMatrix(c, -skyBoxRenderer.sunPos, 1, 260, 25);

	sunShadow.lightSpaceMatrix = mvp;
	sunShadow.lightSpacePosition = posInt;
	
	//sunShadow.lightSpaceMatrix = mat;
	//sunShadow.lightSpacePosition = {};

#pragma region setup uniforms and stuff
	{
		zpassShader.shader.bind();
		glUniformMatrix4fv(zpassShader.u_viewProjection, 1, GL_FALSE, &sunShadow.lightSpaceMatrix[0][0]);
		glUniform3fv(zpassShader.u_positionFloat, 1, &posFloat[0]);
		glUniform3iv(zpassShader.u_positionInt, 1, &sunShadow.lightSpacePosition[0]);
		glUniform1i(zpassShader.u_renderOnlyWater, 0);
	}
#pragma endregion




	for (auto &chunk : chunkSystem.loadedChunks)
	{
		if (chunk)
		{
			if (!chunk->isDontDrawYet())
			{
				int facesCount = chunk->elementCountSize;
				if (facesCount)
				{
					glBindVertexArray(chunk->vao);
					glDrawElementsInstanced(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_BYTE, 0, facesCount);
				}
			}
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glColorMask(1, 1, 1, 1);

}

float cubePositions[] = {
		-0.5f, +0.5f, +0.5f, // 0
		+0.5f, +0.5f, +0.5f, // 1
		+0.5f, +0.5f, -0.5f, // 2
		-0.5f, +0.5f, -0.5f, // 3
		-0.5f, +0.5f, -0.5f, // 4
		+0.5f, +0.5f, -0.5f, // 5
		+0.5f, -0.5f, -0.5f, // 6
		-0.5f, -0.5f, -0.5f, // 7
		+0.5f, +0.5f, -0.5f, // 8
		+0.5f, +0.5f, +0.5f, // 9
		+0.5f, -0.5f, +0.5f, // 10
		+0.5f, -0.5f, -0.5f, // 11
		-0.5f, +0.5f, +0.5f, // 12
		-0.5f, +0.5f, -0.5f, // 13
		-0.5f, -0.5f, -0.5f, // 14
		-0.5f, -0.5f, +0.5f, // 15
		+0.5f, +0.5f, +0.5f, // 16
		-0.5f, +0.5f, +0.5f, // 17
		-0.5f, -0.5f, +0.5f, // 18
		+0.5f, -0.5f, +0.5f, // 19
		+0.5f, -0.5f, -0.5f, // 20
		-0.5f, -0.5f, -0.5f, // 21
		-0.5f, -0.5f, +0.5f, // 22
		+0.5f, -0.5f, +0.5f, // 23
};

unsigned int cubeIndicesData[] = {
	0,   1,  2,  0,  2,  3, // Top
	4,   5,  6,  4,  6,  7, // Back
	8,   9, 10,  8, 10, 11, // Right
	12, 13, 14, 12, 14, 15, // Left
	16, 17, 18, 16, 18, 19, // Front
	20, 22, 21, 20, 23, 22, // Bottom
};

void GyzmosRenderer::create()
{


	gyzmosLineShader.loadShaderProgramFromFile(RESOURCES_PATH "gyzmosLinesShader.vert",
		RESOURCES_PATH "gyzmosCubeShader.frag");
	GET_UNIFORM(gyzmosLineShader, u_gyzmosLineShaderViewProjection);


	gyzmosCubeShader.loadShaderProgramFromFile(RESOURCES_PATH "gyzmosCubeShader.vert",
		RESOURCES_PATH "gyzmosCubeShader.frag");

	GET_UNIFORM(gyzmosCubeShader, u_viewProjection);
	GET_UNIFORM(gyzmosCubeShader, u_positionInt);
	GET_UNIFORM(gyzmosCubeShader, u_positionFloat);
	

	glCreateVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glCreateBuffers(1, &vertexDataBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexDataBuffer);
	glBufferStorage(GL_ARRAY_BUFFER, sizeof(cubePositions), cubePositions, 0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glVertexAttribDivisor(0, 0);

	glCreateBuffers(1, &blockPositionBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, blockPositionBuffer);
	glBufferData(GL_ARRAY_BUFFER, 0, 0, GL_STREAM_DRAW);
	glVertexAttribIPointer(1, 3, GL_INT, 0, 0);
	glEnableVertexAttribArray(1);
	glVertexAttribDivisor(1, 1);

	glCreateBuffers(1, &cubeIndices);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeIndices);
	glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndicesData), (void*)cubeIndicesData, 0);



	glCreateVertexArrays(1, &vaoLines);
	glBindVertexArray(vaoLines);

	glCreateBuffers(1, &vertexDataBufferLines);
	glBindBuffer(GL_ARRAY_BUFFER, vertexDataBufferLines);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

}


void GyzmosRenderer::render(Camera &c, glm::ivec3 posInt, glm::vec3 posFloat)
{


	if (!cubes.empty()) 
	{
		auto mvp = c.getProjectionMatrix() * glm::lookAt({0,0,0}, c.viewDirection, c.up);

		glDisable(GL_CULL_FACE);

		glNamedBufferData(blockPositionBuffer, cubes.size() * sizeof(CubeData), cubes.data(), GL_STATIC_DRAW);

		gyzmosCubeShader.bind();

		glDepthFunc(GL_LEQUAL);
		glBindVertexArray(vao);

		glUniformMatrix4fv(u_viewProjection, 1, GL_FALSE, &mvp[0][0]);
		glUniform3fv(u_positionFloat, 1, &posFloat[0]);
		glUniform3iv(u_positionInt, 1, &posInt[0]);

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0, cubes.size());
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glBindVertexArray(0);
		glDepthFunc(GL_LESS);

		glEnable(GL_CULL_FACE);
	
		cubes.clear();

	}

	if (!lines.empty())
	{
		glLineWidth(10);

		auto mvp = c.getProjectionMatrix() * 
			glm::lookAt( glm::vec3(c.position), glm::vec3(c.position)
			+ c.viewDirection, c.up);


		glNamedBufferData(vertexDataBufferLines, lines.size() * 
			sizeof(LinesData), lines.data(), GL_STREAM_DRAW);

		gyzmosLineShader.bind();

		glDepthFunc(GL_LEQUAL);
		glBindVertexArray(vaoLines);


		glUniformMatrix4fv(u_gyzmosLineShaderViewProjection, 1, GL_FALSE, &mvp[0][0]);

		glDrawArrays(GL_LINES, 0, lines.size()*2);

		glBindVertexArray(0);
		glDepthFunc(GL_LESS);


		lines.clear();
	}

}



void PointDebugRenderer::create()
{
	pointDebugShader.loadShaderProgramFromFile(RESOURCES_PATH "pointDebugShader.vert",
		RESOURCES_PATH "pointDebugShader.frag");

	GET_UNIFORM(pointDebugShader, u_viewProjection);
	GET_UNIFORM(pointDebugShader, u_positionInt);
	GET_UNIFORM(pointDebugShader, u_positionFloat);
	GET_UNIFORM(pointDebugShader, u_blockPositionInt);
	GET_UNIFORM(pointDebugShader, u_blockPositionFloat);


	
}

void PointDebugRenderer::renderPoint(Camera &c, glm::dvec3 point)
{
	pointDebugShader.bind();

	auto mvp = c.getProjectionMatrix() * glm::lookAt({0,0,0}, c.viewDirection, c.up);

	glm::vec3 posFloat = {};
	glm::ivec3 posInt = {};
	c.decomposePosition(posFloat, posInt);

	glUniformMatrix4fv(u_viewProjection, 1, GL_FALSE, &mvp[0][0]);
	glUniform3fv(u_positionFloat, 1, &posFloat[0]);
	glUniform3iv(u_positionInt, 1, &posInt[0]);

	glm::vec3 posFloatBlock = {};
	glm::ivec3 posIntBlock = {};
	decomposePosition(point, posFloatBlock, posIntBlock);

	glUniform3fv(u_blockPositionFloat, 1, &posFloatBlock[0]);
	glUniform3iv(u_blockPositionInt, 1, &posIntBlock[0]);

	glPointSize(15);
	glDrawArrays(GL_POINTS, 0, 1);

}

void PointDebugRenderer::renderCubePoint(Camera &c, glm::dvec3 point)
{
	renderPoint(c, point);
	
	renderPoint(c, point + glm::dvec3(0.5,0.5,-0.5));
	renderPoint(c, point + glm::dvec3(0.5,0.5,0.5));
	renderPoint(c, point + glm::dvec3(-0.5,0.5,-0.5));
	renderPoint(c, point + glm::dvec3(-0.5,0.5,0.5));

	renderPoint(c, point + glm::dvec3(0.5, -0.5, -0.5));
	renderPoint(c, point + glm::dvec3(0.5, -0.5, 0.5));
	renderPoint(c, point + glm::dvec3(-0.5, -0.5, -0.5));
	renderPoint(c, point + glm::dvec3(-0.5, -0.5, 0.5));

}


#undef GET_UNIFORM

void Renderer::FBO::create(GLint addColor, bool addDepth,
	GLint addSecondaryRenderTarget, GLint addThirdRenderTarget)
{
	if (addColor == 1) { addColor = GL_RGBA8; }
	if (addSecondaryRenderTarget == 1) { addSecondaryRenderTarget = GL_RGBA8; }
	if (addThirdRenderTarget == 1) { addThirdRenderTarget = GL_RGBA8; }

	colorFormat = addColor;
	secondaryColorFormat = addSecondaryRenderTarget;
	thirdColorFormat = addThirdRenderTarget;


	permaAssert(!(addColor == 0 && addSecondaryRenderTarget != 0));
	permaAssert(!(addSecondaryRenderTarget == 0 && addThirdRenderTarget != 0));


	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);


	if (addColor)
	{
		glGenTextures(1, &color);
		glBindTexture(GL_TEXTURE_2D, color);
		glTexImage2D(GL_TEXTURE_2D, 0, colorFormat, 1, 1
			, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
	}

	if (addSecondaryRenderTarget)
	{
		glGenTextures(1, &secondaryColor);
		glBindTexture(GL_TEXTURE_2D, secondaryColor);
		// Set the width and height of your texture (e.g., 1024x1024)
		glTexImage2D(GL_TEXTURE_2D, 0, secondaryColorFormat, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		// Attach the secondary color texture to the framebuffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, secondaryColor, 0);

		unsigned int attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
		glDrawBuffers(2, attachments);
	}

	if (addThirdRenderTarget)
	{
		glGenTextures(1, &thirdColor);
		glBindTexture(GL_TEXTURE_2D, thirdColor);
		
		GLenum internalFormat = GL_RGB;
		if (thirdColorFormat == GL_RGB16UI) { internalFormat = GL_RGB_INTEGER; }

		glTexImage2D(GL_TEXTURE_2D, 0, thirdColorFormat, 1, 1, 0, internalFormat, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, secondaryColor, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, thirdColor, 0);

		unsigned int attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
			GL_COLOR_ATTACHMENT2};
		glDrawBuffers(3, attachments);
	}
	

	if (addDepth)
	{
		glGenTextures(1, &depth);
		glBindTexture(GL_TEXTURE_2D, depth);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, 1, 1,
			0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

		if (!addColor)
		{
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
		}

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0);
	}

	// Check for framebuffer completeness
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Framebuffer Error!!!\n";
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (addSecondaryRenderTarget)
	{
		glGenFramebuffers(1, &fboOnlyFirstTarget);
		glBindFramebuffer(GL_FRAMEBUFFER, fboOnlyFirstTarget);

		if (addColor)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
		}

		if (addDepth)
		{
			if (!addColor)
			{
				glDrawBuffer(GL_NONE);
				glReadBuffer(GL_NONE);
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0);
		}

		// Check for framebuffer completeness
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "Framebuffer Error!!!\n";
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	if (addSecondaryRenderTarget)
	{
		glGenFramebuffers(1, &fboOnlySecondTarget);
		glBindFramebuffer(GL_FRAMEBUFFER, fboOnlySecondTarget);

		if (addColor)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, secondaryColor, 0);
		}

		if (addDepth)
		{
			if (!addColor)
			{
				glDrawBuffer(GL_NONE);
				glReadBuffer(GL_NONE);
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0);
		}

		// Check for framebuffer completeness
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "Framebuffer Error!!!\n";
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	if (addThirdRenderTarget)
	{
		glGenFramebuffers(1, &fboOnlyThirdTarget);
		glBindFramebuffer(GL_FRAMEBUFFER, fboOnlyThirdTarget);

		if (addColor)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, \
				thirdColor, 0);
		}

		if (addDepth)
		{
			if (!addColor)
			{
				glDrawBuffer(GL_NONE);
				glReadBuffer(GL_NONE);
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0);
		}

		// Check for framebuffer completeness
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "Framebuffer Error!!!\n";
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}


}

void Renderer::FBO::updateSize(int x, int y)
{
	if (size.x != x || size.y != y)
	{
		if (color)
		{
			glBindTexture(GL_TEXTURE_2D, color);
			glTexImage2D(GL_TEXTURE_2D, 0, colorFormat, x, y,
				0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		}

		if (secondaryColor)
		{
			glBindTexture(GL_TEXTURE_2D, secondaryColor);
			glTexImage2D(GL_TEXTURE_2D, 0, secondaryColorFormat, x, y,
				0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		}

		if (thirdColor)
		{
			GLenum internalFormat = GL_RGB;
			if (thirdColorFormat == GL_RGB16UI) { internalFormat = GL_RGB_INTEGER; }

			glBindTexture(GL_TEXTURE_2D, thirdColor);
			glTexImage2D(GL_TEXTURE_2D, 0, thirdColorFormat, x, y,
				0, internalFormat, GL_UNSIGNED_BYTE, NULL);
		}
		
		if (depth)
		{
			glBindTexture(GL_TEXTURE_2D, depth);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, x, y,
				0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		}

		size = glm::ivec2(x, y);
	}
}

void Renderer::FBO::copyDepthFromMainFBO(int w, int h)
{
	copyDepthFromOtherFBO(0, w, h);
}

void Renderer::FBO::copyColorFromMainFBO(int w, int h)
{
	copyColorFromOtherFBO(0, w, h);
}

void Renderer::FBO::copyDepthFromOtherFBO(GLuint other, int w, int h)
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glClear(GL_DEPTH_BUFFER_BIT);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, other);
	//glBindTexture(GL_TEXTURE_2D, depth);

	glBlitFramebuffer(
		0, 0, w, h,  // Source rectangle (usually the whole screen)
		0, 0, w, h,  // Destination rectangle (the size of your texture)
		GL_DEPTH_BUFFER_BIT, GL_NEAREST// You can adjust the filter mode as needed
	);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::FBO::copyColorFromOtherFBO(GLuint other, int w, int h)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, other);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

	glBlitFramebuffer(
		0, 0, w, h,  // Source rectangle (usually the whole screen)
		0, 0, w, h,  // Destination rectangle (the size of your texture)
		GL_COLOR_BUFFER_BIT, GL_NEAREST// You can adjust the filter mode as needed
	);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void Renderer::FBO::copyDepthAndColorFromOtherFBO(GLuint other, int w, int h)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, other);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

	glBlitFramebuffer(
		0, 0, w, h,  // Source rectangle (usually the whole screen)
		0, 0, w, h,  // Destination rectangle (the size of your texture)
		GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST// You can adjust the filter mode as needed
	);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::FBO::writeAllToOtherFbo(GLuint other, int w, int h)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, other);

	glBlitFramebuffer(
		0, 0, w, h,  // Source rectangle (usually the whole screen)
		0, 0, w, h,  // Destination rectangle (the size of your texture)
		GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST// You can adjust the filter mode as needed
	);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::FBO::clearFBO()
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);


	if (depth && color)
	{
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	}
	else if (depth)
	{
		glClear(GL_DEPTH_BUFFER_BIT);
	}
	else if (color)
	{
		glClear(GL_COLOR_BUFFER_BIT);
	}

	if (secondaryColor)
	{
		const float clearColor2[] = {0.0f, 0.0f, 0.0f, 0.0f};
		glClearBufferfv(GL_COLOR, 1, clearColor2);
	}

	if (thirdColor)
	{
		const float clearColor3[] = {0.0f, 0.0f, 0.0f, 0.0f};
		glClearBufferfv(GL_COLOR, 2, clearColor3);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
