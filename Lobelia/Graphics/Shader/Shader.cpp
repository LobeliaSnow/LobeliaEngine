#include "Common/Common.hpp"
#include "Graphics/GraphicDriverInfo/GraphicDriverInfo.hpp"
#include "Graphics/Device/Device.hpp"
#include "Graphics/Shader/Shader.hpp"
#include "Exception/Exception.hpp"
#include "Graphics/BufferCreator/BufferCreator.h"
namespace Lobelia::Graphics {
	void ShaderCompiler::ReadCSO(const char* file_path, int* size, BYTE* buffer) {
		std::unique_ptr<Utility::FileController> fc = std::make_unique<Utility::FileController>();
		fc->Open(file_path, Utility::FileController::OpenMode::ReadBinary);
		*size = static_cast<int>(fc->GetSize());
		buffer = new BYTE[*size];
		fc->Rewind();
		fc->Read(buffer, *size, *size, 1);
		fc->Close();
	}
	void ShaderCompiler::CompileShaderFromFile(const char* file_path, const char* entry_point, const char* shader_model, ID3DBlob** blob) {
		try {
			HRESULT hr = S_OK;
			ComPtr<ID3DBlob> error = {};
			//これを引数でとるか否かは要調整
			DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
			shaderFlags |= D3DCOMPILE_DEBUG;
#else
			shaderFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif
			hr = D3DCompileFromFile(Utility::ConverteWString(file_path).c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entry_point, shader_model, shaderFlags, 0, blob, &error);
			if (FAILED(hr)) {
				std::stringstream errorMes;
				if (!error)THROW;
				errorMes << "\n//////////////シェーダーコンパイルエラー！//////////////\n\n" << reinterpret_cast<const char*>(error->GetBufferPointer()) << "\n////////////////////////////////////////////////////////\n" << "シェーダーのコンパイルに失敗\n";
				STRICT_THROW(errorMes.str());
			}
		}
		catch (const Exception& exception) {
			exception.BoxMessage();
		}
	}
	ShaderLinkageInstance::ShaderLinkageInstance() = default;
	ShaderLinkageInstance::~ShaderLinkageInstance() = default;
	ComPtr<ID3D11ClassInstance>& ShaderLinkageInstance::Get() { return instance; }
	const std::string& ShaderLinkageInstance::GetName() { return name; }

	ShaderLinkage::ShaderLinkage() :instanceCount(0) { Device::Get()->CreateClassLinkage(classLinkage.GetAddressOf()); }
	ShaderLinkage::~ShaderLinkage() = default;
	InstanceID ShaderLinkage::CreateInstance(const char* instance_name) {
		instances.push_back(std::make_unique<ShaderLinkageInstance>());
		instances[instanceCount]->name = instance_name;
		if (FAILED(classLinkage->CreateClassInstance(instance_name, 0, 0, 0, 0, instances[instanceCount]->instance.GetAddressOf())))STRICT_THROW("インスタンス生成に失敗");
		return instanceCount++;
	}
	InstanceID ShaderLinkage::GetInstance(const char* instance_name, int instance_index) {
		instances.push_back(std::make_unique<ShaderLinkageInstance>());
		instances[instanceCount]->name = instance_name;
		classLinkage->GetClassInstance(instance_name, instance_index, instances[instanceCount]->instance.GetAddressOf());
		return instanceCount++;
	}

	Shader::Shader(const char* file_path, const char* entry_point, const char* shader_model, ShaderLinkage* linkage) :linkage(linkage), instanceCount(0) {
		ShaderCompiler::CompileShaderFromFile(file_path, entry_point, shader_model, blob.GetAddressOf());
	}
	Shader::~Shader() = default;

