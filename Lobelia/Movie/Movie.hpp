#pragma once
//�K���Ɏ����ł��邩����
namespace Lobelia::Movie {
	class MovieSystem {
	public:
		static void Bootup();
		static void Shutdown();
	};
	class Loader {
	private:
		ComPtr<IMFMediaType> iyuv;
		ComPtr<IMFMediaType> argb32;
		ComPtr<IMFSourceReader> reader;
		ComPtr<IMFTransform> mft;
		std::unique_ptr<Graphics::Texture> texture;

	public:
		Loader(const char* file_path);
		void Update();
		Graphics::Texture* GetTexture() { return texture.get(); }
	};
}