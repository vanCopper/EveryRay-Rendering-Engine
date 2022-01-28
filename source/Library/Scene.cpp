#include "stdafx.h"
#include <cstdlib>
#include <stdio.h>
#include <fstream>
#include <iostream>

#include "Scene.h"
#include "GameException.h"
#include "Utility.h"
#include "Model.h"
#include "Material.h"
#include "RenderingObject.h"
#include "MaterialHelper.h"
#include "DepthMapMaterial.h"
#include "StandardLightingMaterial.h"
#include "DeferredMaterial.h"
#include "ParallaxMappingTestMaterial.h"
#include "VoxelizationGIMaterial.h"
#include "Illumination.h"
#include "ER_IlluminationProbeManager.h"

namespace Library 
{
	Scene::Scene(Game& pGame, Camera& pCamera, std::string path) :
		GameComponent(pGame), mCamera(pCamera), mScenePath(path)
	{
		Json::Reader reader;
		std::ifstream scene(path.c_str(), std::ifstream::binary);

		if (!reader.parse(scene, root)) {
			throw GameException(reader.getFormattedErrorMessages().c_str());
		}
		else {

			if (root.isMember("camera_position")) {
				float vec3[3];
				for (Json::Value::ArrayIndex i = 0; i != root["camera_position"].size(); i++)
					vec3[i] = root["camera_position"][i].asFloat();

				cameraPosition = XMFLOAT3(vec3[0], vec3[1], vec3[2]);
			}

			if (root.isMember("camera_direction")) {
				float vec3[3];
				for (Json::Value::ArrayIndex i = 0; i != root["camera_direction"].size(); i++)
					vec3[i] = root["camera_direction"][i].asFloat();

				cameraDirection = XMFLOAT3(vec3[0], vec3[1], vec3[2]);
			}

			if (root.isMember("sun_direction")) {
				float vec3[3];
				for (Json::Value::ArrayIndex i = 0; i != root["sun_direction"].size(); i++)
					vec3[i] = root["sun_direction"][i].asFloat();

				sunDirection = XMFLOAT3(vec3[0], vec3[1], vec3[2]);
			}

			if (root.isMember("sun_color")) {
				float vec3[3];
				for (Json::Value::ArrayIndex i = 0; i != root["sun_color"].size(); i++)
					vec3[i] = root["sun_color"][i].asFloat();

				sunColor = XMFLOAT3(vec3[0], vec3[1], vec3[2]);
			}

			if (root.isMember("ambient_color")) {
				float vec3[3];
				for (Json::Value::ArrayIndex i = 0; i != root["ambient_color"].size(); i++)
					vec3[i] = root["ambient_color"][i].asFloat();

				ambientColor = XMFLOAT3(vec3[0], vec3[1], vec3[2]);
			}

			if (root.isMember("skybox_path")) {
				skyboxPath = root["skybox_path"].asString();
			}

			if (root.isMember("light_probes_volume_bounds_min")) {
				float vec3[3];
				for (Json::Value::ArrayIndex i = 0; i != root["light_probes_volume_bounds_min"].size(); i++)
					vec3[i] = root["light_probes_volume_bounds_min"][i].asFloat();

				mLightProbesVolumeMinBounds = XMFLOAT3(vec3[0], vec3[1], vec3[2]);
			}

			if (root.isMember("light_probes_volume_bounds_max")) {
				float vec3[3];
				for (Json::Value::ArrayIndex i = 0; i != root["light_probes_volume_bounds_max"].size(); i++)
					vec3[i] = root["light_probes_volume_bounds_max"][i].asFloat();

				mLightProbesVolumeMaxBounds = XMFLOAT3(vec3[0], vec3[1], vec3[2]);
			}

			// add rendering objects to scene
			unsigned int numRenderingObjects = root["rendering_objects"].size();
			for (Json::Value::ArrayIndex i = 0; i != numRenderingObjects; i++) {
				std::string name = root["rendering_objects"][i]["name"].asString();
				std::string modelPath = root["rendering_objects"][i]["model_path"].asString();
				bool isInstanced = root["rendering_objects"][i]["instanced"].asBool();
				objects.emplace(name, new Rendering::RenderingObject(name, i, *mGame, mCamera, std::unique_ptr<Model>(new Model(*mGame, Utility::GetFilePath(modelPath), true)), true, isInstanced));
			}

			assert(numRenderingObjects == objects.size());

			int numThreads = std::thread::hardware_concurrency();
			int objectsPerThread = numRenderingObjects / numThreads;
			if (objectsPerThread == 0)
			{
				numThreads = 1;
				objectsPerThread = numRenderingObjects;
			}

			std::vector<std::thread> threads;
			threads.reserve(numThreads);
			
			for (int i = 0; i < numThreads; i++)
			{
				threads.push_back(std::thread([&, numThreads, numRenderingObjects, objectsPerThread, i]
				{
					int endRange = (i < numThreads - 1) ? (i + 1) * objectsPerThread : numRenderingObjects;
					
					for (int j = i * objectsPerThread; j < endRange; j++)
					{
						auto objectI = objects.begin();
						std::advance(objectI, j);
						LoadRenderingObjectData(objectI->second);
					}
				}));
			}
			for (auto& t : threads) t.join();
			
			for (auto& obj : objects)
				LoadRenderingObjectInstancedData(obj.second);
		}
	}

