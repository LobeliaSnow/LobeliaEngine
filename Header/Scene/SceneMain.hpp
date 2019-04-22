#pragma once
namespace Lobelia::Game {
	//研究スペース
	//やろうとしていること
	//全レンダラ共通のシェーダーを用いることで共通のマテリアルを使いまわせるようにする事
	//パフォーマンスも大事だが、とりあえず使いやすさ、汎用性を求めることとする
	//共通のパラメータを渡すリンケージインスタンスを作れば、その前後の頂点変換、シャドウキャストには影響しない
	//これを用いて、各レンダラごとに固有のシェーダー＋マテリアルによるインスタンス指定で表現
	//DynamicShaderLinkageによる表現
	//namespace Develop {
	//	class Material {
	//	public:
	//		//リンケージのインスタンスパスと、名前を持つ
	//		Material(const char* file_path, const char* instance_name);

	//	};
	//	//レンダラの抽象クラス
	//	class Renderer {
	//	public:
	//		Renderer();
	//		void SetMaterial(std::shared_ptr<Material>& material);
	//		std::shared_ptr<Material>& GetMaterial();
	//		//各レンダラごとに代わる
	//		virtual void Render() = 0;
	//	protected:
	//		//シェーダー周り
	//		//ここはレンダラごとに設定され、外部から変更は想定されていない
	//		void SetVertexShader(std::shared_ptr<Graphics::VertexShader>& vs);
	//		std::shared_ptr<Graphics::VertexShader>&  GetVertexShader();
	//		void SetPixelShader(std::shared_ptr<Graphics::PixelShader>& ps);
	//		std::shared_ptr<Graphics::PixelShader>&  GetPixelShader();
	//	private:
	//		std::shared_ptr<Material> material;
	//		std::shared_ptr<Graphics::VertexShader> vs;
	//		std::shared_ptr<Graphics::PixelShader> ps;
	//	};
	//	//全てをスプライトとして描画する
	//	class SpriteRenderer {

	//	};
	//	//頂点バッファを持つ
	//	//いわゆるデータ部
	//	class Mesh {

	//	};
	//	class Model {

	//	};
	//	//トポロジや、ラスタライザ等を持つ
	//	class MeshFilter {
	//		std::shared_ptr<Mesh> mesh;
	//		D3D11_PRIMITIVE_TOPOLOGY topology;
	//		std::shared_ptr<Graphics::RasterizerState> rs;
	//	};
	//	//文字列からシェーダー生成する予定
	//	//(仮)
	//	class ShaderBufferObject {
	//	public:
	//		std::shared_ptr<Graphics::VertexShader> CreateVertexShader(const char* include, const char* vs);
	//		std::shared_ptr<Graphics::PixelShader> CreatePixelShader(const char* include, const char* ps);
	//	};
	//	//メッシュ用のレンダラ
	//	class MeshRenderer :public Renderer {
	//	public:
	//		MeshRenderer();
	//		void SetFilter(std::shared_ptr<MeshFilter> mesh);
	//		void Render()override;
	//	private:
	//	};
	//	//アニメーション用のレンダラ
	//	class SkinMeshRenderer :public Renderer {
	//	public:
	//		SkinMeshRenderer();
	//		void Render()override;
	//	};
	//}

#include <dxgi1_6.h>
	class SceneMain :public Lobelia::Scene {
	private:
		std::unique_ptr<Graphics::View> view;
		std::unique_ptr<Graphics::Font> font;
		std::shared_ptr<Graphics::RenderTarget> d3d11Texture;
		Microsoft::WRL::ComPtr<IDXGIOutputDuplication> outputDuplication;
		IDXGIOutput1* output;
	public:
		SceneMain();
		~SceneMain();
		void Initialize()override;
		void AlwaysUpdate()override;
		void AlwaysRender()override;
	};
}