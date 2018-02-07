#pragma once
namespace Lobelia::Graphics {
	class Device {
	private:
		static ComPtr<ID3D11Device> device;
		static ComPtr<ID3D11DeviceContext> context;
		//static std::unordered_map<DWORD, ComPtr<ID3D11DeviceContext>> contexts;
	public:
		//‘æˆêˆø”‚ª‚¢‚é‚©”Û‚©‚Í’·l
		static void Create(UINT device_flag = 0, GraphicDriverInfo* info = nullptr);
		static void Destroy();
		static ComPtr<ID3D11Device>& Get();
		static ComPtr<ID3D11DeviceContext>& GetContext();
	};
}