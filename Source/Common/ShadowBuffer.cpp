#include "Lobelia.hpp"
#include "../Data/ShaderFile/Define.h"
#include "Common/DeferredBuffer.hpp"
#include "Common/ShadowBuffer.hpp"
#include "Common/ComputeBuffer.hpp"
#include "Common/PostEffect.hpp"

namespace Lobelia::Game {
	ShadowBuffer::ShadowBuffer(const Math::Vector2& size, int split_count, bool use_variance) :size(size), count(split_count) {
		rts.resize(split_count); views.resize(split_count);
		gaussian.resize(split_count);
		DXGI_FORMAT format = DXGI_FORMAT_R16G16_FLOAT;
		if (QUALITY > 0.5f)format = DXGI_FORMAT_R32G32_FLOAT;
		nearZ = 1.0f;
		farZ = 600.0f;
		lamda = 0.5f;
		fov = PI / 4.0f;
		aspect = size.x / size.y;
		Math::Vector2 ssize = size;
		for (int i = 0; i < split_count; i++) {
			rts[i] = std::make_shared<Graphics::RenderTarget>(size, DXGI_SAMPLE_DESC{ 1,0 }, format);
			//�����̎���p�J�����̒u���ʒu����ł͂��������i���Đ��x�グ��邩���B
			views[i] = std::make_unique<Graphics::View>(Math::Vector2(), size, fov, 1, 1000.0f);
			//�k���o�b�t�@�ɂ��ď����҂��̂���������
#ifdef GAUSSIAN_CS
			gaussian[i] = std::make_unique<GaussianFilterCS>(size, format);
#endif
#ifdef GAUSSIAN_PS
			gaussian[i] = std::make_unique<GaussianFilterPS>(size, format);
#endif
			gaussian[i]->SetDispersion(0.1f);
		}
		sampler = std::make_unique<Graphics::SamplerState>(Graphics::SAMPLER_PRESET::COMPARISON_LINEAR, 16);
		vs = std::make_shared<Graphics::VertexShader>("Data/ShaderFile/3D/deferred.hlsl", "CreateShadowMapVS", Graphics::VertexShader::Model::VS_5_0, false);
		ps = std::make_shared<Graphics::PixelShader>("Data/ShaderFile/3D/deferred.hlsl", "CreateShadowMapPS", Graphics::PixelShader::Model::PS_5_0, false);
		cbuffer = std::make_unique<Graphics::ConstantBuffer<Info>>(10, Graphics::ShaderStageList::VS | Graphics::ShaderStageList::PS);
		info.useShadowMap = TRUE; info.useVariance = i_cast(use_variance);
#ifdef _DEBUG
		HostConsole::GetInstance()->IntRegister("deferred", "use shadow", &info.useShadowMap, false);
		HostConsole::GetInstance()->IntRegister("deferred", "use variance", &info.useVariance, false);
#endif
	}
	void ShadowBuffer::SetNearPlane(float near_z) { nearZ = near_z; }
	void ShadowBuffer::SetFarPlane(float far_z) { farZ = far_z; }
	void ShadowBuffer::SetLamda(float lamda) { this->lamda = lamda; }
	void ShadowBuffer::SetPos(const Math::Vector3& pos) { this->pos = pos; }
	void ShadowBuffer::SetTarget(const Math::Vector3& at) { this->at = at; }
	namespace {
		//Vector3�̊e�v�f�ɔ�r�֐��������ĕԂ�
		auto ExecuteVector3Component(const Math::Vector3& v0, const Math::Vector3 v1, std::function<float(float, float)> func) {
			Math::Vector3 out = {};
			for (int i = 0; i < 3; i++) {
				out.v[i] = func(v0.v[i], v1.v[i]);
			}
			return out;
		}
	}
	AABB ShadowBuffer::CalcFrustumAABB(Graphics::View* main_camera, float near_z, float far_z, const DirectX::XMFLOAT4X4& lvp) {
		//������̍쐬
		Math::FrustumVertices frustum = {};
		Math::CreateFrustumVertices(main_camera->GetEyePos(), main_camera->GetEyeTarget(), main_camera->GetEyeUpDirection(), main_camera->GetFov(), near_z, far_z, main_camera->GetAspect(), &frustum);
		DirectX::XMFLOAT4 storageVert(frustum[0].x, frustum[0].y, frustum[0].z, 1.0f);
		//���W�X�^����邪�A���̉��̃X�R�[�v�ȊO�ł��g���̂ł�����
		DirectX::XMMATRIX calcLVP = DirectX::XMLoadFloat4x4(&lvp);
		{//���W�X�^�����ϐ��̓X�R�[�v�؂��Ď�����Z��
			DirectX::XMVECTOR calcPos = DirectX::XMLoadFloat4(&storageVert);
			//�V���h�E�}�b�v�ˉe��ԂɎ����䒸�_��ϊ�
			calcPos = DirectX::XMVector4Transform(calcPos, calcLVP);
			//�v�Z���ʂ�Ԃ�
			DirectX::XMStoreFloat4(&storageVert, calcPos);
		}
		AABB outAABB = {};
		outAABB.min = { storageVert.x ,storageVert.y, storageVert.z };
		outAABB.max = { storageVert.x ,storageVert.y, storageVert.z };
		//AABB�����߂�
		for (int i = 1; i < 8; i++) {
			storageVert = { frustum[i].x, frustum[i].y, frustum[i].z, 1.0f };
			DirectX::XMVECTOR calcPos = DirectX::XMLoadFloat4(&storageVert);
			//�V���h�E�}�b�v�ˉe��ԂɎ����䒸�_��ϊ�
			calcPos = DirectX::XMVector4Transform(calcPos, calcLVP);
			//�v�Z���ʂ�Ԃ�
			DirectX::XMStoreFloat4(&storageVert, calcPos);
			outAABB.min = ExecuteVector3Component(outAABB.min, Math::Vector3(storageVert.x, storageVert.y, storageVert.z), [=](float a, float b) {return min(a, b); });
			outAABB.max = ExecuteVector3Component(outAABB.max, Math::Vector3(storageVert.x, storageVert.y, storageVert.z), [=](float a, float b) {return max(a, b); });
		}
		return outAABB;
	}
	void ShadowBuffer::ComputeSplit(float lamda, float near_z, float far_z, float* split_pos) {
#ifdef CASCADE
		//�ʏ�̃V���h�E�}�b�v
		if (count == 1) {
			split_pos[0] = near_z;
			split_pos[1] = far_z;
			return;
		}
		//�J�X�P�[�h
		float invM = 1.0f / f_cast(count);
		float farDivisionNear = far_z / near_z;
		float farSubNear = far_z - near_z;
		//���p�����X�L�[����K�p
		// �� GPU Gems 3, Chapter 10. Parallel-Split Shadow Maps on Programmable GPUs.
		//    http://http.developer.nvidia.com/GPUGems3/gpugems3_ch10.html ���Q��.
		for (int i = 1; i < count + 1; i++) {
			//�ΐ������X�L�[��
			float log = near_z * powf(farDivisionNear, invM*i);
			//��l�����X�L�[��
			float uni = near_z + farSubNear * i*invM;
			//��L��2�̉��Z���ʂ���`��Ԃ���
			split_pos[i] = lamda * log + uni * (1.0f - lamda);
		}
		split_pos[0] = near_z;
		split_pos[count] = far_z;
#endif
	}
	//AABB���N���b�v���镽�s���e�s��쐬
	DirectX::XMFLOAT4X4 ShadowBuffer::FrustumAABBClippingMatrix(AABB clip_aabb) {
		Math::Vector3 scale; Math::Vector3 offset;
		Math::Vector3 aabbSize = clip_aabb.max - clip_aabb.min;
		//�X�P�[����AABB�̃T�C�Y
		scale = Math::Vector3(2.0f, 2.0f, 2.0f) / (aabbSize);
		//�����]�T������
		scale *= 0.85f;
		//�ʒu��AABB�̒��S*�X�P�[��
		offset = -0.5f*(clip_aabb.max + clip_aabb.min)*scale;
		//�������Ȃ肷�����ꍇ�␳����
		scale = ExecuteVector3Component(scale, Math::Vector3(1.0f, 1.0f, 1.0f), [=](float a, float b) {return max(a, b); });
		scale.z = 1.0f; offset.z = 0.0f;
		//�s��쐬
		DirectX::XMFLOAT4X4 out = {};
		out._11 = scale.x;	out._12 = 0.0f;		out._13 = 0.0f;		out._14 = 0.0f;
		out._21 = 0.0f;		out._22 = scale.y;	out._23 = 0.0f;		out._24 = 0.0f;
		out._31 = 0.0f;		out._32 = 0.0f;		out._33 = scale.z;	out._34 = 0.0f;
		out._41 = offset.x; out._42 = offset.y;	out._43 = offset.z;	out._44 = 1.0f;
		return out;
	}
	void ShadowBuffer::CreateCascade(Graphics::View* main_camera, const DirectX::XMFLOAT4X4& lvp) {
		//�����ʒu�v�Z
		std::unique_ptr<float[]> splitPos = std::make_unique<float[]>(count + 1);
		//���C���J�����𕪊�
		ComputeSplit(lamda, main_camera->GetNear(), main_camera->GetFar(), splitPos.get());
		for (int i = 0; i < count; i++) {
			//�������ꂽ���C���J�����̎������AABB���v�Z(���[�J�����)
			AABB frustumAABB = CalcFrustumAABB(main_camera, splitPos[i], splitPos[i + 1], lvp);
			//�N���b�s���O�ˉe�s��
			DirectX::XMFLOAT4X4 clipMatrix = FrustumAABBClippingMatrix(frustumAABB);
			//�s��ƕ�������
			//�Ƃ肠�������Ń��[�J���ɒu��
			DirectX::XMMATRIX calcLVP = DirectX::XMLoadFloat4x4(&lvp);
			DirectX::XMMATRIX calcClip = DirectX::XMLoadFloat4x4(&clipMatrix);
			DirectX::XMMATRIX cascadeLVP = calcLVP * calcClip;
			DirectX::XMStoreFloat4x4(&info.cascadeLVP[i], DirectX::XMMatrixTranspose(cascadeLVP));
			info.splitRange[i] = splitPos[i + 1];
		}
	}
	void ShadowBuffer::CameraUpdate(Graphics::View* main_camera) {
		//���C�g�s��쐬
		Math::Vector3 front = at - pos; front.Normalize();
		up = Math::Vector3(0.0f, 1.0f, 0.0f); up.Normalize();
		Math::Vector3 right = Math::Vector3::Cross(up, front); right.Normalize();
		up = Math::Vector3::Cross(front, right);
		DirectX::XMFLOAT4X4 lvp = {};
		{//���W�X�^����ϐ�������̂ŃX�R�[�v��؂�
			DirectX::XMFLOAT4 storage(pos.x, pos.y, pos.z, 1.0f);
			DirectX::XMVECTOR calcPos = DirectX::XMLoadFloat4(&storage);
			storage = { at.x, at.y, at.z, 1.0f };
			DirectX::XMVECTOR calcAt = DirectX::XMLoadFloat4(&storage);
			storage = { up.x, up.y, up.z, 1.0f };
			DirectX::XMVECTOR calcUp = DirectX::XMLoadFloat4(&storage);
			DirectX::XMMATRIX lightView = DirectX::XMMatrixLookAtLH(calcPos, calcAt, calcUp);
			DirectX::XMMATRIX lightProjection = DirectX::XMMatrixOrthographicLH(400, 400, nearZ, farZ);
			//DirectX::XMMATRIX lightProjection = DirectX::XMMatrixPerspectiveFovLH(fov, aspect, nearZ, farZ);
			//�g���܂킵
			lightView = lightView * lightProjection;
			DirectX::XMStoreFloat4x4(&lvp, lightView);
		}
		CreateCascade(main_camera, lvp);
		//#ifdef CASCADE
		//		info.pos.x = pos.x; info.pos.y = pos.y; info.pos.z = pos.z; info.pos.w = 1.0f;
		//		info.front.x = front.x; info.front.y = front.y; info.front.z = front.z; info.front.w = 0.0f;
		//		ComputeSplit(lamda, nearZ, farZ);
		//#endif
		//		for (int i = 0; i < count; i++) {
		//			views[i]->SetEyePos(pos);
		//			views[i]->SetEyeTarget(at);
		//			views[i]->SetEyeUpDirection(up);
		//#ifdef CASCADE
		//			views[i]->SetNear(cascadeValues[i]);
		//			views[i]->SetFar(cascadeValues[count]);
		//#endif
		//		}
	}
	void ShadowBuffer::AddModel(std::shared_ptr<Graphics::Model> model) { models.push_back(model); }
	void ShadowBuffer::CreateShadowMap(Graphics::View* active_view, Graphics::RenderTarget* active_rt) {
#ifdef _DEBUG
		if (Input::GetKeyboardKey(DIK_0) == 1)info.useShadowMap = !info.useShadowMap;
		if (Input::GetKeyboardKey(DIK_9) == 1)info.useVariance = !info.useVariance;
#endif
		if (!info.useShadowMap) {
			models.clear();
			return;
		}
		CameraUpdate(active_view);
		auto& defaultVS = Graphics::Model::GetVertexShader();
		auto& defaultPS = Graphics::Model::GetPixelShader();
		Graphics::Model::ChangeVertexShader(vs);
		Graphics::Model::ChangeVertexShader(vs);
		Graphics::Model::ChangePixelShader(ps);
		for (int i = 0; i < count; i++) {
			views[i]->Activate();
			rts[i]->Clear(0xFFFFFFFF);
			rts[i]->Activate();
			info.nowIndex = i;
			cbuffer->Activate(info);
			for (auto&& weak : models) {
				if (weak.expired())continue;
				auto model = weak.lock();
				model->ChangeAnimVS(vs);
				model->Render();
			}
		}
		models.clear();
		Graphics::Model::ChangeVertexShader(defaultVS);
		Graphics::Model::ChangePixelShader(defaultPS);
		active_view->Activate();
		active_rt->Activate();
		//�K�E�X�ɂ��ڂ��� �o���A���X�p
		if (info.useVariance) {
			for (int i = 0; i < count; i++) {
				gaussian[i]->Dispatch(active_view, active_rt, rts[i]->GetTexture());
			}
		}
	}
	void ShadowBuffer::Begin() {
		//���̍X�V
		//DirectX::XMStoreFloat4x4(&info.view, views[0]->GetColumnViewMatrix());
		sampler->Set(1);
		for (int i = 0; i < count; i++) {
			//DirectX::XMStoreFloat4x4(&info.proj[i], views[i]->GetColumnProjectionMatrix());
			if (info.useVariance) {
				gaussian[i]->Begin(7 + i);
			}
			else rts[i]->GetTexture()->Set(7 + i, Graphics::ShaderStageList::PS);
		}
		cbuffer->Activate(info);
	}
	void ShadowBuffer::End() {
		if (info.useVariance) {
			for (int i = 0; i < count; i++) {
				gaussian[i]->End();
			}
		}
		else {
			for (int i = 0; i < count; i++) {
				Graphics::Texture::Clean(7 + i, Graphics::ShaderStageList::PS);
			}
		}
	}
	void ShadowBuffer::DebugRender() {
		for (int i = 0; i < count; i++) {
			Graphics::SpriteRenderer::Render(rts[i].get(), Math::Vector2(i*100.0f, 100.0f), Math::Vector2(100.0f, 100.0f), 0.0f, Math::Vector2(), size, 0xFFFFFFFF);
			gaussian[i]->DebugRender(Math::Vector2(i*100.0f, 200.0f), Math::Vector2(100.0f, 100.0f));
		}
	}

}