	Scene::~Scene()
	{
		for (auto object : objects)
		{
			object.second->MeshMaterialVariablesUpdateEvent->RemoveAllListeners();
			DeleteObject(object.second);
		}
		objects.clear();
	}

	void Scene::LoadRenderingObjectData(Rendering::RenderingObject* aObject)
	{
		if (!aObject)
			return;

		int i = aObject->GetIndexInScene();
		bool isInstanced = aObject->IsInstanced();
		bool hasLODs = false;

		// load flags
		{
			if (root["rendering_objects"][i].isMember("foliageMask"))
				aObject->SetFoliageMask(root["rendering_objects"][i]["foliageMask"].asBool());
			if (root["rendering_objects"][i].isMember("inLightProbe"))
				aObject->SetInLightProbe(root["rendering_objects"][i]["inLightProbe"].asBool());
			if (root["rendering_objects"][i].isMember("placed_on_terrain")) {
				aObject->SetPlacedOnTerrain(root["rendering_objects"][i]["placed_on_terrain"].asBool());
				if (isInstanced && root["rendering_objects"][i].isMember("num_instances_per_vegetation_zone"))
					aObject->SetNumInstancesPerVegetationZone(root["rendering_objects"][i]["num_instances_per_vegetation_zone"].asInt());
			}
			if (root["rendering_objects"][i].isMember("min_scale"))
				aObject->SetMinScale(root["rendering_objects"][i]["min_scale"].asFloat());
			if (root["rendering_objects"][i].isMember("max_scale"))
				aObject->SetMaxScale(root["rendering_objects"][i]["max_scale"].asFloat());
		}

		// load materials
		{
			if (root["rendering_objects"][i].isMember("materials")) {

				unsigned int numMaterials = root["rendering_objects"][i]["materials"].size();
				for (Json::Value::ArrayIndex matIndex = 0; matIndex != numMaterials; matIndex++) {
					std::tuple<Material*, Effect*, std::string> materialData = CreateMaterialData(
						root["rendering_objects"][i]["materials"][matIndex]["name"].asString(),
						root["rendering_objects"][i]["materials"][matIndex]["effect"].asString(),
						root["rendering_objects"][i]["materials"][matIndex]["technique"].asString()
					);

					if (std::get<2>(materialData) == MaterialHelper::shadowMapMaterialName) {
						for (int cascade = 0; cascade < NUM_SHADOW_CASCADES; cascade++)
						{
							const std::string name = MaterialHelper::shadowMapMaterialName + " " + std::to_string(cascade);
							aObject->LoadMaterial(new DepthMapMaterial(), std::get<1>(materialData), name);
							auto material = aObject->GetMaterials()[name];
							if (material)
								material->SetCurrentTechnique(material->GetEffect()->TechniquesByName().at(root["rendering_objects"][i]["materials"][matIndex]["technique"].asString()));
						}
					}
					else if (std::get<2>(materialData) == MaterialHelper::voxelizationGIMaterialName) {
						for (int cascade = 0; cascade < NUM_VOXEL_GI_CASCADES; cascade++)
						{
							const std::string name = MaterialHelper::voxelizationGIMaterialName + "_" + std::to_string(cascade);
							aObject->LoadMaterial(new Rendering::VoxelizationGIMaterial(), std::get<1>(materialData), name);
							auto material = aObject->GetMaterials()[name];
							if (material)
								material->SetCurrentTechnique(material->GetEffect()->TechniquesByName().at(root["rendering_objects"][i]["materials"][matIndex]["technique"].asString()));
						}
					}
					else if (std::get<2>(materialData) == MaterialHelper::forwardLightingForProbesMaterialName) {
						for (int cubemapFaceIndex = 0; cubemapFaceIndex < CUBEMAP_FACES_COUNT; cubemapFaceIndex++)
						{
							std::string name;
							//diffuse
							{
								name = "diffuse_" + MaterialHelper::forwardLightingForProbesMaterialName + "_" + std::to_string(cubemapFaceIndex);
								aObject->LoadMaterial(new Rendering::StandardLightingMaterial(), std::get<1>(materialData), name);
								auto material = aObject->GetMaterials()[name];
								if (material)
									material->SetCurrentTechnique(material->GetEffect()->TechniquesByName().at(root["rendering_objects"][i]["materials"][matIndex]["technique"].asString()));
							}
							//specular
							{
								name = "specular_" + MaterialHelper::forwardLightingForProbesMaterialName + "_" + std::to_string(cubemapFaceIndex);
								aObject->LoadMaterial(new Rendering::StandardLightingMaterial(), std::get<1>(materialData), name);
								auto material = aObject->GetMaterials()[name];
								if (material)
									material->SetCurrentTechnique(material->GetEffect()->TechniquesByName().at(root["rendering_objects"][i]["materials"][matIndex]["technique"].asString()));
							}
						}
					}
					else if (std::get<0>(materialData))
					{
						aObject->LoadMaterial(std::get<0>(materialData), std::get<1>(materialData), std::get<2>(materialData));
						aObject->GetMaterials()[std::get<2>(materialData)]->SetCurrentTechnique(
							aObject->GetMaterials()[std::get<2>(materialData)]->GetEffect()->TechniquesByName().at(root["rendering_objects"][i]["materials"][matIndex]["technique"].asString())
						);
					}

					//TODO maybe parse from a separate flag instead?
					if (std::get<2>(materialData) == MaterialHelper::forwardLightingMaterialName)
						aObject->SetForwardShading(true);
				}
				aObject->LoadRenderBuffers();
			}
		}

		// load custom textures
		{
			if (root["rendering_objects"][i].isMember("textures")) {

				for (Json::Value::ArrayIndex mesh = 0; mesh != root["rendering_objects"][i]["textures"].size(); mesh++) {
					if (root["rendering_objects"][i]["textures"][mesh].isMember("albedo"))
						aObject->mCustomAlbedoTextures[mesh] = root["rendering_objects"][i]["textures"][mesh]["albedo"].asString();
					if (root["rendering_objects"][i]["textures"][mesh].isMember("normal"))
						aObject->mCustomNormalTextures[mesh] = root["rendering_objects"][i]["textures"][mesh]["normal"].asString();
					if (root["rendering_objects"][i]["textures"][mesh].isMember("roughness"))
						aObject->mCustomRoughnessTextures[mesh] = root["rendering_objects"][i]["textures"][mesh]["roughness"].asString();
					if (root["rendering_objects"][i]["textures"][mesh].isMember("metalness"))
						aObject->mCustomMetalnessTextures[mesh] = root["rendering_objects"][i]["textures"][mesh]["metalness"].asString();

					aObject->LoadCustomMeshTextures(mesh);
				}
			}
		}

		// load world transform
		{
			if (root["rendering_objects"][i].isMember("transform")) {
				if (root["rendering_objects"][i]["transform"].size() != 16)
				{
					aObject->SetTransformationMatrix(XMMatrixIdentity());
				}
				else {
					float matrix[16];
					for (Json::Value::ArrayIndex matC = 0; matC != root["rendering_objects"][i]["transform"].size(); matC++) {
						matrix[matC] = root["rendering_objects"][i]["transform"][matC].asFloat();
					}
					XMFLOAT4X4 worldTransform(matrix);
					aObject->SetTransformationMatrix(XMMatrixTranspose(XMLoadFloat4x4(&worldTransform)));
				}
			}
			else
				aObject->SetTransformationMatrix(XMMatrixIdentity());
		}

		// load lods
		{
			hasLODs = root["rendering_objects"][i].isMember("model_lods");
			if (hasLODs) {
				for (Json::Value::ArrayIndex lod = 1 /* 0 is main model loaded before */; lod != root["rendering_objects"][i]["model_lods"].size(); lod++) {
					std::string path = root["rendering_objects"][i]["model_lods"][lod]["path"].asString();
					aObject->LoadLOD(std::unique_ptr<Model>(new Model(*mGame, Utility::GetFilePath(path), true)));
				}
			}
		}
	}

