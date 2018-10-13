#include "Lobelia.hpp"
#include "ParaboloidEnvironmentMap.hpp"
namespace Lobelia::Game {
	//���}�b�v�ƃt���l���͂��̂����B
	//�f���A���p���{�C�h��p�����t���l����������GI�Ƃ�AO�Ƃ���肽���B
	DualParaboloidMap::DualParaboloidMap(const Math::Vector2& size, const Math::Vector3& pos, float radius) :pos(pos), radius(radius), size(size) {
		for (int i = 0; i < 2; i++) {
			views[i] = std::make_unique<Graphics::View>(Math::Vector2(), size, PI / 2.0f, 1.0f, radius);
		}
		rt = std::make_unique<Graphics::RenderTarget>(size, DXGI_SAMPLE_DESC{ 1, 0 }, DXGI_FORMAT_R32G32B32A32_FLOAT, 2);
		info.zNear = 1.0f;
		info.zFar = radius;
	}
	Math::Vector3 DualParaboloidMap::GetPos() { return pos; }
	float DualParaboloidMap::GetRadius() { return radius; }
	Math::Vector2 DualParaboloidMap::GetTextureSize() { return size; }
	void DualParaboloidMap::SetPos(const Math::Vector3& pos) { this->pos = pos; }
	void DualParaboloidMap::SetRadius(float radius) { this->radius = radius; }
	void DualParaboloidMap::UpdateInfo() {
		//���̍X�V
		for (int i = 0; i < 2; i++) {
			views[i]->SetEyePos(pos);
			views[i]->SetEyeUpDirection(Math::Vector3(0.0f, 1.0f, 0.0f));
			//�O��̐ݒ�A�����������甽�΂����H
			if (i == 0)views[i]->SetEyeTarget(pos + Math::Vector3(0.0f, 0.0f, -1.0f));
			else views[i]->SetEyeTarget(pos + Math::Vector3(0.0f, 0.0f, 1.0f));
			views[i]->Update();
			DirectX::XMMATRIX mat = views[i]->GetColumnViewMatrix();
			DirectX::XMStoreFloat4x4(&info.views[i], mat);
		}
		DirectX::XMMATRIX mat = views[0]->GetColumnProjectionMatrix();
		DirectX::XMStoreFloat4x4(&info.projection, mat);
	}
	void DualParaboloidMap::Clear(Utility::Color color) { rt->Clear(color); }
	std::shared_ptr<Graphics::RenderTarget> DualParaboloidMap::GetRenderTarget() { return rt; }
	ParaboloidInfo& DualParaboloidMap::GetParaboloidInfo() { return info; }
	void DualParaboloidMap::Activate() {
		views[0]->Activate();
		Graphics::Texture::Clean(4, Graphics::ShaderStageList::PS);
		rt->Clear(0x00000000);
		rt->Activate();
	}

