#pragma once
#define ALIVE_INTERVAL 2000
#define SERVER_NAME_LENGHT 32
namespace Lobelia::Game {
	struct NetworkServerData {
		//サーバー名
		char name[SERVER_NAME_LENGHT] = {};
		//サーバーへのアドレス
		Lobelia::Network::Address address;
		bool operator <(const NetworkServerData& s) {
			return (strcmp(name, s.name) < 0);
		}
		bool operator ==(const NetworkServerData& s) {
			return (address.addr.sin_addr.S_un.S_addr == s.address.addr.sin_addr.S_un.S_addr);
		}
	};
	//-------------------------------------------------------------------------------------------------------
	// Buffer
	//-------------------------------------------------------------------------------------------------------
	//ネットワーク送受信用バッファ
	template<class T> struct NetworkBuffer {
		union {
			T data;
			char buffer[sizeof(T)];
		};
	};
	//-------------------------------------------------------------------------------------------------------
	// Factory
	//-------------------------------------------------------------------------------------------------------
	enum class SocketType {
		UDP_BROAD_CAST_SERVER, UDP_BROAD_CAST_CLIENT,
		TCP_SERVER, TCP_CONNECT_CLIENT, TCP_CLIENT,
	};
	class BaseSocketFactoryObject Interface {
	public:
		BaseSocketFactoryObject() = default;
		virtual ~BaseSocketFactoryObject() = default;
		virtual std::shared_ptr<Network::Socket> Create() = 0;
	};
	class SocketFactory :public Utility::Singleton<SocketFactory> {
		friend class Utility::Singleton<SocketFactory>;
	private:
		std::map<SocketType, std::unique_ptr<BaseSocketFactoryObject>> factory;
	private:
		SocketFactory();
		~SocketFactory() = default;
	public:
		std::shared_ptr<Network::Socket> Create(SocketType type);
	};
	//-------------------------------------------------------------------------------------------------------
	// NetworkObject
	//-------------------------------------------------------------------------------------------------------
	class NetworkObject {
	private:
		std::shared_ptr<Network::Socket> socket;
	public:
		NetworkObject(SocketType type);
		virtual ~NetworkObject() = default;
		const std::shared_ptr<Network::Socket>& GetSocket() { return socket; }
	};
	//-------------------------------------------------------------------------------------------------------
	// UDP
	//-------------------------------------------------------------------------------------------------------
	class UDPServer :public NetworkObject {
	private:
		Network::Address broadcast;
		char serverName[SERVER_NAME_LENGHT];
	public:
		UDPServer(const char* server_name);
		~UDPServer() = default;
		//サーバー通知
		void OpenNotify();
		bool ConnectionNotify();
	};
	class UDPClient :public NetworkObject {
	private:
		std::list<NetworkServerData> list;
		int serverCount;
	public:
		UDPClient();
		~UDPClient() = default;
		NetworkServerData ServerDetection();
		void SendConnectionNotify(NetworkServerData server);
		int GetServerCount();
		void Clear();
		NetworkServerData GetServerData(int index);
	};
	//-------------------------------------------------------------------------------------------------------
	// TCP
	//-------------------------------------------------------------------------------------------------------
	class TCPServer :public NetworkObject {
	private:
	public:
		TCPServer(int listne_max);
		~TCPServer() = default;
		//通信用のソケットが返ります
		std::shared_ptr<Network::Socket> ConnectionApproval();
		std::shared_ptr<Network::Socket> ConnectionLocal(class TCPClient* client);
	};
	class TCPClient :public NetworkObject {
	private:
	public:
		TCPClient();
		~TCPClient();
		int CennectionServer(NetworkServerData data);
		int ConnectionLocalHost();
	};
	//-------------------------------------------------------------------------------------------------------
	// 送受信サポートクラス
	//-------------------------------------------------------------------------------------------------------
	class NetworkSender {
	private:
		std::weak_ptr<Network::Socket> socket;
	public:
		NetworkSender(std::weak_ptr<Network::Socket> socket);
		~NetworkSender() = default;
		template<class T> bool Send(int slot, const T& data) {
			NetworkBuffer<T> buffer = {};
			buffer.data = data;
			return Send(slot, buffer.buffer, sizeof(T));
		}
		bool Send(int slot, char* buffer, int size);
	};
	//データを受信するためのオブジェクト
	//継承前提
	class NetworkReciverMethod Interface {
	private:
	protected:
		template<class T> int Recive(T* data, std::weak_ptr<Network::Socket> socket) {
			std::shared_ptr<Network::Socket> worker = socket.lock();
			NetworkBuffer<T> buffer = {};
			int ret = 0;
			float timeout = ALIVE_INTERVAL;
			do {
				ret = worker->Receive(buffer.buffer, 0);
				int error = GetLastError();
				if (error != WSAEWOULDBLOCK)timeout -= Application::GetInstance()->GetProcessTime();
				//タイムアウトによりサーバーから切断される
				if (timeout < 0.0f) THROW_E;
			} while (ret <= 0);
			*data = buffer.data;
			return ret;
		}
		//対応するスロットが届いたときにこの関数が呼ばれる
		virtual void Recive(std::weak_ptr<Network::Socket> socket) = 0;
	public:
		NetworkReciverMethod() = default;
		virtual ~NetworkReciverMethod() = default;
		void operator()(std::weak_ptr<Network::Socket> socket) { Recive(socket); }
	};
	//指定された型のデータを受信するためのオブジェクト
	template<class T> class ReciverBase :public NetworkReciverMethod {
	private:
		T data;
		int isRecive;
	private:
		void Recive(std::weak_ptr<Network::Socket> socket) {
			isRecive = NetworkReciverMethod::Recive<T>(&data, socket);
		}
	public:
		ReciverBase() :data{}, isRecive(0){}
		~ReciverBase() = default;
		//一度呼び出せば結果はクリアされます
		bool IsRecive() {
			bool ret = (isRecive != 0);
			isRecive = 0;
			return ret;
		}
		T GetData() { return data; }
	};
#define DISCONNECTION -1
	class NetworkReciver {
	private:
		std::weak_ptr<Network::Socket> socket;
		std::map<int, std::shared_ptr<NetworkReciverMethod>> reciver;
	private:
		int ReciveSlot();
		bool IsExistReciver(int slot);
	public:
		NetworkReciver(std::weak_ptr<Network::Socket> socket);
		~NetworkReciver();
		//自動でスマポ管理されます、間違えてもスマポの内容突っ込まないでください
		void AddReciverObject(int slot, NetworkReciverMethod* reciver_object);
		void DeleteReciverObject(int slot);
		int Recive();
		NetworkReciverMethod* GetReciverObject(int slot);
	};
	class NetworkProcedure {
	private:
		std::shared_ptr<Network::Socket> socket;
		std::unique_ptr<NetworkSender> sender;
		std::unique_ptr<NetworkReciver> reciver;
		int frameProcessCount;
		bool isRecive;
	public:
		NetworkProcedure(std::shared_ptr<Network::Socket> socket);
		~NetworkProcedure();
		void SetFrameProcessCount(int count);
		template<class T> bool Send(int slot, const T& data) { return sender->Send<T>(slot, data); }
		void Send(int slot, char* buffer, int size);
		void AddReciverObject(int slot, NetworkReciverMethod* reciver_object);
		void DeleteReciverObject(int slot);
		bool Update();
		bool IsRecive() { return isRecive; }
		NetworkReciverMethod* GetReciverObject(int slot);
		std::shared_ptr<Network::Socket> GetSocket() { return socket; }
	};
}
