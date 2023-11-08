#include "stdafx.h"

#include "ER_ShadowMapper.h"
#include "ER_Frustum.h"
#include "ER_Projector.h"
#include "ER_Camera.h"
#include "ER_DirectionalLight.h"
#include "ER_CoreTime.h"
#include "ER_Core.h"
#include "ER_CoreException.h"
#include "ER_Scene.h"
#include "ER_MaterialHelper.h"
#include "ER_RenderingObject.h"
#include "ER_ShadowMapMaterial.h"
#include "ER_MaterialsCallbacks.h"
#include "ER_Terrain.h"
#include "ER_Utility.h"

#include <sstream>
#include <iomanip>
#include <algorithm>
#include <limits>

static const std::string psoNameNonInstanced = "ER_RHI_GPUPipelineStateObject: ShadowMapMaterial";
static const std::string psoNameInstanced = "ER_RHI_GPUPipelineStateObject: ShadowMapMaterial w/ Instancing";

namespace EveryRay_Core
{
	ER_ShadowMapper::ER_ShadowMapper(ER_Core& pCore, ER_Camera& camera, ER_DirectionalLight& dirLight, ShadowQuality pQuality, bool isCascaded)
		: ER_CoreComponent(pCore),
		mShadowMaps(0, nullptr), 
		mDirectionalLight(dirLight),
		mCamera(camera),
		mIsCascaded(isCascaded)
	{
		auto rhi = GetCore()->GetRHI();

		switch (pQuality)
		{
		case ShadowQuality::SHADOW_LOW:
			mResolution = 512;
			break;
		case ShadowQuality::SHADOW_MEDIUM:
			mResolution = 1024;
			break;
		case ShadowQuality::SHADOW_HIGH:
			mResolution = 2048;
			break;
		}

		for (int i = 0; i < NUM_SHADOW_CASCADES; i++)
		{
			mLightProjectorCenteredPositions.push_back(XMFLOAT3(0, 0, 0));
			
			mShadowMaps.push_back(rhi->CreateGPUTexture(L"ER_RHI_GPUTexture: Shadow Map #" + std::to_wstring(i)));
			mShadowMaps[i]->CreateGPUTextureResource(rhi, mResolution, mResolution, 1u, ER_FORMAT_D16_UNORM, ER_BIND_DEPTH_STENCIL | ER_BIND_SHADER_RESOURCE);

			mCameraCascadesFrustums.push_back(XMMatrixIdentity());
			if (isCascaded)
				mCameraCascadesFrustums[i].SetMatrix(GetCustomViewProjectionMatrixForCascade(mCamera.ViewMatrix(), mCamera.FieldOfView(), mCamera.AspectRatio(), mCamera.NearPlaneDistance(), i));
			else
				mCameraCascadesFrustums[i].SetMatrix(mCamera.ProjectionMatrix());

			mLightProjectors.emplace_back(pCore);
			mLightProjectors[i].Initialize();

			float sphereRadius = 0.0f;
			mLightProjectors[i].SetProjectionMatrix(GetProjectionBoundingSphere(i, sphereRadius));
			//mLightProjectors[i]->ApplyRotation(mDirectionalLight.GetTransform());
		}

		mRootSignature = rhi->CreateRootSignature(4, 1);
		if (mRootSignature)
		{
			mRootSignature->InitStaticSampler(rhi, 0, ER_RHI_SAMPLER_STATE::ER_TRILINEAR_WRAP, ER_RHI_SHADER_VISIBILITY_PIXEL);
			mRootSignature->InitDescriptorTable(rhi, SHADOWMAP_MAT_ROOT_DESCRIPTOR_TABLE_PIXEL_SRV_INDEX, { ER_RHI_DESCRIPTOR_RANGE_TYPE::ER_RHI_DESCRIPTOR_RANGE_TYPE_SRV }, { 0 }, { 1 }, ER_RHI_SHADER_VISIBILITY_PIXEL);
			mRootSignature->InitDescriptorTable(rhi, SHADOWMAP_MAT_ROOT_DESCRIPTOR_TABLE_VERTEX_SRV_INDEX, { ER_RHI_DESCRIPTOR_RANGE_TYPE::ER_RHI_DESCRIPTOR_RANGE_TYPE_SRV }, { 1 }, { 1 }, ER_RHI_SHADER_VISIBILITY_VERTEX);
			mRootSignature->InitDescriptorTable(rhi, SHADOWMAP_MAT_ROOT_DESCRIPTOR_TABLE_CBV_INDEX, { ER_RHI_DESCRIPTOR_RANGE_TYPE::ER_RHI_DESCRIPTOR_RANGE_TYPE_CBV }, { 0 }, { 2 }, ER_RHI_SHADER_VISIBILITY_ALL);
			mRootSignature->InitConstant(rhi, SHADOWMAP_MAT_ROOT_ROOT_CONSTANT_INDEX, 2 /*we already use 2 slots for CBVs*/, 1 /* only 1 constant for LOD index*/, ER_RHI_SHADER_VISIBILITY_ALL);
			mRootSignature->Finalize(rhi, "ER_RHI_GPURootSignature: ShadowMapMaterial Pass", true);
		}
	}

