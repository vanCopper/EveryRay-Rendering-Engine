#include "stdafx.h"

#include "TestSceneDemo.h"
#include "..\Library\Systems.inl"
#include "..\Library\Materials.inl"
#include "..\Library\Utility.h"

namespace Rendering
{
	RTTI_DEFINITIONS(TestSceneDemo)

	TestSceneDemo::TestSceneDemo(Game& game, Camera& camera, Editor& editor)
		: DrawableGameComponent(game, camera, editor)
	{
	}

	TestSceneDemo::~TestSceneDemo()
	{

	}

	bool TestSceneDemo::IsComponent()
	{
		return mGame->IsInGameLevels<TestSceneDemo*>(mGame->levels, this);
	}
	void TestSceneDemo::Create()
	{
		DemoLevel::Initialize(*mGame, *mCamera, Utility::GetFilePath("content\\levels\\testScene.json"));
		Initialize();
		mGame->levels.push_back(this);
	}
	void TestSceneDemo::Destroy()
	{
		DemoLevel::Destroy();
		TestSceneDemo::~TestSceneDemo();
		mGame->levels.clear();
	}

	void TestSceneDemo::Initialize()
	{
		SetCurrentDirectory(Utility::ExecutableDirectory().c_str());

        for (auto& object : mScene->objects) {
            object.second->MeshMaterialVariablesUpdateEvent->AddListener(MaterialHelper::lightingMaterialName, [&](int meshIndex) { UpdateStandardLightingPBRMaterialVariables(object.first, meshIndex); });
            object.second->MeshMaterialVariablesUpdateEvent->AddListener(MaterialHelper::deferredPrepassMaterialName, [&](int meshIndex) { UpdateDeferredPrepassMaterialVariables(object.first, meshIndex); });
            object.second->MeshMaterialVariablesUpdateEvent->AddListener(MaterialHelper::shadowMapMaterialName + " " + std::to_string(0), [&](int meshIndex) { UpdateShadow0MaterialVariables(object.first, meshIndex); });
            object.second->MeshMaterialVariablesUpdateEvent->AddListener(MaterialHelper::shadowMapMaterialName + " " + std::to_string(1), [&](int meshIndex) { UpdateShadow1MaterialVariables(object.first, meshIndex); });
            object.second->MeshMaterialVariablesUpdateEvent->AddListener(MaterialHelper::shadowMapMaterialName + " " + std::to_string(2), [&](int meshIndex) { UpdateShadow2MaterialVariables(object.first, meshIndex); });
        }

		mEditor->LoadScene(mScene);

		mFoliageSystem->AddFoliage(new Foliage(*mGame, *mCamera, *mDirectionalLight, 1500, Utility::GetFilePath("content\\textures\\foliage\\grass_type1.png"), 2.5f, 100.0f, XMFLOAT3(0.0, 0.0, 0.0), FoliageBillboardType::SINGLE));
		mFoliageSystem->AddFoliage(new Foliage(*mGame, *mCamera, *mDirectionalLight, 2000, Utility::GetFilePath("content\\textures\\foliage\\grass_type4.png"), 2.0f, 100.0f, XMFLOAT3(0.0, 0.0, 0.0), FoliageBillboardType::SINGLE));
		mFoliageSystem->AddFoliage(new Foliage(*mGame, *mCamera, *mDirectionalLight, 2000, Utility::GetFilePath("content\\textures\\foliage\\grass_type6.png"), 2.0f, 100.0f, XMFLOAT3(0.0, 0.0, 0.0), FoliageBillboardType::SINGLE));
		mFoliageSystem->AddFoliage(new Foliage(*mGame, *mCamera, *mDirectionalLight, 100, Utility::GetFilePath("content\\textures\\foliage\\grass_flower_type1.png"), 3.5f, 100.0f, XMFLOAT3(0.0, 0.0, 0.0), FoliageBillboardType::SINGLE));
		mFoliageSystem->AddFoliage(new Foliage(*mGame, *mCamera, *mDirectionalLight, 50, Utility::GetFilePath("content\\textures\\foliage\\grass_flower_type3.png"), 2.5f, 100.0f, XMFLOAT3(0.0, 0.0, 0.0), FoliageBillboardType::SINGLE));
		mFoliageSystem->AddFoliage(new Foliage(*mGame, *mCamera, *mDirectionalLight, 100, Utility::GetFilePath("content\\textures\\foliage\\grass_flower_type10.png"), 3.5f, 100.0f, XMFLOAT3(0.0, 0.0, 0.0), FoliageBillboardType::SINGLE));
		mFoliageSystem->Initialize();
	}

