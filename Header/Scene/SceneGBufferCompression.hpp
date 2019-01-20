#pragma once
#include "Common/Camera.hpp"
#include "../Data/ShaderFile/Define.h"
#include "Common/AdaptiveConsole.hpp"
#include "Common/ComputeBuffer.hpp"
#include "Common/PostEffect.hpp"
#include "Common/PostEffectExperimental.hpp"

namespace Lobelia::Game {
	//圧縮したG-Buffer格納用
	class GBufferManager {
	public:
		//すべて精度はfloat16として保存されます
		ALIGN(16) struct Info {
			//ハーフランバートに対して掛けられる値
			float lightingFactor;
			//スぺキュラの強さ
			float specularFactor;
			//エミッションの強さ
			float emissionFactor;
		};
	public:
		GBufferManager(const Math::Vector2& size);
		~GBufferManager() = default;
		void AddModel(std::shared_ptr<Graphics::Model>& model, Info info);
		void SetSkybox(std::shared_ptr<class SkyBox>& skybox);
		void RenderGBuffer(std::shared_ptr<Graphics::View>& view);
		std::array<std::unique_ptr<Graphics::RenderTarget>, 2>& GetRTs();
	private:
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
		std::list<ModelStorage> modelList;
		std::unique_ptr<Graphics::ConstantBuffer<Info>> cbuffer;
	};
	//圧縮したG-Bufferを使用してシェーディングを行う用
	//また、トーンマップやカラーコレクションも行う
	class DeferredShadeManager {
	public:
		DeferredShadeManager(const Math::Vector2& size);
		~DeferredShadeManager() = default;
		void Render(std::shared_ptr<Graphics::View>& view, std::shared_ptr<GBufferManager>& gbuffer);
	private:
		std::unique_ptr<Graphics::View> viewport;
		std::shared_ptr<Graphics::VertexShader> vs;
		std::shared_ptr<Graphics::PixelShader> ps;
		//std::unique_ptr<Graphics::RenderTarget> rt;
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
		static void LightingIntensityRender(std::shared_ptr<GBufferManager>& gbuffer, const Math::Vector2& pos, const Math::Vector2& size);
		static void SpecularIntensityRender(std::shared_ptr<GBufferManager>& gbuffer, const Math::Vector2& pos, const Math::Vector2& size);
		static void EmissionRender(std::shared_ptr<GBufferManager>& gbuffer, const Math::Vector2& pos, const Math::Vector2& size);
	private:
		static void BufferRender(std::shared_ptr<Graphics::PixelShader>& ps, Graphics::RenderTarget* rt, const Math::Vector2& pos, const Math::Vector2& size);
	private:
		//現状この三つ
		static std::shared_ptr<Graphics::PixelShader> psColor;
		static std::shared_ptr<Graphics::PixelShader> psDepth;
		static std::shared_ptr<Graphics::PixelShader> psWorldPos;
		static std::shared_ptr<Graphics::PixelShader> psNormal;
		static std::shared_ptr<Graphics::PixelShader> psLightingIntensity;
		static std::shared_ptr<Graphics::PixelShader> psSpecularIntensity;
		static std::shared_ptr<Graphics::PixelShader> psEmission;
		static std::shared_ptr<Graphics::PixelShader> psEmissionIntensity;
	};
	//実質カスケード用
	class GaussianTextureArray {
	public:
		GaussianTextureArray(const Math::Vector2& size, int array_count, DXGI_FORMAT format);
		~GaussianTextureArray() = default;
		//サイズ等の変更
		void ChangeBuffer(const Math::Vector2& size, int array_count, DXGI_FORMAT format);
		void Update(float dispersion);
		void Dispatch(Graphics::RenderTarget* active_rt, Graphics::View*active_view, Graphics::Texture* tex);
		std::shared_ptr<Graphics::RenderTarget>& GetRTSResult();
	private:
		static constexpr const int DIVISION = 4;
	private:
		ALIGN(16) struct Info {
			float weight[DIVISION];
			int texIndex;
			Math::Vector2 texSize;
		};
	private:
		int arrayCount;
		std::shared_ptr<Graphics::VertexShader> vsX;
		std::shared_ptr<Graphics::VertexShader> vsY;
		std::shared_ptr<Graphics::PixelShader> ps;
		std::unique_ptr<Graphics::RenderTarget> rtsX;
		std::shared_ptr<Graphics::RenderTarget> rtsResult;
		std::unique_ptr<Graphics::View> viewport;
		std::unique_ptr<Graphics::ConstantBuffer<Info>> cbuffer;
		Info info;
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
		/*void SetDispersion(float dispersion);*/
		void AddModel(std::shared_ptr<Graphics::Model>& model);
		//内部で自動的に呼ばれます、大体の場合は明示的に呼ぶ必要はない
		void ClearModels();
		//シャドウマップへの深度書き込み
		void RenderShadowMap(Graphics::View* active_view, Graphics::RenderTarget* active_rt);
		//シャドウマップを別テクスチャに書き込み
		void DebugShadowMapRender(int index, const Math::Vector2& pos, const Math::Vector2& size);
		void DebugDataTextureRender(const Math::Vector2& pos, const Math::Vector2& size);
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
		std::unique_ptr<GaussianTextureArray> gaussian;
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
		std::shared_ptr<Camera> camera;
		std::shared_ptr<GBufferManager> gbuffer;
		std::unique_ptr<DeferredShadeManager> deferredShader;
		std::unique_ptr<Experimental::MultipleGaussianBloom> bloom;
	public:
		//デモ用関数から触れるように
		std::unique_ptr<CascadeShadowBuffers> shadowMap;
	private:
		std::shared_ptr<Graphics::RenderTarget> offScreen;
		std::shared_ptr<Graphics::RenderTarget> depth;
		std::shared_ptr<Graphics::RenderTarget> emission;
		std::shared_ptr<SkyBox> skybox;
		std::shared_ptr<Graphics::Model> stage;
		std::shared_ptr<Graphics::Model> box;
		//PostEffect
		std::unique_ptr<Experimental::DepthOfField> dof;
		std::unique_ptr<Experimental::SSAO> ssao;
		std::unique_ptr<AdaptiveConsole> operationConsole;
	public:
		//デモ用
		bool cameraMove;
		//G-Bufferの描画フラグ
		bool renderGBuffer;
		bool renderColor;
		bool renderDepth;
		bool renderWPos;
		bool renderNormal;
		bool renderLightingIntensity;
		bool renderSpecularIntensity;
		bool renderEmissionColor;
		bool renderSSAO;
		bool renderShadingBuffer;
		bool renderDoFBuffer;
		bool renderShadowMap;
		bool renderBloomBuffer;
		//ShadowMap用
		Math::Vector2 shadowMapSize;
		bool useShadow;
		bool useVariance;
		int cascadeCount;
		float shadowNearZ;
		float shadowFarZ;
		float shadowLamda;
		bool useSSAO;
		float ssaoThreshold;
		bool useDoF;
		float focusRange;
		//モデル描画用パラメーター
		GBufferManager::Info stageInfo;
		GBufferManager::Info boxInfo;
		//影ライト用
		float rad;
		Math::Vector3 lpos;
	};
}