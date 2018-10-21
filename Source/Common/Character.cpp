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
				//’n–ÊŠÑ’Ê‚·‚éê‡
				if (length < -move.y) {
					//ŠÑ’Ê‚µ‚È‚¢•ª‚¾‚¯ˆÚ“®‚·‚é‚æ‚¤‚É‚·‚é
					move.y = -length;
					jumpPower = 0.0f;
					break;
				}
			}
		}
		result->UnLock();
	}
#endif
	void Character::Update(const Math::Vector3& front) {
		if (Input::GetKeyboardKey(DIK_SPACE) == 1)Jump(60);
		Gravity();
		CalcMove();
#ifdef GPU_RAYCASTER
		//GPU‚É‚æ‚éRay”­ŽË
		//RayŒvŽZ’†‚É‚Å‚«‚é‚±‚Æ‚ð­‚µ‚Å‚àÏ‚Ü‚·
		GPURaycastFloor1Pass();
#endif
		Math::Vector3 right = Math::Vector3::Cross(Math::Vector3(0.0f, 1.0f, 0.0f), front); right.Normalize();
		Math::Vector3 move = {};
		if (Input::GetKeyboardKey(DIK_W))move += front;
		if (Input::GetKeyboardKey(DIK_S))move -= front;
		if (Input::GetKeyboardKey(DIK_D))move += right;
		if (Input::GetKeyboardKey(DIK_A))move -= right;
		move.Normalize();
		SetMove(move, 10.0f);
		AnimationUpdate(Application::GetInstance()->GetProcessTimeMili());
		CalcMove();
#ifdef CPU_RAYCASTER
		//CollisionTerrainWall(terrain.lock().get(), nullptr);
		CollisionTerrainFloor(terrain.lock().get(), nullptr);
#endif
#ifdef GPU_RAYCASTER
		GPURaycastFloor2Pass();
#endif
		Move();
		SmoothRotateY();
		Translation(GetPos());
		RotationYAxis(GetRad());
		CalcWorldMatrix();
	}
}