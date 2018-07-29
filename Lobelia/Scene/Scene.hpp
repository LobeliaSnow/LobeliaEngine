namespace Lobelia {
	class Scene Interface {
	public:
		/**@brief コンストラクタ*/
		Scene() = default;
		/**@brief シーン遷移元のインスタンスが消えた際に動く*/
		virtual void Initialize() {}
		/**@brief デストラクタ*/
		virtual ~Scene() = default;
		/**@brief 常に実行される更新処理 仮想関数*/
		virtual void AlwaysUpdate() {}
		/**@brief 更新処理 純粋仮想関数*/
		virtual void NoPauseUpdate() {}
		/**@brief 一時停止中更新処理 仮想関数*/
		virtual void PauseUpdate() {}
		/**@brief 常に実行される描画処理 仮想関数*/
		virtual void AlwaysRender() {}
		/**@brief 描画処理 純粋仮想関数*/
		virtual void NoPauseRender() {}
		/**@brief 一時停止中描画処理 仮想関数*/
		virtual void PauseRender() {}
	};
	//シーンを自由にレイヤー構造組めるようにしたい。
	class SceneManager :public Utility::Singleton<SceneManager> {
		friend class Utility::Singleton<SceneManager>;
	public:
		//ChangeScene,ChangeReserveを封じる
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