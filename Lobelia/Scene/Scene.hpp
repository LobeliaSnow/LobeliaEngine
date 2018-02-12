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
		using SceneObjcet = std::pair<std::string, std::shared_ptr<Scene>>;
	private:
		std::vector<SceneObjcet> scenes;
	public:
		int GetSceneCount();
		void PushBack(const char* scene_name, std::shared_ptr<Scene>&& scene);
		void PushFront(const char* scene_name, std::shared_ptr<Scene>&& scene);
		void Insert(int index, const char* scene_name, std::shared_ptr<Scene>&& scene);
		void Erase(const char* scene_name);
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