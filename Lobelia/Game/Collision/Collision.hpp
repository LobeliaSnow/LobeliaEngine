#pragma once
#include "Game/Collision/SpatialPartitioning.hpp"
//#define OCTREE

namespace Lobelia::Game {
	//TODO : �S�Ă̑g�ݍ��킹��o�^���Ȃ��Ƃ����Ȃ�
	enum class ShapeType3D :int { SPHERE, AABB , CAPSULE };
	class Actor;
	class Shape abstract {
	public:
		Shape() :enable(true), hitEvent(true), isForce(true), staticPos(false) {}
		~Shape() = default;
		//�����蔻��̗L�������֘A
		void Enable() { enable = true; }
		void Disable() { enable = false; }
		bool IsEnable() { return enable; }
		bool IsDisable() { return !enable; }
		void EnableHitEvent() { hitEvent = true; }
		void DisableHitEvent() { hitEvent = false; }
		bool IsEnableHitEvent() { return hitEvent; }
		bool IsDisableHitEvent() { return !hitEvent; }
		//�����߂����ۂ�
		void SetForce(bool force) { isForce = force; }
		bool IsForce() { return isForce; }
		//�����߂��̍ۂɓ������ۂ�
		void SetStatic(bool static_pos) { staticPos = static_pos; }
		bool IsStatic() { return staticPos; }
	private:
		bool enable;
		bool hitEvent;
		bool isForce;
		//�����߂��̍ۂɓ������ۂ�
		bool staticPos;
	};
	//-------------------------------------------------------------------------------------------------------------------------
	//���̐�3D
	//-------------------------------------------------------------------------------------------------------------------------
	//��Ƀu���[�h�t�F�[�Y�p�̃T�C�Y��`
	class Shape3D abstract :public Shape {
	public:
		Shape3D(ShapeType3D type, const Math::Vector3& pos) :actor(nullptr), type(type), pos(pos) {};
		virtual ~Shape3D() = default;
		void SetParentActor(Actor* actor) { this->actor = actor; }
		Actor* GetActor() { return actor; }
		ShapeType3D GetType() { return type; }
		//���������Ώۂ������Ŕ��ł��܂�
		void IssueHitEvent(Actor * other_actor);
		const Math::Vector3& GetPos() { return pos; }
		const Math::Vector3& GetAABBRadius() { return radius; }
		void SetPos(const Math::Vector3& pos) { this->pos = pos; }
		void SetAABBRadius(const Math::Vector3& radius) { this->radius = radius; }
	protected:
		//���S���W
		Math::Vector3 pos;
		//AABB�̔��a
		Math::Vector3 radius;
	private:
		Actor * actor;
		ShapeType3D type;
	};
	class Sphere :public Shape3D {
	public:
		Sphere(const Math::Vector3& pos, float radius);
		~Sphere() = default;
		void SetRadius(float radius) { this->radius = radius; }
		float GetRadius() { return radius; }
	private:
		float radius;
	};
	class AABB3D :public Shape3D {
	public:
		AABB3D(const Math::Vector3& pos, const Math::Vector3& radius);
		~AABB3D() = default;
		Math::Vector3 GetMin() { return pos - radius; }
		Math::Vector3 GetMax() { return pos + radius; }
	private:
	};
	class Capsule3D : public Shape3D {
	public:
		Capsule3D(const Math::Vector3& start_pos, const Math::Vector3& end_pos, float radius);
		~Capsule3D() = default;
		void SetPos(Math::Vector3 start_pos, Math::Vector3 end_pos) {
			startPosition = start_pos; endPosition = end_pos;
			pos = (start_pos + end_pos) / 2.0f;
		}
		Math::Vector3 GetStartPos() { return startPosition; }
		Math::Vector3 GetEndPos()   { return endPosition;}
		float         GetRadius()   { return radius;}
		void          SetRadius(float radius) { this->radius = radius; }
	private:
		Math::Vector3 startPosition;
		Math::Vector3 endPosition;
		float radius;
	};
	class Collision3D abstract {
	public:
		Collision3D() = default;
		//�p������O�񂾂��A�������̊m�ۂ�����\�肪�Ȃ��̂�non virtual
		~Collision3D() = default;
		virtual bool Execute(Shape3D* s0, Shape3D*s1) = 0;
	protected:
		bool IsForce(Shape3D* s0, Shape3D*s1);
		Actor* ForceActor(Shape3D* s0, Shape3D*s1);
	};
	class SphereToShpere :public Collision3D {
	public:
		bool Execute(Shape3D* s0, Shape3D*s1)override;
	};
	class AABB3DToAABB3D :public Collision3D {
	public:
		bool Execute(Shape3D* s0, Shape3D*s1)override;
	};
	class SphereToAABB3D :public Collision3D {
	public:
		bool Execute(Shape3D* s0, Shape3D*s1)override;
	private:
		float Check(float pn, float min, float max);
	};
	class Capsule3DToCapsule3D : public Collision3D {
	public:
		bool Execute(Shape3D* s0, Shape3D* s1) override;
	private:

	};
	class Capsule3DToAABB3D : public Collision3D {
	public:
		bool Execute(Shape3D* s0, Shape3D* s1) override;
	private:
		float Check(float pn, float min, float max);

	};
	class SphereToCapsule3D : public Collision3D {
	public:
		bool Execute(Shape3D* s0, Shape3D* s1) override;
	private:

	};
	class CollisionManager3DBase {
	public:
		CollisionManager3DBase();
		virtual ~CollisionManager3DBase() = default;
	protected:
		//�����蔻����s�֐�
		bool Execute(Shape3D* s0, Shape3D* s1);
	private:
		std::unique_ptr<Collision3D> collisionTable3D[3][3];

	};
	class CollisionManager3D :public CollisionManager3DBase, public Utility::Singleton<CollisionManager3D> {
	public:
		CollisionManager3D();
		void SetShape(std::shared_ptr<Shape3D> shape) {
			shape3DList.push_back(shape);
			ortList.push_back(std::make_shared<ObjectRegisterTree<Shape3D>>(shape));
		}
		void Update();
	private:
		//�����蔻��p�V�F�C�v�̐�������
		void JudgeAliveShape();
	private:
		//�����̎d�g�݂��ς����ق�������������
		//�ǉ����l�����Ȃ��Ȃ�񎟌��z��ł���O(1)�ŃA�N�Z�X�\�ɂȂ�(�H)
		//std::map<ShapeType3D, std::map<ShapeType3D, std::unique_ptr<Collision>>> collisionTable3D;
		std::list<std::weak_ptr<Shape3D>> shape3DList;
		std::unique_ptr<OctreeManager<Shape3D>> octree;
		std::list<std::shared_ptr<ObjectRegisterTree<Shape3D>>>  ortList;
	};
	//TODO : �w�b�_��������cpp�ֈڂ�
	//TODO : ���Ԃł����烊�t�@�N�^�����O
	class CollisionSlot3D {
		friend class CollisionSlotManager3D;
	public:
		void SetShape(std::shared_ptr<Shape3D> shape) { shapeList.push_back(shape); }
	private:
		std::list<std::weak_ptr<Shape3D>> shapeList;
	};
	//�K��
	class CollisionSlotList3D {
		friend class CollisionSlotManager3D;
	public:
		CollisionSlotList3D() {
			playerSlot = std::make_unique<CollisionSlot3D>();
			enemySlot = std::make_unique<CollisionSlot3D>();
		}
		void SetShapePlayerSlot(std::shared_ptr<Shape3D>& shape) { playerSlot->SetShape(shape); }
		void SetShapeEnemySlot(std::shared_ptr<Shape3D>& shape) { enemySlot->SetShape(shape); }
	private:
		std::unique_ptr<CollisionSlot3D> playerSlot;
		std::unique_ptr<CollisionSlot3D> enemySlot;
	};
	class ObjectRegisterTree3DSlotVersion :public ObjectRegisterTree<Shape3D> {
	public:
		ObjectRegisterTree3DSlotVersion(std::weak_ptr<Shape3D> object, int slot_index) :ObjectRegisterTree<Shape3D>(object), slotIndex(slot_index) {}
		int GetIndex() { return slotIndex; }
	private:
		int slotIndex;
	};

