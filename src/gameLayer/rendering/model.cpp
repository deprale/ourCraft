#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat3x3.hpp>
#include <glm/gtx/transform.hpp>
#include <rendering/model.h>
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp> // for glm::make_mat4
#include <glm/gtx/quaternion.hpp>

glm::mat4 aiToGlm(const aiMatrix4x4 &matrix)
{
	return glm::make_mat4(&matrix.a1); // a1 represents the first element of the matrix
}

const aiNode *findNodeContainingMesh(const aiNode *node, const aiMesh *mesh)
{
	for (unsigned int i = 0; i < node->mNumMeshes; ++i)
	{
		if (node->mMeshes[i] == mesh->mPrimitiveTypes)
		{
			return node;
		}
	}

	for (unsigned int i = 0; i < node->mNumChildren; ++i)
	{
		const aiNode *foundNode = findNodeContainingMesh(node->mChildren[i], mesh);
		if (foundNode)
			return foundNode;
	}

	return nullptr;
}

void ModelsManager::loadAllModels()
{

	//load default texture
	{
		unsigned char data[16] = {};

		{
			int i = 0;
			data[i++] = 0;
			data[i++] = 0;
			data[i++] = 0;
			data[i++] = 255;

			data[i++] = 146;
			data[i++] = 52;
			data[i++] = 235;
			data[i++] = 255;

			data[i++] = 146;
			data[i++] = 52;
			data[i++] = 235;
			data[i++] = 255;

			data[i++] = 0;
			data[i++] = 0;
			data[i++] = 0;
			data[i++] = 255;
		}

		gl2d::Texture t;
		t.createFromBuffer((char *)data, 2, 2, true, false);

		texturesIds.push_back(t.id);
		auto handle = glGetTextureHandleARB(t.id);
		glMakeTextureHandleResidentARB(handle);
		gpuIds.push_back(handle);
	}

	auto loadTexture = [&](const char *path)
	{
		gl2d::Texture texture = {};
		GLuint64 handle = 0;

		texture.loadFromFile(path, true, false);

		if (texture.id)
		{

			texture.bind();
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 6.f);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 6.f);

			glGenerateMipmap(GL_TEXTURE_2D);

			handle = glGetTextureHandleARB(texture.id);
			glMakeTextureHandleResidentARB(handle);
		}
		else
		{
			//todo error report
			std::cout << "Error loading: " << path << "\n";

			handle = gpuIds[0];
			texture.id = texturesIds[0];
		}

		gpuIds.push_back(handle);
		texturesIds.push_back(texture.id);
	};


	//load textures
	{
		//the order matters!!!!
		//loadTexture(RESOURCES_PATH "models/steve.png", steveTexture, steveTextureHandle);
		loadTexture(RESOURCES_PATH "models/giggasteve.png");
		loadTexture(RESOURCES_PATH "models/zombie.png");
		loadTexture(RESOURCES_PATH "models/pig.png");
		

	}


	Assimp::Importer importer;

	// Step 2: Specify Import Options
	unsigned int flags = aiProcess_Triangulate | aiProcess_LimitBoneWeights | aiProcess_JoinIdenticalVertices | aiProcess_ImproveCacheLocality | aiProcess_GenUVCoords | aiProcess_TransformUVCoords | aiProcess_FindInstances;

	importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT); // Remove lines and points
	importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_NORMALS | aiComponent_BONEWEIGHTS | aiComponent_ANIMATIONS);

	struct Data
	{
		glm::vec3 position = {};
		glm::vec3 normal = {};
		glm::vec2 uv = {};
		int boneIndex = 0;
	};

	std::vector<Data> vertexes;
	vertexes.reserve(400);

	std::vector<unsigned int> indices;
	indices.reserve(400);

	auto loadModel = [&](const char *path, Model &model)
	{
		vertexes.clear();
		indices.clear();

		const aiScene *scene = importer.ReadFile(path, flags);

		if (scene)
		{

			glGenVertexArrays(1, &model.vao);
			glBindVertexArray(model.vao);

			glGenBuffers(1, &model.geometry);
			glGenBuffers(1, &model.indexBuffer);

			glBindBuffer(GL_ARRAY_BUFFER, model.geometry);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.indexBuffer);

			unsigned int vertexOffset = 0;

			for (unsigned int i = 0; i < scene->mRootNode->mNumChildren; ++i)
			{

				const aiNode *node = scene->mRootNode->mChildren[i];

				for (int m = 0; m < node->mNumMeshes; m++)
				{
					const aiMesh *mesh = scene->mMeshes[node->mMeshes[m]];

					aiMatrix4x4 transform = node->mTransformation;
					model.transforms.push_back(glm::transpose(aiToGlm(transform)));


					for (unsigned int v = 0; v < mesh->mNumVertices; ++v)
					{
						Data vertex;

						vertex.boneIndex = i;

						// Positions
						vertex.position.x = mesh->mVertices[v].x;
						vertex.position.y = mesh->mVertices[v].y;
						vertex.position.z = mesh->mVertices[v].z;

						// Normals
						vertex.normal.x = mesh->mNormals[v].x;
						vertex.normal.y = mesh->mNormals[v].y;
						vertex.normal.z = mesh->mNormals[v].z;

						// UVs (if present)
						if (mesh->HasTextureCoords(0))
						{
							vertex.uv.x = mesh->mTextureCoords[0][v].x;
							vertex.uv.y = mesh->mTextureCoords[0][v].y;
						}
						else
						{
							vertex.uv = glm::vec2(0.0f, 0.0f);
						}

						vertexes.push_back(vertex);
					}

					for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
					{
						const aiFace &face = mesh->mFaces[i];
						for (unsigned int j = 0; j < face.mNumIndices; ++j)
						{
							indices.push_back(face.mIndices[j] + vertexOffset);
						}
					}

					vertexOffset += mesh->mNumVertices;
				}


			}


			model.vertexCount = indices.size();

			glBufferData(GL_ARRAY_BUFFER, vertexes.size() * sizeof(Data), vertexes.data(), GL_STATIC_DRAW);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 9, 0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 9, (void *)(sizeof(float) * 3));
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 9, (void *)(sizeof(float) * 6));
			glEnableVertexAttribArray(3);
			glVertexAttribIPointer(3, 1, GL_INT, sizeof(float) * 9, (void *)(sizeof(float) * 8));


			glBindVertexArray(0);

		}
		else
		{
			std::cout << "noSchene in" << path << "\n";
			std::cout << importer.GetErrorString() << "\n";
		}


	};


	loadModel(RESOURCES_PATH "models/human.glb", human);
	importer.FreeScene();

	loadModel(RESOURCES_PATH "models/pig.glb", pig);
	

	//todo check if it frees all of them
	importer.FreeScene();


	setupSSBO();
}

