#pragma once
#include "Game/Collision/Collision.hpp"
namespace Lobelia::Game {
	//キャラクターたちの基底クラス
	//基本動作を実現するための関数や変数が存在
	//ワンチャンライブラリ行き
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
		//移動方向と移動する力を設定
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
		//加速
		void Acceleration(float acceleration) { this->acceleration = acceleration; }
		float GetAcceleration() { return acceleration; }
		//自分を登録しているシェイプがほかのシェイプとぶつかったとき
		virtual void HitEvent(Actor* other) {};
		//移動量算出
		void CalcMove();
		//壁との距離を測る
		float CheckWall(Graphics::Model* terrain_data, const Math::Vector3& direction, float dist, Math::Vector3* face_normal, std::string* material_name);
		//地面との距離を測る
		float CheckFloor(Graphics::Model* terrain_data, const Math::Vector3& bias, Math::Vector3* floor_pos, Math::Vector3* face_normal, std::string* material_name);
		//なめらかに回転
		void SmoothRotateY(float coefficient = 18.0f);
		//重力をかける(上下にしか対応していない 下はマイナス)
		void Gravity(float gravity = -300.0f);
		//地形データとの当たり判定(壁 xz) *CalcMoveした後に呼ぶこと
		void CollisionTerrainWall(Graphics::Model* terrain, std::string* mat_name);
		//地形データとの当たり判定(床 y) *CalcMoveした後に呼ぶこと
		void CollisionTerrainFloor(Graphics::Model* terrain, std::string* mat_name);
		//移動 当たり判定関数呼び出す場合は、呼び出した後にこれを呼ぶこと
		void Move() { transform.position += move; }
	protected:
		//シェイプを作成して、親を自分として登録し、マネージャーに登録する
		template<class T, class... Args> std::shared_ptr<T> CreateShape(Args&&... args) {
			std::shared_ptr<T> shape = std::make_shared<T>(std::forward<Args>(args)...);
			shape->SetParentActor(this);
			CollisionManager3D::GetInstance()->SetShape(shape);
			return shape;
		}
		//デフォルトの挙動(動きをラップしたもの)
		void DefaultBehavior(Graphics::Model* terrain, std::string* wall_name, std::string* floor_name);
		bool CheckActorVision(Actor* actor, float vision_rad, float vision_length);
	protected:
		Graphics::Transform3D transform;
		//移動方向(XZ)
		Math::Vector3 moveDirection;
		//移動量
		float power;
		//フレームごとの進む移動量(CalcMoveにより算出)
		Math::Vector3 move;
		//実質Yの移動量
		float jumpPower;
		int hp;
		float acceleration;
	private:
		//重力用変数
		float gravity;
		std::string tag;
	};
}