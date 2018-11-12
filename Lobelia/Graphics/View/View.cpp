#include "Common/Common.hpp"
#include "Math/Math.hpp"
#include "Exception/Exception.hpp"
#include "Graphics/GraphicDriverInfo/GraphicDriverInfo.hpp"
#include "Graphics/Device/Device.hpp"
#include "Graphics/BufferCreator/BufferCreator.h"
#include "Graphics/ConstantBuffer/ShaderStageList.hpp"
#include "Graphics/ConstantBuffer/ConstantBuffer.hpp"
#include "Config/Config.hpp"
#include "Graphics/View/View.hpp"

namespace Lobelia::Graphics {
	DirectX::XMMATRIX View::nowView;
	DirectX::XMMATRIX View::nowProjection;
	Math::Vector2 View::nowSize;

	View::View(const Math::Vector2& left_up, const Math::Vector2& size, float fov_rad, float near_z, float far_z) {
		nearZ = near_z;
		farZ = far_z;
		fov = fov_rad;
		aspect = static_cast<float>(size.x) / static_cast<float>(size.y);
		this->size = size - left_up;
		constantBuffer = std::make_unique<ConstantBuffer<Constant>>(0, Config::GetRefPreference().systemCBActiveStage);
		CreateViewport(left_up, size);
		CreateProjection(fov_rad, static_cast<float>(size.x) / static_cast<float>(size.y), near_z, far_z);
		data.at = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f);
		data.up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
		data.eye = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f);
		CreateView(data);
	}
	View::~View() = default;
	void View::SetEyePos(const Math::Vector3& pos) {
		data.eye = DirectX::XMVectorSet(pos.x, pos.y, pos.z, 1.0f);
		buffer.pos.x = pos.x;
		buffer.pos.y = pos.y;
		buffer.pos.z = pos.z;
		buffer.pos.w = 1.0f;
	}
	void View::SetEyeTarget(const Math::Vector3& target) { data.at = DirectX::XMVectorSet(target.x, target.y, target.z, 1.0f); }
	void View::SetEyeUpDirection(const Math::Vector3& up_direction) { data.up = DirectX::XMVectorSet(up_direction.x, up_direction.y, up_direction.z, 1.0f); }
	void View::SetFov(float fov_rad) { fov = fov_rad; }
	void View::SetNear(float near_z) { nearZ = near_z; }
	void View::SetFar(float far_z) { farZ = far_z; }
	void View::ChangeViewport(const Math::Vector2& pos, const Math::Vector2& size) { CreateViewport(pos, size); }
	void View::Update() {
		CreateProjection(fov, aspect, nearZ, farZ);
		CreateView(data);
		CreateBillboardMat(data);
		DirectX::XMMATRIX viewProjection = buffer.projection * buffer.view;
		DirectX::XMVECTOR temp = {};
		buffer.inverseViewProjection = DirectX::XMMatrixInverse(&temp, viewProjection);
	}
	void View::ViewportActivate() { Device::GetContext()->RSSetViewports(1, &viewport); }
	void View::Activate() {
		Update();
		constantBuffer->Activate(buffer);
		Device::GetContext()->RSSetViewports(1, &viewport);
		nowSize = size;
		nowView = buffer.view; nowProjection = buffer.projection;
	}
	void View::FrameEnd() {
		buffer.previousView = buffer.view;
		buffer.previousProjection = buffer.projection;
	}
	void View::CreateProjection(float fov_rad, float aspect, float near_z, float far_z) {
		buffer.projection = DirectX::XMMatrixPerspectiveFovLH(fov_rad, aspect, near_z, far_z);
		buffer.projection = DirectX::XMMatrixTranspose(buffer.projection);
	}
	void View::CreateView(const Data& data) {
		buffer.view = DirectX::XMMatrixLookAtLH(data.eye, data.at, data.up);
		buffer.view = DirectX::XMMatrixTranspose(buffer.view);
	}
	void View::CreateBillboardMat(const Data& data) {
		DirectX::XMVECTOR origin{ 0.0f, 0.0f, 0.0f, 0.0f };
		DirectX::XMVECTOR invAt = data.eye - data.at;
		invAt = DirectX::XMVector4Normalize(invAt);
		buffer.billboardMat = DirectX::XMMatrixLookAtLH(origin, invAt, data.up);
		DirectX::XMVECTOR arg = {};
		buffer.billboardMat = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(&arg, buffer.billboardMat));
	}
	void View::CreateViewport(const Math::Vector2& pos, const Math::Vector2& size) {
		viewport.Width = static_cast<FLOAT>(size.x);
		viewport.Height = static_cast<FLOAT>(size.y);
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		viewport.TopLeftX = static_cast<FLOAT>(pos.x);
		viewport.TopLeftY = static_cast<FLOAT>(pos.y);
		aspect = static_cast<float>(size.x) / static_cast<float>(size.y);
	}
	//TODO : ��قǊ֐�����
	void View::CreateFrustum() {
		auto XMVectorToLobeliaVector = [=](const DirectX::XMVECTOR& xm_vec, Math::Vector3* l_vec) {
			l_vec->x = DirectX::XMVectorGetX(xm_vec);
			l_vec->y = DirectX::XMVectorGetY(xm_vec);
			l_vec->z = DirectX::XMVectorGetZ(xm_vec);
		};
		//�f�[�^�擾
		Math::Vector3 pos, target, up;
		XMVectorToLobeliaVector(data.eye, &pos);
		XMVectorToLobeliaVector(data.at, &target);
		XMVectorToLobeliaVector(data.up, &up);
		//�J�����O���Z�o
		Math::Vector3 camZ(target - pos);
		camZ.Normalize();
		Math::Vector3 camX(Math::Vector3::Cross(up, camZ));
		camX.Normalize();
		Math::Vector3 camY(Math::Vector3::Cross(camZ, camX));
		camY.Normalize();
		//������̍����Z�o
		float nearHeight = tanf(fov*0.5f)*nearZ;
		float farHeight = tanf(fov*0.5f)*farZ;
		//������̕��擾
		float nearWidth = nearHeight * aspect;
		float farWidth = farHeight * aspect;
		//���S�ʒu���Z�o
		Math::Vector3 nearPlaneCenter = pos + camZ * nearZ;
		Math::Vector3 farPlaneCenter = pos + camZ * farZ;
		//�����䒸�_�Z�o
		Math::Vector3 frustumVertices[8];
		frustumVertices[0] = nearPlaneCenter - camX * nearWidth + camY * nearHeight;
		frustumVertices[1] = nearPlaneCenter - camX * nearWidth - camY * nearHeight;
		frustumVertices[2] = nearPlaneCenter + camX * nearWidth - camY * nearHeight;
		frustumVertices[3] = nearPlaneCenter + camX * nearWidth + camY * nearHeight;
		frustumVertices[4] = farPlaneCenter - camX * farWidth + camY * farHeight;
		frustumVertices[5] = farPlaneCenter - camX * farWidth - camY * farHeight;
		frustumVertices[6] = farPlaneCenter + camX * farWidth - camY * farHeight;
		frustumVertices[7] = farPlaneCenter + camX * farWidth + camY * farHeight;
		auto Vector3ToVector4 = [=](Math::Vector4* out, const Math::Vector3& in) {
			memcpy_s(out, sizeof(Math::Vector4), &in, sizeof(Math::Vector3));
		};
		//�d�_�Z�o
		Math::Vector3 temp;
		temp = (frustumVertices[0] + frustumVertices[1] + frustumVertices[2] + frustumVertices[3]) / 4.0f;
		Vector3ToVector4(&frustum.center[0], temp);
		temp = (frustumVertices[0] + frustumVertices[3] + frustumVertices[4] + frustumVertices[7]) / 4.0f;
		Vector3ToVector4(&frustum.center[1], temp);
		temp = (frustumVertices[0] + frustumVertices[1] + frustumVertices[4] + frustumVertices[5]) / 4.0f;
		Vector3ToVector4(&frustum.center[2], temp);
		temp = (frustumVertices[1] + frustumVertices[2] + frustumVertices[5] + frustumVertices[6]) / 4.0f;
		Vector3ToVector4(&frustum.center[3], temp);
		temp = (frustumVertices[3] + frustumVertices[2] + frustumVertices[7] + frustumVertices[6]) / 4.0f;
		Vector3ToVector4(&frustum.center[4], temp);
		temp = (frustumVertices[4] + frustumVertices[5] + frustumVertices[6] + frustumVertices[7]) / 4.0f;
		Vector3ToVector4(&frustum.center[5], temp);
		//�@���Z�o
		temp = Math::Vector3::Cross(frustumVertices[1] - frustumVertices[0], frustumVertices[2] - frustumVertices[0]);
		temp.Normalize(); Vector3ToVector4(&frustum.normal[0], temp);
		temp = Math::Vector3::Cross(frustumVertices[3] - frustumVertices[0], frustumVertices[4] - frustumVertices[0]);
		temp.Normalize(); Vector3ToVector4(&frustum.normal[1], temp);
		temp = Math::Vector3::Cross(frustumVertices[4] - frustumVertices[0], frustumVertices[1] - frustumVertices[0]);
		temp.Normalize(); Vector3ToVector4(&frustum.normal[2], temp);
		temp = Math::Vector3::Cross(frustumVertices[5] - frustumVertices[1], frustumVertices[2] - frustumVertices[1]);
		temp.Normalize(); Vector3ToVector4(&frustum.normal[3], temp);
		temp = Math::Vector3::Cross(frustumVertices[2] - frustumVertices[3], frustumVertices[7] - frustumVertices[3]);
		temp.Normalize(); Vector3ToVector4(&frustum.normal[4], temp);
		frustum.normal[5] = -frustum.normal[0];
		buffer.frustum = frustum;
	}
	bool View::IsFrustumRange(const Math::Vector3& pos, float rad) {
		auto Vector4ToVector3 = [=](Math::Vector3* out, const Math::Vector4& in) {
			memcpy_s(out, sizeof(Math::Vector3), &in, sizeof(Math::Vector3));
		};
		FOR(i, 6) {
			Math::Vector3 center, normal;
			Vector4ToVector3(&center, frustum.center[i]);
			Vector4ToVector3(&normal, frustum.normal[i]);
			//�I�u�W�F�N�g�ւ̃x�N�g��
			Math::Vector3 dir = const_cast<Math::Vector3&>(pos) - center;
			//�J�����O�\���H
			float l_dot = Math::Vector3::Dot(dir, normal);
			if (l_dot < -rad)return false;
		}
		return true;
	}
	DirectX::XMMATRIX View::GetColumnViewMatrix() { return buffer.view; }
	DirectX::XMMATRIX View::GetColumnProjectionMatrix() { return buffer.projection; }
	DirectX::XMMATRIX View::GetRowViewMatrix() { return DirectX::XMMatrixTranspose(buffer.view); }
	DirectX::XMMATRIX View::GetRowProjectionMatrix() { return DirectX::XMMatrixTranspose(buffer.projection); }
	DirectX::XMMATRIX View::CreateViewportMatrix(const Math::Vector2& size) {
		DirectX::XMFLOAT4X4 storageViewport = {};
		memset(&storageViewport, 0, sizeof(storageViewport));
		storageViewport._11 = size.x / 2;
		storageViewport._22 = -size.y / 2;
		storageViewport._33 = 1.0f;
		storageViewport._41 = size.x / 2;
		storageViewport._42 = size.y / 2;
		storageViewport._44 = 1.0f;
		DirectX::XMMATRIX calcViewport;
		return DirectX::XMLoadFloat4x4(&storageViewport);
	}

	DirectX::XMMATRIX View::GetNowColumnViewMatrix() { return nowView; }
	DirectX::XMMATRIX View::GetNowColumnProjectionMatrix() { return nowProjection; }
	DirectX::XMMATRIX View::GetNowRowViewMatrix() { return DirectX::XMMatrixTranspose(nowView); }
	DirectX::XMMATRIX View::GetNowRowProjectionMatrix() { return DirectX::XMMatrixTranspose(nowProjection); }

}