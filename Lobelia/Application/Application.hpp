#pragma once
namespace Lobelia {
	namespace Graphics { class SwapChain; }

	class Application :public Utility::Singleton<Application> {
		friend class Utility::Singleton<Application>;
	public:
		Application();
		virtual ~Application();
		template<class Scene, class... Args> void Bootup(const Math::Vector2& size, const char* window_name, std::function<LRESULT(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)>wnd_proc, Args&&... args);
		Window* GetWindow();
		Graphics::SwapChain* GetSwapChain();
		float CalcFps();
		WPARAM Run();
		void Shutdown();
		float GetProcessTimeMili();
		float GetProcessTimeSec();
		void SetTimeScale(float time_scale);
	protected:
		virtual bool IsUpdate();
		virtual void Update();
		virtual void Render();
	private:
		std::unique_ptr<Timer> timer;
		std::shared_ptr<Window> window;
		std::unique_ptr<Graphics::SwapChain> swapChain;
		float processTimer;
		float timeScale;
#ifdef _DEBUG
	public:
		bool debugRender;
#endif
	};
}

#include "Application.inl"