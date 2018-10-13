#include "Lobelia.hpp"
#include "SceneFur.hpp"

namespace Lobelia::Game {
	FurShader::FurShader() {
		//�V�F�[�_�[�̃��[�h
		vs = std::make_shared<Graphics::VertexShader>("Data/ShaderFile/3D/Fur.hlsl", "VS", Graphics::VertexShader::Model::VS_5_0);
		hs = std::make_shared<Graphics::HullShader>("Data/ShaderFile/3D/Fur.hlsl", "HS");
		ds = std::make_shared<Graphics::DomainShader>("Data/ShaderFile/3D/Fur.hlsl", "DS");
		gs = std::make_shared<Graphics::GeometryShader>("Data/ShaderFile/3D/Fur.hlsl", "GS");
		ps = std::make_shared<Graphics::PixelShader>("Data/ShaderFile/3D/Fur.hlsl", "PS", Graphics::PixelShader::Model::PS_5_0);
		//�R���X�^���g�o�b�t�@�쐬
		constantBufferSeaInfo = std::make_unique<Graphics::ConstantBuffer<FurInfo>>(6, Graphics::ShaderStageList::HS | Graphics::ShaderStageList::DS | Graphics::ShaderStageList::GS);
		furInfo.min = 0.0f;
		furInfo.max = 20.0f;
		furInfo.maxDevide = 64;
		furInfo.length = 1.0f;
	}
	FurShader::~FurShader() {

	}
	void FurShader::Activate(std::shared_ptr<Graphics::Model> model) {
		//�V�F�[�_�[�̕ύX���Z�b�g
		model->ChangeVertexShader(vs);
		model->ChangePixelShader(ps);
		hs->Set();
		ds->Set();
		gs->Set();
		//�R���X�^���g�o�b�t�@�ƃe�N�X�`���̍X�V
		constantBufferSeaInfo->Activate(furInfo);
	}
	void FurShader::Clean() {
		//���Ŏg��Ȃ��V�F�[�_�[�����Ƃɖ߂�
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
		//�J�����쐬
		view = std::make_unique<Graphics::View>(Math::Vector2(), Application::GetInstance()->GetWindow()->GetSize());
		pos = Math::Vector3(0.0f, 10.0f, -10.0f);
		at = Math::Vector3(0.0f, 0.0f, 0.0f);
		up = Math::Vector3(0.0f, 1.0f, 0.0f);
		//���f���ǂݍ���
		model = std::make_unique<Graphics::Model>("Data/Model/plane.dxd", "Data/Model/plane.mt");
		furShader = std::make_unique<FurShader>();
		wireState = std::make_shared<Graphics::RasterizerState>(Graphics::RasterizerPreset::FRONT, true);
		HostConsole::GetInstance()->IntRegister("Fur Info", "wire", &wire, false);
		wire = FALSE;
		solidState = Graphics::Model::GetRasterizerState();
	}
	void SceneFur::AlwaysUpdate() {
		//�J�����ړ�
		//���Z�o
		Math::Vector3 front;
		Math::Vector3 right;
		front = pos - at; front.Normalize();
		right = Math::Vector3::Cross(up, front);
		float elapsedTime = Application::GetInstance()->GetProcessTimeSec();
		//�O��
		if (Input::GetKeyboardKey(DIK_S)) pos += front * 20.0f*elapsedTime;
		if (Input::GetKeyboardKey(DIK_W)) pos -= front * 20.0f*elapsedTime;
		//���E
		if (Input::GetKeyboardKey(DIK_A)) pos -= right * 20.0f*elapsedTime;
		if (Input::GetKeyboardKey(DIK_D)) pos += right * 20.0f*elapsedTime;
		//�㉺
		if (Input::GetKeyboardKey(DIK_Z)) pos += up * 20.0f*elapsedTime;
		if (Input::GetKeyboardKey(DIK_X)) pos -= up * 20.0f*elapsedTime;
		//�J�����X�V
		view->SetEyePos(pos);
		view->SetEyeTarget(at);
		view->SetEyeUpDirection(up);
	}
	void SceneFur::AlwaysRender() {
		view->Activate();
		//���C���[�t���[�����ۂ�
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