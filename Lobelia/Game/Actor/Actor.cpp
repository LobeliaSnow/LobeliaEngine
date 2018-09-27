#include "Lobelia.hpp"
#include "Game/Actor/Actor.hpp"

namespace Lobelia::Game {
	void Actor::CalcMove() {
		move.x = moveDirection.x * power* Application::GetInstance()->GetProcessTimeSec()*acceleration;
		move.z = moveDirection.z * power* Application::GetInstance()->GetProcessTimeSec()*acceleration;
		move.y = (((Application::GetInstance()->GetProcessTimeSec()*acceleration)*gravity*0.5f) + jumpPower)*(Application::GetInstance()->GetProcessTimeSec()*acceleration);
		jumpPower = move.y / (Application::GetInstance()->GetProcessTimeSec()*acceleration);
		gravity = 0.0f;
	}
	float Actor::CheckWall(Graphics::Model* terrain_data, const Math::Vector3& direction, float dist, Math::Vector3* face_normal, std::string* material_name) {
		//飛ばすレイのベクトルが0ベクトルの時はレイピックを行わない
		if (!terrain_data || direction.Length() == 0.0f)return FLT_MAX;
		Math::Vector3 outPos, outNormal;
		Math::Vector3 rayPos = transform.position + Math::Vector3(0.0f, 0.5f, 0.0f);
		int ret = terrain_data->RayPickWorld(&outPos, &outNormal, rayPos, direction, dist);
		if (ret < 0)return FLT_MAX;
		if (face_normal)*face_normal = outNormal;
		if (material_name) *material_name = terrain_data->GetMaterialName(ret);
		//壁までの距離を返す
		return (outPos - rayPos).Length();
	}
	float Actor::CheckFloor(Graphics::Model* terrain_data, const Math::Vector3& bias, Math::Vector3* floor_pos, Math::Vector3* face_normal, std::string* material_name) {
		Math::Vector3 outPos, outNormal;
		Math::Vector3 rayPos = transform.position + bias;
		int ret = terrain_data->RayPickWorld(&outPos, &outNormal, rayPos, Math::Vector3(0.0f, -1.0f, 0.0f), 4.0f);
		if (floor_pos)*floor_pos = outPos;
		if (face_normal)*face_normal = outNormal;
		if (material_name) *material_name = terrain_data->GetMaterialName(ret);
		if (ret < 0)return FLT_MAX;
		return (outPos - rayPos).Length();
	}
	void Actor::SmoothRotateY(float coefficient) {
		//この先回転
		float dist = moveDirection.Length();
		if (dist > 0) {
			Math::Vector2 x0(moveDirection.x, moveDirection.z);
			Math::Vector2 x1(sinf(transform.rotation.y), cosf(transform.rotation.y));
			float cross = Math::Vector2::Cross(x0, x1);
			float dot = Math::Vector2::Dot(x0, x1) / dist;
			float adjust = (1 - dot)*2.0f;
			if (adjust > coefficient * Application::GetInstance()->GetProcessTimeSec()*acceleration)adjust = coefficient * Application::GetInstance()->GetProcessTimeSec()*acceleration;
			if (cross < 0.0f)transform.rotation.y -= adjust;
			else transform.rotation.y += adjust;
		}
	}
	void Actor::Gravity(float gravity) { this->gravity = gravity; }
	void Actor::CollisionTerrainWall(Graphics::Model* terrain, std::string* mat_name) {
		float moveLength = move.Length();
		Math::Vector3 tempMove = move;
		tempMove.y = 0.0f;
		tempMove.Normalize();
		//面法線
		Math::Vector3 faceNormal;
		float length = CheckWall(terrain, tempMove, 10.0f, &faceNormal, mat_name);
		//壁にめり込んでいるとき
		if (length >= 0.0f&&length < moveLength) {
			faceNormal.Normalize();
			//xz平面での射影成分を取得()
			float cosTheata = Math::Vector2::Dot(Math::Vector2(tempMove.x, tempMove.z), Math::Vector2(faceNormal.x, faceNormal.z));
			//移動できる距離を算出
			length = moveLength - length;
			tempMove = move - faceNormal * cosTheata*length*1.50f;
			moveLength = tempMove.Length();
			faceNormal = tempMove; faceNormal.Normalize();
			//その横にも壁があるか確認
			length = CheckWall(terrain, faceNormal, 10.0f, nullptr, nullptr);
			//無ければめり込まないように補正をかけた移動量を代入
			if (length < 0.0f || length > moveLength) {
				move.x = tempMove.x / 1.5f;
				move.z = tempMove.z / 1.5f;
			}
			else move.x = move.z = 0.0f;
		}
	}
	void Actor::CollisionTerrainFloor(Graphics::Model* terrain, std::string* mat_name) {
		Math::Vector3 pos;
		Math::Vector3 bias(0.0f, 1.0f, 0.0f);
		float length = CheckFloor(terrain, bias, &pos, nullptr, nullptr) - bias.y;
		//地面貫通する場合
		if (length < -move.y) {
			//貫通しない分だけ移動するようにする
			move.y = -length;
			jumpPower = 0.0f;
		}
	}
	void Actor::DefaultBehavior(Graphics::Model* terrain, std::string* wall_name, std::string* floor_name) {
		CalcMove();
		Gravity();
		CollisionTerrainFloor(terrain, floor_name);
		CollisionTerrainWall(terrain, wall_name);
		Move();
		SmoothRotateY();
	}
	bool Actor::CheckActorVision(Actor* actor, float vision_rad, float vision_length) {
		//まず距離判定
		Math::Vector3 direction = actor->GetPos() - GetPos();
		direction.y = 0.0f;
		if (direction.Length() > vision_length)return false;
		direction.Normalize();
		//視界判定
		float dot = Math::Vector3::Dot(GetRadVec3(), direction);
		//角度算出 0~PI
		float rad = acosf(dot);
		return (rad < vision_rad);
	}
}