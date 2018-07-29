#pragma once
namespace Lobelia::Graphics {
	class Device {
	public:
		//�����������邩�ۂ��͒��l
		static void Create(UINT device_flag = 0, GraphicDriverInfo* info = nullptr);
		static void Destroy();
		static ComPtr<ID3D11Device>& Get();
		//�f�B�t�@�[�h�R���e�L�X�g�ɂ��čl����
		static ComPtr<ID3D11DeviceContext>& GetContext();
	private:
		static ComPtr<ID3D11Device> device;
		static ComPtr<ID3D11DeviceContext> context;
		static std::map<DWORD, ComPtr<ID3D11DeviceContext>> deferredContexts;
		static DWORD threadID;
		//static std::unordered_map<DWORD, ComPtr<ID3D11DeviceContext>> contexts;
	};
}