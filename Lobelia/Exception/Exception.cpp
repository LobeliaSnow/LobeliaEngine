#include "Common/Common.hpp"
#include "Exception.hpp"
#include "Console/Console.hpp"
#include <iomanip> 
#include <sstream> 

namespace Lobelia {
	std::string Exception::CreateMessage()const noexcept {
		std::stringstream error;
		error << "\n======! begin exception message !======\n\n" << "\n[source : " << _filename << "]\n\n[function : " << _functionName << "]\n\n[line : " << _executeLine << "]\n\n";
		if (!_msg.empty())error << "[cause : " << _msg << "]\n\n";
		error << "\n======! end exception message !======\n\n";
		return error.str();
	}
	void Exception::OutputLog()const noexcept {
#ifdef USE_IMGUI_AND_CONSOLE
		HostConsole::GetInstance()->SetLog(CreateMessage());
		time_t date = time(nullptr);
		tm* lt = localtime(&date);
		std::stringstream stream;
		stream << "Log/20" << lt->tm_year - 100 << lt->tm_mon + 1 << lt->tm_mday << lt->tm_hour << lt->tm_min << lt->tm_sec << ".txt";
		HostConsole::GetInstance()->SaveLog(stream.str().c_str());
#endif
	}
	Exception::Exception(std::string filename, std::string func_name, int exe_line) noexcept :Exception(filename, func_name, exe_line, "") {}
	Exception::Exception(std::string filename, std::string func_name, int exe_line, std::string msg) noexcept : _filename(filename), _functionName(func_name), _executeLine(exe_line), _msg(msg) {}
	Exception::~Exception() noexcept = default;
	void Exception::ConsoleMessage()const noexcept {
#if (defined(__EXCEPTION_ON__)||defined(__STRICT_EXCEPTION_ON__))&&!defined(__NO_EXCEPTION__)
		std::cout << CreateMessage() << std::endl;
#endif
		OutputLog();
	}
	void Exception::VsConsoleMessage()const noexcept {
#if (defined(__EXCEPTION_ON__)||defined(__STRICT_EXCEPTION_ON__))&&!defined(__NO_EXCEPTION__)
		OutputDebugString(CreateMessage().c_str());
#endif
		OutputLog();
	}
	void Exception::BoxMessage()const noexcept {
#if (defined(__EXCEPTION_ON__)||defined(__STRICT_EXCEPTION_ON__))&&!defined(__NO_EXCEPTION__)
		MessageBox(nullptr, CreateMessage().c_str(), "exception!", MB_OK | MB_ICONERROR);
#endif
		OutputLog();
	}
	const std::string Exception::GetExceptionMessage()const noexcept { return CreateMessage(); }

