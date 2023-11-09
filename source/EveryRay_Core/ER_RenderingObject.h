#pragma once

#include "Common.h"
#include "ER_GenericEvent.h"
#include "ER_ModelMaterial.h"

#include "RHI\ER_RHI.h"

const UINT MAX_DIRECT_INSTANCE_COUNT = 20000; // max count for instances which are NOT GPU indirectly drawn

// Bitmasks for "RenderingObjectFlags" as decimal values
// Keep in sync with content/shaders/Common.hlsli!
const UINT RENDERING_OBJECT_FLAG_USE_GLOBAL_DIF_PROBE		 = 1;    //  0000000000001 // use global diffuse probe
const UINT RENDERING_OBJECT_FLAG_USE_GLOBAL_SPEC_PROBE		 = 2;    //  0000000000010 // use global specular probe
const UINT RENDERING_OBJECT_FLAG_SSS						 = 4;    //  0000000000100 // use sss
const UINT RENDERING_OBJECT_FLAG_POM						 = 8;    //  0000000001000 // use pom
const UINT RENDERING_OBJECT_FLAG_REFLECTION					 = 16;   //  0000000010000 // use reflection mask
const UINT RENDERING_OBJECT_FLAG_FOLIAGE					 = 32;   //  0000000100000 // is foliage
const UINT RENDERING_OBJECT_FLAG_TRANSPARENT				 = 64;   //  0000001000000 // is transparent
const UINT RENDERING_OBJECT_FLAG_FUR						 = 128;  //  0000010000000 // is fur
const UINT RENDERING_OBJECT_FLAG_SKIP_DEFERRED_PASS			 = 256;  //  0000100000000 // skip deferred pass
const UINT RENDERING_OBJECT_FLAG_SKIP_INDIRECT_DIF			 = 512;  //  0001000000000 // skip indirect specular lighting
const UINT RENDERING_OBJECT_FLAG_SKIP_INDIRECT_SPEC			 = 1024; //  0010000000000 // skip indirect specular lighting
const UINT RENDERING_OBJECT_FLAG_GPU_INDIRECT_DRAW			 = 2048; //  0100000000000 // used for GPU indirect drawing
const UINT RENDERING_OBJECT_FLAG_TRIPLANAR_MAPPING			 = 4096; //  1000000000000 // use triplanar mapping

namespace EveryRay_Core
{
	class ER_Core;
	class ER_CoreTime;
	class ER_Material;
	class ER_RenderableAABB;
	class ER_Camera;
	class ER_Model;

	enum RenderingObjectTextureQuality
	{
		OBJECT_TEXTURE_LOW = 0,
		OBJECT_TEXTURE_MEDIUM,
		OBJECT_TEXTURE_HIGH,

		OBJECT_TEXTURE_COUNT,
	};

	struct RenderBufferData
	{
		ER_RHI_GPUBuffer*		VertexBuffer;
		ER_RHI_GPUBuffer*		IndexBuffer;
		UINT					Stride;
		UINT					Offset;
		UINT					IndicesCount;

		RenderBufferData()
			:
			VertexBuffer(nullptr),
			IndexBuffer(nullptr),
			Stride(0),
			Offset(0),
			IndicesCount(0)
		{}

		RenderBufferData(ER_RHI_GPUBuffer* vertexBuffer, ER_RHI_GPUBuffer* indexBuffer, UINT stride, UINT offset, UINT indicesCount)
			:
			VertexBuffer(vertexBuffer),
			IndexBuffer(indexBuffer),
			Stride(stride),
			Offset(offset),
			IndicesCount(indicesCount)
		{ }

		~RenderBufferData()
		{
			DeleteObject(VertexBuffer);
			DeleteObject(IndexBuffer);
		}
	};

	struct ER_ALIGN_GPU_BUFFER ObjectCB
	{
		XMMATRIX World;
		float IndexOfRefraction;
		float CustomRoughness;
		float CustomMetalness;
		float CustomAlphaDiscard;
		UINT OriginalInstanceCount;
		UINT RenderingObjectFlags;
	};

	struct ER_ALIGN_GPU_BUFFER IndirectMeshCB
	{
		XMINT4 IndexCount_StartIndexLoc_BaseVtxLoc_StartInstLoc[MAX_LOD * MAX_MESH_COUNT];
		UINT OriginalInstancesCount;
		XMINT3 pad;
	};