	class OctreeManager3DSlotVersion :public OctreeManager<Shape3D> {
	public:
		bool Register(const Math::Vector3& min, const Math::Vector3& max, std::shared_ptr<ObjectRegisterTree3DSlotVersion> ort) {
			DWORD element = Get3DMortonOrder(min, max);
			if (element < cellCount) {
				if (!cells[element])CreateCell(element);
				return cells[element]->Push(ort);
			}
			//�o�^���s
			return false;
		}
		DWORD CreateAllCollisionList(std::vector<Shape3D*>& col_vector) {
			col_vector.clear();
			//��Ԃ����݂��Ȃ�
			if (!cells[0])return 0;
			std::list<std::pair<std::weak_ptr<Shape3D>, int>> collisionStack;
			CreateCollisionList(0, col_vector, collisionStack);
			return s_cast<DWORD>(col_vector.size());
		}
		bool CreateCollisionList(DWORD element, std::vector<Shape3D*>& col_vector, std::list<std::pair<std::weak_ptr<Shape3D>, int>>& col_stack) {
			std::shared_ptr<ObjectRegisterTree<Shape3D>> ort0 = cells[element]->GetFirstObject();
			//��ԓ��̃I�u�W�F�N�g���m�̏Փ˃��X�g�쐬
			while (ort0) {
				std::shared_ptr<ObjectRegisterTree<Shape3D>> ort1 = ort0->GetNext();
				auto object0 = ort0->GetObjectPointer().lock().get();
				auto tester0 = d_cast<ObjectRegisterTree3DSlotVersion*>(ort0.get());
				while (ort1) {
					auto tester1 = d_cast<ObjectRegisterTree3DSlotVersion*>(ort1.get());
					if (tester0->GetIndex() != tester1->GetIndex()) {
						//����Z�����ł̏Փ˃��X�g�쐬
						col_vector.emplace_back(object0);
						col_vector.emplace_back(ort1->GetObjectPointer().lock().get());
					}
					ort1 = ort1->GetNext();
				}
				//�e��ԂƂ̏Փ˃��X�g�쐬
				for each(auto&& it in col_stack) {
					if (tester0->GetIndex() != it.second) {
						col_vector.emplace_back(object0);
						col_vector.emplace_back(it.first.lock().get());
					}
				}
				ort0 = ort0->GetNext();
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
							auto tester0 = d_cast<ObjectRegisterTree3DSlotVersion*>(ort0.get());
							col_stack.emplace_back(ort0->GetObjectPointer(), tester0->GetIndex());
							objectNum++;
							ort0 = ort0->GetNext();
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

	private:

	};
	class CollisionSlotManager3D :public CollisionManager3DBase, public Utility::Singleton<CollisionSlotManager3D> {
	public:
		CollisionSlotManager3D() {
			list = std::make_unique<CollisionSlotList3D>();
			octree = std::make_unique<OctreeManager3DSlotVersion>();
			octree->Initialize(6, Math::Vector3(-40.0f, -10.0f, -40.0f), Math::Vector3(40.0f, 10.0f, 40.0f));
		}
		void SetShapePlayerSlot(std::shared_ptr<Shape3D> shape) {
			list->SetShapePlayerSlot(shape);
			ortList.push_back(std::make_shared<ObjectRegisterTree3DSlotVersion>(shape, 0));
		}
		void SetShapeEnemySlot(std::shared_ptr<Shape3D> shape) { 
			list->SetShapeEnemySlot(shape); 
			ortList.push_back(std::make_shared<ObjectRegisterTree3DSlotVersion>(shape, 1));
		}
		void Update();
	private:
		std::unique_ptr<CollisionSlotList3D> list;
		std::unique_ptr<OctreeManager3DSlotVersion> octree;
		std::list<std::shared_ptr<ObjectRegisterTree3DSlotVersion>>  ortList;
	};
	//-------------------------------------------------------------------------------------------------------------------------
	//���̐�2D
	//-------------------------------------------------------------------------------------------------------------------------
	
}