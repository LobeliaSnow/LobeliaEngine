#include "Lobelia.hpp"
#include "Common/ComputeBuffer.hpp"
#include "Common/Raycaster.hpp"

namespace Lobelia::Game {
	//---------------------------------------------------------------------------------------------
	//
	//	Raycaster
	//
	//---------------------------------------------------------------------------------------------
	RayMesh::RayMesh(Graphics::Model* model) {
		auto mesh = model->GetMesh();
		//バッファの作成
		polygonCount = i_cast(mesh->GetCount() / 3);
		structuredBuffer = std::make_shared<StructuredBuffer>(sizeof(Input), polygonCount);
		//当たり判定用メッシュの構築
		Graphics::Model::Vertex* srcBuffer = mesh->GetBuffer();
		std::vector<Input> buildMesh(polygonCount);
		for (int i = 0; i < polygonCount; i++) {
			for (int j = 0; j < 3; j++) {
				buildMesh[i].pos[j].x = srcBuffer[i * 3 + j].pos.x;
				buildMesh[i].pos[j].y = srcBuffer[i * 3 + j].pos.y;
				buildMesh[i].pos[j].z = srcBuffer[i * 3 + j].pos.z;
			}
		}
		structuredBuffer->Update(buildMesh.data());
	}
	void RayMesh::Set() { structuredBuffer->Set(1, Graphics::ShaderStageList::CS); }
	int RayMesh::GetPolygonCount() { return polygonCount; }
	//---------------------------------------------------------------------------------------------
	RayResult::RayResult(RayMesh* mesh) {
		structuredBuffer = std::make_shared<StructuredBuffer>(sizeof(Output), mesh->GetPolygonCount());
		uav = std::make_unique<UnorderedAccessView>(structuredBuffer.get());
		readBuffer = std::make_unique<ReadGPUBuffer>(structuredBuffer);
	}
	void RayResult::Set() { uav->Set(1); }
	void RayResult::Clean() { uav->Clean(1); }
	void RayResult::Load() { readBuffer->ReadCopy(); }
	const RayResult::Output* RayResult::Lock() {
		return readBuffer->ReadBegin<Output>();
	}
	void RayResult::UnLock() { readBuffer->ReadEnd(); }
	//---------------------------------------------------------------------------------------------
	std::unique_ptr<Graphics::ComputeShader> Raycaster::cs;
	std::unique_ptr<Graphics::ConstantBuffer<Raycaster::Info>> Raycaster::cbuffer;
	Raycaster::Info Raycaster::info;
	void Raycaster::Initialize() {
		cs = std::make_unique<Graphics::ComputeShader>("Data/ShaderFile/GPGPU/GPGPU.hlsl", "RaycastCS");
		cbuffer = std::make_unique<Graphics::ConstantBuffer<Info>>(11, Graphics::ShaderStageList::CS);
	}
	bool Raycaster::Dispatch(const DirectX::XMMATRIX& world, RayMesh* mesh, RayResult* result, const Math::Vector3& begin, const Math::Vector3& end) {
		if ((begin - end).Length() == 0.0f)return false;
#ifdef _DEBUG
		//Rayのデバッグ表示
		Graphics::DebugRenderer::GetInstance()->SetLine(begin, end, 0xFFFFFFFF);
#endif
		//GPUにバッファ送信
		mesh->Set(); result->Set();
		//Rayの情報更新
		info.rayBegin.x = begin.x; info.rayBegin.y = begin.y; info.rayBegin.z = begin.z;
		info.rayEnd.x = end.x; info.rayEnd.y = end.y; info.rayEnd.z = end.z;
		//行列周り設定
		DirectX::XMStoreFloat4x4(&info.world, DirectX::XMMatrixTranspose(world));
		DirectX::XMVECTOR temp = {};
		DirectX::XMMATRIX inverse = DirectX::XMMatrixInverse(&temp, world);
		DirectX::XMStoreFloat4x4(&info.worldInverse, DirectX::XMMatrixTranspose(inverse));
		cbuffer->Activate(info);
		//Ray判定開始
		cs->Dispatch(mesh->GetPolygonCount(), 1, 1);
		result->Load();
		return true;
	}

}