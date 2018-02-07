#include "Common/Common.hpp"
#include "Graphics/BufferCreator/BufferCreator.h"
#include "Graphics/ConstantBuffer/ShaderStageList.hpp"
#include "Graphics/GraphicDriverInfo/GraphicDriverInfo.hpp"
#include "Graphics/Device/Device.hpp"
#include "Exception/Exception.hpp"

namespace Lobelia::Graphics {
	void BufferCreator::Create(ID3D11Buffer** buffer, void* resource, UINT byte_width, D3D11_USAGE usage, UINT bind_flag, UINT cpu_flag, UINT struct_stride, UINT misc_flags) {
		//頂点バッファ
		D3D11_BUFFER_DESC desc;
		desc.ByteWidth = byte_width;
		desc.Usage = usage;
		desc.BindFlags = bind_flag;
		desc.CPUAccessFlags = cpu_flag;
		desc.MiscFlags = misc_flags;
		desc.StructureByteStride = struct_stride;
		HRESULT hr = S_OK;
		if (resource) {
			D3D11_SUBRESOURCE_DATA subResource;
			subResource.pSysMem = resource;
			subResource.SysMemPitch = 0;
			subResource.SysMemSlicePitch = 0;
			hr = Device::Get()->CreateBuffer(&desc, &subResource, buffer);
		}
		else	hr = Device::Get()->CreateBuffer(&desc, nullptr, buffer);

		if (FAILED(hr))STRICT_THROW("バッファの作成に失敗しました");
	}
}