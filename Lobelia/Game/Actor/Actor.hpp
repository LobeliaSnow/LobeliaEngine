#pragma once
#include "Game/Collision/Collision.hpp"
namespace Lobelia::Game {
	//�L�����N�^�[�����̊��N���X
	//��{������������邽�߂̊֐���ϐ�������
	//�����`�������C�u�����s��
	class Actor abstract {
	public:
		Actor(const char* tag, int hp) :jumpPower(0.0f), hp(hp), gravity(0.0f), tag(tag) { Acceleration(1.0f); }
		virtual ~Actor() = default;
		void Jump(float jump_power) { if (jumpPower == 0.0f)jumpPower += jump_power; }
		int GetHP() { return hp; }
		void Damage(int damage) { hp -= damage; }
		void Heal(int heal) { hp += heal; }
		void SetHP(int hp) { this->hp = hp; }
		virtual bool IsAlive() { return (hp > 0); }
		void SetPos(const Math::Vector3& pos) { transform.position = pos; }
		//�ړ������ƈړ�����͂�ݒ�
		void SetMove(const Math::Vector3& move, float power) {
			moveDirection = move;
			moveDirection.Normalize();
			this->power = power;
		}
		float GetRad() { return transform.rotation.y; }
		Math::Vector3 GetRadVec3() { return Math::Vector3(sinf(GetRad()), 0.0f, cosf(GetRad())); }
		const Math::Vector3& GetPos() { return transform.position; }
		const Math::Vector3& GetAxisDirection() { return moveDirection; }
		const Math::Vector3& GetMove() { return move; }
		float GetPower() { return power; }
		float GetJumpPower() { return jumpPower; }
		void SetTag(const char* tag) { this->tag = tag; }
		std::string GetTag() { return tag; }
		//����
		void Acceleration(float acceleration) { this->acceleration = acceleration; }
		float GetAcceleration() { return acceleration; }
		//������o�^���Ă���V�F�C�v���ق��̃V�F�C�v�ƂԂ������Ƃ�
		virtual void HitEvent(Actor* other) {};
		//�ړ��ʎZ�o
		void CalcMove();
		//�ǂƂ̋����𑪂�
		float CheckWall(Graphics::Model* terrain_data, const Math::Vector3& direction, float dist, Math::Vector3* face_normal, std::string* material_name);
		//�n�ʂƂ̋����𑪂�
		float CheckFloor(Graphics::Model* terrain_data, const Math::Vector3& bias, Math::Vector3* floor_pos, Math::Vector3* face_normal, std::string* material_name);
		//�Ȃ߂炩�ɉ�]
		void SmoothRotateY(float coefficient = 18.0f);
		//�d�͂�������(�㉺�ɂ����Ή����Ă��Ȃ� ���̓}�C�i�X)
		void Gravity(float gravity = -300.0f);
		//�n�`�f�[�^�Ƃ̓����蔻��(�� xz) *CalcMove������ɌĂԂ���
		void CollisionTerrainWall(Graphics::Model* terrain, std::string* mat_name);
		//�n�`�f�[�^�Ƃ̓����蔻��(�� y) *CalcMove������ɌĂԂ���
		void CollisionTerrainFloor(Graphics::Model* terrain, std::string* mat_name);
		//�ړ� �����蔻��֐��Ăяo���ꍇ�́A�Ăяo������ɂ�����ĂԂ���
		void Move() { transform.position += move; }
	protected:
		//�V�F�C�v���쐬���āA�e�������Ƃ��ēo�^���A�}�l�[�W���[�ɓo�^����
		template<class T, class... Args> std::shared_ptr<T> CreateShape(Args&&... args) {
			std::shared_ptr<T> shape = std::make_shared<T>(std::forward<Args>(args)...);
			shape->SetParentActor(this);
			CollisionManager3D::GetInstance()->SetShape(shape);
			return shape;
		}
		//�f�t�H���g�̋���(���������b�v��������)
		void DefaultBehavior(Graphics::Model* terrain, std::string* wall_name, std::string* floor_name);
		bool CheckActorVision(Actor* actor, float vision_rad, float vision_length);
	protected:
		Graphics::Transform3D transform;
		//�ړ�����(XZ)
		Math::Vector3 moveDirection;
		//�ړ���
		float power;
		//�t���[�����Ƃ̐i�ވړ���(CalcMove�ɂ��Z�o)
		Math::Vector3 move;
		//����Y�̈ړ���
		float jumpPower;
		int hp;
		float acceleration;
	private:
		//�d�͗p�ϐ�
		float gravity;
		std::string tag;
	};
}