	struct ER_ALIGN_GPU_BUFFER ObjectFakeRootCB
	{
		UINT CurrentLOD;
	};

	struct TextureData
	{
		ER_RHI_GPUTexture* AlbedoMap			= nullptr;
		ER_RHI_GPUTexture* NormalMap			= nullptr;
		ER_RHI_GPUTexture* SpecularMap			= nullptr;
		ER_RHI_GPUTexture* MetallicMap			= nullptr;
		ER_RHI_GPUTexture* RoughnessMap			= nullptr;
		ER_RHI_GPUTexture* HeightMap			= nullptr; // used for POM
		ER_RHI_GPUTexture* ExtraMaskMap			= nullptr; // used for reflection, transparency or fur masks
		ER_RHI_GPUTexture* ExtraMap2			= nullptr; // can be used for extra textures (AO, opacity, displacement)
		ER_RHI_GPUTexture* ExtraMap3			= nullptr; // can be used for extra textures (AO, opacity, displacement)

		TextureData() {}

		~TextureData()
		{
			// After adding new texture cache functionality in ER_RuntimeCore, we don't need to free here
		}
	};

	struct InstancedData
	{
		XMFLOAT4X4 World;

		InstancedData() {}
		InstancedData(const XMFLOAT4X4& world) : World(world) {}
		InstancedData(CXMMATRIX world) : World() { XMStoreFloat4x4(&World, world); }
	};

	struct InstanceBufferData
	{
		UINT				Stride = 0; 
		UINT				Offset = 0;
		ER_RHI_GPUBuffer*	InstanceBuffer = nullptr;

		InstanceBufferData() : 
			Stride(0),
			Offset(0),
			InstanceBuffer(nullptr)
		{
		}

		~InstanceBufferData()
		{
			DeleteObject(InstanceBuffer);
		}
		
	};

	class ER_RenderingObject
	{
		using Delegate_MeshMaterialVariablesUpdate = std::function<void(int, int)>; // mesh index & lod index for input

	public:
		ER_RenderingObject(const std::string& pName, int index, ER_Core& pCore, ER_Camera& pCamera, const std::string& pModelPath, bool availableInEditor = false, bool isInstanced = false,  bool isCastShadow = true);
		~ER_RenderingObject();

		void LoadMaterial(ER_Material* pMaterial, const std::string& materialName);
		void LoadRenderBuffers(int lod = 0);

		void LoadCustomMeshTextures(int meshIndex);
		void LoadCustomMaterialTextures();
		void LoadAssignedMeshTextures(int meshIndex);

		void Draw(const std::string& materialName, bool toDepth = false, int meshIndex = -1);
		void DrawLOD(const std::string& materialName, bool toDepth, int meshIndex, int lod, bool skipCulling = false);
		void DrawAABB(ER_RHI_GPUTexture* aRenderTarget, ER_RHI_GPUTexture* aDepth, ER_RHI_GPURootSignature* rs);
		void Update(const ER_CoreTime& time);

		std::map<std::string, ER_Material*>& GetMaterials() { return mMaterials; }
		
		TextureData& GetTextureData(int meshIndex) { return mMeshesTextureBuffers[meshIndex]; }
		
		const int GetMeshCount(int lod = 0) const { return mMeshesCount[lod]; }
		const std::vector<XMFLOAT3>& GetVertices(int lod = 0) { return mMeshAllVertices[lod]; }
		const UINT GetInstanceCount(int lod = 0) const { return (mIsInstanced ? static_cast<UINT>(mInstanceData[lod].size()) : 0); }
		std::vector<InstancedData>& GetInstancesData(int lod = 0) { return mInstanceData[lod]; }
		const int GetIndexCount(int lod, int mesh) const { return mMeshRenderBuffers[lod][mesh]->IndicesCount; }

		XMFLOAT4X4 GetTransformationMatrix4X4() const { return XMFLOAT4X4(mCurrentObjectTransformMatrix); }
		const XMMATRIX& GetTransformationMatrix() const { return mTransformationMatrix; }