	// [WARNING] NOT THREAD-SAFE!
	void Scene::LoadRenderingObjectInstancedData(Rendering::RenderingObject* aObject)
	{
		int i = aObject->GetIndexInScene();
		bool isInstanced = aObject->IsInstanced();
		bool hasLODs = root["rendering_objects"][i].isMember("model_lods");
		if (isInstanced) {
			if (hasLODs)
			{
				for (int lod = 0; lod < root["rendering_objects"][i]["model_lods"].size(); lod++)
				{
					aObject->LoadInstanceBuffers(lod);
					if (root["rendering_objects"][i].isMember("instances_transforms")) {
						aObject->ResetInstanceData(root["rendering_objects"][i]["instances_transforms"].size(), true, lod);
						for (Json::Value::ArrayIndex instance = 0; instance != root["rendering_objects"][i]["instances_transforms"].size(); instance++) {
							float matrix[16];
							for (Json::Value::ArrayIndex matC = 0; matC != root["rendering_objects"][i]["instances_transforms"][instance]["transform"].size(); matC++) {
								matrix[matC] = root["rendering_objects"][i]["instances_transforms"][instance]["transform"][matC].asFloat();
							}
							XMFLOAT4X4 worldTransform(matrix);
							aObject->AddInstanceData(XMMatrixTranspose(XMLoadFloat4x4(&worldTransform)), lod);
						}
					}
					else {
						aObject->ResetInstanceData(1, true, lod);
						aObject->AddInstanceData(aObject->GetTransformationMatrix(), lod);
					}
					aObject->UpdateInstanceBuffer(aObject->GetInstancesData(), lod);
				}
			}
			else {
				aObject->LoadInstanceBuffers();
				if (root["rendering_objects"][i].isMember("instances_transforms")) {
					aObject->ResetInstanceData(root["rendering_objects"][i]["instances_transforms"].size(), true);
					for (Json::Value::ArrayIndex instance = 0; instance != root["rendering_objects"][i]["instances_transforms"].size(); instance++) {
						float matrix[16];
						for (Json::Value::ArrayIndex matC = 0; matC != root["rendering_objects"][i]["instances_transforms"][instance]["transform"].size(); matC++) {
							matrix[matC] = root["rendering_objects"][i]["instances_transforms"][instance]["transform"][matC].asFloat();
						}
						XMFLOAT4X4 worldTransform(matrix);
						aObject->AddInstanceData(XMMatrixTranspose(XMLoadFloat4x4(&worldTransform)));
					}
				}
				else {
					aObject->ResetInstanceData(1, true);
					aObject->AddInstanceData(aObject->GetTransformationMatrix());
				}
				aObject->UpdateInstanceBuffer(aObject->GetInstancesData());
			}
		}
	}

