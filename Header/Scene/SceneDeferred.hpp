#pragma once
#include "../Data/ShaderFile/Define.h"

//パフォーマンス表記はあくまでも目安、高速に移り変わるのを目視で確認しただけなのと、
//環境が自分のノートPCのみなので信頼性は低い

namespace Lobelia::Game {
	//---------------------------------------------------------------------------------------------
	//
	//	Skybox
	//
	//---------------------------------------------------------------------------------------------
	class SkyBox {
	public:
		SkyBox(const char* model_path, const char* mt_path);
		~SkyBox() = default;
		void Render(class Camera* camera);
	private:
		std::unique_ptr<Graphics::Model> model;
		std::shared_ptr<Graphics::DepthStencilState> depth;
	};
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
	//---------------------------------------------------------------------------------------------
	//実際にシェーディングする部分
	class DeferredShader {
	public:
		DeferredShader(const char* file_path, const char* entry_vs, const char* entry_ps);
		virtual ~DeferredShader() = default;
		void Render();
	private:
		std::shared_ptr<Graphics::VertexShader> vs;
		std::shared_ptr<Graphics::PixelShader> ps;
	};
	//---------------------------------------------------------------------------------------------
	class SimpleDeferred :public DeferredShader {
	public:
		SimpleDeferred();
	};
	//---------------------------------------------------------------------------------------------
	class PointLightDeferred :public DeferredShader {
	public:
		struct PointLight {
			Math::Vector4 pos;
			Utility::Color color;
			float attenuation;
		};
	public:
		PointLightDeferred();
		void SetLightBuffer(int index, const PointLight& p_light);
		void SetUseCount(int use_count);
		void Update();
	private:
		ALIGN(16) struct PointLights {
			Math::Vector4 pos[LIGHT_SUM];
			Math::Vector4 color[LIGHT_SUM];
			Math::Vector4 attenuation[LIGHT_SUM];
			int usedLightCount;
		}lights;
		std::unique_ptr<Graphics::ConstantBuffer<PointLights>> cbuffer;
	};
	//---------------------------------------------------------------------------------------------
	//現状遠距離がすごく荒いので、カスケード急ぎたい。
	//Parallel Split Shadow Map (カスケード系)実装予定
	//カスケードの参考
	//http://www.project-asura.com/program/d3d11/d3d11_009.html
#ifdef GAUSSIAN_CS
	class GaussianFilterCS;
#endif
#ifdef GAUSSIAN_PS
	class GaussianFilterPS;
#endif
	//後に作り直す
	class ShadowBuffer {
	public:
		ShadowBuffer(const Math::Vector2& size, int split_count, bool use_variance);
		void AddModel(std::shared_ptr<Graphics::Model> model);
		void SetPos(const Math::Vector3& pos);
		void SetTarget(const Math::Vector3& at);
		void CreateShadowMap(Graphics::View* active_view, Graphics::RenderTarget* active_rt);
		//シャドウマップをセットする
		//テクスチャスロット6~6+split_countセットする
		//コンスタントバッファは
		//ALIGN(16) struct Info {
		//	DirectX::XMFLOAT4X4 view;
		//	DirectX::XMFLOAT4X4 proj[split_count];
		//#ifdef CASCADE
		//	Math::Vector4 lpos;
		// Math::Vector4 ldir;
		//	Math::Vector4 splitPos;
		//#endif
		//	int useShadowMap;
		//	int useVariance;
		//};
		//の形式でバインドされる
		void Begin();
		void End();
		void DebugRender();
	private:
		//void AdjustClipPlanes();
		void ComputeSplit(float lamda, float near_z, float far_z);
		void CameraUpdate();
	private:
		//後に代わる
		ALIGN(16) struct Info {
			DirectX::XMFLOAT4X4 view;
#ifdef CASCADE
			//4前提
			std::array<DirectX::XMFLOAT4X4, 4> proj;
			Math::Vector4 pos;
			Math::Vector4 front;
			float splitPos[4];
#else
			std::array<DirectX::XMFLOAT4X4, 1> proj;
#endif
			int useShadowMap;
			int useVariance;
		};
	private:
		std::vector<std::unique_ptr<Graphics::View>> views;
		std::vector<std::shared_ptr<Graphics::RenderTarget>> rts;
		//near/farが入ってます
		std::vector<float> cascadeValues;
		//分割位置が入ってます
		std::vector<float> splitPositions;
		std::list<std::weak_ptr<Graphics::Model>> models;
		std::shared_ptr<Graphics::VertexShader> vs;
		std::shared_ptr<Graphics::PixelShader> ps;
		std::unique_ptr<Graphics::ConstantBuffer<Info>> cbuffer;
		//std::unique_ptr<Graphics::SamplerState> sampler;
		Math::Vector3 pos;
		Math::Vector3 at;
		Math::Vector3 up;
#ifdef GAUSSIAN_CS
		std::vector<std::unique_ptr<GaussianFilterCS>> gaussian;
#endif
#ifdef GAUSSIAN_PS
		std::vector<std::unique_ptr<GaussianFilterPS>> gaussian;
#endif
		Info info;
		Math::Vector2 size;
		const int count;
	};
	//---------------------------------------------------------------------------------------------
	//
	//	Post Effect
	//
	//---------------------------------------------------------------------------------------------
	class PostEffect abstract {
	public:
		PostEffect(const Math::Vector2& size, bool create_rt, DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT);
		virtual ~PostEffect() = default;
		std::shared_ptr<Graphics::RenderTarget>& GetRenderTarget();
		//デフォルトの動作は、rtの描画
		virtual void Render();
		//ポストエフェクトをテクスチャとしてセットする
		virtual void Begin(int slot);
		//後片付け
		virtual void End();
	protected:
		std::shared_ptr<Graphics::RenderTarget> rt;
		Math::Vector2 size;
		int slot;
	};
	//---------------------------------------------------------------------------------------------
	//templateにしよかな
	class StructuredBuffer {
		friend class UnorderedAccessView;
		friend class ReadGPUBuffer;
	public:
		StructuredBuffer(int struct_size, int count);
		~StructuredBuffer() = default;
		//配列サイズ分しっかりと確保されたメモリのポインタ以外はダメ
		void Update(const void* resource);
		void Set(int slot, Graphics::ShaderStageList stage);
		int GetStructSize();
		int GetCount();
		int GetBufferSize();
	private:
		ComPtr<ID3D11Buffer> buffer;
		ComPtr<ID3D11ShaderResourceView> srv;
		const int STRUCT_SIZE;
		const int COUNT;
	};
	//---------------------------------------------------------------------------------------------
	class UnorderedAccessView {
	public:
		UnorderedAccessView(Graphics::Texture* texture);
		UnorderedAccessView(StructuredBuffer* structured_buffer);
		void Set(int slot);
		void Clean(int slot);
	private:
		ComPtr<ID3D11UnorderedAccessView> uav;
	};
	//---------------------------------------------------------------------------------------------
	//STAGINGで作られるバッファ
	//最終的に作り直すときはBufferクラスができる予定なので
	//それを引数対象とする予定だが、プロトタイプなので特定のクラスのみに対応する
	class ReadGPUBuffer {
	public:
		ReadGPUBuffer(std::shared_ptr<StructuredBuffer> buffer);
		~ReadGPUBuffer() = default;
		//ソースのバッファから読める形にコピーする
		void ReadCopy();
		//マップの開始
		void* ReadBegin();
		template<class T> T* ReadBegin() { return r_cast<T*>(ReadBegin()); }
		//アンマップ
		void ReadEnd();
	private:
		ComPtr<ID3D11Buffer> buffer;
		std::weak_ptr<StructuredBuffer> origin;
	};
	//---------------------------------------------------------------------------------------------
	//ブラーパスは省いています
	//Compute Shaderによる実装
	//パフォーマンス計測結果
	//x1280 / y 720で
	//AO無し 9.2ms/約108FPS
	//AO有り 10.2ms/約98FPS
	//大体1ms
	class SSAOCS :public PostEffect {
		friend class SSAOPS;
	public:
		//TODO : 解像度下げれるようにする
		SSAOCS(const Math::Vector2& size);
		//テクスチャそのまま渡せばいいだけの方式にすれば汎用性は上がる
		//useAOオプションが無効の時はAOマップは作成されない
		void CreateAO(Graphics::RenderTarget* active_rt, Graphics::View* active_view, DeferredBuffer* deferred_buffer);
		//AOを直接描画することはないため、デバッグ描画を入れている
		void Render()override;
		void Begin(int slot);
	private:
		ALIGN(16) struct Info {
			float offsetPerPixel;
			int useAO;
			float offsetPerPixelX;
			float offsetPerPixelY;
		};
	private:
		std::unique_ptr<Graphics::View> view;
		std::unique_ptr<Graphics::ComputeShader> cs;
		std::shared_ptr<Graphics::Texture> rwTexture;
		std::unique_ptr<UnorderedAccessView> uav;
		std::unique_ptr<Graphics::ConstantBuffer<Info>> cbuffer;
		Info info;
	};
	//パフォーマンス比較表
	class SSAOPS :public PostEffect {
	public:
		SSAOPS(const Math::Vector2& size);
		void CreateAO(Graphics::RenderTarget* active_rt, DeferredBuffer* deferred_buffer);
		//AOを直接描画することはないため、デバッグ描画を入れている
		void Render()override;
		void Begin(int slot);
	private:
		using Info = SSAOCS::Info;
	private:
		std::shared_ptr<Graphics::PixelShader> ps;
		std::unique_ptr<Graphics::ConstantBuffer<Info>> cbuffer;
		Info info;
	};
	//---------------------------------------------------------------------------------------------
	//ちょっと不具合あるかも (?)
	//Compute Shaderによる実装
	//パフォーマンス計測結果
	//x640 / y 360で
	//何もしない時 6.0ms/約165FPS
	//ぼかし有効化 7.0ms/約138FPS
	//大体1.0ms
	//現状PSのほうが早い。なんでや。
	class GaussianFilterCS :public PostEffect {
		friend class GaussianFilterPS;
	public:
		GaussianFilterCS(const Math::Vector2& size, DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT);
		~GaussianFilterCS() = default;
		//分散率の設定
		void SetDispersion(float dispersion);
		//第三引数が実際にぼかす対象
		void Dispatch(Graphics::View* active_view, Graphics::RenderTarget* active_rt, Graphics::Texture* texture);
		void Dispatch(Graphics::View* active_view, Graphics::RenderTarget* active_rt, std::shared_ptr<Graphics::RenderTarget> rt);
		//XYブラー結果を描画
		void Render()override;
		void Begin(int slot);
		void DebugRender(const Math::Vector2& pos, const Math::Vector2& size);
	private:
		ALIGN(16) struct Info {
			float weight[7];
			float width;
			float height;
		};
	private:
		std::unique_ptr<Graphics::View> view;
		std::unique_ptr<Graphics::ComputeShader> csX;
		std::unique_ptr<Graphics::ComputeShader> csY;
		std::shared_ptr<Graphics::Texture> rwTexturePass1;
		std::shared_ptr<Graphics::Texture> rwTexturePass2;
		std::unique_ptr<UnorderedAccessView> uavPass1;
		std::unique_ptr<UnorderedAccessView> uavPass2;
		std::unique_ptr<Graphics::ConstantBuffer<Info>> cbuffer;
		Info info;
		//分散率
		float dispersion;
	};
	class GaussianFilterPS :public PostEffect {
	public:
		GaussianFilterPS(const Math::Vector2& size, DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT);
		~GaussianFilterPS() = default;
		//分散率の設定
		void SetDispersion(float dispersion);
		//第三引数が実際にぼかす対象
		void Dispatch(Graphics::View* active_view, Graphics::RenderTarget* active_rt, Graphics::Texture* texture);
		void Dispatch(Graphics::View* active_view, Graphics::RenderTarget* active_rt, std::shared_ptr<Graphics::RenderTarget> rt);
		//XYブラー結果を描画
		void Render()override;
		void Begin(int slot);
		void DebugRender(const Math::Vector2& pos, const Math::Vector2& size);
	private:
		using Info = GaussianFilterCS::Info;
	private:
		std::unique_ptr<Graphics::View> view;
		std::shared_ptr<Graphics::VertexShader> vsX;
		std::shared_ptr<Graphics::VertexShader> vsY;
		std::shared_ptr<Graphics::PixelShader> ps;
		std::shared_ptr<Graphics::RenderTarget> pass2;
		std::unique_ptr<Graphics::ConstantBuffer<Info>> cbuffer;
		Info info;
		//分散率
		float dispersion;
	};
	//---------------------------------------------------------------------------------------------
	//
	//	Raycaster
	//
	//---------------------------------------------------------------------------------------------
	//Ray用に作成されるメッシュの形状バッファ
	class RayMesh {
		friend class RayResult;
	public:
		RayMesh(Graphics::Model* model);
		~RayMesh() = default;
		void Set();
		int GetPolygonCount();
	private:
		//GPUへの入力 ポリゴン情報
		struct Input {
			Math::Vector3 pos[3];
		};
	private:
		std::shared_ptr<StructuredBuffer> structuredBuffer;
		int polygonCount;
	};
	//---------------------------------------------------------------------------------------------
	class RayResult {
	private:
		//GPUからの出力用バッファ
		struct Output {
			int hit;
			float length;
			Math::Vector3 normal;
		};
	public:
		RayResult(RayMesh* mesh);
		~RayResult() = default;
		void Set();
		void Clean();
		const Output* Lock();
		void UnLock();
	private:
		std::shared_ptr<StructuredBuffer> structuredBuffer;
		std::unique_ptr<UnorderedAccessView> uav;
		std::unique_ptr<ReadGPUBuffer> readBuffer;
	};
	//---------------------------------------------------------------------------------------------
	//Rayをうつクラス
	//Singletonでもよいかも？
	class Raycaster {
	public:
		Raycaster();
		~Raycaster() = default;
		//第一引数はワールド変換行列
		void Dispatch(const DirectX::XMMATRIX& world, RayMesh* mesh, RayResult* result, const Math::Vector3& begin, const Math::Vector3& end);
	private:
		//コンスタントバッファ
		struct Info {
			Math::Vector4 rayBegin;
			Math::Vector4 rayEnd;
			DirectX::XMFLOAT4X4 world;
			DirectX::XMFLOAT4X4 worldInverse;
		};
	private:
		std::unique_ptr<Graphics::ComputeShader> cs;
		std::unique_ptr<Graphics::ConstantBuffer<Info>> cbuffer;
		Info info;
	};
	//---------------------------------------------------------------------------------------------
	//
	//		Camera
	//
	//---------------------------------------------------------------------------------------------
	class Camera {
	public:
		Camera(const Math::Vector2& scale, const Math::Vector3& pos, const Math::Vector3& at);
		virtual ~Camera() = default;
		void SetPos(const Math::Vector3& pos);
		void SetTarget(const Math::Vector3& at);
		std::shared_ptr<Graphics::View> GetView();
		Math::Vector3 TakeFront();
		Math::Vector3 TakeRight();
		Math::Vector3 TakeUp();
		virtual void Update() {}
		void Activate();
	protected:
		std::shared_ptr<Graphics::View> view;
		Math::Vector3 pos;
		Math::Vector3 at;
		Math::Vector3 up;
	};
	//デバッグ用なので実装めちゃくちゃ
	class DebugCamera :public Camera {
	public:
		DebugCamera(const Math::Vector2& scale, const Math::Vector3& pos, const Math::Vector3& at);
		void Update();
	private:
		float radius;
		Math::Vector3 front;
		Math::Vector3 right;
	};
	//---------------------------------------------------------------------------------------------
	//
	//		Scene
	//
	//---------------------------------------------------------------------------------------------
	class SceneDeferred :public Scene {
	public:
		SceneDeferred() = default;
		~SceneDeferred();
		void Initialize()override;
		void AlwaysUpdate()override;
		void AlwaysRender()override;
	private:
		std::unique_ptr<Camera> camera;
		//std::unique_ptr<Graphics::View> view;
		std::unique_ptr<DeferredBuffer> deferredBuffer;
#ifdef SIMPLE_SHADER
		std::unique_ptr<DeferredShader> deferredShader;
#endif
#ifdef FULL_EFFECT
		std::unique_ptr<PointLightDeferred> deferredShader;
#endif
#ifdef USE_SSAO
#ifdef SSAO_PS
		std::unique_ptr<SSAOPS> ssao;
#endif
#ifdef SSAO_CS
		std::unique_ptr<SSAOCS> ssao;
#endif
#endif
		//std::unique_ptr<GaussianFilterCS> gaussian;
		std::unique_ptr<ShadowBuffer> shadow;
		//Math::Vector3 pos;
		//Math::Vector3 at;
		//Math::Vector3 up;
		//描画オブジェクト
		std::shared_ptr<Graphics::Model> model;
		std::unique_ptr<SkyBox> skybox;
		//レイ判定用オブジェクト
		std::unique_ptr<Raycaster> raycaster;
		std::unique_ptr<RayMesh> rayMesh;
		std::unique_ptr<RayResult> rayResult;
		//描画制御用パラメーター
		int normalMap;
		int useLight;
		int useFog;
	};
}