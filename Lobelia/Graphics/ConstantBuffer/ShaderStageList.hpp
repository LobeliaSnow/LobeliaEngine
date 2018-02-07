#pragma once
namespace Lobelia::Graphics {
	//TODO: この後からすべてコンスタントバッファはこいつに変えていく予定
	enum ShaderStageList :byte {
		VS = 1,
		PS = 2,
		HS = 4,
		GS = 8,
		DS = 16,
		CS = 32,
	};
}