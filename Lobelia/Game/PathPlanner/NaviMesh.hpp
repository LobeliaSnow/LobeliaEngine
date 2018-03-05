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
			bool operator==(const Edge& e) { return (index0 == e.index0&&index1 == e.index1); }
			bool operator<(const Edge& e) { return ((index0 < e.index0) || (index0 == e.index0&&index1 < e.index1)); }
		};
		struct Node {
			Math::Vector3 pos;
			int index;
			Node(const Math::Vector3& pos, int index) :pos(pos), index(index) {}
			Node() = default;
		};
	public:
		NaviMeshGraph(const char* model_path, const char* mt_path);
		~NaviMeshGraph() = default;
		void ConnectNode(int index0, int index1);
		void DisconnectNode(int index0, int index1);
		void AddNode(const Math::Vector3& node);
		bool EraseNode(int index);
		int GetTriangleCount() { return triangleCount; }
		int GetEdgeCount() { return i_cast(edgeList.size()); }
		int GetNodeCount() { return nodeCount; }
		const std::vector<TriPolygon>& GetTriangleList() { return triangleList; }
		const std::list<Edge>& GetEdgeList() { return edgeList; }
		const std::list<Node>& GetNodeList() { return nodeList; }
		void CreateVectorNodeList(std::vector<NaviMeshGraph::Node>& node_list);
	private:
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
}