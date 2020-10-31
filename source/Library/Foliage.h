#pragma once
#include "Common.h"
#include "GameComponent.h"
#include "Camera.h"
#include "DirectionalLight.h"
#include "PostProcessingStack.h"
#include "FoliageMaterial.h"

namespace Library
{
	enum FoliageBillboardType
	{
		SINGLE,
		TWO_QUADS_CROSSING,
		THREE_QUADS_CROSSING
	};

	struct FoliagePatchData //for GPU vertex buffer
	{
		XMFLOAT4 pos;
		XMFLOAT2 uv;
	};

	struct FoliageInstanceData //for GPU instance buffer
	{
		XMMATRIX worldMatrix = XMMatrixIdentity();
		XMFLOAT3 color;
	};

	struct FoliageData //for CPU buffer
	{
		float xPos, yPos, zPos;
		float r, g, b;
		float scale;
	};

	class Foliage : public GameComponent
	{
	public:
		Foliage(Game& pGame, Camera& pCamera, DirectionalLight& pLight, int pPatchesCount, std::string textureName, float scale = 1.0f, float distributionRadius = 100);
		~Foliage();

		void Initialize();
		void Draw();
		void Update(const GameTime& gameTime);

		int GetPatchesCount() { return mPatchesCount; }
		void SetWireframe(bool flag) { mIsWireframe = flag; }

	private:
		void CreateBlendStates();
		void InitializeBuffersGPU();
		void InitializeBuffersCPU();

		Camera& mCamera;
		DirectionalLight& mDirectionalLight;

		FoliageMaterial* mMaterial;

		ID3D11Buffer* mVertexBuffer = nullptr;
		ID3D11Buffer* mIndexBuffer = nullptr;
		ID3D11Buffer* mInstanceBuffer = nullptr;
		ID3D11ShaderResourceView* mAlbedoTexture = nullptr;
		ID3D11BlendState* mAlphaToCoverageState = nullptr;
		ID3D11BlendState* mNoBlendState = nullptr;

		FoliageInstanceData* mPatchesBufferGPU;
		FoliageData* mPatchesBufferCPU;

		int mPatchesCount;
		bool mIsWireframe = false;
		float mScale;
		float mDistributionRadius;
		bool mRotateFromCamPosition = false;
	};
}