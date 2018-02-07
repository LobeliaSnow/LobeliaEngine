#include "Common/Common.hpp"
#include "Graphics/ConstantBuffer/ShaderStageList.hpp"
#include "Config.hpp"

namespace Lobelia {
	Config::Preference Config::preference;

	Config::Preference& Config::GetRefPreference() { return preference; }

}