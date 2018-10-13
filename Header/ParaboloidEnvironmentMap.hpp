#pragma once
namespace Lobelia::Game {
	ALIGN(16) struct ParaboloidInfo {
		DirectX::XMFLOAT4X4 views[2];
		DirectX::XMFLOAT4X4 projection;
		float zNear;
		float zFar;
		int lighting;
	};
	class DualParaboloidMap {
	public:
		DualParaboloidMap(const Math::Vector2& size, const Math::Vector3& pos, float radius);
		~DualParaboloidMap() = default;
		Math::Vector3 GetPos();
		float GetRadius();
		Math::Vector2 GetTextureSize();
		void SetPos(const Math::Vector3& pos);
		void SetRadius(float radius);
		void UpdateInfo();
		void Clear(Utility::Color color = 0x00000000);
		std::shared_ptr<Graphics::RenderTarget> GetRenderTarget();
		ParaboloidInfo& GetParaboloidInfo();
		void Activate();
	private:
		//位置
		Math::Vector3 pos;
		//影響範囲
		float radius;
		//テクスチャサイズ
		Math::Vector2 size;
		//二方向を見るビュー
		std::unique_ptr<Graphics::View> views[2];
		//デュアルパラボイドマップレンダリング用(中で配列として要素数2で作成)
		std::shared_ptr<Graphics::RenderTarget> rt;
		//コンスタントバッファ用情報
		ParaboloidInfo info;
	};
	//双曲面環境マップのマネージャー
	class DualParaboloidMapManager {
	public:
		DualParaboloidMapManager();
		~DualParaboloidMapManager();
		//毎フレームセットが必要
		void AddModelList(std::weak_ptr<Graphics::Model> model, bool lighting);
		//環境マップへの書き込み
		void RenderEnvironmentMap();
		//戻り値を受けなければ内部でもクリーンされる
		//受取先が生存している間のみ内部で生存する
		std::shared_ptr<DualParaboloidMap> CreateEnvironmentMap(const Math::Vector2& size, const Math::Vector3& pos, float radius);
		//一番近い影響下にある環境マップをテクスチャスロット4にセットする
		void Activate(const Math::Vector3& pos);
	private:
		void ClearModelList();
	private:
		std::list<std::weak_ptr<DualParaboloidMap>> environmentMaps;
		//シェーダー
		std::shared_ptr<Graphics::VertexShader> vs;
		std::shared_ptr<Graphics::HullShader> hs;
		std::shared_ptr<Graphics::DomainShader> ds;
		std::shared_ptr<Graphics::GeometryShader> gs;
		std::shared_ptr<Graphics::PixelShader> ps;
		//コンスタントバッファ
		std::unique_ptr<Graphics::ConstantBuffer<ParaboloidInfo>> constantBuffer;
		//モデルリスト
		std::list<std::pair<std::weak_ptr<Graphics::Model>, bool>> models;
#ifdef _DEBUG
	public:
		//デバッグ用に環境マップを表示
		void DebugRender();
	private:
		ALIGN(16) struct DebugInfo { int index; };
		DebugInfo debugInfo;
		std::unique_ptr<Graphics::ConstantBuffer<DebugInfo>> debugBuffer;
		std::shared_ptr<Graphics::VertexShader> debugVS;
		std::shared_ptr<Graphics::PixelShader> debugPS;
#endif
	};
}