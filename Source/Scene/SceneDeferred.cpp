#include "Lobelia.hpp"
#include "SceneDeferred.hpp"

//Screen Space Motion Blur����Ă݂���
//��ʊE�[�x����Ă݂���

//�V�F�[�_�[�̃R���p�C������������A�f�o�b�O���ȊO��cso�ɓ������Ă�肽����
//�X�L�����b�V���̃L������������āA�����蔻��(Raypick)��GPGPU�łƂ�̂��ʔ����Ǝv��
//��������̂�Actor�N���X�g���΂���

//TODO : SSAOPS�̉ω𑜓x�Ή�(�r���[������Ă�邾���H)
//TODO : �V�F�[�_�[�̐���
//TODO : �x������

//�e�X�g�V�[��
//�ŏI�I�ɂ����ł̌o���ƌ��ʂ�p���ă����_�����O�G���W�������\��ł͂��邪�A�A���I�������ɂȂ�Ǝv����B

//�����������Ă���̈ꗗ(���̃V�[���̓f�B�t�@�[�h�V�F�[�f�B���O�ł�)
//Define.h�̃X�C�b�`�ňꕔ�̋@�\�̃X�C�b�`���\
//�n�[�t�����o�[�g
//���`�t�H�O
//�@���}�b�v
//�����|�C���g���C�g
//SSAOPS �x��
//SSAOCS PS��葁��
//�K�E�X�t�B���^PS
//�K�E�X�t�B���^CS ����x�� �œK���s��
//�V���h�E�}�b�v
//�o���A���X�V���h�E�}�b�v
//�J�X�P�[�h�V���h�E�}�b�v
//�J�X�P�[�h�o���A���X�V���h�E�}�b�v(�J�X�P�[�h�V���h�E�}�b�v�ƁA�o���A���X�V���h�E�}�b�v�̍��킹�Z)

namespace Lobelia::Game {
	namespace {
		const constexpr int LIGHT_COUNT = 127;
	}
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
	//---------------------------------------------------------------------------------------------
	//
	//	Geometry Buffer
	//
	//---------------------------------------------------------------------------------------------
	DeferredBuffer::DeferredBuffer(const Math::Vector2& size) :size(size) {
		//for (int i = 0; i < i_cast(BUFFER_TYPE::MAX); i++) {
		//	rts[i] = std::make_shared<Graphics::RenderTarget>(size, DXGI_SAMPLE_DESC{ 1,0 }, DXGI_FORMAT_R16G16B16A16_FLOAT);
		//}
		rts[i_cast(BUFFER_TYPE::POS)] = std::make_shared<Graphics::RenderTarget>(size, DXGI_SAMPLE_DESC{ 1,0 }, DXGI_FORMAT_R16G16B16A16_FLOAT);
		//����ɑ��x�d���Ȃ�A���x�͂�΂������������DXGI_FORMAT_R11G11B10_FLOAT
		rts[i_cast(BUFFER_TYPE::NORMAL)] = std::make_shared<Graphics::RenderTarget>(size, DXGI_SAMPLE_DESC{ 1,0 }, DXGI_FORMAT_R16G16B16A16_FLOAT);
		rts[i_cast(BUFFER_TYPE::COLOR)] = std::make_shared<Graphics::RenderTarget>(size, DXGI_SAMPLE_DESC{ 1,0 }, DXGI_FORMAT_R8G8B8A8_SNORM);
		rts[i_cast(BUFFER_TYPE::VIEW_POS)] = std::make_shared<Graphics::RenderTarget>(size, DXGI_SAMPLE_DESC{ 1,0 }, DXGI_FORMAT_R16G16B16A16_FLOAT);
		rts[i_cast(BUFFER_TYPE::SHADOW)] = std::make_shared<Graphics::RenderTarget>(size, DXGI_SAMPLE_DESC{ 1,0 }, DXGI_FORMAT_R16_FLOAT);
		//VertexShader(const char* file_path, const char* entry_point, Model shader_model, bool use_linkage = false);
		vs = std::make_shared<Graphics::VertexShader>("Data/ShaderFile/3D/deferred.hlsl", "CreateGBufferVS", Graphics::VertexShader::Model::VS_4_0);
		ps = std::make_shared<Graphics::PixelShader>("Data/ShaderFile/3D/deferred.hlsl", "CreateGBufferPS", Graphics::PixelShader::Model::PS_4_0);
		cbuffer = std::make_unique<Graphics::ConstantBuffer<Info>>(8, Graphics::ShaderStageList::VS | Graphics::ShaderStageList::PS);
	}
	std::shared_ptr<Graphics::RenderTarget> DeferredBuffer::GetRenderTarget(BUFFER_TYPE type) { return rts[i_cast(type)]; }
	void DeferredBuffer::AddModel(std::shared_ptr<Graphics::Model> model, bool use_normal_map) {
		models.push_back(ModelStorage{ model,i_cast(use_normal_map),FALSE });
	}
	void DeferredBuffer::RenderGBuffer() {
		End();
		for (int i = 0; i < i_cast(BUFFER_TYPE::MAX); i++) {
			rts[i]->Clear(0x00000000);
		}
		Graphics::RenderTarget::Activate(rts[0].get(), rts[1].get(), rts[2].get(), rts[3].get(), rts[4].get());
		auto& defaultVS = Graphics::Model::GetVertexShader();
		auto& defaultPS = Graphics::Model::GetPixelShader();
		Graphics::Model::ChangeVertexShader(vs);
		Graphics::Model::ChangePixelShader(ps);
		for (auto&& weak : models) {
			if (weak.model.expired())continue;
			std::shared_ptr<Graphics::Model> model = weak.model.lock();
			cbuffer->Activate(weak.info);
			model->Render();
		}
		Graphics::Model::ChangeVertexShader(defaultVS);
		Graphics::Model::ChangePixelShader(defaultPS);
		models.clear();
	}
	void DeferredBuffer::Begin() {
		for (int i = 0; i < i_cast(BUFFER_TYPE::MAX); i++) {
			rts[i]->GetTexture()->Set(i, Graphics::ShaderStageList::PS);
		}
	}
	void DeferredBuffer::End() {
		for (int i = 0; i < i_cast(BUFFER_TYPE::MAX); i++) {
			Graphics::Texture::Clean(i, Graphics::ShaderStageList::PS);
		}
	}
	void DeferredBuffer::DebugRender() {
		for (int i = 0; i < i_cast(BUFFER_TYPE::MAX); i++) {
			Graphics::SpriteRenderer::Render(rts[i].get(), Math::Vector2(i*200.0f, 0.0f), Math::Vector2(200.0f, 200.0f), 0.0f, Math::Vector2(), size, 0xFFFFFFFF);
		}
	}

