#pragma once
namespace Lobelia::Graphics {
	class InputLayout {
	private:
		ComPtr<ID3D11InputLayout> inputLayout;
	private:
		UINT SlotAssignment(const std::string& semantic);
	public:
		InputLayout(VertexShader* vs, Reflection* reflector);
		~InputLayout();
		void Set();
	};
}