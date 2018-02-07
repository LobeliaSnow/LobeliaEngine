#include "Common/Common.hpp"
#include "Graphics/GraphicDriverInfo/GraphicDriverInfo.hpp"
#include "Graphics/Device/Device.hpp"
#include "Graphics/Shader/Shader.hpp"
#include "Graphics/Shader/Reflection/Reflection.hpp"
#include "Graphics/InputLayout/InputLayout.hpp"
#include "Exception/Exception.hpp"

namespace Lobelia::Graphics {
	//TODO : もっと関数分けをする
	InputLayout::InputLayout(VertexShader* vs, Reflection* reflector) {
		std::vector<D3D11_INPUT_ELEMENT_DESC> elements;
		D3D11_SHADER_DESC shaderDesc = reflector->GetShaderDesc();
		elements.resize(shaderDesc.InputParameters);
		for (int index = 0; index < static_cast<int>(shaderDesc.InputParameters); index++) {
			D3D11_SIGNATURE_PARAMETER_DESC parameterDesc = reflector->GetInputParameterDesc(index);
			std::string semantic = parameterDesc.SemanticName;
			UINT instanceTag = SlotAssignment(semantic);
			elements[index].SemanticName = parameterDesc.SemanticName;
			elements[index].SemanticIndex = parameterDesc.SemanticIndex;
			elements[index].Format = reflector->GetDXGIFormat(index);
			elements[index].InputSlot = instanceTag;
			elements[index].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			elements[index].InputSlotClass = instanceTag == 0 ? D3D11_INPUT_PER_VERTEX_DATA : D3D11_INPUT_PER_INSTANCE_DATA;
			elements[index].InstanceDataStepRate = instanceTag;
		}
		HRESULT hr = Device::Get()->CreateInputLayout(elements.data(), static_cast<UINT>(elements.size()), vs->blob->GetBufferPointer(), vs->blob->GetBufferSize(), inputLayout.GetAddressOf());
		if (FAILED(hr))STRICT_THROW("頂点レイアウト作成に失敗");
	}
	InputLayout::~InputLayout() = default;
	//他にも規則追加時はここに
	UINT InputLayout::SlotAssignment(const std::string& semantic) { return (semantic.find("_I") < UINT_MAX) ? 1 : 0; }
	void InputLayout::Set() { Device::GetContext()->IASetInputLayout(inputLayout.Get()); }
}