	void TestSceneDemo::UpdateLevel(const GameTime& gameTime)
	{
		DemoLevel::UpdateLevel(gameTime);
		mFoliageSystem->Update(gameTime, mWindGustDistance, mWindStrength, mWindFrequency);

		mDirectionalLight->UpdateProxyModel(gameTime, mCamera->ViewMatrix4X4(), mCamera->ProjectionMatrix4X4());
		mSkybox->SetUseCustomSkyColor(mEditor->IsSkyboxUsingCustomColor());
		mSkybox->SetSkyColors(mEditor->GetBottomSkyColor(), mEditor->GetTopSkyColor());
		mSkybox->SetSunData(mDirectionalLight->IsSunRendered(),
			mDirectionalLight->DirectionVector(),
			XMVECTOR{ mDirectionalLight->GetDirectionalLightColor().x, mDirectionalLight->GetDirectionalLightColor().y, mDirectionalLight->GetDirectionalLightColor().z, 1.0 },
			mDirectionalLight->GetSunBrightness(), mDirectionalLight->GetSunExponent());
		
		mEditor->Update(gameTime);
		
		UpdateImGui();
	}

	void TestSceneDemo::UpdateImGui()
	{
		ImGui::Begin("Test Demo Scene");
		ImGui::SliderFloat("Wind strength", &mWindStrength, 0.0f, 100.0f);
		ImGui::SliderFloat("Wind gust distance", &mWindGustDistance, 0.0f, 100.0f);
		ImGui::SliderFloat("Wind frequency", &mWindFrequency, 0.0f, 100.0f);
		ImGui::End();
	}

	void TestSceneDemo::DrawLevel(const GameTime& gameTime)
	{
		mRenderStateHelper->SaveRasterizerState();

		ID3D11DeviceContext* direct3DDeviceContext = mGame->Direct3DDeviceContext();
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		
		#pragma region GBUFFER_PREPASS
		
		mGBuffer->Start();

		//draw scene objects in gbuffer
		for (auto it = mScene->objects.begin(); it != mScene->objects.end(); it++)
			it->second->Draw(MaterialHelper::deferredPrepassMaterialName, true);

		//draw foliage in gbuffer
		mFoliageSystem->Draw(gameTime, nullptr, FoliageRenderingPass::TO_GBUFFER);

		mGBuffer->End();

#pragma endregion

		#pragma region DRAW_SHADOWS
		mShadowMapper->Draw(mScene);		
		mRenderStateHelper->RestoreRasterizerState();
		
#pragma endregion

		#pragma region DRAW_GI
		mRenderStateHelper->SaveAll();
		mGI->Draw(gameTime, mGBuffer);
		mRenderStateHelper->RestoreAll();
#pragma endregion

		mPostProcessingStack->Begin(true);

		#pragma region DRAW_LIGHTING
		mSkybox->Draw(gameTime);

		//gizmo
		if (Utility::IsEditorMode)
		{
			mDirectionalLight->DrawProxyModel(gameTime);
			mGI->DrawDebugGizmos();
		}

		//lighting
		for (auto it = mScene->objects.begin(); it != mScene->objects.end(); it++)
			it->second->Draw(MaterialHelper::lightingMaterialName);

		//foliage 
		mFoliageSystem->Draw(gameTime, mShadowMapper);
#pragma endregion

		mPostProcessingStack->End();

		#pragma region DRAW_SUN
		mSkybox->DrawSun(gameTime, mPostProcessingStack);
#pragma endregion

		#pragma region DRAW_VOLUMETRIC_CLOUDS
		mVolumetricClouds->Draw(gameTime);
#pragma endregion

		#pragma region DRAW_POSTPROCESSING
		mPostProcessingStack->UpdateCompositeLightingMaterial(mGI->GetGISRV(), mGI->GetDebugVoxels(), mGI->GetDebugAO());
		mPostProcessingStack->UpdateSSRMaterial(mGBuffer->GetNormals()->getSRV(), mGBuffer->GetDepth()->getSRV(), mGBuffer->GetExtraBuffer()->getSRV(), (float)gameTime.TotalGameTime());
		mPostProcessingStack->DrawEffects(gameTime);
		mPostProcessingStack->ResetMainRTtoOriginal();
#pragma endregion
		
		mRenderStateHelper->SaveAll();

		#pragma region DRAW_IMGUI
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

#pragma endregion
	}

