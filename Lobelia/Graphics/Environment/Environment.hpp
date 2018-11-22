#pragma once
namespace Lobelia::Graphics {
	class Environment :public Utility::Singleton<Environment> {
		friend class Utility::Singleton<Environment>;
	private:
		ALIGN(16) struct Constant {
			Math::Vector4 dir;
			Math::Vector4 color;
			Math::Vector3 fogColor;
			float fogBegin;
			float fogEnd;
			float density;
			int useLinearFog;
		};
		std::unique_ptr<ConstantBuffer<Constant>> constantBuffer;
		Constant buffer;
	private:
		Environment();
		~Environment();
	public:
		Environment(const Environment&) = delete;
		Environment(Environment&&) = delete;
		Environment& operator=(const Environment&) = delete;
		Environment& operator=(Environment&&) = delete;
	public:
		void SetLightDirection(const Math::Vector3& dir);
		void SetAmbientColor(Utility::Color color);
		void SetFogColor(Utility::Color color);
		void SetFogBegin(float begin);
		void SetFogEnd(float end);
		void SetFogDensity(float density);
		void SetActiveLinearFog(bool active);
		bool IsActiveLinearFog();
		void Activate();
	};
}