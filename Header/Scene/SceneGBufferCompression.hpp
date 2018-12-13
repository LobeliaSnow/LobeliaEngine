#pragma once
#include "Common/Camera.hpp"
#include "../Data/ShaderFile/Define.h"
#include "Common/AdaptiveConsole.hpp"

namespace Lobelia::Game {
	//圧縮したG-Buffer格納用
	class GBufferManager {
	public:
		enum class MATERIAL_TYPE :int {
			LAMBERT = MAT_LAMBERT,
			PHONG = MAT_PHONG,
			COLOR = MAT_COLOR,
		};
	public:
		GBufferManager(const Math::Vector2& size);
		~GBufferManager() = default;
		void AddModel(std::shared_ptr<Graphics::Model>& model);
		void SetSkybox(std::shared_ptr<class SkyBox>& skybox);
		void RenderGBuffer(std::shared_ptr<Graphics::View>& view);
		std::array<std::unique_ptr<Graphics::RenderTarget>, 2>& GetRTs();
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
		std::unique_ptr<Graphics::View> viewport;
		std::array<std::unique_ptr<Graphics::RenderTarget>, 2> rts;
		std::shared_ptr<Graphics::VertexShader> vs;
		std::shared_ptr<Graphics::PixelShader> ps;
		std::shared_ptr<Graphics::BlendState> blend;
		std::shared_ptr<SkyBox> skybox;
		std::list<std::weak_ptr<Graphics::Model>> modelList;
		std::unique_ptr<Graphics::ConstantBuffer<Info>> cbuffer;
	};

	//圧縮したG-Bufferを使用してシェーディングを行う用
	class DeferredShadeManager {
	public:
		DeferredShadeManager(const Math::Vector2& size);
		~DeferredShadeManager() = default;
		void Render(std::shared_ptr<Graphics::View>& view, std::shared_ptr<GBufferManager>& gbuffer);
	private:
		std::unique_ptr<Graphics::View> viewport;
		std::shared_ptr<Graphics::PixelShader> ps;
	};

	//圧縮されたG-Bufferをデコードし、個別表示等を行ったりする
	class GBufferDecodeRenderer {
	public:
		static void Initialize();
		static void Finalize();
		static void ColorRender(std::shared_ptr<GBufferManager>& gbuffer, const Math::Vector2& pos, const Math::Vector2& size);
		static void DepthRender(std::shared_ptr<GBufferManager>& gbuffer, const Math::Vector2& pos, const Math::Vector2& size);
		static void WorldPosRender(std::shared_ptr<GBufferManager>& gbuffer, const Math::Vector2& pos, const Math::Vector2& size);
		static void NormalRender(std::shared_ptr<GBufferManager>& gbuffer, const Math::Vector2& pos, const Math::Vector2& size);
	private:
		static void BufferRender(std::shared_ptr<Graphics::PixelShader>& ps, Graphics::RenderTarget* rt, const Math::Vector2& pos, const Math::Vector2& size);
	private:
		//現状この三つ
		static std::shared_ptr<Graphics::PixelShader> psColor;
		static std::shared_ptr<Graphics::PixelShader> psDepth;
		static std::shared_ptr<Graphics::PixelShader> psWorldPos;
		static std::shared_ptr<Graphics::PixelShader> psNormal;
	};

