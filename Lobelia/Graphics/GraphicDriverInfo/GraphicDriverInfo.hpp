#pragma once
namespace Lobelia::Graphics {
	/**
	*@brief グラフィックドライバーの情報を取得するためのクラス
	*/
	class GraphicDriverInfo {
		friend class GraphicDriverInfoList;
		friend class Device;
	private:
	private:
		DXGI_ADAPTER_DESC desc;
		ComPtr<IDXGIAdapter> adapter;
	public:
		GraphicDriverInfo(const ComPtr<IDXGIAdapter>& adapter);
		~GraphicDriverInfo();
		std::string GetDriverName()const noexcept;
		UINT GetVendorId()const noexcept;
		UINT GetDeviceId()const noexcept;
		UINT GetSubSysId()const noexcept;
		UINT GetRevision()const noexcept;
		size_t GetVideoMemory()const noexcept;
		size_t GetSystemMemory()const noexcept;
		size_t GetSharedSystemMemory()const noexcept;
		LUID GetLUID()const noexcept;
	};
	/**
	*@brief グラフィックドライバーの情報を列挙するためのクラス
	*/
	class GraphicDriverInfoList {
	private:
		static ComPtr<IDXGIFactory> factory;
		static std::vector<std::shared_ptr<GraphicDriverInfo>> drivers;
		static int driverCount;
	public:
		/**@brief この関数を呼び出さないと情報は取れない*/
		static void Bootup();
		/**@brief この関数を呼び出すと次にBootup()を呼ぶまで情報は取れない*/
		static void Shutdown();
		static int GetGraphicDriverCount();
		static const std::shared_ptr<GraphicDriverInfo>& GetDriver(int index);
		static const std::vector<std::shared_ptr<GraphicDriverInfo>>& GetDrivers();
	};
}