		ER_AABB& GetLocalAABB() { return mLocalAABB; } //local space (no transforms)
		ER_AABB& GetGlobalAABB() { return mGlobalAABB; } //world space (with transforms)
		ER_AABB& GetInstanceAABB(int index) { return mInstanceAABBs[index]; } //world space (with transforms)

		void SetTransformationMatrix(const XMMATRIX& mat);
		void SetTranslation(float x, float y, float z);
		void SetScale(float x, float y, float z);
		void SetRotation(float x, float y, float z);

		void LoadInstanceBuffers(int lod = 0);
		void UpdateInstanceBuffer(std::vector<InstancedData>& instanceData, int lod = 0);
		void ResetInstanceData(int count, bool clear = false, int lod = 0);
		void AddInstanceData(const XMMATRIX& worldMatrix, int lod = -1);
		void CreateIndirectInstanceData();
		UINT InstanceSize() const;
		
		void PerformCPUFrustumCull(ER_Camera* camera);

		void SetGPUIndirectlyRendered(bool value) { mIsIndirectlyRendered = value; }
		bool IsGPUIndirectlyRendered() { return mIsIndirectlyRendered; }
		ER_RHI_GPUBuffer* GetIndirectNewInstanceBuffer() { return mIndirectNewInstanceDataBuffer; }
		ER_RHI_GPUBuffer* GetIndirectOriginalInstanceBuffer() { return mIndirectOriginalInstanceDataBuffer; }
		ER_RHI_GPUBuffer* GetIndirectArgsBuffer() { return mIndirectArgsBuffer; }
		ER_RHI_GPUBuffer* GetIndirectMeshConstantBuffer();
		const XMINT4* GetIndirectDrawArgsArray() const { return mIndirectDrawArgsArray; }

		void Rename(const std::string& name) { mName = name; }
		const std::string& GetName() { return mName; }

		const int GetLODCount() const {	return 1 + static_cast<int>(mModelLODs.size());	}
		void UpdateLODs();
		void AddLOD(const std::string& pModelLODPath);
		
		float GetMinScale() { return mMinScale; }
		void SetMinScale(float v) { mMinScale = v; }
		
		float GetMaxScale() { return mMaxScale; }
		void SetMaxScale(float v) { mMaxScale = v; }

		bool IsSelected() { return mIsSelected; }
		void SetSelected(bool val) { mIsSelected = val; }

		bool IsInstanced() { return mIsInstanced; }
		bool IsAvailableInEditor() { return mIsAvailableInEditorMode; }

		bool IsCastShadow() {return mIsCastShadow; }

		bool IsRendered() { return mIsRendered; }
		void SetRendered(bool val) { mIsRendered = val; }

		// main camera view flag
		bool IsCulled() { return mIsCulled; }
		void SetCulled(bool val) { mIsCulled = val; }

		float GetCustomAlphaDiscard() { return mCustomAlphaDiscard; }
		void SetCustomAlphaDiscard(float val) { mCustomAlphaDiscard = val; }

		void PlaceProcedurallyOnTerrain(bool isOnInit);
		void StoreInstanceDataAfterTerrainPlacement();
		void SetTerrainPlacement(bool flag) { mIsTerrainPlacement = flag; }
		bool GetTerrainPlacement() { return mIsTerrainPlacement; }
		void SetTerrainProceduralPlacementHeightDelta(float delta) { mTerrainProceduralPlacementHeightDelta = delta; }
		void SetTerrainProceduralPlacementSplatChannel(int channel) { mTerrainProceduralPlacementSplatChannel = channel; }
		int GetTerrainProceduralPlacementSplatChannel() { return mTerrainProceduralPlacementSplatChannel; }
		void SetTerrainProceduralInstanceCount(int count) { mTerrainProceduralInstanceCount = count; }
		int GetTerrainProceduralInstanceCount() { return mTerrainProceduralInstanceCount; }
		void SetTerrainProceduralZoneCenterPos(const XMFLOAT3& pos) { mTerrainProceduralZoneCenterPos = pos; }
		const XMFLOAT3& GetTerrainProceduralZoneCenterPos(const XMFLOAT3& pos) { return mTerrainProceduralZoneCenterPos; }
		void SetTerrainProceduralZoneRadius(float radius) { mTerrainProceduralZoneRadius = radius; }
		float GetTerrainProceduralZoneRadius() { return mTerrainProceduralZoneRadius; }
		void SetTerrainProceduralObjectsMinMaxScale(float minScale, float maxScale) { mTerrainProceduralObjectMinScale = minScale; mTerrainProceduralObjectMaxScale = maxScale; }
		void SetTerrainProceduralObjectsMinMaxYaw(float minYaw, float maxYaw) { mTerrainProceduralObjectMinYaw = XMConvertToRadians(minYaw); mTerrainProceduralObjectMaxYaw = XMConvertToRadians(maxYaw); }
		void SetTerrainProceduralObjectsMinMaxPitch(float minPitch, float maxPitch) { mTerrainProceduralObjectMinPitch = XMConvertToRadians(minPitch); mTerrainProceduralObjectMaxPitch = XMConvertToRadians(maxPitch); }
		void SetTerrainProceduralObjectsMinMaxRoll(float minRoll, float maxRoll) { mTerrainProceduralObjectMinRoll = XMConvertToRadians(minRoll); mTerrainProceduralObjectMaxRoll = XMConvertToRadians(maxRoll); }

