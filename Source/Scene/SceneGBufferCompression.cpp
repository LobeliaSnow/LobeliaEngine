#include "Lobelia.hpp"
#include "SceneGBufferCompression.hpp"

namespace Lobelia::Game {
	GBufferManager::GBufferManager(const Math::Vector2& size) {
		viewport = std::make_unique<Graphics::View>(Math::Vector2(), size);
		//RenderTarget(const Math::Vector2& size, const DXGI_SAMPLE_DESC& sample, const DXGI_FORMAT&  format = DXGI_FORMAT_R32G32B32A32_FLOAT, int array_count = 1);
		for (int i = 0; i < rts.size(); i++) {
			//��񈳏k����Ă͂���
			rts[i] = std::make_unique<Graphics::RenderTarget>(size, DXGI_SAMPLE_DESC{ 1,0 }, DXGI_FORMAT_R32G32B32A32_UINT);
		}
		//�u�����f�B���O�Ȃ�
		blend = std::make_shared<Graphics::BlendState>(Graphics::BLEND_PRESET::NONE, false, false);
		//ShaderModel5.0�ȏ�K�{
		vs = std::make_shared<Graphics::VertexShader>("Data/ShaderFile/3D/GBufferCompression.hlsl", "CreateGBufferVS", Graphics::VertexShader::Model::VS_5_0);
		ps = std::make_shared<Graphics::PixelShader>("Data/ShaderFile/3D/GBufferCompression.hlsl", "CreateGBufferPS", Graphics::PixelShader::Model::PS_5_0);
		cbuffer = std::make_unique<Graphics::ConstantBuffer<Info>>(8, Graphics::ShaderStageList::VS | Graphics::ShaderStageList::PS);
	}
	void GBufferManager::AddModel(std::shared_ptr<Graphics::Model>& model) { modelList.push_back(model); }
	void GBufferManager::SetSkybox(std::shared_ptr<class SkyBox>& skybox) { this->skybox = skybox; }
	void GBufferManager::RenderGBuffer(std::shared_ptr<Graphics::View>& view) {
		for (int i = 0; i < rts.size(); i++) {
			rts[i]->Clear(0x00000000);
		}
		Graphics::RenderTarget::Activate(rts[0].get(), rts[1].get());
		viewport->ViewportActivate();
		//�V�F�[�_�[�̕ۊ�
		std::shared_ptr<Graphics::VertexShader> defaultVS = Graphics::Model::GetVertexShader();
		std::shared_ptr<Graphics::PixelShader> defaultPS = Graphics::Model::GetPixelShader();
		std::shared_ptr<Graphics::BlendState> defaultBlend = Graphics::Model::GetBlendState();
		//�X�e�[�g�̕ύX
		Graphics::Model::ChangeVertexShader(vs);
		Graphics::Model::ChangePixelShader(ps);
		Graphics::Model::ChangeBlendState(blend);
		//G-Buffer�ւ̏�������
		for (auto&& weak : modelList) {
			if (weak.expired())continue;
			std::shared_ptr<Graphics::Model> model = weak.lock();
			model->ChangeAnimVS(vs);
			model->Render();
		}
		//�X�e�[�g��߂�
		Graphics::Model::ChangeVertexShader(defaultVS);
		Graphics::Model::ChangePixelShader(defaultPS);
		Graphics::Model::ChangeBlendState(defaultBlend);
		ID3D11RenderTargetView* nullRT[2] = { nullptr,nullptr };
		Graphics::Device::GetContext()->OMSetRenderTargets(2, nullRT, nullptr);
		view->ViewportActivate();
		modelList.clear();
	}
	std::array<std::unique_ptr<Graphics::RenderTarget>, 2>& GBufferManager::GetRTs() { return rts; }
	DeferredShadeManager::DeferredShadeManager(const Math::Vector2& size) {
		viewport = std::make_unique<Graphics::View>(Math::Vector2(), size);
		//ShaderModel5.0�ȏ�K�{
		ps = std::make_shared<Graphics::PixelShader>("Data/ShaderFile/3D/GBufferCompression.hlsl", "DeferredPS", Graphics::PixelShader::Model::PS_5_0);
	}
	void DeferredShadeManager::Render(std::shared_ptr<Graphics::View>& view, std::shared_ptr<GBufferManager>& gbuffer) {
		viewport->ViewportActivate();
		//gbuffer->GetRTs()[0]->GetTexture()->Set(0, Graphics::ShaderStageList::PS);
		gbuffer->GetRTs()[1]->GetTexture()->Set(1, Graphics::ShaderStageList::PS);
		//�V�F�[�_�[�̕ۊ�
		std::shared_ptr<Graphics::PixelShader> defaultPS = Graphics::SpriteRenderer::GetPixelShader();
		//�X�e�[�g�̕ύX
		Graphics::SpriteRenderer::ChangePixelShader(ps);
		Graphics::SpriteRenderer::Render(gbuffer->GetRTs()[0]->GetTexture());
		//�X�e�[�g��߂�
		Graphics::SpriteRenderer::ChangePixelShader(defaultPS);
		for (int i = 0; i < gbuffer->GetRTs().size(); i++) {
			Graphics::Texture::Clean(i, Graphics::ShaderStageList::PS);
		}
		view->ViewportActivate();
	}

