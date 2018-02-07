#pragma once
namespace Lobelia::Graphics {
	class Reflection {
	private:
		ComPtr<ID3D11ShaderReflection> reflector;
		D3D11_SHADER_DESC shaderDesc;
		std::vector<D3D11_SIGNATURE_PARAMETER_DESC> inputParameterDescs;
	private:
		void TakeInputparameterDescs();
		DXGI_FORMAT ConverteDXGIFormat(D3D10_REGISTER_COMPONENT_TYPE type, BYTE mask);
	public:
		Reflection(Shader* shader);
		~Reflection();
		const D3D11_SHADER_DESC& GetShaderDesc();
		const D3D11_SIGNATURE_PARAMETER_DESC& GetInputParameterDesc(int index);
		DXGI_FORMAT GetDXGIFormat(int index);
	};
}