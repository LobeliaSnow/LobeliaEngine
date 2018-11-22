#pragma once
namespace Lobelia::Game {
	//---------------------------------------------------------------------------------------------
	//
	//	Geometry Buffer
	//
	//---------------------------------------------------------------------------------------------
	//GBuffer周り
	//エミッションとスぺキュラは同一の扱い
	//ただし、エミッションを切った場合でもスぺキュラがあればそれはブルームとして光る
	class DeferredBuffer {
	public:
		enum class BUFFER_TYPE {
			POS,
			NORMAL,
			COLOR,
			VIEW_POS,
			SHADOW_SSS,
			EMISSION_COLOR,
			MATERIAL_ID,
			MAX,
		};
		enum class MATERIAL_TYPE :int {
			LAMBERT = MAT_LAMBERT,
			PHONG = MAT_PHONG,
			COLOR = MAT_COLOR,
		};
	public:
		DeferredBuffer(const Math::Vector2& size);
		~DeferredBuffer() = default;
		void AddModel(std::shared_ptr<Graphics::Model> model, MATERIAL_TYPE material = MATERIAL_TYPE::LAMBERT, float specular_factor = 1.0f, float emission_factor = 0.0f);
		void SetSkybox(std::shared_ptr<class SkyBox>& skybox);
		void RenderGBuffer();
		std::shared_ptr<Graphics::RenderTarget>& GetRenderTarget(BUFFER_TYPE type);
		//フラグ取ってどの情報を有効にするのか切り替えれるといいと思う
		void Begin();
		void End();
		void DebugRender();
	private:
		ALIGN(16) struct Info {
			MATERIAL_TYPE materialType;
			float specularFactor;
			float emissionFactor;
			float transparent;
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
		std::shared_ptr<SkyBox> skybox;
		std::list<ModelStorage> models;
		const Math::Vector2 size;
		std::unique_ptr<Graphics::ConstantBuffer<Info>> cbuffer;
	};

}