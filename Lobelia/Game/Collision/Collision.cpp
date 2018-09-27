#include "Lobelia.hpp"
#include "Game/Collision/Collision.hpp"
//#include "Lobelia/Common.hpp"
#include "Game/Actor/Actor.hpp"

namespace Lobelia::Game {
	void Shape3D::IssueHitEvent(Actor* other_actor) { actor->HitEvent(other_actor); }
	Sphere::Sphere(const Math::Vector3& pos, float radius) :Shape3D(ShapeType3D::SPHERE, pos), radius(radius) {
		SetAABBRadius(Math::Vector3(radius, radius, radius));
	};
	Capsule3D::Capsule3D(const Math::Vector3& start_pos, const Math::Vector3& end_pos, float radius) : Shape3D(ShapeType3D::CAPSULE, (start_pos + end_pos) / 2.0f), radius(radius), startPosition(start_pos), endPosition(end_pos) {}
	AABB3D::AABB3D(const Math::Vector3& pos, const Math::Vector3& radius) : Shape3D(ShapeType3D::AABB, pos) {
		SetAABBRadius(radius);
	}
	bool Collision3D::IsForce(Shape3D* s0, Shape3D* s1) { return (s0->IsForce() && s1->IsForce()); }
	Actor* Collision3D::ForceActor(Shape3D* s0, Shape3D*s1) {
		if (!s0->IsStatic())return s0->GetActor();
		else if (!s1->IsStatic())return s1->GetActor();
		return nullptr;
	}
	bool SphereToShpere::Execute(Shape3D* s0, Shape3D*s1) {
		Sphere* c0 = d_cast<Sphere*>(s0);
		Sphere* c1 = d_cast<Sphere*>(s1);
		if (!c0 || !c1)STRICT_THROW("対応していないシェイプです");
		Math::Vector3 dist = c1->GetPos() - c0->GetPos();
		float length = dist.LengthSq();
		float judgeAling = (c0->GetRadius() + c1->GetRadius())*(c0->GetRadius() + c1->GetRadius());
		bool hit = (length < judgeAling);
		//押し戻し
		if (hit&&IsForce(s0, s1)) {
			//両方が動かないオブジェクトだと押し戻しは発生しない
			Actor* actor = ForceActor(s0, s1);
			if (actor) {
				dist.y = 0.0f; dist.Normalize();
				float bias = judgeAling - length;
				Math::Vector3 noraml;
				//float wallLength = actor->CheckWall(MetaAI::GetInstance()->GetTerrain(), -dist, bias, &noraml, nullptr);
				//if (wallLength < bias) bias = wallLength - 0.1f;
				if (actor == s0->GetActor())actor->SetPos(actor->GetPos() - dist * bias);
				if (actor == s1->GetActor())actor->SetPos(actor->GetPos() + dist * bias);

			}
		}
		return hit;
	}
	bool AABB3DToAABB3D::Execute(Shape3D* s0, Shape3D*s1) {
		AABB3D* aabb0 = d_cast<AABB3D*>(s0);
		AABB3D* aabb1 = d_cast<AABB3D*>(s1);
		//AABB情報作成
		Math::Vector3 min0 = aabb0->GetMin();
		Math::Vector3 max0 = aabb0->GetMax();
		Math::Vector3 min1 = aabb1->GetMin();
		Math::Vector3 max1 = aabb1->GetMax();
		//あってるか知らん
		bool hit = (min0.x < max1.x && max0.x > min1.x && min0.y < max1.y && max0.y > min1.y && min0.z < max1.z && max0.z > min1.z);
		if (hit&&IsForce(s0, s1)) {
			//ここから未実装
			Actor* actor = ForceActor(s0, s1);
			Math::Vector3 amount0 = max0 - min1;
			Math::Vector3 amount1 = max1 - min0;
			if (amount0.Length() < amount1.Length()) {

			}
			Math::Vector3 pos = actor->GetPos();
			actor->SetPos(pos);
		}
		return hit;
	}
	float SphereToAABB3D::Check(float pn, float min, float max) {
		float out = 0.0f;
		float v = pn;
		if (v < min) {
			float val = min - v;
			out += val * val;
		}
		if (v > max) {
			float val = v - max;
			out += val * val;
		}
		return out;
	};
	bool SphereToAABB3D::Execute(Shape3D* s0, Shape3D*s1) {
		Sphere* sphere = d_cast<Sphere*>(s0);
		if (!sphere)sphere = d_cast<Sphere*>(s1);
		AABB3D* aabb = d_cast<AABB3D*>(s1);
		if (!aabb) aabb = d_cast<AABB3D*>(s0);
		if (!sphere || !aabb)STRICT_THROW("シェイプの条件を満たせませんでした");
		Math::Vector3 min = aabb->GetMin();
		Math::Vector3 max = aabb->GetMax();
		float val = 0.0f;
		val += Check(sphere->GetPos().x, min.x, max.x);
		val += Check(sphere->GetPos().y, min.y, max.y);
		val += Check(sphere->GetPos().z, min.z, max.z);
		return (val <= sphere->GetRadius()*sphere->GetRadius());
	}
	bool Capsule3DToCapsule3D::Execute(Shape3D* s0, Shape3D*s1) {
		Capsule3D* capsule0 = d_cast<Capsule3D*>(s0);
		if (!capsule0)capsule0 = d_cast<Capsule3D*>(s1);
		Capsule3D* capsule1 = d_cast<Capsule3D*>(s1);
		if (!capsule1) capsule1 = d_cast<Capsule3D*>(s0);
		if (!capsule0 || !capsule1) STRICT_THROW("シェイプの条件を満たせませんでした");
		float length = Math::CalcSegmentSegmentLength(capsule0->GetStartPos(), capsule0->GetEndPos(), capsule1->GetStartPos(), capsule1->GetEndPos());
		return (length <= capsule0->GetRadius() + capsule1->GetRadius());
	}
	// カプセルを連続した球として衝突検知を行っている
	bool Capsule3DToAABB3D::Execute(Shape3D* s0, Shape3D*s1) {
		AABB3D* aabb = d_cast<AABB3D*>(s0);
		if (!aabb)aabb = d_cast<AABB3D*>(s1);
		Capsule3D* capsule1 = d_cast<Capsule3D*>(s1);
		if (!capsule1) capsule1 = d_cast<Capsule3D*>(s0);
		if (!aabb || !capsule1) STRICT_THROW("シェイプの条件を満たせませんでした");
		float offset_t = capsule1->GetRadius() / (capsule1->GetEndPos() - capsule1->GetStartPos()).Length();
		if (offset_t < .0f) STRICT_THROW("媒介変数の値が間違っています");
		float t = .0f;
		while (1) {
			Math::Vector3 min = aabb->GetMin();
			Math::Vector3 max = aabb->GetMax();
			Math::Vector3 pos = (capsule1->GetStartPos() + capsule1->GetEndPos()) * t;
			float val = 0.0f;
			val += Check(pos.x, min.x, max.x);
			val += Check(pos.y, min.y, max.y);
			val += Check(pos.z, min.z, max.z);
			if (val <= capsule1->GetRadius()*capsule1->GetRadius()) return true; // 衝突を検知
			else if (t < 1.0f) { // 次の球へ
				t += offset_t;
				if (t > 1.0f) t = 1.0f;
			}
			else return false; // 衝突なし
		}
		return false;
	}
	float Capsule3DToAABB3D::Check(float pn, float min, float max) {
		float out = 0.0f;
		float v = pn;
		if (v < min) {
			float val = min - v;
			out += val * val;
		}
		if (v > max) {
			float val = v - max;
			out += val * val;
		}
		return out;
	}
	bool SphereToCapsule3D::Execute(Shape3D* s0, Shape3D*s1) {
		Sphere* sphere = d_cast<Sphere*>(s0);
		if (!sphere)sphere = d_cast<Sphere*>(s1);
		Capsule3D* capsule = d_cast<Capsule3D*>(s1);
		if (!capsule) capsule = d_cast<Capsule3D*>(s0);
		if (!sphere || !capsule) STRICT_THROW("シェイプの条件を満たせませんでした");
		float length = Math::CalcPointSegmentLength(sphere->GetPos(), capsule->GetStartPos(), capsule->GetEndPos());
		return (length <= sphere->GetRadius() + capsule->GetRadius());
	}
	CollisionManager3DBase::CollisionManager3DBase() {
		collisionTable3D[i_cast(ShapeType3D::SPHERE)][i_cast(ShapeType3D::SPHERE)] = std::make_unique<SphereToShpere>();
		collisionTable3D[i_cast(ShapeType3D::AABB)][i_cast(ShapeType3D::AABB)] = std::make_unique<AABB3DToAABB3D>();
		collisionTable3D[i_cast(ShapeType3D::SPHERE)][i_cast(ShapeType3D::AABB)] = std::make_unique<SphereToAABB3D>();
		collisionTable3D[i_cast(ShapeType3D::AABB)][i_cast(ShapeType3D::SPHERE)] = std::make_unique<SphereToAABB3D>();

		collisionTable3D[i_cast(ShapeType3D::CAPSULE)][i_cast(ShapeType3D::CAPSULE)] = std::make_unique<Capsule3DToCapsule3D>();
		collisionTable3D[i_cast(ShapeType3D::CAPSULE)][i_cast(ShapeType3D::SPHERE)] = std::make_unique<SphereToCapsule3D>();
		collisionTable3D[i_cast(ShapeType3D::SPHERE)][i_cast(ShapeType3D::CAPSULE)] = std::make_unique<SphereToCapsule3D>();
		collisionTable3D[i_cast(ShapeType3D::AABB)][i_cast(ShapeType3D::CAPSULE)] = std::make_unique<Capsule3DToAABB3D>();
		collisionTable3D[i_cast(ShapeType3D::CAPSULE)][i_cast(ShapeType3D::AABB)] = std::make_unique<Capsule3DToAABB3D>();
	}
	bool CollisionManager3DBase::Execute(Shape3D* s0, Shape3D* s1) {
		ShapeType3D pair[2] = { s0->GetType(),s1->GetType() };
		return collisionTable3D[i_cast(pair[0])][i_cast(pair[1])]->Execute(s0, s1);
	}

