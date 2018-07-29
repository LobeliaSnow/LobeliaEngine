#pragma once
#include "../DirectXTex/DirectXTex.h"

#ifdef _DEBUG
#pragma comment(lib,"DirectXTexDebug.lib")
#else
#pragma comment(lib,"DirectXTexRelease.lib")
#endif

namespace Lobelia::Graphics {
	class Texture final {
		friend class TextureFileAccessor;
		friend class RenderTarget;
	public:
		enum ACCESS_FLAG {
			DEFAULT = D3D11_USAGE_DEFAULT,
			DYNAMIC = D3D11_USAGE_DYNAMIC,
			IMMUTABLE = D3D11_USAGE_IMMUTABLE,
			STAGING = D3D11_USAGE_STAGING
		};
		enum CPU_ACCESS_FLAG {
			NONE = 0,
			READ = D3D11_CPU_ACCESS_READ,
			WRITE = D3D11_CPU_ACCESS_WRITE,
		};
	private:
		Math::Vector2 size;
		ComPtr<ID3D11ShaderResourceView> view;
		ComPtr<ID3D11Texture2D> texture;
	private:
		//引数要検討
		void CreateTexture(DXGI_FORMAT format, UINT bind_flags, const DXGI_SAMPLE_DESC& sample, ACCESS_FLAG access_flag, CPU_ACCESS_FLAG cpu_flag, int array_count);
		void CreateShaderResourceView(DXGI_FORMAT format);
	public:
		Texture(const Math::Vector2& size, DXGI_FORMAT format, UINT bind_flags, const DXGI_SAMPLE_DESC& sample, ACCESS_FLAG access_flag = ACCESS_FLAG::DEFAULT, CPU_ACCESS_FLAG cpu_flag = CPU_ACCESS_FLAG::NONE, int array_count = 1);
		Texture(const ComPtr<ID3D11Texture2D>& texture);
		~Texture();
		ComPtr<ID3D11Texture2D>& Get();
		const Math::Vector2& GetSize();
		//TODO : セットを用意すること、その際掃除のことも気を付けること
		//↑レンダーターゲットとシェーダーリソースビュー両方同時にセットされてしまう状況になってしまうとまずいのでそれの回避
		void Set(int num_slot, ShaderStageList list);
		static void Clean(int num_slot, ShaderStageList list);
	};
	//未来感じる
	//IWICBitmapDecoder::GetFrame
	//DirectX::ComputeNormalMap
	//DirectX::IsVideo
	class TextureFileAccessor final {
	private:
		enum class Extension {
			NO_SUPPORT = -1,
			PNG,
			JPG,
			TGA,
			BMP,
		};
	private:
		static Extension JudgeExtension(const std::string& file_path);
		static void LoadFile(const wchar_t* file_path, Extension extension, DirectX::TexMetadata* meta, DirectX::ScratchImage& image);
	public:
		static void Load(const char* file_path, Texture** texture, bool force = false);
		static void Save(const char* file_path, Texture* texture);
		static void CreateNormalMap(Texture* src, Texture** normal, float amplitude);
	};

}