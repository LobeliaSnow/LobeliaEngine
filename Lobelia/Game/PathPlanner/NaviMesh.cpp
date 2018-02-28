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
	void NaviMeshGraph::AddNode(const Math::Vector3& node) {
		nodeList.push_back(Node(node, -1));
		//インデックス再割り振り
		UpdateNode();
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
		while (triCount = triangleStorage.size()) {
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
				if (dist.Length() < 1.0f)ConnectNode(node0.index, node1.index);
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
}