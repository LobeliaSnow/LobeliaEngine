#include "Lobelia.hpp"
#include "../Data/ShaderFile/Define.h"
#include "Common/ComputeBuffer.hpp"
#include "Common/Raycaster.hpp"
#include "Common/Character.hpp"
#include "Common/Camera.hpp"

namespace Lobelia::Game {
	Character::Character() :Actor("character", 0), Graphics::Model("Data/Model/Character/character.dxd", "Data/Model/Character/character.mt") {
		Scalling(0.1f);
		AnimationActivate(AnimationLoad("Data/Model/Character/walk.anm"));
		//Translation(Math::Vector3(0.0f, 0.2f, 0.0f));
	}
	void Character::SetTerrainData(std::shared_ptr<Graphics::Model> terrain) { this->terrain = terrain; }
#ifdef GPU_RAYCASTER
	void Character::SetTerrainData(std::shared_ptr<RayMesh> terrain) {
		this->rayMesh = terrain;
		result = std::make_unique<RayResult>(terrain.get());
	}
	void Character::GPURaycastFloor1Pass() {
		if (rayMesh.expired())return;
		auto mesh = rayMesh.lock();
		Math::Vector3 pos = GetPos() + Math::Vector3(0.0f, 2.0f, 0.0f);
		DirectX::XMMATRIX world;
		terrain.lock()->GetWorldMatrix(&world);
		Raycaster::Dispatch(world, mesh.get(), result.get(), pos, pos - Math::Vector3(0.0f, 5.0f, 0.0f));
	}
	void Character::GPURaycastFloor2Pass() {
		if (rayMesh.expired())return;
		auto mesh = rayMesh.lock();
		auto r = result->Lock();
		int polyCount = mesh->GetPolygonCount();
		for (int i = 0; i < polyCount; i++) {
			if (r[i].hit) {
				float length = r[i].length - 2.0f;
				//地面貫通する場合
				if (length < -move.y) {
					//貫通しない分だけ移動するようにする
					move.y = -length;
					jumpPower = 0.0f;
					//break;
				}
			}
		}
		result->UnLock();
	}
	void Character::GPURaycastWall() {
		if (rayMesh.expired())return;
		//移動値周り
		float moveLength = move.Length();
		Math::Vector3 tempMove = move;
		tempMove.y = 0.0f; tempMove.Normalize();
		//Ray発射開始
		auto mesh = rayMesh.lock();
		Math::Vector3 pos = GetPos() + Math::Vector3(0.0f, 1.5f, 0.0f) - moveDirection * 2.0f;
		DirectX::XMMATRIX world;
		terrain.lock()->GetWorldMatrix(&world);
		//Ray発射
		Raycaster::Dispatch(world, mesh.get(), result.get(), pos, pos + moveDirection * 10.0f);
		//GPU同期待ち(本来ならもう少しCPUで処理を行ったほうが活かせるが、汎用性を重視します)
		auto r = result->Lock();
		int polyCount = mesh->GetPolygonCount();
		//一番近い壁を見つける
		struct Min {
			int index = -1;
			float length = 9999999.0f;
		}min;
		//当たっている最短点を検索
		for (int i = 0; i < polyCount; i++) {
			if (r[i].hit) {
				float length = r[i].length - 5.0f;
				if (min.length > length) {
					min.index = i;
					min.length = length;
				}
			}
		}
		//移動先が壁を貫通する場合
		if (min.index >= 0 && min.length >= 0.0f && min.length < moveLength) {
			//当たっている面法線を取得
			Math::Vector3 normal = r[min.index].normal;
			//GPUに返す
			result->UnLock();
			//xz平面での射影成分を取得
			float cosTheata = Math::Vector2::Dot(Math::Vector2(tempMove.x, tempMove.z), Math::Vector2(normal.x, normal.z));
			//移動できる量
			float length = moveLength - min.length;
			//壁擦りベクトルを算出
			tempMove = move - normal * cosTheata * length * 1.50f;
			moveLength = tempMove.Length();
			Math::Vector3 direction = tempMove; direction.Normalize();
			Raycaster::Dispatch(world, mesh.get(), result.get(), pos, pos + direction * 10.0f);
			r = result->Lock();
			for (int i = 0; i < polyCount; i++) {
				if (r[i].hit) {
					float length = r[i].length - 5.0f;
					if (min.length > length) {
						min.index = i;
						min.length = length;
					}
				}
			}
			result->UnLock();
			if (min.length < 0.0f || min.length > moveLength) {
				move = tempMove;
			}
			else move.x = move.z = 0.0f;
		}
		else result->UnLock();
	}
#endif
	void Character::Update(const Math::Vector3& front) {
		if (Input::GetKeyboardKey(DIK_SPACE) == 1)Jump(60);
		Math::Vector3 right = Math::Vector3::Cross(Math::Vector3(0.0f, 1.0f, 0.0f), front); right.Normalize();
		Math::Vector3 move = {};
		if (Input::GetKeyboardKey(DIK_W))move += front;
		if (Input::GetKeyboardKey(DIK_S))move -= front;
		if (Input::GetKeyboardKey(DIK_D))move += right;
		if (Input::GetKeyboardKey(DIK_A))move -= right;
		move.Normalize();
		SetMove(move, 10.0f);
		AnimationUpdate(Application::GetInstance()->GetProcessTimeMili());
#ifdef CPU_RAYCASTER
		Gravity();
#endif
		CalcMove();
#ifdef CPU_RAYCASTER
		CollisionTerrainWall(terrain.lock().get(), nullptr);
		CollisionTerrainFloor(terrain.lock().get(), nullptr);
#endif
#ifdef GPU_RAYCASTER
		Gravity();
		CalcMove();
		GPURaycastWall();
		//GPUによるRay発射
		//Ray計算中にできることを少しでも済ます
		GPURaycastFloor1Pass();
		GPURaycastFloor2Pass();
#endif
		Move();
		SmoothRotateY();
		Translation(GetPos());
		RotationYAxis(GetRad());
		CalcWorldMatrix();
	}
}