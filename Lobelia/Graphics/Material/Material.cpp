#include "Common/Common.hpp"
#include "Graphics/Origin/Origin.hpp"
#include "Graphics/GraphicDriverInfo/GraphicDriverInfo.hpp"
#include "Graphics/Device/Device.hpp"
#include "Graphics/ConstantBuffer/ShaderStageList.hpp"
#include "Graphics/Texture/Texture.hpp"
#include "Exception/Exception.hpp"
#include "Graphics/BufferCreator/BufferCreator.h"
#include "Graphics/ConstantBuffer/ConstantBuffer.hpp"
#include "Graphics/Material/Material.hpp"
#include "Graphics/Shader/Shader.hpp"
#include "Graphics/Shader/ShaderBank.hpp"
#include "Config/Config.hpp"

namespace Lobelia::Graphics {
	Material::Material(const char* name, const char* texture_path) :name(name), texture(nullptr), normal(nullptr), specular(nullptr), emission(nullptr), visible(true), constantBuffer(std::make_unique<ConstantBuffer<Data>>(2, Config::GetRefPreference().systemCBActiveStage)) {
		try {
			TextureFileAccessor::Load(texture_path, &texture);
			std::string directory = Utility::FilePathControl::GetParentDirectory(texture_path);
			std::string fileName = Utility::FilePathControl::GetFilename(texture_path);
			std::string normalPath;
			std::string specularPath;
			std::string emissionPath;
			if (directory.empty()) {
				normalPath = "N" + fileName;
				specularPath = "S" + fileName;
				emissionPath = "E" + fileName;
			}
			else {
				normalPath = directory + "/" + "N" + fileName;
				specularPath = directory + "/" + "S" + fileName;
				emissionPath = directory + "/" + "E" + fileName;
			}
			//
			if (!normalPath.empty() && Utility::FilePathControl::IsFileOrPath(normalPath))TextureFileAccessor::Load(normalPath.c_str(), &normal);
			//else normal
			if (!specularPath.empty() && Utility::FilePathControl::IsFileOrPath(specularPath))TextureFileAccessor::Load(specularPath.c_str(), &specular);
			//if (!emissionPath.empty() && Utility::FilePathControl::IsFileOrPath(emissionPath))
			TextureFileAccessor::Load(emissionPath.c_str(), &emission, false, 0x00000000);
		}
		catch (...) {
			throw;
		}
	}
	Material::~Material() = default;
	const std::string& Material::GetName() { return name; }
	Texture* Material::GetTexture() { return texture; }

	void Material::SetDiffuseColor(const Math::Vector4& diffuse) { data.diffuse = diffuse; }
	void Material::SetAmbientColor(const Math::Vector4& ambient) { data.ambient = ambient; }
	void Material::SetSpecularColor(const Math::Vector4& specular) { data.specular = specular; }
	void Material::SetTexColor(Utility::Color tex_color) {
		data.texColor.x = tex_color.GetNormalizedR();
		data.texColor.y = tex_color.GetNormalizedG();
		data.texColor.z = tex_color.GetNormalizedB();
		data.texColor.w = tex_color.GetNormalizedA();
	}
	void Material::ChangeVisible(bool visible) { this->visible = visible; }
	bool Material::IsVisible() { return visible; }

	//いつでもテクスチャを切り替えれるように。
	void Material::ChangeTexture(Texture* texture) { this->texture = texture; }
	void Material::Set(bool texture, bool color) {
		//if (IsSet())return;
		if (this->texture&&texture)this->texture->Set(0, ShaderStageList::PS);
		if (this->normal&&texture)this->normal->Set(1, ShaderStageList::PS);
		if (this->specular&&texture)this->specular->Set(2, ShaderStageList::PS);
		if (this->emission&&texture)this->emission->Set(3, ShaderStageList::PS);
		data.useNormalMap = (normal != nullptr);
		data.useSpecularMap = (specular != nullptr);
		data.useEmission = (emission != nullptr);
		if (color)constantBuffer->Activate(data);
	}
}