	//�f�R�[�h���ĉ�ʂɏo�͂��邽�߂̃V�F�[�_�[
	std::shared_ptr<Graphics::PixelShader> GBufferDecodeRenderer::psColor;
	std::shared_ptr<Graphics::PixelShader> GBufferDecodeRenderer::psDepth;
	std::shared_ptr<Graphics::PixelShader> GBufferDecodeRenderer::psWorldPos;
	std::shared_ptr<Graphics::PixelShader> GBufferDecodeRenderer::psNormal;
	void GBufferDecodeRenderer::Initialize() {
		psColor = std::make_shared<Graphics::PixelShader>("Data/ShaderFile/2D/GBufferRenderPS.hlsl", "DecodeSDRColorPS", Graphics::PixelShader::Model::PS_5_0);
		psDepth = std::make_shared<Graphics::PixelShader>("Data/ShaderFile/2D/GBufferRenderPS.hlsl", "DecodeDepthPS", Graphics::PixelShader::Model::PS_5_0);
		psWorldPos = std::make_shared<Graphics::PixelShader>("Data/ShaderFile/2D/GBufferRenderPS.hlsl", "DecodeWorldPosPS", Graphics::PixelShader::Model::PS_5_0);
		psNormal = std::make_shared<Graphics::PixelShader>("Data/ShaderFile/2D/GBufferRenderPS.hlsl", "DecodeNormalVectorPS", Graphics::PixelShader::Model::PS_5_0);
	}
	void GBufferDecodeRenderer::Finalize() {
		psColor.reset(); psDepth.reset(); psWorldPos.reset(); psNormal.reset();
	}
	void GBufferDecodeRenderer::ColorRender(std::shared_ptr<GBufferManager>& gbuffer, const Math::Vector2& pos, const Math::Vector2& size) {
		BufferRender(psColor, gbuffer->GetRTs()[0].get(), pos, size);
	}
	void GBufferDecodeRenderer::DepthRender(std::shared_ptr<GBufferManager>& gbuffer, const Math::Vector2& pos, const Math::Vector2& size) {
		BufferRender(psDepth, gbuffer->GetRTs()[0].get(), pos, size);
	}
	void GBufferDecodeRenderer::WorldPosRender(std::shared_ptr<GBufferManager>& gbuffer, const Math::Vector2& pos, const Math::Vector2& size) {
		BufferRender(psWorldPos, gbuffer->GetRTs()[0].get(), pos, size);
	}
	void GBufferDecodeRenderer::NormalRender(std::shared_ptr<GBufferManager>& gbuffer, const Math::Vector2& pos, const Math::Vector2& size) {
		BufferRender(psNormal, gbuffer->GetRTs()[0].get(), pos, size);
	}
	void GBufferDecodeRenderer::BufferRender(std::shared_ptr<Graphics::PixelShader>& ps, Graphics::RenderTarget* rt, const Math::Vector2& pos, const Math::Vector2& size) {
		std::shared_ptr<Graphics::PixelShader>& defaultPS = Graphics::SpriteRenderer::GetPixelShader();
		Graphics::SpriteRenderer::ChangePixelShader(ps);
		Graphics::SpriteRenderer::Render(rt, pos, size, 0.0f, Math::Vector2(), rt->GetTexture()->GetSize(), 0xFFFFFFFF);
		Graphics::SpriteRenderer::ChangePixelShader(defaultPS);
	}