	ER_ShadowMapper::~ER_ShadowMapper()
	{
		DeletePointerCollection(mShadowMaps);

		DeleteObject(mRootSignature);
	}

	void ER_ShadowMapper::Update(const ER_CoreTime& gameTime)
	{
		ER_Frustum& cameraFrustum = mCamera.GetFrustum();
		XMFLOAT3 frustumCorners[8] = {};

		frustumCorners[0] = (cameraFrustum.Corners()[0]);
		frustumCorners[1] = (cameraFrustum.Corners()[1]);
		frustumCorners[2] = (cameraFrustum.Corners()[2]);
		frustumCorners[3] = (cameraFrustum.Corners()[3]);
		frustumCorners[4] = (cameraFrustum.Corners()[4]);
		frustumCorners[5] = (cameraFrustum.Corners()[5]);
		frustumCorners[6] = (cameraFrustum.Corners()[6]);
		frustumCorners[7] = (cameraFrustum.Corners()[7]);

		XMVECTOR direction = XMLoadFloat3(&mDirectionalLight.Direction());
		XMVECTOR upDirection = XMLoadFloat3(&mDirectionalLight.Up());

		for (int i = 0; i < NUM_SHADOW_CASCADES; i++)
		{
			if (mIsCascaded)
				mCameraCascadesFrustums[i].SetMatrix(GetCustomViewProjectionMatrixForCascade(mCamera.ViewMatrix(), mCamera.FieldOfView(), mCamera.AspectRatio(), mCamera.NearPlaneDistance(), i));
			else
				mCameraCascadesFrustums[i].SetMatrix(mCamera.ProjectionMatrix());

			float sphereRadius = 0.0f;
			XMMATRIX projectionMatrix = GetProjectionBoundingSphere(i, sphereRadius);

			if (mIsTexelSizeIncremented)
			{
				assert(sphereRadius);
				float texelUnit = static_cast<float>(mResolution) / (sphereRadius * 2.0f);
				//texelUnit *= 2.0f;
				XMMATRIX scale = XMMatrixScaling(texelUnit, texelUnit, texelUnit);

				XMMATRIX baseViewMatrix = XMMatrixLookToRH({ 0.0, 0.0, 0.0, 1.0 }, direction, upDirection);
				baseViewMatrix *= scale;
				XMMATRIX invBaseViewMatrix = XMMatrixInverse(nullptr, baseViewMatrix);

				XMVECTOR posV = XMLoadFloat3(&mLightProjectorCenteredPositions[i]);
				posV = XMVector3Transform(posV, baseViewMatrix);
				
				XMFLOAT3 pos;
				XMStoreFloat3(&pos, posV);
				pos.x = floor(pos.x);
				pos.y = floor(pos.y);
				posV = XMLoadFloat3(&pos);
				posV = XMVector3Transform(posV, invBaseViewMatrix);
				XMStoreFloat3(&mLightProjectorCenteredPositions[i], posV);
			}

			mLightProjectors[i].SetPosition(mLightProjectorCenteredPositions[i].x, mLightProjectorCenteredPositions[i].y, mLightProjectorCenteredPositions[i].z);
			mLightProjectors[i].SetProjectionMatrix(projectionMatrix);
			mLightProjectors[i].SetViewMatrix(mLightProjectorCenteredPositions[i], mDirectionalLight.Direction(), mDirectionalLight.Up());
			mLightProjectors[i].Update();
		}
	}

