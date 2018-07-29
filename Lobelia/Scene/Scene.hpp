namespace Lobelia {
	class Scene Interface {
	public:
		/**@brief �R���X�g���N�^*/
		Scene() = default;
		/**@brief �V�[���J�ڌ��̃C���X�^���X���������ۂɓ���*/
		virtual void Initialize() {}
		/**@brief �f�X�g���N�^*/
		virtual ~Scene() = default;
		/**@brief ��Ɏ��s�����X�V���� ���z�֐�*/
		virtual void AlwaysUpdate() {}
		/**@brief �X�V���� �������z�֐�*/
		virtual void NoPauseUpdate() {}
		/**@brief �ꎞ��~���X�V���� ���z�֐�*/
		virtual void PauseUpdate() {}
		/**@brief ��Ɏ��s�����`�揈�� ���z�֐�*/
		virtual void AlwaysRender() {}
		/**@brief �`�揈�� �������z�֐�*/
		virtual void NoPauseRender() {}
		/**@brief �ꎞ��~���`�揈�� ���z�֐�*/
		virtual void PauseRender() {}
	};
	//�V�[�������R�Ƀ��C���[�\���g�߂�悤�ɂ������B
	class SceneManager :public Utility::Singleton<SceneManager> {
		friend class Utility::Singleton<SceneManager>;
	public:
		//ChangeScene,ChangeReserve�𕕂���
		void Lock() { lock = true; }
		void Unlock() { lock = false; }
		template<class T, class... Args> void ChangeReserve(Args&& ...args) { if (!lock)tempScene = std::make_shared<T>(std::forward<Args>(args)...); }
		void ChangeReserve(std::shared_ptr<Scene> next) { if (!lock)tempScene = next; }
		void ChangeScene(std::shared_ptr<Scene> next) { if (!lock)scene = next; }
		void ChangeExecute() {
			if (tempScene) {
				scene = std::move(tempScene);
				if (!pop)scene->Initialize();
				else pop = false;
			}
		}
		template<class T, class... Args> void PushReserve(Args&& ...args) {
			stack.push(scene);
			ChangeReserve<T>(std::forward<Args>(args)...);
		}
		void PopReserve() {
			std::shared_ptr<Scene> temp = stack.top();
			stack.pop();
			ChangeReserve(temp);
			pop = true;
		}
		void Pop() {
			std::shared_ptr<Scene> temp = stack.top();
			stack.pop();
			ChangeScene(temp);
		}
		void Update() {
			ChangeExecute();
			if (scene) {
				scene->AlwaysUpdate();
				if (!pause)scene->NoPauseUpdate();
				else scene->PauseUpdate();
			}
		}
		void ClearStack() { while (stack.size() != 0)stack.pop(); }
		void Render() {
			if (scene) {
				scene->AlwaysRender();
				if (!pause)scene->NoPauseRender();
				else scene->PauseRender();
			}
		}
		void Pause(bool flag) { pause = flag; }
		bool IsPause() { return pause; }
		void Clear() {
			while (!stack.empty()) {
				stack.pop();
			}
			tempScene.reset();
			scene.reset();
			int count = stack.size();
		}
		std::shared_ptr<Scene> GetScene() { return scene; }

	private:
		SceneManager() :pause(false), pop(false) {}
		~SceneManager() {
			Clear();
		}
	public:
		SceneManager(const SceneManager&) = delete;
		SceneManager(SceneManager&&) = delete;
		SceneManager& operator=(const SceneManager&) = delete;
		SceneManager& operator=(SceneManager&&) = delete;
	private:
		std::shared_ptr<Scene> tempScene;
		std::shared_ptr<Scene> scene;
		std::stack<std::shared_ptr<Scene>> stack;
		bool pause;
		bool lock;
		bool pop;
	};
}