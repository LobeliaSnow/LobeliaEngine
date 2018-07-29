#pragma once
#include <memory>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "Common/Timer/Timer.h"
#pragma comment(lib,"wsock32.lib")
#pragma comment(lib, "ws2_32.lib")
namespace Lobelia::Network {
	//関数リファレンス
	//http://www.golgo.jp/winsock.html#l_listen
	//参考
	//http://www.geekpage.jp/programming/winsock/tcp.php
	//https://qiita.com/0xfffffff7/items/6ffb317df8345070d0b5
	//http://www.gesource.jp/programming/bcb/84.html
	//https://support.microsoft.com/ja-jp/help/819124/windows-sockets-error-codes-values-and-meanings
	enum class BlockingMode {
		BLOCK = 0, NO_BLOCK = 1
	};
	class System {
	private:
		static const int SDKVersion = 2;
		static constexpr u_short PORT = 9305;
	private:
		static WSADATA wsaData;
	public:
		static int Startup();
		static int Cleanup();
		static constexpr u_short GetPort() { return PORT; }
	};
	class Address {
		friend class Socket;
	public:
		sockaddr_in addr;
	private:
		void Initialize(int address_family, u_short port);
	public:
		Address(int address_family = AF_INET, u_short port = System::GetPort()) {
			Initialize(address_family, port);
			addr.sin_addr.S_un.S_addr = INADDR_ANY;
		}
		explicit Address(const char* ip, int address_family = AF_INET, u_short port = System::GetPort()) {
			Initialize(address_family, port);
			if (InetPtonA(address_family, ip, &addr.sin_addr) != 1)throw;
		}
		~Address();
	};
	class Socket {
	public:
		SOCKET sock;
	public:
		//protocolは基本0入れてください。AF_INETはIPv4を示す、SOCK_STREAMは順序性と信頼性がある双方向のバイトストリームを提供(TCP)
		Socket(int protocol, int address_family = AF_INET, int type = SOCK_STREAM);
		//空のソケット作成
		Socket();
		~Socket();
		//---------------------------------------------------------------------------
		//	サーバーサイド
		//---------------------------------------------------------------------------
		//接続キューの作成 
		int Listen(int max_connect);
		//接続要求受付
		//第一引数 空のソケット 入るソケットで今後の通信
		//第二引数 空のアドレス 接続相手の情報が格納される
		void Accept(Socket* s, Address* client);
		int Close();
		//---------------------------------------------------------------------------
		//---------------------------------------------------------------------------

		//---------------------------------------------------------------------------
		//	クライアントサイド
		//---------------------------------------------------------------------------
		//サーバーに接続
		int Connect(Address* server);
		//---------------------------------------------------------------------------
		//---------------------------------------------------------------------------

		//---------------------------------------------------------------------------
		//	共通部
		//---------------------------------------------------------------------------
		//ローカルアドレスとソケットを関連付ける
		int Bind(Address* address);
		void BlockingModeSetting(BlockingMode mode);
		int SetOption(int level, int option, char* option_value, size_t option_length);
		///////////////////////////////////////////////////////////////////
		//	主にTCPで使用
		///////////////////////////////////////////////////////////////////
		//このソケットへつながっている先に送信する
		//戻り値は実際に送信されたバイト数又は、SOCKET_ERROR
		int Send(const char* buffer, size_t length, int flags = 0);
		template<size_t _size> int Send(const char(&buffer)[_size], int flags) { return Send(buffer, _size, flags); }
		//このソケットへつながっている先で受信する
		int Receive(char* buffer, size_t length, int flags);
		template<size_t _size> int Receive(char(&buffer)[_size], int flags) { return Receive(buffer, _size, flags); }
		///////////////////////////////////////////////////////////////////
		//	主にUDPで使用
		///////////////////////////////////////////////////////////////////
		//指定されたアドレスへデータを送信する。
		int SendTo(const char* buffer, size_t length, int flags, Address* to_addr);
		template<size_t _size> int SendTo(const char(&buffer)[_size], int flags, Address* to_addr) { return SendTo(buffer, _size, flags, to_addr); }
		//データと送信側のアドレスを受信する
		int ReceiveFrom(char* buffer, size_t length, int flags, Address* from_addr, int* from_addr_length);
		template<size_t _size> int ReceiveFrom(char(&buffer)[_size], int flags, Address* from_addr, int* from_addr_length) { return ReceiveFrom(buffer, _size, flags, from_addr, from_addr_length); }
		//---------------------------------------------------------------------------
		//---------------------------------------------------------------------------
	};
	class SocketList {
	private:
		//オリジナル
		static fd_set fds;
		static timeval timeout;
	public:
		static void Initialize();
		static void Register(Socket* socket);
		static void UnRegister(Socket* socket);
		static int ReadSelect();
		static int WriteSelect();
		static int IsSet(Socket* socket);
		static void SetTimeOut(int sec, int micro);
	};

}