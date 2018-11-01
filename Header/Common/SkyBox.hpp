#pragma once
namespace Lobelia::Game {
	class Camera;
	//---------------------------------------------------------------------------------------------
	//
	//	Skybox
	//
	//---------------------------------------------------------------------------------------------
	class SkyBox :public Graphics::Model {
	public:
		SkyBox(const char* model_path, const char* mt_path);
		~SkyBox() = default;
		void SetCamera(std::shared_ptr<Camera> camera);
		void Render(D3D_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, bool no_set = false)override;
	private:
		std::weak_ptr<Camera> camera;
		std::shared_ptr<Graphics::DepthStencilState> depth;
	};

}