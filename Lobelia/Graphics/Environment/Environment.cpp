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
		buffer.fogColor = Math::Vector3(1.0f, 1.0f, 1.0f);
		buffer.fogBegin = 100.0f;
		buffer.fogEnd = 500.0f;
		buffer.density = 0.80;
		buffer.useLinearFog = FALSE;
	}
	Environment::~Environment() = default;
	void Environment::SetLightDirection(const Math::Vector3& dir) { buffer.dir = Math::Vector4(dir.x, dir.y, dir.z, 0.0f)*-1; buffer.dir.Normalize(); }
	void Environment::SetAmbientColor(Utility::Color color) { buffer.color = Math::Vector4(color.GetNormalizedA(), color.GetNormalizedR(), color.GetNormalizedG(), color.GetNormalizedB()); }
	void Environment::SetFogColor(Utility::Color color) { buffer.fogColor = Math::Vector3(color.GetNormalizedR(), color.GetNormalizedG(), color.GetNormalizedB()); }
	void Environment::SetFogBegin(float begin) { buffer.fogBegin = begin; }
	void Environment::SetFogEnd(float end) { buffer.fogEnd = end; }
	void Environment::SetFogDensity(float density) { buffer.density = density; }
	void Environment::SetActiveLinearFog(bool active) { buffer.useLinearFog = i_cast(active); }
	void Environment::Activate() { constantBuffer->Activate(buffer); }
}