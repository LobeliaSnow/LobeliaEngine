namespace Lobelia {
	class Scene {
	public:
		/**@brief �R���X�g���N�^*/
		Scene() = default;
		/**@brief �V�[���J�ڌ��̃C���X�^���X���������ۂɓ���*/
		virtual void Initialize() {}
		/**@brief �f�X�g���N�^*/
		virtual ~Scene() = default;
		/**@brief �X�V���� �������z�֐�*/
		virtual void Update() = 0;
		/**@brief �ꎞ��~���X�V���� ���z�֐�*/
		virtual void PauseUpdate() {}
		/**@brief �`�揈�� �������z�֐�*/
		virtual void Render() = 0;
	};
	//�V�[�������R�Ƀ��C���[�\���g�߂�悤�ɂ������B
	class SceneManager :public Utility::Singleton<SceneManager> {
		friend class Utility::Singleton<SceneManager>;
	private:
		std::shared_ptr<Scene> tempScene;
		std::shared_ptr<Scene> scene;
		std::stack<std::shared_ptr<Scene>> stack;
		bool pause;
	public:
		template<class T, class... Args> void ChangeReserve(Args ...args) { tempScene = std::make_shared<T>(std::forward<Args>(args)...); }
		void ChangeReserve(std::shared_ptr<Scene> next) { tempScene = next; }
		void ChangeExecute() {
			if (tempScene) {
				scene = std::move(tempScene);
				scene->Initialize();
			}
		}
		template<class T, class... Args> void PushReserve(Args ...args) {
			stack.push(scene);
			ChangeReserve(std::forward<Args>(args)...);
		}
		void PopReserve() {
			std::shared_ptr<Scene> temp = stack.top();
			stack.pop();
			ChangeReserve(temp);
		}
		void Update() {
			if (scene) {
				if (!pause)scene->Update();
				else scene->PauseUpdate();
			}
			ChangeExecute();
		}
		void ClearStack() { while (stack.size() != 0)stack.pop(); }
		void Render() { if (scene)scene->Render(); }
		void Pause(bool flag) { pause = flag; }
		void Clear() { scene.reset(); }
	private:
		SceneManager() :pause(false) {}
		~SceneManager() = default;
	public:
		SceneManager(const SceneManager&) = delete;
		SceneManager(SceneManager&&) = delete;
		SceneManager& operator=(const SceneManager&) = delete;
		SceneManager& operator=(SceneManager&&) = delete;
	};
}