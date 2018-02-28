#include "Lobelia.hpp"

namespace Lobelia::Game {
	NaviMeshGraph::NaviMeshGraph(const char* model_path, const char* mt_path) {
		//���f���f�[�^�擾
		std::unique_ptr<Graphics::ModelData> model = std::make_unique<Graphics::ModelData>(model_path, mt_path);
		int meshCount = model->allMeshVertexCountSum;
		if (meshCount % 3 != 0)STRICT_THROW("���b�V�����s�K�i�ł�");
		triangleCount = meshCount / 3;
		//�O�p�`�̗�
		triangleList.resize(triangleCount);
		for (int tri = 0; tri < triangleCount; tri++) {
			triangleList[tri].index = tri;
			Math::Vector3 sum = {};
			for (int i = 0; i < 3; i++) {
				triangleList[tri].pos[i] = model->vertices[tri * 3 + i].pos.xyz;
				//Fbx�ł�x�����]���邽��
				triangleList[tri].pos[i].x *= -1;
				sum += triangleList[tri].pos[i];
			}
			//�d�S�Z�o(���������������ق�����������)
			triangleList[tri].center = sum / 3.0f;
		}
		//�m�[�h���\�z
		BuildNode();
		//�m�[�h�ɃC���f�b�N�X������U��
		UpdateNode();
		//�G�b�W�̍\�z
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
		//�C���f�b�N�X������U��
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
		//�C���f�b�N�X�Ċ���U��
		if (update)UpdateNode();
	}
	bool NaviMeshGraph::EraseNode(int index) {
		if (index >= nodeCount)return false;
		nodeList.erase(std::next(nodeList.begin(), index));
		//�C���f�b�N�X�Ċ���U��
		UpdateNode();
		//�G�b�W���X�g�C���f�b�N�X�C��
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
		//�Z�o���Ă��Ȃ��O�p�`���Ȃ��Ȃ�܂�
		while (triCount = triangleStorage.size()) {
			int index = -1; float length = 9999.0f;
			for (int i = 1; i < triCount; i++) {
				Math::Vector3 dist = triangleStorage[0].center - triangleStorage[i].center;
				float l = dist.Length();
				//�ŒZ�̎O�p�`��T��
				if (length > l) {
					length = l;
					index = i;
				}
			}
			//�m�[�h�̒ǉ�
			Node node;
			if (index == -1)node.pos = triangleStorage[0].center;
			else node.pos = (triangleStorage[0].center + triangleStorage[index].center) / 2.0f;
			nodeList.push_back(node);
			//����o���y�A�̍폜
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