	DualParaboloidMapManager::DualParaboloidMapManager() {
		//�V�F�[�_�[�̃��[�h
		vs = std::make_shared<Graphics::VertexShader>("Data/ShaderFile/3D/ParaboloidEnvironmentMap.hlsl", "VS_CREATE_PARABOLOID", Graphics::VertexShader::Model::VS_5_0);
		hs = std::make_shared<Graphics::HullShader>("Data/ShaderFile/3D/ParaboloidEnvironmentMap.hlsl", "HS_CREATE_PARABOLOID");
		ds = std::make_shared<Graphics::DomainShader>("Data/ShaderFile/3D/ParaboloidEnvironmentMap.hlsl", "DS_CREATE_PARABOLOID");
		gs = std::make_shared<Graphics::GeometryShader>("Data/ShaderFile/3D/ParaboloidEnvironmentMap.hlsl", "GS_CREATE_PARABOLOID");
		ps = std::make_shared<Graphics::PixelShader>("Data/ShaderFile/3D/ParaboloidEnvironmentMap.hlsl", "PS_CREATE_PARABOLOID", Graphics::PixelShader::Model::PS_5_0);
		//�R���X�^���g�o�b�t�@�̍쐬
		constantBuffer = std::make_unique<Graphics::ConstantBuffer<ParaboloidInfo>>(7, Graphics::ShaderStageList::GS);
#ifdef _DEBUG
		debugBuffer = std::make_unique<Graphics::ConstantBuffer<DebugInfo>>(8, Graphics::ShaderStageList::PS);
		debugVS = std::make_shared<Graphics::VertexShader>("Data/ShaderFile/3D/ParaboloidEnvironmentMap.hlsl", "VS_DEBUG_SPRITE", Graphics::VertexShader::Model::VS_5_0);
		debugPS = std::make_shared<Graphics::PixelShader>("Data/ShaderFile/3D/ParaboloidEnvironmentMap.hlsl", "PS_DEBUG_SPRITE", Graphics::PixelShader::Model::PS_5_0);
#endif
	}
	DualParaboloidMapManager::~DualParaboloidMapManager() {

	}
	//���t���[���Z�b�g���K�v
	void DualParaboloidMapManager::AddModelList(std::weak_ptr<Graphics::Model> model, bool lighting) { models.push_back(std::make_pair(model, lighting)); }
	//�֐��������K�v
	void DualParaboloidMapManager::RenderEnvironmentMap() {
		if (environmentMaps.empty())return;
		std::shared_ptr<Graphics::VertexShader> defaultVS = Graphics::Model::GetVertexShader();
		std::shared_ptr<Graphics::PixelShader> defaultPS = Graphics::Model::GetPixelShader();
		Graphics::Model::ChangeVertexShader(vs);
		Graphics::Model::ChangePixelShader(ps);
		gs->Set(); hs->Set(); ds->Set();
		for (auto&& map = environmentMaps.begin(); map != environmentMaps.end();) {
			//�����؂�
			if (map->expired()) {
				map = environmentMaps.erase(map);
				continue;
			}
			std::shared_ptr<DualParaboloidMap> dualParaboloid = map->lock();
			bool renderFlag = false;
			for (auto&& model = models.begin(); model != models.end();) {
				if (model->first.expired()) {
					model = models.erase(model);
					continue;
				}
				std::shared_ptr<Graphics::Model> modelInstance = model->first.lock();
				//�K���͈͂ɂ��邩�ۂ��A���ꂽ�I�u�W�F�N�g�͔��˂Ɋ܂߂Ȃ�
				if ((dualParaboloid->GetPos() - modelInstance->GetTransform().position).Length() > dualParaboloid->GetRadius()) {
					model++;
					continue;
				}
				//���̍X�V �`�悳���Ώۂ����Ȃ����̍X�V��h������
				if (!renderFlag) {
					dualParaboloid->UpdateInfo();
					dualParaboloid->Activate();
					renderFlag = true;
				}
				auto& info = dualParaboloid->GetParaboloidInfo();
				info.lighting = static_cast<int>(model->second);
				constantBuffer->Activate(info);
				constantBuffer->Activate(dualParaboloid->GetParaboloidInfo());
				//���f���̕`��
				modelInstance->Render(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
				model++;
			}
			map++;
		}
		//��Еt��
		Graphics::Model::ChangeVertexShader(defaultVS);
		Graphics::Model::ChangePixelShader(defaultPS);
		Graphics::HullShader::Clean();
		Graphics::DomainShader::Clean();
		Graphics::GeometryShader::Clean();
		//�f�t�H���g�̃����_�[�^�[�Q�b�g�֖߂�
		Application::GetInstance()->GetSwapChain()->GetRenderTarget()->Activate();
		//���f�����X�g���t���b�V��
		ClearModelList();
	}
	std::shared_ptr<DualParaboloidMap> DualParaboloidMapManager::CreateEnvironmentMap(const Math::Vector2& size, const Math::Vector3& pos, float radius) {
		std::shared_ptr<DualParaboloidMap> map = std::make_shared<DualParaboloidMap>(size, pos, radius);
		environmentMaps.push_back(map);
		return map;
	}
	void DualParaboloidMapManager::Activate(const Math::Vector3& pos) {
		struct Info {
			float distance = 1000.0f;
			std::shared_ptr<DualParaboloidMap> dualParaboloid;
		};
		Info info;
		for (auto&& map = environmentMaps.begin(); map != environmentMaps.end();) {
			if (map->expired()) {
				map = environmentMaps.erase(map);
				continue;
			}
			std::shared_ptr<DualParaboloidMap> dualParaboloid = map->lock();
			float distance = (dualParaboloid->GetPos() - pos).Length();
			if (info.distance > distance) {
				info.distance = distance;
				info.dualParaboloid = dualParaboloid;
			}
			map++;
		}
		if (info.dualParaboloid) info.dualParaboloid->GetRenderTarget()->GetTexture()->Set(4, Graphics::ShaderStageList::PS);
	}
	void DualParaboloidMapManager::ClearModelList() { models.clear(); }
#ifdef _DEBUG
	void DualParaboloidMapManager::DebugRender() {
		if (!Application::GetInstance()->debugRender)return;
		debugVS->Set();
		debugPS->Set();
		int count = 0;
		//�f�o�b�O�p�Ȃ�œK��
		for (auto&& map : environmentMaps) {
			if (map.expired())continue;
			std::shared_ptr<DualParaboloidMap> dualParaboloid = map.lock();
			dualParaboloid->GetRenderTarget()->GetTexture()->Set(4, Graphics::ShaderStageList::PS);
			for (int i = 0; i < 2; i++) {
				debugInfo.index = i;
				debugBuffer->Activate(debugInfo);
				Graphics::SpriteRenderer::CustumeRender(Math::Vector2(count*200.0f, (count / 12)*200.0f), Math::Vector2(200.0f, 200.0f), 0.0f, Math::Vector2(), dualParaboloid->GetTextureSize(), dualParaboloid->GetTextureSize());
				count++;
			}
		}
	}
#endif
}