#pragma once
#include "Common/Camera.hpp"

namespace Lobelia::Game {
	//���k����G-Buffer�i�[�p
	class GBufferManager {
	public:
		GBufferManager(const Math::Vector2& size);
		~GBufferManager() = default;
		void AddModel(std::shared_ptr<Graphics::Model>& model);
		void RenderGBuffer(std::shared_ptr<Graphics::View>& view);
		std::array<std::unique_ptr<Graphics::RenderTarget>, 2>& GetRTs();
	private:
		std::unique_ptr<Graphics::View> viewport;
		std::array<std::unique_ptr<Graphics::RenderTarget>, 2> rts;
		std::shared_ptr<Graphics::VertexShader> vs;
		std::shared_ptr<Graphics::PixelShader> ps;
		std::shared_ptr<Graphics::BlendState> blend;
		std::list<std::weak_ptr<Graphics::Model>> modelList;
	};
	//���k����G-Buffer���g�p���ăV�F�[�f�B���O���s���p
	class DeferredShadeManager {
	public:
		DeferredShadeManager(const Math::Vector2& size);
		~DeferredShadeManager() = default;
		void Render(std::shared_ptr<Graphics::View>& view, std::shared_ptr<GBufferManager>& gbuffer);
	private:
		std::unique_ptr<Graphics::View> viewport;
		std::shared_ptr<Graphics::PixelShader> ps;
	};
	//G-Buffer���k�p�̃e�X�g�V�[���ł�
	class GBufferCompressionTest :public Scene {
	public:
		GBufferCompressionTest() = default;
		~GBufferCompressionTest();
		void Initialize()override;
		void AlwaysUpdate()override;
		void AlwaysRender()override;
	private:
		std::unique_ptr<Camera> camera;
		std::shared_ptr<GBufferManager> gbuffer;
		std::shared_ptr<DeferredShadeManager> deferredShader;
		std::shared_ptr<Graphics::RenderTarget> offScreen;
		std::shared_ptr<Graphics::Model> stage;
	};
}