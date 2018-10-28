#include "Lobelia.hpp"
#include "SceneDeferred.hpp"

//Screen Space Motion Blur����Ă݂���

//�V�F�[�_�[�̃R���p�C������������A�f�o�b�O���ȊO��cso�ɓ������Ă�肽����

//TODO : SSAOPS�̉ω𑜓x�Ή�(�r���[������Ă�邾���H)
//TODO : �V�F�[�_�[�̐���
//TODO : �x������

//�e�X�g�V�[��
//�ŏI�I�ɂ����ł̌o���ƌ��ʂ�p���ă����_�����O�G���W�������\��ł͂��邪�A�A���I�������ɂȂ�Ǝv����B
//���̍ۂɂ́AGBuffer�̍ė��p���l�����ق����ǂ������B
//���͉����l�����ɓ����̂��Ƃ�GBuffer���g�p���Ă���B

//�����������Ă���̈ꗗ(���̃V�[���̓f�B�t�@�[�h�V�F�[�f�B���O�ł�)
//Define.h�̃X�C�b�`�ňꕔ�̋@�\�̃X�C�b�`���\
//�n�[�t�����o�[�g
//���`�t�H�O
//�@���}�b�v
//�����|�C���g���C�g(�œK������)
//SSAOPS �x��
//SSAOCS PS��葁��
//�K�E�X�t�B���^PS
//�K�E�X�t�B���^CS ����x�� �œK���s��
//�V���h�E�}�b�v
//�o���A���X�V���h�E�}�b�v
//�J�X�P�[�h�V���h�E�}�b�v
//�J�X�P�[�h�o���A���X�V���h�E�}�b�v(�J�X�P�[�h�V���h�E�}�b�v�ƁA�o���A���X�V���h�E�}�b�v�̍��킹�Z)
//GPURaycast(�����蔻��p ������s���͔̂����̂͂������A�ǂ������{�g���l�b�N�ɂȂ范�x)
//Gaussian Depth of Field

