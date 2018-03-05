#include "Lobelia.hpp"
#include "NetworkObject.hpp"

namespace Lobelia::Game {
	//-------------------------------------------------------------------------------------------------------
	// Factory
	//-------------------------------------------------------------------------------------------------------
	class UDPBroadcastServerFactory :public BaseSocketFactoryObject {
	public:
		std::shared_ptr<Network::Socket> Create()override {
			std::shared_ptr<Network::Socket> udp = std::make_unique<Network::Socket>(0, AF_INET, SOCK_DGRAM);
			BOOL yes = 1;
			udp->SetOption(SOL_SOCKET, SO_BROADCAST, reinterpret_cast<char*>(&yes), sizeof(yes));
			udp->BlockingModeSetting(Network::BlockingMode::NO_BLOCK);
			return udp;
		}
		static Network::Address GetAddress() { return Network::Address("255.255.255.255"); }
	};
	class UDPBroadcastClientFactory :public BaseSocketFactoryObject {
	public:
		std::shared_ptr<Network::Socket> Create()override {
			std::shared_ptr<Network::Socket> udp = std::make_shared<Network::Socket>(0, AF_INET, SOCK_DGRAM);
			Network::Address address = {};
			udp->Bind(&address);
			BOOL yes = 1;
			udp->SetOption(SOL_SOCKET, SO_BROADCAST, reinterpret_cast<char*>(&yes), sizeof(yes));
			udp->BlockingModeSetting(Network::BlockingMode::NO_BLOCK);
			return udp;
		}
	};
	class TCPServerFactory :public BaseSocketFactoryObject {
	public:
		//Listneを忘れずに
		std::shared_ptr<Network::Socket> Create()override {
			std::shared_ptr<Network::Socket> tcp = std::make_unique<Network::Socket>(0);
			Network::Address address = {};
			tcp->Bind(&address);
			return tcp;
		}
	};
	class TCPConnectionClientFactory :public BaseSocketFactoryObject {
	private:
	public:
		std::shared_ptr<Network::Socket> Create()override {
			return std::make_shared<Network::Socket>();
		}
	};
	class TCPClientFactory :public BaseSocketFactoryObject {
	private:
	public:
		std::shared_ptr<Network::Socket> Create()override {
			return std::make_shared<Network::Socket>(0);
		}
	};
	SocketFactory::SocketFactory() {
		//登録
		factory.insert(std::make_pair<SocketType, std::unique_ptr<BaseSocketFactoryObject>>(SocketType::UDP_BROAD_CAST_SERVER, std::make_unique<UDPBroadcastServerFactory>()));
		factory.insert(std::make_pair<SocketType, std::unique_ptr<BaseSocketFactoryObject>>(SocketType::UDP_BROAD_CAST_CLIENT, std::make_unique<UDPBroadcastClientFactory>()));
		factory.insert(std::make_pair<SocketType, std::unique_ptr<BaseSocketFactoryObject>>(SocketType::TCP_SERVER, std::make_unique<TCPServerFactory>()));
		factory.insert(std::make_pair<SocketType, std::unique_ptr<BaseSocketFactoryObject>>(SocketType::TCP_CONNECT_CLIENT, std::make_unique<TCPConnectionClientFactory>()));
		factory.insert(std::make_pair<SocketType, std::unique_ptr<BaseSocketFactoryObject>>(SocketType::TCP_CLIENT, std::make_unique<TCPClientFactory>()));
	}
	std::shared_ptr<Network::Socket> SocketFactory::Create(SocketType type) {
		if (factory.find(type) == factory.end())	 return nullptr;
		return factory[type]->Create();
	}
	//-------------------------------------------------------------------------------------------------------
	// Network
	//-------------------------------------------------------------------------------------------------------
	NetworkObject::NetworkObject(SocketType type) :socket(SocketFactory::GetInstance()->Create(type)) {	}
	//-------------------------------------------------------------------------------------------------------
	// UDP
	//-------------------------------------------------------------------------------------------------------
	namespace {
		int CONNECTION_APP_ID = 119;
	}
	UDPServer::UDPServer(const char* server_name)
		: NetworkObject(SocketType::UDP_BROAD_CAST_SERVER), broadcast(UDPBroadcastServerFactory::GetAddress()) {
		strcpy_s(serverName, server_name);
	}
	void UDPServer::OpenNotify() { GetSocket()->SendTo(serverName, 0, &broadcast); }
	bool UDPServer::ConnectionNotify() {
		NetworkBuffer<int> data;
		if (!(GetSocket()->Receive(data.buffer, 0) < 1)) {
			return (data.data == CONNECTION_APP_ID);
		}
		return false;
	}
	UDPClient::UDPClient() : NetworkObject(SocketType::UDP_BROAD_CAST_CLIENT) {	}
	NetworkServerData UDPClient::ServerDetection() {
		static NetworkServerData ret = {};
		NetworkServerData server = {};
		Network::Address from = {};
		int sockAddressSize = 0;
		if (!(GetSocket()->ReceiveFrom(server.name, 0, &from, &sockAddressSize) < 1)) {
			server.address = from;
			//既に登録済みである場合は登録しない
			if (std::find(list.begin(), list.end(), server) != list.end())return ret;
			list.push_back(server);
			serverCount++;
			return server;
		}

		return ret;
	}
	void UDPClient::SendConnectionNotify(NetworkServerData server) {
		NetworkBuffer<int> data;
		data.data = CONNECTION_APP_ID;
		//接続要求送信
		GetSocket()->SendTo(data.buffer, 0, &server.address);
	}
	void UDPClient::Clear() {
		list.clear();
		serverCount = 0;
	}
	int UDPClient::GetServerCount() { return serverCount; }
	NetworkServerData UDPClient::GetServerData(int index) {
		if (s_cast<UINT>(index) >= s_cast<UINT>(serverCount))STRICT_THROW("範囲外のインデックスが指定されました");
		return *std::next(list.begin(), index);
	}
	//-------------------------------------------------------------------------------------------------------
	// TCP
	//-------------------------------------------------------------------------------------------------------
	TCPServer::TCPServer(int listne_max) :NetworkObject(SocketType::TCP_SERVER) {
		GetSocket()->Listen(listne_max);
	};
	std::shared_ptr<Network::Socket> TCPServer::ConnectionApproval() {
		std::shared_ptr<Network::Socket> client = SocketFactory::GetInstance()->Create(SocketType::TCP_CONNECT_CLIENT);
		Network::Address address;
		//クライアント接続待ち
		GetSocket()->Accept(client.get(), &address);
		client->BlockingModeSetting(Network::BlockingMode::NO_BLOCK);
		return client;
	}
	std::shared_ptr<Network::Socket> TCPServer::ConnectionLocal(class TCPClient* client) {
		std::shared_ptr<Network::Socket> ret;
		std::thread thread = std::thread([&] {ret = ConnectionApproval(); });
		client->ConnectionLocalHost();
		thread.join();
		return ret;
	}
	TCPClient::TCPClient() :NetworkObject(SocketType::TCP_CLIENT) {}
	TCPClient::~TCPClient() {	}
	int TCPClient::CennectionServer(NetworkServerData data) {
		//inet_ntoaは古いAPI 新しいものに乗り換える
		Network::Address address(inet_ntoa(data.address.addr.sin_addr));
		int ret = GetSocket()->Connect(&address);
		int error = GetLastError();
		if (ret == SOCKET_ERROR)return -1;
		GetSocket()->BlockingModeSetting(Network::BlockingMode::NO_BLOCK);
		Network::SocketList::Register(GetSocket().get());
		return ret;
	}
	int TCPClient::ConnectionLocalHost() {
		Network::Address address = {};
		address.addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
		int ret = GetSocket()->Connect(&address);
		if (ret == -1) {
			int error = GetLastError();
			char buffer[64] = {};
			sprintf_s(buffer, "サーバーへの接続に失敗しました %d", error);
			STRICT_THROW(buffer);
		}
		GetSocket()->BlockingModeSetting(Network::BlockingMode::NO_BLOCK);
		Network::SocketList::Register(GetSocket().get());
		return ret;
	}

