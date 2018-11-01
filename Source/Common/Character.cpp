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
		resultBack = std::make_unique<RayResult>(terrain.get());
	}
	void Character::GPURaycastFloor() {
		if (rayMesh.expired())return;
		auto mesh = rayMesh.lock();
		Math::Vector3 pos = GetPos() + Math::Vector3(0.0f, 2.0f, 0.0f);
		DirectX::XMMATRIX world;
		terrain.lock()->GetWorldMatrix(&world);
		Raycaster::Dispatch(world, mesh.get(), result.get(), pos, pos - Math::Vector3(0.0f, 5.0f, 0.0f));
		auto r = result->Lock();
		int polyCount = mesh->GetPolygonCount();
		for (int i = 0; i < polyCount; i++) {
			if (r[i].hit) {
				float length = r[i].length - 2.0f;
				//�n�ʊђʂ���ꍇ
				if (length < -move.y) {
					//�ђʂ��Ȃ��������ړ�����悤�ɂ���
					move.y = -length;
					jumpPower = 0.0f;
					//break;
				}
			}
		}
		result->UnLock();
		std::swap(result, resultBack);
	}
	void Character::GPURaycastWall() {
		if (rayMesh.expired())return;
		//�ړ��l����
		float moveLength = move.Length();
		Math::Vector3 tempMove = move;
		tempMove.y = 0.0f; tempMove.Normalize();
		//Ray���ˊJ�n
		auto mesh = rayMesh.lock();
		Math::Vector3 pos = GetPos() + Math::Vector3(0.0f, 1.5f, 0.0f) - moveDirection * 2.0f;
		DirectX::XMMATRIX world;
		terrain.lock()->GetWorldMatrix(&world);
		//Ray����
		if (!Raycaster::Dispatch(world, mesh.get(), result.get(), pos, pos + moveDirection * 10.0f))return;
		//GPU�����҂�(�{���Ȃ��������CPU�ŏ������s�����ق����������邪�A�ėp�����d�����܂�)
		auto r = result->Lock();
		int polyCount = mesh->GetPolygonCount();
		//��ԋ߂��ǂ�������
		struct Min {
			int index = -1;
			float length = 9999999.0f;
		}min;
		//�������Ă���ŒZ�_������
		for (int i = 0; i < polyCount; i++) {
			if (r[i].hit) {
				float length = r[i].length - 5.0f;
				if (min.length > length) {
					min.index = i;
					min.length = length;
				}
			}
		}
		//�ړ��悪�ǂ��ђʂ���ꍇ
		if (min.index >= 0 && min.length >= 0.0f && min.length < moveLength) {
			//�������Ă���ʖ@�����擾
			Math::Vector3 normal = r[min.index].normal;
			//GPU�ɕԂ�
			result->UnLock();
			//xz���ʂł̎ˉe�������擾
			float cosTheata = Math::Vector2::Dot(Math::Vector2(tempMove.x, tempMove.z), Math::Vector2(normal.x, normal.z));
			//�ړ��ł����
			float length = moveLength - min.length;
			//�ǎC��x�N�g�����Z�o
			tempMove = move - normal * cosTheata * length * 1.50f;
			moveLength = tempMove.Length();
			Math::Vector3 direction = tempMove; direction.Normalize();
			if (!Raycaster::Dispatch(world, mesh.get(), result.get(), pos, pos + direction * 10.0f))return;
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
		//GPU�ɂ��Ray����
		//GPURaycastWall();
		GPURaycastFloor();
#endif
		Move();
		SmoothRotateY();
		Translation(GetPos());
		RotationYAxis(GetRad());
		CalcWorldMatrix();
	}
}