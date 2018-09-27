#pragma once
#include "../D3DCompiler/d3dcompiler.h"
namespace Lobelia::Graphics {
	class ShaderCompiler {
	public:
		//BYTE* bufferにはnew BYTE[*size]された値が入るので、deleteを忘れないように
		//新しく書き直して以来、テストしていないので信頼度も少し低い。
		[[deprecated("ReadCSO is deprecated.We recommend using a CompileShaderFromFile.")]]static void ReadCSO(const char* file_path, int* size, BYTE* buffer);
		static void CompileShaderFromFile(const char* file_path, const char* entry_point, const char* shader_model, ID3DBlob** blob);
		//TODO : FromMemoryも作ること。
	};
	using InstanceID = UINT;
	class ShaderLinkageInstance final {
		friend class ShaderLinkage;
	private:
		std::string name;
		ComPtr<ID3D11ClassInstance> instance;
	public:
		ShaderLinkageInstance();
		~ShaderLinkageInstance();
		ComPtr<ID3D11ClassInstance>& Get();
		const std::string& GetName();
	};
	class ShaderLinkage final {
		friend class VertexShader;
		friend class PixelShader;
	private:
		ComPtr<ID3D11ClassLinkage> classLinkage;
		int instanceCount;
	public:
		std::vector<std::unique_ptr<ShaderLinkageInstance>> instances;
	public:
		ShaderLinkage();
		~ShaderLinkage();
		//リフレクション等は後程
		//クラス内にデータメンバーが存在しない際に呼び出す
		//色々調整が必要となりそう
		InstanceID CreateInstance(const char* instance_name);
		//クラス内にデータメンバーが存在する際に呼び出す
		InstanceID GetInstance(const char* instance_name, int instance_index);
	};

	class Shader {
		friend class Reflection;
	protected:
		ComPtr<ID3DBlob> blob;
		std::unique_ptr<ShaderLinkage> linkage;
		int instanceCount;
		std::vector<ID3D11ClassInstance*> instances;
	public:
		Shader(const char* file_path, const char* entry_point, const char* shader_model, ShaderLinkage* linkage);
		~Shader();
		ShaderLinkage* GetLinkage();
		template<class... Args> void SetLinkage(Args&&... args) {
			instanceCount = 0;
			instances.clear();
			using swallow = std::initializer_list<int>;
			(void)swallow {
				(instances.push_back(linkage->instances[args]->Get().Get()), instanceCount++)...
			};
		}
	};

