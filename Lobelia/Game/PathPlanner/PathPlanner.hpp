#pragma once
namespace Lobelia::Game {
	template<class T> struct DijkstraNodeConnect;
	//�m�[�h
	template<class T> struct DijkstraNode {
	public:
		//���
		T data;
		//�ڑ����Ă���m�[�h
		std::list<DijkstraNodeConnect<T>> connectNode;
		//�v�Z����l
		float cost;
		//�S�[���ւȂ���m�[�h
		DijkstraNode<T>* toGoal;
	public:
		DijkstraNode(const T& data) :data(data) { Reset(); }
		~DijkstraNode() = default;
		void Reset() {
			cost = -1.0f;
			toGoal = nullptr;
		}
		void RemoveConnect(DijkstraNode<T>* node) {
			for (auto&& it = connectNode.begin(); it != connectNode.end(); it++) {
				if ((*it).node == node) {
					connectNode.erase(it);
					break;
				}
			}
		}
	};
	//���ɂ����G�b�W�H
	template<class T> struct DijkstraNodeConnect {
	public:
		//�ڑ���m�[�h
		DijkstraNode<T>* node;
		//�ړ��ɂ�����R�X�g
		float cost;
	public:
		DijkstraNodeConnect(DijkstraNode<T>* node, float cost) :node(node), cost(cost) {};
		~DijkstraNodeConnect() = default;
	};
	//�_�C�N�X�g���@�̃R�X�g�Z�o�p�֐�
	template<class T> using DijkstraCostCalcFunction = std::function<float(const T&, const T&)>;
	//�_�C�N�X�g���@�̋ߎ��m�[�h�Z�o�p�֐�
	template<class T> using DijkstraQueryFunction = std::function<float(const T&, const T&)>;
	//�_�C�N�X�g���@
	//Math::Vector3��Math::Vector2�̓N�G���[�����łɃf�t�H���g�ŃZ�b�g���Ă��܂�
	template<class T> class DijkstraEngine {
	public:
		DijkstraEngine() :begin(nullptr), end(nullptr) {}
		virtual ~DijkstraEngine() {
			for each(auto& it in node) { delete it; }
		}
		void SetBegin(DijkstraNode<T>* node) { begin = node; }
		void SetBegin(const T& value) { SetBegin(QueryFromValue(value)); }
		void SetEnd(DijkstraNode<T>* node) { end = node; }
		void SetEnd(const T& value) { SetEnd(QueryFromValue(value)); }
		const std::list<DijkstraNode<T>*>& GetNodes() { return node; }
		//�J�n�n�_
		const DijkstraNode<T>* GetBeginNode() { return begin; }
		//�����g��Ȃ�
		const DijkstraNode<T>* GetEndNode() { return end; }
		//�����ɋ߂��m�[�h�̌���
		//TODO : �p�t�H�[�}���X���P ��̓I�ɂ͋�ԕ����Hkd-tree(kd��)
		DijkstraNode<T>* Query(const T& data) {
			struct Min {
				int index = -1;
				float value = 999999.0f;
			}min;
			int index = 0;
			for each(auto&& it in node) {
				float value = QueryNode(it->data, data);
				if (min.value > value) {
					min.index = index;
					min.value = value;
				}
				index++;
			}
			if (min.index < 0)return nullptr;
			return QueryFromIndex(min.index);
		}
		//�S�m�[�h���폜
		void Clear() {
			begin = nullptr;
			end = nullptr;
			node.clear();
		}
		//�m�[�h�̒ǉ�
		void AddNode(const T& data) {
			node.push_back(new DijkstraNode<T>(data));
		}
		//�w��m�[�h���폜
		void Remove(DijkstraNode<T>* node) {
			for (auto&& it = this->node.begin(); it != this->node.end(); it++) {
				if (it == node) {
					delete it;
					this->node.erase(it);
					break;
				}
			}
		}
		void Remove(int index) { Remove(QueryFromIndex(index)); }
		//�l����m�[�h���Ȃ���
		void ConnectNode(const T& d0, const T& d1) {
			ConnectNode(QueryFromValue(d0), QueryFromValue(d1));
		}
		//�m�[�h���Ȃ���
		void ConnectNode(DijkstraNode<T>* n0, DijkstraNode<T>* n1) {
			//�o�^�ς݂łȂ����ǂ�����T��
			for each(auto&& it in n0->connectNode) {
				if (it.node == n1)return;
			}
			//�R�X�g�Z�o
			float cost = CalcCost(n0->data, n1->data);
			//�ڑ�
			n0->connectNode.push_back(DijkstraNodeConnect<T>(n1, cost));
			n1->connectNode.push_back(DijkstraNodeConnect<T>(n0, cost));
		}
		//�S�m�[�h���Z�b�g
		void ResetNode() {
			for each(auto&& it in node) {
				it->Reset();
			}
		}
		void SearchExecute() {
			ResetNode();
			//�X�^�[�g�n�_�ƃS�[���n�_���ݒ肳��Ă��Ȃ����
			if (!begin || !end)return;
			std::list<DijkstraNode<T>*> work1;
			std::list<DijkstraNode<T>*> work2;
			std::list<DijkstraNode<T>*>* currentLevel = &work1;
			std::list<DijkstraNode<T>*>* nextLevel = &work2;
			//�S�[������v�Z ���̃m�[�h���v�Z�ς݂Ƃ���
			end->cost = 0;
			//�������K�w�ݒ�
			currentLevel->push_back(end);
			while (currentLevel->size()) {
				for each(auto&& it in *currentLevel) {
					for each(auto&& connect in it->connectNode) {
						float nodeCost = it->cost + connect.cost;
						//���T���܂��́A�R�X�g�I�ɍŒZ�̏ꍇ�X�V����
						if (connect.node->cost == -1 || nodeCost < connect.node->cost) {
							connect.node->cost = nodeCost;
							connect.node->toGoal = it;
						}
						else continue;
						//���̊K�w���X�g�ɓo�^
						nextLevel->push_back(connect.node);
					}
				}
				//���X�g�����ւ��Ď��̊K�w����������
				currentLevel->swap(*nextLevel);
				//�N���A����
				nextLevel->clear();
			}
		}
	private:
		//�m�[�h����C���f�b�N�X���Z�o
		int QueryIndex(DijkstraNode<T>* node) {
			return static_cast<int>(std::distance(this->node.begin(), std::find(this->node.begin(), this->node.end(), node)));
		}
		//�C���f�b�N�X����m�[�h���Z�o
		DijkstraNode<T>* QueryFromIndex(int index) { return *std::next(node.begin(), index); }
		DijkstraNode<T>* QueryFromValue(const T& data) {
			for each(auto it in node) {
				if (it->data == data)return it;
			}
			return nullptr;
		}
	private:
		//�m�[�h�̃X�^�[�g�n�_
		DijkstraNode<T>* begin;
		//�m�[�h�̏I���n�_
		DijkstraNode<T>* end;
		//�m�[�h�Q
		std::list<DijkstraNode<T>*> node;
	public:
		//�R�X�g�Z�o�p�֐��ݒ�(�m���Ɉ�x�Ă΂Ȃ��Ƃ����Ȃ�)
		static void SetQueryCostFunction(DijkstraCostCalcFunction<T> func) { CalcCost = std::move(func); }
		//�ߎ��m�[�h����Query�Z�o�֐��ݒ�(�m���Ɉ�x�Ă΂Ȃ��Ƃ����Ȃ�)
		static void SetQueryFunction(DijkstraCostCalcFunction<T> func) { QueryNode = std::move(func); }
	private:
		static DijkstraCostCalcFunction<T> CalcCost;
		static DijkstraQueryFunction<T> QueryNode;
	};
	template<class T> DijkstraCostCalcFunction<T> DijkstraEngine<T>::CalcCost;
	template<class T> DijkstraQueryFunction<T> DijkstraEngine<T>::QueryNode;
	using DijkstraEngineVector3 = DijkstraEngine<Math::Vector3>;
	using DijkstraEngineVector2 = DijkstraEngine<Math::Vector2>;
}