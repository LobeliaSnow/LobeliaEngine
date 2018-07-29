#pragma once
namespace Lobelia::Audio {
	class Bank :public Utility::Singleton<Bank> {
		friend class Utility::Singleton<Bank>;
	public:
		template<class T> using ReferencePtr = std::weak_ptr<T>;
		using Loader = std::function<void(const char*, Buffer*)>;
	public:
		//���[�_�[�̐ݒ� ������ ���[�h�֐� �������ȍ~ �g���q
		template<class... Args> void AttachLoader(Loader loader, Args... args) {
			//���[�_�[�̓o�^
			loaderList.push_front(std::move(loader));
			//�g���q�ƃ��[�_�[�̕R�Â�
			auto RegisterLoader = [this](std::string tag, std::list<Loader>::iterator loader) {
				loaderMap[tag] = loader;
			};
			//�}�b�v�̐���
			std::initializer_list<int> {
				((void)RegisterLoader(args, loaderList.begin()), 0)...
			};
		}
	public:
		void Load(const char* file_path, const char* tag);
		//TODO : ���݃`�F�b�N
		ReferencePtr<Buffer> GetBuffer(const char* tag) { return sounds[tag]; }
		void Clear(const char* tag);
	private:
		Bank() = default;
		~Bank() = default;
	public:
		Bank(const Bank&) = delete;
		Bank(Bank&&) = delete;
		Bank& operator=(const Bank&) = delete;
		Bank& operator=(Bank&&) = delete;
	private:
		//���[�_�[�̎��Ԃ�ێ�
		std::list<Loader> loaderList;
		//�g���q�ɑΉ��������[�_�[�ւ̃A�h���X������
		std::map<std::string, std::list<Loader>::iterator> loaderMap;
		std::map<std::string, std::shared_ptr<Buffer>> sounds;
	};
}