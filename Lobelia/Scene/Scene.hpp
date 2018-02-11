namespace Lobelia {
	class Scene Interface {
	public:
		/**@brief コンストラクタ*/
		Scene() = default;
		/**@brief シーン遷移元のインスタンスが消えた際に動く*/
		virtual void Initialize() {}
		/**@brief デストラクタ*/
		virtual ~Scene() = default;
		/**@brief 更新処理 純粋仮想関数*/
		virtual void Update() = 0;
		/**@brief 描画処理 純粋仮想関数*/
		virtual void Render() = 0;
	};
	//シーンを自由にレイヤー構造組めるようにしたい。
	class SceneManager :public Utility::Singleton<SceneManager> {
		friend class Utility::Singleton<SceneManager>;
	private:
		struct Layer {
			std::shared_ptr<Scene> scene;
		};
	private:
		SceneManager() = default;
		~SceneManager() = default;
	public:
		SceneManager(const SceneManager&) = delete;
		SceneManager(SceneManager&&) = delete;
		SceneManager& operator=(const SceneManager&) = delete;
		SceneManager& operator=(SceneManager&&) = delete;
	};
}