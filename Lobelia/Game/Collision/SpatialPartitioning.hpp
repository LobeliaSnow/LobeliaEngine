#pragma once
//参考記事
//http://edom18.hateblo.jp/entry/2017/07/28/083153
//http://marupeke296.com/COL_3D_No15_Octree.html
//http://marupeke296.com/COL_2D_No8_QuadTree.html

namespace Lobelia::Game {
	template<class> class Cell;
	template<class> class OctreeManager;
	template<class T> class ObjectRegisterTree {
		friend class Cell<T>;
		friend class OctreeManager<T>;
	public:
		ObjectRegisterTree(std::weak_ptr<T> object) :cell(nullptr), object(object) {}
		virtual ~ObjectRegisterTree() { Remove(); }
		bool Remove() {
			if (!cell)return false;
			cell->OnRemove(this);
			if (previous) {
				previous->next = next;
				previous.reset();
			}
			if (next) {
				next->previous = previous;
				next.reset();
			}
			cell = nullptr;
			return true;
		}
		void RegisterCell(Cell<T>* cell) { this->cell = cell; }
		std::weak_ptr<T> GetObjectPointer() { return object; }
		std::shared_ptr<ObjectRegisterTree<T>>& GetNext() { return next; }

	protected:
		//登録空間
		Cell<T>* cell;
		//判定対象オブジェクト
		std::weak_ptr<T> object;
		//前のオブジェクトへ
		std::shared_ptr<ObjectRegisterTree<T>> previous;
		//次のオブジェクトへ
		std::shared_ptr<ObjectRegisterTree<T>> next;
	};
	template<class T> class OctreeManager {
	private:
		static constexpr const DWORD MAX_LEVEL = 7;
	public:
		OctreeManager() = default;
		virtual ~OctreeManager() = default;
		void Initialize(DWORD level, const Math::Vector3& region_min, const Math::Vector3& region_max) {
			if (level >= MAX_LEVEL)STRICT_THROW("この分割数には対応できません");
			//空間数テーブル作成
			cellCountTable[0] = 1;
			for (int i = 1; i < MAX_LEVEL + 1; i++) {
				cellCountTable[i] = cellCountTable[i - 1] * 8;
			}
			//セルの総数算出(四分木の場合は7->3になる)
			cellCount = (cellCountTable[level + 1] - 1) / 7;
			//レベル(0基点)の配列作成
			cells.resize(cellCount);
			//領域情報の登録
			regionMin = region_min;
			regionMax = region_max;
			cellWidth = regionMax - regionMin;
			unit = cellWidth / f_cast(1 << level);
			this->level = level;
		}
		bool Register(const Math::Vector3& min, const Math::Vector3& max, std::shared_ptr<ObjectRegisterTree<T>> ort) {
			DWORD element = Get3DMortonOrder(min, max);
			if (element < cellCount) {
				if (!cells[element])CreateCell(element);
				return cells[element]->Push(ort);
			}
			//登録失敗
			return false;
		}
		DWORD CreateAllCollisionList(std::vector<T*>& col_vector) {
			col_vector.clear();
			//空間が存在しない
			if (!cells[0])return 0;
			std::list<std::weak_ptr<T>> collisionStack;
			CreateCollisionList(0, col_vector, collisionStack);
			return s_cast<DWORD>(col_vector.size());
		}
	public:
		//後で見る
		//空間内で衝突リストを作成する
		bool CreateCollisionList(DWORD element, std::vector<T*>& col_vector, std::list<std::weak_ptr<T>>& col_stack) {
			std::shared_ptr<ObjectRegisterTree<T>> ort0 = cells[element]->GetFirstObject();
			//空間内のオブジェクト同士の衝突リスト作成
			while (ort0) {
				std::shared_ptr<ObjectRegisterTree<T>> ort1 = ort0->next;
				auto object0 = ort0->object.lock().get();
				while (ort1) {
					//同一セル内での衝突リスト作成
					col_vector.emplace_back(object0);
					col_vector.emplace_back(ort1->object.lock().get());
					ort1 = ort1->next;
				}
				//親空間との衝突リスト作成
				for each(auto&& it in col_stack) {
					col_vector.emplace_back(object0);
					col_vector.emplace_back(it.lock().get());
				}
				ort0 = ort0->next;
			}
			bool child = false;
			//子空間に移動
			DWORD objectNum = 0;
			for (DWORD i = 0; i < 8; i++) {
				DWORD nextElement = element * 8 + 1 + i;
				if (nextElement < cellCount&&cells[nextElement]) {
					if (!child) {
						//登録オブジェクトをスタックに追加
						ort0 = cells[element]->GetFirstObject();
						while (ort0) {
							col_stack.push_back(ort0->object);
							objectNum++;
							ort0 = ort0->next;
						}
					}
					child = true;
					//子空間へ
					CreateCollisionList(nextElement, col_vector, col_stack);	// 子空間へ
				}
			}
			//スタックからオブジェクトを外す
			if (child) {
				for (int i = 0; i < objectNum; i++)col_stack.pop_back();
			}
			return true;
		}
		//Bitを3BitごとにZYX2ZYX1ZYX0にしたい X = Bit(X2X1X0), Y = Bit(Y2Y1Y0), Z = Bit(Z2Z1Z0)
		//そのための00n00n00nという風に情報を分割する
		DWORD BitSeparate(BYTE n) {
			DWORD s = n;
			//0b0000001111000000001111
			s = (s | s << 8) & 0x0000F00F;
			//0b0011000011000011000011
			s = (s | s << 4) & 0x000C30C3;
			//0b1001001001001001001001
			s = (s | s << 2) & 0x00249249;
			return s;
		}
		//その点の空間番号算出
		DWORD Get3DMortonOrder(const Math::Vector3& pos) {
			Math::Vector3 temp = pos - regionMin;
			return Get3DMortonOrder(s_cast<BYTE>(temp.x / unit.x), s_cast<BYTE>(temp.y / unit.y), s_cast<BYTE>(temp.z / unit.z));
		}
		DWORD Get3DMortonOrder(BYTE x, BYTE y, BYTE z) { return (BitSeparate(x) | BitSeparate(y) << 1 | BitSeparate(z) << 2); }
		//モートン番号算出(Bitを3BitごとにZYX2ZYX1ZYX0にする)
		DWORD Get3DMortonOrder(const Math::Vector3& min, const Math::Vector3& max) {
			//左上
			DWORD lt = Get3DMortonOrder(min);
			//右下
			DWORD rb = Get3DMortonOrder(max);
			//XOR
			DWORD morton = rb ^ lt;
			//最上位ビットを探す
			DWORD highLevel = 1;
			for (DWORD i = 0; i < level; i++) {
				//下位3ビットから検索
				DWORD check = (morton >> (i * 3)) & 0b111;
				//ビットが立っていれば最上位レベルの更新
				if (check)highLevel = i + 1;
			}
			//所属している空間番号算出(レベルからシフト数決定して、その空間でのモートン番号算出)
			DWORD spaceNum = rb >> (highLevel * 3);
			//その空間までのセル総数取得
			DWORD addNum = (cellCountTable[level - highLevel] - 1) / 7;
			//目的のモートン番号=その空間が所属している番号+その空間になるまでに出てきたセルの数
			spaceNum += addNum;
			//セルの総数を超えている場合
			if (spaceNum > cellCount)return 0xFFFFFFFF;
			return spaceNum;
		}
		void CreateCell(DWORD element) {
			while (!cells[element]) {
				cells[element] = std::make_shared<Cell<T>>();
				//親空間へジャンプ
				element = (element - 1) >> 3;
				if (element >= cellCount)break;
			}
		}
	protected:
		//最下位レベル
		DWORD level;
		Math::Vector3 regionMin;
		Math::Vector3 regionMax;
		//領域の幅
		Math::Vector3 cellWidth;
		//最小領域の辺の長さ
		Math::Vector3 unit;
		DWORD cellCount;
		//空間
		std::vector<std::shared_ptr<Cell<T>>> cells;
		//各レベルでの空間数配列(8)
		std::array<DWORD, MAX_LEVEL + 1> cellCountTable;
	};
	template<class T> class Cell {
	public:
		Cell() = default;
		virtual ~Cell() {
			if (latest)ResetLink(latest);
		}
		//リンクをすべて解除
		void ResetLink(std::shared_ptr<ObjectRegisterTree<T>>& ort) {
			if (ort->next)ResetLink(ort->next);
			ort.reset();
		}
		bool Push(std::shared_ptr<ObjectRegisterTree<T>> ort) {
			if (!ort || ort->cell == this)return false;
			if (!latest)latest = ort;
			else {
				ort->next = latest;
				latest->previous = ort;
				latest = ort;
			}
			ort->RegisterCell(this);
			return true;
		}
		std::shared_ptr<ObjectRegisterTree<T>>& GetFirstObject() { return latest; }
		void OnRemove(ObjectRegisterTree<T>* remove_obj) {
			if (latest.get() == remove_obj) {
				//次のオブジェクトにつなげ変え
				if (latest) latest = latest->next;
			}
		}
	private:
		//最新のort
		std::shared_ptr<ObjectRegisterTree<T>> latest;

	};
}