	void TestSceneDemo::UpdateStandardLightingPBRMaterialVariables(const std::string& objectName, int meshIndex)
	{
		XMMATRIX worldMatrix = XMLoadFloat4x4(&(mScene->objects[objectName]->GetTransformationMatrix4X4()));
		XMMATRIX vp = mCamera->ViewMatrix() * mCamera->ProjectionMatrix();

		XMMATRIX shadowMatrices[3] = 
		{ 
			mShadowMapper->GetViewMatrix(0) *  mShadowMapper->GetProjectionMatrix(0) * XMLoadFloat4x4(&MatrixHelper::GetProjectionShadowMatrix()) ,
			mShadowMapper->GetViewMatrix(1) *  mShadowMapper->GetProjectionMatrix(1) * XMLoadFloat4x4(&MatrixHelper::GetProjectionShadowMatrix()) ,
			mShadowMapper->GetViewMatrix(2) *  mShadowMapper->GetProjectionMatrix(2) * XMLoadFloat4x4(&MatrixHelper::GetProjectionShadowMatrix()) 
		};

		ID3D11ShaderResourceView* shadowMaps[3] = 
		{
			mShadowMapper->GetShadowTexture(0),
			mShadowMapper->GetShadowTexture(1),
			mShadowMapper->GetShadowTexture(2)
		};

		static_cast<StandardLightingMaterial*>(mScene->objects[objectName]->GetMaterials()[MaterialHelper::lightingMaterialName])->ViewProjection() << vp;
		static_cast<StandardLightingMaterial*>(mScene->objects[objectName]->GetMaterials()[MaterialHelper::lightingMaterialName])->World() << worldMatrix;
		static_cast<StandardLightingMaterial*>(mScene->objects[objectName]->GetMaterials()[MaterialHelper::lightingMaterialName])->ShadowMatrices().SetMatrixArray(shadowMatrices, 0, NUM_SHADOW_CASCADES);
		static_cast<StandardLightingMaterial*>(mScene->objects[objectName]->GetMaterials()[MaterialHelper::lightingMaterialName])->CameraPosition() << mCamera->PositionVector();
		static_cast<StandardLightingMaterial*>(mScene->objects[objectName]->GetMaterials()[MaterialHelper::lightingMaterialName])->SunDirection() << XMVectorNegate(mDirectionalLight->DirectionVector());
		static_cast<StandardLightingMaterial*>(mScene->objects[objectName]->GetMaterials()[MaterialHelper::lightingMaterialName])->SunColor() << XMVECTOR{ mDirectionalLight->GetDirectionalLightColor().x,  mDirectionalLight->GetDirectionalLightColor().y, mDirectionalLight->GetDirectionalLightColor().z , 1.0f };
		static_cast<StandardLightingMaterial*>(mScene->objects[objectName]->GetMaterials()[MaterialHelper::lightingMaterialName])->AmbientColor() << XMVECTOR{ mDirectionalLight->GetAmbientLightColor().x,  mDirectionalLight->GetAmbientLightColor().y, mDirectionalLight->GetAmbientLightColor().z , 1.0f };
		static_cast<StandardLightingMaterial*>(mScene->objects[objectName]->GetMaterials()[MaterialHelper::lightingMaterialName])->ShadowTexelSize() << XMVECTOR{ 1.0f / mShadowMapper->GetResolution(), 1.0f, 1.0f , 1.0f };
		static_cast<StandardLightingMaterial*>(mScene->objects[objectName]->GetMaterials()[MaterialHelper::lightingMaterialName])->ShadowCascadeDistances() << XMVECTOR{ mCamera->GetCameraFarCascadeDistance(0), mCamera->GetCameraFarCascadeDistance(1), mCamera->GetCameraFarCascadeDistance(2), 1.0f };
		static_cast<StandardLightingMaterial*>(mScene->objects[objectName]->GetMaterials()[MaterialHelper::lightingMaterialName])->AlbedoTexture() << mScene->objects[objectName]->GetTextureData(meshIndex).AlbedoMap;
		static_cast<StandardLightingMaterial*>(mScene->objects[objectName]->GetMaterials()[MaterialHelper::lightingMaterialName])->NormalTexture() << mScene->objects[objectName]->GetTextureData(meshIndex).NormalMap;
		static_cast<StandardLightingMaterial*>(mScene->objects[objectName]->GetMaterials()[MaterialHelper::lightingMaterialName])->SpecularTexture() << mScene->objects[objectName]->GetTextureData(meshIndex).SpecularMap;
		static_cast<StandardLightingMaterial*>(mScene->objects[objectName]->GetMaterials()[MaterialHelper::lightingMaterialName])->RoughnessTexture() << mScene->objects[objectName]->GetTextureData(meshIndex).RoughnessMap;
		static_cast<StandardLightingMaterial*>(mScene->objects[objectName]->GetMaterials()[MaterialHelper::lightingMaterialName])->MetallicTexture() << mScene->objects[objectName]->GetTextureData(meshIndex).MetallicMap;
		static_cast<StandardLightingMaterial*>(mScene->objects[objectName]->GetMaterials()[MaterialHelper::lightingMaterialName])->CascadedShadowTextures().SetResourceArray(shadowMaps, 0, NUM_SHADOW_CASCADES);
		static_cast<StandardLightingMaterial*>(mScene->objects[objectName]->GetMaterials()[MaterialHelper::lightingMaterialName])->IrradianceDiffuseTexture() << mGI->GetIBLIrradianceDiffuseSRV();
		static_cast<StandardLightingMaterial*>(mScene->objects[objectName]->GetMaterials()[MaterialHelper::lightingMaterialName])->IrradianceSpecularTexture() << mGI->GetIBLIrradianceSpecularSRV();
		static_cast<StandardLightingMaterial*>(mScene->objects[objectName]->GetMaterials()[MaterialHelper::lightingMaterialName])->IntegrationTexture() << mGI->GetIBLIntegrationSRV();
	}
	void TestSceneDemo::UpdateDeferredPrepassMaterialVariables(const std::string & objectName, int meshIndex)
	{
		XMMATRIX worldMatrix = XMLoadFloat4x4(&(mScene->objects[objectName]->GetTransformationMatrix4X4()));
		XMMATRIX vp = /*worldMatrix * */mCamera->ViewMatrix() * mCamera->ProjectionMatrix();
		static_cast<DeferredMaterial*>(mScene->objects[objectName]->GetMaterials()[MaterialHelper::deferredPrepassMaterialName])->ViewProjection() << vp;
		static_cast<DeferredMaterial*>(mScene->objects[objectName]->GetMaterials()[MaterialHelper::deferredPrepassMaterialName])->World() << worldMatrix;
		static_cast<DeferredMaterial*>(mScene->objects[objectName]->GetMaterials()[MaterialHelper::deferredPrepassMaterialName])->AlbedoMap() << mScene->objects[objectName]->GetTextureData(meshIndex).AlbedoMap;
		static_cast<DeferredMaterial*>(mScene->objects[objectName]->GetMaterials()[MaterialHelper::deferredPrepassMaterialName])->RoughnessMap() << mScene->objects[objectName]->GetTextureData(meshIndex).RoughnessMap;
		static_cast<DeferredMaterial*>(mScene->objects[objectName]->GetMaterials()[MaterialHelper::deferredPrepassMaterialName])->ReflectionMaskFactor() << mScene->objects[objectName]->GetMeshReflectionFactor(meshIndex);
	}
	void TestSceneDemo::UpdateShadow0MaterialVariables(const std::string & objectName, int meshIndex)
	{
		const std::string name = MaterialHelper::shadowMapMaterialName + " " + std::to_string(0);
		static_cast<DepthMapMaterial*>(mScene->objects[objectName]->GetMaterials()[name])->AlbedoAlphaMap() << mScene->objects[objectName]->GetTextureData(meshIndex).AlbedoMap;
	}
	void TestSceneDemo::UpdateShadow1MaterialVariables(const std::string & objectName, int meshIndex)
	{
		const std::string name = MaterialHelper::shadowMapMaterialName + " " + std::to_string(1);
		static_cast<DepthMapMaterial*>(mScene->objects[objectName]->GetMaterials()[name])->AlbedoAlphaMap() << mScene->objects[objectName]->GetTextureData(meshIndex).AlbedoMap;
	}
	void TestSceneDemo::UpdateShadow2MaterialVariables(const std::string & objectName, int meshIndex)
	{
		const std::string name = MaterialHelper::shadowMapMaterialName + " " + std::to_string(2);
		static_cast<DepthMapMaterial*>(mScene->objects[objectName]->GetMaterials()[name])->AlbedoAlphaMap() << mScene->objects[objectName]->GetTextureData(meshIndex).AlbedoMap;
	}
}