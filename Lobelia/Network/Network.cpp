#include "Network.hpp"
#include "Common/Common.hpp"
#include "Exception/Exception.hpp"

namespace Lobelia::Network {
	WSADATA System::wsaData;
	int System::Startup() {
		//第一引数 メジャーバージョン 第二引数 マイナーバージョン
		return WSAStartup(MAKEWORD(SDKVersion, 0), &wsaData);
	}
	int System::Cleanup() { return WSACleanup(); }
	Address::~Address() = default;
	void Address::Initialize(int address_family, u_short port) {
		addr.sin_family = address_family;
		//htons , htonl ... ネットワークバイトオーダーへの変換
		//Host To Network Short
		addr.sin_port = htons(port);
	}

	Socket::Socket(int protocol, int address_family, int type) :sock(socket(address_family, type, protocol)) {
	}
	Socket::Socket() : sock(INVALID_SOCKET) {
	}
	Socket::~Socket() { Close(); }

	int Socket::Listen(int max_connect) {
		//接続待ち状況にする
		//第二引数 いくつまで受け付けるか決める
		return listen(sock, max_connect);
	}
	void Socket::Accept(Socket* s, Address* client) {
		int length = sizeof(client->addr);
		//通信用ソケットが手に入る
		s->sock = accept(sock, reinterpret_cast<sockaddr*>(&client->addr), &length);
	}
	int Socket::Close() {
		if (sock == 0)return 0;
		int ret = closesocket(sock);
		sock = 0;
		return ret;
	}
	int Socket::Connect(Address* server) {
		//サーバーと接続する
		return connect(sock, reinterpret_cast<sockaddr*>(&server->addr), sizeof(server->addr));
	}
	int Socket::Bind(Address* address) {
		return bind(sock, reinterpret_cast<sockaddr*>(&address->addr), sizeof(address->addr));
	}
	void Socket::BlockingModeSetting(BlockingMode mode) {
		u_long val = static_cast<u_long>(mode);
		ioctlsocket(sock, FIONBIO, &val);
	}
	int Socket::SetOption(int level, int option, char* option_value, size_t option_length) {
		return setsockopt(sock, level, option, option_value, i_cast(option_length));
	}
	int Socket::Send(const char* buffer, size_t length, int flags) {
		if (!buffer)throw;
		return send(sock, buffer, static_cast<int>(length), flags);
	}
	int Socket::Receive(char* buffer, size_t length, int flags) {
		return recv(sock, buffer, static_cast<int>(length), flags);
	}
	int Socket::SendTo(const char* buffer, size_t length, int flags, Address* to_addr) {
		return sendto(sock, buffer, i_cast(length), flags, reinterpret_cast<sockaddr*>(&to_addr->addr), sizeof(to_addr->addr));
	}
	int Socket::ReceiveFrom(char* buffer, size_t length, int flags, Address* from_addr, int* from_addr_length) {
		*from_addr_length = sizeof(from_addr->addr);
		return recvfrom(sock, buffer, i_cast(length), flags, reinterpret_cast<sockaddr*>(&from_addr->addr), from_addr_length);
	}

	fd_set SocketList::fds;
	timeval SocketList::timeout;
	void SocketList::Initialize() {
		SetTimeOut(0, 0);
		FD_ZERO(&fds);
	}
	void SocketList::Register(Socket* socket) {
		FD_SET(socket->sock, &fds);
	}
	void SocketList::UnRegister(Socket* socket) {
		FD_CLR(socket->sock, &fds);
	}
	int SocketList::ReadSelect() {
		fd_set storage;
		memcpy(&storage, &fds, sizeof(fd_set));
		return select(0, &storage, nullptr, nullptr, &timeout);
	}
	int SocketList::WriteSelect() {
		fd_set storage;
		memcpy(&storage, &fds, sizeof(fd_set));
		return select(0, nullptr, &storage, nullptr, &timeout);
	}
	int SocketList::IsSet(Socket* socket) {
		int index = -1;
		int ret = 0;
		for (int i = 0; i < i_cast(fds.fd_count); i++) {
			if (socket->sock == fds.fd_array[i])ret = FD_ISSET(socket->sock, &fds);
		}
		return ret;
	}
	void SocketList::SetTimeOut(int sec, int micro) {
		timeout.tv_sec = sec;
		timeout.tv_usec = micro;
	}

}