	DeferredShader::DeferredShader(const char* file_path, const char* entry_vs, const char* entry_ps) {
		if (std::string(entry_vs) != "")vs = std::make_shared<Graphics::VertexShader>(file_path, entry_vs, Graphics::VertexShader::Model::VS_5_0);
		ps = std::make_shared<Graphics::PixelShader>(file_path, entry_ps, Graphics::PixelShader::Model::PS_5_0);
	}
	void DeferredShader::Render() {
		auto& defaultVS = Graphics::SpriteRenderer::GetVertexShader();
		auto& defaultPS = Graphics::SpriteRenderer::GetPixelShader();
		if (vs)Graphics::SpriteRenderer::ChangeVertexShader(vs);
		Graphics::SpriteRenderer::ChangePixelShader(ps);
		Graphics::SpriteRenderer::Render(static_cast<Graphics::Texture*>(nullptr));
		Graphics::SpriteRenderer::ChangeVertexShader(defaultVS);
		Graphics::SpriteRenderer::ChangePixelShader(defaultPS);
	}

	SimpleDeferred::SimpleDeferred() :DeferredShader("Data/ShaderFile/3D/deferred.hlsl", "", "SimpleDeferredPS") {
	}
	PointLightDeferred::PointLightDeferred() : DeferredShader("Data/ShaderFile/3D/deferred.hlsl", "", "FullDeferredPS") {
		cbuffer = std::make_unique<Graphics::ConstantBuffer<PointLights>>(6, Graphics::ShaderStageList::PS);
	}
	void PointLightDeferred::SetLightBuffer(int index, const PointLight& p_light) {
		lights.pos[index] = p_light.pos;
		lights.color[index] = p_light.color;
		lights.attenuation[index].x = p_light.attenuation;
	}
	void PointLightDeferred::SetUseCount(int use_count) { lights.usedLightCount = use_count; }
	void PointLightDeferred::Update() {
		cbuffer->Activate(lights);
	}
	//---------------------------------------------------------------------------------------------
	ShadowBuffer::ShadowBuffer(const Math::Vector2& size, int split_count, bool use_variance) :size(size), count(split_count) {
		rts.resize(split_count); views.resize(split_count);
		//��������ۂ�near/far�����A+1�̗��R�́A�ŏ���near����n�܂��Ď���far�������L�^���Ă�����
		//�ŏ���far������near�Ƃ��ċ��L����Ȃ��̂ŁA���̕���+1
		cascadeValues.resize(split_count + 1);
		splitPositions.resize(split_count);
		gaussian.resize(split_count);
		DXGI_FORMAT format = DXGI_FORMAT_R16G16_FLOAT;
		Math::Vector2 ssize = size;
		for (int i = 0; i < split_count; i++) {
			rts[i] = std::make_shared<Graphics::RenderTarget>(size, DXGI_SAMPLE_DESC{ 1,0 }, format);
			views[i] = std::make_unique<Graphics::View>(Math::Vector2(), size, PI / 4.0f, 50, 400.0f);
#ifdef GAUSSIAN_CS
			gaussian[i] = std::make_unique<GaussianFilterCS>(size, format);
#endif
#ifdef GAUSSIAN_PS
			gaussian[i] = std::make_unique<GaussianFilterPS>(size, format);
#endif
			gaussian[i]->SetDispersion(0.01f);
		}
		//sampler = std::make_unique<Graphics::SamplerState>(Graphics::SAMPLER_PRESET::COMPARISON_LINEAR, 16);
		vs = std::make_shared<Graphics::VertexShader>("Data/ShaderFile/3D/deferred.hlsl", "CreateShadowMapVS", Graphics::VertexShader::Model::VS_5_0, false);
		ps = std::make_shared<Graphics::PixelShader>("Data/ShaderFile/3D/deferred.hlsl", "CreateShadowMapPS", Graphics::PixelShader::Model::PS_5_0, false);
		cbuffer = std::make_unique<Graphics::ConstantBuffer<Info>>(10, Graphics::ShaderStageList::VS | Graphics::ShaderStageList::PS);
		info.useShadowMap = TRUE; info.useVariance = i_cast(use_variance);
		//�k���o�b�t�@�ɂ��ď����҂��̂���������
#ifdef _DEBUG
		HostConsole::GetInstance()->IntRegister("deferred", "use shadow", &info.useShadowMap, false);
		HostConsole::GetInstance()->IntRegister("deferred", "use variance", &info.useVariance, false);
#endif
	}
	void ShadowBuffer::SetPos(const Math::Vector3& pos) { this->pos = pos; }
	void ShadowBuffer::SetTarget(const Math::Vector3& at) { this->at = at; }
	//������Ƒ��v���͕s���A�_�������Ȃ璲�����܂�
	void ShadowBuffer::ComputeSplit(float lamda, float near_z, float far_z) {
#ifdef CASCADE
		//�ʏ�̃V���h�E�}�b�v
		if (count == 1) {
			cascadeValues[0] = near_z;
			cascadeValues[1] = far_z;
			return;
		}
		//�J�X�P�[�h
		float invM = 1.0f / f_cast(count);
		float farDivisionNear = far_z / near_z;
		float farSubNear = far_z - near_z;
		//���p�����X�L�[����K�p
		// �� GPU Gems 3, Chapter 10. Parallel-Split Shadow Maps on Programmable GPUs.
		//    http://http.developer.nvidia.com/GPUGems3/gpugems3_ch10.html ���Q��.
		for (int i = 1; i < count + 1; i++) {
			//�ΐ������X�L�[��
			float log = near_z * powf(farDivisionNear, invM*i);
			//��l�����X�L�[��
			float uni = near_z + farSubNear * i*invM;
			//��L��2�̉��Z���ʂ���`��Ԃ���
			cascadeValues[i] = lamda * log + uni * (1.0f - lamda);
		}
		cascadeValues[0] = near_z;
		cascadeValues[count] = far_z;
		for (int i = 1; i < count + 1; i++) {
			info.splitPos[i - 1] = cascadeValues[i];
		}
#endif
	}
	void ShadowBuffer::CameraUpdate() {
		Math::Vector3 front = at - pos; front.Normalize();
		up = Math::Vector3(0.01f, 1.0f, 0.01f); up.Normalize();
		Math::Vector3 right = Math::Vector3::Cross(up, front); right.Normalize();
		up = Math::Vector3::Cross(front, right);
#ifdef CASCADE
		info.pos.x = pos.x; info.pos.y = pos.y; info.pos.z = pos.z; info.pos.w = 1.0f;
		info.front.x = front.x; info.front.y = front.y; info.front.z = front.z; info.front.w = 0.0f;
		float nearZ = 1.0f;
		float farZ = 1000.0f;
		ComputeSplit(0.5f, nearZ, farZ);
#endif
		for (int i = 0; i < count; i++) {
			views[i]->SetEyePos(pos);
			views[i]->SetEyeTarget(at);
			views[i]->SetEyeUpDirection(up);
#ifdef CASCADE
			views[i]->SetNear(cascadeValues[i]);
			views[i]->SetFar(cascadeValues[count]);
#endif
		}
	}
	void ShadowBuffer::AddModel(std::shared_ptr<Graphics::Model> model) { models.push_back(model); }
	void ShadowBuffer::CreateShadowMap(Graphics::View* active_view, Graphics::RenderTarget* active_rt) {
#ifdef _DEBUG
		if (Input::GetKeyboardKey(DIK_0) == 1)info.useShadowMap = !info.useShadowMap;
		if (Input::GetKeyboardKey(DIK_9) == 1)info.useVariance = !info.useVariance;
#endif
		if (!info.useShadowMap) {
			models.clear();
			return;
		}
		CameraUpdate();
		auto& defaultVS = Graphics::Model::GetVertexShader();
		auto& defaultPS = Graphics::Model::GetPixelShader();
		Graphics::Model::ChangeVertexShader(vs);
		Graphics::Model::ChangePixelShader(ps);
		for (int i = 0; i < count; i++) {
			views[i]->Activate();
			rts[i]->Clear(0x00000000);
			rts[i]->Activate();
			for (auto&& weak : models) {
				if (weak.expired())continue;
				auto model = weak.lock();
				model->Render();
			}
		}
		models.clear();
		Graphics::Model::ChangeVertexShader(defaultVS);
		Graphics::Model::ChangePixelShader(defaultPS);
		active_view->Activate();
		active_rt->Activate();
		//�K�E�X�ɂ��ڂ��� �o���A���X�p
		if (info.useVariance) {
			for (int i = 0; i < count; i++) {
				gaussian[i]->Dispatch(active_view, active_rt, rts[i]->GetTexture());
			}
		}
	}
	void ShadowBuffer::Begin() {
		//���̍X�V
		DirectX::XMStoreFloat4x4(&info.view, views[0]->GetColumnViewMatrix());
		for (int i = 0; i < count; i++) {
			DirectX::XMStoreFloat4x4(&info.proj[i], views[i]->GetColumnProjectionMatrix());
			if (info.useVariance)gaussian[i]->Begin(6 + i);
			else rts[i]->GetTexture()->Set(6 + i, Graphics::ShaderStageList::PS);
		}
		cbuffer->Activate(info);
	}
	void ShadowBuffer::End() {
		if (info.useVariance) {
			for (int i = 0; i < count; i++) {
				gaussian[i]->End();
			}
		}
		else {
			for (int i = 0; i < count; i++) {
				Graphics::Texture::Clean(6 + i, Graphics::ShaderStageList::PS);
			}
		}
	}
	void ShadowBuffer::DebugRender() {
		for (int i = 0; i < count; i++) {
			Graphics::SpriteRenderer::Render(rts[i].get(), Math::Vector2(i*200.0f, 200.0f), Math::Vector2(200.0f, 200.0f), 0.0f, Math::Vector2(), size, 0xFFFFFFFF);
			gaussian[i]->DebugRender(Math::Vector2(i*200.0f, 400.0f), Math::Vector2(200.0f, 200.0f));
		}
	}
	//---------------------------------------------------------------------------------------------
	//
	//	Post Effect
	//
	//---------------------------------------------------------------------------------------------
	PostEffect::PostEffect(const Math::Vector2& size, bool create_rt, DXGI_FORMAT format) :size(size) {
		if (create_rt)rt = std::make_shared<Graphics::RenderTarget>(size, DXGI_SAMPLE_DESC{ 1,0 }, format);
	}
	std::shared_ptr<Graphics::RenderTarget>& PostEffect::GetRenderTarget() {
		if (!rt)STRICT_THROW("�쐬����Ă��Ȃ��o�b�t�@�ł�");
		return rt;
	}
	void PostEffect::Render() { Graphics::SpriteRenderer::Render(rt.get()); }
	void PostEffect::Begin(int slot) { this->slot = slot; }
	//��{�I�Ƀ|�X�g�G�t�F�N�g�̓s�N�Z���V�F�[�_�[�ł����g���Ȃ�
	void PostEffect::End() { Graphics::Texture::Clean(slot, Graphics::ShaderStageList::PS); }
	//---------------------------------------------------------------------------------------------
	StructuredBuffer::StructuredBuffer(int struct_size, int count) :STRUCT_SIZE(struct_size), COUNT(count) {
		D3D11_BUFFER_DESC desc = {};
		desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		desc.ByteWidth = struct_size * count;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.StructureByteStride = struct_size;
		//D3D11_SUBRESOURCE_DATA initData = {};
		//initData.pSysMem = 
		Graphics::Device::Get()->CreateBuffer(&desc, nullptr, buffer.GetAddressOf());
		//SRV�i�V�F�[�_�[���\�[�X�r���[�j�쐬
		D3D11_SHADER_RESOURCE_VIEW_DESC srdesc = {};
		srdesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		srdesc.Format = DXGI_FORMAT_UNKNOWN;
		srdesc.BufferEx.NumElements = count;
		HRESULT hr = Graphics::Device::Get()->CreateShaderResourceView(buffer.Get(), &srdesc, srv.GetAddressOf());
		if (FAILED(hr))STRICT_THROW("SRV�쐬�Ɏ��s");
	}
	void StructuredBuffer::Update(const void* resource) { Graphics::Device::GetContext()->UpdateSubresource(buffer.Get(), 0, nullptr, resource, 0, 0); }
	void StructuredBuffer::Set(int slot, Graphics::ShaderStageList stage) {
		switch (stage) {
		case Graphics::ShaderStageList::VS:	Graphics::Device::GetContext()->VSSetShaderResources(slot, 1, srv.GetAddressOf());	break;
		case Graphics::ShaderStageList::PS:	Graphics::Device::GetContext()->PSSetShaderResources(slot, 1, srv.GetAddressOf());	break;
		case Graphics::ShaderStageList::HS:	Graphics::Device::GetContext()->HSSetShaderResources(slot, 1, srv.GetAddressOf());	break;
		case Graphics::ShaderStageList::GS:	Graphics::Device::GetContext()->GSSetShaderResources(slot, 1, srv.GetAddressOf());	break;
		case Graphics::ShaderStageList::DS:	Graphics::Device::GetContext()->DSSetShaderResources(slot, 1, srv.GetAddressOf());	break;
		case Graphics::ShaderStageList::CS:	Graphics::Device::GetContext()->CSSetShaderResources(slot, 1, srv.GetAddressOf());	break;
		default:	STRICT_THROW("�͈͊O�̒l�ł�");
		}

	}
	int StructuredBuffer::GetStructSize() { return STRUCT_SIZE; }
	int StructuredBuffer::GetCount() { return COUNT; }
	int StructuredBuffer::GetBufferSize() { return STRUCT_SIZE * COUNT; }
	//---------------------------------------------------------------------------------------------
	UnorderedAccessView::UnorderedAccessView(Graphics::Texture* texture) {
		D3D11_UNORDERED_ACCESS_VIEW_DESC desc = {};
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = 0;
		HRESULT hr = Graphics::Device::Get()->CreateUnorderedAccessView(texture->Get().Get(), &desc, uav.GetAddressOf());
		if (FAILED(hr))STRICT_THROW("UAV�̍쐬�Ɏ��s");
	}
	UnorderedAccessView::UnorderedAccessView(StructuredBuffer* structured_buffer) {
		D3D11_UNORDERED_ACCESS_VIEW_DESC desc = {};
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.Buffer.NumElements = structured_buffer->GetCount();
		desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		desc.Texture2D.MipSlice = 0;
		HRESULT hr = Graphics::Device::Get()->CreateUnorderedAccessView(structured_buffer->buffer.Get(), &desc, uav.GetAddressOf());
		if (FAILED(hr))STRICT_THROW("UAV�̍쐬�Ɏ��s");
	}
	void UnorderedAccessView::Set(int slot) {
		Graphics::Device::GetContext()->CSSetUnorderedAccessViews(slot, 1, uav.GetAddressOf(), nullptr);
	}
	void UnorderedAccessView::Clean(int slot) {
		ID3D11UnorderedAccessView* null = nullptr;
		Graphics::Device::GetContext()->CSSetUnorderedAccessViews(slot, 1, &null, nullptr);
	}
	//---------------------------------------------------------------------------------------------
	ReadGPUBuffer::ReadGPUBuffer(std::shared_ptr<StructuredBuffer> buffer) :origin(buffer) {
		if (!buffer)STRICT_THROW("�I���W�i���̃o�b�t�@�����݂��܂���");
		D3D11_BUFFER_DESC desc = {};
		buffer->buffer->GetDesc(&desc);
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		desc.Usage = D3D11_USAGE_STAGING;
		desc.BindFlags = 0; desc.MiscFlags = 0;
		HRESULT hr = Graphics::Device::Get()->CreateBuffer(&desc, nullptr, this->buffer.GetAddressOf());
		if (FAILED(hr))STRICT_THROW("STAGING�o�b�t�@�쐬�Ɏ��s");
	}
	void ReadGPUBuffer::ReadCopy() {
		if (origin.expired())STRICT_THROW("�I���W�i���̃o�b�t�@�����݂��܂���");
		std::shared_ptr<StructuredBuffer> src = origin.lock();
		Graphics::Device::GetContext()->CopyResource(buffer.Get(), src->buffer.Get());
	}
	void* ReadGPUBuffer::ReadBegin() {
		D3D11_MAPPED_SUBRESOURCE resource = {};
		HRESULT hr = Graphics::Device::GetContext()->Map(buffer.Get(), 0, D3D11_MAP_READ, 0, &resource);
		if (FAILED(hr))STRICT_THROW("�}�b�v�Ɏ��s");
		return resource.pData;
	}
	void ReadGPUBuffer::ReadEnd() { Graphics::Device::GetContext()->Unmap(buffer.Get(), 0); }
	//---------------------------------------------------------------------------------------------
	//DXGI_FORMAT_R16G16B16A16_FLOAT�ł���Ƃ�����ăo�O��B���R�s���B
	SSAOCS::SSAOCS(const Math::Vector2& size) :PostEffect(size, true, /*DXGI_FORMAT_R16G16B16A16_FLOAT*/DXGI_FORMAT_R32G32B32A32_FLOAT) {
		cs = std::make_unique<Graphics::ComputeShader>("Data/ShaderFile/2D/PostEffect.hlsl", "SSAOCS");
		//�𑜓x���킹��悤
		view = std::make_unique<Graphics::View>(Math::Vector2(), size);
		rwTexture = std::make_shared<Graphics::Texture>(size, DXGI_FORMAT_R16_FLOAT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS, DXGI_SAMPLE_DESC{ 1,0 });
		uav = std::make_unique<UnorderedAccessView>(rwTexture.get());
		cbuffer = std::make_unique<Graphics::ConstantBuffer<Info>>(7, Graphics::ShaderStageList::CS | Graphics::ShaderStageList::PS);
		info.offsetPerPixel = 5.0f;
		info.useAO = TRUE;
		//PS�p �g��Ȃ��p�����[�^�[
		info.offsetPerPixelX = 1.0f; info.offsetPerPixelY = 1.0f;
#ifdef _DEBUG
		HostConsole::GetInstance()->IntRegister("deferred", "use AO", &info.useAO, false);
		HostConsole::GetInstance()->FloatRegister("deferred", "use offset per pixel", &info.offsetPerPixel, false);
#endif
	}
	void SSAOCS::CreateAO(Graphics::RenderTarget* active_rt, Graphics::View* active_view, DeferredBuffer* deferred_buffer) {
		//int slot, ID3D11ShaderResourceView* uav
#ifdef _DEBUG
		if (Input::GetKeyboardKey(DIK_8) == 1) info.useAO = !info.useAO;
#endif
		cbuffer->Activate(info);
		if (!info.useAO)return;
		uav->Set(0);
		Graphics::RenderTarget* viewRT = deferred_buffer->GetRenderTarget(DeferredBuffer::BUFFER_TYPE::VIEW_POS).get();
		//�o�b�t�@�̃R�s�[
		if (size != viewRT->GetTexture()->GetSize()) {
			rt->Clear(0x00000000);
			rt->Activate();
			view->ViewportActivate();
			Graphics::SpriteRenderer::Render(viewRT);
			active_rt->Activate();
			active_view->Activate();
			rt->GetTexture()->Set(0, Graphics::ShaderStageList::CS);
		}
		else viewRT->GetTexture()->Set(0, Graphics::ShaderStageList::CS);
		cs->Dispatch(i_cast(size.x + SSAO_BLOCK_SIZE - 1) / SSAO_BLOCK_SIZE, i_cast(size.y + SSAO_BLOCK_SIZE - 1) / SSAO_BLOCK_SIZE, 1);
		uav->Clean(0);
		Graphics::Texture::Clean(0, Graphics::ShaderStageList::CS);
	}
	void SSAOCS::Render() {
		Graphics::SpriteRenderer::Render(rwTexture.get(), Math::Vector2(5 * 200.0f, 0.0f), Math::Vector2(200.0f, 200.0f), 0.0f, Math::Vector2(), rt->GetTexture()->GetSize(), 0xFFFFFFFF);
		Graphics::Texture::Clean(0, Graphics::ShaderStageList::PS);
	}
	void SSAOCS::Begin(int slot) {
		PostEffect::Begin(slot);
		rwTexture->Set(this->slot, Graphics::ShaderStageList::PS);
	}
	SSAOPS::SSAOPS(const Math::Vector2& size) :PostEffect(size, true, DXGI_FORMAT_R16_FLOAT) {
		//SSAO�������ݑΏ�
		ps = std::make_unique<Graphics::PixelShader>("Data/ShaderFile/2D/PostEffect.hlsl", "SSAOPS", Graphics::PixelShader::Model::PS_5_0, false);
		cbuffer = std::make_unique<Graphics::ConstantBuffer<Info>>(7, Graphics::ShaderStageList::PS);
		info.offsetPerPixel = 20.0f;
		info.useAO = TRUE;
		info.offsetPerPixelX = 1.0f / size.x;
		info.offsetPerPixelY = 1.0f / size.y;
#ifdef _DEBUG
		HostConsole::GetInstance()->IntRegister("deferred", "use AO", &info.useAO, false);
		HostConsole::GetInstance()->FloatRegister("deferred", "use offset per pixel", &info.offsetPerPixel, false);
#endif
	}
	void SSAOPS::CreateAO(Graphics::RenderTarget* active_rt, DeferredBuffer* deferred_buffer) {
#ifdef _DEBUG
		if (Input::GetKeyboardKey(DIK_8) == 1) info.useAO = !info.useAO;
#endif
		cbuffer->Activate(info);
		if (!info.useAO)return;
		rt->Activate();
		auto& defaultPS = Graphics::SpriteRenderer::GetPixelShader();
		Graphics::SpriteRenderer::ChangePixelShader(ps);
		Graphics::SpriteRenderer::Render(deferred_buffer->GetRenderTarget(DeferredBuffer::BUFFER_TYPE::VIEW_POS)->GetTexture());
		Graphics::SpriteRenderer::ChangePixelShader(defaultPS);
		active_rt->Activate();
	}
	void SSAOPS::Render() {
		Graphics::SpriteRenderer::Render(rt.get(), Math::Vector2(5 * 200.0f, 0.0f), Math::Vector2(200.0f, 200.0f), 0.0f, Math::Vector2(), rt->GetTexture()->GetSize(), 0xFFFFFFFF);
		Graphics::Texture::Clean(0, Graphics::ShaderStageList::PS);
	}
	void SSAOPS::Begin(int slot) {
		PostEffect::Begin(slot);
		rt->GetTexture()->Set(this->slot, Graphics::ShaderStageList::PS);
	}

