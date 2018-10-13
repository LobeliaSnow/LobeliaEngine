#include "Lobelia.hpp"
#include "SceneFur.hpp"

namespace Lobelia::Game {
	FurShader::FurShader() {
		//シェーダーのロード
		vs = std::make_shared<Graphics::VertexShader>("Data/ShaderFile/3D/Fur.hlsl", "VS", Graphics::VertexShader::Model::VS_5_0);
		hs = std::make_shared<Graphics::HullShader>("Data/ShaderFile/3D/Fur.hlsl", "HS");
		ds = std::make_shared<Graphics::DomainShader>("Data/ShaderFile/3D/Fur.hlsl", "DS");
		gs = std::make_shared<Graphics::GeometryShader>("Data/ShaderFile/3D/Fur.hlsl", "GS");
		ps = std::make_shared<Graphics::PixelShader>("Data/ShaderFile/3D/Fur.hlsl", "PS", Graphics::PixelShader::Model::PS_5_0);
		//コンスタントバッファ作成
		constantBufferSeaInfo = std::make_unique<Graphics::ConstantBuffer<FurInfo>>(6, Graphics::ShaderStageList::HS | Graphics::ShaderStageList::DS | Graphics::ShaderStageList::GS);
		furInfo.min = 0.0f;
		furInfo.max = 20.0f;
		furInfo.maxDevide = 64;
		furInfo.length = 1.0f;
	}
	FurShader::~FurShader() {

	}
	void FurShader::Activate(std::shared_ptr<Graphics::Model> model) {
		//シェーダーの変更＆セット
		model->ChangeVertexShader(vs);
		model->ChangePixelShader(ps);
		hs->Set();
		ds->Set();
		gs->Set();
		//コンスタントバッファとテクスチャの更新
		constantBufferSeaInfo->Activate(furInfo);
	}
	void FurShader::Clean() {
		//他で使わないシェーダーをもとに戻す
		hs->Clean();
		ds->Clean();
		gs->Clean();
	}

	SceneFur::SceneFur() {
	}
	SceneFur::~SceneFur() {
		furShader->Clean();
	}
	void SceneFur::Initialize() {
		//カメラ作成
		view = std::make_unique<Graphics::View>(Math::Vector2(), Application::GetInstance()->GetWindow()->GetSize());
		pos = Math::Vector3(0.0f, 10.0f, -10.0f);
		at = Math::Vector3(0.0f, 0.0f, 0.0f);
		up = Math::Vector3(0.0f, 1.0f, 0.0f);
		//モデル読み込み
		model = std::make_unique<Graphics::Model>("Data/Model/plane.dxd", "Data/Model/plane.mt");
		furShader = std::make_unique<FurShader>();
		wireState = std::make_shared<Graphics::RasterizerState>(Graphics::RASTERIZER_PRESET::FRONT, true);
		HostConsole::GetInstance()->IntRegister("Fur Info", "wire", &wire, false);
		wire = FALSE;
		solidState = Graphics::Model::GetRasterizerState();
	}
	void SceneFur::AlwaysUpdate() {
		//カメラ移動
		//情報算出
		Math::Vector3 front;
		Math::Vector3 right;
		front = pos - at; front.Normalize();
		right = Math::Vector3::Cross(up, front);
		float elapsedTime = Application::GetInstance()->GetProcessTimeSec();
		//前後
		if (Input::GetKeyboardKey(DIK_S)) pos += front * 20.0f*elapsedTime;
		if (Input::GetKeyboardKey(DIK_W)) pos -= front * 20.0f*elapsedTime;
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
	void SceneFur::AlwaysRender() {
		view->Activate();
		//ワイヤーフレームか否か
		Graphics::Model::ChangeRasterizerState(wireState);
		//if (wire) Graphics::Model::ChangeRasterizerState(wireState);
		//else Graphics::Model::ChangeRasterizerState(solidState);
		model->Render();
		//Graphics::Model::ChangeRasterizerState(solidState);
		auto defaultVS = model->GetVertexShader();
		auto defaultPS = model->GetPixelShader();
		furShader->Activate(model);
		model->Render(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
		furShader->Clean();
		model->ChangeVertexShader(defaultVS);
		model->ChangePixelShader(defaultPS);
	}
}