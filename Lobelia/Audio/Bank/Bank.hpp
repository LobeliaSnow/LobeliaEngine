#pragma once
namespace Lobelia::Audio {
	class Bank :public Utility::Singleton<Bank> {
		friend class Utility::Singleton<Bank>;
	public:
		template<class T> using ReferencePtr = std::weak_ptr<T>;
		using Loader = std::function<void(const char*, Buffer*)>;
	public:
		//ローダーの設定 第一引数 ロード関数 第二引数以降 拡張子
		template<class... Args> void AttachLoader(Loader loader, Args... args) {
			//ローダーの登録
			loaderList.push_front(std::move(loader));
			//拡張子とローダーの紐づけ
			auto RegisterLoader = [this](std::string tag, std::list<Loader>::iterator loader) {
				loaderMap[tag] = loader;
			};
			//マップの生成
			std::initializer_list<int> {
				((void)RegisterLoader(args, loaderList.begin()), 0)...
			};
		}
	public:
		void Load(const char* file_path, const char* tag);
		//TODO : 存在チェック
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
		//ローダーの実態を保持
		std::list<Loader> loaderList;
		//拡張子に対応したローダーへのアドレスを所持
		std::map<std::string, std::list<Loader>::iterator> loaderMap;
		std::map<std::string, std::shared_ptr<Buffer>> sounds;
	};
}