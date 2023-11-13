#pragma once

#include "Common.h"

#include "ER_VectorHelper.h"
#include "ER_MatrixHelper.h"
#include "ER_ColorHelper.h"
#include "ER_MaterialHelper.h"
#include "ThirdParty/renderdoc_app.h"

namespace EveryRay_Core
{
	class ER_Utility
	{
	public:
		static std::string CurrentDirectory();
		static std::wstring ExecutableDirectory();
		static std::wstring GetFilePath(const std::wstring& inputPath);
		static std::string GetFilePath(const std::string& inputPath);
		static void GetFileName(const std::string& inputPath, std::string& filename);
		static void GetDirectory(const std::string& inputPath, std::string& directory);
		static void GetFileNameAndDirectory(const std::string& inputPath, std::string& directory, std::string& filename);
		static void LoadBinaryFile(const std::wstring& filename, std::vector<char>& data);
		static void ToWideString(const std::string& source, std::wstring& dest);
		static std::wstring ToWideString(const std::string& source);
		static void PathJoin(std::wstring& dest, const std::wstring& sourceDirectory, const std::wstring& sourceFile);
		static void GetPathExtension(const std::wstring& source, std::wstring& dest);
		static float RandomFloat(float a, float b);

		// Replace with: https://sourcegraph.com/github.com/nCine/nCine/-/blob/src/graphics/RenderDocCapture.cpp
		static void InitializeRenderDoc();
		static RENDERDOC_API_1_1_1* GetRenderDocAPI();

		static bool IsEditorMode;
		static bool IsLightEditor;
		static bool IsFoliageEditor;
		static bool IsPostEffectsVolumeEditor;
		static bool IsMainCameraCPUFrustumCulling;
		static bool StopDrawingRenderingObjects;
		static bool IsWireframe;
		static float DistancesLOD[MAX_LOD];
		static float ShadowCascadeDistances[NUM_SHADOW_CASCADES];
		static float ShadowCascadeExponentScale[8];
	private:
		ER_Utility();
		ER_Utility(const ER_Utility& rhs);
		ER_Utility& operator=(const ER_Utility& rhs);

		static HMODULE mRenderDocDLL;
		static RENDERDOC_API_1_1_1* mRenderDocAPI;
	};
}