	CollisionManager3D::CollisionManager3D() {
		octree = std::make_unique<OctreeManager<Shape3D>>();
		octree->Initialize(6, Math::Vector3(-80.0f, -10.0f, -80.0f), Math::Vector3(80.0f, 10.0f, 80.0f));
	}
	void CollisionManager3D::Update() {
		JudgeAliveShape();
#ifdef OCTREE
		DWORD count = 0;
		//八分岐リストに再登録
		for (auto&& ort = ortList.begin(); ort != ortList.end();) {
			(*ort)->Remove();
			if ((*ort)->GetObjectPointer().expired()) {
				ort = ortList.erase(ort);
				continue;
			}
			Shape3D* temp = (*ort)->GetObjectPointer().lock().get();
			const Math::Vector3& pos = temp->GetPos();
			const Math::Vector3& radius = temp->GetAABBRadius();
			Math::Vector3 aabbMin = pos - radius;
			Math::Vector3 aabbMax = pos + radius;
			octree->Register(aabbMin, aabbMax, (*ort));
			count++; ort++;
		}
		std::vector<Shape3D*> colVector; colVector.reserve(count*count * 2);
		DWORD colNum = octree->CreateAllCollisionList(colVector) / 2;
		for (int i = 0; i < colNum; i++) {
			auto s0 = colVector[i * 2];
			auto s1 = colVector[i * 2 + 1];
			if (s0->IsDisable() || s1->IsDisable())continue;
			if (Execute(s0, s1)) {
				if (s0->IsEnableHitEvent())s0->IssueHitEvent(s1->GetActor());
				if (s1->IsEnableHitEvent())s1->IssueHitEvent(s0->GetActor());
			}
		}
#else
		for each(auto&& shape0 in shape3DList) {
			std::shared_ptr<Shape3D> s0 = shape0.lock();
			if (s0->IsDisable())continue;
			for each(auto&& shape1 in shape3DList) {
				std::shared_ptr<Shape3D> s1 = shape1.lock();
				if (s1->IsDisable())continue;
				if (s0 == s1)continue;
				if (Execute(s0.get(), s1.get())) {
					if (s0->IsEnableHitEvent())s0->IssueHitEvent(s1->GetActor());
					if (s1->IsEnableHitEvent())s1->IssueHitEvent(s0->GetActor());
				}
			}
		}
#endif
	}

