#pragma once
namespace Lobelia::Game {
	//---------------------------------------------------------------------------------------------
	//
	//	Skybox
	//
	//---------------------------------------------------------------------------------------------
	class SkyBox {
	public:
		SkyBox(const char* model_path, const char* mt_path);
		~SkyBox() = default;
		void Render(class Camera* camera);
	private:
		std::unique_ptr<Graphics::Model> model;
		std::shared_ptr<Graphics::DepthStencilState> depth;
	};

}