#include "Lobelia.hpp"
#include "Common/SkyBox.hpp"
#include "Common/Camera.hpp"

namespace Lobelia::Game {
	//---------------------------------------------------------------------------------------------
	//
	//	Skybox
	//
	//---------------------------------------------------------------------------------------------
	SkyBox::SkyBox(const char* model_path, const char* mt_path) :Graphics::Model(model_path, mt_path) {
		//model = std::make_shared<Graphics::Model>(model_path, mt_path);
		//DepthStencilState(DEPTH_PRESET preset, bool depth, StencilDesc sdesc, bool stencil);
		depth = std::make_shared<Graphics::DepthStencilState>(Graphics::DEPTH_PRESET::LESS, true, Graphics::StencilDesc(), false);
		Scalling(30.0f);
	}
	void SkyBox::SetCamera(std::shared_ptr<Camera> camera) { this->camera = camera; }
	void SkyBox::Render(D3D_PRIMITIVE_TOPOLOGY topology, bool no_set) {
		auto defaultDepth = Graphics::Model::GetDepthStencilState();
		if (!camera.expired()) {
			DirectX::XMVECTOR temp = {};
			DirectX::XMMATRIX inv = DirectX::XMMatrixInverse(&temp, camera.lock()->GetView()->GetRowViewMatrix());
			Translation(Math::Vector3(inv._41, inv._42, inv._43));
			CalcWorldMatrix();
		}
		GetPixelShader()->SetLinkage(1);
		Graphics::Model::ChangeDepthStencilState(depth);
		Graphics::Model::Render();
		GetPixelShader()->SetLinkage(0);
		Graphics::Model::ChangeDepthStencilState(defaultDepth);
	}

}