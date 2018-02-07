#pragma once
namespace Lobelia::Graphics {
		class DisplayInfo {
			friend class DisplayInfoList;
		private:
			ComPtr<IDXGIOutput> output;
			DXGI_OUTPUT_DESC desc;
			HDC hdc;
			int refreshRate;
			int driverVersion;
			Math::Vector2 size;
			DISPLAY_DEVICE device;
		public:
			DisplayInfo(const ComPtr<IDXGIOutput>& adapter, int index);
			~DisplayInfo();
			HDC GetHDC()const noexcept;
			std::string GetOutputGPUName()const noexcept;
			std::string GetID()const noexcept;
			std::string GetKey()const noexcept;
			int GetRefreshRate()const noexcept;
			int GetDriverVersion()const noexcept;
			RECT GetDesktopRect()const noexcept;
			int GetAttachedDesktopCount()const noexcept;
			DXGI_MODE_ROTATION GetRotationMode()const noexcept;
			//’PˆÊ mm
			Math::Vector2 GetPhysicSize()const noexcept;
		};
		class DisplayInfoList {
		private:
			static ComPtr<IDXGIFactory> factory;
			static std::vector<std::shared_ptr<DisplayInfo>> displays;
			static int displayCount;
		public:
			static void Bootup();
			static void Shutdown();
			static int GetDisplayCount();
			static const std::shared_ptr<DisplayInfo>& GetDisplayInfo(int index);
			static const std::vector<std::shared_ptr<DisplayInfo>>& GetDisplaysInfo();

		};
}