void ModelsManager::setupSSBO()
{

	if (!texturesSSBO)
	{
		glGenBuffers(1, &texturesSSBO);
	}

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, texturesSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, gpuIds.size() * sizeof(gpuIds[0]), gpuIds.data(), GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, texturesSSBO); //todo add some constatns here

}

void animatePlayerLegs(glm::mat4 *poseVector,
	float &currentAngle, int &direction, float deltaTime)
{

	if (direction == 0)
	{
		if (currentAngle)
		{
			if (currentAngle > 0)
			{
				currentAngle -= deltaTime;
				if (currentAngle < 0)
				{
					currentAngle = 0;
				}
			}
			else
			{
				currentAngle += deltaTime;
				if (currentAngle > 0)
				{
					currentAngle = 0;
				}
			}
		
		}
	}
	{
		currentAngle += deltaTime * direction;

		if (direction == 1)
		{
			if (currentAngle >= glm::radians(40.f))
			{
				currentAngle = glm::radians(40.f);
				direction = -1;
			}
		}
		else
		{
			if (currentAngle <= glm::radians(-40.f))
			{
				currentAngle = glm::radians(-40.f);
				direction = 1;
			}
		}

	}

	
	poseVector[0]; //head
	poseVector[1]; //torso
	poseVector[2]; //right hand
	poseVector[3]; //left hand
	poseVector[4]; //right leg
	poseVector[5]; //left leg

	poseVector[4] = poseVector[4] * glm::rotate(currentAngle, glm::vec3{1.f,0.f,0.f});
	poseVector[5] = poseVector[5] * glm::rotate(-currentAngle, glm::vec3{1.f,0.f,0.f});

}


