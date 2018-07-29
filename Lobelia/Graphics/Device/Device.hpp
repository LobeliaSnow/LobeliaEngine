#pragma once
namespace Lobelia::Graphics {
	class Device {
	public:
		//第一引数がいるか否かは長考
		static void Create(UINT device_flag = 0, GraphicDriverInfo* info = nullptr);
		static void Destroy();
		static ComPtr<ID3D11Device>& Get();
		//ディファードコンテキストについて考える
		static ComPtr<ID3D11DeviceContext>& GetContext();
	private:
		static ComPtr<ID3D11Device> device;
		static ComPtr<ID3D11DeviceContext> context;
		static std::map<DWORD, ComPtr<ID3D11DeviceContext>> deferredContexts;
		static DWORD threadID;
		//static std::unordered_map<DWORD, ComPtr<ID3D11DeviceContext>> contexts;
	};
}