#pragma once
//デモ用なので1つのソースファイルにまとめている
namespace Lobelia::Game {
	ALIGN(16) struct CubeInfo {
		DirectX::XMFLOAT4X4 views[6];
		DirectX::XMFLOAT4X4 projection;
		//ライティングするか否か if文使用するが、最適化でパフォーマンスは変わらない
		int isLighting;
	};
	class CubeEnvironmentMap {
	public:
		CubeEnvironmentMap(const Math::Vector2& size, const Math::Vector3& pos, float radius);
		~CubeEnvironmentMap() = default;
		Math::Vector3 GetPos();
		float GetRadius();
		Math::Vector2 GetTextureSize();
		void SetPos(const Math::Vector3& pos);
		void SetRadius(float radius);
		void UpdateInfo();
		void Clear(Utility::Color color = 0x00000000);
		std::shared_ptr<Graphics::RenderTarget> GetRenderTarget();
		CubeInfo& GetCubeInfo();
		void Activate();
	private:
		//位置
		Math::Vector3 pos;
		//影響範囲
		float radius;
		//テクスチャサイズ
		Math::Vector2 size;
		//立方体6面
		std::unique_ptr<Graphics::View> views[6];
		std::shared_ptr<Graphics::RenderTarget> rt;
		//コンスタントバッファ用情報
		CubeInfo info;
	};
	//そのうちテンプレートにする。現状はほぼパラボロイドのコピペ
	class CubeEnvironmentMapManager {
	public:
		CubeEnvironmentMapManager();
		~CubeEnvironmentMapManager();
		//毎フレームセットが必要
		void AddModelList(std::weak_ptr<Graphics::Model> model, bool lighting);
		//環境マップへの書き込み
		void RenderEnvironmentMap();
		//戻り値を受けなければ内部でもクリーンされる
		//受取先が生存している間のみ内部で生存する
		std::shared_ptr<CubeEnvironmentMap> CreateEnvironmentMap(const Math::Vector2& size, const Math::Vector3& pos, float radius);
		//一番近い影響下にある環境マップをテクスチャスロット4にセットする
		void Activate(const Math::Vector3& pos);
	private:
		void ClearModelList();
	private:
		std::list<std::weak_ptr<CubeEnvironmentMap>> environmentMaps;
		//シェーダー
		std::shared_ptr<Graphics::VertexShader> vs;
		std::shared_ptr<Graphics::GeometryShader> gs;
		std::shared_ptr<Graphics::PixelShader> ps;
		//コンスタントバッファ
		std::unique_ptr<Graphics::ConstantBuffer<CubeInfo>> constantBuffer;
		//モデルのリスト(ライティングフラグとのペア)
		std::list<std::pair<std::weak_ptr<Graphics::Model>, bool>> models;
	};
	//海操作用
	ALIGN(16) struct SeaInfo {
		float min;
		float max;
		float maxDevide;
		float height;
		float time;
		float reflectiveRatio;
	};
	class WaterShader {
	public:
		WaterShader();
		void LoadDisplacementMap(const char* file_path);
		void LoadNormalMap(const char* file_path);
		void Activate(std::shared_ptr<Graphics::Model> model);
		void Clean();
	private:
		std::shared_ptr<Graphics::VertexShader> vs;
		std::shared_ptr<Graphics::HullShader> hs;
		std::shared_ptr<Graphics::DomainShader> ds;
		std::shared_ptr<Graphics::GeometryShader> gs;
		std::shared_ptr<Graphics::PixelShader> ps;
		SeaInfo seaInfo;
		std::unique_ptr<Graphics::ConstantBuffer<SeaInfo>> constantBufferSeaInfo;
		//ディスプレースメントマップと称したハイトマップ
		Graphics::Texture* displacement;
		Graphics::Texture* normal;
	};

	class SceneSea :public Lobelia::Scene {
	public:
		SceneSea();
		~SceneSea();
		void Initialize()override;
		void AlwaysUpdate()override;
		void AlwaysRender()override;
	private:
#ifdef __PARABOLOID__
		std::unique_ptr<DualParaboloidMapManager> environmentManager;
		std::shared_ptr<DualParaboloidMap> paraboloidMap;
#else
		std::unique_ptr<CubeEnvironmentMapManager> environmentManager;
		std::shared_ptr<CubeEnvironmentMap> cubeMap;
#endif
		std::unique_ptr<Graphics::View> view;
		Math::Vector3 pos;
		Math::Vector3 at;
		Math::Vector3 up;
		std::shared_ptr<Graphics::Model> model;
		std::shared_ptr<Graphics::Model> stage;
		std::shared_ptr<Graphics::Model> skyBox;
		std::unique_ptr<WaterShader> waterShader;
		std::shared_ptr<Graphics::RasterizerState> wireState;
		std::shared_ptr<Graphics::RasterizerState> solidState;
		std::shared_ptr<Graphics::VertexShader> vs;
		std::shared_ptr<Graphics::PixelShader> ps;
		//フラグ
		int wire;
		int sea;
		D3D11_PRIMITIVE_TOPOLOGY topology;

		Math::Vector3 seaPos;
		Graphics::Texture* caustics;
	};
}