		void SetReflective(bool value) { mIsReflective = value; }
		bool IsReflective() { return mIsReflective; }

		void SetMeshReflectionFactor(int meshIndex, float factor) { mMeshesReflectionFactors[meshIndex] = factor; }
		float GetMeshReflectionFactor(int meshIndex) { return mMeshesReflectionFactors[meshIndex]; }

		bool GetIsMarkedAsFoliage() { return mIsMarkedAsFoliage; }
		void SetIsMarkedAsFoliage(bool value) { mIsMarkedAsFoliage = value; }

		bool IsForwardShading() { return mIsForwardShading; }
		void SetForwardShading(bool value) { mIsForwardShading = value; }

		bool IsInGBuffer() { return mIsInGbuffer; }
		void SetInGBuffer(bool value) { mIsInGbuffer = value; }

		bool IsInVoxelization() { return mIsInVoxelization; }
		void SetInVoxelization(bool value) { mIsInVoxelization = value; }

		bool IsParallaxOcclusionMapping() { return mIsPOM; }
		void SetParallaxOcclusionMapping(bool value) { mIsPOM = value; }

		bool IsInLightProbe() { return mIsInLightProbe; }
		void SetInLightProbe(bool value) { mIsInLightProbe = value; }

		bool IsTransparent() { return mIsTransparent; }
		void SetTransparency(bool value) { mIsTransparent = value; }

		float GetIOR() { return mIOR; }
		void SetIOR(float value) { mIOR = value; }
		
		float GetCustomRoughness() { return mCustomRoughness; }
		void SetCustomRoughness(float value) { mCustomRoughness = value; }

		float GetCustomMetalness() { return mCustomMetalness; }
		void SetCustomMetalness(float value) { mCustomMetalness = value; }

		bool IsSeparableSubsurfaceScattering() { return mIsSeparableSubsurfaceScattering; }
		void SetSeparableSubsurfaceScattering(bool value) { mIsSeparableSubsurfaceScattering = value; }

		void SetUseIndirectGlobalLightProbe(bool value) { mUseIndirectGlobalLightProbe = value; }
		bool GetUseIndirectGlobalLightProbe() { return mUseIndirectGlobalLightProbe; }

		bool IsUsedForGlobalLightProbeRendering() { return mIsUsedForGlobalLightProbeRendering; }
		void SetIsUsedForGlobalLightProbeRendering(bool value) { mIsUsedForGlobalLightProbeRendering = value; }

		bool IsSkippedIndirectSpecular() { return mIsSkippedIndirectSpecular; }
		void SetSkipIndirectSpecular(bool value) { mIsSkippedIndirectSpecular = value; }

		int GetIndexInScene() { return mIndexInScene; }
		void SetIndexInScene(int index) { mIndexInScene = index; }

		bool IsTriplanarMapped() { return mIsTriplanarMapped; }
		void SetTriplanarMapping(bool value) { mIsTriplanarMapped = value; }
		float GetTriplanarMappedSharpness() { return mTriplanarMappingSharpness; }
		void SetTriplanarMappedSharpness(float value) { mTriplanarMappingSharpness = value; }

