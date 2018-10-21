#pragma once
namespace Lobelia::Game {
	//---------------------------------------------------------------------------------------------
//
//	Geometry Buffer
//
//---------------------------------------------------------------------------------------------
//GBuffer周り
	class DeferredBuffer {
	public:
		enum class BUFFER_TYPE {
			POS,
			NORMAL,
			COLOR,
			VIEW_POS,
			SHADOW,
			MAX,
		};
	public:
		DeferredBuffer(const Math::Vector2& size);
		~DeferredBuffer() = default;
		void AddModel(std::shared_ptr<Graphics::Model> model, bool use_normal_map);
		void RenderGBuffer();
		std::shared_ptr<Graphics::RenderTarget> GetRenderTarget(BUFFER_TYPE type);
		//フラグ取ってどの情報を有効にするのか切り替えれるといいと思う
		void Begin();
		void End();
		void DebugRender();
	private:
		ALIGN(16) struct Info {
			int useNormalMap;
			int useSpecularMap;
		};
		struct ModelStorage {
			std::weak_ptr<Graphics::Model> model;
			Info info;
		};
	private:
		//MRT用 バッファの可視性のために法線と深度を現状は分けている a部分にdepthを入れれば数は減る
		std::shared_ptr<Graphics::RenderTarget> rts[i_cast(BUFFER_TYPE::MAX)];
		//情報書き込み用
		std::shared_ptr<Graphics::VertexShader> vs;
		std::shared_ptr<Graphics::PixelShader> ps;
		std::list<ModelStorage> models;
		const Math::Vector2 size;
		std::unique_ptr<Graphics::ConstantBuffer<Info>> cbuffer;
	};

}