	void ER_ShadowMapper::BeginRenderingToShadowMap(int cascadeIndex)
	{
		assert(cascadeIndex < NUM_SHADOW_CASCADES);

		auto rhi = GetCore()->GetRHI();

		mOriginalRS = rhi->GetCurrentRasterizerState();
		mOriginalViewport = rhi->GetCurrentViewport();
		mOriginalRect = rhi->GetCurrentRect();

		ER_RHI_Viewport newViewport;
		newViewport.TopLeftX = 0.0f;
		newViewport.TopLeftY = 0.0f;
		newViewport.Width = static_cast<float>(mShadowMaps[cascadeIndex]->GetWidth());
		newViewport.Height = static_cast<float>(mShadowMaps[cascadeIndex]->GetHeight());
		newViewport.MinDepth = 0.0f;
		newViewport.MaxDepth = 1.0f;

		ER_RHI_Rect newRect = { 0, 0, static_cast<LONG>(mShadowMaps[cascadeIndex]->GetWidth()), static_cast<LONG>(mShadowMaps[cascadeIndex]->GetHeight()) };

		rhi->SetDepthTarget(mShadowMaps[cascadeIndex]);
		rhi->ClearDepthStencilTarget(mShadowMaps[cascadeIndex], 1.0f);
		rhi->SetViewport(newViewport);
		rhi->SetRect(newRect);
	}

	void ER_ShadowMapper::StopRenderingToShadowMap(int cascadeIndex)
	{
		assert(cascadeIndex < NUM_SHADOW_CASCADES);

		auto rhi = GetCore()->GetRHI();

		rhi->UnbindRenderTargets();
		rhi->SetViewport(mOriginalViewport);
		rhi->SetRect(mOriginalRect);
		rhi->SetRasterizerState(mOriginalRS);
	}

	XMMATRIX ER_ShadowMapper::GetViewMatrix(int cascadeIndex /*= 0*/) const
	{
		assert(cascadeIndex < NUM_SHADOW_CASCADES);
		return mLightProjectors[cascadeIndex].ViewMatrix();
	}
	XMMATRIX ER_ShadowMapper::GetProjectionMatrix(int cascadeIndex /*= 0*/) const
	{
		assert(cascadeIndex < NUM_SHADOW_CASCADES);
		return mLightProjectors[cascadeIndex].ProjectionMatrix();
	}

	ER_RHI_GPUTexture* ER_ShadowMapper::GetShadowTexture(int cascadeIndex) const
	{
		assert(cascadeIndex < NUM_SHADOW_CASCADES);
		return mShadowMaps.at(cascadeIndex);
	}

	//void ShadowMapper::ApplyRotation()
	//{
	//	for (int i = 0; i < MAX_NUM_OF_CASCADES; i++)
	//		mLightProjectors[i]->ApplyRotation(mDirectionalLight.GetTransform());
	//}	
	void ER_ShadowMapper::ApplyTransform()
	{
		for (int i = 0; i < NUM_SHADOW_CASCADES; i++)
			mLightProjectors[i].ApplyTransform(mDirectionalLight.GetTransform());
	}

