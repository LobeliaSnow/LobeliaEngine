#pragma once

namespace Lobelia {
	/**
	*@brief リソースマネージャー
	*@param 管理対象の型
	*@param 鍵となる型 デフォルトではstd::string
	*/
	template<class T, class Key = std::string> class ResourceBank final {
	protected:
		/**
		*@brief リソース管理用連想配列
		*/
		static std::map<Key, std::shared_ptr<T>> resource;
	public:
		/**@brief コンストラクタ*/
		ResourceBank()noexcept;
		/**@brief デストラクタ*/
		~ResourceBank()noexcept;
		/**
		*@brief ファクトリ関数
		*@brief インスタンスが作られていれば既存のものを、作られていなければ新規で作成して返します。
		*@param <template>インスタンス作成時に入れた型と継承関係のものを管理したい場合に指定、しなければそのままの型
		*@param <template>インスタンス作成時に使用する引数リスト
		*@param[in] 鍵
		*@param[in] インスタンス生成に必要な引数
		*@return インスタンスへのポインタ
		*/
		template<class U = T, class... Args> static T* Factory(Key key, Args&&... args);
		/**
		*@brief 登録関数
		*@brief ファクトリを使用せず直接インスタンスを預けたいときに使用
		*@param[in] 鍵
		*@param[in] インスタンスへのポインタ
		*/
		static void Register(Key key, std::shared_ptr<T> p)noexcept;
		/**
		*@brief インスタンスが存在するか返す
		*@param[in] 鍵
		*@return 存在しているか否かのbool値
		*/
		static bool IsExist(Key key)noexcept;
		static void Clear()noexcept;
		static T* Get(Key key)noexcept;
		static std::map<Key, std::shared_ptr<T>>& Get()noexcept;
		static void Erase(Key key)noexcept;
	};
	//class FileInfoAnalyzer {
	//private:
	//public:
	//	static long long GetTimeStamp(const char* file_path);
	//	static bool GetAccessAuthority(const char* file_path);
	//};
}

#include "ResourceBank.inl"