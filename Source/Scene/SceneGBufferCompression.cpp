#include "Lobelia.hpp"
#include "SceneGBufferCompression.hpp"
#include "Common/Camera.hpp"
#include "Common/SkyBox.hpp"
//�^�C���x�[�X���\��Ȃ̂ŎQ�l����
//http://momose-d.cocolog-nifty.com/blog/2014/03/post-a593.html
//https://software.intel.com/en-us/articles/deferred-rendering-for-current-and-future-rendering-pipelines/

//���P�x�̕����𒊏o���ău���[���|����悤�ɂ���

//���C�g�͂Ƃ肠�����ȈՓI�ɂ���
//��C�U���V�~�����[�V������������

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
	void GBufferManager::AddModel(std::shared_ptr<Graphics::Model>& model, Info info) { modelList.push_back(ModelStorage{ model,info }); }
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
		if (skybox) {
			static Info info = { 0.0f,0.0f,0.0f };
			cbuffer->Activate(info);
			skybox->Render();
		}
		//G-Buffer�ւ̏�������
		for (auto&& weak : modelList) {
			if (weak.model.expired())continue;
			std::shared_ptr<Graphics::Model> model = weak.model.lock();
			model->ChangeAnimVS(vs);
			cbuffer->Activate(weak.info);
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
		vs = std::make_shared<Graphics::VertexShader>("Data/ShaderFile/3D/GBufferCompression.hlsl", "DeferredVS", Graphics::VertexShader::Model::VS_5_0);
		//ShaderModel5.0�ȏ�K�{
		ps = std::make_shared<Graphics::PixelShader>("Data/ShaderFile/3D/GBufferCompression.hlsl", "DeferredPS", Graphics::PixelShader::Model::PS_5_0);
	}
	void DeferredShadeManager::Render(std::shared_ptr<Graphics::View>& view, std::shared_ptr<GBufferManager>& gbuffer) {
		view->Activate();
		viewport->ViewportActivate();
		//gbuffer->GetRTs()[0]->GetTexture()->Set(0, Graphics::ShaderStageList::PS);
		gbuffer->GetRTs()[1]->GetTexture()->Set(1, Graphics::ShaderStageList::PS);
		//�V�F�[�_�[�̕ۊ�
		std::shared_ptr<Graphics::PixelShader> defaultPS = Graphics::SpriteRenderer::GetPixelShader();
		std::shared_ptr<Graphics::VertexShader> defaultVS = Graphics::SpriteRenderer::GetVertexShader();
		//�X�e�[�g�̕ύX
		Graphics::SpriteRenderer::ChangeVertexShader(vs);
		Graphics::SpriteRenderer::ChangePixelShader(ps);
		Graphics::SpriteRenderer::Render(gbuffer->GetRTs()[0]->GetTexture());
		//�X�e�[�g��߂�
		Graphics::SpriteRenderer::ChangeVertexShader(defaultVS);
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
	std::shared_ptr<Graphics::PixelShader> GBufferDecodeRenderer::psLightingIntensity;
	std::shared_ptr<Graphics::PixelShader> GBufferDecodeRenderer::psSpecularIntensity;
	std::shared_ptr<Graphics::PixelShader> GBufferDecodeRenderer::psEmission;
	std::shared_ptr<Graphics::PixelShader> GBufferDecodeRenderer::psEmissionIntensity;
	void GBufferDecodeRenderer::Initialize() {
		psColor = std::make_shared<Graphics::PixelShader>("Data/ShaderFile/2D/GBufferRenderPS.hlsl", "DecodeSDRColorPS", Graphics::PixelShader::Model::PS_5_0);
		psDepth = std::make_shared<Graphics::PixelShader>("Data/ShaderFile/2D/GBufferRenderPS.hlsl", "DecodeDepthPS", Graphics::PixelShader::Model::PS_5_0);
		psWorldPos = std::make_shared<Graphics::PixelShader>("Data/ShaderFile/2D/GBufferRenderPS.hlsl", "DecodeWorldPosPS", Graphics::PixelShader::Model::PS_5_0);
		psNormal = std::make_shared<Graphics::PixelShader>("Data/ShaderFile/2D/GBufferRenderPS.hlsl", "DecodeNormalVectorPS", Graphics::PixelShader::Model::PS_5_0);
		psLightingIntensity = std::make_shared<Graphics::PixelShader>("Data/ShaderFile/2D/GBufferRenderPS.hlsl", "DecodeLightingIntensityPS", Graphics::PixelShader::Model::PS_5_0);
		psSpecularIntensity = std::make_shared<Graphics::PixelShader>("Data/ShaderFile/2D/GBufferRenderPS.hlsl", "DecodeSpecularIntensityPS", Graphics::PixelShader::Model::PS_5_0);
		psEmission = std::make_shared<Graphics::PixelShader>("Data/ShaderFile/2D/GBufferRenderPS.hlsl", "DecodeEmissionPS", Graphics::PixelShader::Model::PS_5_0);
		psEmissionIntensity = std::make_shared<Graphics::PixelShader>("Data/ShaderFile/2D/GBufferRenderPS.hlsl", "DecodeEmissionIntensityPS", Graphics::PixelShader::Model::PS_5_0);
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
	void GBufferDecodeRenderer::LightingIntensityRender(std::shared_ptr<GBufferManager>& gbuffer, const Math::Vector2& pos, const Math::Vector2& size) {
		BufferRender(psLightingIntensity, gbuffer->GetRTs()[0].get(), pos, size);
	}
	void GBufferDecodeRenderer::SpecularIntensityRender(std::shared_ptr<GBufferManager>& gbuffer, const Math::Vector2& pos, const Math::Vector2& size) {
		BufferRender(psSpecularIntensity, gbuffer->GetRTs()[0].get(), pos, size);
	}
	void GBufferDecodeRenderer::EmissionRender(std::shared_ptr<GBufferManager>& gbuffer, const Math::Vector2& pos, const Math::Vector2& size) {
		BufferRender(psEmission, gbuffer->GetRTs()[1].get(), pos, size);
	}
	void GBufferDecodeRenderer::BufferRender(std::shared_ptr<Graphics::PixelShader>& ps, Graphics::RenderTarget* rt, const Math::Vector2& pos, const Math::Vector2& size) {
		std::shared_ptr<Graphics::PixelShader>& defaultPS = Graphics::SpriteRenderer::GetPixelShader();
		Graphics::SpriteRenderer::ChangePixelShader(ps);
		Graphics::SpriteRenderer::Render(rt, pos, size, 0.0f, Math::Vector2(), rt->GetTexture()->GetSize(), 0xFFFFFFFF);
		Graphics::SpriteRenderer::ChangePixelShader(defaultPS);
	}
	//-------------------------------------------------------------------------------------------------------------------
	//
	//		�����_�[�^�[�Q�b�g�z��p
	//
	//-------------------------------------------------------------------------------------------------------------------
	GaussianTextureArray::GaussianTextureArray(const Math::Vector2& size, int array_count, DXGI_FORMAT format) {
		vsX = std::make_shared<Graphics::VertexShader>("Data/ShaderFile/3D/RenderCascadeShadowMap.hlsl", "GaussianFilterVSX", Graphics::VertexShader::Model::VS_5_0);
		vsY = std::make_shared<Graphics::VertexShader>("Data/ShaderFile/3D/RenderCascadeShadowMap.hlsl", "GaussianFilterVSY", Graphics::VertexShader::Model::VS_5_0);
		ps = std::make_shared<Graphics::PixelShader>("Data/ShaderFile/3D/RenderCascadeShadowMap.hlsl", "GaussianFilterPS", Graphics::PixelShader::Model::PS_5_0);
		cbuffer = std::make_unique<Graphics::ConstantBuffer<Info>>(5, Graphics::ShaderStageList::VS | Graphics::ShaderStageList::PS);
		ChangeBuffer(size, array_count, format);
	}
	//�T�C�Y���̕ύX
	void GaussianTextureArray::ChangeBuffer(const Math::Vector2& size, int array_count, DXGI_FORMAT format) {
		arrayCount = array_count;
		rtsX = std::make_unique<Graphics::RenderTarget>(size, DXGI_SAMPLE_DESC{ 1,0 }, format, array_count);
		rtsResult = std::make_unique<Graphics::RenderTarget>(size, DXGI_SAMPLE_DESC{ 1,0 }, format, array_count);
		viewport = std::make_unique<Graphics::View>(Math::Vector2(), size);
	}
	void GaussianTextureArray::Update(float dispersion) {
		if (dispersion <= 0.0f)dispersion = 0.01f;
		float total = 0.0f;
		// �K�E�X�֐��ɂ��d�݂̌v�Z
		for (int i = 0; i < DIVISION; i++) {
			float pos = (float)i * 2.0f;
			info.weight[i] = expf(-pos * pos * dispersion);
			total += info.weight[i];
		}
		// �d�݂̋K�i��
		for (int i = 0; i < DIVISION; i++) {
			info.weight[i] = info.weight[i] / total * 0.5f;
		}
	}
	void GaussianTextureArray::Dispatch(Graphics::RenderTarget* active_rt, Graphics::View*active_view, Graphics::Texture* tex) {
		viewport->ViewportActivate();
		info.texSize = tex->GetSize();
		auto defaultVS = Graphics::SpriteRenderer::GetVertexShader();
		auto defaultPS = Graphics::SpriteRenderer::GetPixelShader();
		rtsX->Clear(0x00000000);
		rtsX->Activate();
		Graphics::SpriteRenderer::ChangeVertexShader(vsX);
		Graphics::SpriteRenderer::ChangePixelShader(ps);
		for (int i = 0; i < arrayCount; i++) {
			info.texIndex = i;
			cbuffer->Activate(info);
			Graphics::SpriteRenderer::Render(tex);
		}
		rtsResult->Clear(0x00000000);
		rtsResult->Activate();
		Graphics::SpriteRenderer::ChangeVertexShader(vsY);
		for (int i = 0; i < arrayCount; i++) {
			info.texIndex = i;
			cbuffer->Activate(info);
			Graphics::SpriteRenderer::Render(rtsX.get());
		}
		Graphics::Texture::Clean(0, Graphics::ShaderStageList::PS);
		Graphics::SpriteRenderer::ChangeVertexShader(defaultVS);
		Graphics::SpriteRenderer::ChangePixelShader(defaultPS);
		active_rt->Activate();
		active_view->ViewportActivate();
	}
	std::shared_ptr<Graphics::RenderTarget>& GaussianTextureArray::GetRTSResult() { return rtsResult; }
	//-------------------------------------------------------------------------------------------------------------------
	//
	//		�ϒ��J�X�P�[�h�V���h�E
	//
	//-------------------------------------------------------------------------------------------------------------------
	//�J�X�P�[�h�V���h�E��\�����邽�߂̂���
	CascadeShadowBuffers::CascadeShadowBuffers(int split_count, const Math::Vector2& size, FORMAT format, bool use_variance) {
		ChangeState(split_count, size, format, use_variance);
		vs = std::make_shared<Graphics::VertexShader>("Data/ShaderFile/3D/RenderCascadeShadowMap.hlsl", "CreateShadowMapVS", Graphics::VertexShader::Model::VS_5_0);
		ps = std::make_shared<Graphics::PixelShader>("Data/ShaderFile/3D/RenderCascadeShadowMap.hlsl", "CreateShadowMapPS", Graphics::PixelShader::Model::PS_5_0);
		debugPS = std::make_shared<Graphics::PixelShader>("Data/ShaderFile/3D/RenderCascadeShadowMap.hlsl", "RenderShadowMapPS", Graphics::PixelShader::Model::PS_5_0);
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
		if (info.useVariance) {
			if (!gaussian)gaussian = std::make_unique<GaussianTextureArray>(size, split_count, dxgiFormat);
			else gaussian->ChangeBuffer(size, split_count, dxgiFormat);
			gaussian->Update(1.0f);
		}
		else if (gaussian) gaussian.reset();
		viewport = std::make_unique<Graphics::View>(Math::Vector2(), size);
		//1�v�f�� float(splitPos) 2�v�f�� float4x4(lvp)
		//�S����17�v�f�ł������A�e�N�X�`���̃T�C�Y��2�ׂ̂���ɂ��Ȃ��Ɠs���������̂�32
		dataSize = Math::Vector2(8, info.splitCount);
		//�f�[�^�i�[�p�e�N�X�`���쐬
		dataTexture = std::make_unique<Graphics::Texture>(dataSize, DXGI_FORMAT_R32G32B32A32_FLOAT, D3D11_BIND_SHADER_RESOURCE, DXGI_SAMPLE_DESC{ 1,0 }, Graphics::Texture::ACCESS_FLAG::DYNAMIC, Graphics::Texture::CPU_ACCESS_FLAG::WRITE);
	}
	void CascadeShadowBuffers::SetEnable(bool enable) { info.useShadowMap = int(enable); }
	void CascadeShadowBuffers::AddModel(std::shared_ptr<Graphics::Model>& model) { models.push_back(model); }
	void CascadeShadowBuffers::ClearModels() { models.clear(); }
	void CascadeShadowBuffers::RenderShadowMap(Graphics::View* active_view, Graphics::RenderTarget* active_rt) {
		if (!info.useShadowMap) {
			cbuffer->Activate(info);
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
		//�K�E�X�ɂ��ڂ��� �o���A���X�p
		if (info.useVariance) gaussian->Dispatch(active_rt, active_view, rts->GetTexture());
		else {
			active_view->ViewportActivate();
			active_rt->Activate();
		}
		Math::Vector3 lightVec = at - pos; lightVec.Normalize();
		Graphics::Environment::GetInstance()->SetLightDirection(lightVec);
	}
	void CascadeShadowBuffers::DebugShadowMapRender(int index, const Math::Vector2& pos, const Math::Vector2& size) {
		auto& defaultPS = Graphics::SpriteRenderer::GetPixelShader();
		Graphics::SpriteRenderer::ChangePixelShader(debugPS);
		info.nowIndex = index;
		cbuffer->Activate(info);
		if (info.useVariance)Graphics::SpriteRenderer::Render(gaussian->GetRTSResult()->GetTexture(), pos, size, 0.0f, Math::Vector2(), rts->GetTexture()->GetSize(), 0xFFFFFFFF);
		else Graphics::SpriteRenderer::Render(rts->GetTexture(), pos, size, 0.0f, Math::Vector2(), rts->GetTexture()->GetSize(), 0xFFFFFFFF);
		Graphics::SpriteRenderer::ChangePixelShader(defaultPS);
	}
	void CascadeShadowBuffers::DebugDataTextureRender(const Math::Vector2& pos, const Math::Vector2& size) {
		Graphics::SpriteRenderer::Render(dataTexture.get(), pos, size, 0.0f, Math::Vector2(), dataSize, 0xFFFFFFFF);
	}
	void CascadeShadowBuffers::SetShadowMap(int shadow_map_slot, int data_slot) {
		D3D11_MAPPED_SUBRESOURCE resource;
		Graphics::Device::GetContext()->Map(dataTexture->Get().Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
		//Split��񓙏�������
		for (int instance = 0; instance < i_cast(dataSize.y); instance++) {
			int x = 0;
			float* texture = s_cast<float*>(resource.pData);
			//���C�g�s��̏�������
			memcpy_s(&texture[instance * i_cast(dataSize.x) * 4 + x], sizeof(data[instance].lvp), &data[instance].lvp, sizeof(data[instance].lvp));
			x += 4 * 4;//���C�g��񕪐i�߂�
			//������񏑂�����b
			texture[instance * i_cast(dataSize.x) * 4 + x] = data[instance].splitDist;
			x++;//float1����֐i�߂�(����ȗ��\)
		}
		Graphics::Device::GetContext()->Unmap(dataTexture->Get().Get(), 0);
		//�e�N�X�`���̃Z�b�g
		if (info.useVariance)gaussian->GetRTSResult()->GetTexture()->Set(shadow_map_slot, Graphics::ShaderStageList::PS);
		else rts->GetTexture()->Set(shadow_map_slot, Graphics::ShaderStageList::PS);
		dataTexture->Set(data_slot, Graphics::ShaderStageList::PS);
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
			DirectX::XMMATRIX lightProjection = DirectX::XMMatrixOrthographicLH(500, 500, nearZ, farZ);
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
		camera = std::make_shared<ViewerCamera>(wsize, Math::Vector3(57.0f, 66.0f, 106.0f), Math::Vector3());
		offScreen = std::make_shared<Graphics::RenderTarget>(wsize, DXGI_SAMPLE_DESC{ 1,0 }, DXGI_FORMAT_R8G8B8A8_UNORM);
		offScreenPost = std::make_shared<Graphics::RenderTarget>(wsize, DXGI_SAMPLE_DESC{ 1,0 }, DXGI_FORMAT_R32G32B32A32_FLOAT);
		depth = std::make_shared<Graphics::RenderTarget>(wsize, DXGI_SAMPLE_DESC{ 1,0 }, DXGI_FORMAT_R32_FLOAT);
		emission = std::make_shared<Graphics::RenderTarget>(wsize, DXGI_SAMPLE_DESC{ 1,0 }, DXGI_FORMAT_R16G16B16A16_FLOAT);
		gbuffer = std::make_shared<GBufferManager>(wsize);
		deferredShader = std::make_unique<DeferredShadeManager>(wsize);
		bloom = std::make_unique<Experimental::MultipleGaussianBloom>(wsize);
		//�V���h�E�}�b�v����p�p�����[�^�[
		cascadeCount = 4; shadowMapSize = Math::Vector2(1024.0f, 1024.0f);
		rad = 0.0f; shadowNearZ = 25.0f; shadowFarZ = 350.0f; shadowLamda = 0.8f;
		//�f���p�p�����[�^�[
		useShadow = true; renderShadowMap = true; useVariance = true; useSSAO = true; renderSSAO = true;
		ssaoThreshold = 5.0f; cameraMove = true; renderShadowMap = true; useDoF = true; renderDoFBuffer = true;
		renderLightingIntensity = true; renderSpecularIntensity = true; renderEmissionColor = true;
		focusRange = 150.0f; renderShadingBuffer = true; renderBloomBuffer = true; useBloom = true;
		useTonemap = true; renderTonemapBuffer = true; renderAvgLuminance = true; renderExposure = true;
		//���f���`��p�p�����[�^�[
		stageInfo.lightingFactor = 1.0f; stageInfo.specularFactor = 0.0f; stageInfo.emissionFactor = 0.0f;
		boxInfo.lightingFactor = 0.0f; boxInfo.specularFactor = 1.0f; boxInfo.emissionFactor = 5.0f;
		shadowMap = std::make_unique<CascadeShadowBuffers>(cascadeCount, shadowMapSize, CascadeShadowBuffers::FORMAT::BIT_16, useVariance);
		//const Math::Vector2& size, const DXGI_SAMPLE_DESC& sample, const DXGI_FORMAT&  format = DXGI_FORMAT_R32G32B32A32_FLOAT, int array_count = 1);
		skybox = std::make_shared<SkyBox>("Data/Model/skybox.dxd", "Data/Model/skybox.mt");
		skybox->SetCamera(camera);
		gbuffer->SetSkybox(skybox);
		stage = std::make_shared<Graphics::Model>("Data/Model/maps/stage.dxd", "Data/Model/maps/stage.mt");
		stage->Translation(Math::Vector3(0.0f, 1.0f, 0.0f));
		//stage->Scalling(3.0f);
		stage->CalcWorldMatrix();
		box = std::make_shared<Graphics::Model>("Data/Model/box.dxd", "Data/Model/box.mt");
		box->Translation(Math::Vector3(0.0f, 5.0f, 0.0f));
		box->Scalling(3.0f);
		//box->Scalling(9.0f);
		box->CalcWorldMatrix();
		ssao = std::make_unique<Experimental::SSAO>(wsize*0.5f);
		dof = std::make_unique<Experimental::DepthOfField>(wsize, 0.5f);
		tonemap = std::make_unique<Experimental::ToneMap>(wsize);
		GBufferDecodeRenderer::Initialize();
		//�f���p
		renderGBuffer = true; renderColor = true; renderDepth = true; renderWPos = true; renderNormal = true;
		operationConsole = std::make_unique<AdaptiveConsole>("Operation Console");
		//operationConsole->AddFunction([this]() {
		//});
		operationConsole->AddFunction([this]() {
			ImGui::Checkbox("Camera Move", &cameraMove);
			ImGui::SliderFloat("Lighting Factor", &stageInfo.lightingFactor, 0.0f, 1.0f);
		});
		//G-Buffer
		operationConsole->AddFunction([this]() {
			ImGui::Checkbox("Render Buffer", &renderGBuffer);
			if (renderGBuffer&&ImGui::TreeNode("Render Flag")) {
				if (ImGui::TreeNode("G-Buffer 0 RGBA32_UINT")) {
					ImGui::Checkbox("Color", &renderColor);
					ImGui::Checkbox("Depth", &renderDepth);
					ImGui::Checkbox("World Pos", &renderWPos);
					ImGui::Checkbox("Normal", &renderNormal);
					ImGui::Checkbox("Lighting Intensity", &renderLightingIntensity);
					ImGui::Checkbox("Specular Intensity", &renderSpecularIntensity);
					ImGui::TreePop();
				}
				if (ImGui::TreeNode("G-Buffer 1 RGBA32_UINT")) {
					ImGui::Checkbox("Emission Color", &renderEmissionColor);
					ImGui::Text("Three element is empty");
					ImGui::TreePop();
				}
				if (ImGui::TreeNode("Other")) {
					ImGui::Checkbox("SSAO", &renderSSAO);
					ImGui::Checkbox("Shading", &renderShadingBuffer);
					ImGui::Checkbox("ShadowMap", &renderShadowMap);
					ImGui::Checkbox("DoF", &renderDoFBuffer);
					ImGui::Checkbox("Bloom", &renderBloomBuffer);
					ImGui::Checkbox("Tonemap", &renderTonemapBuffer);
					ImGui::Checkbox("Avg Luminace", &renderAvgLuminance);
					ImGui::Checkbox("Exposure", &renderExposure);
					
					ImGui::TreePop();
				}
				ImGui::TreePop();
			}
		});
		//Shadow Map
		operationConsole->AddFunction([this]() {
			ImGui::Checkbox("Use Shadow Map", &useShadow);
			if (ImGui::TreeNode("Shadow Parameters")) {
				ImGui::Checkbox("Use Variance", &useVariance);
				static int size = shadowMapSize.x;
				ImGui::SliderInt("size", &size, 256.0f, 4096);
				shadowMapSize.y = shadowMapSize.x = f_cast(size);
				static int tempCascadeCount = cascadeCount;
				ImGui::SliderInt("Cascade Count", &tempCascadeCount, 1, 8);
				static CascadeShadowBuffers::FORMAT format = CascadeShadowBuffers::FORMAT::BIT_32;
				ImGui::RadioButton("16Bit", r_cast<int*>(&format), i_cast(CascadeShadowBuffers::FORMAT::BIT_16));
				ImGui::SameLine();
				ImGui::RadioButton("32Bit", r_cast<int*>(&format), i_cast(CascadeShadowBuffers::FORMAT::BIT_32));
				if (ImGui::Button("Apply")) {
					cascadeCount = tempCascadeCount;
					shadowMap->ChangeState(cascadeCount, shadowMapSize, format, useVariance);
				}
				ImGui::SliderFloat("Near Z", &shadowNearZ, 1.0f, 999.0f);
				ImGui::SliderFloat("Far Z", &shadowFarZ, 2.0f, 1000.0f);
				ImGui::SliderFloat("Lamda", &shadowLamda, 0.0f, 1.0f);
				ImGui::TreePop();
			}
		});
		//SSAO
		operationConsole->AddFunction([this] {
			ImGui::Checkbox("Use SSAO", &useSSAO);
			if (useSSAO&&ImGui::TreeNode("SSAO Parameters")) {
				ImGui::SliderFloat("SSAO Threshold", &ssaoThreshold, 1.0f, 20.0f);
				ImGui::TreePop();
			}
		});
		//DoF
		operationConsole->AddFunction([this] {
			ImGui::Checkbox("Use DoF", &useDoF);
			if (useSSAO&&ImGui::TreeNode("DoF Parameters")) {
				static float quality = 0.5f;
				ImGui::SliderFloat("Quality", &quality, 0.1f, 1.0f);
				if (ImGui::Button("Apply")) dof->ChangeQuality(quality);
				ImGui::SliderFloat("Focus Range", &focusRange, 1.0f, 1000.0f);
				ImGui::TreePop();
			}
		});
		operationConsole->AddFunction([this] {
			ImGui::Checkbox("Use Bloom", &useBloom);
			if (useBloom) {
				ImGui::SliderFloat("Box Bloom Intensity", &boxInfo.emissionFactor, 0.0f, 20.0f);
			}
		});
		operationConsole->AddFunction([this] {
			ImGui::Checkbox("Use Tonemap", &useTonemap);
		});
	}
	SceneGBufferCompression::~SceneGBufferCompression() {
		GBufferDecodeRenderer::Finalize();
	}
	void SceneGBufferCompression::AlwaysUpdate() {
		if (cameraMove)camera->Update();
		gbuffer->AddModel(stage, stageInfo);
		gbuffer->AddModel(box, boxInfo);
		shadowMap->AddModel(stage);
		shadowMap->AddModel(box);
		//��]���C�g
		rad += Application::GetInstance()->GetProcessTimeSec()*0.1f;
		lpos = Math::Vector3(sinf(rad), 0.0f, cos(rad))*100.0f;
		//lpos.x = lpos.z = 0.0f;
		lpos.y = 80.0f;
		//�Œ胉�C�g
		//lpos = Math::Vector3(200.0f, 130.0f, 200.0f);
		shadowMap->SetPos(lpos);
		shadowMap->SetTarget(Math::Vector3());
		shadowMap->SetNear(shadowNearZ);
		shadowMap->SetFar(shadowFarZ);
		shadowMap->SetLamda(shadowLamda);
		shadowMap->SetEnable(useShadow);
		ssao->SetThresholdDepth(ssaoThreshold);
		ssao->SetEnable(useSSAO);
		dof->SetEnable(useDoF);
		dof->SetFocusRange(focusRange);
	}
	void SceneGBufferCompression::AlwaysRender() {
		//Graphics::Environment::GetInstance()->SetLightDirection(Math::Vector3(-1.0f, -1.0f, -1.0f));
		Graphics::Environment::GetInstance()->Activate();
		camera->Activate();
		gbuffer->RenderGBuffer(camera->GetView());
		shadowMap->RenderShadowMap(camera->GetView().get(), offScreen.get());
		//�[�x�`��
		depth->Clear(0x00000000);
		depth->Activate();
		GBufferDecodeRenderer::DepthRender(gbuffer, Math::Vector2(), depth->GetTexture()->GetSize());
		emission->Clear(0xFF000000);
		emission->Activate();
		GBufferDecodeRenderer::EmissionRender(gbuffer, Math::Vector2(), emission->GetTexture()->GetSize());
		offScreen->Clear(0x00000000);
		offScreen->Activate();
		ssao->Dispatch(offScreen.get(), camera->GetView().get(), depth.get());
		ssao->Begin(4);
		shadowMap->SetShadowMap(2, 3);
		camera->Activate();
		deferredShader->Render(camera->GetView(), gbuffer);
		ssao->End();
		Graphics::Texture::Clean(2, Graphics::ShaderStageList::PS);
		Graphics::Texture::Clean(3, Graphics::ShaderStageList::PS);
		//�o�b�N�o�b�t�@��L����
		Graphics::RenderTarget* rt = offScreenPost.get();
		offScreenPost->Clear(0x00000000);
		offScreenPost->Activate();
		if (useDoF) {
			dof->Dispatch(camera->GetView().get(), rt, offScreen.get(), depth.get());
			dof->Render(Math::Vector2(), rt->GetTexture()->GetSize());
		}
		else Graphics::SpriteRenderer::Render(offScreen.get());
		if (useBloom) {
			bloom->Dispatch(emission, camera->GetView().get(), rt);
			bloom->Render(Math::Vector2(0.0f, 0.0f), Application::GetInstance()->GetWindow()->GetSize());
		}
		rt = Application::GetInstance()->GetSwapChain()->GetRenderTarget();
		rt->Activate();
		if (useTonemap) {
			tonemap->Dispatch(camera->GetView().get(), rt, offScreenPost, 11);
			tonemap->Render(Math::Vector2(), Application::GetInstance()->GetWindow()->GetSize());
		}
		else Graphics::SpriteRenderer::Render(offScreenPost.get(), 0xFFFFFFFF);
		Graphics::Texture::Clean(0, Graphics::ShaderStageList::PS);
		//�f�o�b�O�`��
#ifdef _DEBUG
		if (Application::GetInstance()->debugRender)
#endif
		{
			operationConsole->Update();
			//G-Buffer�f�o�b�O�`��
			if (renderGBuffer) {
				if (renderColor) GBufferDecodeRenderer::ColorRender(gbuffer, Math::Vector2(0.0f, 0.0f), Math::Vector2(100.0f, 100.0f));
				if (renderDepth) Graphics::SpriteRenderer::Render(depth.get(), Math::Vector2(100.0f, 0.0f), Math::Vector2(100.0f, 100.0f), 0.0f, Math::Vector2(), depth->GetTexture()->GetSize(), 0xFFFFFFFF);
				//if (renderDepth) GBufferDecodeRenderer::DepthRender(gbuffer, Math::Vector2(100.0f, 0.0f), Math::Vector2(100.0f, 100.0f));
				if (renderWPos) GBufferDecodeRenderer::WorldPosRender(gbuffer, Math::Vector2(200.0f, 0.0f), Math::Vector2(100.0f, 100.0f));
				if (renderNormal) GBufferDecodeRenderer::NormalRender(gbuffer, Math::Vector2(300.0f, 0.0f), Math::Vector2(100.0f, 100.0f));
				if (renderLightingIntensity) GBufferDecodeRenderer::LightingIntensityRender(gbuffer, Math::Vector2(400.0f, 0.0f), Math::Vector2(100.0f, 100.0f));
				if (renderSpecularIntensity) GBufferDecodeRenderer::SpecularIntensityRender(gbuffer, Math::Vector2(500.0f, 0.0f), Math::Vector2(100.0f, 100.0f));
				//if (renderEmissionColor) GBufferDecodeRenderer::EmissionRender(gbuffer, Math::Vector2(600.0f, 0.0f), Math::Vector2(100.0f, 100.0f));
				if (renderEmissionColor) Graphics::SpriteRenderer::Render(emission.get(), Math::Vector2(600.0f, 0.0f), Math::Vector2(100.0f, 100.0f), 0.0f, Math::Vector2(), depth->GetTexture()->GetSize(), 0xFFFFFFFF);
				if (renderSSAO) ssao->Render(Math::Vector2(700.0f, 0.0f), Math::Vector2(100.0f, 100.0f));
				if (renderShadingBuffer)Graphics::SpriteRenderer::Render(offScreen.get(), Math::Vector2(800.0f, 0.0f), Math::Vector2(100.0f, 100.0f), 0.0f, Math::Vector2(), offScreen->GetTexture()->GetSize(), 0xFFFFFFFF);
				if (renderDoFBuffer) {
					Graphics::SpriteRenderer::Render(dof->GetStep0().get(), Math::Vector2(0.0f, 200.0f), Math::Vector2(100.0f, 100.0f), 0.0f, Math::Vector2(), dof->GetStep0()->GetTexture()->GetSize(), 0xFFFFFFFFF);
					Graphics::SpriteRenderer::Render(dof->GetStep1().get(), Math::Vector2(100.0f, 200.0f), Math::Vector2(100.0f, 100.0f), 0.0f, Math::Vector2(), dof->GetStep1()->GetTexture()->GetSize(), 0xFFFFFFFFF);
				}
				//Shadow Map�f�o�b�O�`��
				if (useShadow&&renderShadowMap) {
					for (int i = 0; i < cascadeCount; i++) {
						shadowMap->DebugShadowMapRender(i, Math::Vector2(100.0f*i, 100.0f), Math::Vector2(100.0f, 100.0f));
					}
					shadowMap->DebugDataTextureRender(Math::Vector2(100.0f*cascadeCount, 100.0f), Math::Vector2(100.0f, 100.0f));
				}
				if (useBloom&&renderBloomBuffer) {
					for (int i = 0; i < 4; i++) {
						std::shared_ptr<Graphics::RenderTarget>& rt = bloom->GetGaussianRenderTarget(i);
						Graphics::SpriteRenderer::Render(rt.get(), Math::Vector2(100.0f*i, 300.0f), Math::Vector2(100.0f, 100.0f), 0.0f, Math::Vector2(), rt->GetTexture()->GetSize(), 0xFFFFFFFFF);
					}
					bloom->Render(Math::Vector2(400.0f, 300.0f), Math::Vector2(100.0f, 100.0f));
				}
				if (useTonemap) {
					if (renderTonemapBuffer)tonemap->Render(Math::Vector2(0.0f, 400.0f), Math::Vector2(100.0f, 100.0f));
					if (renderAvgLuminance) Graphics::SpriteRenderer::Render(tonemap->AvgLuminanceTexture(), Math::Vector2(100.0f, 400.0f), Math::Vector2(100.0f, 100.0f), 0.0f, Math::Vector2(), tonemap->AvgLuminanceTexture()->GetSize(), 0xFFFFFFFF);
					if (renderExposure)tonemap->DebugRender(rt, Math::Vector2(200.0f, 400.0f), Math::Vector2(100.0f, 100.0f));
				}
			}
			Graphics::Texture::Clean(0, Graphics::ShaderStageList::PS);
		}
	}
}