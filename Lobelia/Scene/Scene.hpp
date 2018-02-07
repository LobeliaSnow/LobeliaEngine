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
	protected:
	
	};
}