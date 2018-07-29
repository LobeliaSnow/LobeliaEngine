#pragma once
namespace Lobelia::Game {
	template<class T> struct DijkstraNodeConnect;
	//ノード
	template<class T> struct DijkstraNode {
	public:
		//情報
		T data;
		//接続しているノード
		std::list<DijkstraNodeConnect<T>> connectNode;
		//計算する値
		float cost;
		//ゴールへつながるノード
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
	//俗にいうエッジ？
	template<class T> struct DijkstraNodeConnect {
	public:
		//接続先ノード
		DijkstraNode<T>* node;
		//移動にかかるコスト
		float cost;
	public:
		DijkstraNodeConnect(DijkstraNode<T>* node, float cost) :node(node), cost(cost) {};
		~DijkstraNodeConnect() = default;
	};
	//ダイクストラ法のコスト算出用関数
	template<class T> using DijkstraCostCalcFunction = std::function<float(const T&, const T&)>;
	//ダイクストラ法の近似ノード算出用関数
	template<class T> using DijkstraQueryFunction = std::function<float(const T&, const T&)>;
	//ダイクストラ法
	//Math::Vector3とMath::Vector2はクエリーをすでにデフォルトでセットしています
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
		//開始地点
		const DijkstraNode<T>* GetBeginNode() { return begin; }
		//多分使わない
		const DijkstraNode<T>* GetEndNode() { return end; }
		//条件に近いノードの検索
		//TODO : パフォーマンス改善 具体的には空間分割？kd-tree(kd木)
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
		//全ノードを削除
		void Clear() {
			begin = nullptr;
			end = nullptr;
			node.clear();
		}
		//ノードの追加
		void AddNode(const T& data) {
			node.push_back(new DijkstraNode<T>(data));
		}
		//指定ノードを削除
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
		//値からノードをつなげる
		void ConnectNode(const T& d0, const T& d1) {
			ConnectNode(QueryFromValue(d0), QueryFromValue(d1));
		}
		//ノードをつなげる
		void ConnectNode(DijkstraNode<T>* n0, DijkstraNode<T>* n1) {
			//登録済みでないかどうかを探す
			for each(auto&& it in n0->connectNode) {
				if (it.node == n1)return;
			}
			//コスト算出
			float cost = CalcCost(n0->data, n1->data);
			//接続
			n0->connectNode.push_back(DijkstraNodeConnect<T>(n1, cost));
			n1->connectNode.push_back(DijkstraNodeConnect<T>(n0, cost));
		}
		//全ノードリセット
		void ResetNode() {
			for each(auto&& it in node) {
				it->Reset();
			}
		}
		void SearchExecute() {
			ResetNode();
			//スタート地点とゴール地点が設定されていなければ
			if (!begin || !end)return;
			std::list<DijkstraNode<T>*> work1;
			std::list<DijkstraNode<T>*> work2;
			std::list<DijkstraNode<T>*>* currentLevel = &work1;
			std::list<DijkstraNode<T>*>* nextLevel = &work2;
			//ゴールから計算 このノードを計算済みとする
			end->cost = 0;
			//検索第一階層設定
			currentLevel->push_back(end);
			while (currentLevel->size()) {
				for each(auto&& it in *currentLevel) {
					for each(auto&& connect in it->connectNode) {
						float nodeCost = it->cost + connect.cost;
						//未探索または、コスト的に最短の場合更新する
						if (connect.node->cost == -1 || nodeCost < connect.node->cost) {
							connect.node->cost = nodeCost;
							connect.node->toGoal = it;
						}
						else continue;
						//次の階層リストに登録
						nextLevel->push_back(connect.node);
					}
				}
				//リストを入れ替えて次の階層を検索する
				currentLevel->swap(*nextLevel);
				//クリアする
				nextLevel->clear();
			}
		}
	private:
		//ノードからインデックスを算出
		int QueryIndex(DijkstraNode<T>* node) {
			return static_cast<int>(std::distance(this->node.begin(), std::find(this->node.begin(), this->node.end(), node)));
		}
		//インデックスからノードを算出
		DijkstraNode<T>* QueryFromIndex(int index) { return *std::next(node.begin(), index); }
		DijkstraNode<T>* QueryFromValue(const T& data) {
			for each(auto it in node) {
				if (it->data == data)return it;
			}
			return nullptr;
		}
	private:
		//ノードのスタート地点
		DijkstraNode<T>* begin;
		//ノードの終了地点
		DijkstraNode<T>* end;
		//ノード群
		std::list<DijkstraNode<T>*> node;
	public:
		//コスト算出用関数設定(確実に一度呼ばないといけない)
		static void SetQueryCostFunction(DijkstraCostCalcFunction<T> func) { CalcCost = std::move(func); }
		//近似ノード検索Query算出関数設定(確実に一度呼ばないといけない)
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