#pragma once
namespace Lobelia::Game {

	ALIGN(16)struct DissolveInfo {
		//臒l
		float threshold = 0.0f;
	};
	class DissolveShader {
	public:
		DissolveShader();
		~DissolveShader();
		void Activate(std::shared_ptr<Graphics::Model>& model);
	private:
		std::shared_ptr<Graphics::PixelShader> ps;
		std::shared_ptr<Graphics::PixelShader> defaultPS;
		float rad;
		DissolveInfo dissolveInfo;
		std::unique_ptr<Graphics::ConstantBuffer<DissolveInfo>> constantBufferDissolveInfo;
		Graphics::Texture* dissolveMap;
	};
	class SceneDissolve :public Lobelia::Scene {
	public:
		SceneDissolve();
		~SceneDissolve();
		void Initialize()override;
		void AlwaysUpdate()override;
		void AlwaysRender()override;
	private:
		std::unique_ptr<Graphics::View> view;
		Math::Vector3 pos;
		Math::Vector3 at;
		Math::Vector3 up;
		std::shared_ptr<Graphics::Model> model;
		std::unique_ptr<DissolveShader> dissolve;
	};
}
