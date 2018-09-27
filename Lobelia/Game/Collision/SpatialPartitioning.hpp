#pragma once
//�Q�l�L��
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
		//�o�^���
		Cell<T>* cell;
		//����ΏۃI�u�W�F�N�g
		std::weak_ptr<T> object;
		//�O�̃I�u�W�F�N�g��
		std::shared_ptr<ObjectRegisterTree<T>> previous;
		//���̃I�u�W�F�N�g��
		std::shared_ptr<ObjectRegisterTree<T>> next;
	};
	template<class T> class OctreeManager {
	private:
		static constexpr const DWORD MAX_LEVEL = 7;
	public:
		OctreeManager() = default;
		virtual ~OctreeManager() = default;
		void Initialize(DWORD level, const Math::Vector3& region_min, const Math::Vector3& region_max) {
			if (level >= MAX_LEVEL)STRICT_THROW("���̕������ɂ͑Ή��ł��܂���");
			//��Ԑ��e�[�u���쐬
			cellCountTable[0] = 1;
			for (int i = 1; i < MAX_LEVEL + 1; i++) {
				cellCountTable[i] = cellCountTable[i - 1] * 8;
			}
			//�Z���̑����Z�o(�l���؂̏ꍇ��7->3�ɂȂ�)
			cellCount = (cellCountTable[level + 1] - 1) / 7;
			//���x��(0��_)�̔z��쐬
			cells.resize(cellCount);
			//�̈���̓o�^
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
			//�o�^���s
			return false;
		}
		DWORD CreateAllCollisionList(std::vector<T*>& col_vector) {
			col_vector.clear();
			//��Ԃ����݂��Ȃ�
			if (!cells[0])return 0;
			std::list<std::weak_ptr<T>> collisionStack;
			CreateCollisionList(0, col_vector, collisionStack);
			return s_cast<DWORD>(col_vector.size());
		}
	public:
		//��Ō���
		//��ԓ��ŏՓ˃��X�g���쐬����
		bool CreateCollisionList(DWORD element, std::vector<T*>& col_vector, std::list<std::weak_ptr<T>>& col_stack) {
			std::shared_ptr<ObjectRegisterTree<T>> ort0 = cells[element]->GetFirstObject();
			//��ԓ��̃I�u�W�F�N�g���m�̏Փ˃��X�g�쐬
			while (ort0) {
				std::shared_ptr<ObjectRegisterTree<T>> ort1 = ort0->next;
				auto object0 = ort0->object.lock().get();
				while (ort1) {
					//����Z�����ł̏Փ˃��X�g�쐬
					col_vector.emplace_back(object0);
					col_vector.emplace_back(ort1->object.lock().get());
					ort1 = ort1->next;
				}
				//�e��ԂƂ̏Փ˃��X�g�쐬
				for each(auto&& it in col_stack) {
					col_vector.emplace_back(object0);
					col_vector.emplace_back(it.lock().get());
				}
				ort0 = ort0->next;
			}
			bool child = false;
			//�q��ԂɈړ�
			DWORD objectNum = 0;
			for (DWORD i = 0; i < 8; i++) {
				DWORD nextElement = element * 8 + 1 + i;
				if (nextElement < cellCount&&cells[nextElement]) {
					if (!child) {
						//�o�^�I�u�W�F�N�g���X�^�b�N�ɒǉ�
						ort0 = cells[element]->GetFirstObject();
						while (ort0) {
							col_stack.push_back(ort0->object);
							objectNum++;
							ort0 = ort0->next;
						}
					}
					child = true;
					//�q��Ԃ�
					CreateCollisionList(nextElement, col_vector, col_stack);	// �q��Ԃ�
				}
			}
			//�X�^�b�N����I�u�W�F�N�g���O��
			if (child) {
				for (int i = 0; i < objectNum; i++)col_stack.pop_back();
			}
			return true;
		}
		//Bit��3Bit���Ƃ�ZYX2ZYX1ZYX0�ɂ����� X = Bit(X2X1X0), Y = Bit(Y2Y1Y0), Z = Bit(Z2Z1Z0)
		//���̂��߂�00n00n00n�Ƃ������ɏ��𕪊�����
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
		//���̓_�̋�Ԕԍ��Z�o
		DWORD Get3DMortonOrder(const Math::Vector3& pos) {
			Math::Vector3 temp = pos - regionMin;
			return Get3DMortonOrder(s_cast<BYTE>(temp.x / unit.x), s_cast<BYTE>(temp.y / unit.y), s_cast<BYTE>(temp.z / unit.z));
		}
		DWORD Get3DMortonOrder(BYTE x, BYTE y, BYTE z) { return (BitSeparate(x) | BitSeparate(y) << 1 | BitSeparate(z) << 2); }
		//���[�g���ԍ��Z�o(Bit��3Bit���Ƃ�ZYX2ZYX1ZYX0�ɂ���)
		DWORD Get3DMortonOrder(const Math::Vector3& min, const Math::Vector3& max) {
			//����
			DWORD lt = Get3DMortonOrder(min);
			//�E��
			DWORD rb = Get3DMortonOrder(max);
			//XOR
			DWORD morton = rb ^ lt;
			//�ŏ�ʃr�b�g��T��
			DWORD highLevel = 1;
			for (DWORD i = 0; i < level; i++) {
				//����3�r�b�g���猟��
				DWORD check = (morton >> (i * 3)) & 0b111;
				//�r�b�g�������Ă���΍ŏ�ʃ��x���̍X�V
				if (check)highLevel = i + 1;
			}
			//�������Ă����Ԕԍ��Z�o(���x������V�t�g�����肵�āA���̋�Ԃł̃��[�g���ԍ��Z�o)
			DWORD spaceNum = rb >> (highLevel * 3);
			//���̋�Ԃ܂ł̃Z�������擾
			DWORD addNum = (cellCountTable[level - highLevel] - 1) / 7;
			//�ړI�̃��[�g���ԍ�=���̋�Ԃ��������Ă���ԍ�+���̋�ԂɂȂ�܂łɏo�Ă����Z���̐�
			spaceNum += addNum;
			//�Z���̑����𒴂��Ă���ꍇ
			if (spaceNum > cellCount)return 0xFFFFFFFF;
			return spaceNum;
		}
		void CreateCell(DWORD element) {
			while (!cells[element]) {
				cells[element] = std::make_shared<Cell<T>>();
				//�e��ԂփW�����v
				element = (element - 1) >> 3;
				if (element >= cellCount)break;
			}
		}
	protected:
		//�ŉ��ʃ��x��
		DWORD level;
		Math::Vector3 regionMin;
		Math::Vector3 regionMax;
		//�̈�̕�
		Math::Vector3 cellWidth;
		//�ŏ��̈�̕ӂ̒���
		Math::Vector3 unit;
		DWORD cellCount;
		//���
		std::vector<std::shared_ptr<Cell<T>>> cells;
		//�e���x���ł̋�Ԑ��z��(8)
		std::array<DWORD, MAX_LEVEL + 1> cellCountTable;
	};
	template<class T> class Cell {
	public:
		Cell() = default;
		virtual ~Cell() {
			if (latest)ResetLink(latest);
		}
		//�����N�����ׂĉ���
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
				//���̃I�u�W�F�N�g�ɂȂ��ς�
				if (latest) latest = latest->next;
			}
		}
	private:
		//�ŐV��ort
		std::shared_ptr<ObjectRegisterTree<T>> latest;

	};
}