		ER_RHI_GPUConstantBuffer<ObjectCB>& GetObjectsConstantBuffer() { return mObjectConstantBuffer; }
		ER_RHI_GPUConstantBuffer<ObjectFakeRootCB>& GetObjectsFakeRootConstantBuffer() { return mObjectFakeRootConstantBuffer; }

		ER_GenericEvent<Delegate_MeshMaterialVariablesUpdate>* MeshMaterialVariablesUpdateEvent = new ER_GenericEvent<Delegate_MeshMaterialVariablesUpdate>();
	
		std::vector<std::string> mCustomAlbedoTextures;
		std::vector<std::string> mCustomNormalTextures;
		std::vector<std::string> mCustomRoughnessTextures;
		std::vector<std::string> mCustomMetalnessTextures;
		std::vector<std::string> mCustomHeightTextures;
		std::vector<std::string> mCustomReflectionMaskTextures;

		//snow
		std::string mSnowAlbedoTexturePath;
		std::string mSnowNormalTexturePath;
		std::string mSnowRoughnessTexturePath;

		ER_RHI_GPUTexture* GetSnowAlbedoTexture() { return mSnowAlbedoTexture; }
		ER_RHI_GPUTexture* GetSnowNormalTexture() { return mSnowNormalTexture; }
		ER_RHI_GPUTexture* GetSnowRoughnessTexture() { return mSnowRoughnessTexture; }

		//fresnel outline
		void SetFresnelOutlineColor(const XMFLOAT3& aColor) { mFresnelOutlineColor = aColor; }
		const XMFLOAT3& GetFresnelOutlineColor() { return mFresnelOutlineColor; }
		
		//fur
		std::string mFurHeightTexturePath;
		ER_RHI_GPUTexture* GetFurMaskTexture(int meshIndex) { return mMeshesTextureBuffers[meshIndex].ExtraMaskMap; }
		ER_RHI_GPUTexture* GetFurHeightTexture() { return mFurHeightTexture; }
		int GetFurLayersCount() { return mFurLayersCount; }
		void SetFurLayersCount(int value) { mFurLayersCount = value; }
		XMFLOAT3 GetFurColor() { return XMFLOAT3(mFurColor[0], mFurColor[1], mFurColor[2]); }
		void SetFurColor(float r, float g, float b) { mFurColor[0] = r; mFurColor[1] = g; mFurColor[2] = b; }
		float GetFurColorInterpolation() { return mFurColorInterpolation; }
		void SetFurColorInterpolation(float value) { mFurColorInterpolation = value; }
		void SetFurLength(float val) { mFurLength = val; }
		float GetFurLength() { return mFurLength; }
		void SetFurCutoff(float val) { mFurCutoff = val; }
		float GetFurCutoff() { return mFurCutoff; }	
		void SetFurCutoffEnd(float val) { mFurCutoffEnd = val; }
		float GetFurCutoffEnd() { return mFurCutoffEnd; }	
		void SetFurUVScale(float val) { mFurUVScale = val; }
		float GetFurUVScale() { return mFurUVScale; }
		void SetFurWindFrequency(float v) { mFurWindFrequency = v; }
		void SetFurGravityStrength(float v) { mFurGravityStrength = v; }
		XMFLOAT4 GetFurGravityStrength(); 

		bool IsLoaded() { return mIsLoaded; }
	private:
		void UpdateAABB(ER_AABB& aabb, const XMMATRIX& transformMatrix);
		void LoadTexture(ER_RHI_GPUTexture** aTexture, bool* loadStat, const std::wstring& path, int meshIndex, bool isPlaceholder = false);
		void CreateInstanceBuffer(InstancedData* instanceData, UINT instanceCount, ER_RHI_GPUBuffer* instanceBuffer);
		
		void UpdateGizmos();
		void UpdateBitmaskFlags();
		void ShowInstancesListWindow();
		void ShowObjectsEditorWindow(const float *cameraView, float *cameraProjection, float* matrix);

		ER_Core* mCore = nullptr;
		ER_Camera& mCamera;

		std::map<std::string, ER_Material*>						mMaterials;

		ER_RHI_GPUConstantBuffer<ObjectCB>						mObjectConstantBuffer;
		ER_RHI_GPUConstantBuffer<ObjectFakeRootCB>				mObjectFakeRootConstantBuffer; // for platforms where root constants aren't supported

