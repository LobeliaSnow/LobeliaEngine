#include "Common/Common.hpp"
#include "Graphics/ConstantBuffer/ShaderStageList.hpp"
#include "Graphics/GraphicDriverInfo/GraphicDriverInfo.hpp"
#include "Graphics/Device/Device.hpp"
#include "Exception/Exception.hpp"
#include "Graphics/BufferCreator/BufferCreator.h"
#include "Graphics/ConstantBuffer/ConstantBuffer.hpp"
#include "Environment.hpp"
#include "Config/Config.hpp"

namespace Lobelia::Graphics {
	Environment::Environment() :constantBuffer(std::make_unique<ConstantBuffer<Constant>>(4, Config::GetRefPreference().systemCBActiveStage)) {
		buffer.dir = Math::Vector4(0.0f, 1.0f, 0.0f, 0.0f);
		buffer.color = Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		buffer.fogInfo = Math::Vector4(1.0f, 1.0f, 1.0f, 0.005f);
	}
	Environment::~Environment() = default;
	void Environment::SetLightDirection(const Math::Vector3& dir) { buffer.dir = Math::Vector4(dir.x, dir.y, dir.z, 0.0f)*-1; buffer.dir.Normalize(); }
	void Environment::SetAmbientColor(Utility::Color color) { buffer.color = Math::Vector4(color.GetNormalizedA(), color.GetNormalizedR(), color.GetNormalizedG(), color.GetNormalizedB()); }
	void Environment::Activate() { constantBuffer->Activate(buffer); }
}