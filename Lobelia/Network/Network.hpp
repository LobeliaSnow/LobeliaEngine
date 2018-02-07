#pragma once
#include <memory>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "Common/Timer/Timer.h"
#pragma comment(lib,"wsock32.lib")
#pragma comment(lib, "ws2_32.lib")
namespace Lobelia::Network {
	//�֐����t�@�����X
	//http://www.golgo.jp/winsock.html#l_listen
	//�Q�l
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
		//�������傢���Ƃ����Ȃ����
		static constexpr const u_short PORT = 9305;
	private:
		static WSADATA wsaData;
	public:
		static int Startup();
		static int Cleanup();
		static constexpr const u_short GetPort() { return PORT; }
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
		//protocol�͊�{0����Ă��������BAF_INET��IPv4�������ASOCK_STREAM�͏������ƐM����������o�����̃o�C�g�X�g���[�����(TCP)
		Socket(int protocol, int address_family = AF_INET, int type = SOCK_STREAM);
		//��̃\�P�b�g�쐬
		Socket();
		~Socket();
		//---------------------------------------------------------------------------
		//	�T�[�o�[�T�C�h
		//---------------------------------------------------------------------------
		//�ڑ��L���[�̍쐬 
		int Listen(int max_connect);
		//�ڑ��v����t
		//������ ��̃\�P�b�g ����\�P�b�g�ō���̒ʐM
		//������ ��̃A�h���X �ڑ�����̏�񂪊i�[�����
		void Accept(Socket* s, Address* client);
		int Close();
		//---------------------------------------------------------------------------
		//---------------------------------------------------------------------------

		//---------------------------------------------------------------------------
		//	�N���C�A���g�T�C�h
		//---------------------------------------------------------------------------
		//�T�[�o�[�ɐڑ�
		int Connect(Address* server);
		//---------------------------------------------------------------------------
		//---------------------------------------------------------------------------

		//---------------------------------------------------------------------------
		//	���ʕ�
		//---------------------------------------------------------------------------
		//���[�J���A�h���X�ƃ\�P�b�g���֘A�t����
		int Bind(Address* address);
		void BlockingModeSetting(BlockingMode mode);
		int SetOption(int level, int option, char* option_value, size_t option_length);
		///////////////////////////////////////////////////////////////////
		//	���TCP�Ŏg�p
		///////////////////////////////////////////////////////////////////
		//���̃\�P�b�g�ւȂ����Ă����ɑ��M����
		//�߂�l�͎��ۂɑ��M���ꂽ�o�C�g�����́ASOCKET_ERROR
		int Send(const char* buffer, size_t length, int flags = 0);
		template<size_t _size> int Send(const char(&buffer)[_size], int flags) { return Send(buffer, _size, flags); }
		//���̃\�P�b�g�ւȂ����Ă����Ŏ�M����
		int Receive(char* buffer, size_t length, int flags);
		template<size_t _size> int Receive(char(&buffer)[_size], int flags) { return Receive(buffer, _size, flags); }
		///////////////////////////////////////////////////////////////////
		//	���UDP�Ŏg�p
		///////////////////////////////////////////////////////////////////
		//�w�肳�ꂽ�A�h���X�փf�[�^�𑗐M����B
		int SendTo(const char* buffer, size_t length, int flags, Address* to_addr);
		template<size_t _size> int SendTo(const char(&buffer)[_size], int flags, Address* to_addr) { return SendTo(buffer, _size, flags, to_addr); }
		//�f�[�^�Ƒ��M���̃A�h���X����M����
		int ReceiveFrom(char* buffer, size_t length, int flags, Address* from_addr, int* from_addr_length);
		template<size_t _size> int ReceiveFrom(char(&buffer)[_size], int flags, Address* from_addr, int* from_addr_length) { return ReceiveFrom(buffer, _size, flags, from_addr, from_addr_length); }
		//---------------------------------------------------------------------------
		//---------------------------------------------------------------------------
	};
	class SocketList {
	private:
		//�I���W�i��
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
	class PingEngine {
	public:
		enum class State { NONE, WAIT, SUCCESS, FAILED, TIMEOUT };
	private:
		struct Ping {
			union {
				int ping;
				char buffer[sizeof(int)];
			};
		};
	private:
		std::weak_ptr<Socket> socket;
		State state;
		float timeoutTime;
		Ping in;
		Ping out;
		Timer timer;
		float pingTime;
	public:
		PingEngine(std::weak_ptr<Socket> socket);
		~PingEngine();
		//�^�C���A�E�g���鎞�Ԃ̐ݒ�
		void SetTimeoutTime(float time);
		//�����ping�v���𑗂�
		void PingSend();
		//ping���ʉ����҂�
		State PingReceive();
		float GetPingIntervalTime();
		//���肩���ping���N�G�X�g����������֐�
		bool PingRequestReceive();
	};

}