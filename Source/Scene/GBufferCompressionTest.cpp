#include "Lobelia.hpp"
#include "GBufferCompressionTest.hpp"

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
		vs = std::make_shared<Graphics::VertexShader>("Data/ShaderFile/3D/GBufferCompressionTest.hlsl", "CreateGBufferVS", Graphics::VertexShader::Model::VS_5_0);
		ps = std::make_shared<Graphics::PixelShader>("Data/ShaderFile/3D/GBufferCompressionTest.hlsl", "CreateGBufferPS", Graphics::PixelShader::Model::PS_5_0);
	}
	void GBufferManager::AddModel(std::shared_ptr<Graphics::Model>& model) { modelList.push_back(model); }
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
		view->ViewportActivate();
		modelList.clear();
	}
	std::array<std::unique_ptr<Graphics::RenderTarget>, 2>& GBufferManager::GetRTs() { return rts; }
	//void GBufferManager::Begin() {
	//	for (int i = 0; i < rts.size(); i++) {
	//		rts[i]->GetTexture()->Set(i, Graphics::ShaderStageList::PS);
	//	}
	//}
	//void GBufferManager::End() {
	//	for (int i = 0; i < rts.size(); i++) {
	//		Graphics::Texture::Clean(i, Graphics::ShaderStageList::PS);
	//	}
	//}
	DeferredShadeManager::DeferredShadeManager(const Math::Vector2& size) {
		viewport = std::make_unique<Graphics::View>(Math::Vector2(), size);
		//ShaderModel5.0�ȏ�K�{
		ps = std::make_shared<Graphics::PixelShader>("Data/ShaderFile/3D/GBufferCompressionTest.hlsl", "DeferredPS", Graphics::PixelShader::Model::PS_5_0);
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
	//G-Buffer���k�p�̃e�X�g�V�[���ł�
	GBufferCompressionTest::~GBufferCompressionTest() {
	}
	void GBufferCompressionTest::Initialize() {
		Math::Vector2 wsize = Application::GetInstance()->GetWindow()->GetSize();
		camera = std::make_unique<ViewerCamera>(wsize, Math::Vector3(0.0f, 10.0f, 10.0f), Math::Vector3());
		gbuffer = std::make_shared<GBufferManager>(wsize);
		deferredShader = std::make_shared<DeferredShadeManager>(wsize);
		//const Math::Vector2& size, const DXGI_SAMPLE_DESC& sample, const DXGI_FORMAT&  format = DXGI_FORMAT_R32G32B32A32_FLOAT, int array_count = 1);
		offScreen = std::make_shared<Graphics::RenderTarget>(wsize, DXGI_SAMPLE_DESC{ 1,0 }, DXGI_FORMAT_R8G8B8A8_UNORM);
		stage = std::make_shared<Graphics::Model>("Data/Model/maps/stage.dxd", "Data/Model/maps/stage.mt");
		stage->Translation(Math::Vector3(0.0f, 1.0f, 0.0f));
		//stage->Scalling(3.0f);
		stage->CalcWorldMatrix();
	}
	void GBufferCompressionTest::AlwaysUpdate() {
		camera->Update();
		gbuffer->AddModel(stage);
	}
	void GBufferCompressionTest::AlwaysRender() {
		Graphics::Environment::GetInstance()->SetLightDirection(Math::Vector3(-1.0f, -1.0f, -1.0f));
		Graphics::Environment::GetInstance()->Activate();
		camera->Activate();
		gbuffer->RenderGBuffer(camera->GetView());
		//�o�b�N�o�b�t�@��L����
		offScreen->Clear(0x00000000);
		offScreen->Activate();
		deferredShader->Render(camera->GetView(), gbuffer);
		Application::GetInstance()->GetSwapChain()->GetRenderTarget()->Activate();
		Graphics::SpriteRenderer::Render(offScreen.get());
		Graphics::Texture::Clean(0, Graphics::ShaderStageList::PS);
		//stage->Render();
	}

}