		///****************************************************************************************************************************
		// *** mesh/model data (buffers, textures, etc.) ***
		std::vector<TextureData>								mMeshesTextureBuffers;
		std::vector<std::vector<std::vector<XMFLOAT3>>>			mMeshVertices; // vertices per mesh, per LOD group
		std::vector<std::vector<RenderBufferData*>>				mMeshRenderBuffers; // vertex/index buffers per mesh, per LOD group
		std::vector<std::vector<InstanceBufferData*>>			mMeshesInstanceBuffers; // instance buffers per mesh, per LOD group
		std::vector<std::vector<XMFLOAT3>>						mMeshAllVertices; // vertices of all meshes combined, per LOD group
		std::vector<float>										mMeshesReflectionFactors; // mesh reflection factors, per LOD group
		std::vector<int>										mMeshesCount; // mesh count, per LOD group
		ER_Model*												mModel = nullptr; // just a pointer to the model cache
		std::vector<ER_Model*>									mModelLODs; // just pointers to the model cache
		bool													mIsLoaded = false; // whether 3D model was loaded for this object
		// 
		///****************************************************************************************************************************

		///****************************************************************************************************************************
		// *** instancing data (counters, transforms etc.) ***
		UINT													mInstanceCount = 0;
		std::vector<std::string>								mInstancesNames; // collection of names of instances (mName + index)
		std::vector<ER_AABB>									mInstanceAABBs; // collection of AABBs for every instance (shared for LODs)
		std::vector<bool>										mInstanceCullingFlags; // collection of culling flags for every instance (vector is lame here btw...)
		std::vector<InstancedData>								mTempPostCullingInstanceData; // temp instance data after CPU culling
		std::vector<std::vector<InstancedData>>					mTempPostLoddingInstanceData; // temp instance data after lodding (per LOD group)
		std::vector<UINT>										mInstanceCountToRender; //instance render count  (per LOD group)
		std::vector<std::vector<InstancedData>>					mInstanceData; //original instance data  (per LOD group)
		XMFLOAT4*												mTempInstancesPositions = nullptr;

		// GPU-driven way of culling and rendering instances without CPU readbacks (new and preferred)
		// WARNING: Make sure to use this for objects with high instances counts to make this efficient
		// WARNING: Has nothing to do with indirect lighting!
		bool													mIsIndirectlyRendered = false; // parsed from the scene file
		ER_RHI_GPUBuffer*										mIndirectOriginalInstanceDataBuffer = nullptr; //original instance transforms
		ER_RHI_GPUBuffer*										mIndirectNewInstanceDataBuffer = nullptr; //new instance transforms of all LODs culled and processed in CS
		ER_RHI_GPUBuffer*										mIndirectArgsBuffer = nullptr; // draw indexed instance indirect args for all meshes (instance count is calculated in CS)
		ER_RHI_GPUConstantBuffer<IndirectMeshCB>				mIndirectMeshConstants; // not used on DX11-like APIs;
		XMINT4													mIndirectDrawArgsArray[MAX_LOD * MAX_MESH_COUNT];
		
		///****************************************************************************************************************************

		///****************************************************************************************************************************
		// *** terrain placement & procedural fields ***
		ER_RHI_GPUBuffer*										mInputPositionsOnTerrainBuffer = nullptr; //input positions for on-terrain placement pass
		ER_RHI_GPUBuffer*										mOutputPositionsOnTerrainBuffer = nullptr; //output positions for on-terrain placement pass
		int														mTerrainProceduralPlacementSplatChannel = 4; //TerrainSplatChannel::NONE // on which terrain splat to place
		float													mTerrainProceduralPlacementHeightDelta = 0.0f; //delta from the object's origin above terrain
		int														mTerrainProceduralInstanceCount = 0;
		XMFLOAT3												mTerrainProceduralZoneCenterPos; // center of procedural placement
		float													mTerrainProceduralZoneRadius = 0.0f; // radius of procedural placement
		float													mTerrainProceduralObjectMinScale = 1.0f;
		float													mTerrainProceduralObjectMaxScale = 1.0f;
		float													mTerrainProceduralObjectMinRoll = 0.0f;
		float													mTerrainProceduralObjectMaxRoll = 0.0f;
		float													mTerrainProceduralObjectMinPitch = 0.0f;
		float													mTerrainProceduralObjectMaxPitch = 0.0f;
		float													mTerrainProceduralObjectMinYaw = 0.0f;
		float													mTerrainProceduralObjectMaxYaw = 0.0f;
		bool													mIsTerrainPlacementFinished = false;
		bool													mIsTerrainPlacement = false; //possible/wanted or not
		///****************************************************************************************************************************
		
