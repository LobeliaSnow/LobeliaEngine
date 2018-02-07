#pragma once
namespace Lobelia::Graphics {
	template<class T> class ConstantBuffer {
	public:
	private:
		ComPtr<ID3D11Buffer> buffer;
		UINT slot;
		std::vector<std::function<void()>> functor;
	private:
		void CreateConstantBuffer();
		void SetFunctor(byte activate_shader_elements);
	public:
		//ShaderStageListがフラグになってます。
		ConstantBuffer(UINT slot, byte activate_shader_elements);
		virtual ~ConstantBuffer();
		void Activate(const T& cb_struct);
	};
}

#include "ConstantBuffer.inl"

//note: コンスタントバッファのリフレクション
//reflection->GetResourceBindingDesc
//D3D11_SHADER_INPUT_BIND_DESC