#include "Lobelia.hpp"

namespace Lobelia::Game {
	NaviMeshGraph::NaviMeshGraph(const char* model_path, const char* mt_path) {
		//モデルデータ取得
		std::unique_ptr<Graphics::ModelData> model = std::make_unique<Graphics::ModelData>(model_path, mt_path);
		int meshCount = model->allMeshVertexCountSum;
		if (meshCount % 3 != 0)STRICT_THROW("メッシュが不適格です");
		triangleCount = meshCount / 3;
		//三角形の列挙
		triangleList.resize(triangleCount);
		for (int tri = 0; tri < triangleCount; tri++) {
			triangleList[tri].index = tri;
			Math::Vector3 sum = {};
			for (int i = 0; i < 3; i++) {
				triangleList[tri].pos[i] = model->vertices[tri * 3 + i].pos.xyz;
				//Fbxではxが反転するため
				triangleList[tri].pos[i].x *= -1;
				sum += triangleList[tri].pos[i];
			}
			//重心算出(ここをいじったほうがいいかも)
			triangleList[tri].center = sum / 3.0f;
		}
		//ノードを構築
		BuildNode();
		//ノードにインデックスを割り振る
		UpdateNode();
		//エッジの構築
		BuildEdge();
	}
	NaviMeshGraph::NaviMeshGraph(const char* navi_path) {
		Utility::FileController fc = {};
		fc.Open(navi_path, Utility::FileController::OpenMode::Read);
		char command[10] = {};
		while (true) {
			int ret = i_cast(fc.Scan("%s", command, 10));
			if (ret == EOF)return;
			if (strcmp(command, "Node") == 0)NodeLoad(fc);
			else if (strcmp(command, "Edge") == 0)EdgeLoad(fc);
		}
	}
	void NaviMeshGraph::NodeLoad(Utility::FileController& fc) {
		char temp[6] = {};
		fc.Scan("%s %d", temp, 6, &nodeCount);
		for (int i = 0; i < nodeCount; i++) {
			Math::Vector3 node = {};
			fc.Scan("%f %f %f", &node.x, &node.y, &node.z);
			AddNode(node, false);
		}
		//インデックスを割り振る
		UpdateNode();
	}
	void NaviMeshGraph::EdgeLoad(Utility::FileController& fc) {
		char temp[6] = {};
		int edgeCount = -1;
		fc.Scan("%s %d", temp, 6, &edgeCount);
		for (int i = 0; i < edgeCount; i++) {
			int index0 = -1, index1 = -1;
			fc.Scan("%d %d", &index0, &index1);
			ConnectNode(index0, index1);
		}
	}
	void NaviMeshGraph::ConnectionUnique() {
		edgeList.sort();
		edgeList.unique();
	}
	void NaviMeshGraph::UpdateNode() {
		nodeCount = 0;
		for (auto&& node = nodeList.begin(); node != nodeList.end(); node++) {
			node->index = nodeCount;
			nodeCount++;
		}
	}
	void NaviMeshGraph::ConnectNode(int index0, int index1) {
		if (index0 == index1)return;
		if (index0 < index1)	edgeList.push_back(Edge(index0, index1));
		else edgeList.push_back(Edge(index1, index0));
	}
	void NaviMeshGraph::DisconnectNode(int index0, int index1) {
		if (index0 == index1)return;
		edgeList.erase(std::remove(edgeList.begin(), edgeList.end(), Edge(index0, index1)), edgeList.end());
	}
	void NaviMeshGraph::AddNode(const Math::Vector3& node, bool update) {
		nodeList.push_back(Node(node, -1));
		//インデックス再割り振り
		if (update)UpdateNode();
	}
	bool NaviMeshGraph::EraseNode(int index) {
		if (index >= nodeCount)return false;
		nodeList.erase(std::next(nodeList.begin(), index));
		//インデックス再割り振り
		UpdateNode();
		//エッジリストインデックス修正
		for (auto&& edge = edgeList.begin(); edge != edgeList.end();) {
			if (edge->index0 > index) edge->index0--;
			if (edge->index1 > index) edge->index1--;
			if (edge->index0 == index || edge->index1 == index)edge = edgeList.erase(edge);
			else edge++;
		}
		return true;
	}
	void NaviMeshGraph::BuildNode() {
		std::vector<Node> node;
		std::vector<TriPolygon> triangleStorage = triangleList;
		int triCount = -1; int nodeCount = 0;
		//算出していない三角形がなくなるまで
		while (triCount = i_cast(triangleStorage.size())) {
			int index = -1; float length = 9999.0f;
			for (int i = 1; i < triCount; i++) {
				Math::Vector3 dist = triangleStorage[0].center - triangleStorage[i].center;
				float l = dist.Length();
				//最短の三角形を探す
				if (length > l) {
					length = l;
					index = i;
				}
			}
			//ノードの追加
			Node node;
			if (index == -1)node.pos = triangleStorage[0].center;
			else node.pos = (triangleStorage[0].center + triangleStorage[index].center) / 2.0f;
			node.buildLength = length * 1.5f;
			nodeList.push_back(node);
			//今回出たペアの削除
			triangleStorage.erase(std::find(triangleStorage.begin(), triangleStorage.end(), triangleStorage[index]));
			triangleStorage.erase(std::find(triangleStorage.begin(), triangleStorage.end(), triangleStorage[0]));
		}
	}
	void NaviMeshGraph::BuildEdge() {
		for each(auto&& node0 in nodeList) {
			for each(auto&& node1 in nodeList) {
				if (node0.index == node1.index)continue;
				Math::Vector3 dist = node0.pos - node1.pos;
				if (dist.Length() < node0.buildLength + node1.buildLength)ConnectNode(node0.index, node1.index);
			}
		}
		ConnectionUnique();
	}
	void NaviMeshGraph::CreateVectorNodeList(std::vector<NaviMeshGraph::Node>& node_list) {
		node_list.resize(nodeList.size());
		int i = 0;
		for each(auto&& node in nodeList) {
			node_list[i] = node;
			i++;
		}
	}
	void NaviMeshGraph::CreateVectorEdgeList(std::vector<NaviMeshGraph::Edge>& edge_list) {
		edge_list.resize(edgeList.size());
		int i = 0;
		for each(auto&& edge in edgeList) {
			edge_list[i] = edge;
			i++;
		}

	}
	void ApplyNaviToDijkstra::Apply(DijkstraEngineVector3* engine, NaviMeshGraph* graph, float scale) {
		for each(auto&& node in graph->GetNodeList()) {
			engine->AddNode(node.pos*scale);
		}
		std::vector<NaviMeshGraph::Node> nodeList = {};
		graph->CreateVectorNodeList(nodeList);
		for each(auto&& edge in graph->GetEdgeList()) {
			engine->ConnectNode(nodeList[edge.index0].pos*scale, nodeList[edge.index1].pos*scale);
		}
	}
}