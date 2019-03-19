#include "Lobelia.hpp"
#include "Common/ComputeBuffer.hpp"

namespace Lobelia::Game {
	//---------------------------------------------------------------------------------------------
	StructuredBuffer::StructuredBuffer(int struct_size, int count) :STRUCT_SIZE(struct_size), COUNT(count) {
		D3D11_BUFFER_DESC desc = {};
		desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		desc.ByteWidth = struct_size * count;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.StructureByteStride = struct_size;
		//D3D11_SUBRESOURCE_DATA initData = {};
		//initData.pSysMem = 
		Graphics::Device::Get()->CreateBuffer(&desc, nullptr, buffer.GetAddressOf());
		//SRV（シェーダーリソースビュー）作成
		D3D11_SHADER_RESOURCE_VIEW_DESC srdesc = {};
		srdesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		srdesc.Format = DXGI_FORMAT_UNKNOWN;
		srdesc.BufferEx.NumElements = count;
		HRESULT hr = Graphics::Device::Get()->CreateShaderResourceView(buffer.Get(), &srdesc, srv.GetAddressOf());
		if (FAILED(hr))STRICT_THROW("SRV作成に失敗");
	}
	void StructuredBuffer::Update(const void* resource) { Graphics::Device::GetContext()->UpdateSubresource(buffer.Get(), 0, nullptr, resource, 0, 0); }
	void StructuredBuffer::Set(int slot, Graphics::ShaderStageList stage) {
		switch (stage) {
		case Graphics::ShaderStageList::VS:	Graphics::Device::GetContext()->VSSetShaderResources(slot, 1, srv.GetAddressOf());	break;
		case Graphics::ShaderStageList::PS:	Graphics::Device::GetContext()->PSSetShaderResources(slot, 1, srv.GetAddressOf());	break;
		case Graphics::ShaderStageList::HS:	Graphics::Device::GetContext()->HSSetShaderResources(slot, 1, srv.GetAddressOf());	break;
		case Graphics::ShaderStageList::GS:	Graphics::Device::GetContext()->GSSetShaderResources(slot, 1, srv.GetAddressOf());	break;
		case Graphics::ShaderStageList::DS:	Graphics::Device::GetContext()->DSSetShaderResources(slot, 1, srv.GetAddressOf());	break;
		case Graphics::ShaderStageList::CS:	Graphics::Device::GetContext()->CSSetShaderResources(slot, 1, srv.GetAddressOf());	break;
		default:	STRICT_THROW("範囲外の値です");
		}
	}
	int StructuredBuffer::GetStructSize() { return STRUCT_SIZE; }
	int StructuredBuffer::GetCount() { return COUNT; }
	int StructuredBuffer::GetBufferSize() { return STRUCT_SIZE * COUNT; }
	//---------------------------------------------------------------------------------------------
	UnorderedAccessView::UnorderedAccessView(Graphics::Texture* texture) {
		D3D11_UNORDERED_ACCESS_VIEW_DESC desc = {};
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = 0;
		HRESULT hr = Graphics::Device::Get()->CreateUnorderedAccessView(texture->Get().Get(), &desc, uav.GetAddressOf());
		if (FAILED(hr))STRICT_THROW("UAVの作成に失敗");
	}
	UnorderedAccessView::UnorderedAccessView(StructuredBuffer* structured_buffer) {
		D3D11_UNORDERED_ACCESS_VIEW_DESC desc = {};
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.Buffer.NumElements = structured_buffer->GetCount();
		desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		desc.Texture2D.MipSlice = 0;
		HRESULT hr = Graphics::Device::Get()->CreateUnorderedAccessView(structured_buffer->buffer.Get(), &desc, uav.GetAddressOf());
		if (FAILED(hr))STRICT_THROW("UAVの作成に失敗");
	}
	void UnorderedAccessView::Set(int slot) {
		Graphics::Device::GetContext()->CSSetUnorderedAccessViews(slot, 1, uav.GetAddressOf(), nullptr);
	}
	void UnorderedAccessView::Clean(int slot) {
		ID3D11UnorderedAccessView* null = nullptr;
		Graphics::Device::GetContext()->CSSetUnorderedAccessViews(slot, 1, &null, nullptr);
	}
	//---------------------------------------------------------------------------------------------
//#define TEST
	ReadGPUBuffer::ReadGPUBuffer(std::shared_ptr<StructuredBuffer> buffer) :origin(buffer) {
		if (!buffer)STRICT_THROW("オリジナルのバッファが存在しません");
#ifndef TEST
		D3D11_BUFFER_DESC desc = {};
		buffer->buffer->GetDesc(&desc);
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		desc.Usage = D3D11_USAGE_STAGING;
		desc.BindFlags = 0; desc.MiscFlags = 0;
		HRESULT hr = Graphics::Device::Get()->CreateBuffer(&desc, nullptr, this->buffer.GetAddressOf());
		if (FAILED(hr))STRICT_THROW("STAGINGバッファ作成に失敗");
#endif
	}
	void ReadGPUBuffer::ReadCopy() {
		if (origin.expired())STRICT_THROW("オリジナルのバッファが存在しません");
		std::shared_ptr<StructuredBuffer> src = origin.lock();
#ifdef TEST
		D3D11_BUFFER_DESC desc = {};
		src->buffer->GetDesc(&desc);
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		desc.Usage = D3D11_USAGE_STAGING;
		desc.BindFlags = 0; desc.MiscFlags = 0;
		HRESULT hr = Graphics::Device::Get()->CreateBuffer(&desc, nullptr, this->buffer.ReleaseAndGetAddressOf());
		if (FAILED(hr))STRICT_THROW("STAGINGバッファ作成に失敗");
#endif
		Graphics::Device::GetContext()->CopyResource(buffer.Get(), src->buffer.Get());
	}
	void* ReadGPUBuffer::ReadBegin() {
		if (!buffer)STRICT_THROW("バッファがコピーされていません");
		D3D11_MAPPED_SUBRESOURCE resource = {};
		Timer timer;
		timer.Begin();
		HRESULT hr = Graphics::Device::GetContext()->Map(buffer.Get(), 0, D3D11_MAP_READ, 0, &resource);
		if (FAILED(hr))STRICT_THROW("マップに失敗");
		timer.End();
		HostConsole::GetInstance()->Printf("Map : %f m/sec", timer.GetMilisecondResult());
		return resource.pData;
	}
	void ReadGPUBuffer::ReadEnd() {
		Graphics::Device::GetContext()->Unmap(buffer.Get(), 0);
	}
	RWByteAddressBuffer::RWByteAddressBuffer(void* init_buffer, UINT element_size, UINT element_count, bool is_vertex_buffer, bool is_index_buffer, bool is_indirect_args) {
		//バッファ作成
		CreateRWByteAddressBuffer(buffer, init_buffer, element_size, element_count, is_vertex_buffer, is_index_buffer, is_indirect_args);
		//UAV作成
		CreateUAV(uav, buffer);
	}
	RWByteAddressBuffer::~RWByteAddressBuffer() = default;
	void RWByteAddressBuffer::CreateRWByteAddressBuffer(ComPtr<ID3D11Buffer>& buffer, void* init_buffer, UINT element_size, UINT element_count, bool is_vertex_buffer, bool is_index_buffer, bool is_indirect_args) {
		UINT bindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		//頂点バッファとしてバインドするか否か
		if (is_vertex_buffer)bindFlags |= D3D11_BIND_VERTEX_BUFFER;
		//インデックスバッファとしてバインドするか否か
		if (is_index_buffer)bindFlags |= D3D11_BIND_INDEX_BUFFER;
		UINT miscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
		//Indirect系の引数として使用するか否か
		if (is_indirect_args)miscFlags |= D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
		//バッファ作成
		Graphics::BufferCreator::Create(buffer.GetAddressOf(), init_buffer, element_size*element_count, D3D11_USAGE_DEFAULT, bindFlags, 0, element_size, miscFlags);
	}
	void RWByteAddressBuffer::CreateUAV(ComPtr<ID3D11UnorderedAccessView>& uav, const ComPtr<ID3D11Buffer>& buffer) {
		D3D11_BUFFER_DESC bufferDesc;
		//バッファの情報取得
		buffer->GetDesc(&bufferDesc);
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		if (bufferDesc.MiscFlags == D3D11_RESOURCE_MISC_BUFFER_STRUCTURED) {
			//構造化バッファの場合
			//要素数
			uavDesc.Buffer.NumElements = bufferDesc.ByteWidth / bufferDesc.StructureByteStride;
			uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		}
		else if (bufferDesc.MiscFlags == D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS || bufferDesc.MiscFlags&D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS) {
			//要素数(1要素はfloat又はunsigned intなので4で割る)
			uavDesc.Buffer.NumElements = bufferDesc.ByteWidth / 4UL;
			uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
			uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
		}
		//上の情報をもとにUAV作成
		Graphics::Device::Get()->CreateUnorderedAccessView(buffer.Get(), &uavDesc, uav.GetAddressOf());
		//D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};
		//Graphics::Device::Get()->CreateShaderResourceView(buffer.Get(),)
	}
	void RWByteAddressBuffer::ResourceUpdate(void* data_buffer, UINT element_size, UINT element_count) {
		//まだボックスの部分分かっていないので調査必要
		D3D11_BOX copyRange = { 0,0,0,element_size*element_count,1,1 };
		Graphics::Device::GetContext()->UpdateSubresource(buffer.Get(), 0, &copyRange, data_buffer, element_size, element_count);
	}

}