#pragma once
namespace Lobelia::Game {
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

}