namespace Lobelia {
	//class Layer {
	//private:
	//public:
	//	Layer() = default;
	//	virtual ~Layer() = default;
	//	virtual void Update() = 0;
	//	virtual void Render() = 0;
	//};
	class Scene {
	public:
		/**@brief コンストラクタ*/
		Scene() = default;
		/**@brief シーン遷移元のインスタンスが消えた際に動く*/
		virtual void Initialize() {}
		/**@brief デストラクタ*/
		virtual ~Scene() = default;
		/**@brief 更新処理 純粋仮想関数*/
		virtual void Update() = 0;
		/**@brief 一時停止中更新処理 仮想関数*/
		virtual void PauseUpdate() {}
		/**@brief 描画処理 純粋仮想関数*/
		virtual void Render() = 0;
	};
	//シーンを自由にレイヤー構造組めるようにしたい。
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
		void Render() { if (scene)scene->Render(); }
		void Pause(bool flag) { pause = flag; }
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