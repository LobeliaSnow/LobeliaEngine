#pragma once
namespace Lobelia::Graphics {
	using Microsoft::WRL::ComPtr;
	class Sprite {
	private:
		struct Vertex {
			Math::Vector4 pos;	//x,y,z
			Math::Vector4 col;
			Math::Vector2 tex;	//u,v(x,y)
		};
	private:
		Texture* texture;
	public:
		Sprite(const char* file_path);
		~Sprite();
		Texture* GetTexture();
		void Render(const Transform2D& transform, const Math::Vector2& uv_pos, const Math::Vector2& uv_size, Utility::Color color);
		void Render(const Math::Vector2& pos, const Math::Vector2& size, float rad, const Math::Vector2& uv_begin, const Math::Vector2& uv_end, Utility::Color color);
		void Render(Utility::Color color = 0xFFFFFFFF);
	};
	class SpriteBatch :public RenderableObject<SpriteBatch> {
	private:
		struct Vertex {
			Math::Vector4 pos;	//x,y,z
			Math::Vector2 tex;	//u,v(x,y)
		};
		struct Instance {
			DirectX::XMMATRIX ndc;
			Math::Vector4 col;	//r,g,b,a(x,y,z,w)
			Math::Vector4 uvTrans;
		};
	private:
		const int RENDER_LIMIT;
	private:
		std::unique_ptr<Mesh<Vertex>> mesh;
		Texture* texture;
		ComPtr<ID3D11Buffer> instanceBuffer;
		Instance* instances;
		int renderCount;
		InstanceID id;
	public:
		SpriteBatch(const char* file_path, int render_limit);
		~SpriteBatch();
		Texture* GetTexture();
		void Begin();
		void Set(const Transform2D& transform, const Math::Vector2& upos, const Math::Vector2& usize, /*float rotate_axis_x, float rotate_axis_y,*/ Utility::Color color);
		void Set(const Math::Vector2& pos, const Math::Vector2& size, float rad, const Math::Vector2& uv_begin, const Math::Vector2& uv_size, Utility::Color color);
		void End();
		void Render();
	};
}