namespace Lobelia::Game {
	namespace {
		const constexpr int LIGHT_COUNT = 127;
	}
	//---------------------------------------------------------------------------------------------
	//
	//		Scene
	//
	//---------------------------------------------------------------------------------------------
	void SceneDeferred::Initialize() {
		Math::Vector2 scale = Application::GetInstance()->GetWindow()->GetSize();
		camera = std::make_unique<ViewerCamera>(scale, Math::Vector3(57.0f, 66.0f, 106.0f), Math::Vector3(0.0f, 0.0f, 0.0f));
		rt = std::make_unique<Graphics::RenderTarget>(scale, DXGI_SAMPLE_DESC{ 1,0 });
		deferredBuffer = std::make_unique<DeferredBuffer>(scale);
		normalMap = TRUE; useLight = TRUE; useFog = TRUE;
#ifdef _DEBUG
		HostConsole::GetInstance()->IntRegister("deferred", "normal map", &normalMap, false);
		HostConsole::GetInstance()->IntRegister("deferred", "use light", &useLight, false);
		HostConsole::GetInstance()->IntRegister("deferred", "use fog", &useFog, false);
#endif
		//model = std::make_shared<Graphics::Model>("Data/Model/Deferred/stage.dxd", "Data/Model/Deferred/stage.mt");
		stage = std::make_shared<Graphics::Model>("Data/Model/maps/stage.dxd", "Data/Model/maps/stage.mt");
		stage->Translation(Math::Vector3(0.0f, 1.0f, 0.0f));
		stage->CalcWorldMatrix();
#ifdef SIMPLE_SHADER
		deferredShader = std::make_unique<SimpleDeferred>();
#endif
#ifdef FULL_EFFECT
		//�����ݒu
		deferredShader = std::make_unique<FullEffectDeferred>();
		FullEffectDeferred::PointLight light;
		for (int i = 0; i < LIGHT_COUNT; i++) {
			light.pos = Math::Vector4(Utility::Frand(-60.0f, 60.0f), Utility::Frand(-10.0f, 10.0f), Utility::Frand(-40.0f, 40.0f), 0.0f);
			light.pos += Math::Vector4(-213.0f, 5.0f, -5.0f, 0.0f);
			light.color = Utility::Color(rand() % 255, rand() % 255, rand() % 255, 255);
			light.attenuation = Utility::Frand(0.5f, 10.0f);
			deferredShader->SetLightBuffer(i + 1, light);
		}
#endif
#ifdef USE_SSAO
#ifdef SSAO_PS
		ssao = std::make_unique<SSAOPS>(scale);
#endif
#ifdef SSAO_CS
		ssao = std::make_unique<SSAOCS>(scale*(QUALITY > 1.0f ? 1.0f : QUALITY));
#endif
#endif
#ifdef CASCADE
		shadow = std::make_unique<ShadowBuffer>(scale*QUALITY, 4, true);
#else
		shadow = std::make_unique<ShadowBuffer>(scale*QUALITY, 1, true);
#endif
#ifdef USE_DOF
		dof = std::make_unique<DepthOfField>(scale, QUALITY);
		dof->SetFocus(150.0f);
#endif
		skybox = std::make_unique<SkyBox>("Data/Model/skybox.dxd", "Data/Model/skybox.mt");
#ifdef USE_CHARACTER
		Raycaster::Initialize();
		character = std::make_shared<Character>();
#ifdef GPU_RAYCASTER
		//���C�֌W�̏�����
		rayMesh = std::make_shared<RayMesh>(stage.get());
		character->SetTerrainData(rayMesh);
#endif
		character->SetTerrainData(stage);
#endif
		//shadow = std::make_unique<ShadowBuffer>(Math::Vector2(1280, 720), 1, false);
		//gaussian = std::make_unique<GaussianFilterCS>(scale);
		rad = 0.0f;
		shadow->SetNearPlane(10.0f);
		shadow->SetFarPlane(500.0f);
		shadow->SetLamda(1.0f);

	}
	SceneDeferred::~SceneDeferred() {
#ifdef _DEBUG
		HostConsole::GetInstance()->VariableUnRegister("deferred");
#endif
	}
	void SceneDeferred::AlwaysUpdate() {
		if (Input::GetKeyboardKey(DIK_7) == 1) normalMap = !normalMap;
		if (Input::GetKeyboardKey(DIK_6) == 1) useFog = !useFog;
		if (Input::GetKeyboardKey(DIK_5) == 1) useLight = !useLight;

		deferredBuffer->AddModel(stage, normalMap);
#ifdef USE_CHARACTER
		deferredBuffer->AddModel(character, false);
#endif
#ifdef FULL_EFFECT
		//�����̃J�����ʒu�ɂ�������u��
		FullEffectDeferred::PointLight light;
		//light.pos = Math::Vector4(pos.x, pos.y, pos.z, 0.0f);
		light.pos = Math::Vector4(0.0f, 0.0f, 0.0f, 0.0f);
		light.color = 0xFFFFFFFF;
		//light.color = Utility::Color(rand() % 255, rand() % 255, rand() % 255, 255);
		light.attenuation = 20.0f;
		deferredShader->SetLightBuffer(0, light);
		if (useLight)deferredShader->SetUseCount(LIGHT_COUNT + 1);
		else deferredShader->SetUseCount(0);
		deferredShader->Update();
#endif
		shadow->AddModel(stage);
#ifdef USE_CHARACTER
		shadow->AddModel(character);
#endif
		//��]���C�g
		//rad += Application::GetInstance()->GetProcessTimeSec()*0.1f;
		//lpos = Math::Vector3(sinf(rad), 0.0f, cos(rad))*300.0f;
		//lpos.y = 150.0f;
		//�Œ胉�C�g
		lpos = Math::Vector3(200.0f, 130.0f, 200.0f);
		shadow->SetPos(lpos);
		//shadow->SetPos(pos);
		camera->Update();
#ifdef USE_CHARACTER
		character->Update(Math::Vector3(0.0f, 0.0f, -1.0f));
#endif
	}
	void SceneDeferred::AlwaysRender() {
		Graphics::Environment::GetInstance()->SetLightDirection(-Math::Vector3(1.0f, 1.0f, 1.0f));
		Graphics::Environment::GetInstance()->SetActiveLinearFog(useFog);
		Graphics::Environment::GetInstance()->SetFogBegin(150.0f);
		Graphics::Environment::GetInstance()->SetFogEnd(400.0f);
		Graphics::Environment::GetInstance()->Activate();
		camera->Activate();
		rt->Clear(0x00000000);
		Graphics::RenderTarget* backBuffer = rt.get();
		//shadow->CreateShadowMap(view.get(), backBuffer);
		shadow->CreateShadowMap(camera->GetView().get(), backBuffer);
		skybox->Render(camera.get());
		//view->Activate();
		shadow->Begin();
		deferredBuffer->RenderGBuffer();
		shadow->End();
		backBuffer->Activate();
#ifdef USE_SSAO
#ifdef SSAO_PS
		ssao->CreateAO(backBuffer, deferredBuffer.get());
#endif
#ifdef SSAO_CS
		ssao->CreateAO(backBuffer, camera->GetView().get(), deferredBuffer.get());
#endif
		ssao->Begin(5);
#endif
		deferredBuffer->Begin();
		deferredShader->Render();
		deferredBuffer->End();
#ifdef USE_SSAO
		ssao->End();
#endif
		//�o�b�N�o�b�t�@���X���b�v�`�F�C���̂��̂ɕύX
		backBuffer = Application::GetInstance()->GetSwapChain()->GetRenderTarget();
		backBuffer->Activate();
#ifdef USE_DOF
		dof->Dispatch(camera->GetView().get(), backBuffer, rt.get(), deferredBuffer->GetRenderTarget(DeferredBuffer::BUFFER_TYPE::VIEW_POS).get());
		dof->Render();
#else
		Graphics::SpriteRenderer::Render(rt.get());
		Graphics::Texture::Clean(0, Graphics::ShaderStageList::PS);
#endif
#ifdef _DEBUG
		if (Application::GetInstance()->debugRender) {
			deferredBuffer->DebugRender();
			shadow->DebugRender();
#ifdef USE_SSAO
			ssao->Render();
#endif
		}
#endif
	}
}