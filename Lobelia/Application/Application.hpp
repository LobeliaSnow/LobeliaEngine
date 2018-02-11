#pragma once
namespace Lobelia {
	namespace Graphics { class SwapChain; }
	//TODO : SceneManagerとして分ける
	class Application :public Utility::Singleton<Application> {
		friend class Utility::Singleton<Application>;
	private:
		std::shared_ptr<Scene> scene;
		std::unique_ptr<Timer> timer;
		std::shared_ptr<Window> window;
		std::unique_ptr<Graphics::SwapChain> swapChain;
		float processTimer;
		bool changeScene;
		Scene* tempScene;
	private:
		void FpsRender();
	protected:
		virtual bool IsUpdate();
		virtual void Update();
		virtual void Render();
	public:
		Application();
		virtual ~Application();
		template<class Scene, class ...Args> void Bootup(const Math::Vector2& size, const char* window_name, std::function<LRESULT(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)>wnd_proc, Args... args);
		Window* GetWindow();
		Graphics::SwapChain* GetSwapChain();
		void ResizeBuffer();
		float CalcFps();
		//シーン遷移が予約されます
		//実際に遷移するのはそのクラスの描画を終えたタイミングです
		template<class Scene, class ...Args> void ChangeSceneReserve(Args... args);
		//シーン遷移が実行されます
		//Reserveすれば自動で呼ばれるので明示的に呼ぶ必要はないですが、明示的に呼び出したいタイミングがあれば
		void ChangeSceneExecute();
		//この関数はInitializeは呼ばれません
		void ChangeSceneExecute(Scene* next_scene);
		WPARAM Run();
		void Shutdown();
		float GetProcessTime();
		std::shared_ptr<Scene> GetScene();
	};
}

#include "Application.inl"