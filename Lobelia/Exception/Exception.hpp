#pragma once
#ifndef __HDX_EXCEPTION__
#define __HDX_EXCEPTION__
#ifdef _DEBUG
//これは例外発生時に指定された方式で情報を表示する
#define __EXCEPTION_ON__
//これはログをはくようにする
#define __EXCEPTION_LOG_ON__
#endif
//指定された例外とログファイルを両方はかせる
//#define __STRICT_EXCEPTION_ON__
//これを定義するとExceptionはログファイルも例外も発さない
//一番優先度が高い ほかのコマンドをすべて無視する
//#define __NO_EXCEPTION__

namespace Lobelia {
	/**
	*@brief 例外クラス
	*@brief 一応地震で実体を作って使用することも可能
	*@attention このクラスは自分で実体を生成するのではなく、<br>
	*下で定義されているマクロによって生成させる。<br>
	*メッセージを表示するには、CATCH文でこのクラスを捕まえてメンバ関数によって表示
	*@attention ログ出力等の制御は上にあるコメント参照
	*/
	class Exception final{
	private:
		/**@brief ソースファイル名*/
		std::string _filename;
		/**@brief 実行された関数名*/
		std::string _functionName;
		/**@brief 例外が出た行*/
		int _executeLine;
		/**@brief 任意のメッセージ*/
		std::string _msg;
	private:
		/**
		*@brief エラーメッセージ構築
		*@return エラー文章
		*/
		std::string CreateMessage()const noexcept;
		/**
		*@brief ログ出力
		*/
		void OutputLog()const noexcept;
	public:
		/**
		*@brief コンストラクタ
		*@param[in] ソースファイル名
		*@param[in] 関数名
		*@param[in] 実行行番号
		*/
		Exception(std::string filename, std::string func_name, int exe_line) noexcept;
		/**
		*@brief コンストラクタ
		*@param[in] ソースファイル名
		*@param[in] 関数名
		*@param[in] 実行行番号
		*@param[in] 任意のメッセージ
		*/
		Exception(std::string filename, std::string func_name, int exe_line, std::string msg) noexcept;
		/**@brief デストラクタ*/
		virtual ~Exception() noexcept;
		/**@brief コンソールに出力*/
		void ConsoleMessage()const noexcept;
		/**@brief VisualStudioの出力窓に出力*/
		void VsConsoleMessage()const noexcept;
		/**@brief メッセージボックスにより出力*/
		void BoxMessage()const noexcept;
		/**@brief エラー文章を取得*/
		const std::string GetExceptionMessage()const noexcept;
	};
}

//この先マクロ定義
/**@def 例外クラス実体生成*/
#define STRICT_EXCEPTION_FACTORY(msg) Lobelia::Exception(FILE_NAME,FUNCTION_NAME,EXECUTE_LINE,msg)
/**@def 例外クラス実体生成*/
#define EXCEPTION_FACTORY STRICT_EXCEPTION_FACTORY("")
/**@def try*/
#define TRY try
/**@def throw Exceptionをメッセージ付きで投げる*/
#define STRICT_THROW(msg) throw STRICT_EXCEPTION_FACTORY(msg)
/**@def throw Exceptionを投げる*/
#define THROW_E STRICT_THROW("");

namespace Lobelia {
	bool ErrnoCheck(errno_t err) noexcept;
}
#endif