	//�J�X�P�[�h�V���h�E��\�����邽�߂̂���
	CascadeShadowBuffers::CascadeShadowBuffers(int split_count, const Math::Vector2& size, FORMAT format, bool use_variance) {
		ChangeState(split_count, size, format, use_variance);
		vs = std::make_shared<Graphics::VertexShader>("Data/ShaderFile/3D/CascadeShadow.hlsl", "CreateShadowMapVS", Graphics::VertexShader::Model::VS_5_0);
		ps = std::make_shared<Graphics::PixelShader>("Data/ShaderFile/3D/CascadeShadow.hlsl", "CreateShadowMapPS", Graphics::PixelShader::Model::PS_5_0);
		debugPS = std::make_shared<Graphics::PixelShader>("Data/ShaderFile/3D/CascadeShadow.hlsl", "RenderShadowMapPS", Graphics::PixelShader::Model::PS_5_0);
		cbuffer = std::make_unique<Graphics::ConstantBuffer<Info>>(10, Graphics::ShaderStageList::VS | Graphics::ShaderStageList::PS);
		nearZ = 1.0f; farZ = 500.0f; lamda = 0.5f;
	}
	void CascadeShadowBuffers::SetNear(float near_z) { nearZ = near_z; }
	void CascadeShadowBuffers::SetFar(float far_z) { farZ = far_z; }
	void CascadeShadowBuffers::SetPos(const Math::Vector3& pos) { this->pos = pos; }
	void CascadeShadowBuffers::SetTarget(const Math::Vector3& at) { this->at = at; }
	void CascadeShadowBuffers::SetLamda(float lamda) { this->lamda = lamda; }
	void CascadeShadowBuffers::ChangeState(int split_count, const Math::Vector2& size, FORMAT format, bool use_variance) {
		info.splitCount = split_count;
		data.resize(info.splitCount);
		info.useVariance = use_variance;
		DXGI_FORMAT dxgiFormat;
		if (info.useVariance) {
			switch (format) {
			case FORMAT::BIT_16:dxgiFormat = DXGI_FORMAT_R16G16_FLOAT; break;
			case FORMAT::BIT_32:dxgiFormat = DXGI_FORMAT_R32G32_FLOAT; break;
			}
		}
		else {
			switch (format) {
			case FORMAT::BIT_16:dxgiFormat = DXGI_FORMAT_R16_FLOAT; break;
			case FORMAT::BIT_32:dxgiFormat = DXGI_FORMAT_R32_FLOAT; break;
			}
		}
		//�`��p�e�N�X�`���č쐬
		rts = std::make_unique<Graphics::RenderTarget>(size, DXGI_SAMPLE_DESC{ 1,0 }, dxgiFormat, info.splitCount);
		viewport = std::make_unique<Graphics::View>(Math::Vector2(), size);
		//1�v�f�� float(splitPos) 2�v�f�� float4x4(lvp)
		//�S����17�v�f�ł������A�e�N�X�`���̃T�C�Y��2�ׂ̂���ɂ��Ȃ��Ɠs���������̂�32
		dataSize = Math::Vector2(32, info.splitCount);
		//�f�[�^�i�[�p�e�N�X�`���쐬
		dataTexture = std::make_unique<Graphics::Texture>(dataSize, DXGI_FORMAT_R32_FLOAT, D3D11_BIND_SHADER_RESOURCE, DXGI_SAMPLE_DESC{ 1,0 }, Graphics::Texture::ACCESS_FLAG::DYNAMIC, Graphics::Texture::CPU_ACCESS_FLAG::WRITE);
	}
	void CascadeShadowBuffers::SetEnable(bool enable) { info.useShadowMap = int(enable); }
	void CascadeShadowBuffers::AddModel(std::shared_ptr<Graphics::Model>& model) { models.push_back(model); }
	void CascadeShadowBuffers::ClearModels() { models.clear(); }
	void CascadeShadowBuffers::RenderShadowMap(Graphics::View* active_view, Graphics::RenderTarget* active_rt) {
		if (!info.useShadowMap) {
			ClearModels();
			return;
		}
		Update(active_view);
		auto& defaultVS = Graphics::Model::GetVertexShader();
		auto& defaultPS = Graphics::Model::GetPixelShader();
		Graphics::Model::ChangeVertexShader(vs);
		Graphics::Model::ChangePixelShader(ps);
		viewport->ViewportActivate();
		rts->Clear(0xFFFFFFFF);
		rts->Activate();
		for (int i = 0; i < info.splitCount; i++) {
			info.lightViewProj = data[i].lvp;
			info.nowIndex = i;
			cbuffer->Activate(info);
			for (auto&& weak : models) {
				if (weak.expired())continue;
				auto model = weak.lock();
				model->ChangeAnimVS(vs);
				model->Render();
			}
		}
		ClearModels();
		Graphics::Model::ChangeVertexShader(defaultVS);
		Graphics::Model::ChangePixelShader(defaultPS);
		active_view->ViewportActivate();
		active_rt->Activate();
		////�K�E�X�ɂ��ڂ��� �o���A���X�p
		//if (info.useVariance) {
		//	for (int i = 0; i < count; i++) {
		//		gaussian[i]->Dispatch(active_view, active_rt, rts[i]->GetTexture());
		//	}
		//}
	}
	void CascadeShadowBuffers::DebugShadowMapRender(int index, const Math::Vector2& pos, const Math::Vector2& size) {
		auto& defaultPS = Graphics::SpriteRenderer::GetPixelShader();
		Graphics::SpriteRenderer::ChangePixelShader(debugPS);
		info.nowIndex = index;
		cbuffer->Activate(info);
		Graphics::SpriteRenderer::Render(rts->GetTexture(), pos, size, 0.0f, Math::Vector2(), rts->GetTexture()->GetSize(), 0xFFFFFFFF);
		Graphics::SpriteRenderer::ChangePixelShader(defaultPS);
	}
	namespace {
		//Vector3�̊e�v�f�ɔ�r�֐��������ĕԂ�
		auto ExecuteVector3Component(const Math::Vector3& v0, const Math::Vector3 v1, std::function<float(float, float)> func) {
			Math::Vector3 out = {};
			for (int i = 0; i < 3; i++) {
				out.v[i] = func(v0.v[i], v1.v[i]);
			}
			return out;
		}
	}
	CascadeShadowBuffers::AABB CascadeShadowBuffers::CalcFrustumAABB(Graphics::View* main_camera, float near_z, float far_z, const DirectX::XMFLOAT4X4& lvp) {
		//������̍쐬
		Math::FrustumVertices frustum = {};
		Math::CreateFrustumVertices(main_camera->GetEyePos(), main_camera->GetEyeTarget(), main_camera->GetEyeUpDirection(), main_camera->GetFov(), near_z, far_z, main_camera->GetAspect(), &frustum);
		//���W�X�^����邪�A���̉��̃X�R�[�v�ȊO�ł��g���̂ł�����
		DirectX::XMMATRIX calcLVP = DirectX::XMLoadFloat4x4(&lvp);
		AABB outAABB = {};
		outAABB.min = { FLT_MAX, FLT_MAX, FLT_MAX };
		outAABB.max = { FLT_MIN, FLT_MIN, FLT_MIN };
		DirectX::XMFLOAT4 storageVert(frustum[0].x, frustum[0].y, frustum[0].z, 1.0f);
		//AABB�����߂�
		for (int i = 0; i < 8; i++) {
			storageVert = { frustum[i].x, frustum[i].y, frustum[i].z, 1.0f };
			DirectX::XMVECTOR calcPos = DirectX::XMLoadFloat4(&storageVert);
			//�V���h�E�}�b�v�ˉe��ԂɎ����䒸�_��ϊ�
			calcPos = DirectX::XMVector4Transform(calcPos, calcLVP);
			//�v�Z���ʂ�Ԃ�
			DirectX::XMStoreFloat4(&storageVert, calcPos);
			outAABB.min = ExecuteVector3Component(outAABB.min, Math::Vector3(storageVert.x, storageVert.y, storageVert.z), [=](float a, float b) {return min(a, b); });
			outAABB.max = ExecuteVector3Component(outAABB.max, Math::Vector3(storageVert.x, storageVert.y, storageVert.z), [=](float a, float b) {return max(a, b); });
		}
		return outAABB;
	}
	void CascadeShadowBuffers::ComputeSplit(float lamda, float near_z, float far_z, float* split_pos) {
		//�ʏ�̃V���h�E�}�b�v
		if (info.splitCount == 1) {
			split_pos[0] = near_z;
			split_pos[1] = far_z;
			return;
		}
		//�J�X�P�[�h
		float invM = 1.0f / f_cast(info.splitCount);
		float farDivisionNear = far_z / near_z;
		float farSubNear = far_z - near_z;
		//���p�����X�L�[����K�p
		//http://http.developer.nvidia.com/GPUGems3/gpugems3_ch10.html
		for (int i = 1; i < info.splitCount + 1; i++) {
			//�ΐ������X�L�[��
			float log = near_z * powf(farDivisionNear, invM*i);
			//��l�����X�L�[��
			float uni = near_z + farSubNear * i*invM;
			//��L��2�̉��Z���ʂ���`��Ԃ���
			split_pos[i] = lamda * log + uni * (1.0f - lamda);
		}
		split_pos[0] = near_z;
		split_pos[info.splitCount] = far_z;
	}
	DirectX::XMFLOAT4X4 CascadeShadowBuffers::FrustumAABBClippingMatrix(AABB clip_aabb) {
		Math::Vector3 scale; Math::Vector3 offset;
		Math::Vector3 aabbSize = clip_aabb.max - clip_aabb.min;
		//�X�P�[����AABB�̃T�C�Y
		scale = Math::Vector3(2.0f, 2.0f, 2.0f) / (aabbSize);
		//�����]�T������
		scale *= 0.85f;
		//�ʒu��AABB�̒��S*�X�P�[��
		offset = -0.5f*(clip_aabb.max + clip_aabb.min)*scale;
		//�������Ȃ肷�����ꍇ�␳����
		scale = ExecuteVector3Component(scale, Math::Vector3(1.0f, 1.0f, 1.0f), [=](float a, float b) {return max(a, b); });
		scale.z = 1.0f; offset.z = 0.0f;
		//�s��쐬
		DirectX::XMFLOAT4X4 out = {};
		out._11 = scale.x;	out._12 = 0.0f;		out._13 = 0.0f;		out._14 = 0.0f;
		out._21 = 0.0f;		out._22 = scale.y;	out._23 = 0.0f;		out._24 = 0.0f;
		out._31 = 0.0f;		out._32 = 0.0f;		out._33 = scale.z;	out._34 = 0.0f;
		out._41 = offset.x; out._42 = offset.y;	out._43 = offset.z;	out._44 = 1.0f;
		return out;
	}
	void CascadeShadowBuffers::ComputeCascade(Graphics::View* main_camera, const DirectX::XMFLOAT4X4& lvp) {
		//�����ʒu�v�Z
		std::unique_ptr<float[]> splitPos = std::make_unique<float[]>(info.splitCount + 1);
		//���C���J�����𕪊�
		ComputeSplit(lamda, main_camera->GetNear(), main_camera->GetFar(), splitPos.get());
		for (int i = 0; i < info.splitCount; i++) {
			//�������ꂽ���C���J�����̎������AABB���v�Z(���[�J�����)
			AABB frustumAABB = CalcFrustumAABB(main_camera, splitPos[i], splitPos[i + 1], lvp);
			//�N���b�s���O�ˉe�s��
			DirectX::XMFLOAT4X4 clipMatrix = FrustumAABBClippingMatrix(frustumAABB);
			//�s��ƕ�������
			//�Ƃ肠�������Ń��[�J���ɒu��
			DirectX::XMMATRIX calcLVP = DirectX::XMLoadFloat4x4(&lvp);
			DirectX::XMMATRIX calcClip = DirectX::XMLoadFloat4x4(&clipMatrix);
			DirectX::XMMATRIX cascadeLVP = calcLVP * calcClip;
			DirectX::XMStoreFloat4x4(&data[i].lvp, DirectX::XMMatrixTranspose(cascadeLVP));
			data[i].splitDist = splitPos[i + 1];
		}
	}
	void CascadeShadowBuffers::Update(Graphics::View* main_camera) {
		//���C�g�s��쐬
		Math::Vector3 front = at - pos; front.Normalize();
		up = Math::Vector3(0.0f, 1.0f, 0.0f); up.Normalize();
		Math::Vector3 right = Math::Vector3::Cross(up, front); right.Normalize();
		up = Math::Vector3::Cross(front, right);
		DirectX::XMFLOAT4X4 lvp = {};
		{//���W�X�^����ϐ�������̂ŃX�R�[�v��؂�
			DirectX::XMFLOAT4 storage(pos.x, pos.y, pos.z, 1.0f);
			DirectX::XMVECTOR calcPos = DirectX::XMLoadFloat4(&storage);
			storage = { at.x, at.y, at.z, 1.0f };
			DirectX::XMVECTOR calcAt = DirectX::XMLoadFloat4(&storage);
			storage = { up.x, up.y, up.z, 1.0f };
			DirectX::XMVECTOR calcUp = DirectX::XMLoadFloat4(&storage);
			DirectX::XMMATRIX lightView = DirectX::XMMatrixLookAtLH(calcPos, calcAt, calcUp);
			DirectX::XMMATRIX lightProjection = DirectX::XMMatrixOrthographicLH(400, 400, nearZ, farZ);
			//DirectX::XMMATRIX lightProjection = DirectX::XMMatrixPerspectiveFovLH(fov, aspect, nearZ, farZ);
			//�g���܂킵
			lightView = lightView * lightProjection;
			DirectX::XMStoreFloat4x4(&lvp, lightView);
		}
		ComputeCascade(main_camera, lvp);
	}

