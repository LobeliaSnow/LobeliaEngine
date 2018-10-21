#include "Lobelia.hpp"
#include "../Data/ShaderFile/Define.h"
#include "Common/DeferredBuffer.hpp"
#include "Common/ShadowBuffer.hpp"
#include "Common/ComputeBuffer.hpp"
#include "Common/PostEffect.hpp"

namespace Lobelia::Game {
	ShadowBuffer::ShadowBuffer(const Math::Vector2& size, int split_count, bool use_variance) :size(size), count(split_count) {
		rts.resize(split_count); views.resize(split_count);
		//分割する際のnear/farを持つ、+1の理由は、最初のnearから始まって次のfarだけを記録していくが
		//最初のfarだけはnearとして共有されないので、その分の+1
		cascadeValues.resize(split_count + 1);
		splitPositions.resize(split_count);
		gaussian.resize(split_count);
		DXGI_FORMAT format = DXGI_FORMAT_R16G16_FLOAT;
		Math::Vector2 ssize = size;
		for (int i = 0; i < split_count; i++) {
			rts[i] = std::make_shared<Graphics::RenderTarget>(size, DXGI_SAMPLE_DESC{ 1,0 }, format);
			views[i] = std::make_unique<Graphics::View>(Math::Vector2(), size, PI / 4.0f, 50, 400.0f);
#ifdef GAUSSIAN_CS
			gaussian[i] = std::make_unique<GaussianFilterCS>(size, format);
#endif
#ifdef GAUSSIAN_PS
			gaussian[i] = std::make_unique<GaussianFilterPS>(size, format);
#endif
			gaussian[i]->SetDispersion(0.01f);
		}
		//sampler = std::make_unique<Graphics::SamplerState>(Graphics::SAMPLER_PRESET::COMPARISON_LINEAR, 16);
		vs = std::make_shared<Graphics::VertexShader>("Data/ShaderFile/3D/deferred.hlsl", "CreateShadowMapVS", Graphics::VertexShader::Model::VS_5_0, false);
		ps = std::make_shared<Graphics::PixelShader>("Data/ShaderFile/3D/deferred.hlsl", "CreateShadowMapPS", Graphics::PixelShader::Model::PS_5_0, false);
		cbuffer = std::make_unique<Graphics::ConstantBuffer<Info>>(10, Graphics::ShaderStageList::VS | Graphics::ShaderStageList::PS);
		info.useShadowMap = TRUE; info.useVariance = i_cast(use_variance);
		//縮小バッファにして処理稼ぐのもいいかも
#ifdef _DEBUG
		HostConsole::GetInstance()->IntRegister("deferred", "use shadow", &info.useShadowMap, false);
		HostConsole::GetInstance()->IntRegister("deferred", "use variance", &info.useVariance, false);
#endif
		nearZ = 1.0f;
		farZ = 1000.0f;
		lamda = 0.5f;
	}
	void ShadowBuffer::SetNearPlane(float near_z) { nearZ = near_z; }
	void ShadowBuffer::SetFarPlane(float far_z) { farZ = far_z; }
	void ShadowBuffer::SetLamda(float lamda) { this->lamda = lamda; }
	void ShadowBuffer::SetPos(const Math::Vector3& pos) { this->pos = pos; }
	void ShadowBuffer::SetTarget(const Math::Vector3& at) { this->at = at; }
	//ちょっと大丈夫かは不明、ダメそうなら調整します
	void ShadowBuffer::ComputeSplit(float lamda, float near_z, float far_z) {
#ifdef CASCADE
		//通常のシャドウマップ
		if (count == 1) {
			cascadeValues[0] = near_z;
			cascadeValues[1] = far_z;
			return;
		}
		//カスケード
		float invM = 1.0f / f_cast(count);
		float farDivisionNear = far_z / near_z;
		float farSubNear = far_z - near_z;
		//実用分割スキームを適用
		// ※ GPU Gems 3, Chapter 10. Parallel-Split Shadow Maps on Programmable GPUs.
		//    http://http.developer.nvidia.com/GPUGems3/gpugems3_ch10.html を参照.
		for (int i = 1; i < count + 1; i++) {
			//対数分割スキーム
			float log = near_z * powf(farDivisionNear, invM*i);
			//一様分割スキーム
			float uni = near_z + farSubNear * i*invM;
			//上記の2つの演算結果を線形補間する
			cascadeValues[i] = lamda * log + uni * (1.0f - lamda);
		}
		cascadeValues[0] = near_z;
		cascadeValues[count] = far_z;
		for (int i = 1; i < count + 1; i++) {
			info.splitPos[i - 1] = cascadeValues[i];
		}
#endif
	}
	void ShadowBuffer::CameraUpdate() {
		Math::Vector3 front = at - pos; front.Normalize();
		up = Math::Vector3(0.01f, 1.0f, 0.01f); up.Normalize();
		Math::Vector3 right = Math::Vector3::Cross(up, front); right.Normalize();
		up = Math::Vector3::Cross(front, right);
#ifdef CASCADE
		info.pos.x = pos.x; info.pos.y = pos.y; info.pos.z = pos.z; info.pos.w = 1.0f;
		info.front.x = front.x; info.front.y = front.y; info.front.z = front.z; info.front.w = 0.0f;
		ComputeSplit(lamda, nearZ, farZ);
#endif
		for (int i = 0; i < count; i++) {
			views[i]->SetEyePos(pos);
			views[i]->SetEyeTarget(at);
			views[i]->SetEyeUpDirection(up);
#ifdef CASCADE
			views[i]->SetNear(cascadeValues[i]);
			views[i]->SetFar(cascadeValues[count]);
#endif
		}
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
		CameraUpdate();
		auto& defaultVS = Graphics::Model::GetVertexShader();
		auto& defaultPS = Graphics::Model::GetPixelShader();
		Graphics::Model::ChangeVertexShader(vs);
		Graphics::Model::ChangeVertexShader(vs);
		Graphics::Model::ChangePixelShader(ps);
		for (int i = 0; i < count; i++) {
			views[i]->Activate();
			rts[i]->Clear(0x00000000);
			rts[i]->Activate();
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
		//ガウスによるぼかし バリアンス用
		if (info.useVariance) {
			for (int i = 0; i < count; i++) {
				gaussian[i]->Dispatch(active_view, active_rt, rts[i]->GetTexture());
			}
		}
	}
	void ShadowBuffer::Begin() {
		//情報の更新
		DirectX::XMStoreFloat4x4(&info.view, views[0]->GetColumnViewMatrix());
		for (int i = 0; i < count; i++) {
			DirectX::XMStoreFloat4x4(&info.proj[i], views[i]->GetColumnProjectionMatrix());
			if (info.useVariance)gaussian[i]->Begin(6 + i);
			else rts[i]->GetTexture()->Set(6 + i, Graphics::ShaderStageList::PS);
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
				Graphics::Texture::Clean(6 + i, Graphics::ShaderStageList::PS);
			}
		}
	}
	void ShadowBuffer::DebugRender() {
		for (int i = 0; i < count; i++) {
			Graphics::SpriteRenderer::Render(rts[i].get(), Math::Vector2(i*200.0f, 200.0f), Math::Vector2(200.0f, 200.0f), 0.0f, Math::Vector2(), size, 0xFFFFFFFF);
			gaussian[i]->DebugRender(Math::Vector2(i*200.0f, 400.0f), Math::Vector2(200.0f, 200.0f));
		}
	}

}