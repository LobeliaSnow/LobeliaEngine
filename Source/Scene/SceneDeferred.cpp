#include "Lobelia.hpp"
#include "SceneDeferred.hpp"

//�V�F�[�_�[�̃R���p�C������������A�f�o�b�O���ȊO��cso�ɓ������Ă�肽��
//���̍ہA���݂�1�̃V�F�[�_�[�t�@�C���ɕ����̃G���g���|�C���g������̂ł܂Ƃ߂ăR���p�C��&�o�͂�����c�[�����쐬������

//TODO : �e�A�V�F�[�_�[�d���A�t�H�O�̔Z��
//TODO : MipFog ���ׂ�
//TODO : SSAOPS�̉ω𑜓x�Ή�(�r���[������Ă�邾���H)
//TODO : �V�F�[�_�[�̐���
//TODO : �x������
//TODO : define�ƃt���O���r��Ă�̂ŁA���t�@�N�^�����O(const����镔����t���Ă������)
//TODO : GaussianFileter��CS�ōœK��(1pass����)(2pass�̂ق��������݂���?)
//���Q�lURL
//https://github.com/Unity-Technologies/PostProcessing/blob/v2/PostProcessing/Shaders/Builtins/GaussianDownsample.compute
//http://sygh.hatenadiary.jp/entry/2014/07/05/194143
//TODO : Temporal Reprojection�Ń��[�V�����u���[�̍������C��
//TODO : �}�e���A��ID�o�b�t�@�̃V���h�E�o�b�t�@�Ƃ̌���B�v�f�ɂ����
//TODO : sRGB�t�H�[�}�b�g�̃e�N�X�`���ɐ؂�ւ�
//TODO : SSS�̎���
//TODO : �x���c�[���̍쐬
//TODO : ���_�o�b�t�@�𕡐��Z�b�g����`���ŁA�X���b�g1�ɂ͒��_�A�X���b�g2�ɂ�UV�݂����Ȋ����ł�
//TODO : �`�敔���ƃf�[�^������؂藣��

//Hexa-Drive�A�h�o�C�X
//���낻��G���W�����������n�߂�΁H
//�t�H�O����Ȃ��Ȃ��H(�܂��́A��������)
//�e
//Screen Space Reflection
//�}�e���A���ɂ��ẮA�v�Z���f������p�ӂ��āA�p�����[�^�[��G-Buffer�ɏ������ނ��ƂŐ��䂷��̂��ǂ�
//�܂�APhong�ւ̌W����0�Ȃ炻��̓����o�[�g�ɂȂ�B�悭����̂̓��^���l�X��AEmission Intensity
//Lambert 1/PI�{�ł���Ƃ悢(������Ɍ����U������V�~�����[�V����)
//�p�[�e�B�N�����A�G�~�b�^�[��p�ӂ���GPU��ł��̃G�~�b�^�[���甭��������΃������A�N�Z�X�������č��������ł�����
//GUI�̃`�F�b�N�{�b�N�X�ŃI���I�t�ł���悤�ɂ����ق����ǂ��B���܂�f�o�b�O�L�[�͂悭�Ȃ��B

//�e�X�g�V�[��
//�ŏI�I�ɂ����ł̌o���ƌ��ʂ�p���ă����_�����O�G���W�������\��ł͂��邪�A�A���I�������ɂȂ�Ǝv����B
//���̍ۂɂ́AGBuffer�̍ė��p���l�����ق����ǂ������B
//�|�X�g�G�t�F�N�g�����̍ۂɌ��ݎg�p���Ă��郌���_�[�^�[�Q�b�g���A�O������󂯎��`�ɂ���΂��������}�V�ɂ͂Ȃ�

//Define.h�̃X�C�b�`�ňꕔ�̋@�\�̃X�C�b�`���\
//�����������Ă���̈ꗗ(���̃V�[���̓f�B�t�@�[�h�V�F�[�f�B���O�ł�)
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
//GPURaycast(�����蔻��p ������s���͔̂����̂͂������A�p�C�v���C���X�g�[���̉�����ł��Ȃ��B����ɂ̓t���[���x�������邵���Ȃ��̂�CPU���Ŋ撣��)
//Gaussian Depth of Field (��ʊE�[�x)
//Screen Space Motion Blur
//�쐣���u���[��
//�{���F����
//�w���g�[���}�b�v
//�K���}�␳
//���ӌ���
//�W���}�e���A���ǉ�


