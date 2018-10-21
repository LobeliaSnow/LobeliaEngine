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
		//SRV�i�V�F�[�_�[���\�[�X�r���[�j�쐬
		D3D11_SHADER_RESOURCE_VIEW_DESC srdesc = {};
		srdesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		srdesc.Format = DXGI_FORMAT_UNKNOWN;
		srdesc.BufferEx.NumElements = count;
		HRESULT hr = Graphics::Device::Get()->CreateShaderResourceView(buffer.Get(), &srdesc, srv.GetAddressOf());
		if (FAILED(hr))STRICT_THROW("SRV�쐬�Ɏ��s");
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
		default:	STRICT_THROW("�͈͊O�̒l�ł�");
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
		if (FAILED(hr))STRICT_THROW("UAV�̍쐬�Ɏ��s");
	}
	UnorderedAccessView::UnorderedAccessView(StructuredBuffer* structured_buffer) {
		D3D11_UNORDERED_ACCESS_VIEW_DESC desc = {};
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.Buffer.NumElements = structured_buffer->GetCount();
		desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		desc.Texture2D.MipSlice = 0;
		HRESULT hr = Graphics::Device::Get()->CreateUnorderedAccessView(structured_buffer->buffer.Get(), &desc, uav.GetAddressOf());
		if (FAILED(hr))STRICT_THROW("UAV�̍쐬�Ɏ��s");
	}
	void UnorderedAccessView::Set(int slot) {
		Graphics::Device::GetContext()->CSSetUnorderedAccessViews(slot, 1, uav.GetAddressOf(), nullptr);
	}
	void UnorderedAccessView::Clean(int slot) {
		ID3D11UnorderedAccessView* null = nullptr;
		Graphics::Device::GetContext()->CSSetUnorderedAccessViews(slot, 1, &null, nullptr);
	}
	//---------------------------------------------------------------------------------------------
	ReadGPUBuffer::ReadGPUBuffer(std::shared_ptr<StructuredBuffer> buffer) :origin(buffer) {
		if (!buffer)STRICT_THROW("�I���W�i���̃o�b�t�@�����݂��܂���");
		D3D11_BUFFER_DESC desc = {};
		buffer->buffer->GetDesc(&desc);
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		desc.Usage = D3D11_USAGE_STAGING;
		desc.BindFlags = 0; desc.MiscFlags = 0;
		HRESULT hr = Graphics::Device::Get()->CreateBuffer(&desc, nullptr, this->buffer.GetAddressOf());
		if (FAILED(hr))STRICT_THROW("STAGING�o�b�t�@�쐬�Ɏ��s");
	}
	void ReadGPUBuffer::ReadCopy() {
		if (origin.expired())STRICT_THROW("�I���W�i���̃o�b�t�@�����݂��܂���");
		std::shared_ptr<StructuredBuffer> src = origin.lock();
		Graphics::Device::GetContext()->CopyResource(buffer.Get(), src->buffer.Get());
	}
	void* ReadGPUBuffer::ReadBegin() {
		D3D11_MAPPED_SUBRESOURCE resource = {};
		HRESULT hr = Graphics::Device::GetContext()->Map(buffer.Get(), 0, D3D11_MAP_READ, 0, &resource);
		if (FAILED(hr))STRICT_THROW("�}�b�v�Ɏ��s");
		return resource.pData;
	}
	void ReadGPUBuffer::ReadEnd() { Graphics::Device::GetContext()->Unmap(buffer.Get(), 0); }

}