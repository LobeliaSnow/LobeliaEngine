namespace Lobelia {
	class Scene Interface {
	public:
		/**@brief �R���X�g���N�^*/
		Scene() = default;
		/**@brief �V�[���J�ڌ��̃C���X�^���X���������ۂɓ���*/
		virtual void Initialize() {}
		/**@brief �f�X�g���N�^*/
		virtual ~Scene() = default;
		/**@brief �X�V���� �������z�֐�*/
		virtual void Update() = 0;
		/**@brief �`�揈�� �������z�֐�*/
		virtual void Render() = 0;
	};
	//�V�[�������R�Ƀ��C���[�\���g�߂�悤�ɂ������B
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