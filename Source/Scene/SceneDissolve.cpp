#include "Lobelia.hpp"
#include "SceneDissolve.hpp"

namespace Lobelia::Game {
	DissolveShader::DissolveShader() {
		//PixelShader(const char* file_path, const char* entry_point, Model shader_model, bool use_linkage = false);
		ps = std::make_shared<Graphics::PixelShader>("Data/ShaderFile/3D/Dissolve.hlsl", "PS", Graphics::PixelShader::Model::PS_5_0);
		constantBufferDissolveInfo = std::make_unique<Graphics::ConstantBuffer<DissolveInfo>>(7, Graphics::ShaderStageList::PS);
		rad = 0.0f;
		Graphics::TextureFileAccessor::Load("Data/Model/displacement.png", &dissolveMap);
		defaultPS = Graphics::Model::GetPixelShader();
	}
	DissolveShader::~DissolveShader() {
		Graphics::Model::ChangePixelShader(defaultPS);
	}
	void DissolveShader::Activate(std::shared_ptr<Graphics::Model>& model) {
		model->ChangePixelShader(ps);
		rad += Application::GetInstance()->GetProcessTimeSec();
		dissolveInfo.threshold = (sinf(rad) + 1.0f)*0.5f;
		constantBufferDissolveInfo->Activate(dissolveInfo);
		dissolveMap->Set(3, Graphics::ShaderStageList::PS);
	}

	SceneDissolve::SceneDissolve() {

	}
	SceneDissolve::~SceneDissolve() {

	}
	void SceneDissolve::Initialize() {
		//カメラ作成
		view = std::make_unique<Graphics::View>(Math::Vector2(), Application::GetInstance()->GetWindow()->GetSize());
		pos = Math::Vector3(0.0f, 10.0f, -10.0f);
		at = Math::Vector3(0.0f, 0.0f, 0.0f);
		up = Math::Vector3(0.0f, 1.0f, 0.0f);
		//モデル読み込み
		model = std::make_unique<Graphics::Model>("Data/Model/plane.dxd", "Data/Model/plane.mt");
		model->ChangeBlendState(std::make_shared<Graphics::BlendState>(Graphics::BLEND_PRESET::COPY, true, false));
		//ディゾルブ
		dissolve = std::make_unique<DissolveShader>();
	}
	void SceneDissolve::AlwaysUpdate() {
		//カメラ移動
		//情報算出
		Math::Vector3 front;
		Math::Vector3 right;
		front = pos - at; front.Normalize();
		right = Math::Vector3::Cross(up, front);
		float elapsedTime = Application::GetInstance()->GetProcessTimeSec();
		//前後
		if (Input::GetKeyboardKey(DIK_W)) pos += front * 20.0f*elapsedTime;
		if (Input::GetKeyboardKey(DIK_S)) pos -= front * 20.0f*elapsedTime;
		//左右
		if (Input::GetKeyboardKey(DIK_A)) pos -= right * 20.0f*elapsedTime;
		if (Input::GetKeyboardKey(DIK_D)) pos += right * 20.0f*elapsedTime;
		//上下
		if (Input::GetKeyboardKey(DIK_Z)) pos += up * 20.0f*elapsedTime;
		if (Input::GetKeyboardKey(DIK_X)) pos -= up * 20.0f*elapsedTime;
		//カメラ更新
		view->SetEyePos(pos);
		view->SetEyeTarget(at);
		view->SetEyeUpDirection(up);
	}
	void SceneDissolve::AlwaysRender() {
		view->Activate();
		Application::GetInstance()->GetSwapChain()->GetRenderTarget()->Activate();
		dissolve->Activate(model);
		model->Render();
	}

}
