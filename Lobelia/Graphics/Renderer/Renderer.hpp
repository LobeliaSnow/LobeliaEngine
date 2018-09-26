#pragma once
namespace Lobelia::Graphics {
	class RenderTarget;
	class SpriteRenderer :public RenderableObject<SpriteRenderer> {
	private:
		struct Vertex {
			Math::Vector4 pos;	//x,y,z
			Math::Vector4 col;
			Math::Vector2 tex;	//u,v(x,y)
		};
	private:
		static std::unique_ptr<Mesh<Vertex>> mesh;
		static Math::Vector2 vertex[4];
		static InstanceID id;
	public:
		static Math::Vector4 Trans2DPosition(Math::Vector2 pos);
		static void PositionPlant(const Transform2D& transform);
		static void PositionRotation(const Transform2D& transform);
		static void SetMeshColor(Utility::Color color);
		static void CutUV(const Math::Vector2& tex, const Math::Vector2& uv_pos, const Math::Vector2& uv_size);
		static void MeshTransform();
	public:
		static void Initialize();
		static void Render(Texture* tex, const Transform2D& transform, const Math::Vector2& uv_pos, const Math::Vector2& uv_size, Utility::Color color);
		static void Render(Texture* tex, const Math::Vector2& pos, const Math::Vector2& size, float rad, const Math::Vector2& uv_begin, const Math::Vector2& uv_size, Utility::Color color);
		static void Render(Texture* tex, Utility::Color color = 0xFFFFFFFF);
		static void Render(RenderTarget* rt, const Transform2D& transform, const Math::Vector2& uv_pos, const Math::Vector2& uv_size, Utility::Color color);
		static void Render(RenderTarget* rt, const Math::Vector2& pos, const Math::Vector2& size, float rad, const Math::Vector2& uv_begin, const Math::Vector2& uv_size, Utility::Color color);
		static void Render(RenderTarget* rt, Utility::Color color = 0xFFFFFFFF);
		//内部でシェーダーのセットは無し、ステート等と頂点データをIAに送ってドローコールが呼ばれるのみ
		static void CustumeRender(const Math::Vector2& pos, const Math::Vector2& size, float rad, const Math::Vector2& uv_begin, const Math::Vector2& uv_size, const Math::Vector2& texture_size,Utility::Color color = 0xFFFFFFFF,bool state = true);
	};
	class SpriteBatchRenderer :public RenderableObject<SpriteBatchRenderer>, public Utility::Singleton<SpriteBatchRenderer> {
	private:
		struct Vertex {
			Math::Vector4 pos;	//x,y,z
			Math::Vector2 tex;	//u,v(x,y)
		};
		struct Instance {
			DirectX::XMMATRIX ndc;
			Math::Vector4 col;	//r,g,b,a(x,y,z,w)
			Math::Vector4 uvTrans;
			int textureSlot;
		};
	private:
		static constexpr UINT TEXTURE_COUNT = 8;
	private:
		const int RENDER_LIMIT;
		std::array<Texture*, TEXTURE_COUNT> textures;
		std::unique_ptr<Mesh<Vertex>> mesh;
		ComPtr<ID3D11Buffer> instanceBuffer;
		Instance* instances;
		int renderCount;
	public:
		SpriteBatchRenderer(int render_limit = 10240);
		~SpriteBatchRenderer() = default;
		const Texture* GetTexture(int index);
		//slotは8個まで
		void SetTexture(int slot, Texture* texture);
		void Begin();
		void Set(int slot, const Transform2D& transform, const Math::Vector2& uv_begin, const Math::Vector2& uv_size, Utility::Color color);
		void Set(int slot, const Math::Vector2& pos, const Math::Vector2& size, float rad, const Math::Vector2& uv_begin, const Math::Vector2& uv_size, Utility::Color color);
		void Set(int slot, Utility::Color color = 0xFFFFFFFF);
		void End();
		void Render();
	};
	enum class Topology {
		POINT_LIST = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST,
		LINE_STRIP = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP,
		LINE_LIST = D3D11_PRIMITIVE_TOPOLOGY_LINELIST,
		TRI_STRIP = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
		TRI_LIST = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
	};
	//TODO : DrawIndexに対応と、2D版、ビルボード用等用意したい
	class Polygon3DRenderer :public RenderableObject<Polygon3DRenderer> {
	public:
		struct Vertex {
			Math::Vector4 pos;
			Math::Vector4 normal;
			Math::Vector2 tex;
			Math::Vector4 color;
		};
	public:
		Polygon3DRenderer(int vertex_max);
		~Polygon3DRenderer();
		void ChangeTexture(Graphics::Texture* texture);
		void LoadTexture(const char* file_path);
		void SetTransformAndCalcMatrix(const Transform3D& transform); // 追加!!
		Vertex* Begin();
		void End();
		void Render(int render_count, Topology topology = Topology::TRI_LIST);
	protected:
		std::unique_ptr<Mesh<Vertex>> mesh;
		Texture* texture;
		DirectX::XMMATRIX world;
		std::unique_ptr<ConstantBuffer<DirectX::XMMATRIX>> constantBuffer;
		Transformer transform;
	};
	class Polygon2DRenderer :public RenderableObject<Polygon2DRenderer> {
	public:
		struct Vertex {
			Math::Vector4 pos;
			Math::Vector4 color;
			Math::Vector2 tex;
		};
	public:
		Polygon2DRenderer(int vertex_max);
		~Polygon2DRenderer();
		void ChangeTexture(Graphics::Texture* texture);
		void LoadTexture(const char* file_path);
		Vertex* Begin();
		void End();
		void Render(int render_count, Topology topology = Topology::TRI_LIST);
	private:
		std::unique_ptr<Mesh<Vertex>> mesh;
		Texture* texture;
	};
	//現在球のみ実装
	//後、最低限カプセルと立方体はつけたい
	class Primitive3D {
	public:
		struct Sphere {
			std::vector<Math::Vector4> pos;
			std::vector<Math::Vector4> normal;
			int vertexCount = 0;
		};
	public:
		//重いです
		//ここ参照
		//http://marina.sys.wakayama-u.ac.jp/~tokoi/?date=20090912
		static	void CreateSphere(Sphere* sphere, int division);
	public:
		Primitive3D() = delete;
		~Primitive3D() = delete;
	};
	//Debug時のみ中の関数があります
	//Release時は空の関数となります
	//描画限度個数を引数でとり、セーフティを付けるようにする
	//あくまでもデバッグ用なのでパフォーマンスは良くないです。
	class DebugRenderer :public Utility::Singleton<DebugRenderer> {
		friend class Utility::Singleton<DebugRenderer>;
	public:
		void Initialize();
		void Begin();
		//5120本まで
		void SetLine(const Math::Vector3& p0, const Math::Vector3&p1, Utility::Color color);
		//512個まで
		void SetPoint(const Math::Vector3& p, Utility::Color color);
		//超重いです
		void SetSphere(const Math::Vector3& p, float radius, int division, Utility::Color color);
		//まだマシです
		void SetSphere(const Math::Vector3& p, float radius, const Primitive3D::Sphere& sphere, Utility::Color color);
		void End();
		void Render();
	private:
		DebugRenderer();
		~DebugRenderer() = default;
	private:
#ifdef _DEBUG
		std::unique_ptr<Graphics::Polygon3DRenderer> line;
		Graphics::Polygon3DRenderer::Vertex* verticesLine;
		int lineCount;
		std::unique_ptr<Graphics::Polygon3DRenderer> point;
		Graphics::Polygon3DRenderer::Vertex* verticesPoint;
		int pointCount;
		std::unique_ptr<Graphics::Polygon3DRenderer> sphere;
		Graphics::Polygon3DRenderer::Vertex* verticeSphere;
		int sphereVertexCount;
#endif
	};
}