	//カスケードシャドウを実現するためのバッファ
	class CascadeShadowBuffers {
	public:
		enum FORMAT {
			BIT_16,
			BIT_32,
		};
	public:
		//現状分割数6だけ未対応
		//RenderTargetが6枚で作成した際、キューブマップとして作っているので破綻
		//ライブラリ作り直す際に修正
		CascadeShadowBuffers(int split_count, const Math::Vector2& size, FORMAT format, bool use_variance);
		~CascadeShadowBuffers() = default;
		//ライトカメラ用情報を設定
		void SetNear(float near_z);
		void SetFar(float far_z);
		void SetPos(const Math::Vector3& pos);
		void SetTarget(const Math::Vector3& at);
		//対数分割、一様分割のブレンド率
		void SetLamda(float lamda);
		//レンダーターゲットを作り直します
		void ChangeState(int split_count, const Math::Vector2& size, FORMAT format, bool use_variance);
		//シャドウ有効、無効
		void SetEnable(bool enable);
		void AddModel(std::shared_ptr<Graphics::Model>& model);
		//内部で自動的に呼ばれます、大体の場合は明示的に呼ぶ必要はない
		void ClearModels();
		//シャドウマップへの深度書き込み
		void RenderShadowMap(Graphics::View* active_view, Graphics::RenderTarget* active_rt);
		//シャドウマップを別テクスチャに書き込み
		void DebugShadowMapRender(int index, const Math::Vector2& pos, const Math::Vector2& size);
		//シャドウマップをセットする
		void SetShadowMap(int shadow_map_slot, int data_slot);
	private:
		//制御用パラメータ
		ALIGN(16) struct Info {
			DirectX::XMFLOAT4X4 lightViewProj;
			Math::Vector4 pos;
			Math::Vector4 front;
			int useShadowMap;
			int useVariance;
			int splitCount;
			int nowIndex;
		};
		struct AABB {
			Math::Vector3 min;
			Math::Vector3 max;
		};
		//カスケード用データ
		struct SplitData {
			float splitDist;
			DirectX::XMFLOAT4X4 lvp;
		};
	private:
		//視錘台のAABB計算
		AABB CalcFrustumAABB(Graphics::View* main_camera, float near_z, float far_z, const DirectX::XMFLOAT4X4& lvp);
		//分割計算
		void ComputeSplit(float lamda, float near_z, float far_z, float* split_pos);
		//クロップ行列の作成
		DirectX::XMFLOAT4X4 FrustumAABBClippingMatrix(AABB clip_aabb);
		//カスケードで使用する情報の計算
		void ComputeCascade(Graphics::View* main_camera, const DirectX::XMFLOAT4X4& lvp);
		//ライトのビュープロジェクション行列を算出し、カスケードの計算
		void Update(Graphics::View* main_camera);
	private:
		//RenderTargetArray
		std::unique_ptr<Graphics::RenderTarget> rts;
		//データ用テクスチャのサイズ
		Math::Vector2 dataSize;
		//データ書き込み用テクスチャ
		std::unique_ptr<Graphics::Texture> dataTexture;
		//カスケードで使用するデータ、テクスチャに書き込まれる
		std::vector<SplitData> data;
		//深度バッファ記録用
		std::shared_ptr<Graphics::VertexShader> vs;
		std::shared_ptr<Graphics::PixelShader> ps;
		std::unique_ptr<Graphics::View> viewport;
		//std::unique_ptr<GaussianFilterPS> gaussian;
		//デバッグ描画用
		std::shared_ptr<Graphics::PixelShader> debugPS;
		//シャドウについての定数バッファ
		std::unique_ptr<Graphics::ConstantBuffer<Info>> cbuffer;
		//制御パラメータ
		Info info;
		//描画されるモデルのリスト
		std::list<std::weak_ptr<Graphics::Model>> models;
		//ライトカメラの情報
		float nearZ;
		float farZ;
		float lamda;
		//ライトのトランスフォーム情報
		Math::Vector3 pos;
		Math::Vector3 at;
		Math::Vector3 up;
	};

	//G-Buffer圧縮用のシーンです
	class SceneGBufferCompression :public Scene {
	public:
		SceneGBufferCompression() = default;
		~SceneGBufferCompression();
		void Initialize()override;
		void AlwaysUpdate()override;
		void AlwaysRender()override;
	private:
		std::unique_ptr<Camera> camera;
		std::shared_ptr<GBufferManager> gbuffer;
		std::unique_ptr<DeferredShadeManager> deferredShader;
	public:
		//デモ用関数から触れるように
		std::unique_ptr<CascadeShadowBuffers> shadowMap;
	private:
		std::shared_ptr<Graphics::RenderTarget> offScreen;
		std::shared_ptr<Graphics::Model> stage;
		std::unique_ptr<AdaptiveConsole> operationConsole;
	public:
		//デモ用
		//G-Bufferの描画フラグ
		bool renderGBuffer;
		bool renderColor;
		bool renderDepth;
		bool renderWPos;
		bool renderNormal;
		//ShadowMap用
		Math::Vector2 shadowMapSize;
		bool useShadow;
		bool useVariance;
		bool renderShadowMap;
		int cascadeCount;
		float shadowNearZ;
		float shadowFarZ;
		float shadowLamda;
		float rad;
		Math::Vector3 lpos;
	};
}