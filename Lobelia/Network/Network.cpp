#include "Network.hpp"
#include "Common/Common.hpp"
#include "Exception/Exception.hpp"

namespace Lobelia::Network {
	WSADATA System::wsaData;
	int System::Startup() {
		//������ ���W���[�o�[�W���� ������ �}�C�i�[�o�[�W����
		return WSAStartup(MAKEWORD(SDKVersion, 0), &wsaData);
	}
	int System::Cleanup() { return WSACleanup(); }
	Address::~Address() = default;
	void Address::Initialize(int address_family, u_short port) {
		addr.sin_family = address_family;
		//htons , htonl ... �l�b�g���[�N�o�C�g�I�[�_�[�ւ̕ϊ�
		//Host To Network Short
		addr.sin_port = htons(port);
	}

	Socket::Socket(int protocol, int address_family, int type) :sock(socket(address_family, type, protocol)) {
	}
	Socket::Socket() : sock(INVALID_SOCKET) {
	}
	Socket::~Socket() { Close(); }

	int Socket::Listen(int max_connect) {
		//�ڑ��҂��󋵂ɂ���
		//������ �����܂Ŏ󂯕t���邩���߂�
		return listen(sock, max_connect);
	}
	void Socket::Accept(Socket* s, Address* client) {
		int length = sizeof(client->addr);
		//�ʐM�p�\�P�b�g����ɓ���
		s->sock = accept(sock, reinterpret_cast<sockaddr*>(&client->addr), &length);
	}
	int Socket::Close() {
		if (sock == 0)return 0;
		int ret = closesocket(sock);
		sock = 0;
		return ret;
	}
	int Socket::Connect(Address* server) {
		//�T�[�o�[�Ɛڑ�����
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

	PingEngine::PingEngine(std::weak_ptr<Socket> socket) :socket(socket), state(State::NONE), timeoutTime(1000.0f), pingTime(1000.0f) {}
	PingEngine::~PingEngine() = default;
	//�^�C���A�E�g���鎞�Ԃ̐ݒ�
	void PingEngine::SetTimeoutTime(float time) {
		if (time <= 0.0f)	STRICT_THROW("���ԂƂ��ĕs�K�i�Ȓl�ł�");
		timeoutTime = time;
	};
	//�����ping�v���𑗂�
	void PingEngine::PingSend() {
		if (state != State::NONE)return;
		std::shared_ptr<Socket> sock = socket.lock();
		//�������ʂ̂��߂�pingID
		out.ping = rand();
		//ping���M
		sock->Send(out.buffer, 0);
		//�^�C�}�[�J�n
		timer.Begin();
		state = State::WAIT;
	}
	//ping���ʉ����҂�
	PingEngine::State PingEngine::PingReceive() {
		//���M���Ă��Ȃ���΋A���Ă���͂����Ȃ��̂ŁB
		if (state == State::NONE)	return state;
		State ret = state;
		std::shared_ptr<Socket> sock = socket.lock();
		timer.End();
		if (!(sock->Receive(in.buffer, 0) < 1)) {
			//��������ping����
			pingTime = timer.GetMilisecondResult();
			state = State::NONE;
			//ping���M�f�[�^�Ǝ�M�f�[�^����v
			if (in.ping == out.ping) ret = State::SUCCESS;
			else ret = State::FAILED;//��v���Ȃ������ꍇ
		}
		else if (timer.GetMilisecondResult() > timeoutTime) {
			//��������ping����
			pingTime = timeoutTime;
			state = State::NONE;
			ret = State::TIMEOUT;
		}
		return ret;
	}
	float PingEngine::GetPingIntervalTime() { return pingTime; }
	//���肩���ping���N�G�X�g����������֐�
	bool PingEngine::PingRequestReceive() {
		std::shared_ptr<Socket> sock = socket.lock();
		//ping�����ł����ꍇ
		if (!(sock->Receive(in.buffer, 0) < 1)) {
			out = in;
			//ping��Ԃ�
			sock->Send(out.buffer, 0);
			return true;
		}
		return false;
	}

}