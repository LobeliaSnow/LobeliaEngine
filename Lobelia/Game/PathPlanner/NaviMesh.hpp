#pragma once
namespace Lobelia::Game {
	class NaviMeshGraph {
	private:
		struct TriPolygon {
			Math::Vector3 pos[3];
			Math::Vector3 center;
			int index;
			bool operator==(const TriPolygon& polygon) { return (index == polygon.index); }
		};
	public:
		struct Edge {
			int index0;
			int index1;
			Edge(int index0, int index1) :index0(index0), index1(index1) {}
			Edge() :Edge(-1, -1) {}
			bool operator==(const Edge& e) { return (index0 == e.index0&&index1 == e.index1); }
			bool operator<(const Edge& e) { return ((index0 < e.index0) || (index0 == e.index0&&index1 < e.index1)); }
		};
		struct Node {
			Math::Vector3 pos;
			float buildLength;
			int index;
			Node(const Math::Vector3& pos, int index) :pos(pos), index(index) {}
			Node() = default;
		};
	public:
		NaviMeshGraph(const char* model_path, const char* mt_path);
		NaviMeshGraph(const char* navi_path);
		~NaviMeshGraph() = default;
		void ConnectNode(int index0, int index1);
		void DisconnectNode(int index0, int index1);
		//第二引数は特に意図がなければtrueのままで大丈夫です
		void AddNode(const Math::Vector3& node, bool update = true);
		bool EraseNode(int index);
		int GetTriangleCount() { return triangleCount; }
		int GetEdgeCount() { return i_cast(edgeList.size()); }
		int GetNodeCount() { return nodeCount; }
		const std::vector<TriPolygon>& GetTriangleList() { return triangleList; }
		const std::list<Edge>& GetEdgeList() { return edgeList; }
		const std::list<Node>& GetNodeList() { return nodeList; }
		void CreateVectorNodeList(std::vector<NaviMeshGraph::Node>& node_list);
		void CreateVectorEdgeList(std::vector<NaviMeshGraph::Edge>& edge_list);
	private:
		void NodeLoad(Utility::FileController& fc);
		void EdgeLoad(Utility::FileController& fc);
		void ConnectionUnique();
		//ノードにインデックスを割り振る
		void UpdateNode();
		void BuildNode();
		void BuildEdge();
	private:
		int triangleCount;
		int nodeCount;
		std::vector<TriPolygon> triangleList;
		std::list<Edge> edgeList;
		std::list<Node> nodeList;
	};
	class ApplyNaviToDijkstra {
	public:
		static void Apply(DijkstraEngineVector3* engine, NaviMeshGraph* graph, float scale);
	};
}