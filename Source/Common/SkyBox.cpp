#include "Lobelia.hpp"
#include "Common/SkyBox.hpp"
#include "Common/Camera.hpp"

namespace Lobelia::Game {
	//---------------------------------------------------------------------------------------------
	//
	//	Skybox
	//
	//---------------------------------------------------------------------------------------------
	SkyBox::SkyBox(const char* model_path, const char* mt_path) {
		model = std::make_unique<Graphics::Model>(model_path, mt_path);
		//DepthStencilState(DEPTH_PRESET preset, bool depth, StencilDesc sdesc, bool stencil);
		depth = std::make_shared<Graphics::DepthStencilState>(Graphics::DEPTH_PRESET::LESS, false, Graphics::StencilDesc(), false);
		model->Scalling(30.0f);
	}
	void SkyBox::Render(Camera* camera) {
		auto defaultDepth = Graphics::Model::GetDepthStencilState();
		DirectX::XMVECTOR temp = {};
		DirectX::XMMATRIX inv = DirectX::XMMatrixInverse(&temp, camera->GetView()->GetRowViewMatrix());
		model->GetPixelShader()->SetLinkage(1);
		model->Translation(Math::Vector3(inv._41, inv._42, inv._43));
		model->CalcWorldMatrix();
		Graphics::Model::ChangeDepthStencilState(depth);
		model->Render();
		model->GetPixelShader()->SetLinkage(0);
		Graphics::Model::ChangeDepthStencilState(defaultDepth);
	}

}