	//-------------------------------------------------------------------------------------------------------
	// 送受信サポートクラス
	//-------------------------------------------------------------------------------------------------------
	NetworkSender::NetworkSender(std::weak_ptr<Network::Socket> socket) :socket(socket) {	}
	bool NetworkSender::Send(int slot, char* buffer, int size) {
		std::shared_ptr<Network::Socket> work = socket.lock();
		if (Network::SocketList::WriteSelect() <= 0) return false;
		NetworkBuffer<int> data = {};
		data.data = slot;
		if (Network::SocketList::IsSet(work.get())) {
			//スロットの送信
			work->Send(data.buffer, sizeof(data), 0);
			//実データの送信
			work->Send(buffer, size, 0);
			return true;
		}
		return false;
	}

	NetworkReciver::NetworkReciver(std::weak_ptr<Network::Socket> socket) :socket(socket) {

	}
	NetworkReciver::~NetworkReciver() = default;
	int NetworkReciver::ReciveSlot() {
		NetworkBuffer<int> data = {};
		std::shared_ptr<Network::Socket> worker = socket.lock();
		int isAlive = worker->Receive(data.buffer, 0);
		if (isAlive == 0)return DISCONNECTION;
		if (isAlive < 1) {
			int error = GetLastError();
			if (error != 0 && error != WSAEWOULDBLOCK)return false;
			return INT_MIN;
		}
		return data.data;
	}
	bool NetworkReciver::IsExistReciver(int slot) {
		return (reciver.find(slot) != reciver.end());
	}
	void NetworkReciver::AddReciverObject(int slot, NetworkReciverMethod* reciver_object) {
		if (IsExistReciver(slot))STRICT_THROW("キーが重複しました");
		reciver[slot] = std::shared_ptr<NetworkReciverMethod>(reciver_object);
	}
	void NetworkReciver::DeleteReciverObject(int slot) {
		auto& iterator = reciver.find(slot);
		reciver.erase(iterator);
	}
	int NetworkReciver::Recive() {
		std::shared_ptr<Network::Socket> worker = socket.lock();
		//受信可能なソケットが見つかるまで待つ
		if (Network::SocketList::ReadSelect() <= 0) {
			int error = GetLastError();
			if (error != 0 && error != WSAEWOULDBLOCK)return -1;
			return 0;
		}
		//対象がリストにいるかどうか調べて
		if (Network::SocketList::IsSet(worker.get())) {
			int slot = ReciveSlot();
			//切断された場合
			if (slot == DISCONNECTION)return -1;
			if (slot == INT_MIN)return 0;
			//受信するためのオブジェクトが存在すれば
			if (IsExistReciver(slot)) {
				(*reciver[slot])(socket);
				int error = GetLastError();
				if (error != 0 && error != WSAEWOULDBLOCK)return -1;
				return 1;
			}
		}
		return 0;
	}
	NetworkReciverMethod* NetworkReciver::GetReciverObject(int slot) {
		if (!IsExistReciver(slot))STRICT_THROW("関連付けされたオブジェクトが登録されていません");
		return reciver[slot].get();
	}
	NetworkProcedure::NetworkProcedure(std::shared_ptr<Network::Socket> socket) :socket(socket), sender(std::make_unique<NetworkSender>(socket)), reciver(std::make_unique<NetworkReciver>(socket)) {
		SetFrameProcessCount(5);
	}
	NetworkProcedure::~NetworkProcedure() {
		Network::SocketList::UnRegister(socket.get());
	}
	void NetworkProcedure::SetFrameProcessCount(int count) {
		frameProcessCount = count;
	}
	void NetworkProcedure::Send(int slot, char* buffer, int size) {
		sender->Send(slot, buffer, size);
	}
	void NetworkProcedure::AddReciverObject(int slot, NetworkReciverMethod* reciver_object) {
		reciver->AddReciverObject(slot, reciver_object);
	}
	void NetworkProcedure::DeleteReciverObject(int slot) {
		reciver->DeleteReciverObject(slot);
	}
	bool NetworkProcedure::Update() {
		isRecive = false;
		for (int i = 0; i < frameProcessCount; i++) {
			int ret = reciver->Recive();
			if (ret > 0)isRecive = true;
			//受信
			if (!ret)return false;
		}
		return true;
	}
	NetworkReciverMethod* NetworkProcedure::GetReciverObject(int slot) { return reciver->GetReciverObject(slot); }
}