	//---------------------------------------------------------------------------------------------
	GaussianFilterCS::GaussianFilterCS(const Math::Vector2& size, DXGI_FORMAT format) :PostEffect(size, true) {
		csX = std::make_unique<Graphics::ComputeShader>("Data/ShaderFile/2D/PostEffect.hlsl", "GaussianFilterCSX");
		csY = std::make_unique<Graphics::ComputeShader>("Data/ShaderFile/2D/PostEffect.hlsl", "GaussianFilterCSY");
		rwTexturePass1 = std::make_shared<Graphics::Texture>(size, format, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS, DXGI_SAMPLE_DESC{ 1,0 });
		rwTexturePass2 = std::make_shared<Graphics::Texture>(size, format, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS, DXGI_SAMPLE_DESC{ 1,0 });
		uavPass1 = std::make_unique<UnorderedAccessView>(rwTexturePass1.get());
		uavPass2 = std::make_unique<UnorderedAccessView>(rwTexturePass2.get());
		cbuffer = std::make_unique<Graphics::ConstantBuffer<Info>>(9, Graphics::ShaderStageList::CS);
		view = std::make_unique<Graphics::View>(Math::Vector2(), size);
		dispersion = 0.03f;
	}
	void GaussianFilterCS::SetDispersion(float dispersion) { this->dispersion = dispersion; }
	void GaussianFilterCS::Dispatch(Graphics::View* active_view, Graphics::RenderTarget* active_rt, Graphics::Texture* texture) {
		if (size != texture->GetSize()) {
			//�𑜓x���킹��A��{�_�E���T���v�����O�����
			this->rt->Clear(0x00000000);
			this->rt->Activate();
			view->ViewportActivate();
			Graphics::SpriteRenderer::Render(texture);
			active_rt->Activate();
			active_view->ViewportActivate();
			this->rt->GetTexture()->Set(0, Graphics::ShaderStageList::CS);
		}
		else texture->Set(0, Graphics::ShaderStageList::CS);
		static constexpr const UINT DIVISION = 7;
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
		cbuffer->Activate(info);
		uavPass1->Set(0);
		csX->Dispatch(i_cast((size.x + GAUSSIAN_BLOCK - 1) / GAUSSIAN_BLOCK), i_cast((size.y + GAUSSIAN_BLOCK - 1) / GAUSSIAN_BLOCK), 1);
		uavPass1->Clean(0);
		uavPass2->Set(0);
		rwTexturePass1->Set(0, Graphics::ShaderStageList::CS);
		csY->Dispatch(i_cast((size.x + GAUSSIAN_BLOCK - 1) / GAUSSIAN_BLOCK), i_cast((size.y + GAUSSIAN_BLOCK - 1) / GAUSSIAN_BLOCK), 1);
		uavPass2->Clean(0);
		Graphics::Texture::Clean(0, Graphics::ShaderStageList::CS);
	}
	void GaussianFilterCS::Dispatch(Graphics::View* active_view, Graphics::RenderTarget* active_rt, std::shared_ptr<Graphics::RenderTarget> rt) { Dispatch(active_view, active_rt, rt->GetTexture()); }
	void GaussianFilterCS::Render() {
		Graphics::SpriteRenderer::Render(rwTexturePass2.get());
		Graphics::Texture::Clean(0, Graphics::ShaderStageList::PS);
	}
	void GaussianFilterCS::Begin(int slot) {
		PostEffect::Begin(slot);
		rwTexturePass2->Set(this->slot, Graphics::ShaderStageList::PS);
	}
	void GaussianFilterCS::DebugRender(const Math::Vector2& pos, const Math::Vector2& size) {
		Graphics::SpriteRenderer::Render(rwTexturePass2.get(), pos, size, 0.0f, Math::Vector2(), rwTexturePass2->GetSize(), 0xFFFFFFFF);
		Graphics::Texture::Clean(0, Graphics::ShaderStageList::PS);
	}
	GaussianFilterPS::GaussianFilterPS(const Math::Vector2& size, DXGI_FORMAT format) :PostEffect(size, true, format) {
		view = std::make_unique<Graphics::View>(Math::Vector2(), size);
		vsX = std::make_shared<Graphics::VertexShader>("Data/ShaderFile/2D/PostEffect.hlsl", "GaussianFilterVSX", Graphics::VertexShader::Model::VS_5_0, false);
		vsY = std::make_shared<Graphics::VertexShader>("Data/ShaderFile/2D/PostEffect.hlsl", "GaussianFilterVSY", Graphics::VertexShader::Model::VS_5_0, false);
		ps = std::make_shared<Graphics::PixelShader>("Data/ShaderFile/2D/PostEffect.hlsl", "GaussianFilterPS", Graphics::PixelShader::Model::PS_5_0, false);
		pass2 = std::make_unique<Graphics::RenderTarget>(size, DXGI_SAMPLE_DESC{ 1,0 }, format);
		cbuffer = std::make_unique<Graphics::ConstantBuffer<Info>>(9, Graphics::ShaderStageList::VS | Graphics::ShaderStageList::PS);
		dispersion = 0.03f;
	}
	//���U���̐ݒ�
	void GaussianFilterPS::SetDispersion(float dispersion) { this->dispersion = dispersion; }
	//��O���������ۂɂڂ����Ώ�
	void GaussianFilterPS::Dispatch(Graphics::View* active_view, Graphics::RenderTarget* active_rt, Graphics::Texture* texture) {
		info.width = texture->GetSize().x;
		info.height = texture->GetSize().y;
		static constexpr const UINT DIVISION = 7;
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
		cbuffer->Activate(info);
		rt->Activate();
		view->Activate();
		auto& defaultVS = Graphics::SpriteRenderer::GetVertexShader();
		auto& defaultPS = Graphics::SpriteRenderer::GetPixelShader();
		Graphics::SpriteRenderer::ChangeVertexShader(vsX);
		Graphics::SpriteRenderer::ChangePixelShader(ps);
		Graphics::SpriteRenderer::Render(texture, Math::Vector2(), rt->GetTexture()->GetSize(), 0.0f, Math::Vector2(), texture->GetSize(), 0xFFFFFFFF);
		pass2->Activate();
		Graphics::SpriteRenderer::ChangeVertexShader(vsY);
		Graphics::SpriteRenderer::Render(rt.get(), Math::Vector2(), rt->GetTexture()->GetSize(), 0.0f, Math::Vector2(), rt->GetTexture()->GetSize(), 0xFFFFFFFF);
		Graphics::SpriteRenderer::ChangeVertexShader(defaultVS);
		Graphics::SpriteRenderer::ChangePixelShader(defaultPS);
		Graphics::Texture::Clean(0, Graphics::ShaderStageList::PS);
		active_view->Activate();
		active_rt->Activate();
	}
	void GaussianFilterPS::Dispatch(Graphics::View* active_view, Graphics::RenderTarget* active_rt, std::shared_ptr<Graphics::RenderTarget> rt) {
		Dispatch(active_view, active_rt, rt->GetTexture());
	}
	//XY�u���[���ʂ�`��
	void GaussianFilterPS::Render() {
		Graphics::SpriteRenderer::Render(pass2.get());
		Graphics::Texture::Clean(0, Graphics::ShaderStageList::PS);
	}
	void GaussianFilterPS::Begin(int slot) {
		PostEffect::Begin(slot);
		pass2->GetTexture()->Set(this->slot, Graphics::ShaderStageList::PS);
	}
	void GaussianFilterPS::DebugRender(const Math::Vector2& pos, const Math::Vector2& size) {
		Graphics::SpriteRenderer::Render(pass2.get(), pos, size, 0.0f, Math::Vector2(), pass2->GetTexture()->GetSize(), 0xFFFFFFFF);
		Graphics::Texture::Clean(0, Graphics::ShaderStageList::PS);
	}
	//---------------------------------------------------------------------------------------------
	//
	//	Raycaster
	//
	//---------------------------------------------------------------------------------------------
	RayMesh::RayMesh(Graphics::Model* model) {
		auto mesh = model->GetMesh();
		//�o�b�t�@�̍쐬
		polygonCount = i_cast(mesh->GetCount() / 3);
		structuredBuffer = std::make_shared<StructuredBuffer>(sizeof(Input), polygonCount);
		//�����蔻��p���b�V���̍\�z
		Graphics::Model::Vertex* srcBuffer = mesh->GetBuffer();
		std::vector<Input> buildMesh(polygonCount);
		for (int i = 0; i < polygonCount; i++) {
			for (int j = 0; j < 3; j++) {
				buildMesh[i].pos[j].x = srcBuffer[i * 3 + j].pos.x;
				buildMesh[i].pos[j].y = srcBuffer[i * 3 + j].pos.y;
				buildMesh[i].pos[j].z = srcBuffer[i * 3 + j].pos.z;
			}
		}
		structuredBuffer->Update(buildMesh.data());
	}
	void RayMesh::Set() { structuredBuffer->Set(0, Graphics::ShaderStageList::CS); }
	int RayMesh::GetPolygonCount() { return polygonCount; }
	//---------------------------------------------------------------------------------------------
	RayResult::RayResult(RayMesh* mesh) {
		structuredBuffer = std::make_shared<StructuredBuffer>(sizeof(Output), mesh->GetPolygonCount());
		uav = std::make_unique<UnorderedAccessView>(structuredBuffer.get());
		readBuffer = std::make_unique<ReadGPUBuffer>(structuredBuffer);
	}
	void RayResult::Set() { uav->Set(0); }
	void RayResult::Clean() { uav->Clean(0); }
	const RayResult::Output* RayResult::Lock() {
		readBuffer->ReadCopy();
		return readBuffer->ReadBegin<Output>();
	}
	void RayResult::UnLock() { readBuffer->ReadEnd(); }
	//---------------------------------------------------------------------------------------------
	Raycaster::Raycaster() {
		cs = std::make_unique<Graphics::ComputeShader>("Data/ShaderFile/GPGPU/GPGPU.hlsl", "RaycastCS");
		cbuffer = std::make_unique<Graphics::ConstantBuffer<Info>>(11, Graphics::ShaderStageList::CS);
	}
	void Raycaster::Dispatch(const DirectX::XMMATRIX& world, RayMesh* mesh, RayResult* result, const Math::Vector3& begin, const Math::Vector3& end) {
#ifdef _DEBUG
		//Ray�̃f�o�b�O�\��
		Graphics::DebugRenderer::GetInstance()->SetLine(begin, end, 0xFFFFFFFF);
#endif
		//GPU�Ƀo�b�t�@���M
		mesh->Set(); result->Set();
		//Ray�̏��X�V
		info.rayBegin.x = begin.x; info.rayBegin.y = begin.y; info.rayBegin.z = begin.z;
		info.rayEnd.x = end.x; info.rayEnd.y = end.y; info.rayEnd.z = end.z;
		//�s�����ݒ�
		DirectX::XMStoreFloat4x4(&info.world, DirectX::XMMatrixTranspose(world));
		DirectX::XMVECTOR temp = {};
		DirectX::XMMATRIX inverse = DirectX::XMMatrixInverse(&temp, world);
		DirectX::XMStoreFloat4x4(&info.worldInverse, DirectX::XMMatrixTranspose(inverse));
		cbuffer->Activate(info);
		//Ray����J�n
		cs->Dispatch(mesh->GetPolygonCount(), 1, 1);
	}
	//---------------------------------------------------------------------------------------------
	//
	//		Camera
	//
	//---------------------------------------------------------------------------------------------
	Camera::Camera(const Math::Vector2& scale, const Math::Vector3& pos, const Math::Vector3& at) :pos(pos), at(at) {
		//�����̃r���[���g��Ȃ��O�� �g�������ꍇ�͈����łƂ�
		view = std::make_shared<Graphics::View>(Math::Vector2(), scale);
		up = TakeUp();
	}
	void Camera::SetPos(const Math::Vector3& pos) { this->pos = pos; }
	void Camera::SetTarget(const Math::Vector3& at) { this->at = at; }
	std::shared_ptr<Graphics::View> Camera::GetView() { return view; }
	Math::Vector3 Camera::TakeFront() {
		Math::Vector3 ret = at - pos; ret.Normalize();
		return ret;
	}
	Math::Vector3 Camera::TakeRight() {
		Math::Vector3 front = TakeFront();
		Math::Vector3 tempUP(0.001f, 1.0f, 0.001f); tempUP.Normalize();
		Math::Vector3 ret = Math::Vector3::Cross(tempUP, front); /*ret.Normalize();*/
		return ret;
	}
	Math::Vector3 Camera::TakeUp() {
		Math::Vector3 tempFront = TakeFront();
		Math::Vector3 tempRight = TakeRight();
		Math::Vector3 ret = Math::Vector3::Cross(tempFront, tempRight); /*ret.Normalize();*/
		return ret;
	}
	void Camera::Activate() {
		view->SetEyePos(pos);
		view->SetEyeTarget(at);
		view->SetEyeUpDirection(up);
		view->Activate();
	}
	DebugCamera::DebugCamera(const Math::Vector2& scale, const Math::Vector3& pos, const Math::Vector3& at) :Camera(scale, pos, at) {
		radius = (pos - at).Length();
		front = TakeFront();
		right = TakeRight();
	}
	void DebugCamera::Update() {
		float elapsedTime = Application::GetInstance()->GetProcessTimeSec();
		Math::Vector3 rightMove = {};
		Math::Vector3 upMove = {};
		//���a
		float circumference = radius * PI;
		//���傤�ǂ��������ɓ��������Z�o
		circumference /= 600.0f;
		//�}�E�X
		float wheel = Input::Mouse::GetInstance()->GetWheel();
		//�����_�܂ł̋���
		Math::Vector2 mmove = Input::Mouse::GetInstance()->GetMove();
		rightMove += right * mmove.x * circumference;
		upMove += up * mmove.y * circumference;
		//�L�[�{�[�h
		if (wheel == 0.0f) {
			if (Input::GetKeyboardKey(DIK_W))wheel += 20.0f;
			if (Input::GetKeyboardKey(DIK_S))wheel -= 20.0f;
		}
		if (wheel)radius -= (wheel*circumference*0.1f);
		bool movePos = false; bool moveAt = false;
		if (upMove.LengthSq() == 0) {
			if (Input::GetKeyboardKey(DIK_Z)) {
				upMove += up * 10.0f * circumference;
				movePos = true;
			}
			if (Input::GetKeyboardKey(DIK_UP)) {
				upMove += up * 10.0f * circumference;
				moveAt = true;
			}
			if (Input::GetKeyboardKey(DIK_X)) {
				upMove -= up * 10.0f * circumference;
				movePos = true;
			}
			if (Input::GetKeyboardKey(DIK_DOWN)) {
				upMove -= up * 10.0f * circumference;
				moveAt = true;
			}
		}
		if (rightMove.LengthSq() == 0) {
			if (Input::GetKeyboardKey(DIK_D)) {
				rightMove += right * 10.0f * circumference;
				movePos = true;
			}
			if (Input::GetKeyboardKey(DIK_RIGHT)) {
				rightMove += right * 10.0f * circumference;
				moveAt = true;
			}
			if (Input::GetKeyboardKey(DIK_A)) {
				rightMove -= right * 10.0f * circumference;
				movePos = true;
			}
			if (Input::GetKeyboardKey(DIK_LEFT)) {
				rightMove -= right * 10.0f * circumference;
				moveAt = true;
			}
		}
		if (Input::GetMouseKey(0) || movePos) pos += rightMove + upMove;
		if (Input::GetMouseKey(1) || moveAt) at += rightMove + upMove;
		if (Input::GetMouseKey(2)) {
			pos += rightMove + upMove;
			at += rightMove + upMove;
		}
		if (radius < 2.0f)radius = 2.0f;
		front = TakeFront();
		pos = at - front * radius;
		right = Math::Vector3::Cross(up, front);
		up = Math::Vector3::Cross(front, right);
		//�����O����ǉ��ł���悤�ɂ��Ă��ǂ����A�O����g���\����Ȃ����߂����
		//�J������AO���f����ꏊ��
		if (Input::GetKeyboardKey(DIK_P)) {
			pos = Math::Vector3(57.0f, 66.0f, 106.0f);
			at = Math::Vector3();
			up = Math::Vector3(0.0f, 1.0f, 0.0f);
			radius = (pos - at).Length();
		}
		//���C�g���f����ꏊ��
		if (Input::GetKeyboardKey(DIK_O)) {
			pos = Math::Vector3(-343.0f, 33.0f, -11.0f);
			at = Math::Vector3();
			up = Math::Vector3(0.0f, 1.0f, 0.0f);
			radius = (pos - at).Length();
		}
	}
	//---------------------------------------------------------------------------------------------
	//
	//		Scene
	//
	//---------------------------------------------------------------------------------------------
	void SceneDeferred::Initialize() {
		const constexpr Math::Vector2 scale(1280, 720);
		camera = std::make_unique<DebugCamera>(scale, Math::Vector3(57.0f, 66.0f, 106.0f), Math::Vector3(0.0f, 0.0f, 0.0f));
		deferredBuffer = std::make_unique<DeferredBuffer>(scale);
		normalMap = TRUE; useLight = TRUE; useFog = TRUE;
#ifdef _DEBUG
		HostConsole::GetInstance()->IntRegister("deferred", "normal map", &normalMap, false);
		HostConsole::GetInstance()->IntRegister("deferred", "use light", &useLight, false);
		HostConsole::GetInstance()->IntRegister("deferred", "use fog", &useFog, false);
#endif
		//model = std::make_shared<Graphics::Model>("Data/Model/Deferred/stage.dxd", "Data/Model/Deferred/stage.mt");
		model = std::make_shared<Graphics::Model>("Data/Model/maps/stage.dxd", "Data/Model/maps/stage.mt");
#ifdef SIMPLE_SHADER
		deferredShader = std::make_unique<SimpleDeferred>();
#endif
#ifdef FULL_EFFECT
		//�����ݒu
		deferredShader = std::make_unique<PointLightDeferred>();
		PointLightDeferred::PointLight light;
		for (int i = 0; i < LIGHT_COUNT; i++) {
			light.pos = Math::Vector4(Utility::Frand(-60.0f, 60.0f), Utility::Frand(-10.0f, 10.0f), Utility::Frand(-40.0f, 40.0f), 0.0f);
			light.pos += Math::Vector4(-213.0f, 5.0f, -5.0f, 0.0f);
			light.color = Utility::Color(rand() % 255, rand() % 255, rand() % 255, 255);
			light.attenuation = Utility::Frand(0.5f, 10.0f);
			deferredShader->SetLightBuffer(i + 1, light);
		}
#endif
#ifdef USE_SSAO
#ifdef SSAO_PS
		ssao = std::make_unique<SSAOPS>(scale);
#endif
#ifdef SSAO_CS
		ssao = std::make_unique<SSAOCS>(scale*(QUALITY > 1.0f ? 1.0f : QUALITY));
#endif
#endif
#ifdef CASCADE
		shadow = std::make_unique<ShadowBuffer>(scale*QUALITY, 4, true);
#else
		shadow = std::make_unique<ShadowBuffer>(scale, 1, true);
#endif
		skybox = std::make_unique<SkyBox>("Data/Model/skybox.dxd", "Data/Model/skybox.mt");
		//���C�֌W�̏�����
		raycaster = std::make_unique<Raycaster>();
		rayMesh = std::make_unique<RayMesh>(model.get());
		rayResult = std::make_unique<RayResult>(rayMesh.get());
		//shadow = std::make_unique<ShadowBuffer>(Math::Vector2(1280, 720), 1, false);
		//gaussian = std::make_unique<GaussianFilterCS>(scale);
	}
	SceneDeferred::~SceneDeferred() {
#ifdef _DEBUG
		HostConsole::GetInstance()->VariableUnRegister("deferred");
#endif
	}
	void SceneDeferred::AlwaysUpdate() {
		Math::Vector3 rayBegin(0.0f, 10.0f, 0.0f); Math::Vector3 rayEnd(0.0f, -10.0f, 0.0f);
		Math::Vector3 dir = rayEnd - rayBegin; dir.Normalize();
#ifdef GPU_RAYCASTER
		//Ray����
		DirectX::XMMATRIX world;
		model->GetWorldMatrix(&world);
		raycaster->Dispatch(world, rayMesh.get(), rayResult.get(), rayBegin, rayEnd);
#endif
		if (Input::GetKeyboardKey(DIK_7) == 1) normalMap = !normalMap;
		if (Input::GetKeyboardKey(DIK_6) == 1) useFog = !useFog;
		if (Input::GetKeyboardKey(DIK_5) == 1) useLight = !useLight;

		deferredBuffer->AddModel(model, normalMap);
#ifdef FULL_EFFECT
		//�����̃J�����ʒu�ɂ�������u��
		PointLightDeferred::PointLight light;
		//light.pos = Math::Vector4(pos.x, pos.y, pos.z, 0.0f);
		light.pos = Math::Vector4(0.0f, 0.0f, 0.0f, 0.0f);
		light.color = 0xFFFFFFFF;
		//light.color = Utility::Color(rand() % 255, rand() % 255, rand() % 255, 255);
		light.attenuation = 20.0f;
		deferredShader->SetLightBuffer(0, light);
		if (useLight)deferredShader->SetUseCount(LIGHT_COUNT + 1);
		else deferredShader->SetUseCount(0);
		deferredShader->Update();
#endif
		shadow->AddModel(model);
		shadow->SetPos(Math::Vector3(200.0f, 130.0f, 200.0f));
		//shadow->SetPos(pos);
		camera->Update();
#ifdef CPU_RAYCASTER
		Math::Vector3 out, normal;
		if (model->RayPickWorld(&out, &normal, rayBegin, dir, 20.0f)) {
#ifdef _DEBUG
			Math::Vector3 dir = rayEnd - rayBegin; dir.Normalize();
			Graphics::DebugRenderer::GetInstance()->SetPoint(out, 0xFFFF0000);
#endif
		}
#endif
#ifdef GPU_RAYCASTER
		auto result = rayResult->Lock();
		for (int i = 0; i < rayMesh->GetPolygonCount(); i++) {
			if (result[i].hit) {
#ifdef _DEBUG
				Math::Vector3 dir = rayEnd - rayBegin; dir.Normalize();
				Graphics::DebugRenderer::GetInstance()->SetPoint(rayBegin + dir * result[i].length, 0xFFFF0000);
				Graphics::DebugRenderer::GetInstance()->SetLine(rayBegin, rayBegin + result[i].normal, 0xFF00FFFF);
#endif
			}
		}
		rayResult->UnLock();
#endif
	}
	void SceneDeferred::AlwaysRender() {
		Graphics::Environment::GetInstance()->SetLightDirection(-Math::Vector3(1.0f, 1.0f, 1.0f));
		Graphics::Environment::GetInstance()->SetActiveLinearFog(useFog);
		Graphics::Environment::GetInstance()->SetFogBegin(150.0f);
		Graphics::Environment::GetInstance()->SetFogEnd(400.0f);
		Graphics::Environment::GetInstance()->Activate();
		camera->Activate();
		Graphics::RenderTarget* backBuffer = Application::GetInstance()->GetSwapChain()->GetRenderTarget();
		//shadow->CreateShadowMap(view.get(), backBuffer);
		shadow->CreateShadowMap(camera->GetView().get(), backBuffer);
		skybox->Render(camera.get());
		//view->Activate();
		shadow->Begin();
		deferredBuffer->RenderGBuffer();
		shadow->End();
		backBuffer->Activate();
#ifdef USE_SSAO
#ifdef SSAO_PS
		ssao->CreateAO(backBuffer, deferredBuffer.get());
#endif
#ifdef SSAO_CS
		ssao->CreateAO(backBuffer, camera->GetView().get(), deferredBuffer.get());
#endif
		ssao->Begin(5);
#endif
		deferredBuffer->Begin();
		deferredShader->Render();
		deferredBuffer->End();
#ifdef USE_SSAO
		ssao->End();
#endif
#ifdef _DEBUG
		if (Application::GetInstance()->debugRender) {
			deferredBuffer->DebugRender();
			shadow->DebugRender();
#ifdef USE_SSAO
			ssao->Render();
#endif
		}
#endif
	}
}