#include "Common/Common.hpp"
#include "Graphics/GraphicDriverInfo/GraphicDriverInfo.hpp"
#include "Graphics/Device/Device.hpp"
#include "Graphics/Shader/Shader.hpp"
#include "Reflection.hpp"
#include "Exception/Exception.hpp"

namespace Lobelia::Graphics {
	Reflection::Reflection(Shader* shader) :shaderDesc{} {
		HRESULT hr = S_OK;
		hr = D3DReflect(shader->blob->GetBufferPointer(), shader->blob->GetBufferSize(), IID_ID3D11ShaderReflection, reinterpret_cast<void**>(reflector.GetAddressOf()));
		if (FAILED(hr))STRICT_THROW("reflectorì¬‚ÉŽ¸”s‚µ‚Ü‚µ‚½");
		reflector->GetDesc(&shaderDesc);
		TakeInputparameterDescs();
	}
	Reflection::~Reflection() = default;
	void Reflection::TakeInputparameterDescs() {
		inputParameterDescs.resize(shaderDesc.InputParameters);
		for (int index = 0; index < static_cast<int>(shaderDesc.InputParameters); index++) {
			reflector->GetInputParameterDesc(index, &inputParameterDescs[index]);
		}
	}
	DXGI_FORMAT Reflection::ConverteDXGIFormat(D3D10_REGISTER_COMPONENT_TYPE type, BYTE mask) {
		if (mask == 1) {
			if (type == D3D_REGISTER_COMPONENT_UINT32) return DXGI_FORMAT_R32_UINT;
			else if (type == D3D_REGISTER_COMPONENT_SINT32) return DXGI_FORMAT_R32_SINT;
			else if (type == D3D_REGISTER_COMPONENT_FLOAT32) return DXGI_FORMAT_R32_FLOAT;
		}
		else if (mask <= 3) {
			if (type == D3D_REGISTER_COMPONENT_UINT32) return DXGI_FORMAT_R32G32_UINT;
			else if (type == D3D_REGISTER_COMPONENT_SINT32) return DXGI_FORMAT_R32G32_SINT;
			else if (type == D3D_REGISTER_COMPONENT_FLOAT32) return DXGI_FORMAT_R32G32_FLOAT;
		}
		else if (mask <= 7) {
			if (type == D3D_REGISTER_COMPONENT_UINT32) return DXGI_FORMAT_R32G32B32_UINT;
			else if (type == D3D_REGISTER_COMPONENT_SINT32)return DXGI_FORMAT_R32G32B32_SINT;
			else if (type == D3D_REGISTER_COMPONENT_FLOAT32) return DXGI_FORMAT_R32G32B32_FLOAT;
		}
		else if (mask <= 15) {
			if (type == D3D_REGISTER_COMPONENT_UINT32) return DXGI_FORMAT_R32G32B32A32_UINT;
			else if (type == D3D_REGISTER_COMPONENT_SINT32) return DXGI_FORMAT_R32G32B32A32_SINT;
			else if (type == D3D_REGISTER_COMPONENT_FLOAT32) return DXGI_FORMAT_R32G32B32A32_FLOAT;
		}
		return DXGI_FORMAT_UNKNOWN;
	}
	const D3D11_SHADER_DESC& Reflection::GetShaderDesc() { return shaderDesc; }
	const D3D11_SIGNATURE_PARAMETER_DESC& Reflection::GetInputParameterDesc(int index) { return inputParameterDescs[index]; }
	DXGI_FORMAT Reflection::GetDXGIFormat(int index) {
		D3D11_SIGNATURE_PARAMETER_DESC& desc = inputParameterDescs[index];
		return ConverteDXGIFormat(desc.ComponentType, desc.Mask);
	}
}