	void CollisionManager3D::JudgeAliveShape() {
		for (auto&& shape = shape3DList.begin(); shape != shape3DList.end();) {
			if (shape->expired())
				shape = shape3DList.erase(shape);
			else shape++;
		}
	}
	void CollisionSlotManager3D::Update() {
#ifdef OCTREE
		auto& list0 = list->playerSlot->shapeList;
		for (auto&& it = list0.begin(); it != list0.end();) {
			if (it->expired()) {
				it = list0.erase(it);
				continue;
			}
			else it++;
		}
		auto& list1 = list->enemySlot->shapeList;
		for (auto&& it = list1.begin(); it != list1.end();) {
			if (it->expired()) {
				it = list1.erase(it);
				continue;
			}
			else it++;
		}
		DWORD count = 0;
		//八分岐リストに再登録
		for (auto&& ort = ortList.begin(); ort != ortList.end();) {
			(*ort)->Remove();
			if ((*ort)->GetObjectPointer().expired()) {
				ort = ortList.erase(ort);
				continue;
			}
			Shape3D* temp = (*ort)->GetObjectPointer().lock().get();
			const Math::Vector3& pos = temp->GetPos();
			const Math::Vector3& radius = temp->GetAABBRadius();
			Math::Vector3 aabbMin = pos - radius;
			Math::Vector3 aabbMax = pos + radius;
			octree->Register(aabbMin, aabbMax, (*ort));
			count++; ort++;
		}
		std::vector<Shape3D*> colVector; colVector.reserve(count*count * 2);
		DWORD colNum = octree->CreateAllCollisionList(colVector) / 2;
		for (int i = 0; i < colNum; i++) {
			auto s0 = colVector[i * 2];
			auto s1 = colVector[i * 2 + 1];
			if (s0->IsDisable() || s1->IsDisable())continue;
			if (Execute(s0, s1)) {
				if (s0->IsEnableHitEvent())s0->IssueHitEvent(s1->GetActor());
				if (s1->IsEnableHitEvent())s1->IssueHitEvent(s0->GetActor());
			}
		}
#else
		auto& list0 = list->playerSlot->shapeList;
		auto& list1 = list->enemySlot->shapeList;
		for (auto&& shape0 = list0.begin(); shape0 != list0.end();) {
			if (shape0->expired()) {
				shape0 = list0.erase(shape0);
				continue;
			}
			std::shared_ptr<Shape3D> s0 = shape0->lock();
			if (s0->IsDisable())continue;
			for (auto&& shape1 = list1.begin(); shape1 != list1.end();) {
				if (shape1->expired()) {
					shape1 = list1.erase(shape1);
					continue;
				}
				std::shared_ptr<Shape3D> s1 = shape1->lock();
				if (s1->IsDisable())continue;
				if (s0 == s1)continue;
				if (Execute(s0.get(), s1.get())) {
					if (s0->IsEnableHitEvent())s0->IssueHitEvent(s1->GetActor());
					if (s1->IsEnableHitEvent())s1->IssueHitEvent(s0->GetActor());
				}
				shape1++;
			}
			shape0++;
		}
#endif
	}
}