	class VertexShader :public Shader {
		friend class InputLayout;
	public:
		enum Model { VS_2_0, VS_3_0, VS_4_0, VS_4_1, VS_5_0, VS_5_1, };
	public:
		VertexShader(const char* file_path, const char* entry_point, Model shader_model, bool use_linkage = false);
		~VertexShader();
		void Set();
	private:
		//シェーダーモデルのパース。
		std::string ConverteShaderModelString(Model shader_model);
	private:
		ComPtr<ID3D11VertexShader> vs;
	};
	class PixelShader :public Shader {
	public:
		enum Model { PS_2_0, PS_3_0, PS_4_0, PS_4_1, PS_5_0, PS_5_1, };
	public:
		PixelShader(const char* file_path, const char* entry_point, Model shader_model, bool use_linkage = false);
		~PixelShader();
		void Set();
	private:
		//シェーダーモデルのパース。
		std::string ConverteShaderModelString(Model shader_model);
	private:
		ComPtr<ID3D11PixelShader> ps;
	};
	class GeometryShader :public Shader {
	public:
		GeometryShader(const char* file_path, const char* entry_point);
		~GeometryShader();
		void Set();
		static void Clean();
	private:
		ComPtr<ID3D11GeometryShader> gs;
	};
	class HullShader :public Shader {
	public:
		HullShader(const char* file_path, const char* entry_point);
		~HullShader();
		void Set();
		static void Clean();
	private:
		ComPtr<ID3D11HullShader> hs;
	};
	class DomainShader :public Shader {
	public:
		DomainShader(const char* file_path, const char* entry_point);
		~DomainShader();
		void Set();
		static void Clean();
	private:
		ComPtr<ID3D11DomainShader> ds;

	};
	namespace _ {
		//ComputeShaderで使う入力用バッファ
		//TODO : 引数追加
		class StructuredBuffer {
			friend class ComputeShader;
		private:
			//StructuredBuffer
			ComPtr<ID3D11Buffer> buffer;
			ComPtr<ID3D11ShaderResourceView> srv;
			const size_t ELEMENT_SIZE;
			const size_t ELEMENT_COUNT;
		private:
			void CreateStructuredBuffer(void* data_buffer, size_t element_size, size_t element_count);
			void CreateShaderResourceView();
		public:
			//第一引数 入力用バッファ初期値 第二引数 入力用バッファの一要素のサイズ 第三引数  要素数
			StructuredBuffer(void* data_buffer, size_t element_size, size_t element_count);
			~StructuredBuffer();
			void* Map();
			void Unmap();
			//内部でMapUnmapが行われます
			void DataSet(void* data_buffer, size_t element_count);
			ComPtr<ID3D11Buffer>& GetBuffer();
		};
		//ComputeShaderで使う出力用バッファ
		//UnorderdAccessView
		class UnorderedAccessBuffer {
			friend class ComputeShader;
		public:
			enum Bind {
				VERTEX_BUFFER = D3D11_BIND_VERTEX_BUFFER,
				SHADER_RESOURCE = D3D11_BIND_SHADER_RESOURCE,
				UNORDERED_ACCESS = D3D11_BIND_UNORDERED_ACCESS
			};
			enum Type {
				STRUCTURED = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED,
				RAW_VIEW = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS
			};
			enum CPUAccess {
				NO = 0,
				WRITE = D3D11_CPU_ACCESS_WRITE,
				READ = D3D11_CPU_ACCESS_READ
			};
			enum UAVFlag {
				RAW = D3D11_BUFFER_UAV_FLAG_RAW,
				APPEND = D3D11_BUFFER_UAV_FLAG_APPEND,
				COUNTER = D3D11_BUFFER_UAV_FLAG_COUNTER
			};
		private:
			ComPtr<ID3D11Buffer> buffer;
			ComPtr<ID3D11Buffer> readBuffer;
			ComPtr<ID3D11UnorderedAccessView> uav;
			DWORD cpuFlags;
		private:
			void CreateStructuredBuffer(size_t element_size, size_t element_count, DWORD bind_flags, Type type, DWORD cpu_flags);
			void CreateUnorderdAccessView(UAVFlag uav_flag);
			void BufferCopy();
		public:
			UnorderedAccessBuffer(size_t element_size, size_t element_count, DWORD bind_flags, Type type, DWORD cpu_flags, UAVFlag uav_flag);
			~UnorderedAccessBuffer();
			//GPUでの計算結果入手用
			//現状書き込みには使わない予定(class名的にも)
			void* Map();
			void Unmap();
		};
		//virtual tableが怖いのと描画用シェーダーじゃないのでこのクラスはShaderを継承しません
		class ComputeShader {
		private:
			ComPtr<ID3DBlob> blob;
			ComPtr<ID3D11ComputeShader> cs;
		public:
			ComputeShader(const char* file_path, const char* entry_point);
			~ComputeShader();
			//CSステージにバインドするので、インスタンスに関係しません
			static void SetInputBuffers(int start_slot, int count, StructuredBuffer** buffers);
			static void SetOutputBuffers(int start_slot, int count, UnorderedAccessBuffer** buffers);
			static void ClearInputBuffers(int start_slot, int count);
			static void ClearOutputBuffers(int start_slot, int count);
			void Run(UINT x, UINT y, UINT z);
		};
	}
	class ComputeShader :public Shader {
	private:
		ComPtr<ID3D11ComputeShader> cs;
	public:
		ComputeShader(const char* file_path, const char* entry_point);
		~ComputeShader();
		static void SetShaderResourceView(int slot, ID3D11ShaderResourceView* uav);
		static void SetShaderResourceView(int slot, int sum, ID3D11ShaderResourceView** srvs);
		static void SetUnorderedAccessView(int slot, ID3D11UnorderedAccessView* uav);
		static void SetUnorderedAccessView(int slot, int sum, ID3D11UnorderedAccessView** uavs);
		void Run(int thread_x, int thread_y, int thread_z);
	};
}