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