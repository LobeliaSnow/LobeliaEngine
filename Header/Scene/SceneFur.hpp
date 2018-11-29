#pragma once
namespace Lobelia::Game {
	ALIGN(16) struct FurInfo {
		float min;
		float max;
		float maxDevide;
		float length;
	};

	class FurShader {
	public:
		FurShader();
		~FurShader();
		void Activate(std::shared_ptr<Graphics::Model>& model);
		void Clean();
	private:
		std::shared_ptr<Graphics::VertexShader> vs;
		std::shared_ptr<Graphics::HullShader> hs;
		std::shared_ptr<Graphics::DomainShader> ds;
		std::shared_ptr<Graphics::GeometryShader> gs;
		std::shared_ptr<Graphics::PixelShader> ps;
		FurInfo furInfo;
		std::unique_ptr<Graphics::ConstantBuffer<FurInfo>> constantBufferSeaInfo;
	};

	class SceneFur :public Lobelia::Scene {
	public:
		SceneFur();
		~SceneFur();
		void Initialize()override;
		void AlwaysUpdate()override;
		void AlwaysRender()override;
	private:
		std::unique_ptr<Graphics::View> view;
		Math::Vector3 pos;
		Math::Vector3 at;
		Math::Vector3 up;
		std::shared_ptr<Graphics::Model> model;
		std::unique_ptr<FurShader> furShader;
		std::shared_ptr<Graphics::RasterizerState> wireState;
		std::shared_ptr<Graphics::RasterizerState> solidState;
		int wire;
	};

}