	//-------------------------------------------------------------------------------------------------------------------
	//
	//		G-Buffer���k�p�̃e�X�g�V�[���ł�
	//
	//-------------------------------------------------------------------------------------------------------------------
	void SceneGBufferCompression::Initialize() {
		const Math::Vector2 wsize = Application::GetInstance()->GetWindow()->GetSize();
		camera = std::make_unique<ViewerCamera>(wsize, Math::Vector3(0.0f, 10.0f, 10.0f), Math::Vector3());
		offScreen = std::make_shared<Graphics::RenderTarget>(wsize, DXGI_SAMPLE_DESC{ 1,0 }, DXGI_FORMAT_R8G8B8A8_UNORM);
		gbuffer = std::make_shared<GBufferManager>(wsize);
		deferredShader = std::make_unique<DeferredShadeManager>(wsize);
		//int split_count, const Math::Vector2& size, FORMAT format, bool use_variance
		cascadeCount = 4; shadowMapSize = Math::Vector2(512.0f, 512.0f)*2.0f;
		rad = 0.0f;
		useShadow = true; renderShadowMap = true; useVariance = true;
		shadowMap = std::make_unique<CascadeShadowBuffers>(cascadeCount, shadowMapSize, CascadeShadowBuffers::FORMAT::BIT_16, useVariance);
		//const Math::Vector2& size, const DXGI_SAMPLE_DESC& sample, const DXGI_FORMAT&  format = DXGI_FORMAT_R32G32B32A32_FLOAT, int array_count = 1);
		stage = std::make_shared<Graphics::Model>("Data/Model/maps/stage.dxd", "Data/Model/maps/stage.mt");
		stage->Translation(Math::Vector3(0.0f, 1.0f, 0.0f));
		//stage->Scalling(3.0f);
		stage->CalcWorldMatrix();
		GBufferDecodeRenderer::Initialize();
		//�f���p
		renderGBuffer = true; renderColor = true; renderDepth = true; renderWPos = true; renderNormal = true;
		operationConsole = std::make_unique<AdaptiveConsole>("Operation Console");
		//G-Buffer
		operationConsole->AddFunction([this]() {
			ImGui::Checkbox("Render G-Buffer", &renderGBuffer);
			if (renderGBuffer&&ImGui::TreeNode("Render Flag")) {
				ImGui::Checkbox("Color", &renderColor);
				ImGui::Checkbox("Depth", &renderDepth);
				ImGui::Checkbox("World Pos", &renderWPos);
				ImGui::Checkbox("Normal", &renderNormal);
				ImGui::TreePop();
			}
		});
		//Shadow Map
		operationConsole->AddFunction([this]() {
			ImGui::Checkbox("Use Shadow Map", &useShadow);
			if (renderGBuffer&&ImGui::TreeNode("Shadow Parameters")) {
				ImGui::Checkbox("Use Variance", &useVariance);
				ImGui::SliderFloat2("size", shadowMapSize.v, 256.0f, 4096);
				if (ImGui::Button("Apply")) {
					shadowMap->ChangeState(cascadeCount, shadowMapSize, CascadeShadowBuffers::FORMAT::BIT_16, useVariance);
				}
				ImGui::TreePop();
			}
		});
	}
	SceneGBufferCompression::~SceneGBufferCompression() {
		GBufferDecodeRenderer::Finalize();
	}
	void SceneGBufferCompression::AlwaysUpdate() {
		camera->Update();
		gbuffer->AddModel(stage);
		shadowMap->AddModel(stage);
		//��]���C�g
		rad += Application::GetInstance()->GetProcessTimeSec()*0.1f;
		lpos = Math::Vector3(sinf(rad), 0.0f, cos(rad))*100.0f;
		//lpos.x = lpos.z = 0.0f;
		lpos.y = 100.0f;
		//�Œ胉�C�g
		//lpos = Math::Vector3(200.0f, 130.0f, 200.0f);
		shadowMap->SetPos(lpos);
		shadowMap->SetTarget(Math::Vector3());
		shadowMap->SetNear(1.0f);
		shadowMap->SetFar(1000.0f);
		shadowMap->SetLamda(0.8f);
		shadowMap->SetEnable(useShadow);
	}
	void SceneGBufferCompression::AlwaysRender() {
		Graphics::Environment::GetInstance()->SetLightDirection(Math::Vector3(-1.0f, -1.0f, -1.0f));
		Graphics::Environment::GetInstance()->Activate();
		shadowMap->RenderShadowMap(camera->GetView().get(), offScreen.get());
		//�o�b�N�o�b�t�@��L����
		offScreen->Clear(0x00000000);
		offScreen->Activate();
		camera->Activate();
		gbuffer->RenderGBuffer(camera->GetView());
		offScreen->Activate();
		deferredShader->Render(camera->GetView(), gbuffer);
		Application::GetInstance()->GetSwapChain()->GetRenderTarget()->Activate();
		Graphics::SpriteRenderer::Render(offScreen.get());
		Graphics::Texture::Clean(0, Graphics::ShaderStageList::PS);
		if (Application::GetInstance()->debugRender) {
			operationConsole->Update();
			//G-Buffer�f�o�b�O�`��
			if (renderGBuffer) {
				if (renderColor) GBufferDecodeRenderer::ColorRender(gbuffer, Math::Vector2(0.0f, 0.0f), Math::Vector2(100.0f, 100.0f));
				if (renderDepth) GBufferDecodeRenderer::DepthRender(gbuffer, Math::Vector2(100.0f, 0.0f), Math::Vector2(100.0f, 100.0f));
				if (renderWPos) GBufferDecodeRenderer::WorldPosRender(gbuffer, Math::Vector2(200.0f, 0.0f), Math::Vector2(100.0f, 100.0f));
				if (renderNormal) GBufferDecodeRenderer::NormalRender(gbuffer, Math::Vector2(300.0f, 0.0f), Math::Vector2(100.0f, 100.0f));
			}
			//Shadow Map�f�o�b�O�`��
			if (useShadow&&renderShadowMap) {
				for (int i = 0; i < cascadeCount; i++) {
					shadowMap->DebugShadowMapRender(i, Math::Vector2(100.0f*i, 100.0f), Math::Vector2(100.0f, 100.0f));
				}
			}
			Graphics::Texture::Clean(0, Graphics::ShaderStageList::PS);
		}
	}

}