	XMMATRIX ER_ShadowMapper::GetLightProjectionMatrixInFrustum(int index, ER_Frustum& cameraFrustum, ER_DirectionalLight& light)
	{
		assert(index < NUM_SHADOW_CASCADES);

		//create corners
		XMFLOAT3 frustumCorners[8] = {};

		frustumCorners[0] = (cameraFrustum.Corners()[0]);
		frustumCorners[1] = (cameraFrustum.Corners()[1]);
		frustumCorners[2] = (cameraFrustum.Corners()[2]);
		frustumCorners[3] = (cameraFrustum.Corners()[3]);
		frustumCorners[4] = (cameraFrustum.Corners()[4]);
		frustumCorners[5] = (cameraFrustum.Corners()[5]);
		frustumCorners[6] = (cameraFrustum.Corners()[6]);
		frustumCorners[7] = (cameraFrustum.Corners()[7]);

		XMFLOAT3 frustumCenter = { 0, 0, 0 };

		for (size_t i = 0; i < 8; i++)
		{
			frustumCenter = XMFLOAT3(frustumCenter.x + frustumCorners[i].x,
				frustumCenter.y + frustumCorners[i].y,
				frustumCenter.z + frustumCorners[i].z);
		}

		//calculate frustum's center position
		frustumCenter = XMFLOAT3(
			frustumCenter.x * (1.0f / 8.0f),
			frustumCenter.y * (1.0f / 8.0f),
			frustumCenter.z * (1.0f / 8.0f));

		//mLightProjectorCenteredPositions[index] = frustumCenter;

		float minX = (std::numeric_limits<float>::max)();
		float maxX = (std::numeric_limits<float>::min)();
		float minY = (std::numeric_limits<float>::max)();
		float maxY = (std::numeric_limits<float>::min)();
		float minZ = (std::numeric_limits<float>::max)();
		float maxZ = (std::numeric_limits<float>::min)();

		for (int j = 0; j < 8; j++) {

			// Transform the frustum coordinate from world to light space
			XMVECTOR frustumCornerVector = XMLoadFloat3(&frustumCorners[j]);
			frustumCornerVector = XMVector3Transform(frustumCornerVector, (light.LightMatrix(frustumCenter)));

			XMStoreFloat3(&frustumCorners[j], frustumCornerVector);

			minX = std::min(minX, frustumCorners[j].x);
			maxX = std::max(maxX, frustumCorners[j].x);
			minY = std::min(minY, frustumCorners[j].y);
			maxY = std::max(maxY, frustumCorners[j].y);
			minZ = std::min(minZ, frustumCorners[j].z);
			maxZ = std::max(maxZ, frustumCorners[j].z);
		}

		mLightProjectorCenteredPositions[index] =
			XMFLOAT3(
				frustumCenter.x + light.Direction().x * -maxZ,
				frustumCenter.y + light.Direction().y * -maxZ,
				frustumCenter.z + light.Direction().z * -maxZ
			);

		//set orthographic proj with proper boundaries
		float delta = 10.0f;
		XMMATRIX projectionMatrix = XMMatrixOrthographicRH(maxX - minX, maxY - minY, -delta, maxZ - minZ);
		return projectionMatrix;
	}
	XMMATRIX ER_ShadowMapper::GetProjectionBoundingSphere(int index, float& sphereRadius)
	{
		// Create a bounding sphere around the camera frustum for 360 rotation
		float nearV = index == 0 ? mCamera.NearPlaneDistance() : GetCameraNearShadowCascadeDistance(index);
		float farV = GetCameraFarShadowCascadeDistance(index);
		float endV = nearV + farV;
		XMFLOAT3 sphereCenter = XMFLOAT3(
			mCamera.Position().x + mCamera.Direction().x * (nearV + 0.5f * endV),
			mCamera.Position().y + mCamera.Direction().y * (nearV + 0.5f * endV),
			mCamera.Position().z + mCamera.Direction().z * (nearV + 0.5f * endV)
		);
		// Create a vector to the frustum far corner
		float tanFovY = tanf(mCamera.FieldOfView());
		float tanFovX = mCamera.AspectRatio() * tanFovY;

		XMFLOAT3 farCorner = XMFLOAT3(
			mCamera.Direction().x + mCamera.Right().x * tanFovX + mCamera.Up().x * tanFovY,
			mCamera.Direction().y + mCamera.Right().y * tanFovX + mCamera.Up().y * tanFovY,
			mCamera.Direction().z + mCamera.Right().z * tanFovX + mCamera.Up().z * tanFovY);
		// Compute the frustumBoundingSphere radius
		XMFLOAT3 boundVec = XMFLOAT3(
			mCamera.Position().x + farCorner.x  * farV - sphereCenter.x,
			mCamera.Position().y + farCorner.y  * farV - sphereCenter.y,
			mCamera.Position().z + farCorner.z  * farV - sphereCenter.z);
		sphereRadius = sqrt(boundVec.x * boundVec.x + boundVec.y * boundVec.y + boundVec.z * boundVec.z);

		mLightProjectorCenteredPositions[index] =
			XMFLOAT3(
				mCamera.Position().x + mCamera.Direction().x * 0.5f * GetCameraFarShadowCascadeDistance(index),
				mCamera.Position().y + mCamera.Direction().y * 0.5f * GetCameraFarShadowCascadeDistance(index),
				mCamera.Position().z + mCamera.Direction().z * 0.5f * GetCameraFarShadowCascadeDistance(index)
			);

		XMMATRIX projectionMatrix = XMMatrixOrthographicRH(sphereRadius, sphereRadius, -sphereRadius, sphereRadius);
		return projectionMatrix;
	}

