#pragma once
#include "Common.h"
#include "ER_IlluminationProbeManager.h"

namespace Rendering
{
	class RenderingObject;
}
namespace Library
{
	class Camera;
	class DirectionalLight;
	class ShadowMapper;

	struct ER_MaterialSystems
	{
		const Camera* mCamera = nullptr;
		const DirectionalLight* mDirectionalLight = nullptr;
		const ShadowMapper* mShadowMapper = nullptr;
		const ER_IlluminationProbeManager* mProbesManager = nullptr;
	};

	class ER_MaterialsCallbacks
	{
	public:
		static void UpdateDeferredPrepassMaterialVariables(ER_MaterialSystems neededSystems, Rendering::RenderingObject* obj, int meshIndex);
		static void UpdateForwardLightingMaterial(ER_MaterialSystems neededSystems, Rendering::RenderingObject* obj, int meshIndex);
		static void UpdateShadowMappingMaterialVariables(ER_MaterialSystems neededSystems, Rendering::RenderingObject* obj, int meshIndex, int cascadeIndex);
		static void UpdateVoxelizationGIMaterialVariables(ER_MaterialSystems neededSystems, Rendering::RenderingObject* obj, int meshIndex, int voxelCascadeIndex);
		static void UpdateDebugLightProbeMaterialVariables(ER_MaterialSystems neededSystems, Rendering::RenderingObject* obj, int meshIndex, ER_ProbeType aType, int volumeIndex);
	};
}