		///****************************************************************************************************************************
		// *** extra materials data (snow, etc.) ***
		ER_RHI_GPUTexture*										mSnowAlbedoTexture = nullptr;
		ER_RHI_GPUTexture*										mSnowNormalTexture = nullptr;
		ER_RHI_GPUTexture*										mSnowRoughnessTexture = nullptr;

		XMFLOAT3												mFresnelOutlineColor = XMFLOAT3(1.0, 0.0, 0.0);

		ER_RHI_GPUTexture*										mFurHeightTexture = nullptr;
		float													mFurColor[3] = { 1.0, 1.0, 1.0 };
		float													mFurColorInterpolation = 1.0f;
		int														mFurLayersCount = -1;
		float													mFurLength = 0.25f;
		float													mFurCutoff = 0.5f;
		float													mFurCutoffEnd = 0.5f;
		float													mFurGravityStrength = 0.25f;
		float													mFurGravityDirection[3] = { -0.4f, -0.7f, 0.2f };
		float													mFurWindFrequency = 1.0f;
		float													mFurUVScale = 1.0f;
		///****************************************************************************************************************************

		ER_AABB													mLocalAABB; //mesh space AABB
		ER_AABB													mGlobalAABB; //world space AABB
		XMFLOAT3												mCurrentGlobalAABBVertices[8];
		ER_RenderableAABB*										mDebugGizmoAABB = nullptr;
	
		std::string												mName;
		const char*												mInstancedNamesUI[MAX_DIRECT_INSTANCE_COUNT];
		int														mIndexInScene = -1;
		int														mCurrentLODIndex = 0; //only used for non-instanced object
		int														mEditorSelectedInstancedObjectIndex = 0;
		bool													mIsAABBDebugEnabled = true;
		bool													mIsAvailableInEditorMode = false;
		bool													mIsSelected = false;
		bool													mIsRendered = true;
		bool													mIsInstanced = false;
		bool													mIsCastShadow = true;
		bool													mIsForwardShading = false;
		bool													mIsPOM = false;
		bool													mIsCulled = false; //only for non-instanced objects
		bool													mIsMarkedAsFoliage = false;
		bool													mIsInLightProbe = false;
		bool													mIsSeparableSubsurfaceScattering = false;
		bool													mIsInVoxelization = false;
		bool													mIsInGbuffer = false;
		bool													mUseIndirectGlobalLightProbe = false;
		bool													mIsUsedForGlobalLightProbeRendering = false;
		bool													mIsSkippedIndirectSpecular = false;
		bool													mIsSkippedIndirectDiffuse = false;
		bool													mIsReflective = false; //appeared in SSR and such
		bool													mIsTransparent = false;
		bool													mIsTriplanarMapped = false;
		float													mTriplanarMappingSharpness = 1.0f;
		float													mIOR = 1.52f; // glass IOR by default
		float													mCustomRoughness = -1.0f;
		float													mCustomMetalness = -1.0f;
		float													mCustomAlphaDiscard = 0.1f;
		float													mMinScale = 1.0f;
		float													mMaxScale = 1.0f;
		float													mCameraViewMatrix[16];
		float													mCameraProjectionMatrix[16];
		XMMATRIX												mTransformationMatrix;
		float													mMatrixTranslation[3], mMatrixRotation[3], mMatrixScale[3];
		float													mCurrentObjectTransformMatrix[16] = 
		{   
			1.f, 0.f, 0.f, 0.f,
			0.f, 1.f, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			0.f, 0.f, 0.f, 1.f 
		};

		RenderingObjectTextureQuality							mCurrentTextureQuality = RenderingObjectTextureQuality::OBJECT_TEXTURE_LOW;
		UINT													mObjectShaderBitmaskFlags = 0; // "RenderingObjectFlags" in shaders
	};
}