	/**
	*@brief エラーチェック
	*@brief エラー時にエラー文発生,falseを返す、そうでなければtureを返す
	*@attention まだまだ未完成、必要となったものが出次第更新していく<br>英語が怪しいせいで翻訳ソフト使ってる部分あり、正しい言い回しあればLobeliaまで<br>間違いも報告お願いします
	*@param[in] errno_t
	*@return bool
	*/
	bool ErrnoCheck(errno_t err) noexcept {
		TRY{
			switch (err) {
			case 0:				break;
				//↓/usr/include/asm-generic/errno-base.h
			case EPERM:								STRICT_THROW("操作は許可されていません?/n原文(Operation not permitted)");
			case ENOENT:							STRICT_THROW("ファイルまたはディレクトリが存在しません");
			case ESRCH:								STRICT_THROW("そのようなプロセスはありません?/n原文(No such process)");
			case EINTR:								STRICT_THROW("システムコールが中断されました");
			case EIO:									STRICT_THROW("I/Oエラー");
			case ENXIO:								STRICT_THROW("デバイスやアドレスがありません");
			case E2BIG:								STRICT_THROW("引数リストが長すぎます");
			case ENOEXEC:							STRICT_THROW("実行形式エラー?/n原文(Exec format error)");
			case EBADF:								STRICT_THROW("ファイル番号が正しくありません");
			case ECHILD:								STRICT_THROW("子プロセスはありません");
			case EAGAIN:								STRICT_THROW("再試行してください");
			case ENOMEM:							STRICT_THROW("メモリ不足です?/n原文(Out of memory)");
			case EACCES:								STRICT_THROW("アクセスが拒否されました");
			case EFAULT:								STRICT_THROW("アドレスが正しくありません");
				//case ENOTBLK:						throw"Block device required";
			case EBUSY:								STRICT_THROW("デバイスまたはリソースがビジー状態です");
			case EEXIST:								STRICT_THROW("ファイルが存在しています");
			case EXDEV:								STRICT_THROW("クロスデバイスリンク?/n原文(Cross-device link)");
			case ENODEV:							STRICT_THROW("デバイスが存在しません");
			case ENOTDIR:							STRICT_THROW("ディレクトリではありません");
			case EISDIR:								STRICT_THROW("ディレクトリです?/n原文(Is a directory)");
			case EINVAL:								STRICT_THROW("無効な引数です");
			case ENFILE:								STRICT_THROW("ファイルテーブルオーバーフロー?/n原文(File table overflow)");
			case EMFILE:								STRICT_THROW("開いているファイルが多すぎます");
			case ENOTTY:							STRICT_THROW("タイプライターではありません?/n原文(Not a typewriter)");
			case ETXTBSY:							STRICT_THROW("テキストファイルがビジー状態です");
			case EFBIG:								STRICT_THROW("ファイルのサイズが大きすぎます");
			case ENOSPC:							STRICT_THROW("デバイスにスペースが残っていません?/n原文(No space left on device)");
			case ESPIPE:								STRICT_THROW("不正なシークです");
			case EROFS:								STRICT_THROW("読み取り専用のシステムファイルです");
			case EMLINK:							STRICT_THROW("リンクが多すぎます");
			case EPIPE:								STRICT_THROW("パイプが壊れています?/n原文(Broken pipe)");
			case EDOM:								STRICT_THROW("訳不明/n原文(Math argument out of domain of func)");
			case ERANGE:							STRICT_THROW("数学的結果が表現できません?/n原文(Math result not representable)");
				//↓/usr/include/asm-generic/errno.h
			case EDEADLK:							STRICT_THROW("リソースのデッドロックが発生しています");
			case ENAMETOOLONG:			STRICT_THROW("ファイル名が長すぎます");
			case ENOLCK:							STRICT_THROW("使用可能なレコードロックがありません?/n原文(No record locks available)");
			case ENOSYS:							STRICT_THROW("機能が実装されてません");
			case ENOTEMPTY:						STRICT_THROW("ディレクトリがからではありません");
			case ELOOP:								STRICT_THROW("シンボリックリンクが多すぎます");
			case EWOULDBLOCK:				STRICT_THROW("操作がブロックされました");
			case ENOMSG:							STRICT_THROW("希望タイプのメッセージがありません?/n原文(No message of desired type)");
			case EIDRM:								STRICT_THROW("識別子が削除されました?/n原文(Identifier removed)");
			case ENOTSOCK:						STRICT_THROW("非ソケットでのソケット操作");
			case EDESTADDRREQ:				STRICT_THROW("宛先アドレスが必要です");
			case EMSGSIZE:							STRICT_THROW("メッセージが長すぎます");
			case EPROTOTYPE:					STRICT_THROW("ソケットのプロトコルタイプが間違っています");
			case ENOPROTOOPT:				STRICT_THROW("プロトコルが利用できません");
			case EPROTONOSUPPORT:		STRICT_THROW("Protocol not supported");
			case EOPNOTSUPP:					STRICT_THROW("トランスポートエンドポイントでサポートされていない操作?/n原文(Operation not supported on transport endpoint)");
			case EAFNOSUPPORT:				STRICT_THROW("アドレスファミリはプロトコルでサポートされていません");
			case EADDRINUSE:					STRICT_THROW("アドレスは既に使用されています");
			case EADDRNOTAVAIL:				STRICT_THROW("要求されたアドレスを割り当てることができません");
			case ENETDOWN:						STRICT_THROW("ネットワークが落ちています");
			case ENETUNREACH:				STRICT_THROW("ネットワークに到達できません");
			case ENETRESET:						STRICT_THROW("リセットによるネットワーク接続の切断");
			case ECONNABORTED:				STRICT_THROW("ソフトウェアによる接続の中止");
			case ECONNRESET:					STRICT_THROW("ピアによる接続リセット");
			case ENOBUFS:							STRICT_THROW("バッファの空きがありません");
			case EISCONN:							STRICT_THROW("トランスポートエンドポイントは既に接続されています?/n原文(Transport endpoint is already connected)");
			case ENOTCONN:						STRICT_THROW("トランスポートエンドポイントが接続されていません?/n原文(Transport endpoint is not connected)");
			case ETIMEDOUT:						STRICT_THROW("接続がタイムアウトしました");
			case ECONNREFUSED:				STRICT_THROW("接続が拒否されました");
			default:										STRICT_THROW("エラー/nエラー内容未登録/n");
			}
		}CATCH(Exception& e) {
			e.BoxMessage();
			return false;
		}
		return true;
	}

}