namespace Lobelia::Game {
	namespace {
		const constexpr int LIGHT_COUNT = 256;
	}
	//---------------------------------------------------------------------------------------------
	//
	//		Scene
	//
	//---------------------------------------------------------------------------------------------
	void SceneDeferred::Initialize() {
		Math::Vector2 scale = Application::GetInstance()->GetWindow()->GetSize();
		camera = std::make_shared<ViewerCamera>(scale, Math::Vector3(57.0f, 66.0f, 106.0f), Math::Vector3(0.0f, 0.0f, 0.0f));
		rt = std::make_unique<Graphics::RenderTarget>(scale, DXGI_SAMPLE_DESC{ 1,0 });
		deferredBuffer = std::make_unique<DeferredBuffer>(scale);
		//�f���p
		useLight = TRUE; useFog = TRUE; useMotionBlur = TRUE; useCamera = true; useDof = true;
		useShadow = true; useVariance = true; renderGBuffer = true; renderShadowMap = true;
		renderSSAOBuffer = true; useSSAO = true; ssaoDepthThreshold = 5.0f; focusRange = 150.0f;
		renderBlumeBuffer = true; chromaticAberrationIntensity = 0.005f; useVignette = true;
		blumeIntensity = 6.0f;	radius2 = 5.0f; smooth = 2.1f; mechanicalScale = 0.25f;
		cosFactor = 0.53f; cosPower = 0.45f; naturalScale = 0.09f;
#ifdef _DEBUG
		HostConsole::GetInstance()->IntRegister("deferred", "use light", &useLight, false);
		HostConsole::GetInstance()->IntRegister("deferred", "use fog", &useFog, false);
		HostConsole::GetInstance()->IntRegister("deferred", "use motion blur", &useMotionBlur, false);
#endif
		//stage = std::make_shared<Graphics::Model>("Data/Model/Deferred/stage.dxd", "Data/Model/Deferred/stage.mt");
		stage = std::make_shared<Graphics::Model>("Data/Model/maps/stage.dxd", "Data/Model/maps/stage.mt");
		stage->Translation(Math::Vector3(0.0f, 1.0f, 0.0f));
		//stage->Scalling(3.0f);
		stage->CalcWorldMatrix();
		box = std::make_shared<Graphics::Model>("Data/Model/box.dxd", "Data/Model/box.mt");
		box->Translation(Math::Vector3(0.0f, 5.0f, 0.0f));
		box->Scalling(3.0f);
		//box->Scalling(9.0f);
		box->CalcWorldMatrix();
		stageCollision = std::make_shared<Graphics::Model>("Data/Model/maps/collision.dxd", "Data/Model/maps/collision.mt");
		stageCollision->Translation(Math::Vector3(0.0f, 1.0f, 0.0f));
		stageCollision->CalcWorldMatrix();
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
			deferredShader->SetLightBuffer(i, light);
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
		shadow = std::make_unique<ShadowBuffer>(Math::Vector2(1024, 1024)*QUALITY, 4, true);
#else
		shadow = std::make_unique<ShadowBuffer>(scale*QUALITY, 1, true);
#endif
#ifdef USE_DOF
		dof = std::make_unique<DepthOfField>(scale, DOF_QUALITY);
		dof->SetFocusRange(150.0f);
#endif
		skybox = std::make_shared<SkyBox>("Data/Model/skybox.dxd", "Data/Model/skybox.mt");
		skybox->SetCamera(camera);
		deferredBuffer->SetSkybox(skybox);
#ifdef USE_CHARACTER
		Raycaster::Initialize();
		character = std::make_shared<Character>();
#ifdef GPU_RAYCASTER
		//���C�֌W�̏�����
		rayMesh = std::make_shared<RayMesh>(stageCollision.get());
		character->SetTerrainData(rayMesh);
#endif
		character->SetTerrainData(stageCollision);
#endif
		//shadow = std::make_unique<ShadowBuffer>(Math::Vector2(1280, 720), 1, false);
		//gaussian = std::make_unique<GaussianFilterCS>(scale);
		rad = 0.0f;
		shadow->SetNearPlane(1.0f);
		shadow->SetFarPlane(1000.0f);
		shadow->SetLamda(0.8f);
#ifdef USE_MOTION_BLUR
		motionBlur = std::make_unique<SSMotionBlur>(scale);
#endif
#ifdef _DEBUG
		//----------------------------------------------------------------------------------------------------------------------------------
		//
		//			���̐�f���p���j���[
		//
		//----------------------------------------------------------------------------------------------------------------------------------
		//ImGui�E�C���h�E���쐬
		console = std::make_unique<AdaptiveConsole>("Operation Console");
		//�\�����e���\�z
		//�J����
		console->AddFunction([this]() {
			ImGui::Checkbox("Move Camera", &this->useCamera);
			if (ImGui::TreeNode("Camera")) {
				if (ImGui::Button("Reset Position Start")) {
					this->camera->SetPos(Math::Vector3(57.0f, 66.0f, 106.0f));
					this->camera->SetTarget(Math::Vector3());
					this->camera->SetUp(Math::Vector3(0.0f, 1.0f, 0.0f));
				}
				if (ImGui::Button("Reset Position HDR")) {
					this->camera->SetPos(Math::Vector3(-172.0f, 68.0f, -0.90f));
					this->camera->SetTarget(Math::Vector3(-185.5f, 0.0f, -3.0f));
					this->camera->SetUp(Math::Vector3(0.0f, 1.0f, 0.0f));
				}
				if (ImGui::Button("Reset Position Light")) {
					this->camera->SetPos(Math::Vector3(-343.0f, 33.0f, -11.0f));
					this->camera->SetTarget(Math::Vector3(-185.5f, 0.0f, -3.0f));
					this->camera->SetUp(Math::Vector3(0.0f, 1.0f, 0.0f));
				}
				ImGui::TreePop();
			}
		});
		//G-Buffer
		console->AddFunction([this]() {
			ImGui::Checkbox("Debug Render", &Application::GetInstance()->debugRender);
			if (Application::GetInstance()->debugRender && ImGui::TreeNode("G-Buffer")) {
				ImGui::Checkbox("Render G-Buffer", &this->renderGBuffer);
				ImGui::Checkbox("Render AO Buffer", &this->renderSSAOBuffer);
				ImGui::Checkbox("Render Shadow Map", &this->renderShadowMap);
				ImGui::Checkbox("Render Blume & HDR Buffer", &this->renderBlumeBuffer);
				ImGui::TreePop();
			}
		});
		//�u���[
		console->AddFunction([this]() {
			bool blur = this->useMotionBlur;
			ImGui::Checkbox("Use Blur", &blur);
			this->useMotionBlur = blur;
		});
		//�e
		console->AddFunction([this]() {
			ImGui::Checkbox("Use Shadow", &this->useShadow);
			if (this->useShadow && ImGui::TreeNode("Shadow")) {
				ImGui::Checkbox("Use Variance", &this->useVariance);
				ImGui::TreePop();
			}
		});
		//SSAO
		console->AddFunction([this] {
			ImGui::Checkbox("Use SSAO", &this->useSSAO);
			if (useSSAO && ImGui::TreeNode("AO")) {
				ImGui::SliderFloat("Deoth Threshold", &ssaoDepthThreshold, 1.0f, 30.0f);
				ImGui::TreePop();
			}
		});
		//��ʊE�[�x
		console->AddFunction([this]() {
			ImGui::Checkbox("Use DoF", &this->useDof);
			if (useDof && ImGui::TreeNode("DoF")) {
				ImGui::SliderFloat("Focus Range", &focusRange, 1.0f, 300.0f);
				ImGui::TreePop();
			}
		});
		//�t�H�O
		console->AddFunction([this]() {
			bool fog = this->useFog;
			ImGui::Checkbox("Use Fog", &fog);
			this->useFog = fog;
			static float fogBegin = 300.0f;
			static float fogEnd = 1000.0f;
			if (this->useFog && ImGui::TreeNode("Fog")) {
				ImGui::SliderFloat("Begin", &fogBegin, 0.0f, 999.0f);
				ImGui::SliderFloat("End", &fogEnd, 1.0f, 1000.0f);
				ImGui::TreePop();
			}
			Graphics::Environment::GetInstance()->SetFogBegin(fogBegin);
			Graphics::Environment::GetInstance()->SetFogEnd(fogEnd);
		});
#ifdef FULL_EFFECT
		//���C�g
		console->AddFunction([this]() {
			bool light = this->useLight;
			ImGui::Checkbox("Use Light", &light);
			this->useLight = light;
			static int lightCount = LIGHT_COUNT;
			if (useLight) {
				if (ImGui::TreeNode("Light")) {
					ImGui::SliderInt("count", &lightCount, 0, LIGHT_COUNT);
					if (ImGui::Button("Relocation")) {
						FullEffectDeferred::PointLight light;
						for (int i = 0; i < LIGHT_COUNT; i++) {
							light.pos = Math::Vector4(Utility::Frand(-60.0f, 60.0f), Utility::Frand(-10.0f, 10.0f), Utility::Frand(-40.0f, 40.0f), 0.0f);
							light.pos += Math::Vector4(-213.0f, 5.0f, -5.0f, 0.0f);
							light.color = Utility::Color(rand() % 255, rand() % 255, rand() % 255, 255);
							light.attenuation = Utility::Frand(0.5f, 10.0f);
							deferredShader->SetLightBuffer(i, light);
						}
					}
					ImGui::TreePop();
				}
				deferredShader->SetUseCount(lightCount);
			}
			else deferredShader->SetUseCount(0);
		});
#endif
		//�����Y
		console->AddFunction([this] {
			if (ImGui::TreeNode("Lens")) {
				ImGui::SliderFloat("Chromatic Aberration", &chromaticAberrationIntensity, 0.0f, 0.1f);
				ImGui::Checkbox("Vignette", &useVignette);
				ImGui::SliderFloat("Vignette Radius", &radius2, 0.0f, 10.0f);
				ImGui::SliderFloat("Vignette Smooth", &smooth, 0.0f, 10.0f);
				ImGui::SliderFloat("Vignette Mechanical Scale", &mechanicalScale, 0.0f, 10.0f);
				ImGui::SliderFloat("Vignette Cos Factor", &cosFactor, 0.0f, 10.0f);
				ImGui::SliderFloat("Vignette Cos Power", &cosPower, 0.0f, 10.0f);
				ImGui::SliderFloat("Vignette Natural Scale", &naturalScale, 0.0f, 10.0f);
				ImGui::TreePop();
			}
		});
		//�u���[��
		console->AddFunction([this] {
			ImGui::SliderFloat("Blume Intensity", &blumeIntensity, 0.0f, 20.0f);
		});

		//----------------------------------------------------------------------------------------------------------------------------------
		//----------------------------------------------------------------------------------------------------------------------------------
#endif
	}
	SceneDeferred::~SceneDeferred() {
#ifdef _DEBUG
		HostConsole::GetInstance()->VariableUnRegister("deferred");
#endif
	}
	void SceneDeferred::AlwaysUpdate() {
		if (Input::GetKeyboardKey(DIK_0) == 1) useShadow = !useShadow;
		if (Input::GetKeyboardKey(DIK_2) == 1) useVignette = !useVignette;
		if (Input::GetKeyboardKey(DIK_3) == 1) useMotionBlur = !useMotionBlur;
		if (Input::GetKeyboardKey(DIK_4) == 1) useDof = !useDof;
		if (Input::GetKeyboardKey(DIK_5) == 1) useLight = !useLight;
		if (Input::GetKeyboardKey(DIK_6) == 1) useFog = !useFog;
		if (Input::GetKeyboardKey(DIK_9) == 1) useVariance = !useVariance;
		shadow->SetEnable(useShadow);
		shadow->SetVariance(useVariance);
		deferredBuffer->AddModel(stage, DeferredBuffer::MATERIAL_TYPE::LAMBERT, 1.0f, 1.0f);
		deferredBuffer->AddModel(box, DeferredBuffer::MATERIAL_TYPE::COLOR, 0.0f, blumeIntensity);
		//deferredBuffer->AddModel(stageCollision, normalMap);
#ifdef USE_CHARACTER
		deferredBuffer->AddModel(character, false);
#endif
#ifdef FULL_EFFECT
		deferredShader->Update();
#endif
		shadow->AddModel(box);
		shadow->AddModel(stage);
#ifdef USE_CHARACTER
		shadow->AddModel(character);
#endif
		//��]���C�g
		rad += Application::GetInstance()->GetProcessTimeSec()*0.1f;
		lpos = Math::Vector3(sinf(rad), 0.0f, cos(rad))*100.0f;
		//lpos.x = lpos.z = 0.0f;
		lpos.y = 100.0f;
		//�Œ胉�C�g
		//lpos = Math::Vector3(200.0f, 130.0f, 200.0f);
		shadow->SetPos(lpos);
		shadow->SetTarget(Math::Vector3());
		//shadow->SetPos(pos);
		if (useCamera)camera->Update();
#ifdef USE_DOF
		dof->SetEnable(useDof);
		dof->SetFocusRange(focusRange);
#endif
#ifdef USE_SSAO
		ssao->SetEnable(useSSAO);
		ssao->SetThresholdDepth(ssaoDepthThreshold);
#endif
		deferredShader->EnableVignette(useVignette);
		deferredShader->SetChromaticAberrationIntensity(chromaticAberrationIntensity);
		deferredShader->SetRadius2(radius2);
		deferredShader->SetSmooth(smooth);
		deferredShader->SetMechanicalScale(mechanicalScale);
		deferredShader->SetCosFactor(cosFactor);
		deferredShader->SetCosPower(cosPower);
		deferredShader->SetNaturalScale(naturalScale);
	}
	void SceneDeferred::AlwaysRender() {
		//Graphics::Environment::GetInstance()->SetLightDirection(-Math::Vector3(1.0f, 1.0f, 1.0f));
		Graphics::Environment::GetInstance()->SetActiveLinearFog(useFog);
		Graphics::Environment::GetInstance()->Activate();
		camera->Activate();
		rt->Clear(0xFF000000);
		Graphics::RenderTarget* backBuffer = rt.get();
		//shadow->CreateShadowMap(view.get(), backBuffer);
		shadow->CreateShadowMap(camera->GetView().get(), backBuffer);
		//view->Activate();
		shadow->Begin();
		deferredBuffer->RenderGBuffer();
		backBuffer->Activate();
#ifdef USE_SSAO
#ifdef SSAO_PS
		ssao->CreateAO(backBuffer, deferredBuffer.get());
#endif
#ifdef SSAO_CS
		ssao->CreateAO(backBuffer, camera->GetView().get(), deferredBuffer.get());
#endif
		ssao->Begin(11);
#endif
#ifdef USE_CHARACTER
		Math::Vector3 front = camera->TakeFront(); front.y = 0.0f;
		character->Update(front);
#endif
#ifdef USE_HDR
		//���̂����X�J�C�{�b�N�X�͂��̒��Ŏ�������悤�ɂ��邩��
		deferredShader->RenderHDR(camera->GetView().get(), backBuffer, deferredBuffer.get());
#else
		deferredShader->Render(deferredBuffer.get());
#endif
		shadow->End();
#ifdef USE_SSAO
		ssao->End();
#endif
#ifdef USE_DOF
		dof->Dispatch(camera->GetView().get(), backBuffer, rt.get(), deferredBuffer->GetRenderTarget(DeferredBuffer::BUFFER_TYPE::VIEW_POS).get());
#else
		//Graphics::SpriteRenderer::Render(rt.get());
		//Graphics::Texture::Clean(0, Graphics::ShaderStageList::PS);
#endif
		//�o�b�N�o�b�t�@���X���b�v�`�F�C���̂��̂ɕύX
		if (!useMotionBlur) {
			backBuffer = Application::GetInstance()->GetSwapChain()->GetRenderTarget();
			backBuffer->Activate();
			//skybox->Render();
#ifndef USE_DOF
			Graphics::SpriteRenderer::Render(rt.get());
			Graphics::Texture::Clean(0, Graphics::ShaderStageList::PS);
#endif
		}
#ifdef USE_DOF
		dof->Render();
#endif
		if (useMotionBlur) {
#ifdef USE_MOTION_BLUR
			motionBlur->Dispatch(backBuffer->GetTexture(), deferredBuffer->GetRenderTarget(DeferredBuffer::BUFFER_TYPE::VIEW_POS)->GetTexture());
#endif
			backBuffer = Application::GetInstance()->GetSwapChain()->GetRenderTarget();
			backBuffer->Activate();
			//skybox->Render();
#ifdef USE_MOTION_BLUR
			motionBlur->Render();
#else
			Graphics::SpriteRenderer::Render(rt.get());
			Graphics::Texture::Clean(0, Graphics::ShaderStageList::PS);
#endif
		}

#ifdef _DEBUG
		console->Update();
		if (Application::GetInstance()->debugRender) {
			if (renderGBuffer)deferredBuffer->DebugRender();
			if (renderShadowMap)shadow->DebugRender();
#ifdef USE_SSAO
			if (renderSSAOBuffer)ssao->Render();
#endif
#ifdef USE_HDR
			if (renderBlumeBuffer) deferredShader->DebugRender();
#endif
		}
#endif
		//����̃t���[�����ʂ�ۑ�
		camera->GetView()->FrameEnd();
	}
}