	void ER_ShadowMapper::Draw(const ER_Scene* scene, ER_Terrain* terrain)
	{
		auto rhi = GetCore()->GetRHI();

		ER_MaterialSystems materialSystems;
		materialSystems.mShadowMapper = this;

		for (int i = 0; i < NUM_SHADOW_CASCADES; i++)
		{
			std::string materialName = ER_MaterialHelper::shadowMapMaterialName + " " + std::to_string(i);
			BeginRenderingToShadowMap(i);

			rhi->BeginEventTag("EveryRay: Shadow Maps (terrain), cascade " + std::to_string(i));
			if (terrain)
				terrain->Draw(TerrainRenderPass::TERRAIN_SHADOW, { mShadowMaps[i] }, nullptr, this, nullptr, i);
			rhi->EndEventTag();

			rhi->BeginEventTag("EveryRay: Shadow Maps (objects), cascade " + std::to_string(i));

			rhi->SetRootSignature(mRootSignature);
			rhi->SetTopologyType(ER_RHI_PRIMITIVE_TYPE::ER_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			int objectIndex = 0;
			for (auto renderingObjectInfo = scene->objects.begin(); renderingObjectInfo != scene->objects.end(); renderingObjectInfo++, objectIndex++)
			{
				ER_RenderingObject* renderingObject = renderingObjectInfo->second;
				const std::string& psoName = renderingObject->IsInstanced() ? psoNameInstanced : psoNameNonInstanced;
				auto materialInfo = renderingObject->GetMaterials().find(materialName);
				if (materialInfo != renderingObject->GetMaterials().end())
				{
					ER_Material* material = materialInfo->second;
					if (!rhi->IsPSOReady(psoName))
					{
						rhi->InitializePSO(psoName);
						rhi->SetRasterizerState(ER_SHADOW_RS);
						rhi->SetBlendState(ER_NO_BLEND);
						rhi->SetDepthStencilState(ER_RHI_DEPTH_STENCIL_STATE::ER_DEPTH_ONLY_WRITE_COMPARISON_LESS_EQUAL);
						material->PrepareShaders();
						rhi->SetRenderTargetFormats({}, mShadowMaps[i]);
						rhi->SetRootSignatureToPSO(psoName, mRootSignature);
						rhi->SetTopologyTypeToPSO(psoName, ER_RHI_PRIMITIVE_TYPE::ER_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
						rhi->FinalizePSO(psoName);
					}
					rhi->SetPSO(psoName);
					for (int meshIndex = 0; meshIndex < renderingObject->GetMeshCount(); meshIndex++)
					{
						static_cast<ER_ShadowMapMaterial*>(material)->PrepareForRendering(materialSystems, renderingObject, meshIndex, i, mRootSignature);
						if (!renderingObject->IsInstanced())
							renderingObject->DrawLOD(materialName, true, meshIndex, renderingObject->GetLODCount() - 1); //drawing highest LOD
						else
							renderingObject->Draw(materialName, true, meshIndex);
					}
				}
			}
			rhi->EndEventTag();

			rhi->UnsetPSO();
			StopRenderingToShadowMap(i);
		}
	}

	float ER_ShadowMapper::GetCameraFarShadowCascadeDistance(int index) const
	{
		assert(index < (sizeof(ER_Utility::ShadowCascadeDistances) / sizeof(ER_Utility::ShadowCascadeDistances[0])));
		return ER_Utility::ShadowCascadeDistances[index];
	}
	float ER_ShadowMapper::GetCameraNearShadowCascadeDistance(int index) const
	{
		assert(index > 0); // get camera's near place instead
		assert(index < (sizeof(ER_Utility::ShadowCascadeDistances) / sizeof(ER_Utility::ShadowCascadeDistances[0])));

		return ER_Utility::ShadowCascadeDistances[index - 1];
	}

	XMMATRIX ER_ShadowMapper::GetCustomViewProjectionMatrixForCascade(const XMMATRIX& viewMatrix, float fov, float aspectRatio, float nearPlaneDistance, int cascadeIndex) const
	{
		XMMATRIX projectionMatrix;
		//float delta = 5.0f;

		switch (cascadeIndex)
		{
		case 0:
			projectionMatrix = XMMatrixPerspectiveFovRH(fov, aspectRatio, nearPlaneDistance, ER_Utility::ShadowCascadeDistances[0]);
			break;
		case 1:
			projectionMatrix = XMMatrixPerspectiveFovRH(fov, aspectRatio, ER_Utility::ShadowCascadeDistances[0], ER_Utility::ShadowCascadeDistances[1]);
			break;
		case 2:
			projectionMatrix = XMMatrixPerspectiveFovRH(fov, aspectRatio, ER_Utility::ShadowCascadeDistances[1], ER_Utility::ShadowCascadeDistances[2]);
			break;


		default:
			projectionMatrix = XMMatrixPerspectiveFovRH(fov, aspectRatio, nearPlaneDistance, ER_Utility::ShadowCascadeDistances[2]);
		}

		return XMMatrixMultiply(viewMatrix, projectionMatrix);
	}
}