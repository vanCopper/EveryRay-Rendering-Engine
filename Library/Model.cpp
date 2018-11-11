#include "stdafx.h"

#include "Model.h"
#include "Game.h"
#include "GameException.h"
#include "Mesh.h"
#include "ModelMaterial.h"

#include "..\assimp\Importer.hpp"
#include "..\assimp\scene.h"
#include "..\assimp\postprocess.h"

namespace Library
{
	Model::Model(Game& game, const std::string& filename, bool flipUVs)
		: mGame(game), mMeshes(), mMaterials()
	{
		Assimp::Importer importer;

		UINT flags = aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType | aiProcess_FlipWindingOrder;
		if (flipUVs)
		{
			flags |= aiProcess_FlipUVs;
		}

		const aiScene* scene = importer.ReadFile(filename, flags);
		
		if (scene == nullptr)
		{
			throw GameException(importer.GetErrorString());
		}

		if (scene->HasMaterials())
		{
			for (UINT i = 0; i < scene->mNumMaterials; i++)
			{
				mMaterials.push_back(new ModelMaterial(*this, scene->mMaterials[i]));
			}
		}

		if (scene->HasMeshes())
		{
			for (UINT i = 0; i < scene->mNumMeshes; i++)
			{
				ModelMaterial* material = (mMaterials.size() > i ? mMaterials.at(i) : nullptr);

				Mesh* mesh = new Mesh(*this, *(scene->mMeshes[i]));
				mMeshes.push_back(mesh);
			}
		}
	}

	Model::~Model()
	{
		for (Mesh* mesh : mMeshes)
		{
			delete mesh;
		}

		for (ModelMaterial* material : mMaterials)
		{
			delete material;
		}
	}

	Game& Model::GetGame()
	{
		return mGame;
	}

	bool Model::HasMeshes() const
	{
		return (mMeshes.size() > 0);
	}

	bool Model::HasMaterials() const
	{
		return (mMaterials.size() > 0);
	}

	const std::vector<Mesh*>& Model::Meshes() const
	{
		return mMeshes;
	}

	const std::vector<ModelMaterial*>& Model::Materials() const
	{
		return mMaterials;
	}

	std::vector<XMFLOAT3> Model::GenerateAABB()
	{
		std::vector<XMFLOAT3> vertices;

		for (Mesh* mesh : mMeshes)
		{
			vertices.insert(vertices.end(), mesh->Vertices().begin(), mesh->Vertices().end());
		}


		XMFLOAT3 minVertex = XMFLOAT3(FLT_MAX, FLT_MAX, FLT_MAX);
		XMFLOAT3 maxVertex = XMFLOAT3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

		for (UINT i = 0; i < vertices.size(); i++)
		{

			//Get the smallest vertex 
			minVertex.x = std::min(minVertex.x, vertices[i].x);    // Find smallest x value in model
			minVertex.y = std::min(minVertex.y, vertices[i].y);    // Find smallest y value in model
			minVertex.z = std::min(minVertex.z, vertices[i].z);    // Find smallest z value in model

			//Get the largest vertex 
			maxVertex.x = std::max(maxVertex.x, vertices[i].x);    // Find largest x value in model
			maxVertex.y = std::max(maxVertex.y, vertices[i].y);    // Find largest y value in model
			maxVertex.z = std::max(maxVertex.z, vertices[i].z);    // Find largest z value in model
		}

		std::vector<XMFLOAT3> AABB;

		// Our AABB [0] is the min vertex and [1] is the max
		AABB.push_back(minVertex);
		AABB.push_back(maxVertex);

		return AABB;
	}
}