	void Scene::SaveRenderingObjectsTransforms()
	{
		if (mScenePath.empty())
			throw GameException("Can't save to scene json file! Empty scene name...");

		// store world transform
		for (Json::Value::ArrayIndex i = 0; i != root["rendering_objects"].size(); i++) {
			Json::Value content(Json::arrayValue);
			if (root["rendering_objects"][i].isMember("transform")) {
				XMFLOAT4X4 mat = objects[root["rendering_objects"][i]["name"].asString()]->GetTransformationMatrix4X4();
				XMMATRIX matXM  = XMMatrixTranspose(XMLoadFloat4x4(&mat));
				XMStoreFloat4x4(&mat, matXM);
				float matF[16];
				MatrixHelper::GetFloatArray(mat, matF);
				for (int i = 0; i < 16; i++)
					content.append(matF[i]);
				root["rendering_objects"][i]["transform"] = content;
			}
			else
			{
				float matF[16];
				XMFLOAT4X4 mat;
				XMStoreFloat4x4(&mat, XMMatrixTranspose(XMMatrixIdentity()));
				MatrixHelper::GetFloatArray(mat, matF);

				for (int i = 0; i < 16; i++)
					content.append(matF[i]);
				root["rendering_objects"][i]["transform"] = content;
			}

			if (root["rendering_objects"][i].isMember("instanced")) 
			{
				bool isInstanced = root["rendering_objects"][i]["instanced"].asBool();
				if (isInstanced)
				{
					if (root["rendering_objects"][i].isMember("instances_transforms")) {

						if (root["rendering_objects"][i]["instances_transforms"].size() != objects[root["rendering_objects"][i]["name"].asString()]->GetInstanceCount())
							throw GameException("Can't save instances transforms to scene json file! RenderObject's instance count is not equal to the number of instance transforms in scene file.");

						for (Json::Value::ArrayIndex instance = 0; instance != root["rendering_objects"][i]["instances_transforms"].size(); instance++) {
							Json::Value contentInstanceTransform(Json::arrayValue);

							XMFLOAT4X4 mat = objects[root["rendering_objects"][i]["name"].asString()]->GetInstancesData()[instance].World;
							XMMATRIX matXM = XMMatrixTranspose(XMLoadFloat4x4(&mat));
							XMStoreFloat4x4(&mat, matXM);
							float matF[16];
							MatrixHelper::GetFloatArray(mat, matF);
							for (int i = 0; i < 16; i++)
								contentInstanceTransform.append(matF[i]);

							root["rendering_objects"][i]["instances_transforms"][instance]["transform"] = contentInstanceTransform;
						}
					}
				}
			}
		}

		Json::StreamWriterBuilder builder;
		std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());

