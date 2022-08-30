#pragma once
#include "ER_Material.h"

namespace EveryRay_Core
{
	class ER_Mesh;
	class ER_RenderingObject;

	namespace GBufferMaterial_CBufferData {
		struct ER_ALIGN16 GBufferCB
		{
			XMMATRIX ViewProjection;
			XMMATRIX World;
			XMFLOAT4 Reflection_Foliage_UseGlobalDiffuseProbe_POM_MaskFactor;
			XMFLOAT4 SkipDeferredLighting_UseSSS_CustomAlphaDiscard; // a - empty
		};
	}
	class ER_GBufferMaterial : public ER_Material
	{
	public:
		ER_GBufferMaterial(ER_Core& game, const MaterialShaderEntries& entries, unsigned int shaderFlags, bool instanced = false);
		~ER_GBufferMaterial();

		virtual void PrepareForRendering(ER_MaterialSystems neededSystems, ER_RenderingObject* aObj, int meshIndex) override;
		virtual void CreateVertexBuffer(const ER_Mesh& mesh, ER_RHI_GPUBuffer* vertexBuffer) override;
		virtual int VertexSize() override;

		ER_RHI_GPUConstantBuffer<GBufferMaterial_CBufferData::GBufferCB> mConstantBuffer;
	};
}