	ShaderLinkage* Shader::GetLinkage() { return linkage.get(); }
	VertexShader::VertexShader(const char* file_path, const char* entry_point, Model shader_model, bool use_linkage) :Shader(file_path, entry_point, ConverteShaderModelString(shader_model).c_str(), use_linkage ? new ShaderLinkage() : nullptr) {
		HRESULT hr = Device::Get()->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), linkage ? linkage->classLinkage.Get() : nullptr, vs.GetAddressOf());
		if (FAILED(hr))STRICT_THROW("頂点シェーダーの作成に失敗");
	}
	VertexShader::~VertexShader() = default;
	//templateのほうが良いかも。
	std::string VertexShader::ConverteShaderModelString(Model shader_model) {
		switch (shader_model) {
		case Model::VS_2_0:		return "vs_2_0";
		case Model::VS_3_0:		return "vs_3_0";
		case Model::VS_4_0:		return "vs_4_0";
		case Model::VS_4_1:		return "vs_4_1";
		case Model::VS_5_0:		return "vs_5_0";
		case Model::VS_5_1:		return "vs_5_1";
		default:STRICT_THROW("範囲外の値です");
		}
		return "";
	}

	void VertexShader::Set() { Device::GetContext()->VSSetShader(vs.Get(), instances.data(), instanceCount); }

	PixelShader::PixelShader(const char* file_path, const char* entry_point, Model shader_model, bool use_linkage) :Shader(file_path, entry_point, ConverteShaderModelString(shader_model).c_str(), use_linkage ? new ShaderLinkage() : nullptr) {
		HRESULT hr = Device::Get()->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), linkage ? linkage->classLinkage.Get() : nullptr, ps.GetAddressOf());
		if (FAILED(hr))STRICT_THROW("ピクセルシェーダーの作成に失敗");
	}
	PixelShader::~PixelShader() = default;
	std::string PixelShader::ConverteShaderModelString(Model shader_model) {
		switch (shader_model) {
		case Model::PS_2_0:		return "ps_2_0";
		case Model::PS_3_0:		return "ps_3_0";
		case Model::PS_4_0:		return "ps_4_0";
		case Model::PS_4_1:		return "ps_4_1";
		case Model::PS_5_0:		return "ps_5_0";
		case Model::PS_5_1:		return "ps_5_1";
		default:STRICT_THROW("範囲外の値です");
		}
		return "";
	}
	void PixelShader::Set() { Device::GetContext()->PSSetShader(ps.Get(), instances.data(), instanceCount); }
	GeometryShader::GeometryShader(const char* file_path, const char* entry_point) :Shader(file_path, entry_point, "gs_5_0", nullptr) {
		HRESULT hr = Device::Get()->CreateGeometryShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, gs.GetAddressOf());
		if (FAILED(hr))STRICT_THROW("頂点シェーダーの作成に失敗");
	}
	GeometryShader::~GeometryShader() = default;
	void GeometryShader::Set() {
		Device::GetContext()->GSSetShader(gs.Get(), instances.data(), instanceCount);
	}
	void GeometryShader::Clean() {
		Device::GetContext()->GSSetShader(nullptr, nullptr, 0);
	}
	HullShader::HullShader(const char* file_path, const char* entry_point) :Shader(file_path, entry_point, "hs_5_0", nullptr) {
		HRESULT hr = Device::Get()->CreateHullShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, hs.GetAddressOf());
		if (FAILED(hr))STRICT_THROW("頂点シェーダーの作成に失敗");
	}
	HullShader::~HullShader() = default;
	void HullShader::Set() {
		Device::GetContext()->HSSetShader(hs.Get(), instances.data(), instanceCount);
	}
	void HullShader::Clean() {
		Device::GetContext()->HSSetShader(nullptr, nullptr, 0);
	}
	DomainShader::DomainShader(const char* file_path, const char* entry_point) :Shader(file_path, entry_point, "ds_5_0", nullptr) {
		HRESULT hr = Device::Get()->CreateDomainShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, ds.GetAddressOf());
		if (FAILED(hr))STRICT_THROW("頂点シェーダーの作成に失敗");
	}
	DomainShader::~DomainShader() = default;
	void DomainShader::Set() {
		Device::GetContext()->DSSetShader(ds.Get(), instances.data(), instanceCount);
	}
	void DomainShader::Clean() {
		Device::GetContext()->DSSetShader(nullptr, nullptr, 0);

	}
	namespace _ {
		///////////////////////////////////////////////////////////////////////////////////////////////////
		//	CSInputBuffer
		///////////////////////////////////////////////////////////////////////////////////////////////////
		StructuredBuffer::StructuredBuffer(void* data_buffer, size_t element_size, size_t element_count) :ELEMENT_SIZE(element_size), ELEMENT_COUNT(element_count) {
			CreateStructuredBuffer(data_buffer, element_size, element_count);
			CreateShaderResourceView();
		}
		StructuredBuffer::~StructuredBuffer() = default;
		void StructuredBuffer::CreateStructuredBuffer(void* data_buffer, size_t element_size, size_t element_count) {
			BufferCreator::Create(buffer.GetAddressOf(), data_buffer, s_cast<UINT>(element_size * element_count), {}, D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE, D3D11_CPU_ACCESS_WRITE, s_cast<UINT>(element_size), D3D11_RESOURCE_MISC_BUFFER_STRUCTURED);
		}
		void StructuredBuffer::CreateShaderResourceView() {
			D3D11_BUFFER_DESC desc = {};
			buffer->GetDesc(&desc);
			//この下要調査
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
			srvDesc.BufferEx.FirstElement = 0;
			if (desc.MiscFlags&D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS) {
				srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
				srvDesc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
				srvDesc.BufferEx.NumElements = desc.ByteWidth / 4;
			}
			else if (desc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED) {
				srvDesc.Format = DXGI_FORMAT_UNKNOWN;
				srvDesc.BufferEx.NumElements = desc.ByteWidth / desc.StructureByteStride;
			}
			else THROW_E;
			HRESULT hr = S_OK;
			hr = Graphics::Device::Get()->CreateShaderResourceView(buffer.Get(), &srvDesc, srv.GetAddressOf());
			if (FAILED(hr))STRICT_THROW("SRV作成に失敗");
		}
		void* StructuredBuffer::Map() {
			D3D11_MAPPED_SUBRESOURCE resource;
			HRESULT hr = Device::GetContext()->Map(buffer.Get(), 0, D3D11_MAP_WRITE, 0, &resource);
			return resource.pData;
		}
		void StructuredBuffer::Unmap() { Graphics::Device::GetContext()->Unmap(buffer.Get(), 0); }
		void StructuredBuffer::DataSet(void* data_buffer, size_t element_count) {
			void* mapped_data = Map();
			memcpy_s(mapped_data, ELEMENT_SIZE*ELEMENT_COUNT, data_buffer, ELEMENT_SIZE*element_count);
			Unmap();
		}
		///////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////

		///////////////////////////////////////////////////////////////////////////////////////////////////
		//	UnorderedAccessBuffer
		///////////////////////////////////////////////////////////////////////////////////////////////////
		UnorderedAccessBuffer::UnorderedAccessBuffer(size_t element_size, size_t element_count, DWORD bind_flags, Type type, DWORD cpu_flags, UAVFlag uav_flag) :cpuFlags(cpu_flags) {
			CreateStructuredBuffer(element_size, element_count, bind_flags, type, cpu_flags);
			CreateUnorderdAccessView(uav_flag);
		}
		UnorderedAccessBuffer::~UnorderedAccessBuffer() = default;
		void UnorderedAccessBuffer::CreateStructuredBuffer(size_t element_size, size_t element_count, DWORD bind_flags, Type type, DWORD cpu_flags) {
			BufferCreator::Create(buffer.GetAddressOf(), nullptr, s_cast<UINT>(element_size * element_count), {}, s_cast<UINT>(bind_flags), 0, s_cast<UINT>(element_size), s_cast<UINT>(type));
			D3D11_BUFFER_DESC desc = {};
			buffer->GetDesc(&desc);
			//読み込み用バッファの作成 引数要調整
			BufferCreator::Create(readBuffer.GetAddressOf(), nullptr, desc.ByteWidth, D3D11_USAGE_STAGING, 0, cpu_flags, desc.StructureByteStride, D3D11_RESOURCE_MISC_BUFFER_STRUCTURED);
		}
		void UnorderedAccessBuffer::CreateUnorderdAccessView(UAVFlag flag) {
			D3D11_BUFFER_DESC desc = {};
			buffer->GetDesc(&desc);
			D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
			uavDesc.Buffer.FirstElement = 0;
			if (flag == UAVFlag::APPEND) {
				uavDesc.Format = DXGI_FORMAT_UNKNOWN;
				uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;
				uavDesc.Buffer.NumElements = desc.ByteWidth / 4;
			}
			else if (desc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS) {
				uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
				uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
				uavDesc.Buffer.NumElements = desc.ByteWidth / 4;
			}
			else if (desc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED) {
				uavDesc.Format = DXGI_FORMAT_UNKNOWN;
				uavDesc.Buffer.NumElements = desc.ByteWidth / desc.StructureByteStride;
			}
			else THROW_E;
			HRESULT hr = Graphics::Device::Get()->CreateUnorderedAccessView(buffer.Get(), &uavDesc, uav.GetAddressOf());
			if (FAILED(hr))STRICT_THROW("UAV作成に失敗");
		}
		void UnorderedAccessBuffer::BufferCopy() {
			Graphics::Device::GetContext()->CopyResource(readBuffer.Get(), buffer.Get());
		}
		void* UnorderedAccessBuffer::Map() {
			HRESULT hr = S_OK;
			BufferCopy();
			D3D11_MAPPED_SUBRESOURCE resource;
			//TODO : cpuFlagsに基づく結果になるようにする
			hr = Graphics::Device::GetContext()->Map(readBuffer.Get(), 0, D3D11_MAP_READ, 0, &resource);
			if (FAILED(hr))STRICT_THROW("Mapに失敗しました");
			return resource.pData;
		}
		void UnorderedAccessBuffer::Unmap() { Graphics::Device::GetContext()->Unmap(readBuffer.Get(), 0); }
		///////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////

		///////////////////////////////////////////////////////////////////////////////////////////////////
		//	ComputeShader
		///////////////////////////////////////////////////////////////////////////////////////////////////
		//必要なもの
		//ストラクチャードバッファ用のSRVとアンオーダードアクセスビュー、実データ
		ComputeShader::ComputeShader(const char* file_path, const char* entry_point) {
			Graphics::ShaderCompiler::CompileShaderFromFile(file_path, entry_point, "cs_4_0", blob.GetAddressOf());
			HRESULT hr = S_OK;
			hr = Graphics::Device::Get()->CreateComputeShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, cs.GetAddressOf());
			if (FAILED(hr))STRICT_THROW("ComputeShaderの作成に失敗");

		}
		ComputeShader::~ComputeShader() = default;
		void ComputeShader::SetInputBuffers(int start_slot, int count, StructuredBuffer** buffers) {
			ID3D11ShaderResourceView** srvs = new ID3D11ShaderResourceView*[count];
			for (int i = 0; i < count; i++) {
				srvs[i] = buffers[i]->srv.Get();
			}
			Graphics::Device::GetContext()->CSSetShaderResources(start_slot, count, srvs);
			delete[] srvs;
		}
		void ComputeShader::SetOutputBuffers(int start_slot, int count, UnorderedAccessBuffer** buffers) {
			ID3D11UnorderedAccessView** uavs = new ID3D11UnorderedAccessView*[count];
			for (int i = 0; i < count; i++) {
				uavs[i] = buffers[i]->uav.Get();
			}
			//現状まだ第四引数の意味が分かっていない、おって調査
			Graphics::Device::GetContext()->CSSetUnorderedAccessViews(start_slot, count, uavs, nullptr);
			delete[] uavs;
		}
		void ComputeShader::ClearInputBuffers(int start_slot, int count) {
			ID3D11ShaderResourceView** srvs = new ID3D11ShaderResourceView*[count];
			for (int i = 0; i < count; i++) {
				srvs[i] = nullptr;
			}
			Graphics::Device::GetContext()->CSSetShaderResources(start_slot, count, srvs);
			delete[] srvs;
		}
		void ComputeShader::ClearOutputBuffers(int start_slot, int count) {
			ID3D11UnorderedAccessView** uavs = new ID3D11UnorderedAccessView*[count];
			for (int i = 0; i < count; i++) {
				uavs[i] = nullptr;
			}
			Graphics::Device::GetContext()->CSSetUnorderedAccessViews(start_slot, count, uavs, nullptr);
			delete[] uavs;
		}
		void ComputeShader::Run(UINT x, UINT y, UINT z) {
			Graphics::Device::GetContext()->CSSetShader(cs.Get(), nullptr, 0);
			Graphics::Device::GetContext()->Dispatch(x, y, z);
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////
	}

	ComputeShader::ComputeShader(const char* file_path, const char* entry_point) :Shader(file_path, entry_point, "cs_5_0", nullptr) {
		Device::Get()->CreateComputeShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, cs.GetAddressOf());
	}
	ComputeShader::~ComputeShader() = default;
	void ComputeShader::SetShaderResourceView(int slot, ID3D11ShaderResourceView* srv) {
		Device::GetContext()->CSSetShaderResources(slot, 1, &srv);
	}
	void ComputeShader::SetShaderResourceView(int slot, int sum, ID3D11ShaderResourceView** srvs) {
		Device::GetContext()->CSSetShaderResources(slot, sum, srvs);
	}
	void ComputeShader::SetUnorderedAccessView(int slot, ID3D11UnorderedAccessView* uav) {
		Device::GetContext()->CSSetUnorderedAccessViews(slot, 1, &uav, nullptr);
	}
	void ComputeShader::SetUnorderedAccessView(int slot, int sum, ID3D11UnorderedAccessView** uavs) {
		Device::GetContext()->CSSetUnorderedAccessViews(slot, sum, uavs, nullptr);
	}
	void ComputeShader::Run(int thread_x, int thread_y, int thread_z) {
		Device::GetContext()->CSSetShader(cs.Get(), nullptr, 0);
		Device::GetContext()->Dispatch(thread_x, thread_y, thread_z);
		Device::GetContext()->CSSetShader(nullptr, nullptr, 0);
	}
}