		std::ofstream file_id;
		file_id.open(mScenePath.c_str());
		writer->write(root, &file_id);
	}

	Library::Material* Scene::GetMaterial(const std::string& materialName)
	{
		//TODO load from materials container
		return nullptr;
	}

	std::tuple<Material*, Effect*, std::string> Scene::CreateMaterialData(const std::string& matName, const std::string& effectName, const std::string& techniqueName)
	{
		Material* material = nullptr;

		Effect* effect = new Effect(*mGame);
		effect->CompileFromFile(Utility::GetFilePath(Utility::ToWideString(effectName)));

		std::string materialName = "";

		// cant do reflection in C++
		if (matName == "DepthMapMaterial") {
			materialName = MaterialHelper::shadowMapMaterialName;
			//material = new DepthMapMaterial(); //processed later as we need cascades
		}
		else if (matName == "DeferredMaterial") {
			materialName = MaterialHelper::deferredPrepassMaterialName;
			material = new DeferredMaterial();
		}
		else if (matName == "StandardLightingMaterial") {
			materialName = MaterialHelper::forwardLightingMaterialName;
			material = new Rendering::StandardLightingMaterial();
		}
		else if (matName == "StandardLightingForProbeMaterial") {
			materialName = MaterialHelper::forwardLightingForProbesMaterialName;
			//material = new Rendering::StandardLightingMaterial(); //processed later as we have cubemap faces
		}
		else if (matName == "ParallaxMappingTestMaterial") {
			materialName = MaterialHelper::parallaxMaterialName;
			material = new Rendering::ParallaxMappingTestMaterial();
		}
		else if (matName == "VoxelizationGIMaterial") {
			materialName = MaterialHelper::voxelizationGIMaterialName;
			//material = new Rendering::VoxelizationGIMaterial();//processed later as we need cascades
		}
		else
			material = nullptr;

		return std::tuple<Material*, Effect*, std::string>(material, effect, materialName);
	}
}