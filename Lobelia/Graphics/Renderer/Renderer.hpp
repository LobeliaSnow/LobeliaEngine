#pragma once
namespace Lobelia::Graphics {
	class RenderTarget;
	class SpriteRenderer :private RenderableObject<SpriteRenderer> {
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
	private:
		static Math::Vector4 Trans2DPosition(Math::Vector2 pos);
		static void PositionPlant(const Transform2D& transform);
		static void PositionRotation(const Transform2D& transform);
		static void SetMeshColor(Utility::Color color);
		static void CutUV(Texture* tex, const Math::Vector2& uv_pos, const Math::Vector2& uv_size);
		static void MeshTransform();
	public:
		static void Initialize();
		static void Render(Texture* tex, const Transform2D& transform, const Math::Vector2& uv_pos, const Math::Vector2& uv_size, Utility::Color color);
		static void Render(Texture* tex, const Math::Vector2& pos, const Math::Vector2& size, float rad, const Math::Vector2& uv_begin, const Math::Vector2& uv_end, Utility::Color color);
		static void Render(Texture* tex, Utility::Color color = 0xFFFFFFFF);
		static void Render(RenderTarget* rt, const Transform2D& transform, const Math::Vector2& uv_pos, const Math::Vector2& uv_size, Utility::Color color);
		static void Render(RenderTarget* rt, const Math::Vector2& pos, const Math::Vector2& size, float rad, const Math::Vector2& uv_begin, const Math::Vector2& uv_end, Utility::Color color);
		static void Render(RenderTarget* rt, Utility::Color color = 0xFFFFFFFF);
	};
	class SpriteBatchRenderer :private RenderableObject<SpriteBatchRenderer>, public Utility::Singleton<SpriteBatchRenderer> {
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
		SpriteBatchRenderer(int render_limit = 1024);
		//slot‚Í8ŒÂ‚Ü‚Å
		void SetTexture(int slot, Texture* texture);
		void Begin();
		void Set(int slot, const Transform2D& transform, const Math::Vector2& upos, const Math::Vector2& usize, Utility::Color color);
		void Set(int slot, const Math::Vector2& pos, const Math::Vector2& size, float rad, const Math::Vector2& uv_begin, const Math::Vector2& uv_end, Utility::Color color);
		void Set(int slot, Utility::Color color = 0xFFFFFFFF);
		void End();
		void Render();
	};
}