#pragma once
namespace Lobelia {
	namespace Graphics { class SwapChain; }

	//TODO : SceneManager�Ƃ��ĕ�����

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
		//�V�[���J�ڂ��\�񂳂�܂�
		//���ۂɑJ�ڂ���̂͂��̃N���X�̕`����I�����^�C�~���O�ł�
		template<class Scene, class ...Args> void ChangeSceneReserve(Args... args);
		//�V�[���J�ڂ����s����܂�
		//Reserve����Ύ����ŌĂ΂��̂Ŗ����I�ɌĂԕK�v�͂Ȃ��ł����A�����I�ɌĂяo�������^�C�~���O�������
		void ChangeSceneExecute();
		//���̊֐���Initialize�͌Ă΂�܂���
		void ChangeSceneExecute(Scene* next_scene);
		WPARAM Run();
		void Shutdown();
		float GetProcessTime();
		std::shared_ptr<Scene> GetScene();
	};
}

#include "Application.inl"