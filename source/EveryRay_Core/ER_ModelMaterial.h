#pragma once

#include "Common.h"

struct aiMaterial;

namespace EveryRay_Core
{
	enum TextureType
	{
		TextureTypeDifffuse = 0,
		TextureTypeSpecularMap,
		TextureTypeAmbient,
		TextureTypeEmissive,
		TextureTypeHeightmap,
		TextureTypeNormalMap,
		TextureTypeSpecularPowerMap,
		TextureTypeDisplacementMap,
		TextureTypeLightMap,
		TextureTypeEnd
	};

	class ER_Model;

	class ER_ModelMaterial
	{
	public:
		ER_ModelMaterial(ER_Model& model, aiMaterial* material);
		ER_ModelMaterial(ER_Model& model);
		~ER_ModelMaterial();

		ER_Model& GetModel();
		const std::string& Name() const;
		const std::map<TextureType, std::vector<std::wstring>>& Textures() const;
		const std::vector<std::wstring>& GetTexturesByType(TextureType type) const;
		bool HasTexturesOfType(TextureType type) const;

	private:
		static void InitializeTextureTypeMappings();
		static std::map<TextureType, UINT> sTextureTypeMappings;

		ER_Model& mModel;
		std::string mName;
		std::map<TextureType, std::vector<std::wstring>> mTextures;
	};
}