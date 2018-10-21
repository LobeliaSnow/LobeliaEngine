#pragma once
namespace Lobelia::Game {
	//---------------------------------------------------------------------------------------------
	//実際にシェーディングする部分
	class DeferredShader {
	public:
		DeferredShader(const char* file_path, const char* entry_vs, const char* entry_ps);
		virtual ~DeferredShader() = default;
		void Render();
	private:
		std::shared_ptr<Graphics::VertexShader> vs;
		std::shared_ptr<Graphics::PixelShader> ps;
	};
	//---------------------------------------------------------------------------------------------
	class SimpleDeferred :public DeferredShader {
	public:
		SimpleDeferred();
	};
	//---------------------------------------------------------------------------------------------
	class FullEffectDeferred :public DeferredShader {
	public:
		struct PointLight {
			Math::Vector4 pos;
			Utility::Color color;
			float attenuation;
		};
	public:
		FullEffectDeferred();
		void SetLightBuffer(int index, const PointLight& p_light);
		void SetUseCount(int use_count);
		void Update();
	private:
		ALIGN(16) struct PointLights {
			Math::Vector4 pos[LIGHT_SUM];
			Math::Vector4 color[LIGHT_SUM];
			Math::Vector4 attenuation[LIGHT_SUM];
			int usedLightCount;
		}lights;
		std::unique_ptr<Graphics::ConstantBuffer<PointLights>> cbuffer;
	};
}