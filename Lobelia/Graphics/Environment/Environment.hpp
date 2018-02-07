#pragma once
namespace Lobelia::Graphics {
	class Environment :public Utility::Singleton<Environment> {
		friend class Utility::Singleton<Environment>;
	private:
		struct Constant {
			Math::Vector4 dir;
			Math::Vector4 color;
			Math::Vector4 fogInfo;
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
		void Activate();
	};
}