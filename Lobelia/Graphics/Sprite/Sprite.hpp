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
		void Render(const Transform2D& transform, const Math::Vector2& uv_pos, const Math::Vector2& uv_size, Utility::Color color, bool set_default_pipeline = true);
		void Render(const Math::Vector2& pos, const Math::Vector2& size, float rad, const Math::Vector2& uv_begin, const Math::Vector2& uv_end, Utility::Color color, bool default = true);
		void Render(Utility::Color color = 0xFFFFFFFF, bool set_default_pipeline = true);
	};
	class SpriteBatch {
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
		std::unique_ptr<Material> material;
		std::unique_ptr<InputLayout> inputLayout;
		ComPtr<ID3D11Buffer> instanceBuffer;
		Instance* instances;
		int renderCount;
	public:
		//îÒêÑèß å√Ç¢ã@î\Ç≈Ç∑
		[[deprecated("please use new SpriteBatchRenderer class")]]SpriteBatch(const char* file_path, int render_limit);
		~SpriteBatch();
		Material* GetMaterial();
		void BeginRender();
		void Render(const Transform2D& transform, const Math::Vector2& upos, const Math::Vector2& usize, float rotate_axis_x, float rotate_axis_y, Utility::Color color);
		void EndRender(bool set_default_pipeline = true);
	};
}